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
  // #pragma OPENCL EXTENSION cl_khr_fp64 : enable
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


const char* densitySource = 
"__constant int edgeTable[256]={"
"0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,"
"0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,"
"0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,"
"0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,"
"0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,"
"0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,"
"0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,"
"0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,"
"0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,"
"0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,"
"0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,"
"0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,"
"0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,"
"0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,"
"0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,"
"0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,"
"0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,"
"0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,"
"0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,"
"0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,"
"0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,"
"0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,"
"0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,"
"0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,"
"0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,"
"0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,"
"0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,"
"0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,"
"0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,"
"0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,"
"0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,"
"0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0};\n"

"__constant int triTable[256][16] = {"
"{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},"
"{3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},"
"{3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},"
"{3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},"
"{9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},"
"{1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},"
"{9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},"
"{2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},"
"{8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},"
"{9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},"
"{4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},"
"{3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},"
"{1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},"
"{4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},"
"{4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},"
"{9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},"
"{1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},"
"{5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},"
"{2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},"
"{9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},"
"{0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},"
"{2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},"
"{10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},"
"{4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},"
"{5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},"
"{5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},"
"{9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},"
"{0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},"
"{1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},"
"{10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},"
"{8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},"
"{2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},"
"{7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},"
"{9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},"
"{2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},"
"{11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},"
"{9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},"
"{5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},"
"{11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},"
"{11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},"
"{1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},"
"{9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},"
"{5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},"
"{2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},"
"{0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},"
"{5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},"
"{6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},"
"{0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},"
"{3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},"
"{6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},"
"{5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},"
"{1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},"
"{10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},"
"{6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},"
"{1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},"
"{8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},"
"{7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},"
"{3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},"
"{5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},"
"{0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},"
"{9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},"
"{8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},"
"{5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},"
"{0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},"
"{6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},"
"{10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},"
"{10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},"
"{8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},"
"{1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},"
"{3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},"
"{0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},"
"{10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},"
"{0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},"
"{3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},"
"{6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},"
"{9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},"
"{8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},"
"{3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},"
"{6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},"
"{0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},"
"{10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},"
"{10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},"
"{1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},"
"{2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},"
"{7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},"
"{7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},"
"{2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},"
"{1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},"
"{11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},"
"{8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},"
"{0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},"
"{7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},"
"{10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},"
"{2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},"
"{6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},"
"{7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},"
"{2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},"
"{1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},"
"{10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},"
"{10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},"
"{0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},"
"{7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},"
"{6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},"
"{8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},"
"{9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},"
"{6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},"
"{1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},"
"{4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},"
"{10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},"
"{8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},"
"{0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},"
"{1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},"
"{8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},"
"{10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},"
"{4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},"
"{10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},"
"{5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},"
"{11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},"
"{9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},"
"{6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},"
"{7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},"
"{3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},"
"{7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},"
"{9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},"
"{3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},"
"{6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},"
"{9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},"
"{1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},"
"{4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},"
"{7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},"
"{6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},"
"{3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},"
"{0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},"
"{6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},"
"{1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},"
"{0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},"
"{11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},"
"{6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},"
"{5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},"
"{9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},"
"{1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},"
"{1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},"
"{10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},"
"{0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},"
"{5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},"
"{10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},"
"{11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},"
"{9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},"
"{7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},"
"{2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},"
"{8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},"
"{9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},"
"{9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},"
"{1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},"
"{9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},"
"{9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},"
"{5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},"
"{0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},"
"{10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},"
"{2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},"
"{0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},"
"{0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},"
"{9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},"
"{5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},"
"{3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},"
"{5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},"
"{8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},"
"{9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},"
"{0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},"
"{1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},"
"{3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},"
"{4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},"
"{9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},"
"{11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},"
"{11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},"
"{2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},"
"{9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},"
"{3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},"
"{1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},"
"{4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},"
"{4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},"
"{0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},"
"{3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},"
"{3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},"
"{0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},"
"{9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},"
"{1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},"
"{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};\n"


"float transformX(float xOnBlock, float xPos, float n)\n"
"{\n"
"  return xPos+(xOnBlock*1/(n-1));\n"
" }\n"

"float transformY(float yOnBlock, float yPos, float n)\n"
"{\n"
"  return yPos+(yOnBlock*1/(n-1));\n"
" }\n"

"float transformZ(float zOnBlock, float zPos, float n)\n"
"{\n"
"  return zPos-(zOnBlock*1/(n-1));\n"
" }\n"

