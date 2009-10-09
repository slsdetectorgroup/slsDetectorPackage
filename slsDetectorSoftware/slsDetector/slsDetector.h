


#ifndef SLS_DETECTOR_H
#define SLS_DETECTOR_H

#include "MySocketTCP.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <sstream>
 #include <queue>
extern "C" {
 #include <pthread.h>
}
 #include <fcntl.h>
 #include <unistd.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <sys/uio.h>


#define MAX_TIMERS 10
#define MAX_ROIS 100
#define MAX_BADCHANS 2000
#define MAXPOS 50

#define NMODMAXX 24
#define NMODMAXY 24
#define MAXMODS 36
#define NCHIPSMAX 10
#define NCHANSMAX 65536
#define NDACSMAX 16

#define DEFAULT_HOSTNAME  "localhost"
#define DEFAULT_SHM_KEY  5678

#define defaultTDead {170,90,750}

using namespace std;
/**
 \mainpage Common C++ library for SLS detectors data acquisition
 *
 * \section intro_sec Introduction
 


 * \subsection mot_sec Motivation
 Although the SLS detectors group delvelops several types of detectors (1/2D, counting/integrating etc.) it is common interest of the group to use a common platfor for data acquisition
  \subsection arch_sec System Architecture
  The architecture of the acquisitions system is intended as follows:
  \li A socket server running on the detector (or more than one in some special cases)
  \li C++ classes common to all detectors for client-server communication. These can be supplied to users as libraries and embedded also in acquisition systems which are not developed by the SLS \sa MySocketTCP slsDetector
  \li the possibility of using a Qt-based graphical user interface (with eventually root analisys capabilities)
  \li the possibility of runnin alla commands from command line. In order to ensure a fast operation of this so called "text client" the detector parameters should not be re-initialized everytime. For this reason a shared memory block is allocated where the main detector flags and parameters are stored \sa slsDetector::sharedSlsDetector


 @author Anna Bergamaschi

*/



/**
 * 
 *
@libdoc The slsDetector class is expected to become the interface class for all SLS Detectors acquisition (and analysis) software.
 *
 * @short This is the base class for all SLS detector functionalities
 * @author Anna Bergamaschi
 * @version 0.1alpha (any string)


 */

class slsDetector {



 public:
  

  /** online flags enum \sa setOnline*/
  enum {GET_ONLINE_FLAG, /**< returns wether the detector is in online or offline state */
	OFFLINE_FLAG, /**< detector in offline state (i.e. no communication to the detector - using only local structure - no data acquisition possible!) */
	ONLINE_FLAG /**< detector in online state (i.e. communication to the detector updating the local structure) */
  };


#include "sls_detector_defs.h"


