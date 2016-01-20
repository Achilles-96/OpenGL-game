all: sample2D


sample2D: Sample_GL3_2D.cpp glad.c
	g++ -o sample2D Sample_GL3_2D.cpp glad.c -lGL -lglfw -ldl

mycode: mycode.cpp glad.c
	g++ -o myout mycode.cpp glad.c -lGL -lglfw -ldl

clean:
	rm sample2D sample3D
