#! /bin/tcsh -f

set LOCAL = "1" # "0" if you use batch ques in KEKCC, 
set CHIP  = "0" # 0-3

set FILE_LIST = `ls ../root_data/output25_*.root` # default
#set FILE_LIST = `ls ../test.root`
#set FILE_LIST = `ls ../store/20161017_bias/root_data/output*.root`

#set CH_LIST   = `seq 0 127`
#set CH_LIST   = `seq 0 13`
#set CH_LIST   = "37 59 90"
#set CH_LIST   = "24 27 29 43 47 48 49 53 55 61 64 69 70 72 80 81 82 85 87 88 89 97 98 99 104 109 119 121 122"
set CH_LIST   = "0 1" # Chip#0
#set CH_LIST   = "2 3" # Chip#1
#set CH_LIST   = "0 1 2 3 4 5"
#set CH_LIST   = "0"
#set CH_LIST   = "68 76 79"
#set CH_LIST   = "66"

set DAC_LIST  = `seq -31 31`
#set DAC_LIST  = "-17"

################################################################################################

make cal_eff  || exit

mkdir -p "dat_scurve"
mkdir -p "pic"

foreach FILE( ${FILE_LIST} )
   set NAME = `basename ${FILE} .root`
   foreach CH( ${CH_LIST} )
      set CH_NAME = `printf "%03d" $CH`
      set OUTNAME = "dat_scurve/${NAME}_${CH_NAME}.dat"
      rm -f ${OUTNAME}

      if( $LOCAL )then # 1(local) 0(batch jobs)
         foreach DAC( ${DAC_LIST} )
            ./cal_eff ${FILE} ${CHIP} ${CH} ${CH} ${DAC} | tee -a ${OUTNAME}
         end
         (cat pic/${NAME}_obsch${CH_NAME}_tpch${CH_NAME}_dac*_can1.ps > pic/${NAME}_obsch${CH_NAME}_tpch${CH_NAME}_can1.ps) && ps2pdf pic/${NAME}_obsch${CH_NAME}_tpch${CH_NAME}_can1.ps pic/${NAME}_obsch${CH_NAME}_tpch${CH_NAME}_can1.pdf  && rm -f pic/${NAME}_obsch${CH_NAME}_tpch${CH_NAME}_*can1.ps
         (cat pic/${NAME}_obsch${CH_NAME}_tpch${CH_NAME}_dac*_can2.ps > pic/${NAME}_obsch${CH_NAME}_tpch${CH_NAME}_can2.ps) && ps2pdf pic/${NAME}_obsch${CH_NAME}_tpch${CH_NAME}_can2.ps pic/${NAME}_obsch${CH_NAME}_tpch${CH_NAME}_can2.pdf  && rm -f pic/${NAME}_obsch${CH_NAME}_tpch${CH_NAME}_*can2.ps
         (ls pic/${NAME}_obsch${CH_NAME}_tpch${CH_NAME}_dac*.root | xargs hadd pic/${NAME}_obsch${CH_NAME}_tpch${CH_NAME}.root) && rm -f pic/${NAME}_obsch${CH_NAME}_tpch${CH_NAME}_dac*.root
      else
         #bsub -q s ./exe_cal_eff_sub.sh ${FILE} ${CHIP} ${CH} ${CH} ${OUTNAME} ${NAME}
         echo "./exe_cal_eff_sub.sh ${FILE} ${CHIP} ${CH} ${CH} ${OUTNAME} ${NAME}" >> tmp.list
      endif

   end # CH LOOP
end # FILE LOOP
