#! /bin/tcsh -f

#set CHANNEL_LIST = `seq 0 31`
#set CHANNEL_LIST = `seq 6 31`
set CHANNEL_LIST = 1
#set CHANNEL_LIST = "16 25"
foreach CHANNEL( ${CHANNEL_LIST} )
      ./run_take_data_scan_multi_chip_unit.sh ${CHANNEL}
end
