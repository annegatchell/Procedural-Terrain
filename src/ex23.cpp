/*
 *  OpenCL square matrix multiplier
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <math.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

using namespace std;

#define cubeDim 3
#define cubeDimPlusEdge 4

/*
 *  Return elapsed wall time since last call (seconds)
 */
static double t0=0;
float Elapsed(void)
{
#ifdef _WIN32
   //  Windows version of wall time
   LARGE_INTEGER tv,freq;
   QueryPerformanceCounter((LARGE_INTEGER*)&tv);
   QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
   double t = tv.QuadPart/(double)freq.QuadPart;
#else
   //  Unix/Linux/OSX version of wall time
   struct timeval tv;
   gettimeofday(&tv,NULL);
   double t = tv.tv_sec+1e-6*tv.tv_usec;
#endif
   float s = t-t0;
   t0 = t;
   return s;
}

void printMatrix(float x[], int dim)
{
   for(int i = 0;i<dim;i++)
      cout << "i " << x[i] << " ";
   cout << endl;
}

void printMatrixInSmallestIncs(float x[], int dim, int smallest)
{
    for(int i = 0;i<dim;i++)
   {
      if(!(i%smallest))
         cout << endl;
      cout << "i " << x[i] << " ";
   }
   cout << endl;
}

void printLinearized3dMatrix(float x[], int totalDim, int xDim, int yDim, int zDim)
{
   for(int i = 0; i<xDim; i++)
   {
      for(int j=0; j<yDim; j++)
      {
         for(int k=0; k<zDim;k++)
         {
            cout << "i"<<i<<"j"<<j<<"k"<<k<< " "<< x[k + j*xDim + i*zDim*yDim] << " ";
         }
      }
   }
   cout << endl;
}

void OPprintLinearized3dMatrix(float x[], int totalDim, int xDim, int yDim, int zDim)
{
   for(int k = 0; k<zDim; k++)
   {
      for(int j=0; j<yDim; j++)
      {
         for(int i=0; i<xDim;i++)
         {
            cout << "i"<<i<<"j"<<j<<"k"<<k<< " "<< x[k + j*xDim + i*xDim*yDim] << " ";
         }
      }
   }
   cout << endl;
}

/*
 *  Print message to stderr and exit
 */
void Fatal(const char* format , ...)
{
   va_list args;
   va_start(args,format);
   vfprintf(stderr,format,args);
   va_end(args);
   exit(1);
}

/*
 *  Initialize matrix with random values
 */
void RandomInit(float x[],const unsigned int n)
{
   for (unsigned int i=0;i<n*n;i++)
      x[i] = rand() / (float)RAND_MAX;
}

void generate1dMatrix(float x[], unsigned int dim)
{
   for(unsigned int i=0;i<dim;i++)
   {
      x[i] = i;
   }
}

void generate3dMatrix(float x[], int xDim, int yDim, int zDim)
{
   int g = 0;
   for(int i = 0; i<xDim; i++)
   {
      for(int j=0; j<yDim; j++)
      {
         for(int k=0; k<zDim;k++)
         {
            x[g] = i*100+j*10+k*1;
            g++;
         }
      }
   }
}

void generateBlockSide(float x[])
{
   generate1dMatrix(x, cubeDimPlusEdge);
}
/*
 *  OpenCL notify callback (echo to stderr)
 */
void Notify(const char* errinfo,const void* private_info,size_t cb,void* user_data)
{
   fprintf(stderr,"%s\n",errinfo);
}

/*
 *  Initialize fastest OpenCL device
 */
