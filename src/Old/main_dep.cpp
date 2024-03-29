/*
 *  Nbody Simulator
 *  This program requires OpenGL 3.2 or above
 *
 *  'a' to toggle axes
 *  '0' snaps angles to 0,0
 *  arrows to rotate the world
 */
 
#include "CSCIx239.h"
#include "Block.h"
#include "Cell.h"

using namespace std;

int N=1024;       //  Number of bodies
int src=0;        //  Offset of first star in source
int dst=0;        //  Offset of first star in destination
int axes=0;       //  Display axes
int th=0;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
double dim=10;    //  Size of universe
double vel=0.1;   //  Relative speed
int mode=0;       //  Solver mode
int shader=0;     //  Shader
char* text[] = {"Sequential","OpenMP","OpenMP+Geometry Shader"};



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
   CreateShader(prog,GL_GEOMETRY_SHADER,"nbody.geom");
#endif
   CreateShader(prog,GL_FRAGMENT_SHADER,"nbody.frag");
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
   //  Shader program
  // shader = CreateShaderProgGeom();
   //ErrCheck("init");
   //  Star texture
   LoadTexBMP("star.bmp");
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
