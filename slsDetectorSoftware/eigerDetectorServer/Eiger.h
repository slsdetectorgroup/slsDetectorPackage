 
/**
 * @author Ian Johnson
 * @version 1.0
 */


#ifndef EIGER_H
#define EIGER_H

#include <string>
#include <vector>

#include "Feb.h"


class Module{
  
 private:
  unsigned int module_number;
  bool         top_address_valid;
  unsigned int top_left_address;
  unsigned int top_right_address;
  bool         bottom_address_valid;
  unsigned int bottom_left_address;
  unsigned int bottom_right_address;

  unsigned int idelay_top[4];    //ll,lr,rl,ll
  unsigned int idelay_bottom[4]; //ll,lr,rl,ll
  float        high_voltage;
  float*       top_dac;
  float*       bottom_dac;

 public:
  Module(unsigned int number, unsigned int address_top); //for half module()
  Module(unsigned int number, unsigned int address_top, unsigned int address_bottom);
  ~Module();

  static const unsigned int ndacs;
  static const std::string  dac_names[16];

  unsigned int GetModuleNumber()       {return module_number;}
  bool         TopAddressIsValid()     {return top_address_valid;}
  unsigned int GetTopBaseAddress()     {return (top_left_address&0xff);}
  unsigned int GetTopLeftAddress()     {return top_left_address;}
  unsigned int GetTopRightAddress()    {return top_right_address;}
  unsigned int GetBottomBaseAddress()  {return (bottom_left_address&0xff);}
  bool         BottomAddressIsValid()  {return bottom_address_valid;}
  unsigned int GetBottomLeftAddress()  {return bottom_left_address;}
  unsigned int GetBottomRightAddress() {return bottom_right_address;}
  
  unsigned int SetTopIDelay(unsigned int chip,unsigned int value)    { return TopAddressIsValid() &&chip<4        ? (idelay_top[chip]=value)    : 0;} //chip 0=ll,1=lr,0=rl,1=rr
  unsigned int GetTopIDelay(unsigned int chip)                       { return chip<4                              ?  idelay_top[chip]           : 0;} //chip 0=ll,1=lr,0=rl,1=rr
  unsigned int SetBottomIDelay(unsigned int chip,unsigned int value) { return BottomAddressIsValid() &&chip<4     ? (idelay_bottom[chip]=value) : 0;} //chip 0=ll,1=lr,0=rl,1=rr
  unsigned int GetBottomIDelay(unsigned int chip)                    { return chip<4                              ?  idelay_bottom[chip]        : 0;} //chip 0=ll,1=lr,0=rl,1=rr
 
  float        SetHighVoltage(float value)                           { return TopAddressIsValid()                 ? (high_voltage=value) : -1;}
  float        GetHighVoltage()                                      { return high_voltage;}

  float        SetTopDACVoltage(unsigned int i, float value)         { return (i<ndacs && TopAddressIsValid())    ? (top_dac[i]=value)    : -1;}
  float        GetTopDACVoltage(unsigned int i)                      { return (i<ndacs) ? top_dac[i]:-1;}
  float        SetBottomDACVoltage(unsigned int i, float value)      { return (i<ndacs && BottomAddressIsValid()) ? (bottom_dac[i]=value) : -1;}
  float        GetBottomDACVoltage(unsigned int i)                   { return (i<ndacs) ? bottom_dac[i]:-1;}

};



class Eiger:private Feb{

 private:
  
  std::vector<Module*> modules;
  void ClearModules();

  unsigned int staticBits;   //program=1,m4=2,m8=4,test=8,rotest=16,cs_bar_left=32,cs_bar_right=64 
  unsigned int acquireNReadoutMode; //safe or parallel, half or full speed
  unsigned int triggerMode;         //internal timer, external start, external window, signal polarity (external trigger and enable)
  unsigned int externalEnableMode;  //external enabling engaged and it's polarity 
  unsigned int subFrameMode;

