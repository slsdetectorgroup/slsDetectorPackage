


#ifndef SLS_DETECTOR_H
#define SLS_DETECTOR_H


#include "multiSlsDetector.h"
#include "slsDetectorUtils.h"
#include "energyConversion.h"
#include "angleConversionConstant.h"
#include "MySocketTCP.h"

#include "angleConversionConstant.h"

#include "receiverInterface.h"


/**
 * 
 * @short the slsDetector class takes care of the communication with the detector and all kind actions related with a single detector controller
 * @author Anna Bergamaschi
 * @version 0.1alpha
 */

#define NMODMAXX 24
#define NMODMAXY 24
#define NCHIPSMAX 10
#define NCHANSMAX 65536
#define NDACSMAX 16


/**
   @short complete detector functionalities for a single module detector
*/
class slsDetector : public slsDetectorUtils, public energyConversion {



 public:
  
  /*   /\** online flags enum \sa setOnline*\/ */
  /*   enum {GET_ONLINE_FLAG=-1, /\**< returns wether the detector is in online or offline state *\/ */
  /* 	OFFLINE_FLAG=0, /\**< detector in offline state (i.e. no communication to the detector - using only local structure - no data acquisition possible!) *\/ */
  /* 	ONLINE_FLAG =1/\**< detector in online state (i.e. communication to the detector updating the local structure) *\/ */
  /*   }; */



  /** 
      @short Structure allocated in shared memory to store detector settings.

      Structure allocated in shared memory to store detector settings and be accessed in parallel by several applications on the same machine (take care of possible conflicts, particularly if things are run on different machines!)
      
  */
  typedef  struct sharedSlsDetector {
    /** already existing flag. If the detector does not yet exist (alreadyExisting=0) the sharedMemory will be created, otherwise it will simly be linked */
    int alreadyExisting;




    /** last process id accessing the shared memory */
   
    pid_t  lastPID;




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

    /** path of the output files */
    char filePath[MAX_STR_LENGTH];

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
    /**  number of channels per chip in one direction */
    int nChan[2];
    /**  number of chips per module*/
    int nChips;
    /**  number of chips per module in one direction */
    int nChip[2];
    /**  number of dacs per module*/
    int nDacs;
    /** number of adcs per module */
    int nAdcs;
    /**  number of extra gain values*/
    int nGain;
    /** number of extra offset values */
    int nOffset;
    /** dynamic range of the detector data */
    int dynamicRange;
    /**  size of the data that are transfered from the detector */
    int dataBytes;
    


    /** corrections  to be applied to the data \see ::correctionFlags */
    int correctionMask;
    /** threaded processing flag (i.e. if data are processed and written to file in a separate thread)  */
    int threadedProcessing;
    /** dead time (in ns) for rate corrections */
    double tDead;
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
    double fineOffset;
    /** beamline offset (might be a few degrees beacuse of encoder offset - normally it is kept fixed for a long period of time)  */
    double globalOffset;
    /** number of positions at which the detector should acquire  */
    int numberOfPositions;
    /** list of encoder positions at which the detector should acquire */
    double detPositions[MAXPOS];
    /** bin size for data merging */
    double binSize;
    /** add encoder value flag (i.e. wether the detector is moving - 1 - or stationary - 0) */ 
    int moveFlag;


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
    ////////////////////////// only in the multi detector class?!?!?!? additional shared memory class?!?!?!?
    int actionMask;
  
    mystring actionScript[MAX_ACTIONS];

    mystring actionParameter[MAX_ACTIONS];


    int scanMode[MAX_SCAN_LEVELS];
    mystring scanScript[MAX_SCAN_LEVELS];
    mystring scanParameter[MAX_SCAN_LEVELS];
    int nScanSteps[MAX_SCAN_LEVELS];
    mysteps scanSteps[MAX_SCAN_LEVELS];
    int scanPrecision[MAX_SCAN_LEVELS];
  
    ////////////////////////////////////////////////////////////////////////////////////////////////

    
    /*offsets*/
    /** memory offsets for the flat field coefficients */
    int ffoff;
    /** memory offsets for the flat filed coefficient errors */
    int fferroff;
    /** memory offsets for the module structures  */
    int modoff;
    /** memory offsets for the dac arrays */
    int dacoff;
    /** memory offsets for the adc arrays */
    int adcoff;
    /** memory offsets for the chip register arrays */
    int chipoff;
    /** memory offsets for the channel register arrays  -trimbits*/
    int chanoff;
    /** memory offsets for the gain register arrays */
    int gainoff;
    /** memory offsets for the offset register arrays  -trimbits*/
    int offsetoff;


    /* receiver*/
    /** ip address/hostname of the receiver for the client to connect to**/
    char receiver_hostname[MAX_STR_LENGTH];
    /** is the port used to communicate between client and the receiver*/
    int receiverTCPPort;
    /** is the port used to communicate between detector and the receiver*/
    int receiverUDPPort;
    /** is the port used to communicate between second half module of Eiger detector and the receiver*/
    int receiverUDPPort2;
    /** ip address of the receiver for the detector to send packets to**/
    char receiverUDPIP[MAX_STR_LENGTH];
    /** mac address of receiver for the detector to send packets to **/
    char receiverUDPMAC[MAX_STR_LENGTH];
    /**  mac address of the detector **/
    char detectorMAC[MAX_STR_LENGTH];
    /**  ip address of the detector **/
    char detectorIP[MAX_STR_LENGTH];
    /** online flag - is set if the receiver is connected, unset if socket connection is not possible  */
    int receiverOnlineFlag;

    /** 10 Gbe enable*/
    int tenGigaEnable;

    /** flag for acquiring */
    bool acquiringFlag;

  } sharedSlsDetector;







