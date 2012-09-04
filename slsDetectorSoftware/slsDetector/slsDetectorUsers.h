#ifndef SLS_DETECTOR_USERS_H
#define SLS_DETECTOR_USERS_H



/**
 * 
 *
 *
 * @author Anna Bergamaschi
 * @version 0.1alpha
 */


#include "detectorData.h"

#include <stdint.h>
#include <string>


using namespace std;
/** 

@libdoc The slsDetectorUsers class is a minimal purely virtual interface class which should be instantiated by the users in their acquisition software (EPICS, spec etc.). More advanced configuration functions are not implemented and can be written in a configuration file tha can be read/written.


This class contains the functions accessible by the users to control the slsDetectors (both multiSlsDetector and slsDetector)

 * @short This is the base class for detector functionalities of interest for the users.

*/


class slsDetectorUsers
 { 

 public:

  /** @short default constructor */
   slsDetectorUsers(){};


   /**  @short virtual destructor */
   virtual ~slsDetectorUsers(){};



   /**
       @short useful to define subset of working functions
      \returns "PSI" or "Dectris"
   */
   virtual string getDetectorDeveloper()=0;



  /**  @short sets the onlineFlag
      \param online can be: -1 returns wether the detector is in online (1) or offline (0) state; 0 detector in offline state; 1  detector in online state
      \returns 0 (offline) or 1 (online)
  */
  virtual int setOnline(int const online=-1)=0;

  /**
      @short start measurement and acquires
    \returns OK/FAIL
  */
  virtual int startMeasurement()=0;

  /**
      @short stop measurement
    \returns OK/FAIL
  */
  virtual int stopMeasurement()=0;
 
  /**
      @short get run status
    \returns status mask
  */
  virtual int getDetectorStatus()=0;

  /**
      @short returns the default output files path
  */
  virtual string getFilePath()=0;

 /**
      @short sets the default output files path
     \param s file path
     \returns file path
  */
  virtual string setFilePath(string s)=0;  

  /** 
      @short 
      \returns the default output files root name
  */
  virtual string getFileName()=0;  

  /**
      @short sets the default output files path
     \param s file name
     \returns the default output files root name
     
  */
  virtual string setFileName(string s)=0;  
  
  /** 
      @short 
     \returns the default output file index
  */
  virtual int getFileIndex()=0;
  
  /**
          @short sets the default output file index
     \param i file index
     \returns the default output file index
  */
  virtual int setFileIndex(int i)=0;

  /** 
         @short get flat field corrections file directory
    \returns flat field correction file directory
  */
  virtual string getFlatFieldCorrectionDir()=0; 
  
  /** 
           @short set flat field corrections file directory
      \param dir flat field correction file directory
      \returns flat field correction file directory
  */
  virtual string setFlatFieldCorrectionDir(string dir)=0;
  
  /** 
           @short get flat field corrections file name
      \returns flat field correction file name
  */
  virtual string getFlatFieldCorrectionFile()=0;
  
  /** 
           @short set flat field correction file
      \param fname name of the flat field file (or "" if disable)
      \returns 0 if disable (or file could not be read), >0 otherwise
  */
  virtual int setFlatFieldCorrectionFile(string fname="")=0; 

  

  /** 
           @short enable/disable flat field corrections (without changing file name)
      \param i 0 disables, 1 enables, -1 gets
      \returns 0 if ff corrections disabled, 1 if enabled
  */
  virtual int enableFlatFieldCorrection(int i=-1)=0;

  /**
           @short enable/disable count rate corrections 
      \param i 0 disables, 1 enable, -1 gets
      \returns 0 if count corrections disabled, 1 if enabled
  */
  virtual int enableCountRateCorrection(int i=-1)=0;

  /**
           @short enable/disable bad channel corrections 
      \param i 0 disables, 1 enables, -1 gets
      \returns 0 if bad channels corrections disabled, 1 if enabled
  */
  virtual int enablePixelMaskCorrection(int i=-1)=0;

  /**
           @short enable/disable angular conversion 
      \param i 0 disables, 1 enables, -1 gets
      \returns 0 if angular conversion disabled, 1 if enabled
  */
  virtual int enableAngularConversion(int i=-1)=0;

  /**Enable write file function included*/

  virtual int enableWriteToFile(int i=-1)=0;

  /** 
           @short set  positions for the acquisition
      \param nPos number of positions
      \param pos array with the encoder positions
      \returns number of positions
  */
  virtual int setPositions(int nPos, double *pos)=0;
  
  /** 
           @short get  positions for the acquisition
      \param pos array which will contain the encoder positions
      \returns number of positions
  */
  virtual int getPositions(double *pos=NULL)=0;
  
  /**
     @short sets the detector size
     \param x0 horizontal position origin in channel number (-1 unchanged)
     \param y0 vertical position origin in channel number (-1 unchanged)
     \param nx number of channels in horiziontal  (-1 unchanged)
     \param  ny number of channels in vertical  (-1 unchanged)
     \returns OK/FAIL
  */
  virtual int setDetectorSize(int x0=-1, int y0=-1, int nx=-1, int ny=-1)=0;


  /**
     @short gets detector size
     \param x0 horizontal position origin in channel number 
     \param y0 vertical position origin in channel number 
     \param nx number of channels in horiziontal
     \param  ny number of channels in vertical 
     \returns OK/FAIL
  */
  virtual int getDetectorSize(int &x0, int &y0, int &nx, int &ny)=0;
  /**
     @short setsthe maximum detector size
     \param x0 horizontal position origin in channel number 
     \param y0 vertical position origin in channel number 
     \param nx number of channels in horiziontal
     \param  ny number of channels in vertical 
     \returns OK/FAIL
  */
  virtual int getMaximumDetectorSize(int &nx, int &ny)=0;


    /** 
	@short set/get dynamic range
      \param i dynamic range (-1 get)
      \returns current dynamic range
  */
  virtual int setBitDepth(int i=-1)=0;


 
   /**
         @short set detector settings
    \param isettings  settings index (-1 gets)
    \returns current settings
  */
  virtual int setSettings(int isettings=-1)=0;

  /**
         @short get threshold energy
    \returns current threshold value for imod in ev (-1 failed)
  */
  virtual int getThresholdEnergy()=0;  


  /**
     @short set threshold energy
    \param e_eV threshold in eV
    \returns current threshold value for imod in ev (-1 failed)
  */
  virtual int setThresholdEnergy(int e_eV)=0;

  /**
     @short get beam energy -- only for dectris!
    \returns current beam energy
  */
  virtual int getBeamEnergy()=0;  


  /**
     @short set beam energy -- only for dectris!
    \param e_eV beam in eV
    \returns current beam energyin ev (-1 failed)
  */
  virtual int setBeamEnergy(int e_eV)=0;

  /** 
           @short set/get exposure time value
      \param t time in ns  (-1 gets)
      \returns timer set value in ns
  */

  virtual int64_t setExposureTime(int64_t t=-1)=0;

  /** 
       @short set/get exposure period
      \param t time in ns   (-1 gets)
      \returns timer set value in ns
  */
  virtual int64_t setExposurePeriod(int64_t t=-1)=0;
  
  /** 
       @short set/get delay after trigger
      \param t time in ns   (-1 gets)
      \returns timer set value in ns
  */
  virtual int64_t setDelayAfterTrigger(int64_t t=-1)=0;

  /** 
       @short set/get number of gates
      \param t number of gates  (-1 gets)
      \returns number of gates
  */
  virtual int64_t setNumberOfGates(int64_t t=-1)=0; 
  
  /** 
       @short set/get number of frames i.e. number of exposure per trigger
      \param t number of frames  (-1 gets) 
      \returns number of frames
  */
  virtual int64_t setNumberOfFrames(int64_t t=-1)=0;

  /** 
       @short set/get number of cycles i.e. number of triggers
      \param t number of frames  (-1 gets) 
      \returns number of frames
  */
  virtual int64_t setNumberOfCycles(int64_t t=-1)=0;
  

 /** 
      @short set/get the external communication mode 
      \param pol value to be set \sa getTimingMode
      \returns current external communication mode
  */
  virtual int setTimingMode(int pol=-1)=0;

  /**
      @short Reads the configuration file -- will contain all the informations needed for the configuration (e.g. for a PSI detector caldir, settingsdir, angconv, badchannels, hostname etc.)
     \param fname file name
     \returns OK or FAIL
  */  
  virtual int readConfigurationFile(string const fname)=0;  


  /** 
       @short Reads the parameters from the detector and writes them to file
      \param fname file to write to
      \returns OK or FAIL
  
  */
  virtual int dumpDetectorSetup(string const fname)=0; 
  /** 
      @short Loads the detector setup from file
      \param fname file to read from
      \returns OK or FAIL
  
  */
  virtual int retrieveDetectorSetup(string const fname)=0;

  /**
      @short useful for data plotting etc.
     \returns Mythen, Eiger, Gotthard etc.
  */
  virtual string getDetectorType()=0;

  /**
     @short register calbback for accessing detector final data
     \param userCallback function for plotting/analyzing the data
  */

  virtual void registerDataCallback(int( *userCallback)(detectorData*, void*), void *pArg)=0;

  /**
     @short register calbback for accessing detector final data
     \param func function to be called at the end of the acquisition. gets detector status and progress index as arguments
  */

  virtual void registerAcquisitionFinishedCallback(int( *func)(double,int, void*), void *pArg)=0;
  
  /**
     @short register calbback for reading detector position
     \param func function for reading the detector position
  */
  
  virtual void registerGetPositionCallback( double (*func)(void*),void *arg)=0;
  /**
     @short register callback for connecting to the epics channels
     \param func function for connecting to the epics channels
  */
  virtual void registerConnectChannelsCallback( int (*func)(void*),void *arg)=0;
  /**
     @short register callback to disconnect the epics channels
     \param func function to disconnect the epics channels
  */
  virtual void registerDisconnectChannelsCallback( int (*func)(void*),void *arg)=0;  
  /**
     @short register callback for moving the detector
     \param func function for moving the detector
  */
  virtual void registerGoToPositionCallback( int (*func)(double,void*),void *arg)=0;
  /**
     @short register callback for moving the detector without waiting
     \param func function for moving the detector
  */
  virtual void registerGoToPositionNoWaitCallback( int (*func)(double,void*),void *arg)=0;
  /**
     @short register calbback reading to I0
     \param func function for reading the I0 (called with parameter 0 before the acquisition, 1 after and the return value used as I0)
  */
  virtual void registerGetI0Callback( double (*func)(int,void*),void *arg)=0;
  
  /************************************************************************

                           STATIC FUNCTIONS

  *********************************************************************/  

  /** @short returns string from run status index
      \param s run status index
      \returns string error, waiting, running, data, finished or unknown when wrong index
  */
  static string runStatusType(int s){					\
    switch (s) {							\
    case 0:     return string("idle");					\
    case 1:       return string("error");				\
    case 2:      return  string("waiting");				\
    case 3:      return string("finished");				\
    case 4:      return string("data");					\
    case 5:      return string("running");				\
    default:       return string("unknown");				\
    }};



  /** @short returns detector settings string from index
      \param s can be standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain
      \returns   setting index (-1 unknown string)
  */

  static int getDetectorSettings(string s){		\
    if (s=="standard") return 0;			\
    if (s=="fast") return 1;				\
    if (s=="highgain") return 2;			\
    if (s=="dynamicgain") return 3;			\
    if (s=="lowgain") return 4;				\
    if (s=="mediumgain") return 5;			\
    if (s=="veryhighgain") return 6;			\
    return -1;				         };

  /** @short returns detector settings string from index
      \param s settings index
      \returns standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain, undefined when wrong index
  */
  static string getDetectorSettings(int s){\
    switch(s) {						\
    case 0:      return string("standard");\
    case 1:      return string("fast");\
    case 2:      return string("highgain");\
    case 3:    return string("dynamicgain");	\
    case 4:    return string("lowgain");		\
    case 5:    return string("mediumgain");	\
    case 6:    return string("veryhighgain");			\
    default:    return string("undefined");			\
    }};



  /**
     @short returns external communication mode string from index
     \param f index for communication mode
     \returns  auto, trigger, ro_trigger, gating, triggered_gating, unknown when wrong mode
  */

  static string getTimingMode(int f){	\
    switch(f) {						 \
    case 0:      return string( "auto");			\
    case 1: return string("trigger");			\
    case 2: return string("ro_trigger");				\
    case 3: return string("gating");			\
    case 4: return string("triggered_gating");	\
    default:    return string( "unknown");				\
    }      };

  /**
     @short returns external communication mode string from index
     \param f index for communication mode
     \returns  auto, trigger, ro_trigger, gating, triggered_gating, unknown when wrong mode
  */

  static int getTimingMode(string s){					\
    if (s== "auto") return 0;						\
    if (s== "trigger") return 1;					\
    if (s== "ro_trigger") return 2;					\
    if (s== "gating") return 3;						\
    if (s== "triggered_gating") return 4;				\
    return -1;							};


 };

#endif
