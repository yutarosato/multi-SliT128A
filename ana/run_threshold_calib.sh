#! /bin/tcsh -f

#set INFILE = `ls pic/scurve_obsch*.root`
#set INFILE = `ls ../data/20170128_multi_scurve_after_connection_sensor/dat_scurve/scurve_obsch0[2478]*.root`
set INFILE = `ls ../data/20170323_all_chip_scan/pic_chip3/scurve_obsch*.root`

################################################

make threshold_calib || exit
./threshold_calib ${INFILE}
