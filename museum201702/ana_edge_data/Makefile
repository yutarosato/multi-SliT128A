CXX = g++

ROOTFLAGS = `root-config --cflags`
ROOTLIBS  = `root-config --libs`

all: $(shell ls *.cpp | sed "s|\.cpp||g")

qc_allch:qc_allch.o
	$(CXX) $(ROOTLIBS) -o $@ $^

dist:dist.o
	$(CXX) $(ROOTLIBS) -o $@ $^

coincidence:coincidence.o
	$(CXX) $(ROOTLIBS) -o $@ $^


.cpp.o:
	$(CXX) $(ROOTFLAGS) -c $< -o $@
qc_allch.o             : setting.h qc_allch.cpp
dist.o                 : setting.h dist.cpp
coincidence.o          : setting.h coincidence.cpp

.PHONY: clean
clean:
	rm $(shell ls *.cpp | sed "s|\.cpp|\.o|g") $(shell ls *.cpp | sed "s|\.cpp||g")
