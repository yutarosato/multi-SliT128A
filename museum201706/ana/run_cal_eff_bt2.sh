#! /bin/tcsh -f

set FILE    = $1
set BOARD   = $2
set CHIP    = $3
set CHANNEL = $4

set DAC_LIST  = `seq -31 31`

################################################################################################

make cal_eff  || exit

mkdir -p "dat_scurve"
mkdir -p "pic"

set HEADER = `basename ${FILE} .root`
set CHANNEL_NAME = `printf "%03d" ${CHANNEL}`
set OUTDAT = "dat_scurve/${HEADER}_${CHIP}_${CHANNEL_NAME}.dat"
rm -f ${OUTDAT}
foreach DAC( ${DAC_LIST} )
    ./cal_eff ${FILE} ${BOARD} ${CHIP} ${CHANNEL} ${DAC} | tee -a ${OUTDAT}
end
#set OUTPIC = ${HEADER}_board${BOARD}_chip${CHIP}_channel${CHANNEL_NAME}
#(cat pic/${OUTPIC}_dac*_can1.ps > pic/${OUTPIC}_can1.ps) && ps2pdf     pic/${OUTPIC}_can1.ps pic/${OUTPIC}_can1.pdf  && rm -f pic/${OUTPIC}_*can1.ps
#(cat pic/${OUTPIC}_dac*_can2.ps > pic/${OUTPIC}_can2.ps) && ps2pdf     pic/${OUTPIC}_can2.ps pic/${OUTPIC}_can2.pdf  && rm -f pic/${OUTPIC}_*can2.ps
#ls   pic/${OUTPIC}_dac*.root                              | xargs hadd pic/${OUTPIC}.root                            && rm -f pic/${OUTPIC}_dac*.root
