#ifndef BLOCK_H
#define BLOCK_H

#define CX 32
#define CY 32
#define CZ 32

using namespace std;

#include <iostream>
#include <cstdlib>
#include <stdint.h>

class Block {
private:
	uint8_t cell[CX][CY][CZ];
	bool isEmpty;
	
public: 
	Block();
	void setIsEmpty(bool empty);
	bool getIsEmpty();
};
#endif
