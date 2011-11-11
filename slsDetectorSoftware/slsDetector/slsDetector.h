


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


#include "sls_detector_defs.h"

#define MAX_TIMERS 10
#define MAX_ROIS 100
#define MAX_BADCHANS 2000
#define MAXPOS 50
#define MAX_SCAN_LEVELS 2
#define MAX_SCAN_STEPS 2000

#define NMODMAXX 24
#define NMODMAXY 24
#define MAXMODS 36
#define NCHIPSMAX 10
#define NCHANSMAX 65536
#define NDACSMAX 16

#define DEFAULT_HOSTNAME  "localhost"
#define DEFAULT_SHM_KEY  5678

#define defaultTDead {170,90,750} /**< should be changed in order to have it separate for the different detector types */



enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS};


/**
   data structure to hold the detector data after postprocessing (e.g. to plot, store in a root tree etc.)
 */
class detectorData {
 public:
  /** The constructor
      \param val pointer to the data
      \param err pointer to errors
      \param ang pointer to the angles
      \param f_ind file index
      \param fname file name to which the data are saved
      \param np number of points defaults to the number of detector channels
  */
  detectorData(float *val=NULL, float *err=NULL, float *ang=NULL,  int p_ind=-1, const char *fname="", int np=-1) : values(val), errors(err), angles(ang),  progressIndex(p_ind), npoints(np){strcpy(fileName,fname);};
    /** 
	the destructor
	deletes also the arrays pointing to data/errors/angles if not NULL
    */
    ~detectorData() {if (values) delete [] values; if (errors) delete [] errors; if (angles) delete [] angles;};
    //private:
    float *values; /**< pointer to the data */
    float *errors; /**< pointer to the errors */
    float *angles;/**< pointer to the angles */
    int progressIndex;/**< file index */
    char fileName[1000];/**< file name */
    int npoints;/**< number of points */
};



//using namespace std;
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
 \section howto_sec How to use it
 The best way to operate the slsDetectors is to use the software (text client or GUI) developed by the sls detectors group.
In case you need to embed the detector control in a previously existing software, compile these classes using <BR>
make package
<br>
and link the shared library created to your software bin/libSlsDetector.so.1.0.1
Then in your software you should use the class related to the detector you want to control (mythenDetector or eigerDetector).

 @author Anna Bergamaschi

*/



/**
 * 
 *
@libdoc The slsDetector class is expected to become the interface class for all SLS Detectors acquisition (and analysis) software.
 *
 * @short This is the base class for all SLS detector functionalities
 * @author Anna Bergamaschi
 * @version 0.1alpha


 */

class slsDetector {



 public:
  

  /** online flags enum \sa setOnline*/
  enum {GET_ONLINE_FLAG=-1, /**< returns wether the detector is in online or offline state */
	OFFLINE_FLAG=0, /**< detector in offline state (i.e. no communication to the detector - using only local structure - no data acquisition possible!) */
	ONLINE_FLAG =1/**< detector in online state (i.e. communication to the detector updating the local structure) */
  };