  using slsDetectorUtils::getDetectorType;

  using postProcessing::flatFieldCorrect;
  using postProcessing::rateCorrect;
  using postProcessing::setBadChannelCorrection;

  using angularConversion::readAngularConversion;
  using angularConversion::writeAngularConversion;

  using slsDetectorUtils::getAngularConversion;


  string getDetectorType(){return sgetDetectorsType();};



  /** (default) constructor 
      \param  type is needed to define the size of the detector shared memory 9defaults to GENERIC i.e. the largest shared memory needed by any slsDetector is allocated
      \param  id is the detector index which is needed to define the shared memory id. Different physical detectors should have different IDs in order to work independently
      \param  p is the parent multislsdet to access filename ,path etc

  */

  slsDetector(detectorType type=GENERIC, int id=0, multiSlsDetector *p=NULL);

  /** constructor
      \param  id is the detector index which is needed to define the shared memory id. Different physical detectors should have different IDs in order to work independently
      \param  p is the parent multislsdet to access filename ,path etc
  */
  slsDetector(int id, multiSlsDetector *p=NULL);


  slsDetector(char *name, int id=0,  int cport=DEFAULT_PORTNO, multiSlsDetector *p=NULL);
  //slsDetector(string  const fname);
  //  ~slsDetector(){while(dataQueue.size()>0){}};
  /** destructor */ 
  virtual ~slsDetector();

  int setOnline(int const online=GET_ONLINE_FLAG);
  
  string checkOnline();

  /** returns if the detector already existed
      \returns 1 if the detector structure has already be initlialized, 0 otherwise */
  int exists() {return thisDetector->alreadyExisting;};
  
  /** returns 1 if the detetcor with id has already been allocated and initialized in shared memory
      \param detector id
      \returns 1 if the detector structure has already be initlialized, 0 otherwise */
  static int exists(int id);

  /**  
       configures mac for gotthard, moench readout
     \returns OK or FAIL
  */
  int configureMAC();

  /**
     Prints receiver configuration
     \returns OK or FAIL
  */
  int printReceiverConfiguration();

  /**
     Reads the configuration file fname
     \param fname file name
     \returns OK or FAIL
  */
  int readConfigurationFile(string const fname); 

 
  int readConfigurationFile(ifstream &infile);  


 
  /**  

  Writes the configuration file fname
  \param fname file name
  \returns OK or FAIL

  */
  int writeConfigurationFile(string const fname);
  int writeConfigurationFile(ofstream &outfile, int id=-1);








  /** 
      configure the socket communication and initializes the socket instances

      \param name hostname - if "" the current hostname is used
      \param control_port port for control commands - if -1 the current is used
      \param stop_port port for stop command - if -1 the current is used

      \returns OK is connection succeded, FAIL otherwise
      \sa sharedSlsDetector
  */
  int setTCPSocket(string const name="", int const control_port=-1, int const stop_port=-1);

  /**
     changes/gets the port number
     \param type port type
     \param num new port number (-1 gets)
     \returns actual port number
  */
  int setPort(portType type, int num=-1);
 
  /** returns the detector control port  \sa sharedSlsDetector */
  int getControlPort() {return  thisDetector->controlPort;};
  /** returns the detector stop  port  \sa sharedSlsDetector */
  int getStopPort() {return thisDetector->stopPort;};
  /** returns the receiver port  \sa sharedSlsDetector */
  int getReceiverPort() {return thisDetector->receiverTCPPort;};
 
  /** Locks/Unlocks the connection to the server
      /param lock sets (1), usets (0), gets (-1) the lock
      /returns lock status of the server
  */
  int lockServer(int lock=-1);

  /** 
      Returns the IP of the last client connecting to the detector
  */
  string getLastClientIP();

  /** returns the detector hostname \sa sharedSlsDetector  */
  string getHostname(int ipos=-1) {return string(thisDetector->hostname);};
  /** returns the detector hostname \sa sharedSlsDetector  */
  string setHostname(const char *name, int ipos=-1) {setTCPSocket(string(name)); return string(thisDetector->hostname);};
  /** connect to the control port */
  int connectControl();
  /** disconnect from the control port */
  int disconnectControl();