cl_device_id     devid;
cl_context       context;
cl_command_queue queue;
int InitGPU(int verbose)
{
   cl_uint Nplat;
   cl_int  err;
   char name[1024];
   int  MaxGflops = -1;

   //  Get platforms
   cl_platform_id platforms[1024];
   if (clGetPlatformIDs(1024,platforms,&Nplat))
      Fatal("Cannot get number of OpenCL devices\n");
   else if (Nplat<1)
      Fatal("No OpenCL platforms found\n");
   //  Loop over platforms
   for (unsigned int platform=0;platform<Nplat;platform++)
   {
      if (clGetPlatformInfo(platforms[platform],CL_PLATFORM_NAME,sizeof(name),name,NULL)) Fatal("Cannot get OpenCL platform name\n");
      if (verbose) printf("OpenCL Platform %d: %s\n",platform,name);

      //  Get GPU device IDs
      cl_uint Ndev;
      cl_device_id id[1024];
      if (clGetDeviceIDs(platforms[platform],CL_DEVICE_TYPE_GPU,1024,id,&Ndev))
         Fatal("Cannot get number of OpenCL devices\n");
      else if (Ndev<1)
         Fatal("No OpenCL devices found\n");

      //  Find the fastest device
      for (unsigned int dev=0;dev<Ndev;dev++)
      {
         cl_uint proc,freq;
         if (clGetDeviceInfo(id[dev],CL_DEVICE_MAX_COMPUTE_UNITS,sizeof(proc),&proc,NULL)) Fatal("Cannot get OpenCL device units\n");
         if (clGetDeviceInfo(id[dev],CL_DEVICE_MAX_CLOCK_FREQUENCY,sizeof(freq),&freq,NULL)) Fatal("Cannot get OpenCL device frequency\n");
         if (clGetDeviceInfo(id[dev],CL_DEVICE_NAME,sizeof(name),name, NULL)) Fatal("Cannot get OpenCL device name\n");
         int Gflops = proc*freq;
         if (verbose) printf("OpenCL Device %d: %s Gflops %f\n",dev,name,1e-3*Gflops);
         if(Gflops > MaxGflops)
         {
            devid = id[dev];
            MaxGflops = Gflops;
         }
      }
   }

   //  Print fastest device info
   if (clGetDeviceInfo(devid,CL_DEVICE_NAME,sizeof(name),name,NULL)) Fatal("Cannot get OpenCL device name\n");
   printf("Fastest OpenCL Device: %s\n",name);

   //  Check thread count
   size_t mwgs;
   if (clGetDeviceInfo(devid,CL_DEVICE_MAX_WORK_GROUP_SIZE,sizeof(mwgs),&mwgs,NULL)) Fatal("Cannot get OpenCL max work group size\n");

   size_t mwis[3];
   if (clGetDeviceInfo(devid, CL_DEVICE_MAX_WORK_ITEM_SIZES,sizeof(mwis), &mwis,NULL)) Fatal("Cannot get OpenCL max work item sizes\n");
   cout << "max work item size " << mwis[0] << " " << mwis[1] << " " << mwis[2] << endl;

   
   //  Create OpenCL context for fastest device
   context = clCreateContext(0,1,&devid,Notify,NULL,&err);
   if(!context || err) Fatal("Cannot create OpenCL context\n");

   //  Create OpenCL command queue for fastest device
   queue = clCreateCommandQueue(context,devid,0,&err);
   if(!queue || err) Fatal("Cannot create OpenCL command cue\n");

   return mwgs;
} 

/*
 * C = A * B -- host
 */
void AxBh(float C[], const float A[], const float B[], unsigned int n)
{
   for (unsigned int i=0;i<n;i++)
      for (unsigned int j=0;j<n;j++)
      {
         double sum=0;
         for (unsigned int k=0;k<n;k++)
            sum += (double)A[i*n+k] * (double)B[k*n+j];
         C[i*n+j] = (float)sum;
      }
}
 
/*
* Compute one element of A * B
*/
const char* source =
  "__kernel void AxB(__global float C[],__global const float A[],__global const float B[],const unsigned int n)\n"
  "{\n"
  "   unsigned int j = get_global_id(0);\n"
  "   unsigned int i = get_global_id(1);\n"
  "   float sum =0;\n"
  "   for (int k=0;k<n;k++)\n"
  "      sum = A[i*n+k] * B[k*n+j];\n"
  "   C[i*n+j] = sum;\n"
  "}\n";

const char* densitySource = 
"float transformX(int xOnBlock, float xPos, float n)\n"
"{\n"
"  return xPos+(xOnBlock*1/(n-1));\n"
" }\n"

