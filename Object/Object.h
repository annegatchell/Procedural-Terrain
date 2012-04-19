//
//  Object class
//  Dummy generic object class
//  All functions are virtual
//  In a real example this may have contained some
//  common parameters like (x,y,z)
//
#ifndef OBJECT_H
#define OBJECT_H

class Object
{
public:
   Object() {}                //  Do nothing constructor
   virtual void display()=0;  //  Render the object
   virtual ~Object() {};      //  Do nothing destructor
};

#endif
