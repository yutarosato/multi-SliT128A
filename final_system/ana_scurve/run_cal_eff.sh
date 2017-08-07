#! /bin/tcsh -f

if( $#argv < 1 )then
    echo " Usage : $0 [board]"
    echo "Example: $0    5"
    echo "         board   : 2-5 & 3-6 (MuSEUM BT@201706)"
  exit 1
endif

set BOARD   = $1
set DIR     = '../root_data/'
################################################################################################

make cal_eff  || exit

mkdir -p "dat_scurve"
mkdir -p "pic"

set DAC_LIST  = `seq -31 31`
foreach DAC( ${DAC_LIST} )
    echo "DAC = ${DAC}"
    ls ${DIR}/output06*_board${BOARD}_*_${DAC}.root >& /dev/null
    if( $? == 1 ) then
	echo "CONTINUE"
	continue;
    endif
    set FILE_LIST = `ls ${DIR}/output06*_board${BOARD}_*_${DAC}.root`
    foreach FILE( ${FILE_LIST} )
	echo "FILE = ${FILE}"
	set HEADER = `basename ${FILE} .root`
        echo ${HEADER}
	./cal_eff ${FILE} ${DAC} ${BOARD}
    end # END FILE-LOOP
end # END DAC-LOOP


set DAC    = `echo "${HEADER}" | awk -F "_" '{print $NF}'`
set HEADER = `basename ${HEADER} _${DAC}`
echo ${HEADER}

cat dat_scurve/${HEADER}/* > dat_scurve/${HEADER}.dat;
rm -rf dat_scurve/${HEADER};

(cat pic/${HEADER}_*_can1.ps > pic/${HEADER}_can1.ps) && ps2pdf     pic/${HEADER}_can1.ps pic/${HEADER}_can1.pdf  && rm -f pic/${HEADER}_*can1.ps
(cat pic/${HEADER}_*_can2.ps > pic/${HEADER}_can2.ps) && ps2pdf     pic/${HEADER}_can2.ps pic/${HEADER}_can2.pdf  && rm -f pic/${HEADER}_*can2.ps
ls   pic/${HEADER}_*.root                              | xargs hadd pic/${HEADER}.root                            && rm -f pic/${HEADER}_*.root
