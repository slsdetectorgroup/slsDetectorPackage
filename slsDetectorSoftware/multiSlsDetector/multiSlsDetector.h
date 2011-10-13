


#ifndef MULTI_SLS_DETECTOR_H
#define MULTI_SLS_DETECTOR_H

#include "slsDetector.h"

#define MAXDET 100


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

class multiSlsDetector : public slsDetector {



 public:
  

  /** 
      @short Structure allocated in shared memory to store detector settings and be accessed in parallel by several applications (take care of possible conflicts!)
      
  */


/** (default) constructor 
 \param  type is needed to define the size of the detector shared memory 9defaults to GENERIC i.e. the largest shared memory needed by any slsDetector is allocated
 \param  id is the detector index which is needed to define the shared memory id. Different physical detectors should have different IDs in order to work independently


*/
  multiSlsDetector(detectorType type=GENERIC, int ndet=1, int id=0);
  //slsDetector(string  const fname);
  /** destructor */ 
  virtual ~multiSlsDetector();
  
  


  int addSlsDetector(detectorType type=GENERIC, int id=0);
  int removeSlsDetector(int i=-1);


  int getDetectorId(int i) {if (detectors[i]) return detectors[i]->getDetectorId();};


  /** sets the onlineFlag
      \param off can be: <BR> GET_ONLINE_FLAG, returns wether the detector is in online or offline state;<BR> OFFLINE_FLAG, detector in offline state (i.e. no communication to the detector - using only local structure - no data acquisition possible!);<BR> ONLINE_FLAG  detector in online state (i.e. communication to the detector updating the local structure) */
  int setOnline(int const online=GET_ONLINE_FLAG, int i=-1);  
  /** sets the onlineFlag
      \returns 1 if the detector structure has already be initlialized with the given idand belongs to this multiDetector instance, 0 otherwise */
  int exists(int id) ;

  /**
    Purely virtual function
    Should be implemented in the specific detector class
    /sa mythenDetector::readConfigurationFile
  */

  virtual int readConfigurationFile(string const fname)=0;  
  /**  
    Purely virtual function
    Should be implemented in the specific detector class
    /sa mythenDetector::writeConfigurationFile
  */
  virtual int writeConfigurationFile(string const fname)=0;


  /* 
     It should be possible to dump all the settings of the detector (including trimbits, threshold energy, gating/triggering, acquisition time etc.
     in a file and retrieve it for repeating the measurement with identicals ettings, if necessary
  */
  /** 
    
    Purely virtual function
    Should be implemented in the specific detector class
    /sa mythenDetector::dumpDetectorSetup
  */
  virtual int dumpDetectorSetup(string const fname, int level)=0;  
  /** 
    Purely virtual function
    Should be implemented in the specific detector class
    /sa mythenDetector::retrieveDetectorSetup
  */
  virtual int retrieveDetectorSetup(string const fname, int level)=0;

  /** 
     configure the socket communication and initializes the socket instances

     \param name hostname - if "" the current hostname is used
     \param control_port port for control commands - if -1 the current is used
     \param stop_port port for stop command - if -1 the current is used
     \param data_port port for receiving data - if -1 the current is used

     \returns OK is connection succeded, FAIL otherwise
     \sa sharedSlsDetector
  */
  int setTCPSocket(int i, string const name="", int const control_port=-1, int const stop_port=-1, int const data_port=-1);

  /** returns the detector hostname \sa sharedSlsDetector  */
  char* getHostname(int i) ;
  /** returns the detector control port  \sa sharedSlsDetector */
  int getControlPort(int i=-1);
  /** returns the detector stop  port  \sa sharedSlsDetector */
  int getStopPort(int i=-1);
  /** returns the detector data port  \sa sharedSlsDetector */
  int getDataPort(int i=-1);
 
  /** generates file name without extension

      always appends to file path and file name the run index.

      in case also appends the position index 
       
      Filenames will be of the form: filepath/filename(_px)_i
      where x is the position index and i is the run index

  */
  string createFileName();














  /* I/O */
  /** returns the detector trimbit directory  \sa sharedSlsDetector */
  char* getTrimDir(int i=-1);
  /** sets the detector trimbit directory  \sa sharedSlsDetector */
  char* setTrimDir(string s, int i=-1);
  /** returns the number of trim energies and their value  \sa sharedSlsDetector 
   \param point to the array that will contain the trim energies (in ev)
  \returns number of trim energies

  unused!

  \sa  sharedSlsDetector
  */
  int getTrimEn(int *en=NULL, int i=-1);
  /** sets the number of trim energies and their value  \sa sharedSlsDetector 
   \param nen number of energies
   \param en array of energies
   \returns number of trim energies

   unused!

  \sa  sharedSlsDetector
  */
  int setTrimEn(int nen, int *en=NULL, int i=-1);

