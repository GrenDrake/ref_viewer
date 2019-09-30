CXXFLAGS=-std=c++11 -g -Wall `sdl2-config --cflags`
LIBS=-lSDL2_image `sdl2-config --libs`
OBJS=sdlref.o startup.o filelist.o

sdlref: $(OBJS)
	$(CXX) $(OBJS) $(LIBS) -o sdlref


