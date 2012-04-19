//
//  Wave OBJ class
//
//  The STL vector template class is used to manage arrays
//
#include "CSCIx239.h"
#include "Vec.h"
#include "WaveOBJ.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

//  Material structure
typedef struct
{
   string name;     //  Material name
   Color  Ka,Kd,Ks; //  Colors
   float Ns;        //  Shininess
   float d;         //  Transparency
   int map;         //  Texture
} Material;
static vector<Material> mat;

//  Array of strings
typedef vector<string> Strings;

//  Split string on whitespace
static Strings split(const string &s)
{ 
   Strings words;

   string word;
   istringstream line(s);
   while (line >> word)
      words.push_back(word);
   return words;
}

//  Read float
static float Read1(string s)
{
   istringstream line(s);
   string l;
   float v;
   line >> l >> v;
   if (line.bad()) Fatal("Error reading 2 from %s\n",s.c_str());
   return v;
}

//  Read Vec2
static Vec2 Read2(string s)
{
   istringstream line(s);
   string l;
   Vec2 v;
   line >> l >> v.x >> v.y;
   if (line.bad()) Fatal("Error reading Vec2 from %s\n",s.c_str());
   return v;
}

//  Read Vec3
static Vec3 Read3(string s)
{
   istringstream line(s);
   string l;
   Vec3 v;
   line >> l >> v.x >> v.y >> v.z;
   if (line.bad()) Fatal("Error reading Vec3 from %s\n",s.c_str());
   return v;
}

//
//  Load materials from file
//
static void LoadMaterial(const string& file)
{
   //  Open file or return with warning on error
   ifstream f(file.c_str());
   if (!f.is_open())
   {
      fprintf(stderr,"Cannot open material file %s\n",file.c_str());
      return;
   }

   //  Process materials file
   int k = -1;
   while (f.good())
   {
      //  Read line
      string line;
      getline(f,line);
      //  Split line into words
      Strings word = split(line);
      //  New material
      if (word.size()==2 && word[0]=="newmtl")
      {
         k++;
         Material m;
         //  Initialize materials
         m.name = word[1];
         m.Ns  = 0;
         m.d   = 0;
         m.map = 0;
         mat.push_back(m);
      }
      //  If no material short circuit here
      else if (k<0)
      {}
      //  Ambient color
      else if (word.size()==4 && word[0]=="Ka")
         mat[k].Ka = Read3(line);
      //  Diffuse color
      else if (word.size()==4 && word[0]=="Kd")
         mat[k].Kd = Read3(line);
      //  Specular color
      else if (word.size()==4 && word[0]=="Ks")
         mat[k].Ks = Read3(line);
      //  Material Shininess
      else if (word.size()==2 && word[0]=="Ns")
         mat[k].Ns = Read1(line);
      //  Textures (must be BMP - will fail if not)
      else if (word.size()==2 && word[0]=="map_Kd")
         mat[k].map = LoadTexBMP(word[1].c_str());
      //  Ignore line if we get here
   }
   f.close();
}

//
//  Set material
//
static void SetMaterial(const string& name)
{
   //  Search materials for a matching name
   for (unsigned int k=0;k<mat.size();k++)
      if (mat[k].name==name)
      {
         //  Set material colors
         glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT  ,mat[k].Ka.fv());
         glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE  ,mat[k].Kd.fv());
         glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR ,mat[k].Ks.fv());
         glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&mat[k].Ns);
         //  Bind texture if specified
         if (mat[k].map)
         {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,mat[k].map);
         }
         else
            glDisable(GL_TEXTURE_2D);
         return;
      }
   //  No matches
   fprintf(stderr,"Unknown material %s\n",name.c_str());
}

//
//  Constructor
//
WaveOBJ::WaveOBJ(const char* file)
{
   //  Initialize color and scale
   R = G = B = s = 1;

   //  Open file
   ifstream f(file);
   if (!f.is_open()) Fatal("Cannot open file %s\n",file);

   //  Start new displaylist
   list = glGenLists(1);
   glNewList(list,GL_COMPILE);
   //  Push attributes for textures
   glPushAttrib(GL_TEXTURE_BIT);

   //  Process file
   mat.clear();
   vector<Vec3> V,N;
   vector<Vec2> T;
   while (f.good())
   {
      //  Read line
      string line;
      getline(f,line);
      //  Split line into words
      Strings word = split(line);
      //  Vertex coordinates (always 3)
      if (word.size()==4 && word[0]=="v")
         V.push_back(Read3(line));
      //  Normal coordinates (always 3)
      else if (word.size()==4 && word[0]=="vn")
         N.push_back(Read3(line));
      //  Texture coordinates (always 2)
      else if (word.size()==3 && word[0]=="vt")
         T.push_back(Read2(line));
      //  Read and draw facets
      else if (word.size()>3 && word[0]=="f")
      {
         //  Read Vertex/Texture/Normal triplets
         glBegin(GL_POLYGON);
         for (unsigned int k=1;k<word.size();k++)
         {
            int Kv,Kt,Kn;
            //  Try Vertex/Texture/Normal triplet
            if (sscanf(word[k].c_str(),"%d/%d/%d",&Kv,&Kt,&Kn)==3)
            {}
            //  Try Vertex//Normal pairs
            else if (sscanf(word[k].c_str(),"%d//%d",&Kv,&Kn)==2)
            {
               Kt = 0;
            }
            else if (sscanf(word[k].c_str(),"%d/%d",&Kv,&Kn)==2)
            {
               Kt = 0;
            }
            //  Try Vertex index
            else if (sscanf(word[k].c_str(),"%d",&Kv)==1)
            {
               Kn = 0;
               Kt = 0;
            }
            //  This is an error
            else
               Fatal("Invalid facet %s\n",word[k].c_str());
            //  Check index
            if (Kv<0 || Kv>(int)V.size()) Fatal("Vertex %d out of range 1-%d\n",Kv,V.size());
            if (Kn<0 || Kn>(int)N.size()) Fatal("Normal %d out of range 1-%d\n",Kn,N.size());
            if (Kt<0 || Kt>(int)T.size()) Fatal("Texture %d out of range 1-%d\n",Kt,T.size());
            //  Draw vertex
            if (Kt) glTexCoord2f(T[Kt-1].x,T[Kt-1].y);
            if (Kn) glNormal3f(N[Kn-1].x,N[Kn-1].y,N[Kn-1].z);
            if (Kv) glVertex3f(V[Kv-1].x,V[Kv-1].y,V[Kv-1].z);
         }
         glEnd();
      }
      //  Use material
      else if (word.size()==2 && word[0]=="usemtl")
         SetMaterial(word[1]);
      //  Load materials
      else if (word.size()==2 && word[0]=="mtllib")
         LoadMaterial(word[1]);
      //  Skip this line
   }
   f.close();
   //  Pop attributes (textures)
   glPopAttrib();
   glEndList();

}

//
//  Display
//
void WaveOBJ::display()
{
   glPushMatrix();
   glColor3f(R,G,B);
   glTranslatef(x0,y0,z0);
   glRotatef(th,dx,dy,dz);
   glScalef(s,s,s);
   glCallList(list);
   glPopMatrix();
}

//
//  Set scaling
//
void WaveOBJ::scale(float S)
{
   s = S;
}
  
//
//  Set color
//
void WaveOBJ::color(float r,float g,float b)
{
   R = r;
   G = g;
   B = b;
}
  
//
//  Set rotation
//
void WaveOBJ::rotate(float ang,float dX,float dY,float dZ)
{
   th = ang;
   dx = dX;
   dy = dY;
   dz = dZ;
}
