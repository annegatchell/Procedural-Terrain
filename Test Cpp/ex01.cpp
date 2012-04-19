/*
 *  Objects in C++
 *  Uses the STL vector class to display an array of generic objects
 */
#include "CSCIx239.h"
#include "Object.h"
#include "Cube.h"
#include "Sphere.h"
#include "WaveOBJ.h"
#include <vector>

using namespace std;

int th=0;             //  Azimuth of view angle
int ph=0;             //  Elevation of view angle
const double dim=3;   //  Size of world
vector<Object*> obj;  //  Array of objects

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);

   //  Undo previous transformations
   glLoadIdentity();
   //  Set eye position
   glRotatef(ph,1,0,0);
   glRotatef(th,0,1,0);

   //  Draw all objects
   for (unsigned int k=0;k<obj.size();k++)
      obj[k]->display();

   //  Display parameters
   glColor3f(1,1,1);
   glWindowPos2i(5,5);
   Print("Angle=%d,%d",th,ph);

   //  Draw axes
   glBegin(GL_LINES);
   glVertex3d(0,0,0);
   glVertex3d(2,0,0);
   glVertex3d(0,0,0);
   glVertex3d(0,2,0);
   glVertex3d(0,0,0);
   glVertex3d(0,0,2);
   glEnd();
   //  Label axes
   glRasterPos3d(2,0,0);
   Print("X");
   glRasterPos3d(0,2,0);
   Print("Y");
   glRasterPos3d(0,0,2);
   Print("Z");

   //  Render the scene and make it visible
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
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   double asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set orthogonal projection
   Project(0,asp,dim);
}

/*
 *  Initilize objects
 */
void Init()
{
   obj.push_back(new Cube(1,0,0 , 0.3,0.3,0.3));
   obj.push_back(new Cube(0,1,0 , 0.3,0.3,0.3));
   obj.push_back(new Sphere(1,1,0 , 0.3 , 1,0,0));
   obj.push_back(new Sphere(0,0,1 , 0.3 , 0,1,0));
   obj.push_back(new WaveOBJ("bunny.obj"));
}

/*
 *  Set object states
 */
void Set(float zh)
{
   //  Spin first cube CW
   ((Cube*)obj[0])->rotate(zh);
   //  Spin second cube CCW
   ((Cube*)obj[1])->rotate(-zh);
   //  Oscillate red sphere along Z axis
   ((Sphere*)obj[2])->translate(1,1,Sin(zh));
   //  Change radius of green sphere
   ((Sphere*)obj[3])->radius(0.3+0.1*Sin(zh));
   //  Small brown bunny
   ((WaveOBJ*)obj[4])->color(0.6,0.4,0.2);
   ((WaveOBJ*)obj[4])->scale(0.2);
   ((WaveOBJ*)obj[4])->rotate(zh,1,1,1);
}

/*
 *  GLUT calls this routine when the window is resized
 */
void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   //  Translate time into 90 degree/sec angle
   Set(fmod(90*t,360.0));
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(400,400);
   glutCreateWindow("Objects in C++");
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);
   //  Initialze objects
   Init();
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
