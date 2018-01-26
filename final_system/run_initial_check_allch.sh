#! /bin/tcsh -f

if( $#argv < 2 )then
    echo " Usage : $0 [board] [DAC]"
    echo "Example: $0    2      31"
    echo "         board   : 2-5 & 3-6 (MuSEUM BT@201706)"
    echo "         DAC     : -31~31"
  exit 1
endif

set BOARD         = $1
set CHIP          = 0 # ${CHIP}%${CYCLE_CHIP} will be controlled
set CYCLE_CHIP    = 1
set CHANNEL       = 0 # ${CHANNEL}%${CYCLE_CHANNEL} will be controlled
#set CYCLE_CHANNEL = 1
set CYCLE_CHANNEL = 2 # tmpppp
set CTRL_DAC      = $2
set TIME          = 1

cd slow_control;   make || exit; cd ../;
cd exp_decoder;    make || exit; cd ../;
cd ana_scurve;     make || exit; cd ../;
cd readslit-0.0.0; make || exit; cd ../;

#./run_offset_all_off.sh # all off

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
foreach ICHIP( `seq 0 3` )
    set TMP_CHIP  = `echo "obase=2; ibase=10; ${ICHIP}" | bc`
    set TMP_BOARD = `echo "obase=2; ibase=10; ${BOARD}" | bc`
    set CTRL_CHIP = `printf "%04d%03d" ${TMP_BOARD} ${TMP_CHIP}`
    echo -n "    Chip#${ICHIP} (${CTRL_CHIP}) : "
    # <Slow Control>
    if( `expr ${ICHIP} % ${CYCLE_CHIP}` != ${CHIP} ) then
	echo "OFF"
	./make_control ${BOARD} ${CTRL_CHIP} ${CHANNEL} ${CYCLE_CHANNEL} LLLLL${CTRL_DAC_BIT}LLLLL LLLLL${CTRL_DAC_BIT}LLLLL # default (last 3 bits are digital-output/analog-monitor/test-pulse-in)
    else
	echo "ON"
	./make_control ${BOARD} ${CTRL_CHIP} ${CHANNEL} ${CYCLE_CHANNEL} LLLLL${CTRL_DAC_BIT}LLHLH LLLLL${CTRL_DAC_BIT}LLLLL # default (last 3 bits are digital-output/analog-monitor/test-pulse-in)
    endif
    

    while (1)
	./slit128sc_chip  files/control_${BOARD}_${CTRL_CHIP}.dat 192.168.${BOARD}.${IP};
	if( $? == 0 ) then
	    break
	endif
    end
end #END CHIP-LOOP
cd  ../
./run_fpga.sh ${BOARD}

set OUTNAME = "test.dat"

# <Take Data>
echo "Taking data during ${TIME} sec ....."
cd readslit-0.0.0/;
./readslit -t ${TIME} 192.168.${BOARD}.${IP} 24 ../test.dat
cd ../

# <Decode>
echo "Decoding Data ....."
cd exp_decoder;
./multi-slit128a_exp_decoder ../test.root ../test.dat 0.0 ${CTRL_DAC} -999 -999 -999 -999 ${BOARD} -999
cd ../

# <Plot>
echo "Plot ...."
cd ana_scurve;
./qc_allch ../test.root ${BOARD}