  /** 
      @short Structure allocated in shared memory to store detector settings and be accessed in parallel by several applications (take care of possible conflicts!)
      
  */
typedef  struct sharedSlsDetector {
  /** already existing flag. If the detector does not yet exist (alreadyExisting=0) the sharedMemory will be created, otherwise it will simly be linked */
    int alreadyExisting;
  /** online flag - is set if the detector is connected, unset if socket connection is not possible  */
  int onlineFlag;
 

  /** stopped flag - is set if an acquisition error occurs or the detector is stopped manually. Is reset to 0 at the start of the acquisition */
  int stoppedFlag;

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


  /** path of the trimbits/settings files */
    char settingsDir[MAX_STR_LENGTH];
  /** path of the calibration files */
    char calDir[MAX_STR_LENGTH];
  /** number of energies at which the detector has been trimmed (unused) */
    int nTrimEn;
  /** list of the energies at which the detector has been trimmed (unused) */
    int trimEnergies[100];


  /** indicator for the acquisition progress - set to 0 at the beginning of the acquisition and incremented every time that the data are written to file */   
    int progressIndex;	
  /** total number of frames to be acquired */   
    int totalProgress;	   
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
  /** threaded processing flag (i.e. if data are processed and written to file in a separate thread)  */
  int threadedProcessing;
    /** dead time (in ns) for rate corrections */
    float tDead;
  /** directory where the flat field files are stored */
  char flatFieldDir[MAX_STR_LENGTH];
  /** file used for flat field corrections */
  char flatFieldFile[MAX_STR_LENGTH];
    /** number of bad channels from bad channel list */
    int nBadChans;
  /** file with the bad channels */
  char badChanFile[MAX_STR_LENGTH];
    /** list of bad channels */
    int badChansList[MAX_BADCHANS];
    /** number of bad channels from flat field i.e. channels which read 0 in the flat field file */
    int nBadFF;
    /** list of bad channels from flat field i.e. channels which read 0 in the flat field file */
    int badFFList[MAX_BADCHANS];
    
  /** file with the angular conversion factors */
  char angConvFile[MAX_STR_LENGTH];
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
  /** name root of the output files */  
  char settingsFile[MAX_STR_LENGTH];
  /** detector settings (standard, fast, etc.) */
  detectorSettings currentSettings;
  /** detector threshold (eV) */
  int currentThresholdEV;
  /** timer values */
  int64_t timerValue[MAX_TIMERS];
  /** clock divider */
  //int clkDiv;


  /** Scans and scripts */

  int actionMask;
  
  int actionMode[MAX_ACTIONS];
  char actionScript[MAX_ACTIONS][MAX_STR_LENGTH];
  char actionParameter[MAX_ACTIONS][MAX_STR_LENGTH];


  int scanMode[MAX_SCAN_LEVELS];
  char scanScript[MAX_SCAN_LEVELS][MAX_STR_LENGTH];
  char scanParameter[MAX_SCAN_LEVELS][MAX_STR_LENGTH];
  int nScanSteps[MAX_SCAN_LEVELS];
  float scanSteps[MAX_SCAN_LEVELS][MAX_SCAN_STEPS];
  int scanPrecision[MAX_SCAN_LEVELS];
  

    
  /*offsets*/
    /** memory offsets for the flat field coefficients */
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


  slsDetector(char *name, int id=0,  int cport=DEFAULT_PORTNO);
  //slsDetector(string  const fname);
  //  ~slsDetector(){while(dataQueue.size()>0){}};
  /** destructor */ 
  virtual ~slsDetector();//{ disconnect_channels();};


  /** sets the onlineFlag
      \param off can be: <BR> GET_ONLINE_FLAG, returns wether the detector is in online or offline state;<BR> OFFLINE_FLAG, detector in offline state (i.e. no communication to the detector - using only local structure - no data acquisition possible!);<BR> ONLINE_FLAG  detector in online state (i.e. communication to the detector updating the local structure) */
  int setOnline(int const online=GET_ONLINE_FLAG);  
  /** sets the onlineFlag
      \returns 1 if the detector structure has already be initlialized, 0 otherwise */
  int exists() {return thisDetector->alreadyExisting;};


  /**
    Purely virtual function
    Should be implemented in the specific detector class
    /sa mythenDetector::readConfigurationFile
  */

  virtual int readConfigurationFile(string const fname){};  
  /**  
    Purely virtual function
    Should be implemented in the specific detector class
    /sa mythenDetector::writeConfigurationFile
  */
  virtual int writeConfigurationFile(string const fname){};


  /* 
     It should be possible to dump all the settings of the detector (including trimbits, threshold energy, gating/triggering, acquisition time etc.
     in a file and retrieve it for repeating the measurement with identicals ettings, if necessary
  */
  /** 
    
    Purely virtual function
    Should be implemented in the specific detector class
    /sa mythenDetector::dumpDetectorSetup
  */
  virtual int dumpDetectorSetup(string const fname, int level=0){};  
  /** 
    Purely virtual function
    Should be implemented in the specific detector class
    /sa mythenDetector::retrieveDetectorSetup
  */
  virtual int retrieveDetectorSetup(string const fname, int level=0){};

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
  /** returns the detector trimbit/settings directory  \sa sharedSlsDetector */
  char* getSettingsDir() {return thisDetector->settingsDir;};
  /** sets the detector trimbit/settings directory  \sa sharedSlsDetector */
  char* setSettingsDir(string s) {sprintf(thisDetector->settingsDir, s.c_str()); return thisDetector->settingsDir;};
  /** returns the number of trim energies and their value  \sa sharedSlsDetector 
   \param point to the array that will contain the trim energies (in ev)
  \returns number of trim energies

  unused!

  \sa  sharedSlsDetector
  */
  int getTrimEn(int *en=NULL) {if (en) {for (int ien=0; ien<thisDetector->nTrimEn; ien++) en[ien]=thisDetector->trimEnergies[ien];} return (thisDetector->nTrimEn);};
  /** sets the number of trim energies and their value  \sa sharedSlsDetector 
   \param nen number of energies
   \param en array of energies
   \returns number of trim energies

   unused!

  \sa  sharedSlsDetector
  */
  int setTrimEn(int nen, int *en=NULL) {if (en) {for (int ien=0; ien<nen; ien++) thisDetector->trimEnergies[ien]=en[ien]; thisDetector->nTrimEn=nen;} return (thisDetector->nTrimEn);};

  /**
     Pure virtual function
     reads a trim/settings file
     \param fname name of the file to be read
     \param myMod pointer to the module structure which has to be set. <BR> If it is NULL a new module structure will be created
     \returns the pointer to myMod or NULL if reading the file failed
     \sa mythenDetector::readSettingsFile
  */

  virtual sls_detector_module* readSettingsFile(string fname,  sls_detector_module* myMod=NULL){};

  /**
     Pure virtual function
     writes a trim/settings file
     \param fname name of the file to be written
     \param mod module structure which has to be written to file
     \returns OK or FAIL if the file could not be written

     \sa ::sls_detector_module mythenDetector::writeSettingsFile(string, sls_detector_module)
  */
  virtual int writeSettingsFile(string fname, sls_detector_module mod){}; 
  
  /**
     Pure virtual function
     writes a trim/settings file for module number imod - the values will be read from the current detector structure
     \param fname name of the file to be written
     \param imod module number
     \returns OK or FAIL if the file could not be written   
     \sa ::sls_detector_module sharedSlsDetector mythenDetector::writeSettingsFile(string, int)
  */
  virtual int writeSettingsFile(string fname, int imod){};


  /**
     returns currently the loaded trimfile/settingsfile name
  */
  const char *getSettingsFile(){\
    string s(thisDetector->settingsFile); \
    if (s.length()>6) {\
      if (s.substr(s.length()-6,3)==string(".sn") && s.substr(s.length()-3)!=string("xxx") ) \
	return s.substr(0,s.length()-6).c_str();			\
    }									\
    return thisDetector->settingsFile;\
  };

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
      \param filepath outdir
      \param filename file root name
      \param aMask action mask (scans, positions)
      \param sv0 scan variable 0
      \param prec0 scan precision 0
      \param sv1 scan variable 1
      \param prec1 scan precision 1
      \param pindex position index
      \param number of positions
      \param findex file index
      \returns file name without extension
  */
  static string  createFileName(char *filepath, char *filename, int aMask, float sv0, int prec0, float sv1, int prec1, int pindex, int npos, int findex);
  /** generates file name without extension

      always appends to file path and file name the run index.

      in case also appends the position index 
       
      Filenames will be of the form: filepath/filename(_px)_i
      where x is the position index and i is the run index
      \returns file name without extension
  */

  string createFileName();
  


  /** static function that returns the file index from the file name 
      \param fname file name
      \returns file index*/
  static int getFileIndexFromFileName(string fname);

  /** static function that returns the variables from the file name 
      \param fname file name
      \param index reference to index
      \param p_index reference to position index
      \param sv0 reference to scan variable 0
      \param sv1 reference to scan variable 1
      \returns file index
  */
  static int getVariablesFromFileName(string fname, int &index, int &p_index, float &sv0, float &sv1);
  
    /**
     
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \param err array of arrors on the data. If NULL no errors will be written
       
       \param ang array of angular values. If NULL data will be in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if -1 defaults to the number of installed channels of the detector
       \returns OK or FAIL if it could not write the file or data=NULL
       \sa mythenDetector::writeDataFile
 
  */
  virtual int writeDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=-1){}; 
  
