#! /bin/tcsh -f


if( $#argv < 1 )then
    echo " Usage : $0 [NO]"
    echo "Example: $0   2"
  exit 1
endif

set NO       = $1
set HEADNAME = `printf output%02d ${NO}`
set BOARD    = 5
set VREF     = 230.0 # VREF value [mV]
set CTRL_DAC = 0 # dummy value
set TPCHG    = 6.14 # Test pulse charge [fC]
#   3.0   2.5   2.4   2.2   2.0   1.8   1.6   1.5   1.4   1.3   1.2   1.1   1.0   0.9   0.8   0.7   0.6   0.5   0.4   0.3   0.2   0.1
# 11.52, 9.60, 9.22, 8.45, 7.68, 6.91, 6.14, 5.76, 5.38, 4.99, 4.61, 4.22, 3.84, 3.46, 3.07, 2.69, 2.30, 1.92, 1.54, 1.15, 0.77, 0.38
# 1 MIP = 3.84 fC = 38.4 mV * 100fF
# 1 MIP = 300 mV for 8 divider (actually 7 input) with ~1% error
set THRESHOLD_MIP = 0.2

set DATDIR = '../ana_scurve/dat_calib/'
echo ${HEADNAME}
####################################################################################
ls root_data/${HEADNAME}*.root >& /dev/null
if( $? != 1 ) then
    echo "[Skip] Aleady exist : ${NO}"
#    exit
endif

####################################################################################
set CHIP          = 0 # ${CHIP}%${CHIP_CYCLE} will be controlled
set CHIP_CYCLE    = 1
set TIME          = 3

cd slow_control;   make || exit; cd ../;
cd exp_decoder;    make || exit; cd ../;
cd ana_scurve;     make || exit; cd ../;
cd readslit-0.0.0; make || exit; cd ../;

set CHANNEL       = 0 # ${CHANNEL}%${CHANNEL_CYCLE} will be controlled
set CHANNEL_CYCLE = 2
while ( ${CHANNEL}  < ${CHANNEL_CYCLE} )

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
    set DATFILE = "${DATDIR}/threshold_calib_board${BOARD}_chip${ICHIP}.dat"
     <Slow Control>
    if( `expr ${ICHIP} % ${CHIP_CYCLE}` != ${CHIP} ) then
    	echo -n "OFF, "
    	./make_control_calib ${BOARD} ${CTRL_CHIP} ${DATFILE} ${THRESHOLD_MIP} ${CHANNEL} ${CHANNEL_CYCLE} LLL LLL # default (last 3 bits are digital-output/analog-monitor/test-pulse-in)
    else
    	echo -n "ON, "
    	./make_control_calib ${BOARD} ${CTRL_CHIP} ${DATFILE} ${THRESHOLD_MIP} ${CHANNEL} ${CHANNEL_CYCLE} HLH LLL # default (last 3 bits are digital-output/analog-monitor/test-pulse-in)
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
set OUTNAME = "${HEADNAME}_board${BOARD}_${VREF}_${TPCHG}_${CTRL_DAC}_${CHANNEL}_${CHANNEL_CYCLE}"
mkdir -p binary_data
cd readslit-0.0.0/;
./readslit -t ${TIME} 192.168.${BOARD}.${IP} 24 ../binary_data/${OUTNAME}.dat
cd ../

# <Decode>
echo "Decoding Data ....."
mkdir -p root_data
mkdir -p root_data/tmp
cd exp_decoder;
./multi-slit128a_exp_decoder ../root_data/${OUTNAME}.root ../binary_data/${OUTNAME}.dat ${TPCHG} ${CTRL_DAC} ${CHIP} ${CHIP_CYCLE} ${CHANNEL} ${CHANNEL_CYCLE} ${BOARD} ${VREF}
rm -f ../binary_data/${OUTNAME}.dat
cd ../

@ CHANNEL += 1
end # END CHANNEL-LOOP

set OUTNAME = "${HEADNAME}_board${BOARD}_${VREF}_${TPCHG}_${CTRL_DAC}"
hadd -f root_data/${OUTNAME}.root root_data/${OUTNAME}_*.root && mv root_data/${OUTNAME}_*.root root_data/tmp/. && rm -rf root_data/tmp

# <Analysis>
cd ana_scurve;
mkdir -p "dat_scurve"
mkdir -p "pic"
./cal_eff ../root_data/${OUTNAME}.root ${CTRL_DAC} ${BOARD}

set DAC    = `echo "${OUTNAME}" | awk -F "_" '{print $NF}'`
set HEADER = `basename ${OUTNAME} _${DAC}`
echo ${HEADER}
cat dat_scurve/${HEADER}/* > dat_scurve/${HEADER}.dat;
rm -rf dat_scurve/${HEADER};

cd ../
