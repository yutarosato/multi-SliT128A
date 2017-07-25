#!/bin/sh

ANA_DIR=~/doctor/MuSEUM/201702/ana
EDGE_DIR=~/doctor/MuSEUM/201702/data_silicon/edge_data
PROGRAM=${ANA_DIR}/noise_occupancy
TEXT_OUTPUT=noise_result.txt
rm ${ANA_DIR}/${TEXT_OUTPUT}
cd ${EDGE_DIR}

for N in `seq 86 120` `seq 122 306`

do
    RUN_NO=`printf "%06d" $N`
    FILE=`ls | grep ${RUN_NO}`
    TMP=`echo ${FILE} | awk '{print $1}'`
    echo $TMP
    if [ -f "${TMP}" ]
    then
	echo ${N} | tee -a ${ANA_DIR}/${TEXT_OUTPUT}
	${PROGRAM} ${FILE} 2> /dev/null | tee -a ${ANA_DIR}/${TEXT_OUTPUT}
	mv time.root ${ANA_DIR}/noise_results/${N}.root
    fi
done 