  /**
   
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \returns OK or FAIL if it could not write the file or data=NULL  
       \sa mythenDetector::writeDataFile
  */
  virtual int writeDataFile(string fname, int *data){};
  
  /**
   
       reads a data file
       \param name of the file to be read
       \param data array of data values to be filled
       \param err array of arrors on the data. If NULL no errors are expected on the file
       
       \param ang array of angular values. If NULL data are expected in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if <=0 defaults to the number of installed channels of the detector
       \returns OK or FAIL if it could not read the file or data=NULL
       
       \sa mythenDetector::readDataFile
  */
  virtual int readDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=0){};  

  /**
   
       reads a data file
       \param name of the file to be read
       \param data array of data values
       \returns OK or FAIL if it could not read the file or data=NULL
       \sa mythenDetector::readDataFile
  */
  virtual int readDataFile(string fname, int *data){};

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
  \sa  sharedSlsDetector mythenDetector::readCalibrationFile
  */
  virtual int readCalibrationFile(string fname, float &gain, float &offset){};
  /**
   
      writes a calibration file
      \param fname file to be written
      \param gain 
      \param offset
  \sa  sharedSlsDetector mythenDetector::writeCalibrationFile
  */
  virtual int writeCalibrationFile(string fname, float gain, float offset){};


  /**
   
      reads an angular conversion file
      \param fname file to be read
  \sa  angleConversionConstant mythenDetector::readAngularConversion
  */
  int readAngularConversion(string fname="");
  /**
     Pure virtual function
     writes an angular conversion file
      \param fname file to be written
  \sa  angleConversionConstant mythenDetector::writeAngularConversion
  */
  int writeAngularConversion(string fname="");

  /** Returns the number of channels per chip */
  int getNChans(){return thisDetector->nChans;}; //

  /** Returns the number of chips per module */
  int getNChips(){return thisDetector->nChips;}; //


  /* Communication to server */


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
  string getDetectorType();


  // Detector configuration functions
  /** 
      set/get the size of the detector 
      \param n number of modules
      \param d dimension
      \returns current number of modules in direction d
  */

  // Detector configuration functions
  /** 
      set/get the size of the detector 
      \param n number of modules
      \param d dimension
      \returns current number of modules in direction d
  */
  int setNumberOfModules(int n=GET_FLAG, dimension d=X); // if n=GET_FLAG returns the number of installed modules

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
  virtual int setModule(int reg, int imod=-1); 

  /** 
       configure chip
       \param module module to be set - must contain correct module number and also channel and chip registers
       \returns current register value
       \sa ::sls_detector_module
  */
  virtual int setModule(sls_detector_module module);

  /**
    get module
    \param imod module number
    \returns pointer to module structure (which has bee created and must then be deleted)
  */
  virtual sls_detector_module *getModule(int imod);
 
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

    in this function trimbits/settings and calibration files are searched in the settingsDir and calDir directories and the detector is initialized
  */
  virtual detectorSettings setSettings(detectorSettings isettings, int imod=-1);


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
  //virtual runStatus  getRunStatus()=0;
  runStatus  getRunStatus();

  /**
    start detector acquisition and read all data putting them a data queue
    \returns pointer to the front of the data queue
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
   asks and  receives a data frame from the detector and puts it in the data queue
    \returns pointer to the data or NULL. 
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
   pops the data from thepostprocessed data queue
    \returns pointer to the popped data  or NULL if the queue is empty. 
    \sa  finalDataQueue
  */ 
  detectorData* popFinalDataQueue();




  /**
  resets the raw data queue
    \sa  dataQueue
  */ 
  void resetDataQueue();

  /**
  resets the postprocessed  data queue
    \sa  finalDataQueue
  */ 
  void resetFinalDataQueue();









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




  /** sets/gets the value of important readout speed parameters
      \param sp is the parameter to be set/get
      \param value is the value to be set, if -1 get value
      \returns current value for the specified parameter
      \sa speedVariable
   */
  int setSpeed(speedVariable sp, int value=-1);

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
      set/get dynamic range
      \returns number of bytes sent by the detector
      \sa sharedSlsDetector
  */
  int getDataBytes(){return thisDetector->dataBytes;};
 


  

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
      set/get if the data processing and file writing should be done by a separate thread
