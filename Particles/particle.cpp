/*
 *  Nbody Simulator
 *  This program requires OpenGL 3.2 or above
 *
 *  'a' to toggle axes
 *  '0' snaps angles to 0,0
 *  arrows to rotate the world
 */
#include "CSCIx239.h"
 #include "shader.c"

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

int n;       //  Number of bodies
int N;
int src=0;        //  Offset of first star in source
int dst=0;        //  Offset of first star in destination
int axes=0;       //  Display axes
int th=0;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
double dim=10;    //  Size of universe
double vel=0.1;   //  Relative speed
int currentStarSet = 0;

cl_mem d_StarsOld1, d_StarsOld2, d_StarsNew;
cl_program prog;
cl_int  err;
cl_kernel kernel, initKernel;

int mode=0;       //  Solver mode
int shader=0;     //  Shader
char* text[] = {"Sequential","OpenMP","OpenMP+Geometry Shader"};

//  Star
typedef struct
{
   float x,y,z;  //  Position
   float u,v,w;  //  Velocity
   float r,g,b;  //  Color
}  Star;
Star* stars=NULL;
Star* stars2=NULL;
Star* starsDisplay=NULL;


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

   //  Create OpenCL context for fastest device
   context = clCreateContext(0,1,&devid,Notify,NULL,&err);
   if(!context || err) Fatal("Cannot create OpenCL context\n");

   //  Create OpenCL command queue for fastest device
   queue = clCreateCommandQueue(context,devid,0,&err);
   if(!queue || err) Fatal("Cannot create OpenCL command cue\n");

   return mwgs;
} 


const char* starSource = ReadText("starMove.cl");


void InitStepOnDevice(Star h_StarsNew[],Star h_StarsOld1[],Star h_StarsOld2[], const unsigned int Bw,const unsigned int Bn)
{

   //  Calculate matrix dimensions

   // Allocate device memory and copy StarsOld1 from host to device
   d_StarsOld1 = clCreateBuffer(context,CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,N,h_StarsOld1,&err);
   if (err) Fatal("Cannot create and copy StarsOld1 from host to device\n");

   //Allocate device memory for StarsOld2 on device
   d_StarsOld2 = clCreateBuffer(context,CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,N,h_StarsOld2,&err);
   if (err) Fatal("Cannot create and copy StarsOld2 from host to device\n");

   //  Allocate device memory for StarsNew on device
   d_StarsNew = clCreateBuffer(context,CL_MEM_WRITE_ONLY,N,NULL,&err);
   if (err) Fatal("Cannot create C on device\n");


   //  Compile kernel
   prog = clCreateProgramWithSource(context,1,&starSource,0,&err);
   if (err) Fatal("Cannot create program\n");
   if (clBuildProgram(prog,0,NULL,NULL,NULL,NULL))
   {
      char log[1048576];
      if (clGetProgramBuildInfo(prog,devid,CL_PROGRAM_BUILD_LOG,sizeof(log),log,NULL))
         Fatal("Cannot get build log\n");
      else
         Fatal("Cannot build program\n%s\n",log);
   }

   initKernel = clCreateKernel(prog, "initStar", &err);
   if (err) Fatal("Cannot create initKernel\n");

   kernel = clCreateKernel(prog,"starStep",&err);
   if (err) Fatal("Cannot create kernel\n");

   //  Set parameters for kernel

   if (clSetKernelArg(initKernel,0,sizeof(cl_mem),&d_StarsOld1)) Fatal("Cannot set kernel parameter StarsOld1\n");
   if (clSetKernelArg(initKernel,1,sizeof(cl_mem),&d_StarsOld2)) Fatal("Cannot set kernel parameter StarsOld2\n");
   if (clSetKernelArg(initKernel,2,sizeof(int),&n)) Fatal("Cannot set kernel parameter n\n");
 
   if (clSetKernelArg(kernel,0,sizeof(cl_mem),&d_StarsNew)) Fatal("Cannot set kernel parameter StarsNew\n");
   if (clSetKernelArg(kernel,1,sizeof(int),&n)) Fatal("Cannot set kernel parameter n\n");


   size_t Global[2] = {n,n};
   size_t Local[2]  = {Bw,Bw};
   if (clEnqueueNDRangeKernel(queue,initKernel,2,NULL,Global,Local,0,NULL,NULL)) Fatal("Cannot run kernel\n");
  // return true;
}

