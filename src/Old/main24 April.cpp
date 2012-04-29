
 
#include "CSCIx239.h"
#include "Block.h"
#include "Cell.h"

using namespace std;

double asp=1;     //  Aspect ratio
int axes=1;       //  Display axes
int th=0;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
double dim=10;    //  Size of universe
int mode=0;       //  Mode
int shader=0;     //  Shader
int densityShader=0; //Shader for the density function
char* text[] = {"Terrain"};
int program[2];
unsigned int densityTexture; //3d density texture


GLuint VBOid[1];

Block* singleBlock = new Block();



/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   const double len=2.5;  //  Length of axes
   double Ex = -2*dim*Sin(th)*Cos(ph);
   double Ey = +2*dim        *Sin(ph);
   double Ez = +2*dim*Cos(th)*Cos (ph);

   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT);
   //  Undo previous transformations
   glLoadIdentity();
   Project(0,asp,1.0);
   
   //Set Up for Drawing
   glColor3f(1,1,1);
   glEnable(GL_TEXTURE_3D);
   
   //  Set Density Shader
   glUseProgram(densityShader);
   int id = glGetUniformLocation(densityShader,"density");
   if (id>=0) glUniform1i(id,0);
   
   //Draw shader pass to the triangles
   glBindTexture(GL_TEXTURE_3D, densityTexture);
   
   id = glGetUniformLocation(densityShader, "x");
   if(id >=0) glUniform1f(id, 0);
   id = glGetUniformLocation(densityShader, "y");
   if(id >=0) glUniform1f(id, 0);
   id = glGetUniformLocation(densityShader, "z");
   if(id >=0) glUniform1f(id, 0);
   
   generateDensityValuesForBlock(*singleBlock);
   
   //  Redraw the texture
   //   glClear(GL_COLOR_BUFFER_BIT);
    /*  glBegin(GL_QUADS);
      glTexCoord3f(0,0,0); glVertex2f(-1,-1);
      glTexCoord3f(0,1,0); glVertex2f(-1,+1);
      glTexCoord3f(1,1,0); glVertex2f(+1,+1);
      glTexCoord3f(1,0,0); glVertex2f(+1,-1);
      glEnd();*/
     // glPopMatrix();
     
      
   glDisable(GL_TEXTURE_2D);

   //  Shader off
   glUseProgram(0);
   
   
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
      mode = (mode+1)%1;
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

int CreateShaderProgGeom1(char* vertexShader, char* geometryShader, char* geometryShaderExt, char* fragShader){
	//Create program
	int prog = glCreateProgram();
	//Compile and add shaders
	CreateShader(prog,GL_VERTEX_SHADER, vertexShader);
	#ifdef __APPLE__
   //  OpenGL 3.1 for OSX
   cout << "HERE"<< endl;
   CreateShader(prog,GL_GEOMETRY_SHADER_EXT,geometryShaderExt);
   glProgramParameteriEXT(prog,GL_GEOMETRY_INPUT_TYPE_EXT  ,GL_POINTS);
   glProgramParameteriEXT(prog,GL_GEOMETRY_OUTPUT_TYPE_EXT ,GL_TRIANGLE_STRIP);
   glProgramParameteriEXT(prog,GL_GEOMETRY_VERTICES_OUT_EXT,4);
	#else
   //  OpenGL 3.2 adds layout ()
   CreateShader(prog,GL_GEOMETRY_SHADER, geometryShader);
	#endif
   CreateShader(prog,GL_FRAGMENT_SHADER, fragShader);
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
   glutCreateWindow("Badass Textures");
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);
   //3d texture to store density values
  // const int WIDTH = 33*sizeof(glFloat);
   //const int HEIGHT = 33*sizeof(glFloat);
   //const int CHANNELS = 1; //R
   //const int NUM_SLICES = 33;
   
  // unsigned char* buffer = new unsigned char[WIDTH*HEIGHT*CHANNELS*NUM_SLICES];
    
   glGenTextures(1,&densityTexture);
   glBindTexture(GL_TEXTURE_3D,densityTexture);
   //glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, 33*sizeof(glFloat), 33*sizeof(glFloat), NUM_SLICES, 0, GL_RGB, 
     //        GL_UNSIGNED_BYTE, buffer);
   //  Shader program
   //densityShader = CreateShaderProgGeom1("density.vert","density.geom", "density.geom_ext","density.frag");
   densityShader = CreateShaderProg("simple.vert", "simple.frag");
   ErrCheck("init");
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
