


typedef struct
{
	float x,y,z;  //  Position
  	float u,v,w;  //  Velocity
    float r,g,b;  //  Color
 } Star; 

__global whichStarSet = 0;


__kernel void initStar(__global float starsOld1[],__global float starsOld2[],const unsigned int n)
{
	
}

__kernel void starStep(__global float starsNew[],/*__global float starsOld1[],__global float starsOld2[],*/const unsigned int n)
{

  	unsigned int k0 = get_global_id(0);
//    unsigned int k1 = get_global_id(1);

	float dt = 1e-3;
	int i;

	//  Calculate force components
	double a=0;
	double b=0;
	double c=0;
	double dx, dy, dz, d, f;
	/*for (i=0;i<n;i++)
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
	stars[k1].u = stars[k0].u + dt*a;
	stars[k1].v = stars[k0].v + dt*b;
	stars[k1].w = stars[k0].w + dt*c;
	//  Update position
	stars[k1].x = stars[k0].x + dt*stars[k1].u;
	stars[k1].y = stars[k0].y + dt*stars[k1].v;
	stars[k1].z = stars[k0].z + dt*stars[k1].w;

	for (i=src;i<src+n;i++)
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

/*
 }

