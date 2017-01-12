#! /bin/tcsh -f

if( $#argv < 2 )then
    echo " Usage : $0 [ch] [DAC]"
    echo "Example: $0  60    31"
  exit 1
endif

set CH       = $1
set CTRL_DAC = $2

(cd decoder; make;)

set TMP = `echo "obase=2; ibase=10; ${CTRL_DAC}" | bc | sed 's|-||'`
      if( ${CTRL_DAC} < 1 ) then
        set CTRL_DAC_BIT = `printf "L%05d\n" ${TMP} | sed 's|0|L|g' | sed 's|1|H|g'`
      else
        set CTRL_DAC_BIT = `printf "H%05d\n" ${TMP} | sed 's|0|L|g' | sed 's|1|H|g'`
      endif
echo "   DAC : ${CTRL_DAC} => ${CTRL_DAC_BIT}"

# <Slow Control>
cd slow_control;
./make_control.sh 0000000 $CH LLLLL${CTRL_DAC_BIT}LLHHH LLLLL${CTRL_DAC_BIT}LLLLL

while (1)
   ./slit128sc control.dat 192.168.10.16;
   if( $? == 0 ) then
      break
   endif
end
#./slit128sc -d control.dat 192.168.10.16;
