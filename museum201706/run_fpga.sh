#! /bin/tcsh -f

if( $#argv < 1 )then
    echo " Usage : $0 [board]"
    echo "Example: $0   0"
    echo "         board   : 2-5 & 3-6 (MuSEUM BT@201706)"
  exit 1
endif

set BOARD    = $1

cd slow_control;
set IP = 16
@ IP += ${BOARD}
while (1)
    ./slit128sc_begin  192.168.${BOARD}.${IP};
    if( $? == 0 ) then
	break
    endif
end
while (1)
    ./slit128sc_fpga  192.168.${BOARD}.${IP};
    if( $? == 0 ) then
	break
    endif
end
while (1)
    ./slit128sc_check_status  192.168.${BOARD}.${IP};
    if( $? == 0 ) then
	break
    endif
end
while (1)
    ./slit128sc_end  192.168.${BOARD}.${IP};
    if( $? == 0 ) then
	break
    endif
end
cd  ../
