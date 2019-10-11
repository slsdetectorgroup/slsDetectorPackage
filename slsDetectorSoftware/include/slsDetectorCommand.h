
#ifndef SLS_DETECTOR_COMMAND_H
#define SLS_DETECTOR_COMMAND_H


#include "sls_detector_defs.h"
#include <vector>
class multiSlsDetector;



/** @short This class handles the command line I/Os, help etc. of the text clients  */


class slsDetectorCommand : public virtual slsDetectorDefs { 

 public:


  slsDetectorCommand(multiSlsDetector *det);


  /*
   * Executes a set of string arguments according to a given format.
   * It is used to read/write configuration file, dump and retrieve detector
   * settings and for the command line interface command parsing
   * @param narg number of arguments
   * @param args array of string arguments
   * @param action can be PUT_ACTION or GET_ACTION(from text client even READOUT_ACTION for acquisition)
   * @param detPos -1 for all detectors in multi detector list or position of a specific detector in list
   */
  std::string executeLine(int narg, const char * const args[], int action, int detPos = -1);
  

  std::vector<std::string> getAllCommands();

  /*   /\** */
  /*      returns the help for the executeLine command  */
  /*      \param os output stream to return the help to */
  /*      \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition)  */
  /*   *\/ */
  std::string helpLine(int narg, const char * const args[], int action=HELP_ACTION, int detPos = -1);
  static std::string helpAcquire(int action);
  static std::string helpData(int action);
  static std::string helpFree(int action);
  static std::string helpHostname(int action);
  static std::string helpUser(int action);
  static std::string helpExitServer(int action);
  static std::string helpTrimEn(int action);
  static std::string helpRateCorr(int action);
  static std::string helpThreaded(int action);
  static std::string helpPort(int action);
  static std::string helpOnline(int action);
  static std::string helpDetectorSize(int action);
  static std::string helpSettings(int action);
  static std::string helpSN(int action);
  static std::string helpDigiTest(int action);
  static std::string helpRegister(int action);
  static std::string helpDAC(int action);
  static std::string helpTimer(int action);
  static std::string helpTimeLeft(int action);
  static std::string helpSpeed(int action);
  static std::string helpAdvanced(int action);
  static std::string helpConfiguration(int action);
  static std::string helpCounter(int action);
  static std::string helpADC(int action);
  static std::string helpTempControl(int action);
  static std::string helpReceiver(int action);
  static std::string helpPattern(int action);
  static std::string helpPulse(int action);
  static std::string helpProcessor(int action);

 private:
  multiSlsDetector *myDet;
   
  std::string cmdUnderDevelopment(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdUnknown(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdAcquire(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdData(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdFree(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdHostname(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdUser(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdHelp(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdExitServer(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdTrimEn(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdRateCorr(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdPort(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdOnline(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdDetectorSize(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdSettings(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdSN(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdDigiTest(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdRegister(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdDAC(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdTimer(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdTimeLeft(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdSpeed(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdAdvanced(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdConfiguration(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdCounter(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdADC(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdTempControl(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdReceiver(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdPattern(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdPulse(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdProcessor(int narg, const char * const args[], int action, int detPos = -1);

  int numberOfCommands;
  std::string cmd;

  typedef std::string (slsDetectorCommand::*MemFuncGetter)(int narg, const char * const args[], int action, int detPos);
 

  struct FuncTable
  {
    std::string  m_pFuncName;
    MemFuncGetter m_pFuncPtr;
  };



  FuncTable descrToFuncMap[1000];




};



#endif

