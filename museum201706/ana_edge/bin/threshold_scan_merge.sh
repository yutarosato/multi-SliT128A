#!/bin/sh

ANA_DIR=/home/g2silicon/work/museum/201706/ana/bin/scan_results
ROOT_DIR=/home/g2silicon/work/museum/201706/rootfiles_final
PROGRAM=/home/g2silicon/work/museum/201706/ana/bin/threshold_scan

echo "Input run # to merge"

read i

rm ${ANA_DIR}/scan_txt/scan_result_merge.txt
touch ${ANA_DIR}/scan_txt/scan_result_merge.txt

for N in $i

do

    cat ${ANA_DIR}/scan_txt/scan_result_${N}.txt >> ${ANA_DIR}/scan_txt/scan_result_merge.txt

done


# touch ${ANA_DIR}/scan_result_merge.txt
# rm ${ANA_DIR}/scan_result_merge.txt
# cd ${ROOT_DIR}

# for N in $i

# do
#     RUN_NO=`printf "%06d" $N`
#     FILE=`ls | grep ${RUN_NO}`
#     echo ${FILE}
# #   ${PROGRAM} ${FILE} 2> /dev/null | tee -a ${ANA_DIR}/scan_result_${N}.txt                                                      
#     ${PROGRAM} ${FILE} 2> /dev/null | tee -a ${ANA_DIR}/scan_results/scan_result_merge.txt
#     sed -i "1s/^/$N /" ${ANA_DIR}/scan_results/scan_result_merge.txt
#     mv time.root ${ANA_DIR}/scan_results/${N}.root

# done
