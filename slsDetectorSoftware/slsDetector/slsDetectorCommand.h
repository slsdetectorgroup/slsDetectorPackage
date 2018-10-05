
#ifndef SLS_DETECTOR_COMMAND_H
#define SLS_DETECTOR_COMMAND_H


#include "sls_detector_defs.h"

class multiSlsDetector;



/** @short This class handles the command line I/Os, help etc. of the text clients  */


class slsDetectorCommand : public virtual slsDetectorDefs { 

 public:


  slsDetectorCommand(multiSlsDetector *det);
  virtual ~slsDetectorCommand(){};

  /*
   * Executes a set of string arguments according to a given format.
   * It is used to read/write configuration file, dump and retrieve detector
   * settings and for the command line interface command parsing
   * @param narg number of arguments
   * @param args array of string arguments
   * @param action can be PUT_ACTION or GET_ACTION(from text client even READOUT_ACTION for acquisition)
   * @param detPos -1 for all detectors in multi detector list or position of a specific detector in list
   */
  virtual std::string executeLine(int narg, char *args[], int action, int detPos = -1);
  
  /*   /\** */
  /*      returns the help for the executeLine command  */
  /*      \param os output stream to return the help to */
  /*      \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition)  */
  /*   *\/ */
  std::string helpLine(int narg, int action=HELP_ACTION);
  static std::string helpAcquire(int action);
  static std::string helpData(int action);
  static std::string helpStatus(int action);
  static std::string helpDataStream(int action);
  static std::string helpFree(int action);
  static std::string helpHostname(int action);
  static std::string helpUser(int action);
  static std::string helpExitServer(int action);
  static std::string helpSettingsDir(int action);
  static std::string helpTrimEn(int action);
  static std::string helpOutDir(int action);
  static std::string helpFileName(int action);
  static std::string helpFileIndex(int action);
  static std::string helpRateCorr(int action);
  static std::string helpThreaded(int action);
  static std::string helpNetworkParameter(int action);
  static std::string helpPort(int action);
  static std::string helpLock(int action);
  static std::string helpLastClient(int action);
  static std::string helpOnline(int action);
  static std::string helpConfigureMac(int action);
  static std::string helpDetectorSize(int action);
  static std::string helpSettings(int action);
  static std::string helpSN(int action);
  static std::string helpDigiTest(int action);
  static std::string helpRegister(int action);
  static std::string helpDAC(int action);
  static std::string helpTimer(int action);
  static std::string helpTiming(int action);
  static std::string helpTimeLeft(int action);
  static std::string helpSpeed(int action);
  static std::string helpAdvanced(int action);
  static std::string helpConfiguration(int action);
  static std::string helpImage(int action);
  static std::string helpCounter(int action);
  static std::string helpADC(int action);
  static std::string helpTempControl(int action);
  static std::string helpEnablefwrite(int action);
  static std::string helpOverwrite(int action);
  static std::string helpReceiver(int action);
  static std::string helpPattern(int action);
  static std::string helpPulse(int action);












 private:


  multiSlsDetector *myDet;
   
  std::string cmdUnderDevelopment(int narg, char *args[], int action, int detPos = -1);
  std::string cmdUnknown(int narg, char *args[], int action, int detPos = -1);
  std::string cmdAcquire(int narg, char *args[], int action, int detPos = -1);
  std::string cmdData(int narg, char *args[], int action, int detPos = -1);
  std::string cmdStatus(int narg, char *args[], int action, int detPos = -1);
  std::string cmdDataStream(int narg, char *args[], int action, int detPos = -1);
  std::string cmdFree(int narg, char *args[], int action, int detPos = -1);
  std::string cmdHostname(int narg, char *args[], int action, int detPos = -1);
  std::string cmdUser(int narg, char *args[], int action, int detPos = -1);
  std::string cmdHelp(int narg, char *args[], int action, int detPos = -1);
  std::string cmdExitServer(int narg, char *args[], int action, int detPos = -1);
  std::string cmdSettingsDir(int narg, char *args[], int action, int detPos = -1);
  std::string cmdTrimEn(int narg, char *args[], int action, int detPos = -1);
  std::string cmdOutDir(int narg, char *args[], int action, int detPos = -1);
  std::string cmdFileName(int narg, char *args[], int action, int detPos = -1);
  std::string cmdFileIndex(int narg, char *args[], int action, int detPos = -1);
  std::string cmdRateCorr(int narg, char *args[], int action, int detPos = -1);
  std::string cmdThreaded(int narg, char *args[], int action, int detPos = -1);
  std::string cmdNetworkParameter(int narg, char *args[], int action, int detPos = -1);
  std::string cmdPort(int narg, char *args[], int action, int detPos = -1);
  std::string cmdLock(int narg, char *args[], int action, int detPos = -1);
  std::string cmdLastClient(int narg, char *args[], int action, int detPos = -1);
  std::string cmdOnline(int narg, char *args[], int action, int detPos = -1);
  std::string cmdConfigureMac(int narg, char *args[], int action, int detPos = -1);
  std::string cmdDetectorSize(int narg, char *args[], int action, int detPos = -1);
  std::string cmdSettings(int narg, char *args[], int action, int detPos = -1);
  std::string cmdSN(int narg, char *args[], int action, int detPos = -1);
  std::string cmdDigiTest(int narg, char *args[], int action, int detPos = -1);
  std::string cmdRegister(int narg, char *args[], int action, int detPos = -1);
  std::string cmdDAC(int narg, char *args[], int action, int detPos = -1);
  std::string cmdTiming(int narg, char *args[], int action, int detPos = -1);
  std::string cmdTimer(int narg, char *args[], int action, int detPos = -1);
  std::string cmdTimeLeft(int narg, char *args[], int action, int detPos = -1);
  std::string cmdSpeed(int narg, char *args[], int action, int detPos = -1);
  std::string cmdAdvanced(int narg, char *args[], int action, int detPos = -1);
  std::string cmdConfiguration(int narg, char *args[], int action, int detPos = -1);
  std::string cmdImage(int narg, char *args[], int action, int detPos = -1);
  std::string cmdCounter(int narg, char *args[], int action, int detPos = -1);
  std::string cmdADC(int narg, char *args[], int action, int detPos = -1);
  std::string cmdTempControl(int narg, char *args[], int action, int detPos = -1);
  std::string cmdEnablefwrite(int narg, char *args[], int action, int detPos = -1);
  std::string cmdOverwrite(int narg, char *args[], int action, int detPos = -1);
  std::string cmdReceiver(int narg, char *args[], int action, int detPos = -1);
  std::string cmdPattern(int narg, char *args[], int action, int detPos = -1);
  std::string cmdPulse(int narg, char *args[], int action, int detPos = -1);


  int numberOfCommands;
  std::string cmd;

  typedef std::string (slsDetectorCommand::*MemFuncGetter)(int narg, char *args[], int action, int detPos = -1);
 

  struct FuncTable
  {
    std::string  m_pFuncName;
    MemFuncGetter m_pFuncPtr;
  };



  FuncTable descrToFuncMap[1000];




};



#endif

