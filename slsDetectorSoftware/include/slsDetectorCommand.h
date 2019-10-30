
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
  static std::string helpAcquire(int action);
  static std::string helpConfiguration(int action);


 private:
  multiSlsDetector *myDet;
   
  std::string cmdUnknown(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdAcquire(int narg, const char * const args[], int action, int detPos = -1);
  std::string cmdConfiguration(int narg, const char * const args[], int action, int detPos = -1);


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

