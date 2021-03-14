CC = g++

CFLAGS = -g `pkg-config gtkmm-3.0 --cflags` `pkg-config --cflags gtkmm-plplot-2.0`

INCLUDE = -Iinclude

LDLIBS = -ldl -lGL -lX11 -lepoxy `pkg-config gtkmm-3.0 --libs` `pkg-config --libs gtkmm-plplot-2.0`

LDFLAGS = 

all: gui

gui:  base.cc
	$(CC) $(CFLAGS) $(INCLUDE) base.cc $(LDFLAGS) $(LDLIBS) -o gui
