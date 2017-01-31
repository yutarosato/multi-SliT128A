#! /bin/tcsh -f

set INFILE = `ls dat_scurve/output*.dat` # default
#set INFILE = `ls dat_scurve/test*.dat`

#set INFILE = `ls dat_scurve/output{21,27,28,29,30,31}*.dat`
#set INFILE = `ls dat_scurve/output1[34567]*.dat`
#set INFILE = `ls dat_scurve/ch0/output*.dat`
#set INFILE = `ls dat_scurve/ch1/output*.dat`
#set INFILE = `ls ../store/20161006_wire/dat_scurve/output*.dat`

#set CH_LIST = `seq 0 127`
#set CH_LIST = `seq 122 124`
#set CH_LIST = "0"
set CH_LIST = "0 1 2 3 4 5"
#set CH_LIST = "15"
#if( $#argv < 1 )then
#    echo " Usage : $0 [ch]"
#    echo "Example: $0  127"
#  exit 1
#endif
#set CH_LIST = $1

################################################

make maketree  || exit
make scurve    || exit

set TABLE      = "dat_scurve/scurve.dat"
set TABLE_DIR  = `dirname ${TABLE}`
set TABLE_BASE = ${TABLE_DIR}/`basename ${TABLE} .dat`

cat ${INFILE} | grep -v "nan" > ${TABLE}
wc -l ${INFILE}
wc -l ${TABLE}


foreach CH( ${CH_LIST} )
  set CH_NAME = `printf "%03d" $CH`
  set TABLE_BASE_CH = "${TABLE_BASE}_obsch${CH_NAME}_tpch${CH_NAME}"
  grep "OBSCH${CH_NAME}TPCH${CH_NAME}HOGE" ${TABLE} | sed "s|OBSCH${CH_NAME}TPCH${CH_NAME}HOGE||g" > ${TABLE_BASE_CH}.dat
  cat ${TABLE_BASE_CH}.dat | awk '{print $2" "$3}' | sort | uniq | nl -v 0 > ${TABLE_BASE_CH}_tab.dat
  cat ${TABLE_BASE_CH}.dat | awk '{print $2}'      | sort | uniq | nl -v 0 > ${TABLE_BASE_CH}_tab_vref.dat
  cat ${TABLE_BASE_CH}.dat | awk '{print $3}'      | sort | uniq | nl -v 0 > ${TABLE_BASE_CH}_tab_tpchg.dat
  ./maketree ${TABLE_BASE_CH}.dat ${TABLE_BASE_CH}_tab.dat ${TABLE_BASE_CH}_tab_vref.dat ${TABLE_BASE_CH}_tab_tpchg.dat
  rm -f ${TABLE_BASE_CH}.dat
  rm -f ${TABLE_BASE_CH}_tab.dat
  rm -f ${TABLE_BASE_CH}_tab_vref.dat
  rm -f ${TABLE_BASE_CH}_tab_tpchg.dat
  ./scurve root_data/`basename ${TABLE_BASE_CH}`.root ${CH}
end

set HEAD_BASE = `basename ${TABLE} .dat`
(cat pic/${HEAD_BASE}_*_can1_*.ps > pic/${HEAD_BASE}_can1.ps) && ps2pdf pic/${HEAD_BASE}_can1.ps pic/${HEAD_BASE}_can1.pdf  && rm -f pic/${HEAD_BASE}_*can1*.ps
(cat pic/${HEAD_BASE}_*_can2.ps   > pic/${HEAD_BASE}_can2.ps) && ps2pdf pic/${HEAD_BASE}_can2.ps pic/${HEAD_BASE}_can2.pdf  && rm -f pic/${HEAD_BASE}_*can2*.ps
