CC = g++

ROOTFLAGS = `root-config --cflags`
ROOTLIBS  = `root-config --libs`

all: $(shell ls *.cpp | sed "s/\.cpp//g")

slit128cmd_revise_chmap:slit128cmd_revise_chmap.o $(other)
	$(CXX) $(ROOTLIBS) -o $@ $^

edge_decode:edge_decode.o $(other)
	$(CXX) $(ROOTLIBS) -o $@ $^

fast_edge_decode:fast_edge_decode.o $(other)
	$(CXX) $(ROOTLIBS) -o $@ $^

.cpp.o:
	$(CXX) $(ROOTFLAGS) -c $< -o $@

slit128cmd_revise_chmap.o    : slit128cmd_revise_chmap.cpp
edge_decode.o                : edge_decode.cpp
fast_edge_decode.o           : fast_edge_decode.cpp


clean:
	rm $(shell ls *.cpp | sed "s|\.cpp|\.o|g") $(shell ls *.cpp | sed "s|\.cpp||g")