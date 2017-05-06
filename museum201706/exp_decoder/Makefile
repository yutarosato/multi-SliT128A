CC = g++

ROOTFLAGS = `root-config --cflags`
ROOTLIBS  = `root-config --libs`

all: $(shell ls *.cpp | sed "s/\.cpp//g")

multi-slit128a_exp_decoder:multi-slit128a_exp_decoder.o $(other)
	$(CXX) $(ROOTLIBS) -o $@ $^

.cpp.o:
	$(CXX) $(ROOTFLAGS) -c $< -o $@

multi-slit128a_exp_decoder.o                : multi-slit128a_exp_decoder.cpp

clean:
	rm $(shell ls *.cpp | sed "s|\.cpp|\.o|g") $(shell ls *.cpp | sed "s|\.cpp||g")