  /**
     Pure virtual function
     reads a trim file
     \param fname name of the file to be read
     \param myMod pointer to the module structure which has to be set. <BR> If it is NULL a new module structure will be created
     \returns the pointer to myMod or NULL if reading the file failed
     \sa mythenDetector::readTrimFile
  */

  virtual sls_detector_module* readTrimFile(string fname,  sls_detector_module* myMod=NULL)=0;

  /**
     Pure virtual function
     writes a trim file
     \param fname name of the file to be written
     \param mod module structure which has to be written to file
     \returns OK or FAIL if the file could not be written

     \sa ::sls_detector_module mythenDetector::writeTrimFile(string, sls_detector_module)
  */
  virtual int writeTrimFile(string fname, sls_detector_module mod)=0; 
  
  /**
     returns currently the loaded trimfile name
  */

  const char *getTrimFile(int i=-1);

  /**
     Pure virtual function
     writes a trim file for module number imod - the values will be read from the current detector structure
     \param fname name of the file to be written
     \param imod module number
     \returns OK or FAIL if the file could not be written   
     \sa ::sls_detector_module sharedSlsDetector mythenDetector::writeTrimFile(string, int)
  */
  virtual int writeTrimFile(string fname, int imod, int i)=0;

  /**
     sets the default output files path
  \sa  sharedSlsDetector
  */
  char* setFilePath(string s, int i=-1);

  /**
     sets the default output files root name
  \sa  sharedSlsDetector
  */
  char* setFileName(string s, int i=-1); 

  /**
     sets the default output file index
  \sa  sharedSlsDetector
  */
  int setFileIndex(int i, int id=-1); 
  
  /**
     returns the default output files path
  \sa  sharedSlsDetector
  */
  char* getFilePath(int id=-1);
  
  /**
     returns the default output files root name
  \sa  sharedSlsDetector
  */
  char* getFileName(int id=-1) ;

  /**
     returns the default output file index
  \sa  sharedSlsDetector
  */
  int getFileIndex(int id=-1) ;
  

  
    /**
       Pure virtual function
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
  virtual int writeDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=-1)=0; 
  
  /**
     Pure virtual function
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \returns OK or FAIL if it could not write the file or data=NULL  
       \sa mythenDetector::writeDataFile
  */
  virtual int writeDataFile(string fname, int *data)=0;
  
  /**
     Pure virtual function
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
  virtual int readDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=0)=0;  

  /**
     Pure virtual function
       reads a data file
       \param name of the file to be read
       \param data array of data values
       \returns OK or FAIL if it could not read the file or data=NULL
       \sa mythenDetector::readDataFile
  */
  virtual int readDataFile(string fname, int *data)=0;

 /**
     returns the location of the calibration files
  \sa  sharedSlsDetector
  */
  char* getCalDir(int id=-1);


   /**
      sets the location of the calibration files
  \sa  sharedSlsDetector
  */
  char* setCalDir(string s, int id=-1)
  /**
     Pure virtual function
      reads a calibration file
      \param fname file to be read
      \param gain reference to the gain variable
      \offset reference to the offset variable
  \sa  sharedSlsDetector mythenDetector::readCalibrationFile
  */
  virtual int readCalibrationFile(string fname, float &gain, float &offset)=0;
  /**
     Pure virtual function
      writes a calibration file
      \param fname file to be written
      \param gain 
      \param offset
  \sa  sharedSlsDetector mythenDetector::writeCalibrationFile
  */
  virtual int writeCalibrationFile(string fname, float gain, float offset)=0;


  /**
     Pure virtual function
      reads an angular conversion file
      \param fname file to be read
  \sa  angleConversionConstant mythenDetector::readAngularConversion
  */
  virtual int readAngularConversion(string fname="", int id=-1)=0;
  /**
     Pure virtual function
     writes an angular conversion file
      \param fname file to be written
  \sa  angleConversionConstant mythenDetector::writeAngularConversion
  */
  virtual int writeAngularConversion(string fname="", int id=-1)=0;

  /** Returns the number of channels per chip */
  int getNChans(int id=-1); //

  /** Returns the number of chips per module */
  int getNChips(int id=-1); //


  /* Communication to server */


  /**
     executes a system command on the server 
     e.g. mount an nfs disk, reboot and returns answer etc.
     \param cmd is the command to be executed
     \param answer is the answer from the detector
     \returns OK or FAIL depending on the command outcome
  */
  int execCommand(string cmd, string answer, int id=-1);
  
  /**
     sets/gets detector type
     normally  the detector knows what type of detector it is
     \param type is the detector type (defaults to GET_DETECTOR_TYPE)
     \returns returns detector type index (1 GENERIC, 2 MYTHEN, 3 PILATUS, 4 XFS, 5 GOTTHARD, 6 AGIPD, -1 command failed)
  */
  int setDetectorType(detectorType type=GET_DETECTOR_TYPE, int id=-1);  