"float4 transform(float4 onBlock, float4 Pos, float n)\n"
"{\n"
"  float4 ret;\n"
"  ret.x = Pos.x+(onBlock.x*1/(n-1));\n"
"  ret.y = Pos.y+(onBlock.y*1/(n-1));\n"
"  ret.z = Pos.z-(onBlock.z*1/(n-1));\n"
"  ret.w = 0.0f;\n"
"  return ret;\n"
" }\n"

"float densityFunction(float x, float y, float z)\n"
"{\n"
"  float temp = -(y-0.5);\n"
"  return temp;\n"
"}\n"

"float4 VertexInterp(float isolevel, float4 p1, float4 p2, float valp1, float valp2)\n"
"{\n"
"  float mu;\n"
"  float4 p;\n"
"  float temp = isolevel - valp1;\n"
"  if(temp < 0) temp *= -1;\n"
"  if (temp < 0.0001)\n"
"      return(p1);\n"

"  temp = isolevel - valp2;\n"
"  if(temp < 0) temp *= -1;\n"
"  if (temp < 0.0001)\n"
"      return(p2);\n"

"  temp = valp1-valp2;\n"
"  if(temp < 0) temp *= -1;\n"
"  if (temp < 0.0001)\n"
"      return(p1);\n"

"  mu = (isolevel - valp1) / (valp2 - valp1);\n"
"  p.x = p1.x + mu * (p2.x - p1.x);\n"
"  p.y = p1.y + mu * (p2.y - p1.y);\n"
"  p.z = p1.z + mu * (p2.z - p1.z);\n"
"  p.w = 0.0f;\n"

"   return(p); \n"
"}\n"

"__kernel void densityCalc(__global float density[],__global const float xPos[],__global const float yPos[], __global const float zPos[], "
                          " const float x, const float y, const float z, const unsigned int n,"
                           "__global float numTriangles[],__global float triangles[],__global float normals[])\n"
"{\n"
"  unsigned int i = get_global_id(0);\n"
"  unsigned int j = get_global_id(1);\n"
"  unsigned int k = get_global_id(2);\n"
"  float actualX = transformX(i, x, n);\n"
"  float actualY = transformY(j, y, n);\n"
"  float actualZ = transformZ(k, z, n);\n"
"  float4 actualPos = (float4) (x,y,z,0.0f);\n"
"  int i0,i1,i2,i3,i4,i5,i6,i7;\n"
"  i0 = k + j*n + i*n*n;\n"
"  i1 = (k+1) + j*n + i*n*n;\n"
"  i2 = (k+1) + j*n + (i+1)*n*n;\n"
"  i3 = k + j*n + (i+1)*n*n;\n"
"  i4 = k + (j+1)*n + i*n*n;\n"
"  i5 = (k+1) + (j+1)*n + i*n*n;\n"
"  i6 = (k+1) + (j+1)*n + (i+1)*n*n;\n"
"  i7 = k + (j+1)*n +(i+1)*n*n;\n"

"  float4 p0,p1,p2,p3,p4,p5,p6,p7;\n"
"  p0 = transform((float4)(i, j, k, 0.0f), actualPos, n);\n"
"  p1 = transform((float4)(i, j, k+1, 0.0f), actualPos, n);\n"
"  p2 = transform((float4)(i+1, j, k+1, 0.0f), actualPos, n);\n"
"  p3 = transform((float4)(i+1, j, k, 0.0f), actualPos, n);\n"
"  p4 = transform((float4)(i, j+1, k, 0.0f), actualPos, n);\n"
"  p5 = transform((float4)(i, j+1, k+1, 0.0f), actualPos, n);\n"
"  p6 = transform((float4)(i+1, j+1, k+1, 0.0f), actualPos, n);\n"
"  p7 = transform((float4)(i+1, j+1, k, 0.0f), actualPos, n);\n"

"  float isolevel = 0;\n"
"  density[k + j*n + i*n*n] = densityFunction(actualX, actualY, actualZ);\n"
"//}\n"
"/*__kernel void triangleCalc(__global float density[],"
                          " const float x, const float y, const float z, const unsigned int n,"
                          "__global float numTriangles[],__global float triangles[],__global float normals[])*/\n"
"  int numberOfTrianglesInThisWorkItem = 0;\n"
"  float4 empty = (float4) {0.0f,0.0f,0.0f,0.0f};\n"
"  float4 vertlist[12];\n"

"  float4 triangleArrays[5][4];\n"
"  int cubeindex = 0;\n"

