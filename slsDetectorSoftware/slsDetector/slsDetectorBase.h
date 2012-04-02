
#ifndef SLS_DETECTOR_BASE_H
#define SLS_DETECTOR_BASE_H


#include "sls_detector_defs.h"

#include <string>


using namespace std;
/** 

This class contains the functions accessible by the users to control the slsDetectors (both multiSlsDetector and slsDetector)

*/


class slsDetectorBase : public slsDetectorDefs
 { 

 public:

  /** default constructor */
   slsDetectorBase(){};


   /** virtual destructor */
   virtual ~slsDetectorBase(){};

  /** sets the onlineFlag
      \param online can be: <BR> GET_ONLINE_FLAG, returns wether the detector is in online or offline state;<BR> OFFLINE_FLAG, detector in offline state (i.e. no communication to the detector - using only local structure - no data acquisition possible!);<BR> ONLINE_FLAG  detector in online state (i.e. communication to the detector updating the local structure) 
      \returns ONLINE_FLAG or OFFLINE_FLAG
  */
  virtual int setOnline(int const online=GET_ONLINE_FLAG)=0;

 

  /** performs a complete acquisition including scansand data processing 
     moves the detector to next position <br>
     starts and reads the detector <br>
     reads the IC (if required) <br>
     reads the encoder (iof required for angualr conversion) <br>
     processes the data (flat field, rate, angular conversion and merging ::processData())
      \param delflag 0 leaves the data in the final data queue
      \returns nothing
  */
  virtual void acquire(int delflag)=0;

  /**
   asks and  receives a data frame from the detector, writes it to disk and processes the data
    \returns pointer to the data or NULL (unused!!!). 
  */ 
  virtual int* readFrame()=0;

/*   /\** processes the data */
/*       \param delflag 0 leaves the data in the final data queue */
/*       \returns nothing */
      
/*   *\/ */
/*   virtual void* processData(int delflag)=0; */

  /**
    start detector acquisition
    \returns OK/FAIL
  */
  virtual int startAcquisition()=0;
  /**
    stop detector acquisition
    \returns OK/FAIL
  */
  virtual int stopAcquisition()=0;

  /**
     get run status
    \returns status mask
  */
  virtual runStatus getRunStatus()=0;

/*   /\** Frees the shared memory  -  should not be used except for debugging*\/ */
/*   virtual int freeSharedMemory()=0; */

/*   /\** adds the detector with ID id in postion pos */
/*    \param id of the detector to be added (should already exist!) */
/*    \param pos position where it should be added (normally at the end of the list (default to -1) */
/*    \returns the actual number of detectors or -1 if it failed (always for slsDetector) */
/*   *\/ */
/*   virtual int addSlsDetector(int id, int pos=-1){return -1;}; */


/*   /\** adds the detector name in position pos */
/*    \param name of the detector to be added (should already exist in shared memory or at least be online)  */
/*    \param pos position where it should be added (normally at the end of the list (default to -1) */
/*    \return the actual number of detectors or -1 if it failed (always for slsDetector) */
/*   *\/ */
/*   virtual int addSlsDetector(char* name, int pos=-1){return -1;}; */


/*   /\** */
/*      removes the detector in position pos from the multidetector */
/*      \param pos position of the detector to be removed from the multidetector system (defaults to -1 i.e. last detector) */
/*      \returns the actual number of detectors or -1 if it failed (always for slsDetector) */
/*   *\/ */
/*   virtual int removeSlsDetector(int pos=-1){return -1;}; */

/*  /\**removes the detector in position pos from the multidetector */
/*     \param name is the name of the detector */
/*     \returns the actual number of detectors or -1 if it failed  (always for slsDetector) */
/*  *\/ */
/*   virtual int removeSlsDetector(char* name){return -1;}; */

  
  /**
     returns the default output files path
  */
  virtual string getFilePath()=0;

 /**
     sets the default output files path
     \param s file path
     \returns file path
  */
  virtual string setFilePath(string s)=0;  

  /**
     returns the default output files root name
  */
  virtual string getFileName()=0;  

  /**
     sets the default output files path
  */
  virtual string setFileName(string s)=0;  
  
  /**
     returns the default output file index
  */
  virtual int getFileIndex()=0;
  
  /**
     sets the default output file index
  */
  virtual int setFileIndex(int i)=0;

  /** 
    get flat field corrections file directory
    \returns flat field correction file directory
  */
  virtual string getFlatFieldCorrectionDir()=0; 
  
  /** 
      set flat field corrections file directory
      \param dir flat field correction file directory
      \returns flat field correction file directory
  */
  virtual string setFlatFieldCorrectionDir(string dir)=0;
  
  /** 
      get flat field corrections file name
      \returns flat field correction file name
  */
  virtual string getFlatFieldCorrectionFile()=0;
  
  /** 
      set flat field corrections
      \param fname name of the flat field file (or "" if disable)
      \returns 0 if disable (or file could not be read), >0 otherwise
  */
  virtual int setFlatFieldCorrection(string fname="")=0; 
  
  /** 
      set rate correction
      \param t dead time in ns - if 0 disable correction, if >0 set dead time to t, if <0 set deadtime to default dead time for current settings
      \returns 0 if rate correction disabled, >0 otherwise
  */
  virtual int setRateCorrection(float t=0)=0;

  /** 
      get rate correction
      \param t reference for dead time
      \returns 0 if rate correction disabled, >0 otherwise
  */
  virtual int getRateCorrection(float &t)=0;
  
  /** 
      get rate correction
      \returns 0 if rate correction disabled, >0 otherwise
  */
  virtual int getRateCorrection()=0;

/*   /\**  */
/*       returns the bad channel list file  */
/*   *\/ */
/*   virtual string getBadChannelCorrectionFile()=0; */
  



  virtual int enableBadChannelCorrection(int i=-1)=0;

  virtual int enableAngularConversion(int i=-1)=0;


/*   /\**  */
/*       set angular conversion */
/*       \param fname file with angular conversion constants ("" disable) */
/*       \returns 0 if angular conversion disabled, >0 otherwise */
/*   *\/ */
/*   virtual int setAngularConversionFile(string fname="")=0; */
  

/*   /\** */
/*      pure virtual function */
/*       returns the angular conversion file  */
/*   *\/ */
/*   virtual string getAngularConversionFile()=0; */


  /** 
      set  positions for the acquisition
      \param nPos number of positions
      \param pos array with the encoder positions
      \returns number of positions
  */
  virtual int setPositions(int nPos, float *pos)=0;
  
  /** 
      get  positions for the acquisition
      \param pos array which will contain the encoder positions
      \returns number of positions
  */
  virtual int getPositions(float *pos=NULL)=0;
  
/*  /\**  */
/*       set action  */
/*       \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS} */
/*       \param fname for script ("" disable) */
/*       \returns 0 if action disabled, >0 otherwise */
/*   *\/ */
/*   virtual int setActionScript(int iaction, string fname="")=0; */

/*   /\**  */
/*       set action  */
/*       \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS} */
/*       \param par for script ("" disable) */
/*       \returns 0 if action disabled, >0 otherwise */
/*   *\/ */
/*   virtual int setActionParameter(int iaction, string par="")=0; */

/*   /\**  */
/*       returns action script */
/*       \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript} */
/*       \returns action script */
/*   *\/ */
/*   virtual string getActionScript(int iaction)=0; */

/*   /\**  */
/* 	returns action parameter */
/* 	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript} */
/* 	\returns action parameter */
/*     *\/ */
/*   virtual string getActionParameter(int iaction)=0; */

/*  /\**  */
/*       set scan script */
/*       \param index is the scan index (0 or 1) */
/*       \param script fname for script ("" disable, "none" disables and overwrites current, "threshold" makes threshold scan, "trimbits" make trimbits scan, "energy" makes energy scan) */
/*       \returns 0 if scan disabled, >0 otherwise */
/*   *\/ */
/*   virtual int setScanScript(int index, string script="")=0; */

/*  /\**  */
/*       set scan script parameter */
/*       \param index is the scan index (0 or 1) */
/*       \param spar parameter to be passed to the scan script with syntax par=spar */
/*       \returns 0 if scan disabled, >0 otherwise */
/*   *\/ */
/*   virtual int setScanParameter(int index, string spar="")=0; */

/*  /\**  */
/*       set scan precision */
/*       \param index is the scan index (0 or 1) */
/*       \param precision number of decimals to use for the scan variable in the file name */
/*       \returns 0 if scan disabled, >0 otherwise */
/*   *\/ */
/*   virtual int setScanPrecision(int index, int precision=-1)=0;  */

/*   /\**  */
/*       set scan steps (passed to the scan script as var=step) */
/*       \param index is the scan index (0 or 1) */
/*       \param nvalues is the number of steps */
/*       \param values array of steps */
/*       \returns 0 if scan disabled, >0 otherwise */
/*   *\/ */
/*   virtual int setScanSteps(int index, int nvalues=-1, float *values=NULL)=0; */

/*    /\**  */
/*       get scan script */
/*       \param index is the scan index (0 or 1) */
/*       \returns "none" if disables, "threshold" threshold scan, "trimbits" trimbits scan, "energy"  energy scan or scan script name */
/*   *\/ */
/*   virtual string getScanScript(int index)=0; */

/*      /\**  */
/*       get scan script */
/*       \param index is the scan index (0 or 1) */
/*       \returns scan script parameter */
/*   *\/ */
/*   virtual string getScanParameter(int index)=0; */

/*    /\**  */
/*       get scan precision */
/*       \param index is the scan index (0 or 1) */
/*       \returns precision i.e. number of decimals to use for the scan variable in the file name */
/*   *\/ */
/*   virtual int getScanPrecision(int index)=0; */

/*      /\**  */
/*       get scan steps */
/*       \param index is the scan index (0 or 1) */
/*       \param values pointer to array of values (must be allocated in advance) */
/*       \returns number of steps */
/*   *\/ */
/*   virtual int getScanSteps(int index, float *values=NULL)=0;  */

  /** Locks/Unlocks the connection to the server
      /param lock sets (1), usets (0), gets (-1) the lock
      /returns lock status of the server
  */
  virtual int lockServer(int i=-1)=0;  



  /** 
      set/get the size of the detector 
      \param i number of modules
      \param d dimension
      \returns current number of modules in direction d
  */
  virtual int setNumberOfModules(int i=-1, dimension d=X)=0;



    /** 
      set/get dynamic range
      \param i dynamic range (-1 get)
      \returns current dynamic range
  */
  virtual int setDynamicRange(int i=-1)=0;

  /**
    get detector settings
    \param imod module number (-1 all)
    \returns current settings
  */
  virtual detectorSettings getSettings(int imod=-1)=0; 

   /**
    set detector settings
    \param isettings  settings
    \param imod module number (-1 all)
    \returns current settings

    in this function trimbits/settings and calibration files are searched in the settingsDir and calDir directories and the detector is initialized
  */
  virtual detectorSettings setSettings(detectorSettings isettings, int imod=-1)=0;

  /**
    get threshold energy
    \param imod module number (-1 all)
    \returns current threshold value for imod in ev (-1 failed)
  */
  virtual int getThresholdEnergy(int imod=-1)=0;  


  /**
    set threshold energy
    \param e_eV threshold in eV
    \param imod module number (-1 all)
    \param isettings ev. change settings
    \returns current threshold value for imod in ev (-1 failed)
  */
  virtual int setThresholdEnergy(int e_eV, int imod=-1, detectorSettings isettings=GET_SETTINGS)=0;




  /** 
      set/get timer value
      \param index timer index
      \param t time in ns or number of...(e.g. frames, gates, probes)
      \returns timer set value in ns or number of...(e.g. frames, gates, probes)
  */
  virtual int64_t setTimer(timerIndex index, int64_t t=-1)=0;



 /** 
     set/get the external communication mode
      \param pol value to be set \sa externalCommunicationMode
      \returns current external communication mode
  */
  virtual externalCommunicationMode setExternalCommunicationMode(externalCommunicationMode pol=GET_EXTERNAL_COMMUNICATION_MODE)=0;


  /**
     Reads the configuration file -- will contain all the informations needed for the configuration (e.g. for a PSI detector caldir, settingsdir, angconv, badchannels etc.)
     \param fname file name
     \returns OK or FAIL
  */  
  virtual int readConfigurationFile(string const fname)=0;  


/*   /\**   */
/*      Writes the configuration file -- will contain all the informations needed for the configuration (e.g. for a PSI detector caldir, settingsdir, angconv, badchannels etc.) */
/*      \param fname file name */
/*      \returns OK or FAIL */
/*   *\/ */
/*   virtual int writeConfigurationFile(string const fname)=0; */

  /** 
      Reads the parameters from the detector and writes them to file
      \param fname file to write to
      \param level if 2 reads also trimbits, flat field, angular correction etc. and writes them to files with automatically added extension
      \returns OK or FAIL
  
  */
  virtual int dumpDetectorSetup(string const fname, int level=0)=0; 
  /** 
     Loads the detector setup from file
      \param fname file to read from
      \param level if 2 reads also reads trimbits, angular conversion coefficients etc. from files with default extensions as generated by dumpDetectorSetup
      \returns OK or FAIL
  
  */
  virtual int retrieveDetectorSetup(string const fname, int level=0)=0;


/*  /\** */
/*       Loads dark image or gain image to the detector */
/*       \param index can be DARK_IMAGE or GAIN_IMAGE */
/*       \param fname file name to load data from */
/*       \returns OK or FAIL */
/*  *\/ */
/*   virtual int loadImageToDetector(imageType index,string const fname)=0; */

  
  /************************************************************************

                           STATIC FUNCTIONS

  *********************************************************************/  

  /** returns string from run status index
      \param s can be ERROR, WAITING, RUNNING, TRANSMITTING, RUN_FINISHED
      \returns string error, waiting, running, data, finished
  */
  static string runStatusType(runStatus s){\
    switch (s) {				\
    case ERROR:       return string("error");		\
    case  WAITING:      return  string("waiting");	\
    case RUNNING:      return string("running");\
    case TRANSMITTING:      return string("data");	\
    case  RUN_FINISHED:      return string("finished");	\
    default:       return string("idle");		\
    }};
  
  /** returns detector type string from detector type index
      \param t string can be Mythen, Pilatus, Eiger, Gotthard, Agipd, Unknown
      \returns MYTHEN, PILATUS, EIGER, GOTTHARD, AGIPD, GENERIC
  */
  static string getDetectorType(detectorType t){\
    switch (t) {\
    case MYTHEN:    return string("Mythen");	\
    case PILATUS:    return string("Pilatus");	\
    case EIGER:    return string("Eiger");	\
    case GOTTHARD:    return string("Gotthard");	\
    case AGIPD:    return string("Agipd");		\
    default:    return string("Unknown");		\
  }};

  /** returns detector type index from detector type string
      \param type can be MYTHEN, PILATUS, EIGER, GOTTHARD, AGIPD, GENERIC
      \returns Mythen, Pilatus, Eiger, Gotthard, Agipd, Unknown
  */
  static detectorType getDetectorType(string const type){\
    if (type=="Mythen")      return MYTHEN;\
    if  (type=="Pilatus")      return PILATUS;	\
    if  (type=="Eiger")    return EIGER;		\
    if  (type=="Gotthard")    return GOTTHARD;	\
    if  (type=="Agipd")    return AGIPD;		\
    return GENERIC;};



  /** returns synchronization type index from string
      \param type can be none, gating, trigger, complementary
      \returns ONE, MASTER_GATES, MASTER_TRIGGERS, SLAVE_STARTS_WHEN_MASTER_STOPS
  */
  static synchronizationMode getSyncType(string const type){\
    if (type=="none")    return NO_SYNCHRONIZATION;\
    if (type=="gating")    return MASTER_GATES;\
    if (type=="trigger")    return MASTER_TRIGGERS;		\
    if (type=="complementary") return SLAVE_STARTS_WHEN_MASTER_STOPS; \
    return GET_SYNCHRONIZATION_MODE;				\
  };

  /** returns synchronization type string from index
      \param s can be NONE, MASTER_GATES, MASTER_TRIGGERS, SLAVE_STARTS_WHEN_MASTER_STOPS
      \returns none, gating, trigger, complementary, unknown
  */
  static string getSyncType(synchronizationMode s ){\
    switch(s) {					    \
    case NO_SYNCHRONIZATION:    return string("none");	    \
    case MASTER_GATES:    return string("gating");	\
    case MASTER_TRIGGERS:    return string("trigger");			\
    case SLAVE_STARTS_WHEN_MASTER_STOPS:    return string("complementary"); \
    default:    return string("unknown");				\
    }};



  /** returns string from external signal type index
      \param f can be SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW, TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE, RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH, GATE_OUT_ACTIVE_LOW, =TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE, RO_TRIGGER_OUT_FALLING_EDGE, OUTPUT_LOW, OUTPUT_HIGH, MASTER_SLAVE_SYNCHRONIZATION,  GET_EXTERNAL_SIGNAL_FLAG
      \returns string  off, gate_in_active_high, gate_in_active_low, trigger_in_rising_edge, trigger_in_falling_edge, ro_trigger_in_rising_edge, ro_trigger_in_falling_edge, gate_out_active_high, gate_out_active_low, trigger_out_rising_edge, trigger_out_falling_edge, ro_trigger_out_rising_edge, ro_trigger_out_falling_edge, gnd, vcc, sync, unknown
  */
  static string externalSignalType(externalSignalFlag f){\
    switch(f) {						 \
    case SIGNAL_OFF:      return string( "off");			\
    case GATE_IN_ACTIVE_HIGH:    return string( "gate_in_active_high");	\
    case GATE_IN_ACTIVE_LOW:    return string( "gate_in_active_low");	\
    case TRIGGER_IN_RISING_EDGE:    return string( "trigger_in_rising_edge"); \
    case TRIGGER_IN_FALLING_EDGE:    return string( "trigger_in_falling_edge");	\
    case RO_TRIGGER_IN_RISING_EDGE:    return string( "ro_trigger_in_rising_edge"); \
    case RO_TRIGGER_IN_FALLING_EDGE:    return string( "ro_trigger_in_falling_edge"); \
    case GATE_OUT_ACTIVE_HIGH:    return string( "gate_out_active_high"); \
    case GATE_OUT_ACTIVE_LOW:    return string( "gate_out_active_low");	\
    case TRIGGER_OUT_RISING_EDGE:    return string( "trigger_out_rising_edge");	\
    case TRIGGER_OUT_FALLING_EDGE:    return string( "trigger_out_falling_edge"); \
    case RO_TRIGGER_OUT_RISING_EDGE:      return string( "ro_trigger_out_rising_edge");	\
    case RO_TRIGGER_OUT_FALLING_EDGE:    return string( "ro_trigger_out_falling_edge");	\
    case MASTER_SLAVE_SYNCHRONIZATION: return string("sync");		\
    case OUTPUT_LOW: return string("gnd");		\
    case OUTPUT_HIGH: return string("vcc");		\
    default:    return string( "unknown");				\
    }    };
  



  /** returns external signal type index from string
      \param sval  off, gate_in_active_high, gate_in_active_low, trigger_in_rising_edge, trigger_in_falling_edge, ro_trigger_in_rising_edge, ro_trigger_in_falling_edge, gate_out_active_high, gate_out_active_low, trigger_out_rising_edge, trigger_out_falling_edge, ro_trigger_out_rising_edge, ro_trigger_out_falling_edge, gnd, vcc, sync, unknown
      \returns can be SIGNAL_OFF, GATE_IN_ACTIVE_HIGH, GATE_IN_ACTIVE_LOW, TRIGGER_IN_RISING_EDGE, TRIGGER_IN_FALLING_EDGE, RO_TRIGGER_IN_RISING_EDGE, RO_TRIGGER_IN_FALLING_EDGE, GATE_OUT_ACTIVE_HIGH, GATE_OUT_ACTIVE_LOW, TRIGGER_OUT_RISING_EDGE, TRIGGER_OUT_FALLING_EDGE, RO_TRIGGER_OUT_RISING_EDGE, RO_TRIGGER_OUT_FALLING_EDGE, OUTPUT_LOW, OUTPUT_HIGH, MASTER_SLAVE_SYNCHRONIZATION,  GET_EXTERNAL_SIGNAL_FLAG (if unknown)
  */

  static externalSignalFlag externalSignalType(string sval){\
  if (sval=="off")      return SIGNAL_OFF;\
  if (sval=="gate_in_active_high")      return GATE_IN_ACTIVE_HIGH;	\
  if  (sval=="gate_in_active_low") return GATE_IN_ACTIVE_LOW;\
  if  (sval=="trigger_in_rising_edge")  return TRIGGER_IN_RISING_EDGE;\
  if  (sval=="trigger_in_falling_edge") return TRIGGER_IN_FALLING_EDGE;\
  if  (sval=="ro_trigger_in_rising_edge") return RO_TRIGGER_IN_RISING_EDGE;\
  if  (sval=="ro_trigger_in_falling_edge") return RO_TRIGGER_IN_FALLING_EDGE;\
  if (sval=="gate_out_active_high")      return GATE_OUT_ACTIVE_HIGH;\
  if  (sval=="gate_out_active_low") return GATE_OUT_ACTIVE_LOW;\
  if  (sval=="trigger_out_rising_edge") return TRIGGER_OUT_RISING_EDGE;\
  if  (sval=="trigger_out_falling_edge") return TRIGGER_OUT_FALLING_EDGE;\
  if  (sval=="ro_trigger_out_rising_edge") return RO_TRIGGER_OUT_RISING_EDGE;\
  if  (sval=="ro_trigger_out_falling_edge") return RO_TRIGGER_OUT_FALLING_EDGE;\
  if  (sval=="sync") return MASTER_SLAVE_SYNCHRONIZATION;\
  if  (sval=="gnd") return OUTPUT_LOW;\
  if  (sval=="vcc") return OUTPUT_HIGH;\
  return GET_EXTERNAL_SIGNAL_FLAG ;};

  /** returns detector settings string from index
      \param s can be standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain, undefined
      \returns   STANDARD, FAST, HIGHGAIN, DYNAMICGAIN, LOWGAIN, MEDIUMGAIN, VERYHIGHGAIN, GET_SETTINGS
  */

  static detectorSettings getDetectorSettings(string s){\
    if (s=="standard") return STANDARD;\
    if (s=="fast") return FAST;\
    if (s=="highgain") return HIGHGAIN;		\
    if (s=="dynamicgain") return DYNAMICGAIN;	\
    if (s=="lowgain") return LOWGAIN;		\
    if (s=="mediumgain") return MEDIUMGAIN;	\
    if (s=="veryhighgain") return VERYHIGHGAIN;	\
    return GET_SETTINGS;\
  };

  /** returns detector settings string from index
      \param s can be STANDARD, FAST, HIGHGAIN, DYNAMICGAIN, LOWGAIN, MEDIUMGAIN, VERYHIGHGAIN, GET_SETTINGS
      \returns standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain, undefined
  */
  static string getDetectorSettings(detectorSettings s){\
    switch(s) {\
    case STANDARD:      return string("standard");\
    case FAST:      return string("fast");\
    case HIGHGAIN:      return string("highgain");\
    case DYNAMICGAIN:    return string("dynamicgain");	\
    case LOWGAIN:    return string("lowgain");		\
    case MEDIUMGAIN:    return string("mediumgain");	\
    case VERYHIGHGAIN:    return string("veryhighgain");	\
    default:    return string("undefined");			\
    }};


  /**
     returns external communication mode string from index
     \param f can be AUTO_TIMING, TRIGGER_EXPOSURE, TRIGGER_READOUT, GATE_FIX_NUMBER, GATE_WITH_START_TRIGGER, GET_EXTERNAL_COMMUNICATION_MODE
     \returns  auto, trigger, ro_trigger, gating, triggered_gating, unknown
  */

  static string externalCommunicationType(externalCommunicationMode f){	\
    switch(f) {						 \
    case AUTO_TIMING:      return string( "auto");			\
    case TRIGGER_EXPOSURE: return string("trigger");			\
    case TRIGGER_READOUT: return string("ro_trigger");			\
    case GATE_FIX_NUMBER: return string("gating");			\
    case GATE_WITH_START_TRIGGER: return string("triggered_gating");	\
    default:    return string( "unknown");				\
    }    };
  



  /**
     returns external communication mode index from string
     \param sval can be auto, trigger, ro_trigger, gating, triggered_gating
     \returns AUTO_TIMING, TRIGGER_EXPOSURE, TRIGGER_READOUT, GATE_FIX_NUMBER, GATE_WITH_START_TRIGGER, GET_EXTERNAL_COMMUNICATION_MODE
  */

  static externalCommunicationMode externalCommunicationType(string sval){\
  if (sval=="auto")      return AUTO_TIMING;\
  if (sval=="trigger")     return TRIGGER_EXPOSURE;	\
  if  (sval=="ro_trigger") return TRIGGER_READOUT;\
  if  (sval=="gating") return GATE_FIX_NUMBER;\
  if  (sval=="triggered_gating") return GATE_WITH_START_TRIGGER;\
  return GET_EXTERNAL_COMMUNICATION_MODE;};

};
#endif
