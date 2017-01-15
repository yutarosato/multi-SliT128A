#! /bin/tcsh -f

set FILE    = $1
set CH1     = $2
set CH2     = $3
set OUTNAME = $4
set NAME    = $5

foreach DAC( `seq -31 31` )
  ./cal_eff ${FILE} ${CH1} ${CH2} ${DAC} | tee -a ${OUTNAME}
end

set CH1 = `printf "%03d" ${CH1}`
set CH2 = `printf "%03d" ${CH2}`
(cat pic/${NAME}_obsch${CH1}_tpch${CH2}_dac*_can1.ps > pic/${NAME}_obsch${CH1}_tpch${CH2}_can1.ps) && ps2pdf pic/${NAME}_obsch${CH1}_tpch${CH2}_can1.ps pic/${NAME}_obsch${CH1}_tpch${CH2}_can1.pdf  && rm -f pic/${NAME}_obsch${CH1}_tpch${CH2}_*can1.ps
(cat pic/${NAME}_obsch${CH1}_tpch${CH2}_dac*_can2.ps > pic/${NAME}_obsch${CH1}_tpch${CH2}_can2.ps) && ps2pdf pic/${NAME}_obsch${CH1}_tpch${CH2}_can2.ps pic/${NAME}_obsch${CH1}_tpch${CH2}_can2.pdf  && rm -f pic/${NAME}_obsch${CH1}_tpch${CH2}_*can2.ps
#psmerge -opic/${NAME}_obsch${CH1}_tpch${CH2}_can1.ps  pic/${NAME}_obsch${CH1}_tpch${CH2}_dac*_can1.ps && ps2pdf pic/${NAME}_obsch${CH1}_tpch${CH2}_can1.ps pic/${NAME}_obsch${CH1}_tpch${CH2}_can1.pdf  && rm -f pic/${NAME}_obsch${CH1}_tpch${CH2}_*can1.ps
#psmerge -opic/${NAME}_obsch${CH1}_tpch${CH2}_can2.ps  pic/${NAME}_obsch${CH1}_tpch${CH2}_dac*_can2.ps && ps2pdf pic/${NAME}_obsch${CH1}_tpch${CH2}_can2.ps pic/${NAME}_obsch${CH1}_tpch${CH2}_can2.pdf  && rm -f pic/${NAME}_obsch${CH1}_tpch${CH2}_*can2.ps
(ls pic/${NAME}_obsch${CH1}_tpch${CH2}_dac*.root | xargs hadd pic/${NAME}_obsch${CH1}_tpch${CH2}.root) && rm -f pic/${NAME}_obsch${CH1}_tpch${CH2}_dac*.root
