#include "CSCIx239.h"
#include <iostream>
using namespace std;

unsigned int* voxelArray;
GLint MaxTexSize;
GLint Width = 600; //Window Width
GLint Height = 600; //Window Height
GLfloat asp = (Height > 0) ? Width/Height : 1;
GLint W, H; //Density 3d texture dimensions
#define D 33 //Depth of density texture

static GLuint densityImg = 0; //Density texture location
GLuint densityShader;

#define testImageSize 64
static GLubyte checkimage[testImageSize][testImageSize][4];

void makeDensityCube(void){
	int i, j, c;

	for(i = 0; i < testImageSize; i++){
		for(j = 0; j < testImageSize; j ++){
			c = (((i&0x8)==0)^((j&0x8)==0));
			checkimage[i][j][0] = (GLubyte) c;
			checkimage[i][j][1] = (GLubyte) c;
			checkimage[i][j][2] = (GLubyte) c;
			checkimage[i][j][3] = (GLubyte) 255;
		}
	}
}

//using namespace std;

void init(void)
{
	//glGenBuffers(300, voxelArray);

	/*select clearing (background) color */
	glClearColor(0.0,0.0,0.0,0.0);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);
	makeDensityCube();
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	glGenTextures(1, &densityImg);
	glBindTexture(GL_TEXTURE_2D, densityImg);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, testImageSize, testImageSize, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, checkimage);

	densityShader = CreateShaderProg("density.vert", "density.frag");
	//  Maximum texture size
   //glGetIntegerv(GL_MAX_TEXTURE_SIZE,&MaxTexSize);
	
}

void display(void)
{
/* clear all pixels */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_TEXTURE_2D);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	//glBindTexture(GL_TEXTURE_2D, densityImg);
	glUseProgram(densityShader);
	int id = glGetUniformLocation(densityShader, "densityImg");
	if(id>=0) glUniform1f(id, 0);

	//Disable depth
    glDisable(GL_DEPTH_TEST);
   	//  Identity projections
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glViewport(0,0,Width,Height);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, densityImg);
	glBegin(GL_QUADS);
	glTexCoord2f(0,0); glVertex2f(-1,-1);
    glTexCoord2f(0,1); glVertex2f(-1,+1);
    glTexCoord2f(1,1); glVertex2f(+1,+1);
    glTexCoord2f(1,0); glVertex2f(+1,-1);
	glEnd();

	glFlush();
	glDisable(GL_TEXTURE_2D);
	glutSwapBuffers();
}



void reshape(int w, int h)	
{
	/*// Ratio of the width to height of window
	asp = (h>0) ? (GLfloat) w/h : 1;
	// Set the viewport to the entire window
	glViewport(0,0, (GLsizei) w, (GLsizei) h);

	//Set size of 3d densithy texture
	W = Width = w;
	H = Height = h;
	if (W>MaxTexSize) W = MaxTexSize;
	if (H>MaxTexSize) H = MaxTexSize;
	//Set texture offsets for kernel
	dX = 1.0/W;
	dY = 1.0/H;

	//Allocate and size 3d texture;
	glGenTextures(1,&densityImg);
	glBindTexture(GL_TEXTURE_3D, densityImg);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, W, H, D,
					0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	*/
	glViewport(0,0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho2D(0.0, (GLdouble) w, 0.0, (GLdouble) h);
	gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 30.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0,0.0,-3.6);
}

void mouse(int button, int state, int x, int y)
{
	switch (button) {
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN)
				//glutIdleFunc(spinDisplay);
			break;
		case GLUT_RIGHT_BUTTON:
			if (state == GLUT_DOWN)
				//glutIdleFunc(NULL);
			break;
		default:
			break;
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(600,600);
	glutInitWindowPosition(100,100);
	glutCreateWindow("The Best Window I Have Ever Seen");
//#ifdef USEGLEW
//  Initialize GLEW
  // if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
  // if (!GLEW_VERSION_2_0) Fatal("OpenGL 2.0 not supported\n");
//#endif
	init();
	GLint tex  = 1;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS,&tex);
	cout << "texture units " << tex << endl; 
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
// Create Shader Programs
//	densityShader = CreateShaderProg("density.vert", "density.frag");
	glutMainLoop();
	return 0;	
}