#! /bin/tcsh -f

if( $#argv < 3 )then
    echo " Usage : $0 [board] [chip] [channel] [DAC]"
    echo "Example: $0   0       0       60       31"
    echo "         board   : 2-5 & 3-6 (MuSEUM BT@201706)"
    echo "         chip    : 0~3"
    echo "         channel : 0~127"
    echo "         DAC     : -31~31"
  exit 1
endif

set BOARD    = $1
set CHIP     = $2
set CHANNEL  = $3
set CTRL_DAC = $4

set BOARD_LIST = "2 3 5 6"
set CHIP_LIST  = "0 1 2 3"

(cd slow_control; make || exit)
#./run_offset_all_off.sh # tmppp

set TMP = `echo "obase=2; ibase=10; ${CTRL_DAC}" | bc | sed 's|-||'`
      if( ${CTRL_DAC} < 1 ) then
        set CTRL_DAC_BIT = `printf "L%05d\n" ${TMP} | sed 's|0|L|g' | sed 's|1|H|g'`
      else
        set CTRL_DAC_BIT = `printf "H%05d\n" ${TMP} | sed 's|0|L|g' | sed 's|1|H|g'`
      endif
echo "   DAC : ${CTRL_DAC} => ${CTRL_DAC_BIT}"

cd slow_control;
#foreach IBOARD( ${BOARD_LIST} )
foreach IBOARD( ${BOARD} ) # tmppp
    set IP = 16
    @ IP += ${IBOARD}
    echo "Board#${IBOARD}, IP=192.168.10.${IP}"
    #foreach ICHIP( 0 1 2 3 )
    #foreach ICHIP( 0 ) # tmppp
    foreach ICHIP( ${CHIP} ) # tmppp
	if( ${ICHIP} == 0 ) then
	    set CTRL_CHIP = "0000000"
	else if( ${ICHIP} == 1 ) then
	    set CTRL_CHIP = "0000001"
	else if( ${ICHIP} == 2 ) then
	    set CTRL_CHIP = "0000010"
	else if( ${ICHIP} == 3 ) then
	    set CTRL_CHIP = "0000011"
	else
	    echo "Wrong CHIP-ID : ${CHIP}"
	    exit
	endif
	echo "    Chip#${ICHIP}"
	# <Slow Control>
	if( ${ICHIP} == ${CHIP} ) then
	    ./make_control ${IBOARD} ${CTRL_CHIP} ${CHANNEL} LLLLL${CTRL_DAC_BIT}LLHHH LLLLL${CTRL_DAC_BIT}LLLLL # default (last 3 bits are digital-output/analog-monitor/test-pulse-in)
	else
	    ./make_control ${IBOARD} ${CTRL_CHIP} ${CHANNEL} LLLLL${CTRL_DAC_BIT}LLLLL LLLLL${CTRL_DAC_BIT}LLLLL # default
	endif
	
	while (1)
	    ./slit128sc_chip  files/control_${IBOARD}_${CTRL_CHIP}.dat 192.168.10.${IP};
	    if( $? == 0 ) then
		break
	    endif
	end
	#sleep 0.001
    end #end chip-loop

    while (1)
	./slit128sc_begin  192.168.10.${IP};
	if( $? == 0 ) then
	    break
	endif
    end
    while (1)
	./slit128sc_fpga  192.168.10.${IP};
	if( $? == 0 ) then
	    break
	endif
    end
    while (1)
	./slit128sc_check  192.168.10.${IP};
	if( $? == 0 ) then
	    break
	endif
    end
    while (1)
	./slit128sc_end  192.168.10.${IP};
	if( $? == 0 ) then
	    break
	endif
    end
end #end board-loop
cd  ../