  /** connect to the receiver port */
  int connectData();
  /** disconnect from the receiver port */
  int disconnectData();

  /** connect to the stop port */
  int connectStop();
  /** disconnect from the stop port */
  int disconnectStop();

  /**
     sets the network parameters
     \param i network parameter type can be RECEIVER_IP, RECEIVER_MAC, SERVER_MAC
     \param s value to be set
     \returns parameter

  */
  char* setNetworkParameter(networkParameter index, string value);

  /**
     gets the network parameters
     \param i network parameter type can be RECEIVER_IP, RECEIVER_MAC, SERVER_MAC
     \returns parameter

  */
  char* getNetworkParameter(networkParameter index);

  /* I/O */

  /** returns the detector trimbit/settings directory  \sa sharedSlsDetector */
  char* getSettingsDir() {return thisDetector->settingsDir;};
  /** sets the detector trimbit/settings directory  \sa sharedSlsDetector */
  char* setSettingsDir(string s) {sprintf(thisDetector->settingsDir, s.c_str()); return thisDetector->settingsDir;};



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


  //virtual int writeSettingsFile(string fname, sls_detector_module mod); 
  
  /**
     writes a trim/settings file for module number imod - the values will be read from the current detector structure
     \param fname name of the file to be written
     \param imod module number
     \param iodelay io delay (detector specific)
     \returns OK or FAIL if the file could not be written   
     \sa ::sls_detector_module sharedSlsDetector mythenDetector::writeSettingsFile(string, int)
  */
  using energyConversion::writeSettingsFile;
  int writeSettingsFile(string fname, int imod, int* iodelay=0);


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


 
  /** loads the modules settings/trimbits reading from a file  
      \param fname file name . If not specified, extension is automatically generated!
      \param imod module number, -1 means all modules
      \returns OK or FAIL
  */ 
  int loadSettingsFile(string fname, int imod=-1);


  /** saves the modules settings/trimbits writing to  a file  
      \param fname file name . Axtension is automatically generated!
      \param imod module number, -1 means all modules
      \returns OK or FAIL
  */
  int saveSettingsFile(string fname, int imod=-1);

  /** sets all the trimbits to a particular value
      \param val trimbit value
      \param imod module number, -1 means all modules
      \returns OK or FAIL
  */
  int setAllTrimbits(int val, int imod=-1);


  /** loads the modules calibration data  reading from a file
      \param fname file name . If not specified, extension is automatically generated!
      \param imod module number, -1 means all modules
      \returns OK or FAIL
  */
  int loadCalibrationFile(string fname, int imod=-1);


  /** saves the modules calibration data  writing to  a file
      \param fname file name . Axtension is automatically generated!
      \param imod module number, -1 means all modules
      \returns OK or FAIL
  */
  int saveCalibrationFile(string fname, int imod=-1);


  /**
   
  reads an angular conversion file
  \param fname file to be read
  \sa  angleConversionConstant mythenDetector::readAngularConversion
  */
  int readAngularConversionFile(string fname="");


  /**
   
  reads an angular conversion file
  \param fname file to be read
  \sa  angleConversionConstant mythenDetector::readAngularConversion
  */
  int readAngularConversion(ifstream& ifs);



  /**
     Pure virtual function
     writes an angular conversion file
     \param fname file to be written
     \sa  angleConversionConstant mythenDetector::writeAngularConversion
  */
  int writeAngularConversion(string fname="");



  /**
     Pure virtual function
     writes an angular conversion file
     \param fname file to be written
     \sa  angleConversionConstant mythenDetector::writeAngularConversion
  */
  int writeAngularConversion(ofstream &ofs);





  /** Returns the number of channels per chip (without connecting to the detector) */
  int getNChans(){return thisDetector->nChans;}; //

  /** Returns the number of channels per chip (without connecting to the detector) in one direction */
  int getNChans(dimension d){return thisDetector->nChan[d];}; //

  /** Returns the number of chips per module (without connecting to the detector) */
  int getNChips(){return thisDetector->nChips;}; //

  /** Returns the number of chips per module (without connecting to the detector) */
  int getNChips(dimension d){return thisDetector->nChip[d];}; //

  /** Returns the number of  modules (without connecting to the detector) */
  int getNMods(){return thisDetector->nMods;}; //
  
  /** Returns the number of  modules in direction d (without connecting to the detector) */
  int getNMod(dimension d){return thisDetector->nMod[d];}; //

  int getChansPerMod(int imod=0){return thisDetector->nChans*thisDetector->nChips;};

  int getChansPerMod( dimension d,int imod=0){return thisDetector->nChan[d]*thisDetector->nChip[d];};

  /** Returns the max number of  modules in direction d (without connecting to the detector) */
  int getNMaxMod(dimension d){return thisDetector->nModMax[d];}; //

  /** Returns the number of  modules (without connecting to the detector) */
  int getMaxMods(){return thisDetector->nModsMax;}; //


  int getTotalNumberOfChannels(){return thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;};

