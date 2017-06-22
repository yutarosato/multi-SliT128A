#! /bin/tcsh -f

set BOARD_LIST   = "2"
#set CHIP_LIST    = "0 1 2 3" # 0-3
set CHIP_LIST    = "0 1 3" # for board #2
#set CHANNEL_LIST = `seq 0 10`
set CHANNEL_LIST = `seq 0 127`


foreach BOARD  ( ${BOARD_LIST}   )
foreach CHIP   ( ${CHIP_LIST}    )
foreach CHANNEL( ${CHANNEL_LIST} )
    @ FAKE_CHANNEL = ${CHANNEL} % 32
    set FILE = "../data/scurve_data/201706131507/board2/root_data/output_*_${BOARD}_${FAKE_CHANNEL}.root"
    ./run_cal_eff_bt2.sh ${FILE} ${BOARD} ${CHIP} ${CHANNEL}
end
end
end
