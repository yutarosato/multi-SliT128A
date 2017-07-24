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

cal_eff_all:cal_eff_all.o
	$(CXX) $(ROOTLIBS) -o $@ $^

cal_eff_all32:cal_eff_all32.o
	$(CXX) $(ROOTLIBS) -o $@ $^

maketree:maketree.o
	$(CXX) $(ROOTLIBS) -o $@ $^

scurve:scurve.o
	$(CXX) $(ROOTLIBS) -o $@ $^

threshold_calib:threshold_calib.o
	$(CXX) $(ROOTLIBS) -o $@ $^

tot_chg:tot_chg.o
	$(CXX) $(ROOTLIBS) -o $@ $^

.cpp.o:
	$(CXX) $(ROOTFLAGS) -c $< -o $@
qc_allch.o             : setting.h qc_allch.cpp
qc_onech.o             : setting.h qc_onech.cpp
cal_eff.o              : setting.h cal_eff.cpp
cal_eff_all.o          : setting.h cal_eff_all.cpp
cal_eff_all32.o        : setting.h cal_eff_all32.cpp
maketree.o             : setting.h maketree.cpp
scurve.o               : setting.h scurve.cpp
threshold_calib.o      : setting.h threshold_calib.cpp
tot_chg.o              : setting.h tot_chg.cpp

.PHONY: clean
clean:
	rm $(shell ls *.cpp | sed "s|\.cpp|\.o|g") $(shell ls *.cpp | sed "s|\.cpp||g")
