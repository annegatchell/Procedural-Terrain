//
//  Density Vertex shader
//

uniform sampler3D density;
uniform float x;
uniform float y;
uniform float z;

void main()
{
   //  Remember the color
   gl_FrontColor = gl_Color;
   //  Defer all transformations to geometry shader
   gl_Position   = gl_Vertex;
}
