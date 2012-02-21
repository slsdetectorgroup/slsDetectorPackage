
#ifndef SLS_DETECTOR_COMMAND_H
#define SLS_DETECTOR_COMMAND_H


#include "sls_detector_defs.h"
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

class slsDetectorCommand;



/* /\** This class handles the command line I/Os, help etc.  */
/*     It is inherited by both slsDetector and multiSlsDetector */

/* *\/ */
 class slsDetectorCommand { 

 public:


   slsDetectorCommand();


  /*   /\** */
/*      executes a set of string arguments according to a given format. It is used to read/write configuration file, dump and retrieve detector settings and for the command line interface command parsing */
/*      \param narg number of arguments */
/*      \param args array of string arguments */
/*      \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition) */
/*      \returns answer string  */
/*   *\/ */
   string executeLine(int narg, char *args[], int action=HELP_ACTION); 
  
/*   /\** */
/*      returns the help for the executeLine command  */
/*      \param os output stream to return the help to */
/*      \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition)  */
/*   *\/ */
   static string helpLine(int narg, char *args[], int action=HELP_ACTION); 
   
   string cmdUnderDevelopment(int narg, char *args[], int action); 

   string cmdUnknown(int narg, char *args[], int action); 

   string cmdAcquire(int narg, char *args[], int action); 
   static string helpAcquire(int narg, char *args[], int action);
   
   string cmdData(int narg, char *args[], int action); 
   static string helpData(int narg, char *args[], int action);
   
   string cmdFrame(int narg, char *args[], int action); 
   static string helpFrame(int narg, char *args[], int action);
   
   string cmdStatus(int narg, char *args[], int action); 
   static string helpStatus(int narg, char *args[], int action);
      
   string cmdFree(int narg, char *args[], int action);
   static string helpFree(int narg, char *args[], int action);
   
   string cmdAdd(int narg, char *args[], int action);
   static string helpAdd(int narg, char *args[], int action);

   string cmdRemove(int narg, char *args[], int action);
   static string helpRemove(int narg, char *args[], int action);

   string cmdHostname(int narg, char *args[], int action);
   static string helpHostname(int narg, char *args[], int action);

   string cmdId(int narg, char *args[], int action);
   static string helpId(int narg, char *args[], int action);

   string cmdMaster(int narg, char *args[], int action);
   static string helpMaster(int narg, char *args[], int action);

   string cmdSync(int narg, char *args[], int action);
   static string helpSync(int narg, char *args[], int action);

   string cmdHelp(int narg, char *args[], int action);

   string cmdExitServer(int narg, char *args[], int action);
   static string helpExitServer(int narg, char *args[], int action);


   string cmdSettingsDir(int narg, char *args[], int action);
   static string helpSettingsDir(int narg, char *args[], int action);

   string cmdCalDir(int narg, char *args[], int action);
   static string helpCalDir(int narg, char *args[], int action);


   string cmdTrimEn(int narg, char *args[], int action);
   static string helpTrimEn(int narg, char *args[], int action);

   string cmdOutDir(int narg, char *args[], int action);
   static string helpOutDir(int narg, char *args[], int action);

   string cmdFileName(int narg, char *args[], int action);
   static string helpFileName(int narg, char *args[], int action);


   string cmdFileIndex(int narg, char *args[], int action);
   static string helpFileIndex(int narg, char *args[], int action);

   string cmdFlatField(int narg, char *args[], int action);
   static string helpFlatField(int narg, char *args[], int action);


   string cmdRateCorr(int narg, char *args[], int action);
   static string helpRateCorr(int narg, char *args[], int action);


   string cmdBadChannels(int narg, char *args[], int action);
   static string helpBadChannels(int narg, char *args[], int action);


   string cmdAngConv(int narg, char *args[], int action);
   static string helpAngConv(int narg, char *args[], int action);

   string cmdThreaded(int narg, char *args[], int action);
   static string helpThreaded(int narg, char *args[], int action);

   string cmdImage(int narg, char *args[], int action);
   static string helpImage(int narg, char *args[], int action);

   string cmdPositions(int narg, char *args[], int action);
   static string helpPositions(int narg, char *args[], int action);

   string cmdScripts(int narg, char *args[], int action);
   static string helpScripts(int narg, char *args[], int action);

   string cmdScans(int narg, char *args[], int action);
   static string helpScans(int narg, char *args[], int action);


   string cmdNetworkParameter(int narg, char *args[], int action);
   static string helpNetworkParameter(int narg, char *args[], int action);

   string cmdPort(int narg, char *args[], int action);
   static string helpPort(int narg, char *args[], int action);


   string cmdLock(int narg, char *args[], int action);
   static string helpLock(int narg, char *args[], int action);


   string cmdLastClient(int narg, char *args[], int action);
   static string helpLastClient(int narg, char *args[], int action);


   string cmdOnline(int narg, char *args[], int action);
   static string helpOnline(int narg, char *args[], int action);


   string cmdConfigureMac(int narg, char *args[], int action);
   static string helpConfigureMac(int narg, char *args[], int action);


   string cmdDetectorSize(int narg, char *args[], int action);
   static string helpDetectorSize(int narg, char *args[], int action);


   string cmdSettings(int narg, char *args[], int action);
   static string helpSettings(int narg, char *args[], int action);


   string cmdSN(int narg, char *args[], int action);
   static string helpSN(int narg, char *args[], int action);


   string cmdDigiTest(int narg, char *args[], int action);
   static string helpDigiTest(int narg, char *args[], int action);

   string cmdRegister(int narg, char *args[], int action);
   static string helpRegister(int narg, char *args[], int action);


   string cmdDAC(int narg, char *args[], int action);
   static string helpDAC(int narg, char *args[], int action);

   string cmdADC(int narg, char *args[], int action);
   static string helpADC(int narg, char *args[], int action);

   string cmdTimer(int narg, char *args[], int action);
   static string helpTimer(int narg, char *args[], int action);


   string cmdTimeLeft(int narg, char *args[], int action);
   static string helpTimeLeft(int narg, char *args[], int action);

   string cmdSpeed(int narg, char *args[], int action);
   static string helpSpeed(int narg, char *args[], int action);

   string cmdAdvanced(int narg, char *args[], int action);
   static string helpAdvanced(int narg, char *args[], int action);


   string cmdConfiguration(int narg, char *args[], int action);
   static string helpConfiguration(int narg, char *args[], int action);























  virtual int setOnline(int const online=GET_ONLINE_FLAG)=0;
  virtual void acquire(int delflag)=0;
  virtual int* readAll()=0;
  virtual int* readFrame()=0;
  virtual void* processData(int delflag)=0;
  virtual int startAcquisition()=0;
  virtual int stopAcquisition()=0;
  virtual runStatus getRunStatus()=0;
  virtual int freeSharedMemory()=0;
  virtual int addSlsDetector(int, int pos=-1){return -1;};
  virtual int addSlsDetector(char*, int pos=-1){return -1;};
  virtual int removeSlsDetector(int pos=-1){return -1;};
  virtual int removeSlsDetector(char*){return -1;};
  virtual string setHostname(char*, int pos=-1)=0;
  virtual string getHostname(int pos=-1)=0;
  virtual int getDetectorId(int i=-1) =0;
  virtual int setDetectorId(int ival, int i=-1){return -1;};
  virtual synchronizationMode setSynchronization(synchronizationMode sync=GET_SYNCHRONIZATION_MODE){return GET_SYNCHRONIZATION_MODE;};
  virtual int exitServer()=0;
  virtual int setMaster(int i=-1){return -1;};
  virtual char* getSettingsDir()=0;
  virtual char* setSettingsDir(string s)=0;  
  virtual char* getCalDir()=0;
  virtual char* setCalDir(string s)=0;  
  virtual char* getFilePath()=0;
  virtual char* setFilePath(string s)=0;  
  virtual char* getFileName()=0;
  virtual char* setFileName(string s)=0;  
  virtual int getFileIndex()=0;
  virtual int setFileIndex(int i)=0;
  virtual int setFlatFieldCorrection(string fname="")=0; 
  virtual int getFlatFieldCorrection(float *corr=NULL, float *ecorr=NULL)=0;
  virtual int setFlatFieldCorrection(float *corr=NULL, float *ecorr=NULL)=0;
  virtual char *getFlatFieldCorrectionDir()=0;
  virtual void setFlatFieldCorrectionDir(string dir)=0;
  virtual char *getFlatFieldCorrectionFile()=0;
  virtual int setRateCorrection(float t=0)=0;
  virtual int getRateCorrection(float &t)=0;
  virtual float getRateCorrectionTau()=0;
  virtual int getRateCorrection()=0;
  virtual int setBadChannelCorrection(string fname="")=0;
  virtual int setBadChannelCorrection(int nch, int *chs, int ff=0)=0;
  virtual int getBadChannelCorrection(int *bad=NULL)=0;
  virtual string getBadChannelCorrectionFile()=0;
  virtual int setAngularConversion(string fname="")=0;
  virtual int getAngularConversion(int &direction,  angleConversionConstant *angconv=NULL)=0;
  virtual string getAngularConversion()=0;
  virtual float setGlobalOffset(float f)=0; 
  virtual float setFineOffset(float f)=0;
  virtual float getFineOffset()=0;
  virtual float getGlobalOffset()=0;
  virtual float setBinSize(float f)=0;
  virtual float getBinSize()=0;
  virtual int writeDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=-1)=0; 
  virtual int writeAngularConversion(string fname)=0;
  virtual int setThreadedProcessing(int i=-1)=0;;
  virtual int setPositions(int nPos, float *pos)=0;
  virtual int getPositions(float *pos=NULL)=0;
  virtual int setActionScript(int iaction, string fname="")=0;
  virtual int setActionParameter(int iaction, string par="")=0;
  virtual string getActionScript(int iaction)=0;
  virtual string getActionParameter(int iaction)=0;
  virtual int setScanScript(int index, string script="")=0;
  virtual int setScanParameter(int index, string par="")=0;
  virtual int setScanPrecision(int index, int precision=-1)=0;
  virtual int setScanSteps(int index, int nvalues=-1, float *values=NULL)=0;
  virtual string getScanScript(int index)=0;
  virtual string getScanParameter(int index)=0;
  virtual int getScanPrecision(int index)=0;
  virtual int getScanSteps(int index, float *values=NULL)=0;
  virtual char *getNetworkParameter(networkParameter i)=0;
  virtual char *setNetworkParameter(networkParameter i, string s)=0;
  virtual int setPort(portType t, int i=-1)=0;
  virtual int lockServer(int i=-1)=0;
  virtual string getLastClientIP()=0;
  virtual int configureMAC()=0;
  virtual int setNumberOfModules(int i=-1, dimension d=X)=0;
  virtual int getMaxNumberOfModules(dimension d=X)=0;
  virtual int setDynamicRange(int i=-1)=0;
  virtual detectorSettings getSettings(int imod=-1)=0;  
  virtual detectorSettings setSettings(detectorSettings isettings, int imod=-1)=0;
  virtual int getThresholdEnergy(int imod=-1)=0;  
  virtual int setThresholdEnergy(int e_eV, int imod=-1, detectorSettings isettings=GET_SETTINGS)=0; 
  virtual int64_t getId(idMode mode, int imod=0)=0;
  virtual int digitalTest(digitalTestMode mode, int imod=0)=0;
  virtual int executeTrimming(trimMode mode, int par1, int par2, int imod=-1)=0;
  virtual const char *getSettingsFile()=0;
  virtual int loadSettingsFile(string fname, int imod=-1)=0;
  virtual int saveSettingsFile(string fname, int imod=-1)=0;
  virtual int writeRegister(int addr, int val)=0;
  virtual int readRegister(int addr)=0;
  virtual float setDAC(float , dacIndex, int imod=-1)=0;
  virtual float getADC(dacIndex, int imod=0)=0;
  virtual int64_t setTimer(timerIndex index, int64_t t=-1)=0;
  virtual int64_t getTimeLeft(timerIndex index)=0;
  virtual int setSpeed(speedVariable sp, int value=-1)=0;
  virtual int setTrimEn(int nen, int *en=NULL)=0;
  virtual int getTrimEn(int *en=NULL)=0;
  virtual externalSignalFlag setExternalSignalFlags(externalSignalFlag pol=GET_EXTERNAL_SIGNAL_FLAG , int signalindex=0)=0;
  virtual int setReadOutFlags(readOutFlags flag=GET_READOUT_FLAGS)=0;
  virtual externalCommunicationMode setExternalCommunicationMode(externalCommunicationMode pol=GET_EXTERNAL_COMMUNICATION_MODE)=0;
  virtual int readConfigurationFile(string const fname)=0;  
  virtual int writeConfigurationFile(string const fname)=0;
  virtual int dumpDetectorSetup(string const fname, int level=0)=0; 
  virtual int retrieveDetectorSetup(string const fname, int level=0)=0;
  virtual int loadImageToDetector(int index,string const fname)=0;

  virtual int testFunction(int times=0)=0;






  /** returns string from run status index
      \param s can be ERROR, WAITING, RUNNING, TRANSMITTING, RUN_FINISHED
      \returns string error, waiting, running, data, finished
  */
  static string runStatusType(runStatus);

  /** returns detector type string from detector type index
      \param type string can be Mythen, Pilatus, Eiger, Gotthard, Agipd, Unknown
      \returns MYTHEN, PILATUS, EIGER, GOTTHARD, AGIPD, GENERIC
  */
  static string getDetectorType(detectorType t);

  /** returns detector type index from detector type string
      \param t can be MYTHEN, PILATUS, EIGER, GOTTHARD, AGIPD, GENERIC
      \returns Mythen, Pilatus, Eiger, Gotthard, Agipd, Unknown
  */
  static detectorType getDetectorType(string const type);


  /** returns synchronization type index from string
      \param t can be none, gating, trigger, complementary
      \returns ONE, MASTER_GATES, MASTER_TRIGGERS, SLAVE_STARTS_WHEN_MASTER_STOPS
  */
  static synchronizationMode getSyncType(string const type);



  /** returns synchronization type string from index
      \param t can be NONE, MASTER_GATES, MASTER_TRIGGERS, SLAVE_STARTS_WHEN_MASTER_STOPS
      \returns none, gating, trigger, complementary
  */
  static string getSyncType(synchronizationMode s );


  /** returns string from external signal type index
      \param f can be SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW, TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE, RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH, GATE_OUT_ACTIVE_LOW, =TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE, RO_TRIGGER_OUT_FALLING_EDGE
      \returns string  off, gate_in_active_high, gate_in_active_low, trigger_in_rising_edge, trigger_in_falling_edge, ro_trigger_in_rising_edge, ro_trigger_in_falling_edge, gate_out_active_high, gate_out_active_low, trigger_out_rising_edge, trigger_out_falling_edge, ro_trigger_out_rising_edge, ro_trigger_out_falling_edge, unknown
  */
  static string externalSignalType(externalSignalFlag f);



  /** returns external signal type index from string
      \param string  off, gate_in_active_high, gate_in_active_low, trigger_in_rising_edge, trigger_in_falling_edge, ro_trigger_in_rising_edge, ro_trigger_in_falling_edge, gate_out_active_high, gate_out_active_low, trigger_out_rising_edge, trigger_out_falling_edge, ro_trigger_out_rising_edge, ro_trigger_out_falling_edge, unknown
      \returns f can be SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW, TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE, RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH, GATE_OUT_ACTIVE_LOW, =TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE, RO_TRIGGER_OUT_FALLING_EDGE,GET_EXTERNAL_SIGNAL_FLAG (if unknown)
  */

  static externalSignalFlag externalSignalType(string sval);


  /** returns synchronization type string from index
      \param t can be NONE, MASTER_GATES, MASTER_TRIGGERS, SLAVE_STARTS_WHEN_MASTER_STOPS
      \returns none, gating, trigger, complementary
  */
  static detectorSettings getDetectorSettings(string s);

  /** returns detector settings string from index
      \param t can be STANDARD, FAST, HIGHGAIN, DYNAMICGAIN, LOWGAIN, MEDIUMGAIN, VERYHIGHGAIN, GET_SETTINGS
      \returns standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain, undefined
  */
  static string getDetectorSettings(detectorSettings s);

  




 private:
    int numberOfCommands;
    string cmd;

    typedef string (slsDetectorCommand::*MemFuncGetter)(int narg, char *args[], int action);
 

    struct FuncTable
    {
      string  m_pFuncName;
      //const char* m_pFuncName;
      MemFuncGetter m_pFuncPtr;
    };



    FuncTable descrToFuncMap[1000];






};



#endif

