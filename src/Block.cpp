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
