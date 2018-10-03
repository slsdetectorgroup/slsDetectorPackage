#ifndef PROGRAM_FPGA_H
#define PROGRAM_FPGA_H

#include "ansi.h"

#include <stdio.h>
#include <unistd.h> 	// usleep
#include <string.h>


/* global variables */
#define CTRL_SRVR_INIT_TIME_US		(300 * 1000)
int gpioDefined=0;
#define MTDSIZE 10
char mtdvalue[MTDSIZE];



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
		printf("gpio pins defined\n");
		gpioDefined = 1;
	}else printf("gpio pins already defined earlier\n");
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
	cprintf(BLUE,"\n*** Reseting FPGA ***\n");
	FPGAdontTouchFlash();
	FPGATouchFlash();
	usleep(CTRL_SRVR_INIT_TIME_US);
}

/**
 * Erasing flash
 */
void eraseFlash(){
#ifdef VERY_VERBOSE
	printf("\nErasing Flash\n");
#endif
	char command[255];
	memset(command, 0, 255);
	sprintf(command,"flash_eraseall %s",mtdvalue);
	system(command);
	printf("Flash erased\n");
}

/**
 * Open the drive to copy program and
 * notify FPGA not to touch the program
 * @param filefp pointer to flash
 * @return 0 for success, 1 for fail (cannot open file for writing program)
 */
int startWritingFPGAprogram(FILE** filefp){
#ifdef VERY_VERBOSE
	printf("\nStart Writing of FPGA program\n");
#endif

	//getting the drive
	char output[255];
	memset(output, 0, 255);
	FILE* fp = popen("awk \'$4== \"\\\"bitfile(spi)\\\"\" {print $1}\' /proc/mtd", "r");
	if (fp == NULL) {
		cprintf(RED,"popen returned NULL. Need that to get mtd drive.\n");
		return 1;
	}
	if (fgets(output, sizeof(output), fp) == NULL) {
		cprintf(RED,"fgets returned NULL. Need that to get mtd drive.\n");
		return 1;
	}
	pclose(fp);
	//cprintf(RED,"output: %s\n", output);
	memset(mtdvalue, 0, MTDSIZE);
	strcpy(mtdvalue,"/dev/");
	char* pch = strtok(output,":");
	if(pch == NULL){
		cprintf(RED,"Could not get mtd value\n");
		return 1;
	}
	strcat(mtdvalue,pch);
	printf ("\nFlash drive found: %s\n",mtdvalue);

	FPGAdontTouchFlash();

	//writing the program to flash
	*filefp = fopen(mtdvalue, "w");
	if(*filefp == NULL){
		cprintf(RED,"Unable to open %s in write mode\n",mtdvalue);
		return 1;
	}
	printf("Flash ready for writing\n");

	return 0;
}

/**
 * When done writing the program, close file pointer and
 * notify FPGA to pick up the program from flash
 * @param filefp pointer to flash
 */
void stopWritingFPGAprogram(FILE* filefp){
#ifdef VERY_VERBOSE
	printf("\nStopping of writing FPGA program\n");
#endif

	int wait = 0;
	if(filefp!= NULL){
		fclose(filefp);
		wait = 1;
	}

	//touch and program
	FPGATouchFlash();

	if(wait){
#ifdef VERY_VERBOSE
		printf("Waiting for FPGA to program from flash\n");
#endif
		//waiting for success or done
		char output[255];
		int res=0;
		while(res == 0){
			FILE* sysFile = popen("cat /sys/class/gpio/gpio7/value", "r");
			fgets(output, sizeof(output), sysFile);
			pclose(sysFile);
			sscanf(output,"%d",&res);
#ifdef VERY_VERBOSE
			printf("gpi07 returned %d\n",res);
#endif
		}
	}
	printf("FPGA has picked up the program from flash\n\n");
}


/**
 * Write FPGA Program to flash
 * @param fpgasrc source program
 * @param fsize size of program
 * @param filefp pointer to flash
 * @return 0 for success, 1 for fail (cannot write)
 */
int writeFPGAProgram(char* fpgasrc, size_t fsize, FILE* filefp){
#ifdef VERY_VERBOSE
	printf("\nWriting of FPGA Program\n");
	cprintf(BLUE,"address of fpgasrc:%p\n",(void *)fpgasrc);
	cprintf(BLUE,"fsize:%lu\n",fsize);
	cprintf(BLUE,"pointer:%p\n",(void*)filefp);
#endif

	if(fwrite((void*)fpgasrc , sizeof(char) , fsize , filefp )!= fsize){
		cprintf(RED,"Could not write FPGA source to flash (size:%lu)\n", fsize);
		return 1;
	}
#ifdef VERY_VERBOSE
	cprintf(BLUE, "program written to flash\n");
#endif
	return 0;
}

#endif	//PROGRAM_FPGA_H
