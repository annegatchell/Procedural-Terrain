EX=particle


#  Libraries - Linux
LIBS= -lglut -lGLU -lOpenCL
#  Libraries - OSX
#LIBS=-framework GLUT -framework OpenGL
#  Libraries - MinGW
#LIBS=-lglut32cu -lglu32 -lopengl32

#  Executables

particle:particle.o CSCIx239.a
	g++ -Wall -O3 -fopenmp -o $@ -I/opt/AMDAPP/include $^ $(LIBS)
#particle:particle.o CSCIx239.a
#	g++ -Wall -O3 -o $@ -I/usr/local/cuda/include $^ $(LIBS)
#ex20:ex20.c CSCIx239.a;  gcc -fopenmp -Wall -o $@ $^ $(LIBS)

#  Generic compile rules
.c.o:
	gcc -c -O3 -fopenmp -Wall -I/opt/AMDAPP/include $<
.cpp.o:
	g++ -c -O3 -fopenmp -Wall -I/opt/AMDAPP/include $<

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
particle.o: particle.cpp CSCIx239.h
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