"float transformY(int yOnBlock, float yPos, float n)\n"
"{\n"
"  return yPos+(yOnBlock*1/(n-1));\n"
" }\n"

"float transformZ(int zOnBlock, float zPos, float n)\n"
"{\n"
"  return zPos-(zOnBlock*1/(n-1));\n"
" }\n"

"float densityFunction(float x, float y, float z)\n"
"{\n"
"  float temp = -(y-0.5);\n"
"  return temp;\n"
"}\n"

"__kernel void densityCalc(__global float density[],__global const float xPos[],__global const float yPos[], __global const float zPos[], "
                          " const float x, const float y, const float z, const unsigned int n)\n"
"{\n"
"  unsigned int i = get_global_id(0);\n"
"  unsigned int j = get_global_id(1);\n"
"  unsigned int k = get_global_id(2);\n"
"  float actualX = transformX(i, x, n);\n"
"  float actualY = transformX(j, y, n);\n"
"  float actualZ = transformX(k, z, n);\n"
"  density[k + j*n + i*n*n] = densityFunction(actualX, actualY, actualZ);\n"
" }\n"; 


void densityCalc(float h_density[], float h_xPos[], float h_yPos[], float h_zPos[], float cornerPosX, float cornerPosY, float cornerPosZ)
{
   // Calculate matrix dimensions
   int n = cubeDimPlusEdge;
   int N = cubeDimPlusEdge*cubeDimPlusEdge*cubeDimPlusEdge*sizeof(float);

   //Allocate device memory and copy A&B from host to device
   cl_int err;
   cl_mem d_xPos = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,n,h_xPos,&err);
   if(err) Fatal("Cannot create and copy xPos from host to device\n");
   cl_mem d_yPos = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,n,h_yPos,&err);
   if(err) Fatal("Cannot create and copy yPos from host to device\n");
   cl_mem d_zPos = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,n,h_zPos,&err);
   if(err) Fatal("Cannot create and copy zPos from host to device\n");

   //Allocate device memory for C on device
   cl_mem d_density = clCreateBuffer(context, CL_MEM_WRITE_ONLY,N,NULL,&err);
   if(err) Fatal("Cannot create density on device, sad day indeed\n");

   //Compile kernel
   cl_program prog = clCreateProgramWithSource(context,1,&densitySource,0,&err);
   if(err) Fatal("Cannot create density program on device");
   if(clBuildProgram(prog,0,NULL,NULL,NULL,NULL))
   {
      char log[1048576];
      if(clGetProgramBuildInfo(prog,devid,CL_PROGRAM_BUILD_LOG,sizeof(log),log,NULL))
         Fatal("Cannot get build log\n");
      else
         Fatal("Cannot build program\n%s\n",log);
   }
   cl_kernel kernel = clCreateKernel(prog,"densityCalc",&err);
   if(err) Fatal("Cannot create kernel\n");

   //Set parameters for kernel
   if(clSetKernelArg(kernel,0,sizeof(cl_mem),&d_density)) Fatal("Cannot set kernel parameter d_density\n");
   if(clSetKernelArg(kernel,1,sizeof(cl_mem),&d_xPos)) Fatal("Cannot set kernel parameter d_xPos\n");
   if(clSetKernelArg(kernel,2,sizeof(cl_mem),&d_yPos)) Fatal("Cannot set kernel parameter d_yPos\n");
   if(clSetKernelArg(kernel,3,sizeof(cl_mem),&d_zPos)) Fatal("Cannot set kernel parameter d_zPos\n");
   if(clSetKernelArg(kernel,4,sizeof(float),&cornerPosX)) Fatal("Cannot set kernel parameter x\n");
   if(clSetKernelArg(kernel,5,sizeof(float),&cornerPosY)) Fatal("Cannot set kernel parameter y\n");
   if(clSetKernelArg(kernel,6,sizeof(float),&cornerPosZ)) Fatal("Cannot set kernel parameter z\n");
   if(clSetKernelArg(kernel,7,sizeof(int),&n)) Fatal("Cannot set kernel parameter n\n");

   //Run Kernel
   size_t Global[3] = {n,n,n};
   if (clEnqueueNDRangeKernel(queue,kernel,3,NULL,Global,NULL,0,NULL,NULL)) Fatal("Cannot run kernel\n");

   //  Release kernel and program
   if (clReleaseKernel(kernel)) Fatal("Cannot release kernel\n");
   if (clReleaseProgram(prog)) Fatal("Cannot release program\n");

   // Copy C from device to host (block until done)
   if (clEnqueueReadBuffer(queue,d_density,CL_TRUE,0,N,h_density,0,NULL,NULL)) Fatal("Cannot copy density from device to host\n");

   //  Free device memory
   clReleaseMemObject(d_xPos);
   clReleaseMemObject(d_yPos);
   clReleaseMemObject(d_zPos);
   clReleaseMemObject(d_density);
}

