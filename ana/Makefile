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

linearity:linearity.o
	$(CXX) $(ROOTLIBS) -o $@ $^

offset_noise_gain:offset_noise_gain.o
	$(CXX) $(ROOTLIBS) -o $@ $^

timing:timing.o
	$(CXX) $(ROOTLIBS) -o $@ $^

timewalk_width:timewalk_width.o
	$(CXX) $(ROOTLIBS) -o $@ $^

timewalk_width_allch:timewalk_width_allch.o
	$(CXX) $(ROOTLIBS) -o $@ $^

idac_sim:idac_sim.o
	$(CXX) $(ROOTLIBS) -o $@ $^

modify_tree:modify_tree.o
	$(CXX) $(ROOTLIBS) -o $@ $^

threshold_calib:threshold_calib.o
	$(CXX) $(ROOTLIBS) -o $@ $^

.cpp.o:
	$(CXX) $(ROOTFLAGS) -c $< -o $@
qc_allch.o             : setting.h qc_allch.cpp
qc_onech.o             : setting.h qc_onech.cpp
cal_eff.o              : setting.h cal_eff.cpp
maketree.o             : setting.h maketree.cpp
scurve.o               : setting.h scurve.cpp
timing.o               : setting.h timing.cpp
timewalk_width.o       : setting.h timewalk_width.cpp
timewalk_width_allch.o : setting.h timewalk_width_allch.cpp
offset_noise_gain.o    : setting.h offset_noise_gain.cpp
linearity.o            : setting.h linearity.cpp
idac_sim.o             : setting.h idac_sim.cpp
modify_tree.o          : setting.h modify_tree.cpp
threshold_calib.o      : setting.h threshold_calib.cpp


.PHONY: clean
clean:
	rm $(shell ls *.cpp | sed "s|\.cpp|\.o|g") $(shell ls *.cpp | sed "s|\.cpp||g")
