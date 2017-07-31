CXX = g++

ROOTFLAGS = `root-config --cflags`
ROOTLIBS  = `root-config --libs`
BIN_DIR   =../bin
all: $(shell ls *.cpp | sed "s|\.cpp||g")

s_curve:s_curve.o
	$(CXX) $^ $(ROOTLIBS) -o $@
	mv $@ ${BIN_DIR}
threshold_scan: threshold_scan.o
	$(CXX) $^ $(ROOTLIBS) -o $@
	mv $@ ${BIN_DIR}
comparison: comparison.o
	$(CXX) $^ $(ROOTLIBS) -o $@
	mv $@ ${BIN_DIR}
noise_occupancy: noise_occupancy.o
	$(CXX) $^ $(ROOTLIBS) -o $@
	mv $@ ${BIN_DIR}
cluster: cluster.o
	$(CXX) $^ $(ROOTLIBS) -o $@
	mv $@ ${BIN_DIR}
check_coincidence: check_coincidence.o
	$(CXX) $^ $(ROOTLIBS) -o $@
	mv $@ ${BIN_DIR}
lt_vs_tot: lt_vs_tot.o
	$(CXX) $^ $(ROOTLIBS) -o $@
	mv $@ ${BIN_DIR}
event_checker: event_checker.o
	$(CXX) $^ $(ROOTLIBS) -o $@
	mv $@ ${BIN_DIR}
.cpp.o:
	$(CXX) $(ROOTFLAGS) -c $< -o $@

qc_allch.o             : setting.h qc_allch.cpp
cal_eff.o              : setting.h cal_eff.cpp

.PHONY: clean
clean:
	rm $(shell ls *.cpp | sed "s|\.cpp|\.o|g")
	rm *~
