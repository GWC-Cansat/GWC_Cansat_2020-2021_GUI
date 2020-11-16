CC = g++

CFLAGS = -g `pkg-config gtkmm-3.0 --cflags`

INCLUDE = -Iinclude

LDLIBS = -ldl `pkg-config gtkmm-3.0 --libs`

LDFLAGS = 

all: gui

gui:  base.cc
	$(CC) $(CFLAGS) $(INCLUDE) base.cc glad.c $(LDFLAGS) $(LDLIBS) -o gui
