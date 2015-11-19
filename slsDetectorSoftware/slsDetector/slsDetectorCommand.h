
#ifndef SLS_DETECTOR_COMMAND_H
#define SLS_DETECTOR_COMMAND_H


#include "sls_detector_defs.h"
#include "slsDetectorUtils.h"
using namespace std;


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
  virtual string executeLine(int narg, char *args[], int action); 
  
  /*   /\** */
  /*      returns the help for the executeLine command  */
  /*      \param os output stream to return the help to */
  /*      \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition)  */
  /*   *\/ */
  string helpLine(int narg, char *args[], int action=HELP_ACTION); 
  static string helpAcquire(int narg, char *args[], int action);
  static string helpData(int narg, char *args[], int action);
  static string helpFrame(int narg, char *args[], int action);
  static string helpStatus(int narg, char *args[], int action);
  static string helpFree(int narg, char *args[], int action);
  static string helpAdd(int narg, char *args[], int action);
  static string helpRemove(int narg, char *args[], int action);
  static string helpHostname(int narg, char *args[], int action);
  static string helpId(int narg, char *args[], int action);
  static string helpMaster(int narg, char *args[], int action);
  static string helpSync(int narg, char *args[], int action);
  static string helpExitServer(int narg, char *args[], int action);
  static string helpSettingsDir(int narg, char *args[], int action);
  static string helpCalDir(int narg, char *args[], int action);
  static string helpTrimEn(int narg, char *args[], int action);
  static string helpOutDir(int narg, char *args[], int action);
  static string helpFileName(int narg, char *args[], int action);
  static string helpFileIndex(int narg, char *args[], int action);
  static string helpFlatField(int narg, char *args[], int action);
  static string helpRateCorr(int narg, char *args[], int action);
  static string helpBadChannels(int narg, char *args[], int action);
  static string helpAngConv(int narg, char *args[], int action);
  static string helpThreaded(int narg, char *args[], int action);
  static string helpPositions(int narg, char *args[], int action);
  static string helpScripts(int narg, char *args[], int action);
  static string helpScans(int narg, char *args[], int action);
  static string helpNetworkParameter(int narg, char *args[], int action);
  static string helpPort(int narg, char *args[], int action);
  static string helpLock(int narg, char *args[], int action);
  static string helpLastClient(int narg, char *args[], int action);
  static string helpOnline(int narg, char *args[], int action);
  static string helpConfigureMac(int narg, char *args[], int action);
  static string helpDetectorSize(int narg, char *args[], int action);
  static string helpSettings(int narg, char *args[], int action);
  static string helpSN(int narg, char *args[], int action);
  static string helpDigiTest(int narg, char *args[], int action);
  static string helpRegister(int narg, char *args[], int action);
  static string helpDAC(int narg, char *args[], int action);
  static string helpTimer(int narg, char *args[], int action);
  static string helpTiming(int narg, char *args[], int action);
  static string helpTimeLeft(int narg, char *args[], int action);
  static string helpSpeed(int narg, char *args[], int action);
  static string helpAdvanced(int narg, char *args[], int action);
  static string helpConfiguration(int narg, char *args[], int action);
  static string helpImage(int narg, char *args[], int action);
  static string helpCounter(int narg, char *args[], int action);
  static string helpADC(int narg, char *args[], int action);
  static string helpEnablefwrite(int narg, char *args[], int action);
  static string helpOverwrite(int narg, char *args[], int action);
  static string helpReceiver(int narg, char *args[], int action);
  static string helpPattern(int narg, char *args[], int action);
  static string helpPulse(int narg, char *args[], int action);












 private:


  slsDetectorUtils *myDet;
   
  string cmdUnderDevelopment(int narg, char *args[], int action); 
  string cmdUnknown(int narg, char *args[], int action); 
  string cmdAcquire(int narg, char *args[], int action); 
  string cmdData(int narg, char *args[], int action); 
  string cmdFrame(int narg, char *args[], int action); 
  string cmdStatus(int narg, char *args[], int action); 
  string cmdFree(int narg, char *args[], int action);
  string cmdAdd(int narg, char *args[], int action);
  string cmdRemove(int narg, char *args[], int action);
  string cmdHostname(int narg, char *args[], int action);
  string cmdId(int narg, char *args[], int action);
  string cmdMaster(int narg, char *args[], int action);
  string cmdSync(int narg, char *args[], int action);
  string cmdHelp(int narg, char *args[], int action);
  string cmdExitServer(int narg, char *args[], int action);
  string cmdSettingsDir(int narg, char *args[], int action);
  string cmdCalDir(int narg, char *args[], int action);
  string cmdTrimEn(int narg, char *args[], int action);
  string cmdOutDir(int narg, char *args[], int action);
  string cmdFileName(int narg, char *args[], int action);
  string cmdFileIndex(int narg, char *args[], int action);
  string cmdFlatField(int narg, char *args[], int action);
  string cmdRateCorr(int narg, char *args[], int action);
  string cmdBadChannels(int narg, char *args[], int action);
  string cmdAngConv(int narg, char *args[], int action);
  string cmdThreaded(int narg, char *args[], int action);
  string cmdPositions(int narg, char *args[], int action);
  string cmdScripts(int narg, char *args[], int action);
  string cmdScans(int narg, char *args[], int action);
  string cmdNetworkParameter(int narg, char *args[], int action);
  string cmdPort(int narg, char *args[], int action);
  string cmdLock(int narg, char *args[], int action);
  string cmdLastClient(int narg, char *args[], int action);
  string cmdOnline(int narg, char *args[], int action);
  string cmdConfigureMac(int narg, char *args[], int action);
  string cmdDetectorSize(int narg, char *args[], int action);
  string cmdSettings(int narg, char *args[], int action);
  string cmdSN(int narg, char *args[], int action);
  string cmdDigiTest(int narg, char *args[], int action);
  string cmdRegister(int narg, char *args[], int action);
  string cmdDAC(int narg, char *args[], int action);
  string cmdTiming(int narg, char *args[], int action);
  string cmdTimer(int narg, char *args[], int action);
  string cmdTimeLeft(int narg, char *args[], int action);
  string cmdSpeed(int narg, char *args[], int action);
  string cmdAdvanced(int narg, char *args[], int action);
  string cmdConfiguration(int narg, char *args[], int action);
  string cmdImage(int narg, char *args[], int action);
  string cmdCounter(int narg, char *args[], int action);
  string cmdADC(int narg, char *args[], int action);
  string cmdEnablefwrite(int narg, char *args[], int action);
  string cmdOverwrite(int narg, char *args[], int action);
  string cmdReceiver(int narg, char *args[], int action);
  string cmdPattern(int narg, char *args[], int action);
  string cmdPulse(int narg, char *args[], int action);


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

