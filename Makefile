#This is the target that compiles our executable
all : base.cc
	g++ -ggdb base.cc -lGLEW -lGLU -lGL -Iinclude `pkg-config gtkmm-3.0 --cflags --libs`