/*
 *  Earth in Stored Textures
 *
 *  Based on the examples in Orange Book Chapter 10
 *  The textures are NASA's Blue Marble Next Generation Plain
 *
 *  'm' to switch mode (season/diurnal/clouds)
 *  's' to start/stop light
 *  [/] to move light
 *  +/- to change day
 *  '0' snaps angles to 0,0
 *  arrows to rotate the world
 *  PgUp/PgDn zooms in/out
 */
#include "CSCIx239.h"
int mode=0;           //  Mode
int move=1;           //  Move light
int th=0;             //  Azimuth of view angle
int ph=0;             //  Elevation of view angle
int zh=0;             //  Light azimuth
double asp=1;         //  Aspect ratio
double dim=3.0;       //  Size of world
int shader;           //  Shader programs
int day[12];          //  Day textures
int night;            //  Night texture
int cloudgloss;       //  Cloudgloss texture
GLUquadricObj* ball;  //  Sphere
const int DT=365*360;
#define MODE 3
const char* text[] = {"Seasonal","Diurnal","Clouds"};

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   int ndm[] = {31,28,31,30,31,30,31,31,30,31,30,31};
   int doy = zh/360;
   int mo,dy,hr,mn;
   int id;
   //  Sun angle
   float fh = doy*360.0/365.0;
   //  Light direction
   float Position[]  = {Cos(fh),0.0,Sin(fh),0.0};

   //  Time of day
   id = (zh+(int)fh)%360;
   hr = (id/15)%24;
   mn = 4*(id%15);
   //  Compute month and day
   dy = doy+1;
   for (mo=0;dy>ndm[mo];mo++)
      dy -= ndm[mo];
   fh = (dy-1)/(float)ndm[mo];
   mo++;

   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Set tranformation
   glLoadIdentity();
   glRotatef(ph,1,0,0);
   glRotatef(th,0,1,0);

   //  OpenGL should normalize normal vectors
   glEnable(GL_NORMALIZE);
   //  Enable lighting
   glEnable(GL_LIGHTING);
   //  Enable light 0
   glEnable(GL_LIGHT0);
   //  Set position of light 0
   glLightfv(GL_LIGHT0,GL_POSITION,Position);

   //  Texture for this month
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D,day[mo-1]);
   //  Texture for next month
   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D,day[mo%12]);

   //  Rotate Z up and inclined 23.5 degrees
   glRotated(-90,1,0,0);
   glRotated(-23.5,0,1,0);
   //  Draw planet
   glColor3f(1,1,1);
   glEnable(GL_TEXTURE_2D);
   //  Rotation around spin axis
   glRotated(zh,0,0,1);
   //  Solid
   gluQuadricDrawStyle(ball,GLU_FILL);
   //  Calculate normals
   gluQuadricNormals(ball,GLU_SMOOTH);
   //  Apply Textures
   gluQuadricTexture(ball,1);
   //  Enable shader
   glUseProgram(shader);
   //  Set textures
   id = glGetUniformLocation(shader,"DayTex0");
   if (id>=0) glUniform1i(id,0);
   id = glGetUniformLocation(shader,"DayTex1");
   if (id>=0) glUniform1i(id,1);
   id = glGetUniformLocation(shader,"NightTex");
   if (id>=0) glUniform1i(id,2);
   id = glGetUniformLocation(shader,"CloudGloss");
   if (id>=0) glUniform1i(id,3);
   id = glGetUniformLocation(shader,"mode");
   if (id>=0) glUniform1i(id,mode);
   id = glGetUniformLocation(shader,"frac");
   if (id>=0) glUniform1f(id,fh);
   //  Draw the ball
   gluSphere(ball,2.0,72,72);
   //  Shader off
   glUseProgram(0);

   //  No lighting from here on
   glDisable(GL_LIGHTING);
   //  Axes
   glBegin(GL_LINES);
   glVertex3f(0,0,+2.5);
   glVertex3f(0,0,-2.5);
   glEnd();

   //  Display parameters
   glColor3f(1,1,1);
   glWindowPos2i(5,5);
   Print("FPS=%d Dim=%.1f %d/%.2d %.2d:%.2d UTC %s", 
      FramesPerSecond(),dim,mo,dy,hr,mn,text[mode]);
   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when idle
 */
void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   //  Rotate at 15/sec
   int it = fmod(15*t,DT);
   //  Set movement by days or degrees
   if (move) zh = (mode==0) ? (360*it)%DT : it;
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
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
   //  PageUp key - increase dim
   else if (key == GLUT_KEY_PAGE_DOWN)
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_UP && dim>1)
      dim -= 0.1;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Set projection
   Project(0,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   int dt = (mode==0) ? 360 : 1;
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   //  Toggle light movement
   else if (ch == 's' || ch == 'S')
      move = 1-move;
   //  Toggle mode
   else if (ch == 'm')
      mode = (mode+1)%MODE;
   else if (ch == 'M')
      mode = (mode+MODE-1)%MODE;
   //  Day position
   else if (ch == '-')
      zh -= 360;
   else if (ch == '+')
      zh += 360;
   //  Light position
   else if (ch == '[')
      zh -= dt;
   else if (ch == ']')
      zh += dt;
   //  Adjust zh into range
   while (zh<0)
     zh += DT;
   while (zh>=DT)
     zh -= DT;
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
   Project(0,asp,dim);
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   int n;
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(600,600);
   glutCreateWindow("Stored Textures");
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
   glutIdleFunc(idle);
   //  Make sure enough texture units are available
   glGetIntegerv(GL_MAX_TEXTURE_UNITS,&n);
   if (n<4) Fatal("Insufficient texture Units %d\n",n);
   //  Allocate quadric for ball
   ball = gluNewQuadric();
   //  Load daytime textures
   day[0]  = LoadTexBMP("day01.bmp");
   day[1]  = LoadTexBMP("day02.bmp");
   day[2]  = LoadTexBMP("day03.bmp");
   day[3]  = LoadTexBMP("day04.bmp");
   day[4]  = LoadTexBMP("day05.bmp");
   day[5]  = LoadTexBMP("day06.bmp");
   day[6]  = LoadTexBMP("day07.bmp");
   day[7]  = LoadTexBMP("day08.bmp");
   day[8]  = LoadTexBMP("day09.bmp");
   day[9]  = LoadTexBMP("day10.bmp");
   day[10] = LoadTexBMP("day11.bmp");
   day[11] = LoadTexBMP("day12.bmp");
   //  Load nightime texture
   glActiveTexture(GL_TEXTURE2);
   night = LoadTexBMP("night.bmp");
   //  Load cloud & gloss texture
   glActiveTexture(GL_TEXTURE3);
   cloudgloss = LoadTexBMP("cloudgloss.bmp");
   //  Create shader programs
   shader = CreateShaderProg("earth.vert","earth.frag");
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