  /** 
      @short Structure allocated in shared memory to store detector settings and be accessed in parallel by several applications (take care of possible conflicts!)
      
  */
typedef  struct sharedSlsDetector {
  /** already existing flag. If the detector does not yet exist (alreadyExisting=0) the sharedMemory will be created, otherwise it will simly be linked */
    int alreadyExisting;
    
    /** is the hostname (or IP address) of the detector. needs to be set before startin the communication */
    char hostname[MAX_STR_LENGTH];
    /** is the port used for control functions normally it should not be changed*/
    int controlPort;
  /** is the port used to stop the acquisition normally it should not be changed*/
    int stopPort;
  /** is the port used to acquire the data normally it should not be changed*/
    int dataPort;

  /** detector type  \ see :: detectorType*/
    detectorType myDetectorType;


  /** path of the trimbits files */
    char trimDir[MAX_STR_LENGTH];
  /** path of the calibration files */
    char calDir[MAX_STR_LENGTH];
  /** number of energies at which the detector has been trimmed (unused) */
    int nTrimEn;
  /** list of the energies at which the detector has been trimmed (unused) */
    int trimEnergies[100];

	   
  /** current index of the output file */   
    int fileIndex;
  /** path of the output files */  
    char filePath[MAX_STR_LENGTH];
  /** name root of the output files */  
    char fileName[MAX_STR_LENGTH];

  /* size of the detector */
    
    /** number of installed modules of the detector (x and y directions) */  
    int nMod[2];
    /**  number of modules ( nMod[X]*nMod[Y]) \see nMod */
    int nMods;
    /** maximum number of modules of the detector (x and y directions) */ 
    int nModMax[2];
    /**  maximum number of modules (nModMax[X]*nModMax[Y]) \see nModMax */
    int nModsMax;
    /**  number of channels per chip */
    int nChans;
    /**  number of chips per module*/
    int nChips;
    /**  number of dacs per module*/
    int nDacs;
    /** number of adcs per module */
    int nAdcs;
    /** dynamic range of the detector data */
    int dynamicRange;
    /**  size of the data that are transfered from the detector */
    int dataBytes;
    
    /** corrections  to be applied to the data \see ::correctionFlags */
    int correctionMask;
    /** dead time (in ns) for rate corrections */
    float tDead;
    /** number of bad channels from bad channel list */
    int nBadChans;
    /** list of bad channels */
    int badChansList[MAX_BADCHANS];
    /** number of bad channels from flat field i.e. channels which read 0 in the flat field file */
    int nBadFF;
    /** list of bad channels from flat field i.e. channels which read 0 in the flat field file */
    int badFFList[MAX_BADCHANS];
    
    /** array of angular conversion constants for each module \see ::angleConversionConstant */
    angleConversionConstant angOff[MAXMODS];
    /** angular direction (1 if it corresponds to the encoder direction i.e. channel 0 is 0, maxchan is positive high angle, 0 otherwise  */
    int angDirection;
     /** beamline fine offset (of the order of mdeg, might be adjusted for each measurements)  */
    float fineOffset;
     /** beamline offset (might be a few degrees beacuse of encoder offset - normally it is kept fixed for a long period of time)  */
    float globalOffset;
     /** number of positions at which the detector should acquire  */
    int numberOfPositions;
     /** list of encoder positions at which the detector should acquire */
    float detPositions[MAXPOS];
  /** bin size for data merging */
  float binSize;

  /* infos necessary for the readout to determine the size of the data */
    /** number of rois defined */
    int nROI;
    /** list of rois */
    ROI roiLimits[MAX_ROIS];
    /** readout flags */
    readOutFlags roFlags;

 /* detector setup - not needed */
    /** detector settings (standard, fast, etc.) */
    detectorSettings currentSettings;
    /** detector threshold (eV) */
    int currentThresholdEV;
    /** timer values */
    int64_t timerValue[MAX_TIMERS];
    /** clock divider */
    int clkDiv;
    
  /*offsets*/
    /** memory offsets for the flat filed coefficients */
    int ffoff;
    /** memory offsets for the flat filed coefficient errors */
    int fferroff;
    /** memory offsets for the module structures */
    int modoff;
    /** memory offsets for the dac arrays */
    int dacoff;
    /** memory offsets for the adc arrays */
    int adcoff;
    /** memory offsets for the chip register arrays */
    int chipoff;
    /** memory offsets for the channel register arrays */
    int chanoff;

} sharedSlsDetector;



/** (default) constructor 
 \param  type is needed to define the size of the detector shared memory 9defaults to GENERIC i.e. the largest shared memory needed by any slsDetector is allocated
 \param  id is the detector index which is needed to define the shared memory id. Different physical detectors should have different IDs in order to work independently


*/
  slsDetector(detectorType type=GENERIC, int id=0);
  //slsDetector(string  const fname);
  //  ~slsDetector(){while(dataQueue.size()>0){}};
  /** destructor */ 
  ~slsDetector(){};


  /** sets the onlineFlag
      \param off can be: <BR> GET_ONLINE_FLAG, returns wether the detector is in online or offline state;<BR> OFFLINE_FLAG, detector in offline state (i.e. no communication to the detector - using only local structure - no data acquisition possible!);<BR> ONLINE_FLAG  detector in online state (i.e. communication to the detector updating the local structure) */
  int setOnline(int const online);  
  /** sets the onlineFlag
      \returns 1 if the detector structure has already be initlialized, 0 otherwise */
  int exists() {return thisDetector->alreadyExisting;};

  /**
     Every detector should have a basic configuration file containing:
     type (mythen, pilatus etc.)
     hostname
     portnumber
     communication type (default TCP/IP)
     eventually secondary portnumber (e.g. mythen stop function)
     number of modules installed if different from the detector size (x,y)
  
     to be changed
  */

  int readConfigurationFile(string const fname);  
  /**
     Every detector should have a basic configuration file containing:
     type (mythen, pilatus etc.)
     hostname
     portnumber
     communication type (default TCP/IP)
     eventually secondary portnumber (e.g. mythen stop function)
     number of modules installed if different from the detector size (x,y)

     to be changed
  */
  int writeConfigurationFile(string const fname);


  /* 
     It should be possible to dump all the settings of the detector (including trimbits, threshold energy, gating/triggering, acquisition time etc.
     in a file and retrieve it for repeating the measurement with identicals ettings, if necessary
  */
  /** 
      not yet implemented

      should dump to a file all the current detector parameters
  */
  int dumpDetectorSetup(string const fname);  
  /** 
      not yet implemented

      should retrieve from a file all the current detector parameters
  */
  int retrieveDetectorSetup(string const fname);

