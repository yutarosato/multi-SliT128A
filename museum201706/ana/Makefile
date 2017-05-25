CXX = g++

ROOTFLAGS = `root-config --cflags`
ROOTLIBS  = `root-config --libs`

all: $(shell ls *.cpp | sed "s|\.cpp||g")

qc_allch:qc_allch.o
	$(CXX) $(ROOTLIBS) -o $@ $^

qc_onech:qc_onech.o
	$(CXX) $(ROOTLIBS) -o $@ $^

.cpp.o:
	$(CXX) $(ROOTFLAGS) -c $< -o $@
qc_allch.o             : setting.h qc_allch.cpp
qc_onech.o             : setting.h qc_onech.cpp

.PHONY: clean
clean:
	rm $(shell ls *.cpp | sed "s|\.cpp|\.o|g") $(shell ls *.cpp | sed "s|\.cpp||g")