void StepOnDevice(Star h_StarsNew[], const unsigned int Bw,const unsigned int Bn)
{
 //  Run kernel
   size_t Global[2] = {n,n};
   size_t Local[2]  = {Bw,Bw};
   if (clEnqueueNDRangeKernel(queue,kernel,2,NULL,Global,Local,0,NULL,NULL)) Fatal("Cannot run kernel\n");



   // Copy C from device to host (block until done)
   if (clEnqueueReadBuffer(queue,d_StarsNew,CL_TRUE,0,N,h_StarsNew,0,NULL,NULL)) Fatal("Cannot copy StarsNew from device to host\n");
}

void closeStepOnDevice()
{

      //  Release kernel and program
   if (clReleaseKernel(kernel)) Fatal("Cannot release kernel\n");
   if (clReleaseKernel(initKernel)) Fatal("Cannot release kernel\n");
   if (clReleaseProgram(prog)) Fatal("Cannot release program\n");
      //  Free device memory
   clReleaseMemObject(d_StarsOld1);
   clReleaseMemObject(d_StarsOld2);
   clReleaseMemObject(d_StarsNew);
}



/*
 *  Advance time one time step for star k
 */
void Move(int k)
{

   int k0 = k+src;
   int k1 = k+dst;
   float dt = 1e-3;
   int i;
   //  Calculate force components
   double a=0;
   double b=0;
   double c=0;
   for (i=src;i<src+n;i++)
   {
      double dx = stars[i].x-stars[k0].x;
      double dy = stars[i].y-stars[k0].y;
      double dz = stars[i].z-stars[k0].z;
      double d = sqrt(dx*dx+dy*dy+dz*dz)+1;  //  Add 1 to d to dampen movement
      double f = 1/(d*d*d); // Normalize and scale to 1/r^2
      a += f*dx;
      b += f*dy;
      c += f*dz;
   }
   //  Update velocity
   stars[k1].u = stars[k0].u + dt*a;
   stars[k1].v = stars[k0].v + dt*b;
   stars[k1].w = stars[k0].w + dt*c;
   //  Update position
   stars[k1].x = stars[k0].x + dt*stars[k1].u;
   stars[k1].y = stars[k0].y + dt*stars[k1].v;
   stars[k1].z = stars[k0].z + dt*stars[k1].w;
}

/*
 *  Advance time one time step
 */
void Step()
{
   int k;
   //  Switch source and destination
   src = src?0:n;
   dst = dst?0:n;
   //  OpenMP
   if (mode)
      #pragma omp parallel for
      for (k=0;k<n;k++)
         Move(k);
   //  Sequential
   else
      for (k=0;k<n;k++)
         Move(k);
}

/*
 *  Scaled random value
 */
void rand3(float Sx,float Sy,float Sz,float* X,float* Y,float* Z)
{
   float x = 0;
   float y = 0;
   float z = 0;
   float d = 2;
   while (d>1)
   {
      x = rand()/(0.5*RAND_MAX)-1;
      y = rand()/(0.5*RAND_MAX)-1;
      z = rand()/(0.5*RAND_MAX)-1;
      d = x*x+y*y+z*z;
   }
   *X = Sx*x;
   *Y = Sy*y;
   *Z = Sz*z;
}

/*
 *  Initialize Nbody problem
 */
