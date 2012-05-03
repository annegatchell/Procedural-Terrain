//  Identity vertex shader

void main()
{
   //  Use color unchanged
   gl_FrontColor = gl_Color;
   //  Set vertex coordinates
   //if(gl_Vertex.x == 42 || gl_Vertex.y == 42 || gl_Vertex.z == 42)
   	//gl_FrontColor = vec4(0,0,0,0);
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
