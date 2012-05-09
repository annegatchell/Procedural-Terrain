/*
 *  OpenCL square matrix multiplier
 */
#include "CSCIx239.h"
#include "shader.c"

#include <iostream>
#include <vector>;

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

#define cubeDim 32
#define cubeDimPlusEdge 33
#define numBlocks 2*2*2
#define xVoxelWidth 2
#define yVoxelWidth 2
#define zVoxelWidth 2

int axes=1;       //  Display axes
int mode=0;       //  Shader mode
int move=1;       //  Move light
int proj=0;       //  Projection type
int obj=0;        //  Object
int th=0;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
int fov=55;       //  Field of view (for perspective)
double asp=1;     //  Aspect ratio
double dim=3.0;   //  Size of world
int model;        //  Model display list
int zh=90;        //  Light azimuth
float Ylight=4;   //  Light elevation
int shader[] = {0,0,0}; //  Shader program
char* text[] = {"No Shader","Simple Shader","Basic Shader"};

float noise[16][16][16];


typedef struct
{
   float x;
   float y;
   float z;
} float3;

typedef struct
{
   float x;
   float y;
   float z;
   float w;
} float4;

typedef struct
{
   float4 coordAmb;
   float4 normal;
} rockVertex;

typedef struct
{
   rockVertex rv1;
   rockVertex rv2;
   rockVertex rv3;
} triangle;

typedef struct
{
   float3 coordAmb;
   float3 normal;
} rockVertex3;

typedef struct
{
   rockVertex3 rv1;
   rockVertex3 rv2;
   rockVertex3 rv3;
} triangle3;

typedef struct
{
   float3 rv1;
   float3 rv2;
   float3 rv3;  
} triangleVert;

typedef struct
{
   float3 n1;
   float3 n2;
   float3 n3;
} triangleNorm;

float* verteces;
triangleVert* trianglePtr;
triangleNorm* normalPtr;
int numTriangles;


//vector<triangleVert> allTriangles;
//vector<triangleNorm> allNormals;

triangleVert *trianglePtrArray[numBlocks];
triangleNorm *normalPtrArray[numBlocks];
int numTrianglesPtrArray[numBlocks];

triangleVert allPossibleTriangleVerts[5*cubeDim*cubeDim*cubeDim*numBlocks];
triangleNorm allPossibleTriangleNorms[5*cubeDim*cubeDim*cubeDim*numBlocks];

int totalNumberTriangles=0;

struct block
{
   float3 position;
   bool hasBeenGenerated;
   bool isEmpty;

   int numTriangles;
   triangleVert realTriangles[];
   triangleNorm realNormals[];

   block()
   {
   position = position;
    hasBeenGenerated = hasBeenGenerated;
    isEmpty = isEmpty;
    numTriangles = numTriangles;
    //realTriangles = new triangleVert[cubeDim*cubeDim*cubeDim*5];
    //realNormals = new triangleNorm[cubeDim*cubeDim*cubeDim*5]; 
   }
   
   block(float3 position, bool hasBeenGenerated, bool isEmpty, int numTriangles, triangleVert realTriangles[],
   triangleNorm realNormals[])
   {
    position = position;
    hasBeenGenerated = hasBeenGenerated;
    isEmpty = isEmpty;
    numTriangles = numTriangles;
    realTriangles = new triangleVert[numTriangles];
    realTriangles = realTriangles;
    realNormals = new triangleNorm[numTriangles]; 
    realNormals = realNormals; 
   }

};
int STUPID = 0;
block *voxelGroup;

