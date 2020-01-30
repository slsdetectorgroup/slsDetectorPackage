#include "programFpgaBlackfin.h"
#include "ansi.h"
#include "clogger.h"
#include "slsDetectorServer_defs.h"

#include <unistd.h> 	// usleep
#include <string.h>


/* global variables */
#define MTDSIZE                     10

int gpioDefined = 0;
char mtdvalue[MTDSIZE] = {0};

void defineGPIOpins(){
	if (!gpioDefined) {
		//define the gpio pins
		system("echo 7 > /sys/class/gpio/export");
		system("echo 9 > /sys/class/gpio/export");
		//define their direction
		system("echo in  > /sys/class/gpio/gpio7/direction");
		system("echo out > /sys/class/gpio/gpio9/direction");
		FILE_LOG(logINFO, ("gpio pins defined\n"));
		gpioDefined = 1;
	}else FILE_LOG(logDEBUG1, ("gpio pins already defined earlier\n"));
}

void FPGAdontTouchFlash(){
	//tell FPGA to not touch flash
	system("echo 0 > /sys/class/gpio/gpio9/value");
	//usleep(100*1000);
}

void FPGATouchFlash(){
	//tell FPGA to touch flash to program itself
	system("echo 1 > /sys/class/gpio/gpio9/value");
}

void resetFPGA(){
    FILE_LOG(logINFOBLUE, ("Reseting FPGA\n"));
	FPGAdontTouchFlash();
	FPGATouchFlash();
	usleep(CTRL_SRVR_INIT_TIME_US);
}

void eraseFlash(){
    FILE_LOG(logDEBUG1, ("Erasing Flash\n"));
	char command[255];
	memset(command, 0, 255);
	sprintf(command,"flash_eraseall %s",mtdvalue);
	system(command);
	FILE_LOG(logINFO, ("Flash erased\n"));
}

int startWritingFPGAprogram(FILE** filefp){
    FILE_LOG(logDEBUG1, ("Start Writing of FPGA program\n"));

	//getting the drive
	//root:/>  cat /proc/mtd
	//dev:    size   erasesize  name
	//mtd0: 00040000 00020000 "bootloader(nor)"
	//mtd1: 00100000 00020000 "linux kernel(nor)"
	//mtd2: 002c0000 00020000 "file system(nor)"
	//mtd3: 01000000 00010000 "bitfile(spi)"
	char output[255];
	memset(output, 0, 255);
	FILE* fp = popen("awk \'$4== \"\\\"bitfile(spi)\\\"\" {print $1}\' /proc/mtd", "r");
	if (fp == NULL) {
	    FILE_LOG(logERROR, ("popen returned NULL. Need that to get mtd drive.\n"));
		return 1;
	}
	if (fgets(output, sizeof(output), fp) == NULL) {
	    FILE_LOG(logERROR, ("fgets returned NULL. Need that to get mtd drive.\n"));
		return 1;
	}
	pclose(fp);
	memset(mtdvalue, 0, MTDSIZE);
	strcpy(mtdvalue,"/dev/");
	char* pch = strtok(output,":");
	if(pch == NULL){
	    FILE_LOG(logERROR, ("Could not get mtd value\n"));
		return 1;
	}
	strcat(mtdvalue,pch);
	FILE_LOG(logINFO, ("Flash drive found: %s\n", mtdvalue));

	FPGAdontTouchFlash();

	//writing the program to flash
	*filefp = fopen(mtdvalue, "w");
	if(*filefp == NULL){
	    FILE_LOG(logERROR, ("Unable to open %s in write mode\n", mtdvalue));
		return 1;
	}
	FILE_LOG(logINFO, ("Flash ready for writing\n"));

	return 0;
}

void stopWritingFPGAprogram(FILE* filefp){
    FILE_LOG(logDEBUG1, ("Stopping of writing FPGA program\n"));

	int wait = 0;
	if(filefp!= NULL){
		fclose(filefp);
		wait = 1;
	}

	//touch and program
	FPGATouchFlash();

	if(wait){
	    FILE_LOG(logDEBUG1, ("Waiting for FPGA to program from flash\n"));
		//waiting for success or done
		char output[255];
		int res=0;
		while(res == 0){
			FILE* sysFile = popen("cat /sys/class/gpio/gpio7/value", "r");
			fgets(output, sizeof(output), sysFile);
			pclose(sysFile);
			sscanf(output,"%d",&res);
			FILE_LOG(logDEBUG1, ("gpi07 returned %d\n", res));
		}
	}
	FILE_LOG(logINFO, ("FPGA has picked up the program from flash\n"));
}

int writeFPGAProgram(char* fpgasrc, uint64_t fsize, FILE* filefp){
    FILE_LOG(logDEBUG1, ("Writing of FPGA Program\n"
            "\taddress of fpgasrc:%p\n"
            "\tfsize:%llu\n\tpointer:%p\n",
            (void *)fpgasrc, (long long unsigned int)fsize, (void*)filefp));

	if(fwrite((void*)fpgasrc , sizeof(char) , fsize , filefp )!= fsize){
	    FILE_LOG(logERROR, ("Could not write FPGA source to flash (size:%llu)\n", (long long unsigned int)fsize));
		return 1;
	}
	FILE_LOG(logDEBUG1, ("program written to flash\n"));
	return 0;
}
