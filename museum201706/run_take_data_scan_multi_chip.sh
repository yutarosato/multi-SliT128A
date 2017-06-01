#! /bin/tcsh -f

set HEADNAME = "output"
set BOARD    = 0 # (2,5), (3,6)

set VREF     = 170.0 # VREF value [mV]
set TPCHG    = 1.15 # Test pulse charge [fC] : 3.84 fC = 38.4 mV * 100fF (@1MIP)

set CHIP_LIST     = "0 1 2 3" # 0-3

set CHANNEL_LIST = 1 
#set CHANNEL_LIST = `seq 122 124`
#set CHANNEL_LIST = `seq 0 127`

set CTRL_DAC_LIST = `seq -31 31`
#set CTRL_DAC_LIST = `seq 28 31`
#set CTRL_DAC_LIST = 31

###########################################
(cd slow_control; make || exit;)
(cd exp_decoder;  make || exit;)
./run_offset_all_off.sh

foreach CHANNEL( ${CHANNEL_LIST} )
   echo "START S-Curve scan for Board#${BOARD} Channel#${CHANNEL}"
   set CNT=0
   foreach CTRL_DAC( ${CTRL_DAC_LIST} )
      set TMP_DAC = `echo "obase=2; ibase=10; ${CTRL_DAC}" | bc | sed 's|-||'`
      if( ${CTRL_DAC} < 1 ) then
         set CTRL_DAC_BIT = `printf "L%05d\n" ${TMP_DAC} | sed 's|0|L|g' | sed 's|1|H|g'`
      else
         set CTRL_DAC_BIT = `printf "H%05d\n" ${TMP_DAC} | sed 's|0|L|g' | sed 's|1|H|g'`
      endif
      echo "   DAC : ${CTRL_DAC} => ${CTRL_DAC_BIT}"

      foreach CHIP( ${CHIP_LIST} )
	set TMP_CHIP  = `echo "obase=2; ibase=10; ${CHIP}" | bc`
	set TMP_BOARD = `echo "obase=2; ibase=10; ${BOARD}" | bc`
	set CTRL_CHIP = `printf "%04d%03d" ${TMP_BOARD} ${TMP_CHIP}`
	# <Slow Control>
	cd slow_control;
	./make_control ${CTRL_CHIP} $CHANNEL LLLLL${CTRL_DAC_BIT}LLHLH LLLLLLLLLLLLLLLL # other DAC = 0 # default (last 3 bits are digital-output/analog-monitor/test-pulse-in)
	while (1)
	    ./slit128sc_chip  files/control_${BOARD}_${CTRL_CHIP}.dat 192.168.${BOARD}.${IP};
	    if( $? == 0 ) then
		break
	    endif
	end
      end # end loot for CHIP

      set OUTNAME = "${HEADNAME}_${VREF}_${TPCHG}_${BOARD}_${CHANNEL}_${CTRL_DAC}"
      # <Take Data>
      cd ../;
      mkdir -p binary_data
      nc -d 192.168.${BOARD}.${IP} > binary_data/${OUTNAME}.dat &
      #sleep 3
      sleep 4
      kill -9 $!

      # <Decode>
      mkdir -p root_data
      mkdir -p root_data/tmp

      cd exp_decoder;
      ./multi-slit128da_exp_decoder ../root_data/${OUTNAME}.root ../binary_data/${OUTNAME}.dat ${VREF} ${TPCHG} ${CHANNEL} ${CTRL_DAC}
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
      cd ../

   end # DAC LOOP
end # CHANNEL LOOP

hadd -f root_data/${HEADNAME}_${VREF}_${TPCHG}_${BOARD}.root root_data/${HEADNAME}_${VREF}_${TPCHG}_${BOARD}_*.root && mv root_data/${HEADNAME}_${VREF}_${TPCHG}_${BOARD}_*.root root_data/tmp/.