  /** 
     configure the socket communication and initializes the socket instances

     \param name hostname - if "" the current hostname is used
     \param control_port port for control commands - if -1 the current is used
     \param stop_port port for stop command - if -1 the current is used
     \param data_port port for receiving data - if -1 the current is used

     \returns OK is connection succeded, FAIL otherwise
     \sa sharedSlsDetector
  */
  int setTCPSocket(string const name="", int const control_port=-1, int const stop_port=-1, int const data_port=-1);
  /** returns the detector hostname \sa sharedSlsDetector  */
  char* getHostname() {return thisDetector->hostname;};
  /** returns the detector control port  \sa sharedSlsDetector */
  int getControlPort() {return  thisDetector->controlPort;};
  /** returns the detector stop  port  \sa sharedSlsDetector */
  int getStopPort() {return thisDetector->stopPort;};
  /** returns the detector data port  \sa sharedSlsDetector */
  int getDataPort() {return thisDetector->dataPort;};
 

  /* I/O */
  /** returns the detector trimbit directory  \sa sharedSlsDetector */
  char* getTrimDir() {return thisDetector->trimDir;};
  /** sets the detector trimbit directory  \sa sharedSlsDetector */
  char* setTrimDir(string s) {sprintf(thisDetector->trimDir, s.c_str()); return thisDetector->trimDir;};
  /** returns the number of trim energies and their value  \sa sharedSlsDetector 
   \param point to the array that will contain the trim energies (in ev)
  \returns number of trim energies

  \sa  sharedSlsDetector
  */
  int getTrimEn(int *en) {for (int ien=0; ien<thisDetector->nTrimEn; ien++) en[ien]=thisDetector->trimEnergies[ien]; return (thisDetector->nTrimEn);};

  /**
     reads a trim file
     \param fname name of the file to be read
     \param myMod pointer to the module structure which has to be set. <BR> If it is NULL a new module structure will be created
     \returns the pointer to myMod or NULL if reading the file failed
  */

  sls_detector_module* readTrimFile(string fname,  sls_detector_module* myMod=NULL);

  /**
     writes a trim file
     \param fname name of the file to be written
     \param mod module structure which has to be written to file
     \returns OK or FAIL if the file could not be written

     \sa ::sls_detector_module
  */
  int writeTrimFile(string fname, sls_detector_module mod); 
  
  /**
     writes a trim file for module number imod - the values will be read from the current detector structure
     \param fname name of the file to be written
     \param imod module number
     \returns OK or FAIL if the file could not be written   
     \sa ::sls_detector_module sharedSlsDetector
  */
  int writeTrimFile(string fname, int imod);

  /**
     sets the default output files path
  \sa  sharedSlsDetector
  */
  char* setFilePath(string s) {sprintf(thisDetector->filePath, s.c_str()); return thisDetector->filePath;};

  /**
     sets the default output files root name
  \sa  sharedSlsDetector
  */
  char* setFileName(string s) {sprintf(thisDetector->fileName, s.c_str()); return thisDetector->fileName;}; 

  /**
     sets the default output file index
  \sa  sharedSlsDetector
  */
  int setFileIndex(int i) {thisDetector->fileIndex=i; return thisDetector->fileIndex;}; 
  
  /**
     returns the default output files path
  \sa  sharedSlsDetector
  */
  char* getFilePath() {return thisDetector->filePath;};
  
  /**
     returns the default output files root name
  \sa  sharedSlsDetector
  */
  char* getFileName() {return thisDetector->fileName;};

  /**
     returns the default output file index
  \sa  sharedSlsDetector
  */
  int getFileIndex() {return thisDetector->fileIndex;};
  
  /** generates file name without extension

      always appends to file path and file name the run index.

      in case also appends the position index 
       
      Filenames will be of the form: filepath/filename(_px)_i
      where x is the position index and i is the run index

  */

  string createFileName();
  