  unsigned int photon_energy_eV;

  unsigned int nimages;
  float        exposure_time_in_sec;
  float        exposure_period_in_sec;

  unsigned int   trimbit_size;
  unsigned char* last_downloaded_trimbits;

  void PrintModuleList();
  bool GetModuleIndex(unsigned int module_number, unsigned int& module_index);
  bool CheckModuleAddresses(unsigned int top_address, unsigned int bottom_address);
  bool AddModule(unsigned int module_number, unsigned int top_address);
  bool AddModule(unsigned int module_number, unsigned int top_address, unsigned int bottom_address, bool half_module=0);

  bool  GetDACNumber(std::string s, unsigned int& n);
  bool  SendDACValue(unsigned int dst_num, unsigned int ch, float& value);
  bool  VoltageToDAC(float value, unsigned int& digital, unsigned int nsteps, float vmin, float vmax);
  float DACToVoltage(unsigned int digital,unsigned int nsteps,float vmin,float vmax);

  bool SendHighVoltage(unsigned int module_index, float& value);

  bool SendIDelays(unsigned int dst_num, bool chip_lr, unsigned int channels, unsigned int ndelay_units);

  bool SetStaticBits();
  bool SetStaticBits(unsigned int the_static_bits);

  unsigned int ConvertTimeToRegister(float time_in_sec);

  bool SetCommandRegister(unsigned int cmd);
  bool GetDAQStatusRegister(int socket_num, unsigned int &ret_status);
  bool StartDAQOnlyNWaitForFinish(int sleep_time_us=5000);
  bool ResetDataStream();

  bool ResetChipCompletely();

 public:
  Eiger();
  virtual ~Eiger();

  bool Init();
  bool ReadSetUpFileToAddModules(std::string file_name);
  bool ReadSetUpFile(unsigned int module_num, std::string file_name);
  bool CheckSetup();

  unsigned int GetNModules();
  unsigned int GetNHalfModules();

  //bool SetHighVoltage(float value);
  bool SetHighVoltage(unsigned int module_num,float value);

  bool         SetPhotonEnergy(unsigned int full_energy_eV);
  unsigned int GetPhotonEnergy(){return photon_energy_eV;}

  bool SetIDelays(unsigned int module_num, unsigned int  ndelay_units);
  bool SetIDelays(unsigned int module_num, unsigned int chip_pos, unsigned int ndelay_units);

  bool SetDAC(std::string s, float value);
  bool GetDAC(std::string s, float& ret_value);
  bool GetDACName(unsigned int dac_num, std::string &s);

  bool           SetTrimbits(unsigned char* trimbits);
  unsigned char* GetTrimbits();


  
  bool Reset();
  bool StartAcquisition();
  bool StopAcquisition();
  bool AcquisitionInProgress();
  bool WaitForFinishedFlag(int sleep_time_us=5000);

  //functions for setting up exposure
  void          PrintAcquisitionSetup();
  bool          SetNImages(unsigned int n_images);
  unsigned int  GetNImages();
  bool          SetExposureTime(float the_exposure_time_in_sec);
  float         GetExposureTime();
  bool          SetExposurePeriod(float the_exposure_period_in_sec);
  float         GetExposurePeriod();
  bool          SetDynamicRange(unsigned int four_eight_sixteen_or_thirtytwo);
  unsigned int  GetDynamicRange();
  bool          SetReadoutSpeed(unsigned int readout_speed=0); //0->full,1->half,2->quarter or 3->super_slow
  bool          SetReadoutMode(unsigned int readout_mode=0); //0->parallel,1->non-parallel,2-> safe_mode
  bool          SetTriggerMode(unsigned int trigger_mode=0, bool polarity=1);
  bool          SetExternalEnableMode(bool use_external_enable=0, bool polarity=1);


  //functions for testing
  bool SetTestModeVariable(bool on=1);
  bool GetTestModeVariable();

  bool FebTest(){return Feb::Test();}

};


#endif 
