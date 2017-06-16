#! /bin/tcsh -f

set INFILE        = $1
set OUTFILE       = $2

echo "#"            >  ${OUTFILE}
echo "# dip switch" >> ${OUTFILE}
echo "XXXXXXXX"     >> ${OUTFILE}
echo "#"            >> ${OUTFILE}

cat ${INFILE} | awk -f make_control_calib.awk >> ${OUTFILE}
