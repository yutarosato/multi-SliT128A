CXX = g++

ROOTFLAGS = `root-config --cflags`
ROOTLIBS  = `root-config --libs`

all: $(shell ls *.cpp | sed "s|\.cpp||g")

qc_allch:qc_allch.o
	$(CXX) $(ROOTLIBS) -o $@ $^

qc_onech:qc_onech.o
	$(CXX) $(ROOTLIBS) -o $@ $^

cal_eff:cal_eff.o
	$(CXX) $(ROOTLIBS) -o $@ $^

maketree:maketree.o
	$(CXX) $(ROOTLIBS) -o $@ $^

scurve:scurve.o
	$(CXX) $(ROOTLIBS) -o $@ $^

tot_width:tot_width.o
	$(CXX) $(ROOTLIBS) -o $@ $^

autocalib:autocalib.o
	$(CXX) $(ROOTLIBS) -o $@ $^

.cpp.o:
	$(CXX) $(ROOTFLAGS) -c $< -o $@
qc_allch.o             : setting.h qc_allch.cpp
qc_onech.o             : setting.h qc_onech.cpp
cal_eff.o              : setting.h cal_eff.cpp
maketree.o             : setting.h maketree.cpp
scurve.o               : setting.h scurve.cpp
tot_width.o            : setting.h tot_width.cpp
autocalib.o            : setting.h autocalib.cpp

.PHONY: clean
clean:
	rm $(shell ls *.cpp | sed "s|\.cpp|\.o|g") $(shell ls *.cpp | sed "s|\.cpp||g")
