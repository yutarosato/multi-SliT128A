#!/bin/sh

ANA_DIR=/home/g2silicon/work/museum/201706/ana_yutaro/bin/scan_results/
ROOT_DIR=/home/g2silicon/work/museum/201706/rootfiles_final
PROGRAM=/home/g2silicon/work/museum/201706/ana_yutaro/bin/threshold_scan

echo "Input Run #"
read i

CURRENT=`pwd`
# touch ${ANA_DIR}/scan_result.txt
#rm ${ANA_DIR}/scan_result.txt
cd ${ROOT_DIR}

for N in $i
#for N in 30
#for N in `seq 27 38`
do
    echo "Run number == $N"
    rm -f ${ANA_DIR}/${N}.txt
    RUN_NO=`printf "%06d" $N`
    FILE=`ls | grep ${RUN_NO}`
    echo ${FILE}
   ${PROGRAM} ${FILE}
    mv time.root   ${ANA_DIR}/${N}.root
    mv time.log    ${ANA_DIR}/${N}.log
    mv time_b2.eps ${ANA_DIR}/${N}_b2.eps
    mv time_b5.eps ${ANA_DIR}/${N}_b5.eps
    mv time_b2.png ${ANA_DIR}/${N}_b2.png
    mv time_b5.png ${ANA_DIR}/${N}_b5.png
done 
