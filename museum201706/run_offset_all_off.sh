#! /bin/tcsh -f

#set BOARD_LIST = "2 3 5 6"
set BOARD_LIST = "2" # tmppp
set CHIP_LIST  = "0 1 2 3"

set CHANNEL  = 0
set CTRL_DAC = 0

(cd slow_control; make || exit)

set TMP = `echo "obase=2; ibase=10; ${CTRL_DAC}" | bc | sed 's|-||'`
      if( ${CTRL_DAC} < 1 ) then
        set CTRL_DAC_BIT = `printf "L%05d\n" ${TMP} | sed 's|0|L|g' | sed 's|1|H|g'`
      else
        set CTRL_DAC_BIT = `printf "H%05d\n" ${TMP} | sed 's|0|L|g' | sed 's|1|H|g'`
      endif
echo "   DAC : ${CTRL_DAC} => ${CTRL_DAC_BIT}"

cd slow_control;
foreach IBOARD( ${BOARD_LIST} )
    set IP = 16
    @ IP += ${IBOARD}
    echo "Board#${IBOARD}, IP=192.168.10.${IP}"
    foreach ICHIP( ${CHIP_LIST} )
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
        ./make_control ${IBOARD} ${CTRL_CHIP} ${CHANNEL} LLLLL${CTRL_DAC_BIT}LLLLL LLLLL${CTRL_DAC_BIT}LLLLL # default (all off)
	
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
