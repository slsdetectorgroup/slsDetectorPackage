 
/**
 * @author Ian Johnson
 * @version 1.0
 */


#ifndef FEBCONTROL_H
#define FEBCONTROL_H

#include <netinet/in.h> 
#include <stdlib.h>
//#include <string>
//#include <vector>


#include "FebInterface.h"


struct Module{
	  unsigned int module_number;
	  int         top_address_valid;
	  unsigned int top_left_address;
	  unsigned int top_right_address;
	  int         bottom_address_valid;
	  unsigned int bottom_left_address;
	  unsigned int bottom_right_address;

	  unsigned int idelay_top[4];    //ll,lr,rl,ll
	  unsigned int idelay_bottom[4]; //ll,lr,rl,ll
	  float        high_voltage;
	  int*         top_dac;
	  int*         bottom_dac;
};


void Module_Module(struct Module* mod,unsigned int number, unsigned int address_top);
void Module_ModuleBottom(struct Module* mod,unsigned int number, unsigned int address_bottom);
void Module_Module1(struct Module* mod,unsigned int number, unsigned int address_top, unsigned int address_bottom);
unsigned int Module_GetModuleNumber(struct Module* mod);
int         Module_TopAddressIsValid(struct Module* mod);
unsigned int Module_GetTopBaseAddress(struct Module* mod);
unsigned int Module_GetTopLeftAddress(struct Module* mod) ;
unsigned int Module_GetTopRightAddress(struct Module* mod);
unsigned int Module_GetBottomBaseAddress(struct Module* mod);
int         Module_BottomAddressIsValid(struct Module* mod);
unsigned int Module_GetBottomLeftAddress(struct Module* mod);
unsigned int Module_GetBottomRightAddress(struct Module* mod);
unsigned int Module_SetTopIDelay(struct Module* mod,unsigned int chip,unsigned int value);
unsigned int Module_GetTopIDelay(struct Module* mod,unsigned int chip) ;
unsigned int Module_SetBottomIDelay(struct Module* mod,unsigned int chip,unsigned int value);
unsigned int Module_GetBottomIDelay(struct Module* mod,unsigned int chip);

float        Module_SetHighVoltage(struct Module* mod,float value);
float        Module_GetHighVoltage(struct Module* mod);

int          Module_SetTopDACValue(struct Module* mod,unsigned int i, int value);
int          Module_GetTopDACValue(struct Module* mod,unsigned int i);
int          Module_SetBottomDACValue(struct Module* mod,unsigned int i, int value);
int          Module_GetBottomDACValue(struct Module* mod,unsigned int i);