/*

static int    p[MAXB + MAXB + 2];
static double g3[MAXB + MAXB + 2][3];

static int start=0;
static int B;
static int BM;
*/
//
//  Normalize 3D vector
//
/*void normalize3(double v[3])
{
   double s;

   s = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
   v[0] /=  s;
   v[1] /=  s;
   v[2] /=  s;
}

//
//  Initialize Noise
//
void InitNoise(void)
{
   int i,j,k;

   srand(30757);
   for (i=0;i<B;i++)
   {
      p[i] = i;
      for (j=0;j<3;j++)
         g3[i][j] = (double)((rand() % (B + B)) - B) / B;
      normalize3(g3[i]);
   }

   while (--i)
   {
      k = p[i];
      p[i] = p[j = rand() % B];
      p[j] = k;
   }

   for (i=0;i<B+2;i++)
   {
      p[B+i] = p[i];
      for (j=0;j<3;j++)
         g3[B+i][j] = g3[i][j];
   }
}

//
//  Set noise frequency
//
void setNoiseFrequency(int frequency)
{
   start = 1;
   B = frequency;
   BM = B-1;
}
double noise3(double vec[3])
{
   int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
   double rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
   int i, j;

   if (start)
   {
      start = 0;
      InitNoise();
   }

   setup(0, bx0, bx1, rx0, rx1);
   setup(1, by0, by1, ry0, ry1);
   setup(2, bz0, bz1, rz0, rz1);

   i = p[bx0];
   j = p[bx1];

   b00 = p[i + by0];
   b10 = p[j + by0];
   b01 = p[i + by1];
   b11 = p[j + by1];

   t  = s_curve(rx0);
   sy = s_curve(ry0);
   sz = s_curve(rz0);

   q = g3[b00 + bz0]; u = at3(rx0, ry0, rz0);
   q = g3[b10 + bz0]; v = at3(rx1, ry0, rz0);
   a = lerp(t,u,v);

   q = g3[b01 + bz0]; u = at3(rx0, ry1, rz0);
   q = g3[b11 + bz0]; v = at3(rx1, ry1, rz0);
   b = lerp(t, u, v);

   c = lerp(sy,a,b);

   q = g3[b00 + bz1]; u = at3(rx0, ry0, rz1);
   q = g3[b10 + bz1]; v = at3(rx1, ry0, rz1);
   a = lerp(t, u, v);

   q = g3[b01 + bz1]; u = at3(rx0, ry1, rz1);
   q = g3[b11 + bz1]; v = at3(rx1, ry1, rz1);
   b = lerp(t, u, v);

   d = lerp(sy,a,b);

   return lerp(sz,c,d);
}


void make3DNoiseTexture(void)
{
   int f, i, j,k,inc;
   int startFrequency = 4;
   int numOctaves = 4;
   double ni[3];
   double inci, incj, inck;
   int frequency = startFrequency;
   GLubyte *ptr;
   double amp = 0.5;
   int size = 16;

   for(f = 0, inc = 0; f <numOctaves;
      f++, frequency *= 2, ++inc, amp *= 0.5)
   {
      setNoiseFrequency(frequency);
      ni[0] = ni[1] = ni[2] = 0;

      inci = 1.0 / (size/frequency);
      for(i = 0; i < size; ++i, ni[0]+=inci)
      {
         incj = 1.0/(size/frequency);
         for(j = 0; j < size; ++i, ni[1] += incj)
         {
            inck = 1.0/ (size/frequency);
            for(k=0; k < size; ++k, ni[2] += inck, ptr+= 4)
            {
               noise[i][j][k] = (noise3(ni)+1.0)* amp;
            }
         }
      }
   }
}
*/
int getTotalTriangles(float triangleCounts[],int triCountSize)
{
   int totalTriangles = 0;
   for(int i=0;i<triCountSize;i++)
   {
      totalTriangles += triangleCounts[i];
   }
   return totalTriangles;
}

void getRidOfBogusTriangles(triangleVert newTris[], triangleNorm newNorms[], 
                           triangleVert rockTriangles[], triangleNorm rockNormals[],
                           float triangleCounts[], int rockTrianglesSize, int triCountSize)
{
   cout << "here"<<endl;
   int totalTriangles = 0;
   int inc = 0;
   for(int i=0;i<triCountSize;i++)
   {
      totalTriangles += triangleCounts[i];
      if(triangleCounts[i]>0)
      {
         for(int j=0;j<triangleCounts[i];j++)
         {
             newTris[inc]=rockTriangles[i*5+j];
             newNorms[inc]=rockNormals[i*5+j];
             inc++;
         }
      }
     
   }

}

void printMatrix(float x[], int dim)
{
   for(int i = 0;i<dim;i++)
      cout << "i " << x[i] << " ";
   cout << endl;
}

