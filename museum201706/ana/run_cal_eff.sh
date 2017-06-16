#! /bin/tcsh -f

set LOCAL = "1" # "0" if you use batch ques in KEKCC, 
set FILE_LIST = `ls ../data/scurve_data/output01_*.root`

set BOARD_LIST   = "2"
#set BOARD_LIST   = "5"
#set BOARD_LIST   = "2 5"
set CHIP_LIST    = "0 1 2 3" # 0-3
#set CHANNEL_LIST = "15"
#set CHANNEL_LIST = "0"
set CHANNEL_LIST  = `seq 0 127`

set DAC_LIST  = `seq -31 31`
#set DAC_LIST  = "-17"

################################################################################################

make cal_eff  || exit

mkdir -p "dat_scurve"
mkdir -p "pic"

foreach FILE( ${FILE_LIST} )
   set HEADER = `basename ${FILE} .root`
   foreach BOARD  ( ${BOARD_LIST}   )
   foreach CHIP   ( ${CHIP_LIST}    )
   foreach CHANNEL( ${CHANNEL_LIST} )
      set CHANNEL_NAME = `printf "%03d" ${CHANNEL}`
      set OUTDAT = "dat_scurve/${HEADER}_${BOARD}_${CHIP}_${CHANNEL_NAME}.dat"
      rm -f ${OUTDAT}

      if( $LOCAL )then # 1(local) 0(batch jobs)
         foreach DAC( ${DAC_LIST} )
            ./cal_eff ${FILE} ${BOARD} ${CHIP} ${CHANNEL} ${DAC} | tee -a ${OUTDAT}
         end
	 set OUTPIC = ${HEADER}_board${BOARD}_chip${CHIP}_channel${CHANNEL_NAME}
         (cat pic/${OUTPIC}_dac*_can1.ps > pic/${OUTPIC}_can1.ps) && ps2pdf     pic/${OUTPIC}_can1.ps pic/${OUTPIC}_can1.pdf  && rm -f pic/${OUTPIC}_*can1.ps
         (cat pic/${OUTPIC}_dac*_can2.ps > pic/${OUTPIC}_can2.ps) && ps2pdf     pic/${OUTPIC}_can2.ps pic/${OUTPIC}_can2.pdf  && rm -f pic/${OUTPIC}_*can2.ps
         ls   pic/${OUTPIC}_dac*.root                              | xargs hadd pic/${OUTPIC}.root                            && rm -f pic/${OUTPIC}_dac*.root
      else
         #bsub -q s ./exe_cal_eff_sub.sh ${FILE} ${CHIP} ${CHANNEL} ${CHANNEL} ${OUTDAT} ${NAME}
         #echo "./exe_cal_eff_sub.sh ${FILE} ${CHIP} ${CHANNEL} ${CHANNEL} ${OUTDAT} ${NAME}" >> tmp.list
      endif

   end # CHANNEL LOOP
   end # CHIP LOOP
   end # BOARD LOOP
end # FILE LOOP
