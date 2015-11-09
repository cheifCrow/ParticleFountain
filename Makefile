CPP = g++
FLAGS = -Wall
OBJ = main.cpp
LIBS = -lGL -lGLU -lglut

particles.bin: $(OBJ)
	g++ $(FLAGS) $(LIBS) -o $@ $^