void printRockTriangles(triangleVert rockTriangles[], int dim)
{
   for(int i=0;i<dim; i++)
   {
      //cout << "tri"<<i<<": " <<endl;
      if(!(i%5)) cout <<endl;
      cout <<"{";
      cout <<"x "<<rockTriangles[i].rv1.x<<" ";
      cout <<"y "<<rockTriangles[i].rv1.y<<" ";
      cout <<"z "<<rockTriangles[i].rv1.z;
      
      cout<<"}{";

      cout <<"x "<<rockTriangles[i].rv2.x<<" ";
      cout <<"y "<<rockTriangles[i].rv2.y<<" ";
      cout <<"z "<<rockTriangles[i].rv2.z;

      cout<<"}{";
      
      cout <<"x "<<rockTriangles[i].rv3.x<<" ";
      cout <<"y "<<rockTriangles[i].rv3.y<<" ";
      cout <<"z "<<rockTriangles[i].rv3.z;
      cout<<"}";
      cout<<endl;
      
   }
}
void printRockNormals(triangleNorm rockTriangles[], int dim)
{
   for(int i=0;i<dim; i++)
   {
      //cout << "tri"<<i<<": " <<endl;
      if(!(i%5)) cout <<endl;
      cout <<"{";
      cout <<"x "<<rockTriangles[i].n1.x<<" ";
      cout <<"y "<<rockTriangles[i].n1.y<<" ";
      cout <<"z "<<rockTriangles[i].n1.z;
      
      cout<<"}{";

      cout <<"x "<<rockTriangles[i].n2.x<<" ";
      cout <<"y "<<rockTriangles[i].n2.y<<" ";
      cout <<"z "<<rockTriangles[i].n2.z;

      cout<<"}{";
      
      cout <<"x "<<rockTriangles[i].n3.x<<" ";
      cout <<"y "<<rockTriangles[i].n3.y<<" ";
      cout <<"z "<<rockTriangles[i].n3.z;
      cout<<"}";
      cout<<endl;
      
   }
}
void printRockTriangleVector(vector<triangle> * newTris,int totalTriangles)
{
   cout << "in func"<<endl;
   for(int i=0;i<totalTriangles; i++)
   {
      cout << "tri"<<i<<":" <<endl;
      cout <<"{";
      cout <<"x "<<(*newTris)[i].rv1.coordAmb.x<<" ";
      cout <<"y "<<(*newTris)[i].rv1.coordAmb.y<<" ";
      cout <<"z "<<(*newTris)[i].rv1.coordAmb.w<<" ";
      cout <<"a "<<(*newTris)[i].rv1.coordAmb.w<<" ";
      
      cout<<"}{";

      cout <<"x "<<(*newTris)[i].rv2.coordAmb.x<<" ";
      cout <<"y "<<(*newTris)[i].rv2.coordAmb.y<<" ";
      cout <<"z "<<(*newTris)[i].rv2.coordAmb.z<<" ";
      cout <<"z "<<(*newTris)[i].rv2.coordAmb.w<<" ";

      cout<<"}{";
      
      cout <<"x "<<(*newTris)[i].rv3.coordAmb.x<<" ";
      cout <<"y "<<(*newTris)[i].rv3.coordAmb.y<<" ";
      cout <<"z "<<(*newTris)[i].rv3.coordAmb.z<<" ";
      cout <<"a "<<(*newTris)[i].rv3.coordAmb.w;
      cout<<"}";
      cout<<endl;
      
   }

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

void printTriangleMatrix(float x[])
{
   for(int i=0; i<cubeDim*cubeDim*cubeDim;i++)
   {
      cout<< (i/cubeDim/cubeDim) << endl;
      cout<< (i/cubeDim) << endl;
       cout << "{" <<endl;
      for (int a=0;a<5;a++)
      {
        
         for(int b=0;b<3;b++){
            cout << "{";
            for(int c=0;c<4;c++){
               cout << x[5*3*4*i+3*4*a+4*b+c] << " ";
            }
            cout << "}";
         }
         cout <<endl;
      }
      cout << "}" << endl <<endl;
   }
}

void printTriangleMatrixIndeces()
{
   for(int i=0; i<cubeDim*cubeDim*cubeDim;i++)
   {
      cout<< (i/cubeDim/cubeDim) << endl;
       cout << "{" <<endl;
      for (int a=0;a<5;a++)
      {
        
         for(int b=0;b<3;b++){
            cout << "{";
            for(int c=0;c<4;c++){
               cout << 5*3*4*i+3*4*a+4*b+c << " ";
            }
            cout << "}";
         }
         cout <<endl;
      }
      cout << "}" << endl <<endl;
   }
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


const char* densitySource = ReadText("terrain_kernel.cl");



void densityCalc( 
         float h_density[], 
         float cornerPosX, float cornerPosY, float cornerPosZ)
{
   // Calculate matrix dimensions
   int n = cubeDimPlusEdge;
   int N = cubeDimPlusEdge*cubeDimPlusEdge*cubeDimPlusEdge;

   //Allocate device memory and copy A&B from host to device
   cl_int err;

   //Allocate device memory for C on device
   cl_mem d_density = clCreateBuffer(context, CL_MEM_WRITE_ONLY,N*sizeof(float),NULL,&err);
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
   if(clSetKernelArg(kernel,1,sizeof(float),&cornerPosX)) Fatal("Cannot set kernel parameter x\n");
   if(clSetKernelArg(kernel,2,sizeof(float),&cornerPosY)) Fatal("Cannot set kernel parameter y\n");
   if(clSetKernelArg(kernel,3,sizeof(float),&cornerPosZ)) Fatal("Cannot set kernel parameter z\n");
   if(clSetKernelArg(kernel,4,sizeof(int),&n)) Fatal("Cannot set kernel parameter n\n");

   //Run Kernel
   size_t Global[3] = {n,n,n};
   //size_t Local[2]  = {10,10};
   if (clEnqueueNDRangeKernel(queue,kernel,3,NULL,Global,NULL,0,NULL,NULL)) Fatal("Cannot run kernel\n");

   //  Release kernel and program
   if (clReleaseKernel(kernel)) Fatal("Cannot release kernel\n");
   if (clReleaseProgram(prog)) Fatal("Cannot release program\n");

   // Copy C from device to host (block until done)
   if (clEnqueueReadBuffer(queue,d_density,CL_TRUE,0,N*sizeof(float),h_density,0,NULL,NULL)) Fatal("Cannot copy density from device to host\n");

   //  Free device memory
   clReleaseMemObject(d_density);

}

void triangleCalc(float h_numTriangles[], float h_triangles[], triangleNorm h_rockNormals[], 
                  float h_density[], 
                  float cornerPosX, float cornerPosY, float cornerPosZ, 
                  float h_debug[], triangleVert h_rockTriangles[], int rockTrisSize)
{
   // Calculate matrix dimensions
   int n = cubeDimPlusEdge;
   int N = cubeDimPlusEdge*cubeDimPlusEdge*cubeDimPlusEdge*sizeof(float);
   //int NTRI3 = cubeDim*cubeDim*cubeDim*5*3*3*sizeof(float);
   int NTRI4 = cubeDim*cubeDim*cubeDim*5*4*3*sizeof(float);
   int nvox = cubeDim;
   int debugDim = cubeDim*cubeDim*cubeDim*8*sizeof(float);


   //Allocate device memory and copy A&B from host to device
   cl_int err;
   cl_mem d_density = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,N,h_density,&err);
   if(err) Fatal("Cannot create and copy density from host to device\n");

   //Allocate device memory for C on device
   cl_mem d_numTriangles = clCreateBuffer(context, CL_MEM_WRITE_ONLY,N,NULL,&err);
   if(err) Fatal("Cannot create numTriangles on device, sad day indeed\n");
   cl_mem d_triangles = clCreateBuffer(context, CL_MEM_WRITE_ONLY,NTRI4,NULL,&err);
   if(err) Fatal("Cannot create triangles on device, sad day indeed\n");
   cl_mem d_rockNormals = clCreateBuffer(context, CL_MEM_WRITE_ONLY,rockTrisSize,NULL,&err);
   if(err) Fatal("Cannot create normals on device, sad day indeed\n");
   cl_mem d_debug = clCreateBuffer(context, CL_MEM_WRITE_ONLY,debugDim,NULL,&err);
   if(err) Fatal("Cannot create normals on device, sad day indeed\n");
   cl_mem d_rockTriangles = clCreateBuffer(context, CL_MEM_WRITE_ONLY,rockTrisSize,NULL,&err);
   if(err) Fatal("Cannot create rockTriangles on device, sad day indeed\n");
//cout << "debug dim " << debugDim << endl;
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
   cl_kernel kernel = clCreateKernel(prog,"triangles",&err);
   if(err) Fatal("Cannot create kernel\n");

   //Set parameters for kernel
   if(clSetKernelArg(kernel,0,sizeof(cl_mem),&d_density)) Fatal("Cannot set kernel parameter d_density\n");
   if(clSetKernelArg(kernel,1,sizeof(float),&cornerPosX)) Fatal("Cannot set kernel parameter x\n");
   if(clSetKernelArg(kernel,2,sizeof(float),&cornerPosY)) Fatal("Cannot set kernel parameter y\n");
   if(clSetKernelArg(kernel,3,sizeof(float),&cornerPosZ)) Fatal("Cannot set kernel parameter z\n");
   if(clSetKernelArg(kernel,4,sizeof(int),&n)) Fatal("Cannot set kernel parameter n\n");
   if(clSetKernelArg(kernel,5,sizeof(cl_mem),&d_numTriangles)) Fatal("Cannot set kernel parameter d_numTriangles\n");
   if(clSetKernelArg(kernel,6,sizeof(cl_mem),&d_triangles)) Fatal("Cannot set kernel parameter d_triangles\n");
   if(clSetKernelArg(kernel,7,sizeof(cl_mem),&d_rockNormals)) Fatal("Cannot set kernel parameter d_normals\n");
   if(clSetKernelArg(kernel,8,sizeof(int),&nvox)) Fatal("Cannot set kernel parameter nvox\n");
   if(clSetKernelArg(kernel,9,sizeof(cl_mem),&d_debug)) Fatal("Cannot set debug parameter debug\n");
   if(clSetKernelArg(kernel,10,sizeof(cl_mem),&d_rockTriangles)) Fatal("Cannot set debug parameter debug\n");

   //Run Kernel
   size_t Global[3] = {cubeDim,cubeDim,cubeDim};
   //size_t Local[2]  = {10,10};
   if (clEnqueueNDRangeKernel(queue,kernel,3,NULL,Global,NULL,0,NULL,NULL)) Fatal("Cannot run kernel\n");

   //  Release kernel and program
   if (clReleaseKernel(kernel)) Fatal("Cannot release kernel\n");
   if (clReleaseProgram(prog)) Fatal("Cannot release program\n");

   // Copy C from device to host (block until done)
   if (clEnqueueReadBuffer(queue,d_numTriangles,CL_TRUE,0,N,h_numTriangles,0,NULL,NULL)) Fatal("Cannot copy numTriangles from device to host\n");
   if (clEnqueueReadBuffer(queue,d_triangles,CL_TRUE,0,NTRI4,h_triangles,0,NULL,NULL)) Fatal("Cannot copy triangles from device to host\n");
   if (clEnqueueReadBuffer(queue,d_rockNormals,CL_TRUE,0,rockTrisSize,h_rockNormals,0,NULL,NULL)) Fatal("Cannot copy normals from device to host\n");
   if (clEnqueueReadBuffer(queue,d_debug,CL_TRUE,0,debugDim,h_debug,0,NULL,NULL)) Fatal("Cannot copy debug from device to host\n");
   if (clEnqueueReadBuffer(queue,d_rockTriangles,CL_TRUE,0,rockTrisSize,h_rockTriangles,0,NULL,NULL)) Fatal("Cannot copy test from device to host\n");
   //  Free device memory
   clReleaseMemObject(d_density);
   clReleaseMemObject(d_triangles);
   clReleaseMemObject(d_numTriangles);
   clReleaseMemObject(d_rockNormals);
   clReleaseMemObject(d_debug);
   clReleaseMemObject(d_rockTriangles);
}


/*
 *  Draw a cube
 */
static void Cube(void)
{
   //  Front
   glColor3f(1,0,0);
   glBegin(GL_QUADS);
   glNormal3f( 0, 0,+1);
   glTexCoord2f(0,0); glVertex3f(-1,-1,+1);
   glTexCoord2f(1,0); glVertex3f(+1,-1,+1);
   glTexCoord2f(1,1); glVertex3f(+1,+1,+1);
   glTexCoord2f(0,1); glVertex3f(-1,+1,+1);
   glEnd();
   //  Back
   glColor3f(0,0,1);
   glBegin(GL_QUADS);
   glNormal3f( 0, 0,-1);
   glTexCoord2f(0,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,-1);
   glEnd();
   //  Right
   glColor3f(1,1,0);
   glBegin(GL_QUADS);
   glNormal3f(+1, 0, 0);
   glTexCoord2f(0,0); glVertex3f(+1,-1,+1);
   glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,+1);
   glEnd();
   //  Left
   glColor3f(0,1,0);
   glBegin(GL_QUADS);
   glNormal3f(-1, 0, 0);
   glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(-1,-1,+1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,+1);
   glTexCoord2f(0,1); glVertex3f(-1,+1,-1);
   glEnd();
   //  Top
   glColor3f(0,1,1);
   glBegin(GL_QUADS);
   glNormal3f( 0,+1, 0);
   glTexCoord2f(0,0); glVertex3f(-1,+1,+1);
   glTexCoord2f(1,0); glVertex3f(+1,+1,+1);
   glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(-1,+1,-1);
   glEnd();
   //  Bottom
   glColor3f(1,0,1);
   glBegin(GL_QUADS);
   glNormal3f( 0,-1, 0);
   glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(+1,-1,+1);
   glTexCoord2f(0,1); glVertex3f(-1,-1,+1);
   glEnd();
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   glClearColor(0.0,0.1,1.0,0.0);
   const double len=2.0;  //  Length of axes
   //  Light position and colors

   float Ambient[]   = {0.3,0.3,0.3,1.0};
   float Diffuse[]   = {1.0,1.0,1.0,1.0};
   float Specular[]  = {1.0,1.0,1.0,1.0};
   float Position[]  = {2,Ylight,2};
   
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);

   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective - set eye position
   if (proj)
   {
      double Ex = -2*dim*Sin(th)*Cos(ph);
      double Ey = +2*dim        *Sin(ph);
      double Ez = +2*dim*Cos(th)*Cos(ph);
      gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
   }
   //  Orthogonal - set world orientation
   else
   {
      glRotatef(ph,1,0,0);
      glRotatef(th,0,1,0);
   }
