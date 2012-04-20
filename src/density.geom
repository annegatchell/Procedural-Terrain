//
//  Density geometry shader
//

//  Requires OpenGL 3.2 in compatibility mode
#version 150 compatibility
#extension GL_EXT_geometry_shader4 : enable
layout(points) in;
layout(triangle_strip,max_vertices=1) out;

void main()
{
   float s = 0.5;
}