  int getTotalNumberOfChannels(dimension d){return thisDetector->nChan[d]*thisDetector->nChip[d]*thisDetector->nMod[d];};

  int getMaxNumberOfChannels(){return thisDetector->nChans*thisDetector->nChips*thisDetector->nModsMax;};

  int getMaxNumberOfChannels(dimension d){return thisDetector->nChan[d]*thisDetector->nChip[d]*thisDetector->nModMax[d];};

  /** Returns number of rois */
  int getNRoi(){return thisDetector->nROI;};



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
     \returns returns detector type index (1 GENERIC, 2 MYTHEN, 3 PILATUS, 4 XFS, 5 GOTTHARD, 6 AGIPD, 7 MOENCH, -1 command failed)
  */
  int setDetectorType(detectorType type=GET_DETECTOR_TYPE);  

  /** 
      sets/gets detector type
      normally  the detector knows what type of detector it is
      \param type is the detector type ("Mythen", "Pilatus", "XFS", "Gotthard", Agipd", "Mönch")
      \returns returns detector type index (1 GENERIC, 2 MYTHEN, 3 PILATUS, 4 XFS, 5 GOTTHARD, 6 AGIPD, 7 MOENCH, -1 command failed)
  */
  int setDetectorType(string type);  

  /** 
      gets detector type
      normally  the detector knows what type of detector it is
      \returns returns detector type index (1 GENERIC, 2 MYTHEN, 3 PILATUS, 4 XFS, 5 GOTTHARD, 6 AGIPD, 7 MOENCH,-1 command failed)
  */
  detectorType getDetectorsType(int pos=-1);

  detectorType setDetectorsType(detectorType type=GET_DETECTOR_TYPE, int pos=-1){return getDetectorsType(pos);};

  string sgetDetectorsType(int pos=-1){return getDetectorType(getDetectorsType(pos));};

  string ssetDetectorsType(detectorType type=GET_DETECTOR_TYPE, int pos=-1){return getDetectorType(getDetectorsType(pos));};
  string ssetDetectorsType(string t, int pos=-1){return getDetectorType(getDetectorsType(pos));}

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
  int giveCalibrationPulse(double vcal, int npulses);

  // Expert Initialization functions
 

  /** 
      write  register 
      \param addr address
      \val value
      \returns current register value

  */
  int writeRegister(int addr, int val);
  

  /** 
      write  register 
      \param addr address
      \val value
      \returns current register value

  */
  int writeAdcRegister(int addr, int val);
  
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
     \param mV 0 in dac units or 1 in mV
     \param imod module number (if -1 alla modules)
     \returns current DAC value
  */
  dacs_t setDAC(dacs_t val, dacIndex index , int mV, int imod=-1);
  
  /**
     set dacs value
     \param index ADC index
     \param imod module number
     \returns current ADC value
  */
  dacs_t getADC(dacIndex index, int imod=0);
 
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
  //virtual int setModule(int reg, int imod=-1); 

  /** 
      configure chip
      \param module module to be set - must contain correct module number and also channel and chip registers
      \param gainval pointer to extra gain values
      \param offsetval pointer to extra offset values
      \param iodelay iodelay (detector specific)
      \returns current register value
      \sa ::sls_detector_module
  */
  int setModule(sls_detector_module module, int* gainval, int* offsetval,int* iodelay);
  //virtual int setModule(sls_detector_module module);

  /**
     get module
     \param imod module number
     \returns pointer to module structure (which has bee created and must then be deleted)
  */
  sls_detector_module *getModule(int imod);
  //virtual sls_detector_module *getModule(int imod);
 
  // calibration functions
  //  int setCalibration(int imod, detectorSettings isettings, double gain, double offset);
  //int getCalibration(int imod, detectorSettings isettings, double &gain, double &offset);
  

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
  detectorSettings setSettings(detectorSettings isettings, int imod=-1);
  //virtual detectorSettings setSettings(detectorSettings isettings, int imod=-1);

  /**
     gets the trimbits from shared memory *chanRegs
     \param retval is the array with the trimbits
     \param fromDetector is true if the trimbits shared memory have to be uploaded from detector
     \returns the total number of channels for the detector
     \sa ::sls_detector_module
  */
  int getChanRegs(double* retval,bool fromDetector);

  /**

  updates the shared memory receiving the data from the detector (without asking and closing the connection
  /returns OK

  */

  int updateDetectorNoWait();

  /**

  updates the shared memory receiving the data from the detector 
  /returns OK

  */


  int updateDetector();

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
     start detector acquisition and read out, but does not read data from socket  leaving socket opened
     \returns OK or FAIL
   
  */ 
  int startAndReadAllNoWait(); 

  /*   /\** */
  /*     receives a data frame from the detector socket */
  /*     \returns pointer to the data or NULL. If NULL disconnects the socket */
  /*     \sa getDataFromDetector */
  /*   *\/  */
  /*   int* getDataFromDetectorNoWait();  */

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
     asks and  receives all data  from the detector  and leaves the socket opened
     \returns OK or FAIL
  */ 
  int readAllNoWait();





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
       \param n number of rois
       \param roiLimits array of roi
       \returns success or failure
  */
   int setROI(int n=-1,ROI roiLimits[]=NULL);