//  Draw light position as sphere (still no lighting here)
   glColor3f(1,1,1);
   glPushMatrix();
   glTranslated(Position[0],Position[1],Position[2]);
   glutSolidSphere(0.03,10,10);
   glPopMatrix();
   //  OpenGL should normalize normal vectors
   glEnable(GL_NORMALIZE);
   //  Enable lighting
   glEnable(GL_LIGHTING);
   //  glColor sets ambient and diffuse color materials
   glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
   glEnable(GL_COLOR_MATERIAL);
   //  Enable light 0
   glEnable(GL_LIGHT0);
   //  Set ambient, diffuse, specular components and position of light 0
   glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
   glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
   glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
   glLightfv(GL_LIGHT0,GL_POSITION,Position);
   //  Select shader (0 => no shader)
   glUseProgram(shader[mode]);

   //  Export time to uniform variable
   if (mode)
   {
      float time = 0.001*glutGet(GLUT_ELAPSED_TIME);
      int id = glGetUniformLocation(shader[mode],"time");
      if (id>=0) glUniform1f(id,time);
   }

  /* float testverts[]=
   {
      0,0,0,10,
      1,0,0,10,
      1,1,0,10,

      2,2,0,10,
      1,2,0,10,
      1,1,0,10,

      10,10,10,10,
      10,10,10,10,
      10,10,10,10
   };

   float testverts2[]=
   {
      0, 0, -0.5, 0, 0, 0.5, -0.5, 0,0, 0.5, 0, 0,
0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01,
0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01,
0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01,
0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01,

0, 0.5, -0.5, 0,0.5, 0.5, -0.5, 0,0, 0, -0.5, 0,
0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01,
0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01,
0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01,
0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01,0.01, 0.01, 0.01, 0.01

   };
   float4 testverts3[] = 
   {
      {0,0,0,10},
      {1,0,0,10},
      {1,1,0,10},

      {2,2,0,10},
      {1,2,0,10},
      {1,1,0,10}

   };

   triangle testverts4[] = 
   {
      {
      {{0,0,0,10}, {1,1,1,0}},
      {{1,0,0,10}, {1,1,1,0}},
      {{1,1,0,10}, {1,1,1,0}}
      }
      ,
      {
      {{2,2,0,10}, {1,1,1,0}},
      {{1,2,0,10}, {1,1,1,0}},
      {{1,1,0,10}, {1,1,1,0}}
      }
   };

triangle3 testverts5[] = 
   {
      {
      {{0,0,0}, {1,1,1}},
      {{1,0,0}, {1,1,1}},
      {{1,1,0}, {1,1,1}}
      }
      ,
      {
      {{2,2,0}, {1,1,1}},
      {{1,2,0}, {1,1,1}},
      {{1,1,0}, {1,1,1}}
      }
   };

float3 testverts6[] = 
   {
      {0,0,0},
      {1,0,0}, 
      {1,1,0},

      {2,2,0}, 
      {1,2,0}, 
      {1,1,0},

      {10,10,100},
      {10,9,8},
      {0,10,10}
      
   };

triangleVert testverts7[] = 
   {
      {{0,0,0},
      {1,0,0}, 
      {1,1,0}},

      {{2,2,0}, 
      {1,2,0}, 
      {1,1,0}},

      {{10,10,100},
      {10,9,8},
      {0,10,10}}
      
   };*/

   //  Draw the model, teapot or cube
   glColor3f(1,1,0);
   if (obj==1)
   {
      glEnableClientState(GL_VERTEX_ARRAY);
      glEnableClientState(GL_NORMAL_ARRAY);

      glNormalPointer(GL_FLOAT,0,allPossibleTriangleNorms);
      glVertexPointer(3,GL_FLOAT,0,allPossibleTriangleVerts);
      glDrawArrays(GL_TRIANGLES,0,totalNumberTriangles*3);

     //for(int i=0;i<numBlocks;i++){

      //Using the arrays of pointers
     // glNormalPointer(GL_FLOAT,0,normalPtrArray[i]);
    //  glVertexPointer(3,GL_FLOAT,0,trianglePtrArray[i]);
      //glDrawArrays(GL_TRIANGLES,0,numTrianglesPtrArray[i]*3);
      


      //tests
      //glVertexPointer(3,GL_FLOAT,0,testverts7);
      //glDrawArrays(GL_TRIANGLES,0,18);
 
     // }
      //glDrawElements(GL_TRIANGLES,3*3*5*cubeDimPlusEdge*cubeDimPlusEdge*cubeDimPlusEdge, GL_UNSIGNED_BYTE, &verteces);
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
   }
   else
      Cube();

   //  No shader for what follows
   glUseProgram(0);
