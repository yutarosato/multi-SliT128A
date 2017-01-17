#! /bin/tcsh -f

set INFILE = `ls pic/scurve_obsch*.root`
#set INFILE = `ls pic/scurve_obsch0[2478]*.root`

################################################

make threshold_calib || exit
./threshold_calib ${INFILE}
