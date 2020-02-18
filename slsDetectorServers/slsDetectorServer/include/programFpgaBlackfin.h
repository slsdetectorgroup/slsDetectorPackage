#pragma once

#include <stdio.h>
#include <stdint.h>

/**
 * Define GPIO pins if not defined
 */
void defineGPIOpins();

/**
 * Notify FPGA to not touch flash
 */
void FPGAdontTouchFlash();

/**
 * Notify FPGA to program from flash
 */
void FPGATouchFlash();

/**
 * Reset FPGA
 */
void resetFPGA();

/**
 * Erasing flash
 */
void eraseFlash();

/**
 * Open the drive to copy program and
 * notify FPGA not to touch the program
 * @param filefp pointer to flash
 * @return 0 for success, 1 for fail (cannot open file for writing program)
 */
int startWritingFPGAprogram(FILE** filefp);

/**
 * When done writing the program, close file pointer and
 * notify FPGA to pick up the program from flash
 * @param filefp pointer to flash
 */
void stopWritingFPGAprogram(FILE* filefp);

/**
 * Write FPGA Program to flash
 * @param fpgasrc source program
 * @param fsize size of program
 * @param filefp pointer to flash
 * @return 0 for success, 1 for fail (cannot write)
 */
int writeFPGAProgram(char* fpgasrc, uint64_t fsize, FILE* filefp);