glDisable(GL_LIGHTING); 
   //  Draw axes - no lighting from here on
   glColor3f(1,1,1);
   if (axes)
   {
      glBegin(GL_LINES);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(len,0.0,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,len,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,0.0,len);
      glEnd();
      //  Label axes
      glRasterPos3d(len,0.0,0.0);
      Print("X");
      glRasterPos3d(0.0,len,0.0);
      Print("Y");
      glRasterPos3d(0.0,0.0,len);
      Print("Z");
   }

   //  Display parameters
   glWindowPos2i(5,5);
   Print("Angle=%d,%d  Dim=%.1f Projection=%s %s",
     th,ph,dim,proj?"Perpective":"Orthogonal",text[mode]);

   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int xch,int ych)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;
   //  PageUp key - increase dim
   else if (key == GLUT_KEY_PAGE_DOWN)
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_UP && dim>1)
      dim -= 0.1;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Update projection
   Project(proj?fov:0,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int xch,int ych)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   //  Toggle axes
   else if (ch == 'a' || ch == 'A')
      axes = 1-axes;
   //  Toggle projection type
   else if (ch == 'p' || ch == 'P')
      proj = 1-proj;
   //  Toggle objects
   else if (ch == 'o' || ch == 'O')
      obj = (obj+1)%2;
   //  Cycle modes
   else if (ch == 'm' || ch == 'M')
      mode = (mode+1)%3;
   //  Reproject
   Project(proj?fov:0,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(proj?fov:0,asp,dim);
}

