APP_NAME=generate

OBJS+=$(OBJDIR)/Clarkson-Delaunay.o
OBJS+=$(OBJDIR)/generate.o
OBJS+=$(OBJDIR)/main.o
OBJDIR=objs

CXX = g++ -m64 -std=c++11
CXXFLAGS = -L ../../local/lib -I include/SDL2 -Wall `sdl2-config --cflags --libs`#-O3

.PHONY: clean

default: $(APP_NAME)

dirs:
	/bin/mkdir -p $(OBJDIR)/

$(APP_NAME): dirs $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

clean:
	/bin/rm -rf *~ *.o $(APP_NAME) $(OBJDIR)