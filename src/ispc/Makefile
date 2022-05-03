APP_NAME=generate_ispc

TASKSYS_CXX=tasksys.cpp
TASKSYS_LIB=-lpthread
TASKSYS_OBJ=$(addprefix $(OBJDIR)/, $(TASKSYS_CXX:.cpp=.o))

OBJS+=$(OBJDIR)/Clarkson-Delaunay.o
OBJS+=$(OBJDIR)/generate.o
OBJS+=$(OBJDIR)/main.o
OBJS+=$(OBJDIR)/generate_ispc.o
OBJS+=$(TASKSYS_OBJ)
OBJDIR=objs

CXX = g++ -m64 -std=c++11
CXXFLAGS = -L ../../local/lib -I include/SDL2 -Wall `sdl2-config --cflags --libs`#-O3
ISPC = ispc
# note: change target to sse4 for SSE capable machines
ISPCFLAGS=-O2 --target=avx1-i32x8 --arch=x86-64

.PHONY: clean

default: $(APP_NAME)

dirs:
	/bin/mkdir -p $(OBJDIR)/

$(APP_NAME): dirs $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) -lm $(TASKSYS_LIB)

$(OBJDIR)/%.o: ../%.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

$(OBJDIR)/%.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

$(OBJDIR)/generate.o: $(OBJDIR)/generate_ispc.h

$(OBJDIR)/%_ispc.h $(OBJDIR)//%_ispc.o: %.ispc
	$(ISPC) $(ISPCFLAGS) $< -o $(OBJDIR)/$*_ispc.o -h $(OBJDIR)/$*_ispc.h

clean:
	/bin/rm -rf *~ *.o $(APP_NAME) $(OBJDIR)