/*
 *  GLUT calls this routine every 50ms
 */
void timer(int k)
{
   glutPostRedisplay();
   glutTimerFunc(50,timer,0);
}

void init()
{
  /* for(int i=0;i<xVoxelWidth;i++)
   {
      for(int j=0;j<yVoxelWidth;j++)
      {
         for(int k=0;k<zVoxelWidth;k++)
         {
            cout << i <<endl;
            voxelGroup[i][j][k].position.x = i-xVoxelWidth/2;
            voxelGroup[i][j][k].position.y = j-yVoxelWidth/2;
            voxelGroup[i][j][k].position.z = k-zVoxelWidth/2;
            voxelGroup[i][j][k].hasBeenGenerated=false;
            voxelGroup[i][j][k].isEmpty = false;
         }
      }
   }*/
}
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
   
  // init();
   cout <<"after init"<<endl;
   //Allocate host memory for density calcs
   int n = cubeDimPlusEdge;
   int N = n*n*n;
   int rockTrisSize = cubeDim*cubeDim*cubeDim*5*3*3*sizeof(float);
   int numTriangles;
   int isEmpty=false;
   
   // for(int i=0;i<16;i++)
   // {
   //    for(int j=0;j<16;j++)
   //    {
   //       for(int k=0; j<16;k++)
   //       {
   //          noise
   //       }
   //    }
   // }

   float* h_density = (float*)malloc(N*sizeof(float));
   float* h_triangles = (float*)malloc(4*3*5*cubeDim*cubeDim*cubeDim*sizeof(float));
   //float* h_normals = (float*)malloc(3*3*5*cubeDim*cubeDim*cubeDim*sizeof(float));
   float* h_numTriangles = (float*)malloc(N*sizeof(float));
   float* h_debug = (float*)malloc(N*sizeof(float)*8);
   //The rockVertex struct has 8 floats. Each triangle has 3 rockVerteces
   triangleVert* h_rockTriangles = (triangleVert*)malloc(rockTrisSize);
   triangleNorm* h_rockNormals = (triangleNorm*)malloc(rockTrisSize);
   if (!h_density || !h_triangles || !h_rockNormals || 
      !h_numTriangles || !h_debug || !h_rockTriangles) Fatal("Cannot allocate host memory\n");



