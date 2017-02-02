#! /bin/tcsh -f

echo "Chip#0"
#./slit128sc1 control_calib_chip0.dat   192.168.10.16
echo "Chip#1"
./slit128sc1 control_calib_chip1.dat   192.168.10.16
echo "Chip#2"
./slit128sc1 control_nocalib_chip2.dat 192.168.10.16
echo "Chip#3"
./slit128sc1 control_nocalib_chip3.dat 192.168.10.16
echo "Final Setting"
./slit128sc2 control_nocalib_chip3.dat 192.168.10.16
echo "Finish!"
