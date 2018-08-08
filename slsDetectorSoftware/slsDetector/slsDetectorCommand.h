
#ifndef SLS_DETECTOR_COMMAND_H
#define SLS_DETECTOR_COMMAND_H


#include "sls_detector_defs.h"
#include "slsDetectorUtils.h"



/** @short This class handles the command line I/Os, help etc. of the text clients  */


class slsDetectorCommand : public virtual slsDetectorDefs { 

 public:


  slsDetectorCommand(slsDetectorUtils *det);
  virtual ~slsDetectorCommand(){};

  /*   /\** */
  /*      executes a set of string arguments according to a given format. It is used to read/write configuration file, dump and retrieve detector settings and for the command line interface command parsing */
  /*      \param narg number of arguments */
  /*      \param args array of string arguments */
  /*      \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition) */
  /*      \returns answer string  */
  /*   *\/ */
  virtual std::string executeLine(int narg, char *args[], int action); 
  
  /*   /\** */
  /*      returns the help for the executeLine command  */
  /*      \param os output stream to return the help to */
  /*      \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition)  */
  /*   *\/ */
  std::string helpLine(int narg, char *args[], int action=HELP_ACTION); 
  static std::string helpAcquire(int narg, char *args[], int action);
  static std::string helpData(int narg, char *args[], int action);
  static std::string helpFrame(int narg, char *args[], int action);
  static std::string helpStatus(int narg, char *args[], int action);
  static std::string helpDataStream(int narg, char *args[], int action);
  static std::string helpFree(int narg, char *args[], int action);
  static std::string helpHostname(int narg, char *args[], int action);
  static std::string helpUser(int narg, char *args[], int action);
  static std::string helpMaster(int narg, char *args[], int action);
  static std::string helpSync(int narg, char *args[], int action);
  static std::string helpExitServer(int narg, char *args[], int action);
  static std::string helpSettingsDir(int narg, char *args[], int action);
  static std::string helpCalDir(int narg, char *args[], int action);
  static std::string helpTrimEn(int narg, char *args[], int action);
  static std::string helpOutDir(int narg, char *args[], int action);
  static std::string helpFileName(int narg, char *args[], int action);
  static std::string helpFileIndex(int narg, char *args[], int action);
  static std::string helpFlatField(int narg, char *args[], int action);
  static std::string helpRateCorr(int narg, char *args[], int action);
  static std::string helpBadChannels(int narg, char *args[], int action);
  static std::string helpAngConv(int narg, char *args[], int action);
  static std::string helpThreaded(int narg, char *args[], int action);
  static std::string helpPositions(int narg, char *args[], int action);
  static std::string helpScripts(int narg, char *args[], int action);
  static std::string helpScans(int narg, char *args[], int action);
  static std::string helpNetworkParameter(int narg, char *args[], int action);
  static std::string helpPort(int narg, char *args[], int action);
  static std::string helpLock(int narg, char *args[], int action);
  static std::string helpLastClient(int narg, char *args[], int action);
  static std::string helpOnline(int narg, char *args[], int action);
  static std::string helpConfigureMac(int narg, char *args[], int action);
  static std::string helpDetectorSize(int narg, char *args[], int action);
  static std::string helpSettings(int narg, char *args[], int action);
  static std::string helpSN(int narg, char *args[], int action);
  static std::string helpDigiTest(int narg, char *args[], int action);
  static std::string helpRegister(int narg, char *args[], int action);
  static std::string helpDAC(int narg, char *args[], int action);
  static std::string helpTimer(int narg, char *args[], int action);
  static std::string helpTiming(int narg, char *args[], int action);
  static std::string helpTimeLeft(int narg, char *args[], int action);
  static std::string helpSpeed(int narg, char *args[], int action);
  static std::string helpAdvanced(int narg, char *args[], int action);
  static std::string helpConfiguration(int narg, char *args[], int action);
  static std::string helpImage(int narg, char *args[], int action);
  static std::string helpCounter(int narg, char *args[], int action);
  static std::string helpADC(int narg, char *args[], int action);
  static std::string helpTempControl(int narg, char *args[], int action);
  static std::string helpEnablefwrite(int narg, char *args[], int action);
  static std::string helpOverwrite(int narg, char *args[], int action);
  static std::string helpReceiver(int narg, char *args[], int action);
  static std::string helpPattern(int narg, char *args[], int action);
  static std::string helpPulse(int narg, char *args[], int action);












