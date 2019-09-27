#pragma once

#include <sys/types.h>
#include <inttypes.h>


/**
 * Write into a 32 bit register
 * @param offset address offset
 * @param data 32 bit data
 */
void bus_w(u_int32_t offset, u_int32_t data);

/**
 * Read from a 32 bit register
 * @param offset address offset
 * @retuns 32 bit data read
 */
u_int32_t bus_r(u_int32_t offset);

/**
 * Read from a 64 bit register
 * @param aLSB LSB offset address
 * @param aMSB MSB offset address
 * @returns 64 bit data read
 */
int64_t get64BitReg(int aLSB, int aMSB);

/**
 * Write into a 64 bit register
 * @param value 64 bit data
 * @param aLSB LSB offset address
 * @param aMSB MSB offset address
 * @returns 64 bit data read
 */
int64_t set64BitReg(int64_t value, int aLSB, int aMSB);

/**
 * Read unsigned 64 bit from a 64 bit register
 * @param aLSB LSB offset address
 * @param aMSB MSB offset address
 * @returns unsigned 64 bit data read
 */
uint64_t getU64BitReg(int aLSB, int aMSB);

/**
 * Write unsigned 64 bit into a 64 bit register
 * @param value unsigned 64 bit data
 * @param aLSB LSB offset address
 * @param aMSB MSB offset address
 */
void setU64BitReg(uint64_t value, int aLSB, int aMSB);

/**
 * Read from a 32 bit register (literal register value provided by client)
 * @param offset address offset
 * @retuns 32 bit data read
 */
u_int32_t readRegister(u_int32_t offset);

/**
 * Write into a 32 bit register (literal register value provided by client)
 * @param offset address offset
 * @param data 32 bit data
 */
u_int32_t writeRegister(u_int32_t offset, u_int32_t data);

/**
 * Map FPGA
 */
int mapCSP0(void);

/**
 * Get Nios base address
 */
u_int32_t* Nios_getBaseAddress();
