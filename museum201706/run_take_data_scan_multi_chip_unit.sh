#! /bin/tcsh -f

set HEADNAME     = "output"
#set BOARD_LIST   = "2" # (2,5), (3,6)
#set CHIP_LIST    = "0 1 3" # 0-3
set BOARD_LIST   = "5" # (2,5), (3,6)
set CHIP_LIST    = "1 2 3" # 0-3

#set CHIP_LIST    = "0 1 2 3" # 0-3
#set CHANNEL_LIST = "12"
#set CHANNEL_LIST = `seq 0 31`
set CHANNEL_LIST = $1
#set VREF         = 300.0 # VREF value [mV]
set VREF         = 400.0 # VREF value [mV]
#set TPCHG        = 1.92 # Test pulse charge [fC] : 3.84 fC = 38.4 mV * 100fF (@1MIP)
set TPCHG        = 2.30 # Test pulse charge [fC] : 3.84 fC = 38.4 mV * 100fF (@1MIP)
# 1 MIP = 279 mV @ 8 divider

###########################################
(cd slow_control;   (make || exit;))
(cd exp_decoder;    (make || exit;))
(cd readslit-0.0.0; (make || exit;))

foreach CHANNEL( ${CHANNEL_LIST} )
    set CNT=0
    set CTRL_DAC = -31
    while ( ${CTRL_DAC} <= 31 )
    #set CTRL_DAC = 13
    #while ( ${CTRL_DAC} <= 14 )
	set TMP_DAC = `echo "obase=2; ibase=10; ${CTRL_DAC}" | bc | sed 's|-||'`
	if( ${CTRL_DAC} < 1 ) then
	    set CTRL_DAC_BIT = `printf "L%05d\n" ${TMP_DAC} | sed 's|0|L|g' | sed 's|1|H|g'`
	else
	    set CTRL_DAC_BIT = `printf "H%05d\n" ${TMP_DAC} | sed 's|0|L|g' | sed 's|1|H|g'`
	endif
	echo "   DAC : ${CTRL_DAC} => ${CTRL_DAC_BIT}"

	# <Slow Control>
	cd slow_control;
	foreach BOARD  ( ${BOARD_LIST}   )
	foreach CHIP   ( ${CHIP_LIST}    )
	    set TMP_CHIP  = `echo "obase=2; ibase=10; ${CHIP}" | bc`
	    set TMP_BOARD = `echo "obase=2; ibase=10; ${BOARD}" | bc`
	    set CTRL_CHIP = `printf "%04d%03d" ${TMP_BOARD} ${TMP_CHIP}`
	    set IP = 16
	    @ IP += ${BOARD}
	    #./make_control ${BOARD} ${CTRL_CHIP} ${CHANNEL} LLLLL${CTRL_DAC_BIT}LLHLH LLLLLLLLLLLLLLLL # other DAC = 0 # default (last 3 bits are digital-output/analog-monitor/test-pulse-in)
	    ./make_control_unit ${BOARD} ${CTRL_CHIP} ${CHANNEL} LLLLL${CTRL_DAC_BIT}LLHLH LLLLLLLLLLLLLLLL # other DAC = 0 # default (last 3 bits are digital-output/analog-monitor/test-pulse-in)
	    while (1)
		./slit128sc_chip  files/control_${BOARD}_${CTRL_CHIP}.dat 192.168.${BOARD}.${IP};
		if( $? == 0 ) then
		    break
		endif
	    end
	    echo "      SLOWCONTROL : Board#${BOARD}, Chip#${CHIP}"
	end # end loop for CHIP
	end # end loop for BOARD
	cd ../;

	foreach BOARD( ${BOARD_LIST} )
	    ./run_fpga.sh ${BOARD}
	end # end loop for BOARD

	# <Take Data>
        echo "               Take Data"
	mkdir -p binary_data
	set PID = -999
	foreach BOARD( ${BOARD_LIST} )
	    echo "                   Board#${BOARD}"
	    set IP = 16
	    @ IP += ${BOARD}
	    set OUTNAME = "${HEADNAME}_${VREF}_${TPCHG}_${BOARD}_${CHANNEL}_${CTRL_DAC}"
	    #nc -d 192.168.${BOARD}.${IP} 24 > binary_data/${OUTNAME}.dat &
	    #set PID = (${PID} $!)
	    cd readslit-0.0.0/;
	    ./readslit -t 2 192.168.${BOARD}.${IP} 24 ../binary_data/${OUTNAME}.dat &
	    cd ../
	end # end loop for BOARD
	sleep 2;
	#sleep 3;
        #echo "               Kill Process(nc)"
	#foreach IPID( ${PID} )
	#    if( ${IPID} < 0 ) then
	#	continue;
	#    endif
	#    kill -9 ${IPID}
	#end # end loop for PID

	# <Decode>
	    echo "               Decode Data"
	mkdir -p root_data
	mkdir -p root_data/tmp
	cd exp_decoder;
	set NOHIT = 0
	foreach BOARD( ${BOARD_LIST} )
	    echo "                   Board#${BOARD}"
	    set OUTNAME = "${HEADNAME}_${VREF}_${TPCHG}_${BOARD}_${CHANNEL}_${CTRL_DAC}"
	    echo "./multi-slit128a_exp_decoder_for_scurve ../root_data/${OUTNAME}.root ../binary_data/${OUTNAME}.dat ${VREF} ${TPCHG} ${BOARD} -999 -999 ${CTRL_DAC}" >> tmp.sh
	    set TMPNOHIT = 1
	    ./multi-slit128a_exp_decoder_for_scurve ../root_data/${OUTNAME}.root ../binary_data/${OUTNAME}.dat ${VREF} ${TPCHG} ${BOARD} -999 -999 ${CTRL_DAC}
	    set TMPNOHIT = $?
	    @ NOHIT += ${TMPNOHIT};
	    #rm -f ../binary_data/${OUTNAME}.dat
	end # end loop for BOARD

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
	#set NOHIT = `cat tmp_nhit.log`
	set NOHIT = 10000000;
	echo ${NOHIT}
	cd ../
	if( ${NOHIT} >= 35000 )then
	    @ CTRL_DAC += 4
	else
	    @ CTRL_DAC += 4 # tmpppp
	endif
    end # end loop for DAC
end # end loop for Channel

#echo "hadd -f root_data/${HEADNAME}_${VREF}_${TPCHG}_${BOARD_LIST}_${CHANNEL_LIST}.root root_data/${HEADNAME}_${VREF}_${TPCHG}_${BOARD_LIST}_${CHANNEL_LIST}_*.root && mv root_data/${HEADNAME}_${VREF}_${TPCHG}_${BOARD_LIST}_${CHANNEL_LIST}_*.root root_data/tmp/." >> tmp.sh
hadd -f root_data/${HEADNAME}_${VREF}_${TPCHG}_${BOARD_LIST}_${CHANNEL_LIST}.root root_data/${HEADNAME}_${VREF}_${TPCHG}_${BOARD_LIST}_${CHANNEL_LIST}_*.root && mv root_data/${HEADNAME}_${VREF}_${TPCHG}_${BOARD_LIST}_${CHANNEL_LIST}_*.root root_data/tmp/.
