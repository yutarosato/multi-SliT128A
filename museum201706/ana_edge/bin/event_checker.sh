#!/bin/sh

ANA_DIR=~/doctor/MuSEUM/201702/ana
EDGE_DIR=~/doctor/MuSEUM/201702/data_silicon/edge_data
PROGRAM=${ANA_DIR}/event_checker
TEXT_OUTPUT=log.txt
rm ${ANA_DIR}/${TEXT_OUTPUT}

cd ${EDGE_DIR}

#for N in 18 19 20 26 27 29 30 41 43 44 47 49 `seq 53 56` 58 59 `seq 61 65` `seq 69 73` `seq 76 81` 375 `seq 86 120` `seq 122 306`
#for N in `seq 86 120` `seq 122 306`
for N in 77
do
    RUN_NO=`printf "%06d" $N`
    FILE=`ls | grep ${RUN_NO}`
    TMP=`echo ${FILE} | awk '{print $1}'`
    echo $TMP
    if [ -f "${TMP}" ]
    then
	echo ${N} | tee -a ${ANA_DIR}/${TEXT_OUTPUT}
	${PROGRAM} ${FILE} 2> /dev/null | tee -a ${ANA_DIR}/${TEXT_OUTPUT}
#	mv time.root ${ANA_DIR}/tot_results/${N}.root
    fi
done 



