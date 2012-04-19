//
//  Sphere class
//  The constructor sets the position and radius
//  All parameters are assigned default values
//
#ifndef SPHERE_H
#define SPHERE_H

#include "Object.h"

class Sphere: public Object
{
private:
   float x0,y0,z0;  //  Center
   float r0;        //  Radius
   float R,G,B;     //  Color
public:
   Sphere(float x=0,float y=0,float z=0,float r=1,float R=1,float G=1,float B=1);
   void translate(float x,float y,float z); //  Set translation
   void radius(float r);                    //  Set radius
   void display();                          //  Render the sphere
};

#endif
