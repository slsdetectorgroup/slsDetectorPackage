#pragma once

#include <stdint.h>
#include <stdio.h>

#define NIOS_MAX_APP_IMAGE_SIZE (0x00580000)

/** Notify microcontroller of successful server start up */
void NotifyServerStartSuccess();

/** reset fpga and controller(only implemented for >= v1.1 boards) */
void rebootControllerAndFPGA();

/** finds the right mtd drive
 * @param mess error message
 * @returns ok or fail
 */
int findFlash(char *mess);

/** erase flash */
void eraseFlash();

/** erase and write flash
 * @param mess error message
 * @param fpgasrc program source
 * @param fsize file size
 * @returns ok or fail
 */
int eraseAndWriteToFlash(char *mess, char *fpgasrc, uint64_t fsize);

/**
 * Write FPGA Program to flash
 * @param mess error message
 * @param fpgasrc source program
 * @param fsize size of program
 * @param filefp pointer to flash
 * @return ok or fail
 */
int writeFPGAProgram(char *mess, char *fpgasrc, uint64_t fsize, FILE *filefp);
