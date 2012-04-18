#  Executables
EX=main

#  Libraries - Linux
LIBS=-lglut -lGLU
#  Libraries - OSX
#LIBS=-framework GLUT -framework OpenGL
#  Libraries - MinGW
#LIBS=-lglut32cu -lglu32 -lopengl32

#  Main target
all: $(EX)

#  Generic compile rules
.c.o:
	gcc -c -O3 -Wall $<
.cpp.o:
	g++ -c -O3 -Wall $<

#  Generic compile and link
%: %.c CSCIx239.a
	gcc -Wall -O3 -o $@ $^ $(LIBS)

#  Delete
clean:
	rm -f $(EX) *.o *.a

#  Create archive (include glWindowPos if you need it)
CSCIx239.a:fatal.o loadtexbmp.o print.o project.o errcheck.o object.o fps.o elapsed.o shader.o noise.o
	ar -rcs CSCIx239.a $^

#  OpenMP
ex20:ex20.c CSCIx239.a;  gcc -fopenmp -Wall -o $@ $^ $(LIBS)


#  Obligatory UNIX inside joke
love:
	@echo "not war?"
