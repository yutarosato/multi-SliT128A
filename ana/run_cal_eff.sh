#! /bin/tcsh -f

set LOCAL = "0" # "0" if you use batch ques in KEKCC, 
set CHIP  = "0" # 0-3

set FILE_LIST = `ls ../root_data/output*.root` # default
#set FILE_LIST = `ls ../store/20161017_bias/root_data/output*.root`

set CH_LIST   = `seq 0 127`
#set CH_LIST   = `seq 122 124`
#set CH_LIST   = "1 10 100"
#set CH_LIST   = "123"

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
