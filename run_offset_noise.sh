#! /bin/tcsh -f

if( $#argv < 3 )then
    echo " Usage : $0 [chip] [ch] [DAC]"
    echo "Example: $0   0     60   31"
  exit 1
endif

set CHIP     = $1
set CH       = $2
set CTRL_DAC = $3

(cd decoder; make;)

set TMP = `echo "obase=2; ibase=10; ${CTRL_DAC}" | bc | sed 's|-||'`
      if( ${CTRL_DAC} < 1 ) then
        set CTRL_DAC_BIT = `printf "L%05d\n" ${TMP} | sed 's|0|L|g' | sed 's|1|H|g'`
      else
        set CTRL_DAC_BIT = `printf "H%05d\n" ${TMP} | sed 's|0|L|g' | sed 's|1|H|g'`
      endif
echo "   DAC : ${CTRL_DAC} => ${CTRL_DAC_BIT}"


foreach ICHIP( 0 1 2 3 )
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

    # <Slow Control>
    cd slow_control;
    if( ${ICHIP} == ${CHIP} ) then
       ./make_control.sh ${CTRL_CHIP} ${CH} LLLLL${CTRL_DAC_BIT}LLHHH LLLLL${CTRL_DAC_BIT}LLLLL
    else
       ./make_control.sh ${CTRL_CHIP} ${CH} LLLLL${CTRL_DAC_BIT}LLLLL LLLLL${CTRL_DAC_BIT}LLLLL
    endif

    while (1)
     ./slit128sc control_${CTRL_CHIP}.dat 192.168.10.16;
     if( $? == 0 ) then
        break
     endif
    end
    #./slit128sc -d control_${CTRL_CHIP}.dat 192.168.10.16;
    sleep 0.1
    cd ../
    echo $ICHIP
end
