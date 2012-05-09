
//
//  Density fragment shader
//

uniform sampler2D densityImg;

void main()
{
   gl_FragColor = texture2D(densityImg,gl_TexCoord[0].st);
  // gl_FragColor = vec4(1.0,0.0,0.0,1.0);
}