"  if(i<=n && j<=n && k<=n)\n"
"  {\n"
"     if (density[i0] > isolevel) cubeindex |= 1;\n"
"     if (density[i1] > isolevel) cubeindex |= 2;\n"
"     if (density[i2] > isolevel) cubeindex |= 4;\n"
"     if (density[i3] > isolevel) cubeindex |= 8;\n"
"     if (density[i4] > isolevel) cubeindex |= 16;\n"
"     if (density[i5] > isolevel) cubeindex |= 32;\n"
"     if (density[i6] > isolevel) cubeindex |= 64;\n"
"     if (density[i7] > isolevel) cubeindex |= 128;\n"
"     /* Cube is entirely in/out of the surface */ \n"
"     if (edgeTable[cubeindex] == 0) \n"
"     {\n"
"        numberOfTrianglesInThisWorkItem = 0; \n"
"        for(int c = 0; c<12;c++)\n"
"           {  vertlist[c] = empty;}\n"
"     }\n"
"     else{ \n"
"   /* Find the vertices where the surface intersects the cube */"
         "if (edgeTable[cubeindex] & 1)\n"
            "vertlist[0] ="
               "VertexInterp(isolevel,p0,p1,density[i0],density[i1]) ;\n"
         "if (edgeTable[cubeindex] & 2)\n"
            "vertlist[1] ="
               "VertexInterp(isolevel,p1,p2,density[i1],density[i2]);\n"
         "if (edgeTable[cubeindex] & 4)\n"
            "vertlist[2] ="
               "VertexInterp(isolevel,p2,p3,density[i2],density[i3]);\n"
         "if (edgeTable[cubeindex] & 8)\n"
            "vertlist[3] ="
               "VertexInterp(isolevel,p3,p0,density[i3],density[i0]);\n"
         "if (edgeTable[cubeindex] & 16)\n"
            "vertlist[4] ="
               "VertexInterp(isolevel,p4,p5,density[i4],density[i5]);\n"
         "if (edgeTable[cubeindex] & 32)\n"
            "vertlist[5] ="
               "VertexInterp(isolevel,p5,p6,density[i5],density[i6]);\n"
         "if (edgeTable[cubeindex] & 64)\n"
            "vertlist[6] ="
               "VertexInterp(isolevel,p6,p7,density[i6],density[i7]);\n"
         "if (edgeTable[cubeindex] & 128)\n"
            "vertlist[7] ="
               "VertexInterp(isolevel,p7,p4,density[i7],density[i4]);\n"
         "if (edgeTable[cubeindex] & 256)\n"
            "vertlist[8] ="
               "VertexInterp(isolevel,p0,p4,density[i0],density[i4]);\n"
         "if (edgeTable[cubeindex] & 512)\n"
            "vertlist[9] ="
               "VertexInterp(isolevel,p1,p5,density[i1],density[i5]);\n"
         "if (edgeTable[cubeindex] & 1024)\n"
            "vertlist[10] ="
               "VertexInterp(isolevel,p2,p6,density[i2],density[i6]);\n"
         "if (edgeTable[cubeindex] & 2048)\n"
            "vertlist[11] ="
               "VertexInterp(isolevel,p3,p7,density[i3],density[i7]);\n"
"     /* Create the triangle */ \n"
   "     int ntriang = 0;\n"
   "     int w = 0;\n"

   "     //for (int w=0;triTable[cubeindex][w]!=-1;w+=3)\n"
   "     for (int h=0;h<5;h+=1)\n"
   "     { \n"
   "        triangleArrays[h][0] = vertlist[triTable[cubeindex][w  ]];\n"
   "        triangleArrays[h][1] = vertlist[triTable[cubeindex][w+1]];\n"
   "        triangleArrays[h][2] = vertlist[triTable[cubeindex][w+2]];\n"
   "        triangleArrays[h][3] = empty;\n"
   "        w+=3;\n"
   "        if(triTable[cubeindex][w] != -1) ntriang++;\n"
   "     }\n"
   "     numberOfTrianglesInThisWorkItem = ntriang;\n"
"     }\n"
"  }\n"
"  numTriangles[k + j*n + i*n*n] = numberOfTrianglesInThisWorkItem;\n"
"  for(int h = 0; h < 5; h+=1)\n"
"     {\n"
"        triangles[(k*5 + j*n*5 + i*n*n*5)+0] = triangleArrays[h][0].x;\n"
"        triangles[(k*5 + j*n*5 + i*n*n*5)+1] = triangleArrays[h][0].y;\n"
"        triangles[(k*5 + j*n*5 + i*n*n*5)+2] = triangleArrays[h][0].z;\n"
"        triangles[(k*5 + j*n*5 + i*n*n*5)+3] = triangleArrays[h][0].w;\n"

