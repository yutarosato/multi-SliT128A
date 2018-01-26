#! /bin/tcsh -f



set BOARD_LIST = "5";
set CHIP_LIST  = "0 1 2 3";
set CHANNEL_LIST = `seq 0 127`

#if( $#argv < 1 )then
#    echo " Usage : $0 [board]"
#    echo "Example: $0   5"
#  exit 1
#endif
 
################################################

make maketree  || exit
make scurve    || exit

foreach BOARD  ( ${BOARD_LIST}   )
    #set INFILE     = `ls dat_scurve/output*_board${BOARD}_*.dat`
    set INFILE     = `ls dat_scurve/output{07,06,05}*_board${BOARD}_*.dat`
    set TABLE      = "dat_scurve/scurve_board${BOARD}.dat"
    set TABLE_DIR  = `dirname ${TABLE}`
    set TABLE_BASE = ${TABLE_DIR}/`basename ${TABLE} .dat`
    cat ${INFILE} | grep -v "nan" | grep "BOARD${BOARD}"| awk '{sub("BOARD.*",""); print$0;}'  > ${TABLE}
    wc -l ${INFILE}
    wc -l ${TABLE}

    cat ${TABLE_BASE}.dat | awk '{print $2" "$3}' | sort | uniq | nl -v 0 > ${TABLE_BASE}_tab.dat
    cat ${TABLE_BASE}.dat | awk '{print $2}'      | sort | uniq | nl -v 0 > ${TABLE_BASE}_tab_vref.dat
    cat ${TABLE_BASE}.dat | awk '{print $3}'      | sort | uniq | nl -v 0 > ${TABLE_BASE}_tab_tpchg.dat
    ./maketree ${TABLE_BASE}.dat ${TABLE_BASE}_tab.dat ${TABLE_BASE}_tab_vref.dat ${TABLE_BASE}_tab_tpchg.dat
    rm -f ${TABLE_BASE}.dat
    rm -f ${TABLE_BASE}_tab.dat
    rm -f ${TABLE_BASE}_tab_vref.dat
    rm -f ${TABLE_BASE}_tab_tpchg.dat
    ./scurve root_data/`basename ${TABLE_BASE}`.root ${BOARD}
end
