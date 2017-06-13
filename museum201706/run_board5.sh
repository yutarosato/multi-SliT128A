#! /bin/tcsh -f

set CHANNEL_LIST = `seq 0 31`

foreach CHANNEL( ${CHANNEL_LIST} )
      ./run_take_data_scan_multi_chip_unit.sh ${CHANNEL}
end