"        triangles[(k*5 + j*n*5 + i*n*n*5)+4] = triangleArrays[h][1].x;\n"
"        triangles[(k*5 + j*n*5 + i*n*n*5)+5] = triangleArrays[h][1].y;\n"
"        triangles[(k*5 + j*n*5 + i*n*n*5)+6] = triangleArrays[h][1].z;\n"
"        triangles[(k*5 + j*n*5 + i*n*n*5)+7] = triangleArrays[h][1].w;\n"

"        triangles[(k*5 + j*n*5 + i*n*n*5)+8] = triangleArrays[h][2].x;\n"
"        triangles[(k*5 + j*n*5 + i*n*n*5)+9] = triangleArrays[h][2].y;\n"
"        triangles[(k*5 + j*n*5 + i*n*n*5)+10] = triangleArrays[h][2].z;\n"
"        triangles[(k*5 + j*n*5 + i*n*n*5)+11] = triangleArrays[h][2].w;\n"
"     }\n"
" }\n"; 


void densityCalc(float h_numTriangles[], float h_triangles[], float h_normals[], 
         float h_density[], float h_xPos[], float h_yPos[], float h_zPos[], 
         float cornerPosX, float cornerPosY, float cornerPosZ)
{
   // Calculate matrix dimensions
   int n = cubeDimPlusEdge;
   int N = cubeDimPlusEdge*cubeDimPlusEdge*cubeDimPlusEdge*sizeof(float);
   int NTRI3 = N*5*3*3;
   int NTRI4 = N*5*4*3;
   //Allocate device memory and copy A&B from host to device
   cl_int err;
   cl_mem d_xPos = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,n,h_xPos,&err);
   if(err) Fatal("Cannot create and copy xPos from host to device\n");
   cl_mem d_yPos = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,n,h_yPos,&err);
   if(err) Fatal("Cannot create and copy yPos from host to device\n");
   cl_mem d_zPos = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,n,h_zPos,&err);
   if(err) Fatal("Cannot create and copy zPos from host to device\n");

   //Allocate device memory for C on device
   cl_mem d_density = clCreateBuffer(context, CL_MEM_READ_WRITE,N,NULL,&err);
   if(err) Fatal("Cannot create density on device, sad day indeed\n");
   cl_mem d_numTriangles = clCreateBuffer(context, CL_MEM_WRITE_ONLY,N,NULL,&err);
   if(err) Fatal("Cannot create numTriangles on device, sad day indeed\n");
   cl_mem d_triangles = clCreateBuffer(context, CL_MEM_WRITE_ONLY,NTRI4,NULL,&err);
   if(err) Fatal("Cannot create triangles on device, sad day indeed\n");
   cl_mem d_normals = clCreateBuffer(context, CL_MEM_WRITE_ONLY,NTRI3,NULL,&err);
   if(err) Fatal("Cannot create normals on device, sad day indeed\n");

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
   if(clSetKernelArg(kernel,8,sizeof(cl_mem),&d_numTriangles)) Fatal("Cannot set kernel parameter d_numTriangles\n");
   if(clSetKernelArg(kernel,9,sizeof(cl_mem),&d_triangles)) Fatal("Cannot set kernel parameter d_triangles\n");
   if(clSetKernelArg(kernel,10,sizeof(cl_mem),&d_normals)) Fatal("Cannot set kernel parameter d_normals\n");


   //Run Kernel
   size_t Global[3] = {n,n,n};
   //size_t Local[2]  = {10,10};
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

