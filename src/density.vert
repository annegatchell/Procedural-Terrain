//
//  Density Vertex shader
//

uniform float time;

void main()
{
   //  Set vertex coordinates
   vec4 pos = gl_Vertex;
   gl_Position = gl_ModelViewProjectionMatrix * pos;
}
