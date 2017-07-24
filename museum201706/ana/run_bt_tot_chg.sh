#! /bin/tcsh -f



#set BOARD_LIST   = "2"
#set VREF  = "250.0"
#set BOARD_LIST   = "5"
#set VREF  = "400.0"
set BOARD_LIST   = $1
set VREF  = $2

set CHIP_LIST    = "0 1 2 3" # 0-3
set CHANNEL_LIST  = `seq 0 127`
set DAC = "0"

set TPCHG = $3
#set TPCHG = "11.50" # 3.00 MIP
#set TPCHG = "7.68" # 2.00 MIP
#set TPCHG = "3.84" # 1.00 MIP
#set TPCHG = "2.88" # 0.75 MIP
#set TPCHG = "1.92" # 0.50 MIP



################################################################################################

make cal_eff  || exit

mkdir -p "dat_scurve"
mkdir -p "pic"


foreach BOARD  ( ${BOARD_LIST}   )
foreach CHIP   ( ${CHIP_LIST}    )
foreach CHANNEL( ${CHANNEL_LIST} )
    @ DUMMY_CHANNEL = ${CHANNEL} % 32
    set FILE = "/home/belle/syutaro/museum201706/afterBT/tot_chg/root_data/output_${VREF}_${TPCHG}_${BOARD}_${DUMMY_CHANNEL}.root"
    if(! -f ${FILE} ) then
	echo "NOT FOUND ROOT FILE"
	continue;
    endif
    set HEADER = `basename ${FILE} .root`
    set CHANNEL_NAME = `printf "%03d" ${CHANNEL}`
    set OUTDAT = "dat_scurve/${HEADER}_${BOARD}_${CHIP}_${CHANNEL_NAME}.dat"
    echo $OUTDAT
    rm -f ${OUTDAT}
    ./cal_eff ${FILE} ${BOARD} ${CHIP} ${CHANNEL} ${DAC} | tee -a ${OUTDAT}
end # CHANNEL LOOP
set TMPNAME  = "board${BOARD}_chip${CHIP}_channel???"
set OUTPIC = "${HEADER}_board${BOARD}_chip${CHIP}"
echo "+++++++++++++++++++++++++++++++++++++++"
(cat pic/*_${TMPNAME}_dac*_can1.ps > pic/${OUTPIC}_can1.ps) && ps2pdf     pic/${OUTPIC}_can1.ps pic/${OUTPIC}_can1.pdf && rm -f  pic/*_${TMPNAME}_dac*_can1.ps pic/${OUTPIC}_can1.ps
(cat pic/*_${TMPNAME}_dac*_can2.ps > pic/${OUTPIC}_can2.ps) && ps2pdf     pic/${OUTPIC}_can2.ps pic/${OUTPIC}_can2.pdf && rm -f  pic/*_${TMPNAME}_dac*_can2.ps pic/${OUTPIC}_can2.ps
 ls  pic/*_${TMPNAME}_dac*.root                              | xargs hadd pic/${OUTPIC}.root                           && rm -f pic/*_${TMPNAME}_dac*.root
end # CHIP LOOP
end # BOARD LOOP