int Feb_Control_IsBottomModule();
int Feb_Control_GetModuleNumber();




  void Feb_Control_PrintModuleList();
  int Feb_Control_GetModuleIndex(unsigned int module_number, unsigned int* module_index);
  int Feb_Control_CheckModuleAddresses(struct Module* m);
  int Feb_Control_AddModule(unsigned int module_number, unsigned int top_address);
  /*int Feb_Control_AddModule(unsigned int module_number, unsigned int top_address, unsigned int bottom_address, int half_module=0);*/
  int Feb_Control_AddModule1(unsigned int module_number, int top_enable, unsigned int top_address, unsigned int bottom_address, int half_module);

  int  Feb_Control_GetDACNumber(char* s, unsigned int* n);
  int  Feb_Control_SendDACValue(unsigned int dst_num, unsigned int ch, unsigned int* value);
  int  Feb_Control_VoltageToDAC(float value, unsigned int* digital, unsigned int nsteps, float vmin, float vmax);
  float Feb_Control_DACToVoltage(unsigned int digital,unsigned int nsteps,float vmin,float vmax);

  int Feb_Control_SendHighVoltage(unsigned int module_index, float* value);

  int Feb_Control_SendIDelays(unsigned int dst_num, int chip_lr, unsigned int channels, unsigned int ndelay_units);

  int Feb_Control_SetStaticBits();
  int Feb_Control_SetStaticBits1(unsigned int the_static_bits);

  int Feb_Control_SendBitModeToBebServer();

  unsigned int Feb_Control_ConvertTimeToRegister(float time_in_sec);

  unsigned int Feb_Control_AddressToAll();
  int Feb_Control_SetCommandRegister(unsigned int cmd);
  int Feb_Control_GetDAQStatusRegister(unsigned int dst_address, unsigned int* ret_status);
  /*int Feb_Control_StartDAQOnlyNWaitForFinish(int sleep_time_us=5000);*/
  int Feb_Control_StartDAQOnlyNWaitForFinish(int sleep_time_us);

  int Feb_Control_ResetChipCompletely();
  int Feb_Control_ResetChipPartially();

  //struct sockaddr_in Feb_Control_serv_addr;
 /*
  int Feb_Control_SetupSendToSocket(const char* ip_address_hostname, unsigned short int port);
  int  Feb_Control_WriteNRead(char* message, int length, int max_length);
*/


  void Feb_Control_FebControl();
  int Feb_Control_Init(int master, int top, int module_num);
  int Feb_Control_ReadSetUpFileToAddModules(char* file_name);
  int Feb_Control_ReadSetUpFile(unsigned int module_num, char* file_name);
  int Feb_Control_CheckSetup();

  unsigned int Feb_Control_GetNModules();
  unsigned int Feb_Control_GetNHalfModules();

  int Feb_Control_SetHighVoltage(float value);
  int Feb_Control_SetHighVoltage1(unsigned int module_num,float value);


  int Feb_Control_SetIDelays(unsigned int module_num, unsigned int  ndelay_units);
  int Feb_Control_SetIDelays1(unsigned int module_num, unsigned int chip_pos, unsigned int ndelay_units);
  
  int Feb_Control_DecodeDACString(char* dac_str, unsigned int* module_index, int* top, int* bottom, unsigned int* dac_ch);
  /*int Feb_Control_SetDAC(string s, int value, int is_a_voltage_mv=0);*/
  int Feb_Control_SetDAC(char* s, int value, int is_a_voltage_mv);
 /* int Feb_Control_GetDAC(string s, int* ret_value, int voltage_mv=0);*/
  int Feb_Control_GetDAC(char* s, int* ret_value, int voltage_mv);
  int Feb_Control_GetDACName(unsigned int dac_num,char* s);

  int Feb_Control_SetTrimbits(unsigned int module_num, unsigned int* trimbits);
  unsigned int* Feb_Control_GetTrimbits();


  /**Added by Dhanya */
  int Feb_Control_SaveAllTrimbitsTo(int value);

  

  int Feb_Control_Reset();
  int Feb_Control_PrepareForAcquisition();

  int Feb_Control_StartAcquisition();
  int Feb_Control_StopAcquisition();
  int Feb_Control_AcquisitionInProgress();
  int Feb_Control_AcquisitionStartedBit();
  /*int Feb_Control_WaitForFinishedFlag(int sleep_time_us=5000);*/
  int Feb_Control_WaitForFinishedFlag(int sleep_time_us);
  int Feb_Control_WaitForStartedFlag(int sleep_time_us, int prev_flag);

  //functions for setting up exposure
  void Feb_Control_PrintAcquisitionSetup();
  int Feb_Control_SetNExposures(unsigned int n_images);
  unsigned int Feb_Control_GetNExposures();
  int Feb_Control_SetExposureTime(double the_exposure_time_in_sec);
  double Feb_Control_GetExposureTime();
  int Feb_Control_SetSubFrameExposureTime(int64_t the_subframe_exposure_time_in_10nsec);
  int64_t Feb_Control_GetSubFrameExposureTime();
  int Feb_Control_SetExposurePeriod(double the_exposure_period_in_sec);
  double Feb_Control_GetExposurePeriod();
  int Feb_Control_SetDynamicRange(unsigned int four_eight_sixteen_or_thirtytwo);
  unsigned int Feb_Control_GetDynamicRange();
  int Feb_Control_SetReadoutSpeed(unsigned int readout_speed); //0 was default, 0->full,1->half,2->quarter or 3->super_slow
  int Feb_Control_SetReadoutMode(unsigned int readout_mode); ///0 was default,0->parallel,1->non-parallel,2-> safe_mode
  int Feb_Control_SetTriggerMode(unsigned int trigger_mode, int polarity);//0 and 1 was default,
  int Feb_Control_SetExternalEnableMode(int use_external_enable, int polarity);//0 and 1 was default,

  //functions for testing
  /*int Feb_Control_SetTestModeVariable(int on=1);*/
  int Feb_Control_SetInTestModeVariable(int on);
  int Feb_Control_GetTestModeVariable();

  void Feb_Control_Set_Counter_Bit(int value);
  int Feb_Control_Get_Counter_Bit();
  int Feb_Control_Pulse_Pixel(int npulses,int x, int y);
  int Feb_Control_PulsePixelNMove(int npulses, int inc_x_pos, int inc_y_pos);
  int Feb_Control_Shift32InSerialIn(unsigned int value_to_shift_in);
  int Feb_Control_SendTokenIn();
  int Feb_Control_ClockRowClock(unsigned int ntimes);
  int Feb_Control_PulseChip(int npulses);

  int64_t Feb_Control_Get_RateTable_Tau_in_nsec();
  int64_t Feb_Control_Get_RateTable_Subexptime_in_nsec();
  int Feb_Control_SetRateCorrectionTau(int64_t tau_in_Nsec);
  int Feb_Control_SetRateCorrectionTable(unsigned int *table);
  int Feb_Control_GetRateCorrectionVariable();
  void Feb_Control_SetRateCorrectionVariable(int activate_rate_correction);
  int Feb_Control_PrintCorrectedValues();

#endif
