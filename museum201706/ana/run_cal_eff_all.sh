#! /bin/tcsh -f

#set FILE_LIST = `ls ../root_data/*.root`
set FILE_LIST = `ls ../data/scurve_data/output01_*.root`

set BOARD_LIST   = "2"
#set BOARD_LIST   = "2 5"
set DAC_LIST  = `seq -31 31`
#set DAC_LIST  = -31

set LOCAL = "1" # "0" if you use batch ques in KEKCC, 
################################################################################################

make cal_eff_all  || exit

mkdir -p "dat_scurve"
mkdir -p "pic"

foreach FILE( ${FILE_LIST} )
   set HEADER = `basename ${FILE} .root`
   foreach BOARD  ( ${BOARD_LIST}   )
   echo "*****************BOARD#${BOARD}*****************"
      #set OUTDAT = "dat_scurve/${HEADER}_${BOARD}_${CHIP}_${CHANNEL_NAME}.dat"
      #rm -f ${OUTDAT}

      if( $LOCAL )then # 1(local) 0(batch jobs)
         foreach DAC( ${DAC_LIST} )
	    echo "   DAC = ${DAC}"
            ./cal_eff_all ${FILE} ${BOARD} ${DAC}
         end # DAC LOOP
         #(cat pic/${HEADER}_board${BOARD}_*_dac*_can1.ps > pic/${HEADER}}_board${BOARD}_can1.ps) && ps2pdf pic/${HEADER}_board${BOARD}_can1.ps pic/${HEADER}_board${BOARD}_can1.pdf  && rm -f pic/${HEADER}_board${BOARD}_*can1.ps
         #(cat pic/${HEADER}_board${BOARD}_*_dac*_can2.ps > pic/${HEADER}}_board${BOARD}_can2.ps) && ps2pdf pic/${HEADER}_board${BOARD}_can2.ps pic/${HEADER}_board${BOARD}_can2.pdf  && rm -f pic/${HEADER}_board${BOARD}_*can2.ps
         #ls   pic/${HEADER}_board${BOARD}_*_dac*.root | xargs hadd pic/${HEADER}_board${BOARD}.root && rm -f pic/${HEADER}_board${BOARD}_*_dac*.root
      else
         #bsub -q s ./exe_cal_eff_sub.sh ${FILE} ${CHIP} ${CHANNEL} ${CHANNEL} ${OUTDAT} ${NAME}
         #echo "./exe_cal_eff_sub.sh ${FILE} ${CHIP} ${CHANNEL} ${CHANNEL} ${OUTDAT} ${NAME}" >> tmp.list
      endif
   end # BOARD LOOP
end # FILE LOOP
