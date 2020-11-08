#This is the target that compiles our executable
all : base.cc
	g++ -ggdb base.cc -lGLEW -lGLU -lGL `pkg-config gtkmm-3.0 --cflags --libs`