// typedef struct
// {
//    float3 position;
//    bool hasBeenGenerated;
//    bool isEmpty;

   // int numTriangles;
   // triangleVert realTriangles[];
   // triangleNorm realNormals[];

// } block;

   for(int i=0;i<xVoxelWidth;i++)
   {
      for(int j=0;j<yVoxelWidth;j++)
      {
         for(int k=0;k<zVoxelWidth;k++)
         {
               //Perform density calculation
            densityCalc(h_density,i,j,k);

            //Print out the density
            printMatrixInSmallestIncs(h_density, n*n*n, n);

            //Perform triangle calculation with the density cubes
            triangleCalc(h_numTriangles, h_triangles, h_rockNormals, 
                           h_density, 
                           i, j, k, 
                           h_debug, h_rockTriangles, rockTrisSize);

            //Perform the 
            //verteces = h_triangles;
           // printTriangleMatrix(h_triangles);
           //cout <<endl;
            //printMatrixInSmallestIncs(h_numTriangles, cubeDim*cubeDim*cubeDim, cubeDim);
            //cout <<endl;
            //printTriangleMatrixIndeces();
            //printMatrixInSmallestIncs(h_debug, 8*cubeDim*cubeDim*cubeDim, 8);

          //  printRockTriangles(h_rockTriangles,5*cubeDim*cubeDim*cubeDim);
           // cout << "here"<<endl;
            numTriangles = getTotalTriangles(h_numTriangles,cubeDim*cubeDim*cubeDim);
            totalNumberTriangles+=numTriangles;

            triangleVert realTriangles[numTriangles];
          
            triangleNorm realNormals[numTriangles];
          
            //cout << "here"<<endl;

            getRidOfBogusTriangles(realTriangles, realNormals, 
                                 h_rockTriangles, h_rockNormals, 
                                 h_numTriangles, 5*cubeDim*cubeDim*cubeDim, cubeDim*cubeDim*cubeDim);
            //cout << "total Triangles " << numTriangles<< endl;
            if(!numTriangles) isEmpty = true;
            //allTriangles.insert(allTriangles.end(),numTriangles,&realTriangles);
            //allNormals.insert(allNormals.end(),numTriangles,&realNormals);
            //cout << "realTris in main " << realTriangles[0].rv1.x<<endl;
            //printRockTriangleVector(&realTriangles,numTriangles);
            //printRockTriangles(voxelGroup[i][j][k].realTriangles,voxelGroup[i][j][k].numTriangles);
            //cout << endl << endl;
            //printRockNormals(voxelGroup[i][j][k].realNormals,voxelGroup[i][j][k].numTriangles);
            //allTriangles.push_back(realTriangles);
            trianglePtrArray[STUPID] = realTriangles;
            normalPtrArray[STUPID] = realNormals;
            numTrianglesPtrArray[STUPID] = numTriangles;
            STUPID++;
            for(int i=0;i<numTriangles;i++)
            {
               allPossibleTriangleVerts[totalNumberTriangles+ i] = realTriangles[i];
               allPossibleTriangleNorms[totalNumberTriangles+ i] = realNormals[i];
            }
            totalNumberTriangles+=numTriangles;

           /* float3 position = {i,j,k};
            block temp = new block(position, true, isEmpty, numTriangles, realTriangles, realNormals);
            voxelGroup[STUPID] = &temp;
            STUPID++;*/
            //voxelGroup.push_back(temp);

         }
      }
   }

  //trianglePtr = voxelGroup[0][0][0].realTriangles;
   //normalPtr = voxelGroup[0][0][0].realNormals;


   //cout << h_test[0].x << h_test[0].y << h_test[0].z << h_test[0].w;
///DRAWING///////////////////////////////////////////////////////////
  //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(600,600);
   glutInitWindowPosition(1000,100);
   glutCreateWindow("Simple Shader");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
   if (!GLEW_VERSION_2_0) Fatal("OpenGL 2.0 not supported\n");
#endif
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   timer(1);

   //  Create Shader Programs
   shader[1] = CreateShaderProg("simple.vert","simple.frag");
   shader[2] = CreateShaderProg("pixelphong.vert","pixelphong.frag");
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
 
   free(h_density);
   free(h_triangles);
   free(h_rockNormals);
   free(h_numTriangles);
   free(h_rockTriangles);

   //  Done
   return 0;
}
