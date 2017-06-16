#! /bin/tcsh -f

#set BOARD = 2;
#set CHIP  = 1;
set BOARD = $1;
set CHIP  = $2;
#set INFILE = `ls board2/root_data/scurve_board${BOARD}_chip${CHIP}_*.root`
set INFILE = `ls pic/scurve_board${BOARD}_chip${CHIP}_*.root`

################################################

make threshold_calib || exit
./threshold_calib ${INFILE}

mv pic/threshold_calib_can0.eps  pic/threshold_calib_can0_b${BOARD}c${CHIP}.eps
mv pic/threshold_calib_can0.root pic/threshold_calib_can0_b${BOARD}c${CHIP}.root
mv pic/threshold_calib_can1.eps  pic/threshold_calib_can1_b${BOARD}c${CHIP}.eps
mv pic/threshold_calib_can1.root pic/threshold_calib_can1_b${BOARD}c${CHIP}.root
mv threshold_calib.dat threshold_calib_b${BOARD}c${CHIP}.dat
