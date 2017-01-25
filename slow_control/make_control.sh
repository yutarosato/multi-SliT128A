#! /bin/tcsh -f

if( $#argv < 3 )then
    echo " Usage : $0  [chip_no] [channel]    [command]    (command for others)"
    echo "Example: $0   1111111      3    LLLLLLLLLLLLLHHH  (LLLLLLLLLLLLLLLL)"
    echo "           chip_no : 7 bit"
    echo "           channel : 0-127"
    echo "           command(16 bit) for specific channel"
    echo "           command(16 bit) for other channels : default(LLLLLLLLLLLLLLLL)"
    exit 1
endif


set CHIP_NO       = $1
set CHANNEL_NO    = $2
set COMMAND       = $3
set OUTFILE       = "control_${CHIP_NO}.dat"
if( $#argv > 3 )then
   set COMMAND_OTHER = $4
else
   set COMMAND_OTHER = "LLLLLLLLLLLLLLLL"
endif

  
echo "#"            >  ${OUTFILE}
echo "# dip switch" >> ${OUTFILE}
echo ${CHIP_NO}     >> ${OUTFILE}
echo "#"            >> ${OUTFILE}

set CNT_CH = 0
set CH_ID  = 127
while( 1 )
  printf "%-5s" ${CNT_CH} >> ${OUTFILE}
  if( ${CH_ID} == ${CHANNEL_NO} ) then
    echo ${COMMAND} >> ${OUTFILE}
  else
    echo ${COMMAND_OTHER} >> ${OUTFILE}
  endif

    
  if( ${CNT_CH} == 127) then
    break;
  endif
  @ CNT_CH += 1
  @ CH_ID  -= 1
end
  
