#! /bin/tcsh -f

set HEADNAME = "output"
set BOARD    = 5
set THRESHOLD_MIP = 0.2 # [MIP]
set DIR = '../ana_scurve/dat_calib/'

####################################################################################
set CHIP          = 0 # ${CHIP}%${CHIP_CYCLE} will be controlled
set CHIP_CYCLE    = 1
set CHANNEL       = 0 # ${CHANNEL}%${CHANNEL_CYCLE} will be controlled
set CHANNEL_CYCLE = 2
set TIME          = 3

cd slow_control;   make || exit; cd ../;
cd exp_decoder;    make || exit; cd ../;
cd ana_scurve;     make || exit; cd ../;
cd readslit-0.0.0; make || exit; cd ../;

cd slow_control;
set IP = 16
@ IP += ${BOARD}
echo "   Board#${BOARD}, IP=192.168.${BOARD}.${IP}"
echo -n "   "
foreach ICHIP( `seq 0 3` )
    set TMP_CHIP  = `echo "obase=2; ibase=10; ${ICHIP}" | bc`
    set TMP_BOARD = `echo "obase=2; ibase=10; ${BOARD}" | bc`
    set CTRL_CHIP = `printf "%04d%03d" ${TMP_BOARD} ${TMP_CHIP}`
    echo -n "Chip#${ICHIP}(${CTRL_CHIP}):"
    set INFILE = "${DIR}/threshold_calib_board${BOARD}_chip${ICHIP}.dat"
    # <Slow Control>
    if( `expr ${ICHIP} % ${CHIP_CYCLE}` != ${CHIP} ) then
	echo -n "OFF, "
	./make_control_calib ${BOARD} ${CTRL_CHIP} ${INFILE} ${THRESHOLD_MIP} ${CHANNEL} ${CHANNEL_CYCLE} LLL LLL # default (last 3 bits are digital-output/analog-monitor/test-pulse-in)
    else
	echo -n "ON, "
	./make_control_calib ${BOARD} ${CTRL_CHIP} ${INFILE} ${THRESHOLD_MIP} ${CHANNEL} ${CHANNEL_CYCLE} HLH LLL # default (last 3 bits are digital-output/analog-monitor/test-pulse-in)
    endif    

    while (1)
	./slit128sc_chip  files_calib/control_${BOARD}_${CTRL_CHIP}.dat 192.168.${BOARD}.${IP};
	if( $? == 0 ) then
	    break
	endif
    end
end #END CHIP-LOOP
echo ""
cd  ../
./run_fpga.sh ${BOARD} 0

# <Take Data>
echo -n "   Taking data during ${TIME} sec ....."
cd readslit-0.0.0/;
./readslit -t ${TIME} 192.168.${BOARD}.${IP} 24 ../test.dat
cd ../

# <Decode>
echo "Decoding Data ....."
cd exp_decoder;
./multi-slit128a_exp_decoder ../test.root ../test.dat 0 0 ${CHIP} ${CHIP_CYCLE} ${CHANNEL} ${CHANNEL_CYCLE} ${BOARD} 0
cd ../

# <Plot>
echo "Plot ...."
cd ana_scurve;
./qc_allch ../test.root ${BOARD}