    /**
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \param err array of arrors on the data. If NULL no errors will be written
       
       \param ang array of angular values. If NULL data will be in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if -1 defaults to the number of installed channels of the detector
       \returns OK or FAIL if it could not write the file or data=NULL
       
 
  */
  int writeDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=-1); 
  
  /**
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \returns OK or FAIL if it could not write the file or data=NULL
  */
  int writeDataFile(string fname, int *data);
  
  /**
       reads a data file
       \param name of the file to be read
       \param data array of data values to be filled
       \param err array of arrors on the data. If NULL no errors are expected on the file
       
       \param ang array of angular values. If NULL data are expected in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if <=0 defaults to the number of installed channels of the detector
       \returns OK or FAIL if it could not read the file or data=NULL
       
 
  */
  int readDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=0);  

  /**
       reads a data file
       \param name of the file to be read
       \param data array of data values
       \returns OK or FAIL if it could not read the file or data=NULL
  */
  int readDataFile(string fname, int *data);

 /**
     returns the location of the calibration files
  \sa  sharedSlsDetector
  */
  char* getCalDir() {return thisDetector->calDir;};
   /**
      sets the location of the calibration files
  \sa  sharedSlsDetector
  */
  char* setCalDir(string s) {sprintf(thisDetector->calDir, s.c_str()); return thisDetector->calDir;}; 
  /**
      reads a calibration file
      \param fname file to be read
      \param gain reference to the gain variable
      \offset reference to the offset variable
  \sa  sharedSlsDetector
  */
  int readCalibrationFile(string fname, float &gain, float &offset);
  /**
      writes a clibration file
      \param fname file to be written
      \param gain 
      \param offset
  \sa  sharedSlsDetector
  */
  int writeCalibrationFile(string fname, float gain, float offset);


  /**
      reads an angular conversion file
      \param fname file to be read
  \sa  angleConversionConstant
  */
  int readAngularConversion(string fname="");
  /**
     writes an angular conversion file
      \param fname file to be written
  \sa  angleConversionConstant
  */
  int writeAngularConversion(string fname="");
 


  /* Communication to server */

  // General purpose functions

  /**
     executes a system command on the server 
     e.g. mount an nfs disk, reboot and returns answer etc.
     \param cmd is the command to be executed
     \param answer is the answer from the detector
     \returns OK or FAIL depending on the command outcome
  */
  int execCommand(string cmd, string answer);
  
  /**
     sets/gets detector type
     normally  the detector knows what type of detector it is
     \param type is the detector type (defaults to GET_DETECTOR_TYPE)
     \returns returns detector type index (1 GENERIC, 2 MYTHEN, 3 PILATUS, 4 XFS, 5 GOTTHARD, 6 AGIPD, -1 command failed)
  */
  int setDetectorType(detectorType type=GET_DETECTOR_TYPE);  

  /** 
     sets/gets detector type
     normally  the detector knows what type of detector it is
     \param type is the detector type ("Mythen", "Pilatus", "XFS", "Gotthard", Agipd")
     \returns returns detector type index (1 GENERIC, 2 MYTHEN, 3 PILATUS, 4 XFS, 5 GOTTHARD, 6 AGIPD, -1 command failed)
  */
  int setDetectorType(string type);  

  /** 
     gets detector type
     normally  the detector knows what type of detector it is
     \param type is the string where the detector type will be written ("Mythen", "Pilatus", "XFS", "Gotthard", Agipd")
  */
  void getDetectorType(char *type);


  // Detector configuration functions
  /** 
      set/get the size of the detector 
      \param n number of modules
      \param d dimension
      \returns current number of modules in direction d
  */
  int setNumberOfModules(int n, dimension d=X); // if n=GET_FLAG returns the number of installed modules

  /*
    returns the instrinsic size of the detector (maxmodx, maxmody, nchans, nchips, ndacs
    enum numberOf {
    MAXMODX,
    MAXMODY,
    CHANNELS,
    CHIPS,
    DACS
    }
  */


  /** 
      get the maximum size of the detector 
      \param d dimension
      \returns maximum number of modules that can be installed in direction d
  */
  int getMaxNumberOfModules(dimension d=X); //
 
 
  /** 
     set/get the use of an external signal 
      \param pol meaning of the signal \sa externalSignalFlag
      \param signalIndex index of the signal
      \returns current meaning of signal signalIndex
  */
  externalSignalFlag setExternalSignalFlags(externalSignalFlag pol=GET_EXTERNAL_SIGNAL_FLAG , int signalindex=0);

 
  /** 
     set/get the external communication mode
     
     obsolete \sa setExternalSignalFlags
      \param pol value to be set \sa externalCommunicationMode
      \returns current external communication mode
  */
  externalCommunicationMode setExternalCommunicationMode(externalCommunicationMode pol=GET_EXTERNAL_COMMUNICATION_MODE);


  // Tests and identification
 
  /**
     get detector ids/versions for module
     \param mode which id/version has to be read
     \param imod module number for module serial number
     \returns id
  */
  int64_t getId(idMode mode, int imod=0);
  /**
    Digital test of the modules
    \param mode test mode
    \param imod module number for chip test or module firmware test
    \returns OK or error mask
  */
  int digitalTest(digitalTestMode mode, int imod=0);
  /**
     analog test
     \param modte test mode
     \return pointer to acquired data
 
     not yet implemented
  */

  int* analogTest(analogTestMode mode);
  
  /** 
     enable analog output of channel ichan
 
     not yet implemented
  */
  int enableAnalogOutput(int ichan);
  
  /** 
     enable analog output of channel ichan, chip ichip, module imod
 
     not yet implemented
  */
  int enableAnalogOutput(int imod, int ichip, int ichan);

  /**
     give a train of calibration pulses 
     \param vcal pulse amplitude
     \param npulses number of pulses
 
     not yet implemented
     
  */ 
  int giveCalibrationPulse(float vcal, int npulses);

  // Expert Initialization functions
 

  /** 
      write  register 
      \param addr address
      \val value
      \returns current register value

  */
  int writeRegister(int addr, int val);
  
  /** 
      read  register 
      \param addr address
      \returns current register value

  */
  int readRegister(int addr);

  /**
    set dacs value
    \param val value (in V)
    \param index DAC index
    \param imod module number (if -1 alla modules)
    \returns current DAC value
  */
  float setDAC(float val, dacIndex index, int imod=-1);
  
  /**
    set dacs value
    \param index ADC index
    \param imod module number
    \returns current ADC value
  */
  float getADC(dacIndex index, int imod=0); 
 
  /**
    configure channel
    \param reg channel register
    \param ichan channel number (-1 all)
    \param ichip chip number (-1 all)
    \param imod module number (-1 all)
    \returns current register value
    \sa ::sls_detector_channel
  */
  int setChannel(int64_t reg, int ichan=-1, int ichip=-1, int imod=-1);  

  /**
    configure channel
    \param chan channel to be set - must contain correct channel, module and chip number
    \returns current register value
  */
  int setChannel(sls_detector_channel chan);

  /**
    get channel
    \param ichan channel number 
    \param ichip chip number
    \param imod module number
    \returns current channel structure for channel
  */
  sls_detector_channel getChannel(int ichan, int ichip, int imod);
  


  /** 
       configure chip
       \param reg chip register
       \param ichip chip number (-1 all)
       \param imod module number (-1 all)
       \returns current register value
       \sa ::sls_detector_chip
  */
  int setChip(int reg, int ichip=-1, int imod=-1);
  
  /** 
       configure chip
       \param chip chip to be set - must contain correct module and chip number and also channel registers
       \returns current register value
       \sa ::sls_detector_chip
  */
  int setChip(sls_detector_chip chip); 

  /**
    get chip
    \param ichip chip number
    \param imod module number
    \returns current chip structure for channel

    \bug probably does not return corretly!
  */
  sls_detector_chip getChip(int ichip, int imod);


  /** 
     configure module
       \param imod module number (-1 all)
       \returns current register value
       \sa ::sls_detector_module
  */
  int setModule(int reg, int imod=-1); 

  /** 
       configure chip
       \param module module to be set - must contain correct module number and also channel and chip registers
       \returns current register value
       \sa ::sls_detector_module
  */
  int setModule(sls_detector_module module);

  /**
    get module
    \param imod module number
    \returns pointer to module structure (which has bee created and must then be deleted)
  */
  sls_detector_module *getModule(int imod);
 
  // calibration functions
  //  int setCalibration(int imod, detectorSettings isettings, float gain, float offset);
  //int getCalibration(int imod, detectorSettings isettings, float &gain, float &offset);
  

  /*
    calibrated setup of the threshold
  */  
  /**
    get threshold energy
    \param imod module number (-1 all)
    \returns current threshold value for imod in ev (-1 failed)
  */
  int getThresholdEnergy(int imod=-1);  

  /**
    set threshold energy
    \param e_eV threshold in eV
    \param imod module number (-1 all)
    \param isettings ev. change settings
    \returns current threshold value for imod in ev (-1 failed)
  */
  int setThresholdEnergy(int e_eV, int imod=-1, detectorSettings isettings=GET_SETTINGS); 
 
  /**
    get detector settings
    \param imod module number (-1 all)
    \returns current settings
  */
  detectorSettings getSettings(int imod=-1);  

  /**
    set detector settings
    \param isettings  settings
    \param imod module number (-1 all)
    \returns current settings

    in this function trimbits and calibration files are searched in the trimDir and calDir directories and the detector is initialized
  */
  detectorSettings setSettings(detectorSettings isettings, int imod=-1);


