//
//  Generic Object class
//
#include "CSCIx239.h"
#include "Object.h"

//
//  Constructor
//
Object::Object(float x,float y,float z)
{
   x0 = x;
   y0 = y;
   z0 = z;
}

//
//  Set translation
//
void Object::translate(float x,float y,float z)
{
   x0 = x;
   y0 = y;
   z0 = z;
}
