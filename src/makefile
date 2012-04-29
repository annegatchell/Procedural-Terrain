EX=vortex ex23


#  Libraries - Linux
LIBS=-lglut -lGLU -lOpenCL
#  Libraries - OSX
#LIBS=-framework GLUT -framework OpenGL
#  Libraries - MinGW
#LIBS=-lglut32cu -lglu32 -lopengl32

#  Executables
#vortex:vortex.o CSCIx239.a
#	g++ -Wall -O3 -o $@ $^ $(LIBS)
ex23:ex23.o
	g++ -Wall -O3 -o $@ -I/opt/AMDAPP/include $^ $(LIBS)

#  Main target
#all: $(EX)
#  Executables
#main:main.o Cell.o Block.o CSCIx239.a
#	g++ -Wall -O3 -o $@ $^ -lglut -lGLU

#  Generic compile rules
.c.o:
	gcc -c -O3 -Wall -I/opt/AMDAPP/include $<
.cpp.o:
	g++ -c -O3 -Wall -I/opt/AMDAPP/include $<

#  Generic compile and link
#%: %.c CSCIx239.a
#	gcc -Wall -O3 -o $@ $^ $(LIBS)

#  Delete
clean:
	rm -f $(EX) *.o *.a

#  Create archive (include glWindowPos if you need it)
CSCIx239.a:fatal.o loadtexbmp.o print.o project.o errcheck.o object.o fps.o elapsed.o shader.o noise.o
	ar -rcs CSCIx239.a $^

errcheck.o: errcheck.c CSCIx239.h
fatal.o: fatal.c CSCIx239.h
loadtexbmp.o: loadtexbmp.c CSCIx239.h
main.o: main.cpp CSCIx239.h
object.o: object.c CSCIx239.h
print.o: print.c CSCIx239.h
project.o: project.c CSCIx239.h
fps.o: fps.c CSCIx239.h
elapsed.o: elapsed.c CSCIx239.h
shader.o: shader.c CSCIx239.h
noise.o: noise.c CSCIx239.h


#  Obligatory UNIX inside joke
love:
	@echo "not war?"
