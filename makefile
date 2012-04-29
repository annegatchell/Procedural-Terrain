#  Executables
ex23:ex23.o
	g++ -Wall -O3 -o $@ -I/opt/AMDAPP/include $^ -lglut -lGLU -lOpenCL

#  Libraries - Linux
#LIBS=-lglut -lGLU
#  Libraries - OSX
#LIBS=-framework GLUT -framework OpenGL
#  Libraries - MinGW
#LIBS=-lglut32cu -lglu32 -lopengl32

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
	rm -f ex23 *.o *.a

#  Create archive (include glWindowPos if you need it)


#  Obligatory UNIX inside joke
love:
	@echo "not war?"
