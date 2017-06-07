#! /bin/tcsh -f

if( $#argv < 2 )then
    echo " Usage : $0 [channel][DAC]"
    echo "Example: $0     11     31"
    echo "         channel : 0~127"
    echo "         DAC     : -31~31"
  exit 1
endif

set CHANNEL  = $1
set CTRL_DAC = $2

#set BOARD_LIST = "2 3 5 6"
set BOARD_LIST = "2 5" # tmppp
#set BOARD_LIST = "2" # tmppp
set CHIP_LIST  = "0 1 2 3"


(cd slow_control; make || exit)

set TMP_DAC = `echo "obase=2; ibase=10; ${CTRL_DAC}" | bc | sed 's|-||'`
if( ${CTRL_DAC} < 1 ) then
    set CTRL_DAC_BIT = `printf "L%05d\n" ${TMP_DAC} | sed 's|0|L|g' | sed 's|1|H|g'`
else
    set CTRL_DAC_BIT = `printf "H%05d\n" ${TMP_DAC} | sed 's|0|L|g' | sed 's|1|H|g'`
endif
echo "   DAC : ${CTRL_DAC} => ${CTRL_DAC_BIT}"

cd slow_control;
foreach IBOARD( ${BOARD_LIST} )
    set IP = 16
    @ IP += ${IBOARD}
    echo "Board#${IBOARD}, IP=192.168.${IBOARD}.${IP}"
    foreach ICHIP( ${CHIP_LIST} )
	set TMP_CHIP  = `echo "obase=2; ibase=10; ${ICHIP}" | bc`
	set TMP_BOARD = `echo "obase=2; ibase=10; ${IBOARD}" | bc`
	set CTRL_CHIP = `printf "%04d%03d" ${TMP_BOARD} ${TMP_CHIP}`
	echo "    Chip#${ICHIP} (${CTRL_CHIP})"
	# <Slow Control>
        ./make_control ${IBOARD} ${CTRL_CHIP} ${CHANNEL} LLLLL${CTRL_DAC_BIT}LLHLH LLLLL${CTRL_DAC_BIT}LLLLL # digital output(ON)
	
	while (1)
	    ./slit128sc_chip  files/control_${IBOARD}_${CTRL_CHIP}.dat 192.168.${IBOARD}.${IP};
	    if( $? == 0 ) then
		break
	    endif
	end
	#sleep 0.001
    end #end chip-loop
end #end board-loop
cd  ../

foreach IBOARD( ${BOARD_LIST} )
    ./run_fpga.sh ${IBOARD}
end
