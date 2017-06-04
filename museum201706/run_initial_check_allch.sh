#! /bin/tcsh -f

if( $#argv < 2 )then
    echo " Usage : $0 [board] [DAC]"
    echo "Example: $0    2      31"
    echo "         board   : 2-5 & 3-6 (MuSEUM BT@201706)"
    echo "         DAC     : -31~31"
  exit 1
endif

set BOARD     = $1
set CHIP_LIST = "0 1 2 3"
set CHANNEL   = 0
set CTRL_DAC  = $3

(cd slow_control; make || exit;)
(cd exp_decoder;  make || exit;)
(cd ana;          make || exit;)
./run_offset_all_off.sh # all off

set TMP_DAC = `echo "obase=2; ibase=10; ${CTRL_DAC}" | bc | sed 's|-||'`
if( ${CTRL_DAC} < 1 ) then
   set CTRL_DAC_BIT = `printf "L%05d\n" ${TMP_DAC} | sed 's|0|L|g' | sed 's|1|H|g'`
else
   set CTRL_DAC_BIT = `printf "H%05d\n" ${TMP_DAC} | sed 's|0|L|g' | sed 's|1|H|g'`
endif
echo "   DAC : ${CTRL_DAC} => ${CTRL_DAC_BIT}"

cd slow_control;
foreach IBOARD( ${BOARD} )
    set IP = 16
    @ IP += ${IBOARD}
    echo "Board#${IBOARD}, IP=192.168.${IBOARD}.${IP}"
    foreach ICHIP( ${CHIP_LIST} )
	set TMP_CHIP  = `echo "obase=2; ibase=10; ${ICHIP}" | bc`
	set TMP_BOARD = `echo "obase=2; ibase=10; ${IBOARD}" | bc`
	set CTRL_CHIP = `printf "%04d%03d" ${TMP_BOARD} ${TMP_CHIP}`
	echo "    Chip#${ICHIP} (${CTRL_CHIP})"
	# <Slow Control>
	./make_control ${IBOARD} ${CTRL_CHIP} ${CHANNEL} LLLLL${CTRL_DAC_BIT}LLHLH LLLLL${CTRL_DAC_BIT}LLHLH # default (last 3 bits are digital-output/analog-monitor/test-pulse-in)

	while (1)
	    ./slit128sc_chip  files/control_${IBOARD}_${CTRL_CHIP}.dat 192.168.${IBOARD}.${IP};
	    if( $? == 0 ) then
		break
	    endif
	end
    end #end chip-loop
end #end board-loop
cd  ../

set OUTNAME = "test.dat"

# <Take Data>
nc -d 192.168.${BOARD}.${IP} 24 > test.dat &
sleep 2
kill -9 $!

# <Decode>
cd exp_decoder;
./multi-slit128a_exp_decoder_for_scurve ../test.root ../test.dat 0.0 0.0 ${BOARD} 0 0 ${CTRL_DAC}
cd ../

# <Plot>
cd ana;
./qc_allch ../test.root ${BOARD}

