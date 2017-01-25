#! /bin/tcsh -f

set HEADNAME = "output"
set CHIP     = 0 # 0-3
set VREF     = 0.0 # VREF value [mV]
set TPCHG    = 1.92 # Test pulse charge [fC] : 3.84 fC = 38.4 mV * 100fF (@1MIP)


set CH_LIST = 123 
#set CH_LIST = `seq 122 124`
#set CH_LIST = `seq 0 127`

set CTRL_DAC_LIST = `seq -31 31`
#set CTRL_DAC_LIST = `seq 28 31`
#set CTRL_DAC_LIST = 31

###########################################
(cd decoder; make;)

# BEGIN (TREATMENT FOR OTHER CHIP)
echo -n "Suppress All Chip : "
foreach ICHIP( 0 1 2 3 )
    echo -n "${ICHIP} "
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
    cd slow_control;
    ./make_control.sh ${CTRL_CHIP} 0 LLLLLLLLLLLLLLLL LLLLLLLLLLLLLLLL # other DAC = 0 # default
    while (1)
       ./slit128sc control_${CTRL_CHIP}.dat 192.168.10.16;
       if( $? == 0 ) then
       break
       endif
    end
    #./slit128sc -d control_${CTRL_CHIP}.dat 192.168.10.16;
    cd ../
    echo ""
end
# END (TREATMENT FOR OTHER CHIP)
####################################################################################################

if( ${CHIP} == 0 ) then
    set CTRL_CHIP = "0000000"
else if( ${CHIP} == 1 ) then
    set CTRL_CHIP = "0000001"
else if( ${CHIP} == 2 ) then
    set CTRL_CHIP = "0000010"
else if( ${CHIP} == 3 ) then
    set CTRL_CHIP = "0000011"
else
    echo "Wrong CHIP-ID : ${CHIP}"
    exit
endif

foreach CH( ${CH_LIST} )
   echo "START S-Curve scan for Channel $CH"
   set CNT=0
   foreach CTRL_DAC( ${CTRL_DAC_LIST} )
      set TMP = `echo "obase=2; ibase=10; ${CTRL_DAC}" | bc | sed 's|-||'`
      if( ${CTRL_DAC} < 1 ) then
         set CTRL_DAC_BIT = `printf "L%05d\n" ${TMP} | sed 's|0|L|g' | sed 's|1|H|g'`
      else
         set CTRL_DAC_BIT = `printf "H%05d\n" ${TMP} | sed 's|0|L|g' | sed 's|1|H|g'`
      endif
      echo "   DAC : ${CTRL_DAC} => ${CTRL_DAC_BIT}"

      # <Slow Control>
      pwd
      cd slow_control;
      #./make_control.sh ${CTRL_CHIP} $CH LLLLL${CTRL_DAC_BIT}LLHHH LLLLL${CTRL_DAC_BIT}LLLLL
      #./make_control.sh ${CTRL_CHIP} $CH LLLLL${CTRL_DAC_BIT}LLHLH LLLLL${CTRL_DAC_BIT}LLLLL
      ./make_control.sh ${CTRL_CHIP} $CH LLLLL${CTRL_DAC_BIT}LLHLH LLLLLLLLLLLLLLLL # other DAC = 0 # default
      #./make_control.sh ${CTRL_CHIP} $CH LLLLL${CTRL_DAC_BIT}LLHLH LLLLLHHHHHHLLLLL # other DAC = +31
      #./make_control.sh ${CTRL_CHIP} $CH LLLLL${CTRL_DAC_BIT}LLHLH LLLLLLHHHHHLLLLL # other DAC = -31
      #./make_control.sh ${CTRL_CHIP} $CH LLLLL${CTRL_DAC_BIT}LLHLH LLLLLHLHHHHLLLLL # other DAC = +15
      #./make_control.sh ${CTRL_CHIP} $CH LLLLL${CTRL_DAC_BIT}LLHLH LLLLLLLHHHHLLLLL # other DAC = -15

      while (1)
         ./slit128sc control.dat 192.168.10.16;
         if( $? == 0 ) then
         break
         endif
      end
      #./slit128sc -d control.dat 192.168.10.16;
      #exit

      set OUTNAME = "${HEADNAME}_${VREF}_${TPCHG}_${CHIP}_${CH}_${CTRL_DAC}"

      # <Take Data>
      cd ../;
      mkdir -p binary_data
      nc -d 192.168.10.16 24 > binary_data/${OUTNAME}.dat &
      #sleep 3
      sleep 4
      #sleep 5
      #sleep 6
      #sleep 16
      #sleep 32
      #sleep 64
      kill -9 $!
      usleep 100000 # 0.1 sec

      # <Decode>
      mkdir -p root_data
      mkdir -p root_data/tmp

      cd decoder;
      ./slit128cmd_revise_chmap ../root_data/${OUTNAME}.root ../binary_data/${OUTNAME}.dat ${VREF} ${TPCHG} ${CH} ${CTRL_DAC}
      set NOHIT = $?
      rm -f ../binary_data/${OUTNAME}.dat
      #./slit128cmd ../root_data/${OUTNAME}.root ../binary_data/${OUTNAME}.dat ${VREF} ${TPCHG} ${CH} ${CTRL_DAC} && rm -f ../binary_data/${OUTNAME}.dat
      
      if( ${NOHIT} == 0 )then
         @ CNT = 0
      else
         @ CNT += 1
      endif
      if( ${CNT} == 2 )then
         echo "FINISH"
         cd ../
         break;
      endif
      cd ../

   end # DAC LOOP
   ./run_offset_noise.sh ${CHIP} 123 0
end # CHANNEL LOOP

hadd -f root_data/${HEADNAME}_${VREF}_${TPCHG}_${CHIP}.root root_data/${HEADNAME}_${VREF}_${TPCHG}_${CHIP}_*.root && mv root_data/${HEADNAME}_${VREF}_${TPCHG}_${CHIP}_*.root root_data/tmp/.

