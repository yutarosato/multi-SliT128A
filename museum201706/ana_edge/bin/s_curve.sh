
#!/bin/sh

rm s_curve_result.txt
#for N in 27 36 37 39 41 43 44 45 49 72
#for N in `seq 74 81` 84 85 86
for FILE in `ls edge_data` 
do
#    RUN_NO=`printf "%06d" $N`
#    FILE=`ls edge_data| grep ${RUN_NO}`
    ./s_curve edge_data/${FILE} | tee -a s_curve_result.txt
#    mv tmp.root results/${N}.root

done 
