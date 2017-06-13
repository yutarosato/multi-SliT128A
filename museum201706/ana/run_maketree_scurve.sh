#! /bin/tcsh -f

set INFILE = `ls dat_scurve/output*.dat` # default

#set BOARD_LIST = 2;
#set CHIP_LIST  = 0;
#set CHANNEL_LIST = `seq 0 127`
#set CHANNEL_LIST = "1"

if( $#argv < 3 )then
    echo " Usage : $0 [board] [chip] [channel]"
    echo "Example: $0   2       3       127"
  exit 1
endif
set BOARD_LIST   = $1
set CHIP_LIST    = $2
set CHANNEL_LIST = $3

################################################

make maketree  || exit
make scurve    || exit

set TABLE      = "dat_scurve/scurve.dat"
set TABLE_DIR  = `dirname ${TABLE}`
set TABLE_BASE = ${TABLE_DIR}/`basename ${TABLE} .dat`

cat ${INFILE} | grep -v "nan" > ${TABLE}
wc -l ${INFILE}
wc -l ${TABLE}


foreach BOARD( ${BOARD_LIST} )
foreach CHIP( ${CHIP_LIST} )
foreach CHANNEL( ${CHANNEL_LIST} )
  set CHANNEL_NAME = `printf "%03d" $CHANNEL`
  set TABLE_BASE_CHANNEL = "${TABLE_BASE}_board${BOARD}_chip${CHIP}_channel${CHANNEL_NAME}"
  grep "BOARD${BOARD} CHIP${CHIP} CHANNEL${CHANNEL}HOGEEEE" ${TABLE} | sed "s|BOARD${BOARD} CHIP${CHIP} CHANNEL${CHANNEL}HOGEEEE||g" > ${TABLE_BASE_CHANNEL}.dat
  cat ${TABLE_BASE_CHANNEL}.dat | awk '{print $2" "$3}' | sort | uniq | nl -v 0 > ${TABLE_BASE_CHANNEL}_tab.dat
  cat ${TABLE_BASE_CHANNEL}.dat | awk '{print $2}'      | sort | uniq | nl -v 0 > ${TABLE_BASE_CHANNEL}_tab_vref.dat
  cat ${TABLE_BASE_CHANNEL}.dat | awk '{print $3}'      | sort | uniq | nl -v 0 > ${TABLE_BASE_CHANNEL}_tab_tpchg.dat
  ./maketree ${TABLE_BASE_CHANNEL}.dat ${TABLE_BASE_CHANNEL}_tab.dat ${TABLE_BASE_CHANNEL}_tab_vref.dat ${TABLE_BASE_CHANNEL}_tab_tpchg.dat
  rm -f ${TABLE_BASE_CHANNEL}.dat
  rm -f ${TABLE_BASE_CHANNEL}_tab.dat
  rm -f ${TABLE_BASE_CHANNEL}_tab_vref.dat
  rm -f ${TABLE_BASE_CHANNEL}_tab_tpchg.dat
  ./scurve root_data/`basename ${TABLE_BASE_CHANNEL}`.root ${CHANNEL}
end
end
end

#set HEAD_BASE = `basename ${TABLE} .dat`
#(cat pic/${HEAD_BASE}_*_can1_*.ps > pic/${HEAD_BASE}_can1.ps) && ps2pdf pic/${HEAD_BASE}_can1.ps pic/${HEAD_BASE}_can1.pdf  && rm -f pic/${HEAD_BASE}_*can1*.ps
#(cat pic/${HEAD_BASE}_*_can2.ps   > pic/${HEAD_BASE}_can2.ps) && ps2pdf pic/${HEAD_BASE}_can2.ps pic/${HEAD_BASE}_can2.pdf  && rm -f pic/${HEAD_BASE}_*can2*.ps
