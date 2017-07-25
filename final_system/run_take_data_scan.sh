#! /bin/tcsh -f

set HEADNAME = "output"
set BOARD    = 5
set VREF     = 200.0 # VREF value [mV]
set TPCHG    = 3.84 # Test pulse charge [fC] : 3.84 fC = 38.4 mV * 100fF (@1MIP)

####################################################################################
set CHIP_LIST     = "0 1 2 3"
set CHIP          = 0 # ${CHIP}%${CHIP_CYCLE} will be controlled
set CHIP_CYCLE    = 1
set CTRL_DAC      = -31
set TIME          = 3
set STEP          = 3

cd slow_control;   make || exit; cd ../;
cd exp_decoder;    make || exit; cd ../;
cd ana_scurve;     make || exit; cd ../;
cd readslit-0.0.0; make || exit; cd ../;

#./run_offset_all_off.sh # all off

set CTRL_DAC = -31
while ( ${CTRL_DAC} <= 31 )
set CHANNEL       = 0 # ${CHANNEL}%${CHANNEL_CYCLE} will be controlled
set CHANNEL_CYCLE = 2
while ( ${CHANNEL}  < ${CHANNEL_CYCLE} )

set TMP_DAC = `echo "obase=2; ibase=10; ${CTRL_DAC}" | bc | sed 's|-||'`
if( ${CTRL_DAC} < 1 ) then
   set CTRL_DAC_BIT = `printf "L%05d\n" ${TMP_DAC} | sed 's|0|L|g' | sed 's|1|H|g'`
else
   set CTRL_DAC_BIT = `printf "H%05d\n" ${TMP_DAC} | sed 's|0|L|g' | sed 's|1|H|g'`
endif
echo "DAC : ${CTRL_DAC} => ${CTRL_DAC_BIT}, Channel : ${CHANNEL}%${CHANNEL_CYCLE}"

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
    # <Slow Control>
    if( `expr ${ICHIP} % ${CHIP_CYCLE}` != ${CHIP} ) then
	echo -n "OFF, "
	./make_control ${BOARD} ${CTRL_CHIP} ${CHANNEL} ${CHANNEL_CYCLE} LLLLL${CTRL_DAC_BIT}LLLLL LLLLL${CTRL_DAC_BIT}LLLLL # default (last 3 bits are digital-output/analog-monitor/test-pulse-in)
    else
	echo -n "ON, "
	./make_control ${BOARD} ${CTRL_CHIP} ${CHANNEL} ${CHANNEL_CYCLE} LLLLL${CTRL_DAC_BIT}LLHLH LLLLL${CTRL_DAC_BIT}LLLLL # default (last 3 bits are digital-output/analog-monitor/test-pulse-in)
    endif    

    while (1)
	./slit128sc_chip  files/control_${BOARD}_${CTRL_CHIP}.dat 192.168.${BOARD}.${IP};
	if( $? == 0 ) then
	    break
	endif
    end
end #END CHIP-LOOP
echo ""
cd  ../
./run_fpga.sh ${BOARD} 0

# <Take Data>
echo -n "   Taking data during ${TIME} sec => "
set OUTNAME = "${HEADNAME}_board${BOARD}_${VREF}_${TPCHG}_${CTRL_DAC}_${CHANNEL}_${CHANNEL_CYCLE}"
mkdir -p binary_data
cd readslit-0.0.0/;
./readslit -t ${TIME} 192.168.${BOARD}.${IP} 24 ../binary_data/${OUTNAME}.dat
cd ../

# <Decode>
echo "Decoding Data"
mkdir -p root_data
mkdir -p root_data/tmp
cd exp_decoder;
./multi-slit128a_exp_decoder ../root_data/${OUTNAME}.root ../binary_data/${OUTNAME}.dat ${TPCHG} ${CTRL_DAC} ${CHIP} ${CHIP_CYCLE} ${CHANNEL} ${CHANNEL_CYCLE} ${BOARD} ${VREF}
rm -f ../binary_data/${OUTNAME}.dat
cd ../

@ CHANNEL += 1
end # END CHANNEL-LOOP

set OUTNAME = "${HEADNAME}_board${BOARD}_${VREF}_${TPCHG}_${CTRL_DAC}"
hadd -f root_data/${OUTNAME}.root root_data/${OUTNAME}_*.root && mv root_data/${OUTNAME}_*.root root_data/tmp/.

@ CTRL_DAC += ${STEP}
end # END DAC-LOOP

