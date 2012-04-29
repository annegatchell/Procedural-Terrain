#include <iostream>
#include <cstdlib>
#include "Block.h"


using namespace std;

Block::Block(){
	int temp[] = {0,0,0};
	setLocation(temp);
	isEmpty = false;
	//TODO Initialize the cell array
}

void Block::setIsEmpty(bool empty){
	isEmpty = empty;
	
}

bool Block::getIsEmpty(){
	return isEmpty;
	
}

void Block::setLocation(int loc[3]){
	location.x = loc[0];
	location.y = loc[1];
	location.z = loc[2];
}	

GridLocation Block::getLocation(){
	return location;
}

/*
 * Function to use a frag shader to get all the values at the corners of
 * all the cells in the given block.
 * Uses ARB_draw_instanced to draw 33 instances of two triangles that
 * cover up the Rnder Portal.
 */
 void generateDensityValuesForBlock(Block block){
	 glLoadIdentity();
	 
	 int instanceCount = 33;
	 
	 const GLfloat portalTriangles[6][2] = {
		 { 0.0, 0.0},   /*index 0*/
		 { 0.0, 1.0},   /*index 1*/
		 { 1.0, 0.0},   /*index 2*/
		 { 0.0, 1.0},
		 { 1.0, 1.0},
		 { 1.0, 0.0} }; 
	 //const GLubyte indices[6] = {0, 1, 2,  1, 2, 3};
	 glGenBuffers(1, &VBOid[0]);
	 glBindBuffer(GL_ARRAY_BUFFER, VBOid[0]);
	 glBufferData(GL_ARRAY_BUFFER, 6*2*sizeof(GLfloat), portalTriangles, GL_STATIC_DRAW);
	 glDrawElementsInstanced(GL_TRIANGLE_STRIP,6,GL_UNSIGNED_SHORT, 0, instanceCount);
}
