/** 
    @internal

   function indexes to call on the server
   All set functions with argument -1 work as get, when possible 

*/
#ifndef SLS_DETECTOR_FUNCS_H
#define SLS_DETECTOR_FUNCS_H

enum {

  // General purpose functions
  F_EXEC_COMMAND=0, /**< command is executed */
  F_GET_ERROR,  /**< return detector error status */

  // configuration  functions
  F_GET_DETECTOR_TYPE, /**< return detector type */
  F_SET_NUMBER_OF_MODULES, /**< set/get number of installed modules */
  F_GET_MAX_NUMBER_OF_MODULES, /**< get maximum number of installed modules */
  F_SET_EXTERNAL_SIGNAL_FLAG,  /**< set/get flag for external signal */
  F_SET_EXTERNAL_COMMUNICATION_MODE, /**< set/get  external communication mode (obsolete) */


  // Tests and identification

  F_GET_ID, /**< get detector id of version */
  F_DIGITAL_TEST,  /**< digital test of the detector */
  F_ANALOG_TEST,  /**<analog test of the detector */
  F_ENABLE_ANALOG_OUT,  /**<enable the analog output */
  F_CALIBRATION_PULSE,  /**<pulse the calibration input */

  // Initialization functions
  F_SET_DAC,   /**< set DAC value */
  F_GET_ADC,  /**< get ADC value */
  F_WRITE_REGISTER,  /**< write to register */
  F_READ_REGISTER, /**< read register */
  F_WRITE_MEMORY,  /**< write to memory */
  F_READ_MEMORY, /**< read memory */


  F_SET_CHANNEL,  /**< initialize channel */
  F_GET_CHANNEL,  /**< get channel register */
  F_SET_ALL_CHANNELS,  /**< initialize all channels */

  F_SET_CHIP,  /**< initialize chip */
  F_GET_CHIP,  /**< get chip status */
  F_SET_ALL_CHIPS,  /**< initialize all chips */

  F_SET_MODULE, /**< initialize module */
  F_GET_MODULE,  /**< get module status */
  F_SET_ALL_MODULES,  /**< initialize all modules */


  F_SET_SETTINGS,  /**< set detector settings */
  F_GET_THRESHOLD_ENERGY,  /**< get detector threshold (in eV) */
  F_SET_THRESHOLD_ENERGY,  /**< set detector threshold (in eV) */


  // Acquisition functions
  F_START_ACQUISITION, /**< start acquisition */
  F_STOP_ACQUISITION, /**< stop acquisition */
  F_START_READOUT, /**< start readout */
  F_GET_RUN_STATUS,  /**< get acquisition status */
  F_START_AND_READ_ALL,  /**< start acquisition and read all frames*/
  F_READ_FRAME,  /**< read one frame */
  F_READ_ALL,  /**< read alla frames */
  
  //Acquisition setup functions
  F_SET_TIMER,  /**< set/get timer value */
  F_GET_TIME_LEFT,  /**< get current value of the timer (time left) */



  F_SET_DYNAMIC_RANGE,  /**< set/get detector dynamic range */
  F_SET_READOUT_FLAGS,  /**< set/get readout flags */
  F_SET_ROI,  /**< set/get region of interest */

  F_SET_SPEED,  /**< set/get readout speed parameters */

  //Trimming
  F_EXECUTE_TRIMMING,   /**< execute trimming */

  F_EXIT_SERVER,  /**< turn off detector server */

  F_LOCK_SERVER, /**< Locks/Unlocks server communication to the given client */ 

  F_GET_LAST_CLIENT_IP,  /**< returns the IP of the client last connected to the detector */ 
  
  F_SET_PORT, /**< Changes communication port of the server */

  F_UPDATE_CLIENT, /**< Returns all the important parameters to update the shared memory of the client */

  F_CONFIGURE_MAC, /**< Configures MAC for Gotthard readout */
  
  F_LOAD_IMAGE,   /**< Loads Dark/Gain image to the Gotthard detector */

  // multi detector structures

  F_SET_MASTER, /**< sets master/slave flag for multi detector structures */

  F_SET_SYNCHRONIZATION_MODE, /**< sets master/slave synchronization mode for multidetector structures */

  F_READ_COUNTER_BLOCK, /**< reads the counter block memory for gotthard */

  F_RESET_COUNTER_BLOCK, /**< resets the counter block memory for gotthard */


  //receiver

  F_SET_FILE_PATH, 			/**< sets receiver file directory */

  F_SET_FILE_NAME, 			/**< sets receiver file name */

  F_SET_FILE_INDEX, 		/**< sets receiver file index */

  F_START_RECEIVER,			/**< starts the receiver listening mode */

  F_STOP_RECEIVER,			/**< stops the receiver listening mode */

  F_GET_RECEIVER_STATUS,	/**< gets the status of receiver listening mode */

  F_GET_FRAMES_CAUGHT,		/**< gets the number of frames caught by receiver */

  F_LOCK_RECEIVER,			/**< locks receiver */

  F_GET_FRAME_INDEX,		/**< gets the frame index */

  F_RESET_FRAMES_CAUGHT     /**< resets the frames caught */

  /* Always append functions hereafter!!! */

};

#endif
/** @endinternal */