s
      \param b 0 sequencial data acquisition and file writing, 1 separate thread, -1 get
      \returns thread flag
  */

  int setThreadedProcessing(int b=-1) {if (b>=0) thisDetector->threadedProcessing=b; return  thisDetector->threadedProcessing;}

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
  int getFlatFieldCorrection(float *corr=NULL, float *ecorr=NULL);

 /** 
      get flat field corrections file directory
      \returns flat field correction file directory
  */
  char *getFlatFieldCorrectionDir(){return thisDetector->flatFieldDir;};
 /** 
      set flat field corrections file directory
      \param flat field correction file directory
  */
  void setFlatFieldCorrectionDir(string dir){strcpy(thisDetector->flatFieldDir,dir.c_str());};
  
 /** 
      get flat field corrections file name
      \returns flat field correction file name
  */
  char *getFlatFieldCorrectionFile(){  if (thisDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION)) return thisDetector->flatFieldFile; else return "none";};

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
  int getRateCorrection(float &t);

  
  /** 
      get rate correction tau
      \returns 0 if rate correction disabled, otherwise the tau used for the correction
  */
  float getRateCorrectionTau();
  /** 
      get rate correction
      \returns 0 if rate correction disabled, >0 otherwise
  */
  int getRateCorrection();
  
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
  int getBadChannelCorrection(int *bad=NULL);

  /** returns the bad channel list file */
  string getBadChannelCorrectionFile() {if (thisDetector->correctionMask&(1<< DISCARD_BAD_CHANNELS)) return string(thisDetector->badChanFile); else return string("none");};

  
  /** 
      pure virtual function
      set angular conversion
      \param fname file with angular conversion constants ("" disable)
      \returns 0 if angular conversion disabled, >0 otherwise
      \sa mythenDetector::setAngularConversion
  */
  int setAngularConversion(string fname="");

  /** 
      pure virtual function
      get angular conversion
      \param reference to diffractometer direction
      \param angconv array that will be filled with the angular conversion constants
      \returns 0 if angular conversion disabled, >0 otherwise
      \sa mythenDetector::getAngularConversion
  */
  int getAngularConversion(int &direction,  angleConversionConstant *angconv=NULL) ;
  
  
  /**
      pure virtual function
      returns the angular conversion file
      \sa mythenDetector::getAngularConversion */
  string getAngularConversion(){if (thisDetector->correctionMask&(1<< ANGULAR_CONVERSION)) return string(thisDetector->angConvFile); else return string("none");};
  
  /** 
      pure virtual function
      set detector global offset
      \sa mythenDetector::setGlobalOffset
  */
  float setGlobalOffset(float f){thisDetector->globalOffset=f; return thisDetector->globalOffset;}; 

  /** 
      pure virtual function
      set detector fine offset
      \sa mythenDetector::setFineOffset
  */
  float setFineOffset(float f){thisDetector->fineOffset=f; return thisDetector->fineOffset;};
  /** 
      pure virtual function
      get detector fine offset
      \sa mythenDetector::getFineOffset
  */
  float getFineOffset(){return thisDetector->fineOffset;};
  
  /** 
      pure virtual function
      get detector global offset
      \sa mythenDetector::getGlobalOffset
  */
  float getGlobalOffset(){return thisDetector->globalOffset;};

  /** 
      pure virtual function
      set  positions for the acquisition
      \param nPos number of positions
      \param pos array with the encoder positions
      \returns number of positions
      \sa mythenDetector::setPositions
  */
  int setPositions(int nPos, float *pos);
   /** 
      pure virtual function
      get  positions for the acquisition
      \param pos array which will contain the encoder positions
      \returns number of positions
      \sa mythenDetector::getPositions
  */
  int getPositions(float *pos=NULL);
  
  
  /** pure virtual function
      set detector bin size used for merging (approx angular resolution)
      \param bs bin size in degrees
      \returns current bin size
      \sa mythenDetector::setBinSize
*/
  float setBinSize(float bs){thisDetector->binSize=bs; return thisDetector->binSize;};

  /** pure virtual function
      return detector bin size used for merging (approx angular resolution)
      \sa mythenDetector::getBinSize
  */
  float getBinSize() {return thisDetector->binSize;};





  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param fname for script ("" disable)
      \param par for script 
      \returns 0 if action disabled, >0 otherwise
  */
  int setAction(int iaction, string fname="", string par="");

  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param fname for script ("" disable)
      \returns 0 if action disabled, >0 otherwise
  */
  int setActionScript(int iaction, string fname="");
  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param par for script ("" disable)
      \returns 0 if action disabled, >0 otherwise
  */
  int setActionParameter(int iaction, string par="");

  /** 
      returns action script
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
      \returns action script
  */
  string getActionScript(int iaction);

    /** 
	returns action parameter
	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
	\returns action parameter
    */
  string getActionParameter(int iaction);

   /** 
	returns action mode
	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
	\returns action mode
    */
  int getActionMode(int iaction);


  /** 
      set scan 
      \param index of the scan (0,1)
      \param fname for script ("" disables, "none" disables and overwrites current)
      \param nvalues number of steps (0 disables, -1 leaves current value)
      \param values pointer to steps (if NULL leaves current values)
      \param par parameter for the scan script ("" leaves unchanged)
      \returns 0 is scan disabled, >0 otherwise
  */
  int setScan(int index, string script="", int nvalues=-1, float *values=NULL, string par="", int precision=-1);
  
  int setScanScript(int index, string script="");
  int setScanParameter(int index, string par="");
  int setScanPrecision(int index, int precision=-1);
  int setScanSteps(int index, int nvalues=-1, float *values=NULL);
  /** 
      returns scan script
      \param iscan can be (0,1) 
      \returns scan script
  */
  string getScanScript(int iscan);

    /** 
	returns scan parameter
	\param iscan can be (0,1)
	\returns scan parameter
    */
  string getScanParameter(int iscan);

   /** 
	returns scan mode
	\param iscan can be (0,1)
	\returns scan mode
    */
  int getScanMode(int iscan);

   /** 
	returns scan steps
	\param iscan can be (0,1)
	\param v is the pointer to the scan steps
	\returns scan steps
    */
  int getScanSteps(int iscan, float *v=NULL);


   /** 
	returns scan precision
	\param iscan can be (0,1)
	\returns scan precision
    */
  int getScanPrecision(int iscan);




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

  
  /** 
      pure virtual function
  sets the arrays of the merged data to 0. NB The array should be created with size >= 360./getBinSize(); 
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \returns OK or FAIL
      \sa mythenDetector::resetMerging
  */
  int resetMerging(float *mp, float *mv,float *me, int *mm);
  /** 
      pure virtual function
  merge dataset
      \param p1 angular positions of dataset
      \param v1 data
      \param e1 errors
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \sa mythenDetector::addToMerging
  */
  int addToMerging(float *p1, float *v1, float *e1, float *mp, float *mv,float *me, int *mm);

  /** pure virtual function
      calculates the "final" positions, data value and errors for the emrged data
      \param mp already merged postions
      \param mv already merged data
      \param me already merged errors (squared sum)
      \param mm multiplicity of merged arrays
      \returns FAIL or the number of non empty bins (i.e. points belonging to the pattern)
      \sa mythenDetector::finalizeMerging
  */
  int finalizeMerging(float *mp, float *mv,float *me, int *mm);

  /** 
      turns off server
  */
  int exitServer();

  /** pure virtual function
     function for processing data
     /param delflag if 1 the data are processed, written to file and then deleted. If 0 they are added to the finalDataQueue
     \sa mythenDetector::processData
  */
  void* processData(int delflag=1); // thread function
  /** Allocates the memory for a sls_detector_module structure and initializes it
      \returns myMod the pointer to the allocate dmemory location

  */
  sls_detector_module*  createModule();
  /** frees the memory for a sls_detector_module structure 
      \param myMod the pointer to the memory to be freed

  */
  
  void deleteModule(sls_detector_module *myMod);


  /** pure virtual function
      performs the complete acquisition and data processing 
     moves the detector to next position <br>
     starts and reads the detector <br>
     reads the IC (if required) <br>
     reads the encoder (iof required for angualr conversion) <br>
     processes the data (flat field, rate, angular conversion and merging ::processData())
     /param delflag if 1 the data are processed, written to file and then deleted. If 0 they are added to the finalDataQueue
     \sa mythenDetector::acquire()
  */
  
  void acquire(int delflag=1);

  /** calcualtes the total number of steps of the acquisition.
      called when number of frames, number of cycles, number of positions and scan steps change
  */
  int setTotalProgress();

  /** returns the current progress in % */
  float getCurrentProgress();
  



 /**
    type of action performed (for text client)
 */
