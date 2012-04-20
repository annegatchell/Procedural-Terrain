/*
 *  Real Time Image Processing
 *
 *  'm' to switch filters
 *  arrows pan
 *  PgUp/PgDn zooms in/out
 */
#include "CSCIx239.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
int mode=0;         //  Filter to use
int axes=1;         //  Draw crosshairs
double asp=1;       //  Aspect ratio
double zoom=1;      //  Zoom factor
int   N=1;          //  Passes
int   W0,H0;        //  Capture dimensions
int   W1,H1;        //  Window dimensions
float X=0,Y=0;      //  Initial position
CvCapture*   cv;    //  OpenCV context
unsigned int cvtex; //  OpenCV texture
unsigned int imtex; //  Image texture
#define MODE 9
int shader[MODE] = {0}; //  Shader programs
char* text[] = {"No Shader","Copy","Sharpen","Blur","Erosion","Dilation","Laplacian Edge Detection","Prewitt Edge Detection","Sobel Edge Detection"};

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   int k;
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT);

   //  Set projection
   glLoadIdentity();
   Project(0,asp,1.0);

   //  Set up for drawing
   glColor3f(1,1,1);
   glEnable(GL_TEXTURE_2D);

   //  Set shader
   if (mode>0)
   {
      glUseProgram(shader[mode]);
      int id = glGetUniformLocation(shader[mode],"img");
      if (id>=0) glUniform1i(id,0);
   }

   //  Draw shader passes to a quad
   for (k=0;k<N;k++)
   {
      //  Quad width
      float w = (k==0) ? 1 : asp;
      glPushMatrix();
      //  Initial pass - camera image
      if (k==0)
      {
         glBindTexture(GL_TEXTURE_2D,cvtex);
         glScaled(zoom,zoom,1);
         glTranslated(X,Y,0);
      }
      //  Repeat pass - screen mage
      else
      {
         glBindTexture(GL_TEXTURE_2D,imtex);
         glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,0,0,W1,H1,0); 
      }

      //  Set offsets
      if (mode>0)
      {
         float dX = (k==0) ? 1.0/W0 : zoom/W1;
         float dY = (k==0) ? 1.0/H0 : zoom/H1;
         int id = glGetUniformLocation(shader[mode],"dX");
         if (id>=0) glUniform1f(id,dX);
         id = glGetUniformLocation(shader[mode],"dY");
         if (id>=0) glUniform1f(id,dY);
      }

      //  Redraw the texture
      glClear(GL_COLOR_BUFFER_BIT);
      glBegin(GL_QUADS);
      glTexCoord2f(0,0); glVertex2f(-w,-1);
      glTexCoord2f(0,1); glVertex2f(-w,+1);
      glTexCoord2f(1,1); glVertex2f(+w,+1);
      glTexCoord2f(1,0); glVertex2f(+w,-1);
      glEnd();
      glPopMatrix();
   }
   glDisable(GL_TEXTURE_2D);

   //  Shader off
   glUseProgram(0);

   //  Draw crosshairs
   if (axes)
   {
      glBegin(GL_LINES);
      glVertex2f(-0.1,0);
      glVertex2f(+0.1,0);
      glVertex2f(0,-0.1);
      glVertex2f(0,+0.1);
      glEnd();
   }

   //  Display parameters
   glWindowPos2i(5,5);
   Print("Zoom=%.1f Offset=%f,%f Mode=%s Passes=%d",
     zoom,X,Y,text[mode],N);
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
      X -= 0.03/zoom;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      X += 0.03/zoom;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      Y -= 0.03/zoom;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      Y += 0.03/zoom;
   //  PageUp key - increase zoom
   else if (key == GLUT_KEY_PAGE_DOWN)
      zoom /= 1.1;
   //  PageDown key - decrease zoom
   else if (key == GLUT_KEY_PAGE_UP)
      zoom *= 1.1;
   //  Limit zoom
   if (zoom<1)
   {
      zoom = 1;
      X = Y = 0;
   }
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
   //  Reset view
   else if (ch == '0')
      X = Y = 0;
   //  Cycle modes
   else if (ch == 'm')
      mode = (mode+1)%MODE;
   else if (ch == 'M')
      mode = (mode+MODE-1)%MODE;
   //  Passes
   else if (ch == 'N' && N>1)
      N--;
   else if (ch == 'n')
      N++;
   //  Toggle axes
   else if (ch == 'a')
      axes = !axes;
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
   //  Set image dimensions
   W1 = width;
   H1 = height;
}

/*
 *  Function called to capture images at 20ms intervals
 */
void capture(int k)
{
   char* rgb;    //  Packed image
   int   i,j;    //  Pixel index
   int   r,g,b;  //  Color offset
   //  Capture image
   IplImage* img = cvQueryFrame(cv); 
   if (!img) Fatal("Image capture failed\n");
   W0 = img->width;
   H0 = img->height;
   if (img->depth!=IPL_DEPTH_8U && img->depth!=IPL_DEPTH_8S) Fatal("Only 8 bits/pixel supported\n");

   //  Map RGB channels
   if (img->nChannels<3)
     r = g = b = 0;
   else if (strcmp(img->channelSeq,"RGB"))
   {
      r = 2; g = 1; b = 0;
   }
   else
   {
      r = 0; g = 1; b = 2;
   }

   //  Pack image into RGB with lower left origin
   rgb = (char*)malloc(3*W0*H0);
   if (!rgb) Fatal("Cannot allocate %d bytes for image\n",3*W0*H0);
   for (i=0;i<H0;i++)
   {
      char* src = img->imageData+ (img->origin ? i : H0-i-1)*img->widthStep;
      char* dst = rgb + i*3*W0;
      for (j=0;j<W0;j++)
      {
         *dst++ = *(src+r);
         *dst++ = *(src+g);
         *dst++ = *(src+b);
         src += img->nChannels;
      }
   }

   //  Copy image to texture
   glBindTexture(GL_TEXTURE_2D,cvtex);
   glTexImage2D(GL_TEXTURE_2D,0,3,W0,H0,0,GL_RGB,GL_UNSIGNED_BYTE,rgb);
   ErrCheck("Capture");

   //  Free image
   free(rgb);

   //  Set timer
   glutTimerFunc(20,capture,0);

   //  Display
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
   glutInitWindowSize(600,600);
   glutCreateWindow("Video Processing");
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
   //  Initialize OpenCV
   cv = cvCreateCameraCapture(0);
   if (!cv) Fatal("Could not initialize OpenCV\n");
   //  Texture to store image
   glGenTextures(1,&cvtex);
   glBindTexture(GL_TEXTURE_2D,cvtex);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
   //  Texture to post-process image
   glGenTextures(1,&imtex);
   glBindTexture(GL_TEXTURE_2D,imtex);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
   //  Start image capture
   capture(0);
   //  Create Shader Programs
   shader[1] = CreateShaderProg(NULL,"copy.frag");
   shader[2] = CreateShaderProg(NULL,"sharpen.frag");
   shader[3] = CreateShaderProg(NULL,"blur.frag");
   shader[4] = CreateShaderProg(NULL,"erosion.frag");
   shader[5] = CreateShaderProg(NULL,"dilation.frag");
   shader[6] = CreateShaderProg(NULL,"laplacian.frag");
   shader[7] = CreateShaderProg(NULL,"prewitt.frag");
   shader[8] = CreateShaderProg(NULL,"sobel.frag");
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