void InitLoc()
{
   int k;
   //  Allocate room for twice as many bodies to facilitate ping-pong
   if (!stars) stars = (Star*)malloc(n*sizeof(Star));
   if (!stars) Fatal("Error allocating memory for %d stars in stars\n",n);

   if (!stars) stars2 = (Star*)malloc(n*sizeof(Star));
   if (!stars) Fatal("Error allocating memory for %d stars in stars2\n",n);

   if (!stars) starsDisplay = (Star*)malloc(n*sizeof(Star));
   if (!stars) Fatal("Error allocating memory for %d starsDisplay\n",n);

   src = n;
   dst = 0;
   //  Assign random locations
   for (k=0;k<n;k++)
   {
      rand3(dim/2,dim/2,dim/3,&stars[k].x,&stars[k].y,&stars[k].z);
      rand3(vel,vel,vel,&stars[k].u,&stars[k].v,&stars[k].w);
      switch (k%3)
      {
         case 0:
           stars[k].r = 1.0;
           stars[k].g = 1.0;
           stars[k].b = 1.0;
           break;
         case 1:
           stars[k].r = 1.0;
           stars[k].g = 0.9;
           stars[k].b = 0.5;
           break;
         case 2:
           stars[k].r = 0.5;
           stars[k].g = 0.9;
           stars[k].b = 1.0;
           break;
      }
      stars[k+n].r = stars[k].r;
      stars[k+n].g = stars[k].g;
      stars[k+n].b = stars[k].b;
   }
   for(k=0; k<n;k++)
   {
      stars2[k] = stars[k];
   }
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   const double len=2.5;  //  Length of axes
   double Ex = -2*dim*Sin(th)*Cos(ph);
   double Ey = +2*dim        *Sin(ph);
   double Ez = +2*dim*Cos(th)*Cos(ph);

   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT);
   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective - set eye position
   gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);

   //  Integrate
   Step();

   //  Set shader
   if (mode==2)
   {
      glUseProgram(shader);
      int id = glGetUniformLocation(shader,"star");
      if (id>=0) glUniform1i(id,0);
      glBlendFunc(GL_ONE,GL_ONE);
      glEnable(GL_BLEND);
   }

   //  Draw stars using vertex arrays
   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_COLOR_ARRAY);
   glVertexPointer(3,GL_FLOAT,sizeof(Star),&stars[0].x);
   glColorPointer(3,GL_FLOAT,sizeof(Star),&stars[0].r);
   //  Draw all stars from dst count n
   glDrawArrays(GL_POINTS,dst,n);
   //  Disable vertex arrays
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);

   //  Unset shader
   if (mode==2)
   {
      glUseProgram(0);
      glDisable(GL_BLEND);
   }

   //  Draw axes
   glDisable(GL_LIGHTING);
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
   Print("FPS=%d Angle=%d,%d Mode=%s",
      FramesPerSecond(),th,ph,text[mode]);
   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
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
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Cycle modes
   else if (ch == 'm')
      mode = (mode+1)%3;
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   //  Reset simulation
   else if (ch == 'r')
      InitLoc();
   //  Toggle axes
   else if (ch == 'a' || ch == 'A')
      axes = 1-axes;
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   int fov=55;       //  Field of view (for perspective)
   //  Ratio of the width to the height of the window
   double asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(fov,asp,dim);
}

/*
 *  GLUT calls this routine when the window is resized
 */
void idle()
{
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

//
//  Create Shader Program including Geometry Shader
//
int CreateShaderProgGeom()
{
   //  Create program
   int prog = glCreateProgram();
   //  Compile and add shaders
   CreateShader(prog,GL_VERTEX_SHADER  ,"nbody.vert");
#ifdef __APPLE__
   //  OpenGL 3.1 for OSX
   CreateShader(prog,GL_GEOMETRY_SHADER_EXT,"nbody.geom_ext");
   glProgramParameteriEXT(prog,GL_GEOMETRY_INPUT_TYPE_EXT  ,GL_POINTS);
   glProgramParameteriEXT(prog,GL_GEOMETRY_OUTPUT_TYPE_EXT ,GL_TRIANGLE_STRIP);
   glProgramParameteriEXT(prog,GL_GEOMETRY_VERTICES_OUT_EXT,4);
#else
   //  OpenGL 3.2 adds layout ()
   CreateShader(prog,GL_GEOMETRY_SHADER_ARB,"nbody.geom");
#endif
   CreateShader(prog,GL_FRAGMENT_SHADER_ARB,"nbody.frag");
   //  Link program
   glLinkProgram(prog);
   //  Check for errors
   PrintProgramLog(prog);
   //  Return name
   return prog;
}



/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
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
   n = Bw*Bn;
   N = 2*n*sizeof(Star);
   printf("Bw=%d Bn=%d n=%d\n",Bw,Bn,n);

      //  Initialize GPU
   int Mw = InitGPU(verbose);
   cout << "Max Work group size is " << Mw << endl;
   if (Mw<Bw*Bw) Fatal("Thread count %d exceeds max work group size of %d\n",Bw*Bw,Mw);


   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
   glutInitWindowSize(600,600);
   glutCreateWindow("Nbody Simulator");
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);

   //  Initialize stars
   InitLoc();

   //Intialize device
   InitStepOnDevice(starsDisplay, stars, stars2, Bw,Bn);
   //  Shader program
   shader = CreateShaderProgGeom();
   ErrCheck("init");
   //  Star texture
   LoadTexBMP("star.bmp");

   StepOnDevice(starsDisplay, Bw, Bn);

   closeStepOnDevice();

   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
