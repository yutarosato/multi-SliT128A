#! /bin/tcsh -f

set HEADNAME = "output"
set BOARD    = 5 # (2,5), (3,6)
set CHIP     = 1 # 0-3
set VREF     = 450.0 # VREF value [mV]
set TPCHG    = 1.92 # Test pulse charge [fC] : 3.84 fC = 38.4 mV * 100fF (@1MIP)

set CHANNEL_LIST = 15
#set CHANNEL_LIST = `seq 122 124`
#set CHANNEL_LIST = `seq 0 127`

###########################################
(cd slow_control;   (make || exit;))
(cd exp_decoder;    (make || exit;))
(cd readslit-0.0.0; (make || exit;))
./run_offset_all_off.sh

set TMP_CHIP  = `echo "obase=2; ibase=10; ${CHIP}" | bc`
set TMP_BOARD = `echo "obase=2; ibase=10; ${BOARD}" | bc`
set CTRL_CHIP = `printf "%04d%03d" ${TMP_BOARD} ${TMP_CHIP}`

set IP = 16
@ IP += ${BOARD}

foreach CHANNEL( ${CHANNEL_LIST} )
   echo "START S-Curve scan for Board#${BOARD} Channel#${CHANNEL}"
   set CNT=0
   set CTRL_DAC = -31
   while ( ${CTRL_DAC} <= 31 )
      set TMP_DAC = `echo "obase=2; ibase=10; ${CTRL_DAC}" | bc | sed 's|-||'`
      if( ${CTRL_DAC} < 1 ) then
         set CTRL_DAC_BIT = `printf "L%05d\n" ${TMP_DAC} | sed 's|0|L|g' | sed 's|1|H|g'`
      else
         set CTRL_DAC_BIT = `printf "H%05d\n" ${TMP_DAC} | sed 's|0|L|g' | sed 's|1|H|g'`
      endif
      echo "   DAC : ${CTRL_DAC} => ${CTRL_DAC_BIT}"

      # <Slow Control>
      cd slow_control;
      ./make_control ${BOARD} ${CTRL_CHIP} ${CHANNEL} LLLLL${CTRL_DAC_BIT}LLHLH LLLLLLLLLLLLLLLL # other DAC = 0 # default (last 3 bits are digital-output/analog-monitor/test-pulse-in)

      while (1)
	 ./slit128sc_chip  files/control_${BOARD}_${CTRL_CHIP}.dat 192.168.${BOARD}.${IP};
         if( $? == 0 ) then
	    break
         endif
      end
      cd ../
      ./run_fpga.sh ${BOARD}

      # <Take Data>
      set OUTNAME = "${HEADNAME}_${VREF}_${TPCHG}_${BOARD}_${CHIP}_${CHANNEL}_${CTRL_DAC}"
      #mkdir -p binary_data
      #nc -d 192.168.${BOARD}.${IP} 24 > binary_data/${OUTNAME}.dat &
      #sleep 3
      #kill -9 $!
      cd readslit-0.0.0/;
      ./readslit -t 3 192.168.${BOARD}.${IP} 24 ../binary_data/${OUTNAME}.dat
      cd ../

      # <Decode>
      mkdir -p root_data
      mkdir -p root_data/tmp

      cd exp_decoder;
      ./multi-slit128a_exp_decoder_for_scurve ../root_data/${OUTNAME}.root ../binary_data/${OUTNAME}.dat ${VREF} ${TPCHG} ${BOARD} ${CHIP} ${CHANNEL} ${CTRL_DAC}
      set NOHIT = $?
      rm -f ../binary_data/${OUTNAME}.dat
      
      if( ${NOHIT} == 0 )then
         @ CNT = 0
      else
         @ CNT += 1
      endif
      if( ${CNT} == 2 )then
         echo "FINISH"
         cd ../
         break;
      endif

      # skip at early-point
      set NOHIT = `cat tmp_nhit.log`
      echo ${NOHIT}
      cd ../
      if( ${NOHIT} >= 35000 )then
        @ CTRL_DAC += 3
      else
        @ CTRL_DAC += 3 # tmpppp
      endif

   end # DAC LOOP
end # CHANNEL LOOP

hadd -f root_data/${HEADNAME}_${VREF}_${TPCHG}_${BOARD}_${CHIP}.root root_data/${HEADNAME}_${VREF}_${TPCHG}_${BOARD}_${CHIP}_*.root && mv root_data/${HEADNAME}_${VREF}_${TPCHG}_${BOARD}_${CHIP}_*.root root_data/tmp/.