   /**
    	  get roi from each detector and convert it to the multi detector scale
    	  \param n number of roi
    	  \returns an array of multidetector's rois
    */
   slsDetectorDefs::ROI* getROI(int &n);


   int sendROI(int n=-1,ROI roiLimits[]=NULL);

  /**
     set/get readout flags
     \param flag readout flag to be set
     \returns current flag
  */
   int setReadOutFlags(readOutFlags flag=GET_READOUT_FLAGS);

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
      set flat field corrections
      \param corr if !=NULL the flat field corrections will be filled with corr (NULL usets ff corrections)
      \param ecorr if !=NULL the flat field correction errors will be filled with ecorr (1 otherwise)
      \returns 0 if ff correction disabled, >0 otherwise
  */
  int setFlatFieldCorrection(double *corr, double *ecorr=NULL);


  /** 
      get flat field corrections
      \param corr if !=NULL will be filled with the correction coefficients
      \param ecorr if !=NULL will be filled with the correction coefficients errors
      \returns 0 if ff correction disabled, >0 otherwise
  */
  int getFlatFieldCorrection(double *corr=NULL, double *ecorr=NULL);


  /** 
      set rate correction
      \param t dead time in ns - if 0 disable correction, if >0 set dead time to t, if <0 set deadtime to default dead time for current settings
      \returns 0 if rate correction disabled, >0 otherwise
  */
  int setRateCorrection(double t=0);

  
  /** 
      get rate correction
      \param t reference for dead time
      \returns 0 if rate correction disabled, >0 otherwise
  */
  int getRateCorrection(double &t);

  
  /** 
      get rate correction tau
      \returns 0 if rate correction disabled, otherwise the tau used for the correction
  */
  double getRateCorrectionTau();
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
      set bad channels correction
      \param nch number of bad channels
      \param chs array of channels
      \param ff 0 if normal bad channels, 1 if ff bad channels
      \returns 0 if bad channel disabled, >0 otherwise
  */
  int setBadChannelCorrection(int nch, int *chs, int ff=0);

  /** 
      get bad channels correction
      \param bad pointer to array that if bad!=NULL will be filled with the bad channel list
      \returns 0 if bad channel disabled or no bad channels, >0 otherwise
  */
  int getBadChannelCorrection(int *bad=NULL);

 
  /** 
      pure virtual function
      get angular conversion
      \param reference to diffractometer direction
      \param angconv array that will be filled with the angular conversion constants
      \returns 0 if angular conversion disabled, >0 otherwise
      \sa mythenDetector::getAngularConversion
  */
  int getAngularConversion(int &direction,  angleConversionConstant *angconv=NULL) ;
  
  angleConversionConstant *getAngularConversionPointer(int imod=0) {return &thisDetector->angOff[imod];};



  /** 
      decode data from the detector converting them to an array of doubles, one for each channle
      \param datain data from the detector
      \returns pointer to a double array with a data per channel
  */
  double* decodeData(int *datain, double *fdata=NULL);

  
  
  
  
  /** 
      flat field correct data
      \param datain data array
      \param errin error array on data (if NULL will default to sqrt(datain)
      \param dataout array of corrected data
      \param errout error on corrected data (if not NULL)
      \returns 0
  */
  int flatFieldCorrect(double* datain, double *errin, double* dataout, double *errout);
 

  

  /** 
      rate correct data
      \param datain data array
      \param errin error array on data (if NULL will default to sqrt(datain)
      \param dataout array of corrected data
      \param errout error on corrected data (if not NULL)
      \returns 0
  */
  int rateCorrect(double* datain, double *errin, double* dataout, double *errout);

  
  /*   /\**  */
  /*       pure virtual function */
  /*   sets the arrays of the merged data to 0. NB The array should be created with size >= 360./getBinSize();  */
  /*       \param mp already merged postions */
  /*       \param mv already merged data */
  /*       \param me already merged errors (squared sum) */
  /*       \param mm multiplicity of merged arrays */
  /*       \returns OK or FAIL */
  /*       \sa mythenDetector::resetMerging */
  /*   *\/ */
  
  /*   int resetMerging(double *mp, double *mv,double *me, int *mm); */
  
  /*   /\**  */
  /*       pure virtual function */
  /*   merge dataset */
  /*       \param p1 angular positions of dataset */
  /*       \param v1 data */
  /*       \param e1 errors */
  /*       \param mp already merged postions */
  /*       \param mv already merged data */
  /*       \param me already merged errors (squared sum) */
  /*       \param mm multiplicity of merged arrays */
  /*       \sa mythenDetector::addToMerging */
  /*   *\/ */
  /*   int addToMerging(double *p1, double *v1, double *e1, double *mp, double *mv,double *me, int *mm); */

