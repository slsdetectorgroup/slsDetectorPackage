#!/bin/sh
serv="pc8498"
f="Jungfrau_CTB.rawbin"
if [ "$#" -gt 0 ]; then
    f=$1
fi
if [ "$#" -gt 1 ]; then
    serv=$2
fi
echo "File is $f server is $serv"
mount -t tmpfs none /mnt/

cd /mnt/
tftp -r $f  -g $serv
 
echo 7 > /sys/class/gpio/export
echo 9 > /sys/class/gpio/export
echo in  > /sys/class/gpio/gpio7/direction
echo out > /sys/class/gpio/gpio9/direction


echo 0 > /sys/class/gpio/gpio9/value


flash_eraseall /dev/mtd3
cat /mnt/$f > /dev/mtd3

echo 1 > /sys/class/gpio/gpio9/value
cat /sys/class/gpio/gpio7/value

