#!/bin/sh

ANA_DIR=~/doctor/MuSEUM/201702/ana
EDGE_DIR=~/doctor/MuSEUM/201702/data_silicon/edge_data
PROGRAM=${ANA_DIR}/cluster
#TEXT_OUTPUT=noise_result.txt
#rm ${ANA_DIR}/${TEXT_OUTPUT}
cd ${EDGE_DIR}

for N in `seq 87 120` `seq 122 306`
#for N in 120
do
    RUN_NO=`printf "%06d" $N`
    FILE=`ls | grep ${RUN_NO}`
    TMP=`echo ${FILE} | awk '{print $1}'`
    echo $TMP
    if [ -f "${TMP}" ]
    then
#	echo ${N} | tee -a ${ANA_DIR}/${TEXT_OUTPUT}
	${PROGRAM} ${FILE} 2> /dev/null
	mv cluster.root ${ANA_DIR}/result_cluster/${N}.root
    fi
done 
