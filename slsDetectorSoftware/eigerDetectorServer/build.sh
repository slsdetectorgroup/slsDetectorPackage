
echo -e "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"

. /opt/eldk-5.1/powerpc-4xx-softfloat/environment-setup-ppc440-linux

#powerpc-4xx-softfloat-g++ -Wall -o test Test.cxx HardwareIO.cxx LocalLinkInterface.cxx Feb.cxx -I . 
#powerpc-4xx-softfloat-g++ -Wall -o hardware_interface_test HardwareInterfaceTest.cxx HardwareInterface.cxx HardwareMMapping.cxx -I . 


#powerpc-4xx-softfloat-g++ -Wall -o eiger_test EigerTest.cxx Eiger.cxx HardwareIO.cxx LocalLinkInterface.cxx Feb.cxx -I . 

powerpc-4xx-softfloat-g++ -Wall -o feb_debug FebServer.cxx FebControl.cxx FebInterface.cxx LocalLinkInterface.cxx HardwareIO.cxx -I .

powerpc-4xx-softfloat-g++ -Wall -o beb_debug BebServer.cxx Beb.cxx LocalLinkInterface.cxx HardwareIO.cxx -I .


cp eiger_test rootfs_executables/.


