#pragma once

#include "ansi.h"

#include <unistd.h> 	// usleep
#include <string.h>


/* global variables */
#define MTDSIZE                     10

int gpioDefined = 0;
char mtdvalue[MTDSIZE] = {0};


/**
 * Define GPIO pins if not defined
 */
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

/**
 * Notify FPGA to not touch flash
 */
void FPGAdontTouchFlash(){
	//tell FPGA to not touch flash
	system("echo 0 > /sys/class/gpio/gpio9/value");
	//usleep(100*1000);
}


/**
 * Notify FPGA to program from flash
 */
void FPGATouchFlash(){
	//tell FPGA to touch flash to program itself
	system("echo 1 > /sys/class/gpio/gpio9/value");
}

/**
 * Reset FPGA
 */
void resetFPGA(){
    FILE_LOG(logINFOBLUE, ("Reseting FPGA\n"));
	FPGAdontTouchFlash();
	FPGATouchFlash();
	usleep(CTRL_SRVR_INIT_TIME_US);
}

/**
 * Erasing flash
 */
void eraseFlash(){
    FILE_LOG(logDEBUG1, ("Erasing Flash\n"));
	char command[255];
	memset(command, 0, 255);
	sprintf(command,"flash_eraseall %s",mtdvalue);
	system(command);
	FILE_LOG(logINFO, ("Flash erased\n"));
}

/**
 * Open the drive to copy program and
 * notify FPGA not to touch the program
 * @param filefp pointer to flash
 * @return 0 for success, 1 for fail (cannot open file for writing program)
 */
int startWritingFPGAprogram(FILE** filefp){
    FILE_LOG(logDEBUG1, ("Start Writing of FPGA program\n"));

	//getting the drive
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

/**
 * When done writing the program, close file pointer and
 * notify FPGA to pick up the program from flash
 * @param filefp pointer to flash
 */
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


/**
 * Write FPGA Program to flash
 * @param fpgasrc source program
 * @param fsize size of program
 * @param filefp pointer to flash
 * @return 0 for success, 1 for fail (cannot write)
 */
int writeFPGAProgram(char* fpgasrc, size_t fsize, FILE* filefp){
    FILE_LOG(logDEBUG1, ("Writing of FPGA Program\n"
            "\taddress of fpgasrc:%p\n"
            "\tfsize:%lu\n\tpointer:%p\n",
            (void *)fpgasrc, fsize, (void*)filefp));

	if(fwrite((void*)fpgasrc , sizeof(char) , fsize , filefp )!= fsize){
	    FILE_LOG(logERROR, ("Could not write FPGA source to flash (size:%lu)\n", fsize));
		return 1;
	}
	FILE_LOG(logDEBUG1, ("program written to flash\n"));
	return 0;
}