enum {GET_ACTION, PUT_ACTION, READOUT_ACTION};


  /**
     executes a set of string arguments according to a given format. It is used to read/write configuration file, dump and retrieve detector settings and for the command line interface command parsing
     \param narg number of arguments
     \param args array of string arguments
     \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition)
     \returns answer string 
  */
    string executeLine(int narg, char *args[], int action=GET_ACTION);
  
  /**
     returns the help for the executeLine command 
     \param os output stream to return the help to
     \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition) 
  */
   static string helpLine(int action=GET_ACTION);



  /**
     returns the detector type from hostname and controlport
     \param 
     \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition) 
  */
   static detectorType getDetectorType(char *name, int cport=DEFAULT_PORTNO);

  /**
     returns the detector type from hostname and controlport
     \param 
     \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition) 
  */
   static detectorType getDetectorType(int id);


   int getDetectorId() { return detId;};
   


 protected:
 

  static const int64_t thisSoftwareVersion=0x20110113;

  /**
    address of the detector structure in shared memory
  */
  sharedSlsDetector *thisDetector;

  //  /**
  //   \sa setOnline
  // */
  //int onlineFlag;
  
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
     queue containing the postprocessed data
  */
  queue<detectorData*> finalDataQueue;
  
  


  /**
     current position of the detector
  */
  float currentPosition;
  
  /**
     current position index of the detector
  */
  int currentPositionIndex;
  
  /**
     I0 measured
  */
  float currentI0;
  
  


  /**
     current scan variable of the detector
  */
  float currentScanVariable[MAX_SCAN_LEVELS];
  
  /**
     current scan variable index of the detector
  */
  int currentScanIndex[MAX_SCAN_LEVELS];
  
  


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
    start data processing thread
  */
  //void startThread();
  
  /**
     fill bad channel mask (0 if channel is good, 1 if bad)
  */
  int fillBadChannelMask();


  /**
    start data processing thread
  */
  void startThread(int delflag=1); //
  /** the data processing thread */

  pthread_t dataProcessingThread;

 /** sets when the acquisition is finished */
  int jointhread;

 /** data queue size */
  int queuesize;




 /** mutex to synchronize threads */
  pthread_mutex_t mp;









};

static void* startProcessData(void *n);
static void* startProcessDataNoDelete(void *n);


//static void* startProcessData(void *n);
#endif