/*void triangleCalc(float h_numTriangles[], float h_triangles[], float h_normals[], 
         float h_density[], 
         float cornerPosX, float cornerPosY, float cornerPosZ)
{
   // Calculate matrix dimensions
   int n = cubeDimPlusEdge;
   int N = cubeDimPlusEdge*cubeDimPlusEdge*cubeDimPlusEdge*sizeof(float);
   int NTRI3 = N*5*3*3;
   int NTRI4 = N*5*4*3;

   //Allocate device memory and copy A&B from host to device
   cl_int err;
   cl_mem d_density = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,n,h_density,&err);
   if(err) Fatal("Cannot create and copy density from host to device\n");

   //Allocate device memory for C on device
   cl_mem d_numTriangles = clCreateBuffer(context, CL_MEM_WRITE_ONLY,N,NULL,&err);
   if(err) Fatal("Cannot create numTriangles on device, sad day indeed\n");
   cl_mem d_triangles = clCreateBuffer(context, CL_MEM_WRITE_ONLY,NTRI4,NULL,&err);
   if(err) Fatal("Cannot create triangles on device, sad day indeed\n");
   cl_mem d_normals = clCreateBuffer(context, CL_MEM_WRITE_ONLY,NTRI3,NULL,&err);
   if(err) Fatal("Cannot create normals on device, sad day indeed\n");

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
   if(clSetKernelArg(kernel,8,sizeof(cl_mem),&d_numTriangles)) Fatal("Cannot set kernel parameter d_numTriangles\n");
   if(clSetKernelArg(kernel,9,sizeof(cl_mem),&d_triangles)) Fatal("Cannot set kernel parameter d_triangles\n");
   if(clSetKernelArg(kernel,10,sizeof(cl_mem),&d_normals)) Fatal("Cannot set kernel parameter d_normals\n");


   //Run Kernel
   size_t Global[3] = {n,n,n};
   //size_t Local[2]  = {10,10};
   if (clEnqueueNDRangeKernel(queue,kernel,3,NULL,Global,NULL,0,NULL,NULL)) Fatal("Cannot run kernel\n");

   //  Release kernel and program
   if (clReleaseKernel(kernel)) Fatal("Cannot release kernel\n");
   if (clReleaseProgram(prog)) Fatal("Cannot release program\n");

   // Copy C from device to host (block until done)
   if (clEnqueueReadBuffer(queue,d_density,CL_TRUE,0,N,h_density,0,NULL,NULL)) Fatal("Cannot copy density from device to host\n");
   if (clEnqueueReadBuffer(queue,d_numTriangles,CL_TRUE,0,N,h_numTriangles,0,NULL,NULL)) Fatal("Cannot copy density from device to host\n");
   if (clEnqueueReadBuffer(queue,d_triangles,CL_TRUE,0,N,h_triangles,0,NULL,NULL)) Fatal("Cannot copy density from device to host\n");
   if (clEnqueueReadBuffer(queue,d_normals,CL_TRUE,0,N,h_normals,0,NULL,NULL)) Fatal("Cannot copy density from device to host\n");

   //  Free device memory
   clReleaseMemObject(d_xPos);
   clReleaseMemObject(d_yPos);
   clReleaseMemObject(d_zPos);
   clReleaseMemObject(d_density);
}
*/
/*
 *  main
 */
int main(int argc, char* argv[])
{
  
   int verbose=0;
 

   //  Initialize GPU
   int Mw = InitGPU(verbose);
   cout << "Max Work group size is " << Mw << endl;
 

   //Start other calculation
   cout << "Now to calculate the density" << endl;

   
   //Allocate host memory for density calcs
   int n = cubeDimPlusEdge;
   int N = n*n*n*sizeof(float);

   float* h_xPos = (float*)malloc(n*sizeof(float));
   float* h_yPos = (float*)malloc(n*sizeof(float));
   float* h_zPos = (float*)malloc(n*sizeof(float));
   float* h_density = (float*)malloc(N);
   float* h_triangles = (float*)malloc(4*3*5*n*n*n*sizeof(float));
   float* h_normals = (float*)malloc(3*3*5*n*n*n*sizeof(float));
   float* h_numTriangles = (float*)malloc(N);
   if (!h_xPos || !h_yPos || !h_zPos || !h_density || !h_triangles || !h_normals || !h_numTriangles) Fatal("Cannot allocate host memory\n");

   //Initialize x, y, z
   generateBlockSide(h_xPos);
   printMatrix(h_xPos, n);
   generateBlockSide(h_yPos);
   printMatrix(h_yPos, n);
   generateBlockSide(h_zPos);
   printMatrix(h_zPos, n);
   
   //Perform density calculation
   densityCalc(h_numTriangles, h_normals, h_triangles, h_density,h_xPos,h_yPos,h_zPos,0,0,0);
   cout << "here" << endl;
   //Print out the density
   printMatrixInSmallestIncs(h_density, n*n*n, n);
   printMatrixInSmallestIncs(h_triangles, n*n*n*3*4*5, 4);
   

   free(h_xPos);
   free(h_yPos);
   free(h_zPos);
   free(h_density);
   free(h_triangles);
   free(h_normals);
   free(h_numTriangles);


   //  Done
   return 0;
}
