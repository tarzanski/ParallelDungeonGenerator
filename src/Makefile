APP_NAME=generate

OBJS=generate.o
OBJS+=main.o

CXX = g++ -m64 -std=c++11
CXXFLAGS = -I. -Wall #-O3

default: $(APP_NAME)

$(APP_NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) `sdl2-config --cflags --libs` -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $< $(CXXFLAGS) `sdl2-config --cflags --libs` -c -o $@

clean:
	/bin/rm -rf *~ *.o $(APP_NAME)