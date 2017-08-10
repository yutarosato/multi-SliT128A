#! /bin/tcsh -f

set BOARD = "5";
 
################################################

make tot_width    || exit

set FILE_LIST = `ls pic/output*.root`
./tot_width ${BOARD} ${FILE_LIST}