/*
 * C = A * B -- device
 */
void AxBd(float Ch[],float Ah[],float Bh[],const unsigned int Bw,const unsigned int Bn)
{
   //  Calculate matrix dimensions
   int n = Bw*Bn;
   int N = n*n*sizeof(float);

   // Allocate device memory and copy A&B from host to device
   cl_int  err;
   cl_mem Ad = clCreateBuffer(context,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,N,Ah,&err);
   if (err) Fatal("Cannot create and copy A from host to device\n");
   cl_mem Bd = clCreateBuffer(context,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,N,Bh,&err);
   if (err) Fatal("Cannot create and copy B from host to device\n");

   //  Allocate device memory for C on device
   cl_mem Cd = clCreateBuffer(context,CL_MEM_WRITE_ONLY,N,NULL,&err);
   if (err) Fatal("Cannot create C on device\n");

   //  Compile kernel
   cl_program prog = clCreateProgramWithSource(context,1,&source,0,&err);
   if (err) Fatal("Cannot create program\n");
   if (clBuildProgram(prog,0,NULL,NULL,NULL,NULL))
   {
      char log[1048576];
      if (clGetProgramBuildInfo(prog,devid,CL_PROGRAM_BUILD_LOG,sizeof(log),log,NULL))
         Fatal("Cannot get build log\n");
      else
         Fatal("Cannot build program\n%s\n",log);
   }
   cl_kernel kernel = clCreateKernel(prog,"AxB",&err);
   if (err) Fatal("Cannot create kernel\n");

   size_t wgs;
   if(clGetKernelWorkGroupInfo(kernel, devid, CL_KERNEL_WORK_GROUP_SIZE, sizeof(wgs), &wgs,NULL)) Fatal("Cannot get OpenCL kernel work group size\n");
   cout << "Kernel work group info " << wgs << endl;

   //  Set parameters for kernel
   if (clSetKernelArg(kernel,0,sizeof(cl_mem),&Cd)) Fatal("Cannot set kernel parameter Cd\n");
   if (clSetKernelArg(kernel,1,sizeof(cl_mem),&Ad)) Fatal("Cannot set kernel parameter Ad\n");
   if (clSetKernelArg(kernel,2,sizeof(cl_mem),&Bd)) Fatal("Cannot set kernel parameter Bd\n");
   if (clSetKernelArg(kernel,3,sizeof(int),&n)) Fatal("Cannot set kernel parameter n\n");

   //  Run kernel
   size_t Global[2] = {n,n};
   size_t Local[2]  = {Bw,Bw};
   if (clEnqueueNDRangeKernel(queue,kernel,2,NULL,Global,Local,0,NULL,NULL)) Fatal("Cannot run kernel\n");

   //  Release kernel and program
   if (clReleaseKernel(kernel)) Fatal("Cannot release kernel\n");
   if (clReleaseProgram(prog)) Fatal("Cannot release program\n");

   // Copy C from device to host (block until done)
   if (clEnqueueReadBuffer(queue,Cd,CL_TRUE,0,N,Ch,0,NULL,NULL)) Fatal("Cannot copy C from device to host\n");

   //  Free device memory
   clReleaseMemObject(Ad);
   clReleaseMemObject(Bd);
   clReleaseMemObject(Cd);
}

/*
 *  main
 */
