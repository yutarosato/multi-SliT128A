#! /bin/tcsh -f

set BOARD = 2;
set CHIP  = 0;
set INFILE = `ls board2/root_data/scurve_board${BOARD}_chip${CHIP}_*.root`
echo ${INFILE}

################################################

make threshold_calib || exit
./threshold_calib ${INFILE}
