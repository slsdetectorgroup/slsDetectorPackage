add the following to /etc/rc before using programfpga command before cat motd


#registering 7th and 9th pin to linux kernel
echo 7 > /sys/class/gpio/export
echo 9 > /sys/class/gpio/export
#define direction for the linux kernel
echo in  > /sys/class/gpio/gpio7/direction
echo out > /sys/class/gpio/gpio9/direction
#needed, else all write errors when server starts up, because linux tries to take control fof gpio
echo 1 > /sys/class/gpio/gpio9/value
