#! /bin/tcsh -f

set HEADNAME = "output"
set CHIP     = 3 # 0-3
set VREF     = 180.0 # VREF value [mV]
set TPCHG    = 1.15 # Test pulse charge [fC] : 3.84 fC = 38.4 mV * 100fF (@1MIP)

#set CH_LIST = 123 
#set CH_LIST = "37 59 90"
#set CH_LIST = "24 27 29 43 47 48 49 53 55 61 64 69 70 72 80 81 82 85 87 88 89 97 98 99 104 109 119 121 122"
#set CH_LIST = "2 3" # Chip#0
#set CH_LIST = "0 1" # Chip#1
#set CH_LIST = "0 1 2 3 4 5"
#set CH_LIST = "68 76 79"
#set CH_LIST = 0
#set CH_LIST = 66
#set CH_LIST = `seq 122 124`
#set CH_LIST = `seq 0 127`
#set CH_LIST = `seq 87 127`
#set CH_LIST = "1 2" # demo-power supply
set CH_LIST = "1 16 32 48 64 80 96 112" # all chip scan


###########################################
(cd decoder; make;)

# BEGIN (TREATMENT FOR OTHER CHIP)
echo -n "Suppress All Chip : "
foreach ICHIP( 0 1 2 3 )
#foreach ICHIP( 0 )
    echo -n "CHIP = ${ICHIP} "
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
   set CTRL_DAC = -31
   while ( ${CTRL_DAC} <= 31 )
      set TMP = `echo "obase=2; ibase=10; ${CTRL_DAC}" | bc | sed 's|-||'`
      if( ${CTRL_DAC} < 1 ) then
         set CTRL_DAC_BIT = `printf "L%05d\n" ${TMP} | sed 's|0|L|g' | sed 's|1|H|g'`
      else
         set CTRL_DAC_BIT = `printf "H%05d\n" ${TMP} | sed 's|0|L|g' | sed 's|1|H|g'`
      endif
      echo "   DAC : ${CTRL_DAC} => ${CTRL_DAC_BIT}"

      # <Slow Control>
      #while(1)
      pwd
      cd slow_control;
      ./make_control.sh ${CTRL_CHIP} $CH LLLLL${CTRL_DAC_BIT}LLHLH LLLLLLLLLLLLLLLL # other DAC = 0 # default

      while (1)
        ./slit128sc control_${CTRL_CHIP}.dat 192.168.10.16;
        if( $? == 0 ) then
           break
        endif
      end
      #./slit128sc control_${CTRL_CHIP}.dat 192.168.10.16;
      #exit

      set OUTNAME = "${HEADNAME}_${VREF}_${TPCHG}_${CHIP}_${CH}_${CTRL_DAC}"

      # <Take Data>
      cd ../;
      mkdir -p binary_data
      nc -d 192.168.10.16 24 > binary_data/${OUTNAME}.dat &
      sleep 2
      #sleep 4

      kill -9 $!
      usleep 100000 # 0.1 sec

      # <Decode>
      mkdir -p root_data
      mkdir -p root_data/tmp

      cd decoder;
      ./slit128cmd_revise_chmap ../root_data/${OUTNAME}.root ../binary_data/${OUTNAME}.dat ${VREF} ${TPCHG} ${CH} ${CTRL_DAC}

      set NOHIT = $?

	#if( $NOHIT == 0 ) then
	#    echo "BREAK"
	#    break;
	#else
	#    echo "RE-DO"
	#    cd ../
	#endif
      #end

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

      # skip at early-point
      pwd
      set NOHIT = `cat tmp.log`
      echo ${NOHIT}
      cd ../
      if( ${NOHIT} >= 35000 )then
        @ CTRL_DAC += 4
      else if( ${NOHIT} >= 24000 )then
        @ CTRL_DAC += 3
      else if( ${NOHIT} >= 22000)then
        #@ CTRL_DAC += 2
        @ CTRL_DAC += 3 # tmpppp
      else
        #@ CTRL_DAC += 1
        @ CTRL_DAC += 3 # tmpppp
      endif
   end # DAC LOOP
   #./run_offset_noise.sh ${CHIP} 123 0
end # CHANNEL LOOP
./run_offset_noise.sh ${CHIP} 123 0

hadd -f root_data/${HEADNAME}_${VREF}_${TPCHG}_${CHIP}.root root_data/${HEADNAME}_${VREF}_${TPCHG}_${CHIP}_*.root && mv root_data/${HEADNAME}_${VREF}_${TPCHG}_${CHIP}_*.root root_data/tmp/.

