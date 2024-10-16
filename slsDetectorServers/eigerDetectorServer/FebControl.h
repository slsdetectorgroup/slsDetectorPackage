// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "FebInterface.h"
#include "slsDetectorServer_defs.h"
#include <netinet/in.h>

// setup
void Feb_Control_activate(int activate);
int Feb_Control_FebControl(int normal);
int Feb_Control_OpenSerialCommunication();
void Feb_Control_CloseSerialCommunication();
int Feb_Control_CheckSetup();
unsigned int Feb_Control_AddressToAll();
int Feb_Control_SetCommandRegister(unsigned int cmd);
int Feb_Control_SetStaticBits();
int Feb_Control_SetStaticBits1(unsigned int the_static_bits);
int Feb_Control_SetInTestModeVariable(int on);
int Feb_Control_GetTestModeVariable();
// idelays
int Feb_Control_SetIDelays(unsigned int ndelay_units);
int Feb_Control_SetIDelays1(unsigned int chip_pos, unsigned int ndelay_units);
int Feb_Control_SendIDelays(unsigned int dst_num, int chip_lr,
                            unsigned int channels, unsigned int ndelay_units);
// high voltage
int Feb_Control_SetHighVoltage(int value);
int Feb_Control_GetHighVoltage(int *value);
int Feb_Control_SendHighVoltage(int dacvalue);
int Feb_Control_ReceiveHighVoltage(unsigned int *value);
// dacs
int Feb_Control_VoltageToDAC(float value, unsigned int *digital,
                             unsigned int nsteps, float vmin, float vmax);
float Feb_Control_DACToVoltage(unsigned int digital, unsigned int nsteps,
                               float vmin, float vmax);
int Feb_Control_SetDAC(unsigned int ch, int value);

// trimbits
int Feb_Control_SetTrimbits(unsigned int *trimbits, int top);
int Feb_Control_SaveAllTrimbitsTo(int value, int top);
unsigned int *Feb_Control_GetTrimbits();

// acquisition
int Feb_Control_AcquisitionInProgress();
int Feb_Control_ProcessingInProgress();
int Feb_Control_AcquisitionStartedBit();
int Feb_Control_WaitForStartedFlag(int sleep_time_us, int prev_flag);
int Feb_Control_WaitForFinishedFlag(int sleep_time_us, int tempLock);
int Feb_Control_GetDAQStatusRegister(unsigned int dst_address,
                                     unsigned int *ret_status);
int Feb_Control_StartDAQOnlyNWaitForFinish(int sleep_time_us);
int Feb_Control_Reset();
int Feb_Control_ResetChipCompletely();
int Feb_Control_ResetChipPartially();
int Feb_Control_SendBitModeToBebServer();
unsigned int Feb_Control_ConvertTimeToRegister(float time_in_sec);
int Feb_Control_PrepareForAcquisition();
int Feb_Control_PrintAcquisitionSetup();
int Feb_Control_StartAcquisition();
int Feb_Control_StopAcquisition();
int Feb_Control_IsReadyForTrigger(int *readyForTrigger);
int Feb_Control_SendSoftwareTrigger();
int Feb_Control_SoftwareTrigger(int block);

// parameters
int Feb_Control_SetDynamicRange(int dr);
int Feb_Control_GetDynamicRange(int *retval);
int Feb_Control_Disable16bitConversion(int disable);
int Feb_Control_Get16bitConversionDisabled();
int Feb_Control_SetReadoutSpeed(unsigned int readout_speed);
int Feb_Control_SetReadoutMode(unsigned int readout_mode);
int Feb_Control_SetTriggerMode(unsigned int trigger_mode);
int Feb_Control_SetExternalEnableMode(int use_external_enable, int polarity);
int Feb_Control_SetNExposures(unsigned int n_images);
unsigned int Feb_Control_GetNExposures();
int Feb_Control_SetExposureTime(double the_exposure_time_in_sec);
double Feb_Control_GetExposureTime();
int64_t Feb_Control_GetExposureTime_in_nsec();
int Feb_Control_SetSubFrameExposureTime(
    int64_t the_subframe_exposure_time_in_10nsec);
int64_t Feb_Control_GetSubFrameExposureTime();
int Feb_Control_SetSubFramePeriod(int64_t the_subframe_period_in_10nsec);
int64_t Feb_Control_GetSubFramePeriod();
int Feb_Control_SetExposurePeriod(double the_exposure_period_in_sec);
double Feb_Control_GetExposurePeriod();
void Feb_Control_Set_Counter_Bit(int value);
int Feb_Control_Get_Counter_Bit();
int Feb_Control_SetInterruptSubframe(int val);
int Feb_Control_GetInterruptSubframe();
int Feb_Control_SetTop(enum TOPINDEX ind, int left, int right);
int Feb_Control_SetMaster(enum MASTERINDEX ind);
int Feb_Control_SetMasterEffects(int master, int controlServer);
int Feb_Control_SetQuad(int val);
int Feb_Control_SetChipSignalsToTrimQuad(int enable);
int Feb_Control_SetReadNRows(int value);
int Feb_Control_GetReadNRows();
int Feb_Control_WriteRegister(uint32_t offset, uint32_t data, int validate);
int Feb_Control_ReadRegister(uint32_t offset, uint32_t *retval);
int Feb_Control_WriteRegister_BitMask(uint32_t offset, uint32_t data,
                                      uint32_t bitmask, int validate);
int Feb_Control_ReadRegister_BitMask(uint32_t offset, uint32_t *retval,
                                     uint32_t bitmask);
// pulsing
int Feb_Control_Pulse_Pixel(int npulses, int x, int y);
int Feb_Control_PulsePixelNMove(int npulses, int inc_x_pos, int inc_y_pos);
int Feb_Control_Shift32InSerialIn(unsigned int value_to_shift_in);
int Feb_Control_SendTokenIn();
int Feb_Control_ClockRowClock(unsigned int ntimes);
int Feb_Control_PulseChip(int npulses);

// rate correction
int64_t Feb_Control_Get_RateTable_Tau_in_nsec();
int64_t Feb_Control_Get_RateTable_Period_in_nsec();
int Feb_Control_SetRateCorrectionTau(int64_t tau_in_Nsec);
int Feb_Control_SetRateCorrectionTable(unsigned int *table);
int Feb_Control_GetRateCorrectionVariable();
void Feb_Control_SetRateCorrectionVariable(int activate_rate_correction);
int Feb_Control_PrintCorrectedValues();

// adcs
int Feb_Control_GetLeftFPGATemp();
int Feb_Control_GetRightFPGATemp();
int64_t Feb_Control_GetFrontLeftFirmwareVersion();
int64_t Feb_Control_GetFrontRightFirmwareVersion();
int64_t Feb_Control_GetMeasuredPeriod();
int64_t Feb_Control_GetSubMeasuredPeriod();
