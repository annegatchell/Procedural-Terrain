#  Executables
main:main.o Sphere.o Cube.o CSCIx229.a
	g++ -Wall -O3 -o $@ $^ -lglut -lGLU

#  Compile rules
.c.o:
	g++ -c -O3 -Wall $<
.cpp.o:
	g++ -c -O3 -Wall $<

#  Delete
clean:
	rm -f main *.o *.a

#  Create archive (include glWindowPos if you need it)
CSCIx229.a:fatal.o loadtexbmp.o print.o project.o errcheck.o object.o
	ar -rcs CSCIx229.a $^

Cube.o: Cube.cpp CSCIx229.h Cube.h Object.h
Sphere.o: Sphere.cpp CSCIx229.h Object.h Sphere.h
errcheck.o: errcheck.c CSCIx229.h
fatal.o: fatal.c CSCIx229.h
loadtexbmp.o: loadtexbmp.c CSCIx229.h
main.o: main.cpp CSCIx229.h Cube.h Object.h Sphere.h
object.o: object.c CSCIx229.h
print.o: print.c CSCIx229.h
project.o: project.c CSCIx229.h