int main(int argc, char* argv[])
{
   //  Process options
   int opt;
   int verbose=0;
   while ((opt=getopt(argc,argv,"v"))!=-1)
   {
      if (opt=='v')
         verbose++;
      else
         Fatal("Usage: [-v] <block width> <number of blocks>\n");
   }
   argc -= optind;
   argv += optind;
 
   //  Get width and number of blocks
   if (argc!=2) Fatal("Usage: [-v] <block width> <number of blocks>\n");
   int Bw = atoi(argv[0]);
   if (Bw<1) Fatal("Block width out of range %d\n",Bw);
   int Bn = atoi(argv[1]);
   if (Bn<1) Fatal("Number of blocks out of range %d\n",Bn);
   //  Total width is block times number of blocks
   int n = Bw*Bn;
   int N = n*n*sizeof(float);
   printf("Bw=%d Bn=%d n=%d\n",Bw,Bn,n);

   //  Initialize GPU
   int Mw = InitGPU(verbose);
   cout << "Max Work group size is " << Mw << endl;
   if (Mw<Bw*Bw) Fatal("Thread count %d exceeds max work group size of %d\n",Bw*Bw,Mw);


   // Allocate host matrices A/B/C/R
   float* Ah = (float*)malloc(N);
   float* Bh = (float*)malloc(N);
   float* Ch = (float*)malloc(N);
   float* Rh = (float*)malloc(N);
   if (!Ah || !Bh || !Ch || !Rh) Fatal("Cannot allocate host memory\n");

   // Initialize A & B
   srand(9999);
   RandomInit(Ah,n);
   RandomInit(Bh,n);


   //  Compute R = AB on host
   Elapsed();
   AxBh(Rh,Ah,Bh,n);
   float Th = Elapsed();

   //  Compute C = AB on device
   Elapsed();
   AxBd(Ch,Ah,Bh,Bw,Bn);
   float Td = Elapsed();

   //  Compute difference between R and C
   double r2=0;
   for (int i=0;i<n*n;i++)
      r2 += fabs(Ch[i]-Rh[i]);
   r2 /= n*n;

   //  Free host memory
   free(Ah);
   free(Bh);
   free(Ch);
   free(Rh);

   //  Print results
   printf("Host   Time = %6.3f s\n",Th);
   printf("Device Time = %6.3f s\n",Td);
   printf("Speedup = %.1f\n",Th/Td);
   printf("Difference = %.2e\n",r2);

   //Start other calculation
   cout << "Now to calculate the density" << endl;

   
   //Allocate host memory for density calcs
   n = cubeDimPlusEdge;
   N = n*n*n*sizeof(float);

   float* test3d = (float*)malloc(N);
   generate1dMatrix(test3d, n*n*n);
   printMatrix(test3d, n*n*n);
   printLinearized3dMatrix(test3d, n*n*n, n, n, n);
   cout << endl;
   OPprintLinearized3dMatrix(test3d, n*n*n, n,n,n);

   cout << endl;
   float* test3dAgain = (float*)malloc(N);
   generate3dMatrix(test3dAgain, n,n,n);
   printLinearized3dMatrix(test3dAgain,n*n*n, n,n,n);
    cout << endl;
   OPprintLinearized3dMatrix(test3dAgain,n*n*n, n,n,n);

   float* h_xPos = (float*)malloc(n);
   float* h_yPos = (float*)malloc(n);
   float* h_zPos = (float*)malloc(n);
   float* h_density = (float*)malloc(N);
   if (!h_xPos || !h_yPos || !h_zPos || !h_density) Fatal("Cannot allocate host memory\n");

   //Initialize x, y, z
   generateBlockSide(h_xPos);
   printMatrix(h_xPos, n);
   generateBlockSide(h_yPos);
   printMatrix(h_yPos, n);
   generateBlockSide(h_zPos);
   printMatrix(h_zPos, n);
   
   
   
   //Perform density calculation
   densityCalc(h_density,h_xPos,h_yPos,h_zPos,0,0,0);

   //Print out the density
   printMatrixInSmallestIncs(h_density, n*n*n, n);


   free(h_xPos);
   free(h_yPos);
   free(h_zPos);
   free(h_density);


   //  Done
   return 0;
}