  /*   /\** pure virtual function */
  /*       calculates the "final" positions, data value and errors for the emrged data */
  /*       \param mp already merged postions */
  /*       \param mv already merged data */
  /*       \param me already merged errors (squared sum) */
  /*       \param mm multiplicity of merged arrays */
  /*       \returns FAIL or the number of non empty bins (i.e. points belonging to the pattern) */
  /*       \sa mythenDetector::finalizeMerging */
  /*   *\/ */
  /*   int finalizeMerging(double *mp, double *mv,double *me, int *mm); */

  /** 
      turns off server
  */
  int exitServer();


  /** Allocates the memory for a sls_detector_module structure and initializes it
      \returns myMod the pointer to the allocate dmemory location

  */
  sls_detector_module*  createModule(){return createModule(thisDetector->myDetectorType);};


  /** Allocates the memory for a sls_detector_module structure and initializes it
      \returns myMod the pointer to the allocate dmemory location

  */
  sls_detector_module*  createModule(detectorType myDetectorType);

  /** frees the memory for a sls_detector_module structure 
      \param myMod the pointer to the memory to be freed

  */
  
  void deleteModule(sls_detector_module *myMod);


  /** calcualtes the total number of steps of the acquisition.
      called when number of frames, number of cycles, number of positions and scan steps change
  */
  int setTotalProgress();

  /** returns the current progress in % */
  double getCurrentProgress();
  

  //  double* convertAngles(double pos);





  /**
     returns the detector type from hostname and controlport
     \param 
     \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition) 
  */
  static detectorType getDetectorType(const char *name, int cport=DEFAULT_PORTNO);
 
  /**
     returns the detector type from hostname and controlport
     \param 
     \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition) 
  */
  static detectorType getDetectorType(int id);
 

  /** 
      Returns detector id
      \returns detector id
  */

  int getDetectorId(int i=-1) {return detId;};
  /** 
      Receives a data frame from the detector socket
      \returns pointer to the data (or NULL if failed)

  */
  int* getDataFromDetector(int *retval=NULL);

  //int*


  /** returns if the detector is Master, slave or nothing 
      \param flag can be GET_MASTER, NO_MASTER, IS_MASTER, IS_SLAVE
      \returns master flag of the detector
  */
  masterFlags  setMaster(masterFlags flag);

  /**
     Loads dark image or gain image from a file and sends it to the detector
     \param index is 0 for dark image and 1 for gain image
     \param fname file name to load data from

  */
  int loadImageToDetector(imageType index,string const fname);

  /**
     Called from loadImageToDetector to send the image to detector
     \param index is 0 for dark image and 1 for gain image
     \param arg image

  */
  int sendImageToDetector(imageType index,short int imageVals[]);

  /** 
      Sets/gets the synchronization mode of the various detectors
      \param sync syncronization mode can be GET_SYNCHRONIZATION_MODE, NONE, MASTER_GATES, MASTER_TRIGGERS, SLAVE_STARTS_WHEN_MASTER_STOPS
      \returns current syncronization mode   
  */   
  synchronizationMode setSynchronization(synchronizationMode sync=GET_SYNCHRONIZATION_MODE);
  
  /**
     writes the counter memory block from the detector
     \param startACQ is 1 to start acquisition after reading counter
     \param fname file name to load data from
     \returns OK or FAIL
  */
  int writeCounterBlockFile(string const fname,int startACQ=0);


  /**
     gets counter memory block in detector
     \param startACQ is 1 to start acquisition after reading counter
     \param arg counter memory block from detector
     \returns OK or FAIL
  */
  int getCounterBlock(short int arg[],int startACQ=0);


  /**
     Resets counter in detector
     \param startACQ is 1 to start acquisition after resetting counter
     \returns OK or FAIL
  */
  int resetCounterBlock(int startACQ=0);

  /** set/get counter bit in detector
   * @param i is -1 to get, 0 to reset and any other value to set the counter bit
     /returns the counter bit in detector
   */
  int setCounterBit(int i = -1);


  int getMoveFlag(int imod){if (moveFlag) return *moveFlag; else return 1;};

  /** Frees the shared memory  -  should not be used*/
  int freeSharedMemory();





  //receiver


  /**
     calls setReceiverTCPSocket if online and sets the flag
  */
  int setReceiverOnline(int const online=GET_ONLINE_FLAG);

  /**
     Checks if the receiver is really online
  */
  string checkReceiverOnline();

  /**
     configure the socket communication and initializes the socket instances

     \param name receiver ip - if "" the current receiver hostname is used
     \param receiver_port port for receiving data - if -1 the current is used

     \returns OK is connection succeded, FAIL otherwise
     \sa sharedSlsDetector
  */
  int setReceiverTCPSocket(string const name="", int const receiver_port=-1);


  /**
     Sets up the file directory
     @param s fileDir file directory
     \returns file dir
  */
  string setFilePath(string s="");

