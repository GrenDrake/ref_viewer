CXXFLAGS=-std=c++11 -g -Wall `sdl2-config --cflags`
LIBS=-lSDL2_image `sdl2-config --libs`
OBJS=src/sdlref.o src/startup.o src/filelist.o

sdlref: $(OBJS)
	$(CXX) $(OBJS) $(LIBS) -o sdlref

clean:
	$(RM) src/*.o

.PHONY: clean
