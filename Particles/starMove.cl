typedef struct
{
	float x,y,z;  //  Position
  	float u,v,w;  //  Velocity
    float r,g,b;  //  Color
 } Star;

__kernel void starStep(__global Star starsNew[],__global const Star starsOld1[],const unsigned int n)
{

  	unsigned int k0 = get_global_id(0);

	float dt = 0.1;
	int i;

	//  Calculate force components
	float a=0;
	float b=0;
	float c=0;
	float dx, dy, dz, d, f;
	for (i=0;i<n;i++)
	{
	  dx = starsOld1[i].x-starsOld1[k0].x;
	  dy = starsOld1[i].y-starsOld1[k0].y;
	  dz = starsOld1[i].z-starsOld1[k0].z;
	  d = sqrt(dx*dx+dy*dy+dz*dz)+1;  //  Add 1 to d to dampen movement
	  f = 1/(d*d*d); // Normalize and scale to 1/r^2
	  a += f*dx;
	  b += f*dy;
	  c += f*dz;
	}
	//  Update velocity
	starsNew[k0].u = starsOld1[k0].u + dt*a;
	starsNew[k0].v = starsOld1[k0].v + dt*b;
	starsNew[k0].w = starsOld1[k0].w + dt*c;
	//  Update position
	starsNew[k0].x = starsOld1[k0].x + dt*(starsOld1[k0].u + dt*a);
	starsNew[k0].y = starsOld1[k0].y + dt*(starsOld1[k0].v + dt*b);
	starsNew[k0].z = starsOld1[k0].z + dt*(starsOld1[k0].w + dt*c);

	starsNew[k0].r = starsOld1[k0].r;
	starsNew[k0].g = starsOld1[k0].g;
	starsNew[k0].b = starsOld1[k0].b;
 }

/*void Move(int k)
{

   int k0 = k+0;
   int k1 = k+dst;
   float dt = 1e-3;
   int i;
   //  Calculate force components
   double a=0;
   double b=0;
   double c=0;
   for (i=0;i<0+n;i++)
   {
      double dx = stars[i].x-stars[k0].x;
      double dy = stars[i].y-stars[k0].y;
      double dz = stars[i].z-stars[k0].z;
      double d = sqrt(dx*dx+dy*dy+dz*dz)+1;  //  Add 1 to d to dampen movement
      double f = 1/(d*d*d); // Normalize and scale to 1/r^2
      a += f*dx;
      b += f*dy;
      c += f*dz;
   }
   //  Update velocity
   stars[k1].u = stars[k0].u + dt*a;
   stars[k1].v = stars[k0].v + dt*b;
   stars[k1].w = stars[k0].w + dt*c;
   //  Update position
   stars[k1].x = stars[k0].x + dt*stars[k1].u;
   stars[k1].y = stars[k0].y + dt*stars[k1].v;
   stars[k1].z = stars[k0].z + dt*stars[k1].w;
}*/