  /**
     Sets up the file name
     @param s file name
     \returns file name
  */
  string setFileName(string s="");

  /**
     Sets up the file index
     @param i file index
     \returns file index
  */
  int setFileIndex(int i=-1);

  /**
     \returns file dir
  */
  string getFilePath(){return setFilePath();};

  /**
     \returns file name
  */
  string getFileName(){return setFileName();};

  /**
     \returns file index
  */
  int getFileIndex(){return setFileIndex();};


  /**   Starts the listening mode of receiver
        \returns OK or FAIL
  */
  int startReceiver();

  /**   Stops the listening mode of receiver
        \returns OK or FAIL
  */
  int stopReceiver();

  /** Sets the receiver to start any readout remaining in the fifo and
   * change status to transmitting.
   * The status changes to run_finished when fifo is empty
   */
  runStatus startReceiverReadout();

  /**   Sets(false) or Resets(true) the CPU bit in detector
        \returns OK or FAIL
  */
  int detectorSendToReceiver(bool set);

  /**   gets the status of the listening mode of receiver
        \returns status
  */
  runStatus getReceiverStatus();

  /**   gets the number of frames caught by receiver
        \returns number of frames caught by receiver
  */
  int getFramesCaughtByReceiver();

  /**  gets the current frame index of receiver
     \returns current frame index of receiver
  */
 int getReceiverCurrentFrameIndex();

 /**
  * resets framescaught
  * @param index frames caught by receiver
 */
 int resetFramesCaught();

 /**
  * Reads a frame from receiver
  * @param fName file name of current frame()
  * @param acquisitionIndex current acquisition index
  * @param frameIndex current frame index (for each scan)
  * @param subFrameIndex current sub frame index (for 32 bit mode for eiger)
  /returns a frame read from recever
 */
 int* readFrameFromReceiver(char* fName, int &acquisitionIndex, int &frameIndex, int &subFrameIndex);

  /** Locks/Unlocks the connection to the receiver
      /param lock sets (1), usets (0), gets (-1) the lock
      /returns lock status of the receiver
  */
  int lockReceiver(int lock=-1);

  /**
     Returns the IP of the last client connecting to the receiver
  */
  string getReceiverLastClientIP();

  /**
     updates the shared memory receiving the data from the detector (without asking and closing the connection
     /returns OK
  */
  int updateReceiverNoWait();

  /**
     updates the shared memory receiving the data from the detector
     /returns OK
  */
  int updateReceiver();

  /**
      Turns off the receiver server!
  */
  int exitReceiver();

  /**
     Sets/Gets receiver file write enable
     @param enable 1 or 0 to set/reset file write enable
     /returns file write enable
  */
  int enableWriteToFile(int enable=-1);

  /**
     Sets/Gets file overwrite enable
     @param enable 1 or 0 to set/reset file overwrite enable
     /returns file overwrite enable
  */
  int overwriteFile(int enable=-1);


  /**
   * set frame index to 0 or -1
   * @param index is the frame index
  */
  int setFrameIndex(int index=-1);



  int fillModuleMask(int *mM);


  /** Starts acquisition, calibrates pedestal and writes to fpga
     /returns number of frames
  */
  int calibratePedestal(int frames = 0);


  /** Clears error mask and also the bit in parent det multi error mask
     /returns error mask
  */
  int64_t clearAllErrorMask();


  /** returns the  detector MAC address\sa sharedSlsDetector  */
  char* getDetectorMAC() {return thisDetector->detectorMAC;};
  /** returns the  detector IP address\sa sharedSlsDetector  */
  char* getDetectorIP() {return thisDetector->detectorIP;};
  /** returns the receiver IP address \sa sharedSlsDetector  */
  char* getReceiver() {return thisDetector->receiver_hostname;};
  /** returns the receiver UDP IP address \sa sharedSlsDetector  */
  char* getReceiverUDPIP() {return thisDetector->receiverUDPIP;};
  /** returns the receiver UDP MAC address \sa sharedSlsDetector  */
  char* getReceiverUDPMAC() {return thisDetector->receiverUDPMAC;};
  /** returns the receiver UDP IP address \sa sharedSlsDetector  */
  char* getReceiverUDPPort() {char *c= new char[MAX_STR_LENGTH];sprintf(c,"%d",thisDetector->receiverUDPPort); return c;};
  /** returns the receiver UDP2 for Eiger IP address \sa sharedSlsDetector  */
  char* getReceiverUDPPort2() {char *c= new char[MAX_STR_LENGTH];sprintf(c,"%d",thisDetector->receiverUDPPort2); return c;};

