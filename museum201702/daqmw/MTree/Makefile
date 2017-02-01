CXX = g++

ROOTFLAGS = `root-config --cflags`
ROOTLIBS  = `root-config --libs`

all: $(shell ls *.cpp | sed "s/\.cpp/\.o/g")

.cpp.o:
	$(CXX) $(ROOTFLAGS) -c $< -o $@
MTree.o       : MTree.h  MTree.cpp

.PHONY: clean
clean:
	rm $(shell ls *.cpp | sed "s/\.cpp/\.o/g")
