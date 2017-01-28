#! /bin/tcsh -f

if( $#argv < 4 )then
    echo " Usage : $0       [INFILE]       [THCHG] [chip_no] [channel]    [command]    (command for others)"
    echo "Example: $0  threshold_calib.dat   0.8       0         3          LLHHH             (LLLLL)"
    echo "           THCHG   : threshold charge [fC] (e.g. : 0.2 MIP = 0.768 fC)"
    echo "           chip_no : 0-3"
    echo "           channel : 0-127"
    echo "           command(5 bit) for specific channel : LLH(digital-output)H(analog-monitor)H(test-pulse-input)"
    echo "           command(5 bit) for other channels : default(LLLLL)"
    exit 1
endif


set INFILE        = $1
set THCHG         = $2
set CHIP_NO       = $3
set CHANNEL_NO    = $4
set COMMAND       = $5
set OUTFILE       = "control_calib_chip${CHIP_NO}.dat"
if( $#argv > 5 )then
   set COMMAND_OTHER = $6
else
   set COMMAND_OTHER = "LLLLL"
endif

if( ${CHIP_NO} == 0 ) then
	set CTRL_CHIP = "0000000"
else if( ${CHIP_NO} == 1 ) then
	set CTRL_CHIP = "0000001"
else if( ${CHIP_NO} == 2 ) then
	set CTRL_CHIP = "0000010"
else if( ${CHIP_NO} == 3 ) then
	set CTRL_CHIP = "0000011"
else
	echo "Wrong CHIP-ID : ${CHIP}"
	exit
endif

  
echo "#"            >  ${OUTFILE}
echo "# dip switch" >> ${OUTFILE}
echo ${CTRL_CHIP}   >> ${OUTFILE}
echo "#"            >> ${OUTFILE}

cat ${INFILE} | awk -v "selch=${CHANNEL_NO}" -v "com=${COMMAND}" -v "comoth=${COMMAND_OTHER}" -v "target=${THCHG}" -f make_control_calib.awk >> ${OUTFILE}