// Acquisition functions


  /**
    start detector acquisition
    \returns OK/FAIL
  */
  int startAcquisition();

  /**
    stop detector acquisition
    \returns OK/FAIL
  */
  int stopAcquisition();
  
  /**
    start readout (without exposure or interrupting exposure)
    \returns OK/FAIL
  */
  int startReadOut();

  /**
     get run status
    \returns status mask
  */
  int getRunStatus();

  /**
    start detector acquisition and read all data putting them a data queue
    \returns pointer to the fron tof the data queue
    \sa startAndReadAllNoWait getDataFromDetector dataQueue
  */ 
  int* startAndReadAll();
  
  /**
    start detector acquisition and read out, but does not read data from socket 
   
  */ 
  int startAndReadAllNoWait(); 

  /**
    receives a data frame from the detector socket
    \returns pointer to the data or NULL. If NULL disconnects the socket
    \sa getDataFromDetector
  */ 
  int* getDataFromDetectorNoWait(); 

  /**
   asks and  receives a data frame from the detector
    \returns pointer to the data or NULL. If NULL disconnects the socket
    \sa getDataFromDetector
  */ 
  int* readFrame(); 

  /**
   asks and  receives all data  from the detector  and puts them in a data queue
    \returns pointer to the front of the queue  or NULL. 
    \sa getDataFromDetector  dataQueue
  */ 
  int* readAll();

  
  /**
   pops the data from the data queue
    \returns pointer to the popped data  or NULL if the queue is empty. 
    \sa  dataQueue
  */ 
  int* popDataQueue();

  /** 
      set/get timer value
      \param index timer index
      \param t time in ns or number of...(e.g. frames, gates, probes)
      \returns timer set value in ns or number of...(e.g. frames, gates, probes)
  */
  int64_t setTimer(timerIndex index, int64_t t=-1);

  /** 
      get current timer value
      \param index timer index
      \returns elapsed time value in ns or number of...(e.g. frames, gates, probes)
  */
  int64_t getTimeLeft(timerIndex index);

  /** 
      set clock divider
      
      not implemented (should be something more general like "set speed including also waitstates, set cycles etc.)
      
  */
  int setClockDivider(int i) {cout << "not implemented any longer!"<< endl; thisDetector->clkDiv=i;return thisDetector->clkDiv;};  

  /** 
      get clock divider
      
      not implemented (should be something more general like "set speed including also waitstates, set cycles etc.)
      
  */
  int getClockDivider() {return thisDetector->clkDiv;};  

  /** 
      set length cycles
      
      not implemented (should be something more general like "set speed including also waitstates, set cycles etc.)
      
  */
  int setSetLength(int i) {cout << "not implemented any longer!"<< endl; thisDetector->clkDiv=i;return 3;}; 

  /** 
      get length cycles
      
      not implemented (should be something more general like "set speed including also waitstates, set cycles etc.)
  */
  int getSetLength() {return 3;};
  

  // Flags
  /** 
      set/get dynamic range
      \param n dynamic range (-1 get)
      \returns current dynamic range
      updates the size of the data expected from the detector
      \sa sharedSlsDetector
  */
  int setDynamicRange(int n=-1);
 
  /** 
      set roi

      not yet implemented
  */
  int setROI(int nroi=-1, int *xmin=NULL, int *xmax=NULL, int *ymin=NULL, int *ymax=NULL);
  

  /**
     set/get readout flags
     \param flag readout flag to be set
     \returns current flag
  */
  int setReadOutFlags(readOutFlags flag);

    /**
       execute trimming
     \param mode trim mode
     \param par1 if noise, beam or fixed setting trimming it is count limit, if improve maximum number of iterations
     \param par2 if noise or beam nsigma, if improve par2!=means vthreshold will be optimized, if fixed settings par2<0 trimwith median, par2>=0 trim with level
     \param imod module number (-1 all)
     \returns OK or FAIl (FAIL also if some channel are 0 or 63
  */
  int executeTrimming(trimMode mode, int par1, int par2, int imod=-1);

 
  //Corrections

  /** 
      set flat field corrections
      \param fname name of the flat field file (or "" if disable)
      \returns 0 if disable (or file could not be read), >0 otherwise
  */
  int setFlatFieldCorrection(string fname=""); 

  /** 
      get flat field corrections
      \param corr if !=NULL will be filled with the correction coefficients
      \param ecorr if !=NULL will be filled with the correction coefficients errors
      \returns 0 if ff correction disabled, >0 otherwise
  */
  int getFlatFieldCorrections(float *corr=NULL, float *ecorr=NULL);

  /** 
      set rate correction
      \param t dead time in ns - if 0 disable correction, if >0 set dead time to t, if <0 set deadtime to default dead time for current settings
      \returns 0 if rate correction disabled, >0 otherwise
  */
  int setRateCorrection(float t=0);

  
  /** 
      get rate correction
      \param t reference for dead time
      \returns 0 if rate correction disabled, >0 otherwise
  */
  int getRateCorrections(float &t);  

  /** 
      get rate correction
      \returns 0 if rate correction disabled, >0 otherwise
  */
  int getRateCorrections();
  
  /** 
      set bad channels correction
      \param fname file with bad channel list ("" disable)
      \returns 0 if bad channel disabled, >0 otherwise
  */
  int setBadChannelCorrection(string fname="");
  
  /** 
      get bad channels correction
      \param bad pointer to array that if bad!=NULL will be filled with the bad channel list
      \returns 0 if bad channel disabled or no bad channels, >0 otherwise
  */
  int getBadChannelCorrections(int *bad=NULL);

  
  /** 
      set angular conversion
      \param fname file with angular conversion constants ("" disable)
      \returns 0 if angular conversion disabled, >0 otherwise
  */
  int setAngularConversion(string fname="");

  /** 
      get angular conversion
      \param reference to diffractometer direction
      \param angconv array that will be filled with the angular conversion constants
      \returns 0 if angular conversion disabled, >0 otherwise
  */
  int getAngularConversion(int &direction,  angleConversionConstant *angconv=NULL);
  
  /** 
      set detector global offset
  */
  float setGlobalOffset(float f){thisDetector->globalOffset=f; return thisDetector->globalOffset;}; 

  /** 
      set detector fine offset
  */
  float setFineOffset(float f){thisDetector->fineOffset=f; return thisDetector->fineOffset;};
  /** 
      get detector fine offset
  */
  float getFineOffset(){return thisDetector->fineOffset;};
  
  /** 
      get detector global offset
  */
  float getGlobalOffset(){return thisDetector->globalOffset;};

  /** 
      set  positions for the acquisition
      \param nPos number of positions
      \param pos array with the encoder positions
      \returns number of positions
  */
  int setPositions(int nPos, float *pos){thisDetector->numberOfPositions=nPos; for (int ip=0; ip<nPos; ip++) thisDetector->detPositions[ip]=pos[ip]; return thisDetector->numberOfPositions;};
   /** 
      get  positions for the acquisition
      \param pos array which will contain the encoder positions
      \returns number of positions
  */
  int getPositions(float *pos=NULL){ if (pos ) {for (int ip=0; ip<thisDetector->numberOfPositions; ip++) pos[ip]=thisDetector->detPositions[ip];} return thisDetector->numberOfPositions;};
  
  
  /** set detector bin size used for merging (approx angular resolution)*/
  float setBinSize(float bs) {thisDetector->binSize=bs; return thisDetector->binSize;}
  /** return detector bin size used for merging (approx angular resolution)*/
  float getBinSize() {return thisDetector->binSize;}






  /** 
     decode data from the detector converting them to an array of floats, one for each channle
     \param datain data from the detector
     \returns pointer to a float array with a data per channel
  */
  float* decodeData(int *datain);

  
  
  
  /** 
     flat field correct data
     \param datain data
     \param errin error on data (if<=0 will default to sqrt(datain)
     \param dataout corrected data
     \param errout error on corrected data
     \param ffcoefficient flat field correction coefficient
     \param fferr erro on ffcoefficient
     \returns 0
  */
  int flatFieldCorrect(float datain, float errin, float &dataout, float &errout, float ffcoefficient, float fferr);
  
  /** 
     flat field correct data
     \param datain data array
     \param errin error array on data (if NULL will default to sqrt(datain)
     \param dataout array of corrected data
     \param errout error on corrected data (if not NULL)
     \returns 0
  */
  int flatFieldCorrect(float* datain, float *errin, float* dataout, float *errout);
 

  
  /** 
     rate correct data
     \param datain data
     \param errin error on data (if<=0 will default to sqrt(datain)
     \param dataout corrected data
     \param errout error on corrected data
     \param tau dead time 9in ns)
     \param t acquisition time (in ns)
     \returns 0
  */
  int rateCorrect(float datain, float errin, float &dataout, float &errout, float tau, float t);
  
  /** 
     rate correct data
     \param datain data array
     \param errin error array on data (if NULL will default to sqrt(datain)
     \param dataout array of corrected data
     \param errout error on corrected data (if not NULL)
     \returns 0
  */
  int rateCorrect(float* datain, float *errin, float* dataout, float *errout);

  
  /** sets the arrays of the merged data to 0. NB The array should be created with size >= 360./getBinSize(); 
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \returns OK or FAIL
  */
  int resetMerging(float *mp, float *mv,float *me, int *mm);
  /** not yet implemented 
      merge dataset
      \param p1 angular positions of dataset
      \param v1 data
      \param e1 errors
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
  */
  int addToMerging(float *p1, float *v1, float *e1, float *mp, float *mv,float *me, int *mm);

  /** 
      calculates the "final" positions, data value and errors for the emrged data
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \returns FAIL or the 
  */
  int finalizeMerging(float *mp, float *mv,float *me, int *mm);

  /** 
      turns of server
  */
  int exitServer();

  /**
     function for processing data
  */
  void* processData(); // thread function
  /** Allocates the memory for a sls_detector_module structure and initializes it
      \returns myMod the pointer to the allocate dmemory location

  */
  sls_detector_module*  createModule();
  /** frees the memory for a sls_detector_module structure 
      \param myMod the pointer to the memory to be freed

  */
  
  void deleteModule(sls_detector_module *myMod);


  void acquire();

 protected:
 
  /**
    address of the detector structure in shared memory
  */
  sharedSlsDetector *thisDetector;

  /**
     \sa setOnline
  */
  int onlineFlag;
  
  /**
     detector ID
  */
  int detId;
  
  /**
     shared memeory ID
  */
  int shmId;

  /**
     socket for control commands
  */
  MySocketTCP *controlSocket;

  /**
     socket for emergency stop
  */
  MySocketTCP *stopSocket;
  
  /**
     socket for data acquisition
  */
  MySocketTCP *dataSocket; 
  
  /**
     data queue
  */
  queue<int*> dataQueue;
  
  /**
     data processing thread???
  */
  pthread_t dataProcessingThread;
  
  /**
     current position of the detector
  */
  float currentPosition;
  
  /**
     current position index of the detector
  */
  float currentPositionIndex;
  
  /**
     I0 measured
  */
  float currentI0;
  


  /** merging bins */
  float *mergingBins;

  /** merging counts */
  float *mergingCounts;

  /** merging errors */
  float *mergingErrors;

  /** merging multiplicity */
  int *mergingMultiplicity;
  





 
  /** pointer to flat field coefficients */
  float *ffcoefficients;
  /** pointer to flat field coefficient errors */
  float *fferrors;
  /** pointer to detector module structures */
  sls_detector_module *detectorModules;
  /** pointer to dac valuse */
  float *dacs;
  /** pointer to adc valuse */
  float *adcs;
  /** pointer to chip registers */
  int *chipregs;
  /** pointer to channal registers */
  int *chanregs;
  /** pointer to bad channel mask  0 is channel is good 1 if it is bad \sa fillBadChannelMask() */
  int *badChannelMask;

  /** 
      Receives a data frame from the detector socket
      \return pointer to the data (or NULL if failed)

  */
  int* getDataFromDetector();

  /** Initializes the shared memory 
      \param type is needed to define the size of the shared memory
      \param id is the detector id needed to define the shared memory id
      \return shm_id shared memory id
  */
  int initSharedMemory(detectorType type=GENERIC, int id=0);

  /** Frees the shared memory  -  should not be used*/
  int freeSharedMemory();
  /** 
      Initializes the thisDetector structure
      \param type is needed to define the number of channels, chips, modules etc.
      \sa sharedSlsDetector
  */
  int initializeDetectorSize(detectorType type);
  /**
     Initializes the module structures in thisDetector if the detector did not exists before
  */
  int initializeDetectorStructure(); 
  /**
     send a sls_detector_channel structure over socket
  */
  int sendChannel(sls_detector_channel*); 
  /**
     send a sls_detector_chip structure over socket
  */
  int sendChip(sls_detector_chip*);
  /**
     send a sls_detector_module structure over socket
  */
  int sendModule(sls_detector_module*);
  /**
     receive a sls_detector_channel structure over socket
  */
  int receiveChannel(sls_detector_channel*);
  /**
     receive a sls_detector_chip structure over socket
  */
  int receiveChip(sls_detector_chip*);
  /**
     receive a sls_detector_module structure over socket
  */
  int receiveModule(sls_detector_module*);
  
  /**
    start data processing threas
  */
  void startThread();
  
  /**
     fill bad channel mask (0 if channel is good, 1 if bad)
  */
  int fillBadChannelMask();
};


//static void* startProcessData(void *n);
#endif
