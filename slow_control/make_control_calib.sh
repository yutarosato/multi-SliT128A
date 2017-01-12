#! /bin/tcsh -f

if( $#argv < 4 )then
    echo " Usage : $0       [INFILE]       [THCHG] [chip_no] [channel]    [command]    (command for others)"
    echo "Example: $0  threshold_calib.dat   0.8    1111111      3          LLHHH             (LLLLL)"
    echo "           THCHG   : threshold charge [fC] (e.g. : 0.2 MIP = 0.768 fC)"
    echo "           chip_no : 7 bit"
    echo "           channel : 0-127"
    echo "           command(5 bit) for specific channel"
    echo "           command(5 bit) for other channels : default(LLLLL)"
    exit 1
endif

set OUTFILE       = "control_calib.dat"
set INFILE        = $1
set THCHG         = $2
set CHIP_NO       = $3
set CHANNEL_NO    = $4
set COMMAND       = $5
if( $#argv > 5 )then
   set COMMAND_OTHER = $6
else
   set COMMAND_OTHER = "LLLLL"
endif

  
echo "#"            >  ${OUTFILE}
echo "# dip switch" >> ${OUTFILE}
echo ${CHIP_NO}     >> ${OUTFILE}
echo "#"            >> ${OUTFILE}

cat ${INFILE} | awk -v "selch=${CHANNEL_NO}" -v "com=${COMMAND}" -v "comoth=${COMMAND_OTHER}" -f make_control_calib.awk