 private:


  slsDetectorUtils *myDet;
   
  std::string cmdUnderDevelopment(int narg, char *args[], int action); 
  std::string cmdUnknown(int narg, char *args[], int action); 
  std::string cmdAcquire(int narg, char *args[], int action); 
  std::string cmdData(int narg, char *args[], int action); 
  std::string cmdFrame(int narg, char *args[], int action); 
  std::string cmdStatus(int narg, char *args[], int action); 
  std::string cmdDataStream(int narg, char *args[], int action);
  std::string cmdFree(int narg, char *args[], int action);
  std::string cmdHostname(int narg, char *args[], int action);
  std::string cmdUser(int narg, char *args[], int action);
  std::string cmdMaster(int narg, char *args[], int action);
  std::string cmdSync(int narg, char *args[], int action);
  std::string cmdHelp(int narg, char *args[], int action);
  std::string cmdExitServer(int narg, char *args[], int action);
  std::string cmdSettingsDir(int narg, char *args[], int action);
  std::string cmdCalDir(int narg, char *args[], int action);
  std::string cmdTrimEn(int narg, char *args[], int action);
  std::string cmdOutDir(int narg, char *args[], int action);
  std::string cmdFileName(int narg, char *args[], int action);
  std::string cmdFileIndex(int narg, char *args[], int action);
  std::string cmdFlatField(int narg, char *args[], int action);
  std::string cmdRateCorr(int narg, char *args[], int action);
  std::string cmdBadChannels(int narg, char *args[], int action);
  std::string cmdAngConv(int narg, char *args[], int action);
  std::string cmdThreaded(int narg, char *args[], int action);
  std::string cmdPositions(int narg, char *args[], int action);
  std::string cmdScripts(int narg, char *args[], int action);
  std::string cmdScans(int narg, char *args[], int action);
  std::string cmdNetworkParameter(int narg, char *args[], int action);
  std::string cmdPort(int narg, char *args[], int action);
  std::string cmdLock(int narg, char *args[], int action);
  std::string cmdLastClient(int narg, char *args[], int action);
  std::string cmdOnline(int narg, char *args[], int action);
  std::string cmdConfigureMac(int narg, char *args[], int action);
  std::string cmdDetectorSize(int narg, char *args[], int action);
  std::string cmdSettings(int narg, char *args[], int action);
  std::string cmdSN(int narg, char *args[], int action);
  std::string cmdDigiTest(int narg, char *args[], int action);
  std::string cmdRegister(int narg, char *args[], int action);
  std::string cmdDAC(int narg, char *args[], int action);
  std::string cmdTiming(int narg, char *args[], int action);
  std::string cmdTimer(int narg, char *args[], int action);
  std::string cmdTimeLeft(int narg, char *args[], int action);
  std::string cmdSpeed(int narg, char *args[], int action);
  std::string cmdAdvanced(int narg, char *args[], int action);
  std::string cmdConfiguration(int narg, char *args[], int action);
  std::string cmdImage(int narg, char *args[], int action);
  std::string cmdCounter(int narg, char *args[], int action);
  std::string cmdADC(int narg, char *args[], int action);
  std::string cmdTempControl(int narg, char *args[], int action);
  std::string cmdEnablefwrite(int narg, char *args[], int action);
  std::string cmdOverwrite(int narg, char *args[], int action);
  std::string cmdReceiver(int narg, char *args[], int action);
  std::string cmdPattern(int narg, char *args[], int action);
  std::string cmdPulse(int narg, char *args[], int action);


  int numberOfCommands;
  std::string cmd;

  typedef std::string (slsDetectorCommand::*MemFuncGetter)(int narg, char *args[], int action);
 

  struct FuncTable
  {
    std::string  m_pFuncName;
    MemFuncGetter m_pFuncPtr;
  };



  FuncTable descrToFuncMap[1000];




};



#endif

