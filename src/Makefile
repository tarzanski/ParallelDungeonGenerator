APP_NAME=generate

OBJS+=Clarkson-Delaunay.o
OBJS+=generate.o
OBJS+=main.o

CXX = g++ -m64 -std=c++11
#CXXFLAGS = -L ../../local/lib -I include/SDL2 -Wall #-O3
CXXFLAGS= -Wall

default: $(APP_NAME)

$(APP_NAME): $(OBJS)
	$(CXX) $(CXXFLAGS)  -o $@ $(OBJS) `sdl2-config --cflags --libs`

%.o: %.cpp
	$(CXX) $< $(CXXFLAGS)  -c -o $@ `sdl2-config --cflags --libs`

clean:
	/bin/rm -rf *~ *.o $(APP_NAME)
