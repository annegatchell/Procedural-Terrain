#ifndef BLOCK_H
#define BLOCK_H

#define CX 32
#define CY 32
#define CZ 32

using namespace std;

#include <iostream>
#include <cstdlib>
#include <stdint.h>
#include "Cell.h"
#include "GridLocation.h"

class Block {
private:
	uint8_t cell[CX][CY][CZ];
	bool isEmpty;
	GridLocation location;
	
public: 
	Block();
	void setIsEmpty(bool empty);
	bool getIsEmpty();
	void setLocation(int loc[3]);
	GridLocation getLocation();
};
#endif
