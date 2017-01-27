#! /bin/tcsh -f


#set INFILE = `ls dat_scurve/output*.dat` # default
set INFILE = `ls dat_scurve/test*.dat` # default
#set INFILE = `ls ../store/20161006_wire/dat_scurve/output*.dat`

#set CH_LIST = `seq 0 127`
#set CH_LIST = `seq 122 124`
set CH_LIST = "0"

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
  #set TABLE_BASE_CH = "${TABLE_BASE}_obsch${CH}_tpch${CH}"
  #grep "OBSCH${CH}TPCH${CH}HOGE" ${TABLE} | sed "s|OBSCH${CH}TPCH${CH}HOGE||g" > ${TABLE_BASE_CH}.dat
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