  /** validates the format of detector MAC address and sets it \sa sharedSlsDetector  */
  char* setDetectorMAC(string detectorMAC);
  /** validates the format of detector IP address and sets it \sa sharedSlsDetector  */
  char* setDetectorIP(string detectorIP);
  /** validates and sets the receiver IP address/hostname \sa sharedSlsDetector  */
  char* setReceiver(string receiver);
  /** validates the format of receiver udp ip and sets it \sa sharedSlsDetector  */
  char* setReceiverUDPIP(string udpip);
  /** validates the format of receiver udp mac and sets it \sa sharedSlsDetector  */
  char* setReceiverUDPMAC(string udpmac);
  /** sets the receiver udp port \sa sharedSlsDetector  */
  int setReceiverUDPPort(int udpport);
  /** sets the receiver udp port2 for Eiger \sa sharedSlsDetector  */
  int setReceiverUDPPort2(int udpport);

  /** Sets the read receiver frequency
   	  if Receiver read upon gui request, readRxrFrequency=0,
   	   else every nth frame to be sent to gui
   	   @param getFromReceiver is 1 if it should ask the receiver,
   	   0 if it can get it from multislsdetecter
   	   @param i is the receiver read frequency
   	   /returns read receiver frequency
   */
  int setReadReceiverFrequency(int getFromReceiver, int i=-1);

  /** enable/disable or get data compression in receiver
   * @param i is -1 to get, 0 to disable and 1 to enable
     /returns data compression in receiver
   */
  int enableReceiverCompression(int i = -1);

  /** send the detector host name to the eiger receiver
   * for various handshaking required with the detector
   */
  void setDetectorHostname();

  /** enable/disable or 10Gbe
   * @param i is -1 to get, 0 to disable and 1 to enable
     /returns if 10Gbe is enabled
   */
  int enableTenGigabitEthernet(int i = -1);

  /** set/get receiver fifo depth
   * @param i is -1 to get, any other value to set the fifo deph
     /returns the receiver fifo depth
   */
  int setReceiverFifoDepth(int i = -1);

  /******** CTB funcs */

  /** opens pattern file and sends pattern to CTB 
      @param fname pattern file to open
      @returns OK/FAIL
  */
  int setCTBPattern(string fname);

  
  /** Writes a pattern word to the CTB
      @param addr address of the word, -1 is I/O control register,  -2 is clk control register
      @param word 64bit word to be written, -1 gets
      @returns actual value
  */
  uint64_t setCTBWord(int addr,uint64_t word=-1);  
  
  /** Sets the pattern or loop limits in the CTB
      @param level -1 complete pattern, 0,1,2, loop level
      @param start start address if >=0
      @param stop stop address if >=0
      @param n number of loops (if level >=0)
      @returns OK/FAIL
  */
  int setCTBPatLoops(int level,int &start, int &stop, int &n);  


  /** Sets the wait address in the CTB
      @param level  0,1,2, wait level
      @param addr wait address, -1 gets
      @returns actual value
  */
  int setCTBPatWaitAddr(int level, int addr=-1);  

   /** Sets the wait time in the CTB
      @param level  0,1,2, wait level
      @param t wait time, -1 gets
      @returns actual value
  */
  int setCTBPatWaitTime(int level, uint64_t t=-1);  

  /**
     Pulse Pixel
     \param n is number of times to pulse
     \param x is x coordinate
     \param y is y coordinate
     \returns OK or FAIL
  */
  int pulsePixel(int n=0,int x=0,int y=0);

  /**
     Pulse Pixel and move by a relative value
     \param n is number of times to pulse
     \param x is relative x value
     \param y is relative y value
     \returns OK or FAIL
  */
  int pulsePixelNMove(int n=0,int x=0,int y=0);

  /**
     Pulse Chip
     \param n is number of times to pulse
     \returns OK or FAIL
  */
  int pulseChip(int n=0);

  /**
     Set acquiring flag in shared memory
     \param b acquiring flag
   */
  void setAcquiringFlag(bool b=false);

  /**
     Get acquiring flag from shared memory
     \returns acquiring flag
   */
  bool getAcquiringFlag();

 
 protected:
 

  /**
     address of the detector structure in shared memory
  */
  sharedSlsDetector *thisDetector;

  
  /**
     detector ID
  */
  int detId;
  

  /**
   * parent multi detector
   * */

  multiSlsDetector *parentDet;

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

  
  /** pointer to flat field coefficients */
  double *ffcoefficients;
  /** pointer to flat field coefficient errors */
  double *fferrors;


  /** pointer to detector module structures */
  sls_detector_module *detectorModules;
  /** pointer to dac valuse */
  dacs_t *dacs;
  /** pointer to adc valuse */
  dacs_t *adcs;
  /** pointer to chip registers */
  int *chipregs;
  /** pointer to channal registers */
  int *chanregs;
  /** pointer to gain values */
  int *gain;
  /** pointer to offset values */
  int *offset;

  receiverInterface *thisReceiver;


  /** Initializes the shared memory 
      \param type is needed to define the size of the shared memory
      \param id is the detector id needed to define the shared memory id
      \return shm_id shared memory id
  */
  int initSharedMemory(detectorType type=GENERIC, int id=0);

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
  
  /** Gets MAC from receiver and sets up UDP Connection */
  int setUDPConnection();


};

#endif