  /** 
     sets/gets detector type
     normally  the detector knows what type of detector it is
     \param type is the detector type ("Mythen", "Pilatus", "XFS", "Gotthard", Agipd")
     \returns returns detector type index (1 GENERIC, 2 MYTHEN, 3 PILATUS, 4 XFS, 5 GOTTHARD, 6 AGIPD, -1 command failed)
  */
  int setDetectorType(string type, int id=-1);  

  /** 
     gets detector type
     normally  the detector knows what type of detector it is
     \param type is the string where the detector type will be written ("Mythen", "Pilatus", "XFS", "Gotthard", Agipd")
  */
  void getDetectorType(char *type, int id=-1);


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
  int setNumberOfModules(int n=GET_FLAG, dimension d=X, int id=-1); // if n=GET_FLAG returns the number of installed modules

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
  int getMaxNumberOfModules(dimension d=X, int id=-1); //
 
 
  /** 
     set/get the use of an external signal 
      \param pol meaning of the signal \sa externalSignalFlag
      \param signalIndex index of the signal
      \returns current meaning of signal signalIndex
  */
  externalSignalFlag setExternalSignalFlags(externalSignalFlag pol=GET_EXTERNAL_SIGNAL_FLAG , int signalindex=0, int id=-1);

 
  /** 
     set/get the external communication mode
     
     obsolete \sa setExternalSignalFlags
      \param pol value to be set \sa externalCommunicationMode
      \returns current external communication mode
  */
  externalCommunicationMode setExternalCommunicationMode(externalCommunicationMode pol=GET_EXTERNAL_COMMUNICATION_MODE, int id=-1);


  // Tests and identification
 
  /**
     get detector ids/versions for module
     \param mode which id/version has to be read
     \param imod module number for module serial number
     \returns id
  */
  int64_t getId(idMode mode, int imod=0, int id=0);
  int64_t getId(idMode mode, int imod=0);
  /**
    Digital test of the modules
    \param mode test mode
    \param imod module number for chip test or module firmware test
    \returns OK or error mask
  */
  int digitalTest(digitalTestMode mode, int imod=0);
  int digitalTest(digitalTestMode mode, int imod=0, int id=0);
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

    in this function trimbits and calibration files are searched in the trimDir and calDir directories and the detector is initialized
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
  virtual runStatus  getRunStatus()=0;

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
  virtual int setAngularConversion(string fname="")=0;

  /** 
      pure virtual function
      get angular conversion
      \param reference to diffractometer direction
      \param angconv array that will be filled with the angular conversion constants
      \returns 0 if angular conversion disabled, >0 otherwise
      \sa mythenDetector::getAngularConversion
  */
  virtual int getAngularConversion(int &direction,  angleConversionConstant *angconv=NULL)=0;
  
  
  /**
      pure virtual function
      returns the angular conversion file
      \sa mythenDetector::getAngularConversion */
  virtual string getAngularConversion()=0;
  
  /** 
      pure virtual function
      set detector global offset
      \sa mythenDetector::setGlobalOffset
  */
  virtual float setGlobalOffset(float f)=0; 

  /** 
      pure virtual function
      set detector fine offset
      \sa mythenDetector::setFineOffset
  */
  virtual float setFineOffset(float f)=0;
  /** 
      pure virtual function
      get detector fine offset
      \sa mythenDetector::getFineOffset
  */
  virtual float getFineOffset()=0;
  
  /** 
      pure virtual function
      get detector global offset
      \sa mythenDetector::getGlobalOffset
  */
  virtual float getGlobalOffset()=0;

  /** 
      pure virtual function
      set  positions for the acquisition
      \param nPos number of positions
      \param pos array with the encoder positions
      \returns number of positions
      \sa mythenDetector::setPositions
  */
  virtual int setPositions(int nPos, float *pos)=0;
   /** 
      pure virtual function
      get  positions for the acquisition
      \param pos array which will contain the encoder positions
      \returns number of positions
      \sa mythenDetector::getPositions
  */
  virtual int getPositions(float *pos=NULL)=0;
  
  
  /** pure virtual function
      set detector bin size used for merging (approx angular resolution)
      \param bs bin size in degrees
      \returns current bin size
      \sa mythenDetector::setBinSize
*/
  virtual float setBinSize(float bs)=0;

  /** pure virtual function
      return detector bin size used for merging (approx angular resolution)
      \sa mythenDetector::getBinSize
  */
  virtual float getBinSize()=0;





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
  virtual int resetMerging(float *mp, float *mv,float *me, int *mm)=0;
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
  virtual int addToMerging(float *p1, float *v1, float *e1, float *mp, float *mv,float *me, int *mm)=0;

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
  virtual void* processData(int delflag=1)=0; // thread function
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
  
  virtual void acquire(int delflag=1)=0;

  /** calcualtes the total number of steps of the acquisition.
      called when number of frames, number of cycles, number of positions and scan steps change
  */
  int setTotalProgress();

  /** returns the current progress in % */
  float getCurrentProgress();
  

 protected:
 



  int nDetectors;


  slsDetector *detectors[MAXDET];


};


#endif
