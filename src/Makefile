APP_NAME=generate

CU_FILES=generate.cu

OBJDIR=objs
OBJS+=$(OBJDIR)/Clarkson-Delaunay.o
OBJS+=$(OBJDIR)/generate.o
OBJS+=$(OBJDIR)/main.o

CXX = g++ -m64 -std=c++11
CXXFLAGS= -L ../../local/lib -I include/SDL2 -Wall
LDFLAGS=-L/usr/local/depot/cuda-10.2/lib64/ -lcudart
NVCC=nvcc
NVCCFLAGS=-m64 -std=c++11 --expt-relaxed-constexpr --gpu-architecture compute_61 -ccbin /usr/bin/gcc

default: $(APP_NAME)

dirs:
	mkdir -p $(OBJDIR)/

$(APP_NAME): dirs $(OBJS)
	$(CXX) $(CXXFLAGS)  -o $@ $(OBJS) $(LDFLAGS) `sdl2-config --cflags --libs`

$(OBJDIR)/%.o: %.cpp
	$(CXX) $< $(CXXFLAGS)  -c -o $@ `sdl2-config --cflags --libs`

$(OBJDIR)/%.o: %.cu
	$(NVCC) $< $(NVCCFLAGS) -c -o $@


clean:
	/bin/rm -rf $(OBJDIR) *~ *.o $(APP_NAME)
