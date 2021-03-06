#! /bin/tcsh -f

if( $#argv < 4 )then
    echo " Usage : $0 [board] [chip] [channel] [DAC]"
    echo "Example: $0    2       0      60       31"
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

(cd slow_control;   (make || exit;))
(cd exp_decoder;    (make || exit;))
(cd ana;            (make || exit;))
(cd readslit-0.0.0; (make || exit;))
#./run_offset_all_off.sh # tmppp

set TMP_DAC = `echo "obase=2; ibase=10; ${CTRL_DAC}" | bc | sed 's|-||'`
if( ${CTRL_DAC} < 1 ) then
   set CTRL_DAC_BIT = `printf "L%05d\n" ${TMP_DAC} | sed 's|0|L|g' | sed 's|1|H|g'`
else
   set CTRL_DAC_BIT = `printf "H%05d\n" ${TMP_DAC} | sed 's|0|L|g' | sed 's|1|H|g'`
endif
echo "   DAC : ${CTRL_DAC} => ${CTRL_DAC_BIT}"

cd slow_control;
set IP = 16
@ IP += ${BOARD}
echo "Board#${BOARD}, IP=192.168.${BOARD}.${IP}"
foreach ICHIP( ${CHIP} )
    set TMP_CHIP  = `echo "obase=2; ibase=10; ${ICHIP}" | bc`
    set TMP_BOARD = `echo "obase=2; ibase=10; ${BOARD}" | bc`
    set CTRL_CHIP = `printf "%04d%03d" ${TMP_BOARD} ${TMP_CHIP}`
    echo "    Chip#${ICHIP} (${CTRL_CHIP})"
    # <Slow Control>
    if( ${ICHIP} == ${CHIP} ) then
	./make_control ${BOARD} ${CTRL_CHIP} ${CHANNEL} LLLLL${CTRL_DAC_BIT}LLHLH LLLLL${CTRL_DAC_BIT}LLLLL # default (last 3 bits are digital-output/analog-monitor/test-pulse-in)
    else
	./make_control ${BOARD} ${CTRL_CHIP} ${CHANNEL} LLLLL${CTRL_DAC_BIT}LLLLL LLLLL${CTRL_DAC_BIT}LLLLL # default
    endif

    while (1)
	./slit128sc_chip  files/control_${BOARD}_${CTRL_CHIP}.dat 192.168.${BOARD}.${IP};
	if( $? == 0 ) then
	    break
	endif
    end
end #end chip-loop
cd  ../
./run_fpga.sh ${BOARD}

set OUTNAME = "test.dat"

# <Take Data>
#nc -d 192.168.${BOARD}.${IP} 24 > test.dat &
#sleep 3
#kill -9 $!
cd readslit-0.0.0/;
./readslit -t 3 192.168.${BOARD}.${IP} 24 ../test.dat
cd ../

# <Decode>
cd exp_decoder;
./multi-slit128a_exp_decoder_for_scurve ../test.root ../test.dat 0.0 0.0 ${BOARD} ${CHIP} ${CHANNEL} ${CTRL_DAC}
cd ../

# <Plot>
cd ana;
#./qc_allch ../test.root ${CHIP}
./qc_onech ../test.root ${BOARD} ${CHIP} ${CHANNEL} ${CTRL_DAC}

