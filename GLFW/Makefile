all: sample2D


sample2D: Sample_GL3_2D.cpp glad.c
	g++ -o sample2D Sample_GL3_2D.cpp glad.c -lGL -lglfw -ldl

mycode: mycode.cpp glad.c
	g++  -o myout mycode.cpp glad.c -lGL -lglfw -lftgl -lSOIL -ldl -I/usr/include -I/usr/local/include  -I/usr/local/include/freetype2 -L/usr/local/lib

clean:
	rm sample2D sample3D
