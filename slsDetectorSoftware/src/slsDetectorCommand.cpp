#include "slsDetectorCommand.h"
#include "multiSlsDetector.h"
#include "string_utils.h"

#include <cstdlib>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

#include <iomanip>


/*! \page CLI Command line interface


This program is intended to control the SLS detectors via command line interface.
This is the only way to access all possible functionality of the detectors, however it is often recommendable to avoid changing the most advanced settings, rather leaving the task to configuration files, as when using the GUI or the API provided.

The command line interface consists in four main functions:

- \b sls_detector_acquire to acquire data from the detector
- \b sls_detector_put to set detector parameters
- \b sls_detector_get to retrieve detector parameters
- \b sls_detector_help to get help concerning the text commands
Additionally the program slsReceiver should be started on the machine expected to receive the data from the detector.


If you need control a single detector, the use of the command line interface does not need any additional arguments.

For commands addressing a single controller of your detector, the command  cmd should be called with the index i of the controller:


<b>sls_detector_clnt i:cmd</b>


where \b sls_detector_clnt is the text client (put, get, acquire, help).

In case more than one detector is configured on the control PC, the command  cmd should be called with their respective index  j:


<b>sls_detector_clnt j-cmd</b>


where \b sls_detector_clnt is the text client (put, get, acquire, help).

To address a specific controller i of detector j use:

<b>sls_detector_clnt j-i:cmd</b>


To use different shared memory segements for different detectors on the same
client pc, one can use environment variable <b>SLSDETNAME</b> set to any string to
different strings to make the shared memory segments unique. One can then use
the same multi detector id for both detectors as they have a different shared memory names.

For additional questions concerning the indexing of the detector, please refer to the SLS Detectors FAQ documentation.

The commands are sudivided into different pages depending on their functionalities:
 - \subpage acquisition "Acquisition": commands to start/stop the acquisition and retrieve data
 - \subpage config "Configuration": commands to configure the detector
 - \subpage timing "Timing": commands to configure the detector timing
 - \subpage data "Data postprocessing": commands to process the data
 - \subpage settings "Settings": commands to define detector settings/threshold.
 - \subpage output "Output": commands to define output file destination and format
 - \subpage network "Network": commands to setup the network between client, detector and receiver
 - \subpage receiver "Receiver": commands to configure the receiver
 - \subpage prototype "Chip Test Board / Moench": commands specific for the chiptest board or moench
 - \subpage test "Developer": commands to be used only for software debugging. Avoid using them!
 
 */

slsDetectorCommand::slsDetectorCommand(multiSlsDetector *det) {

    myDet = det;

    int i = 0;

    cmd = std::string("none");

    /*! \page test Developer
    Commands to be used only for software debugging. Avoid using them!
    - \b test returns an error
	 */

    descrToFuncMap[i].m_pFuncName = "test";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdUnderDevelopment;
    ++i;

    /*! \page test
   - <b>help</b> Returns a list of possible commands.
	 */
    descrToFuncMap[i].m_pFuncName = "help";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdHelp;
    ++i;

    /*! \page test
   - <b>exitserver</b> Shuts down all the detector servers. Don't use it!!!!
	 */
    descrToFuncMap[i].m_pFuncName = "exitserver";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdExitServer;
    ++i;

    /*! \page test
   - <b>rx_exit</b> Shuts down all the receivers. Don't use it!!!!
	 */
    descrToFuncMap[i].m_pFuncName = "rx_exit";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdExitServer;
    ++i;

    /*! \page test
   - <b>execcommand</b> Executes a command on the detector server. Don't use it!!!!
	 */
    descrToFuncMap[i].m_pFuncName = "execcommand";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdExitServer;
    ++i;

    /*! \page test
   - <b>rx_execcommand</b> Executes a command on the receiver server. Don't use it!!!!
	 */
    descrToFuncMap[i].m_pFuncName = "rx_execcommand";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdExitServer;
    ++i;

    /* digital test and debugging */

    /*! \page test
   - <b>bustest</b> performs test of the bus interface between FPGA and embedded Linux system. Can last up to a few minutes. Cannot set! Jungfrau only. Only get!
	 */
    descrToFuncMap[i].m_pFuncName = "bustest";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDigiTest;
    ++i;

    /*! \page test
   - <b>firmwaretest</b> performs the firmware test.  Cannot set! Jungfrau only. Only get!
	 */
    descrToFuncMap[i].m_pFuncName = "firmwaretest";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDigiTest;
    ++i;

    /*! \page test
   - <b>reg [addr] [val]</b> ??? writes to an register \c addr with \c value in hexadecimal format.
	 */
    descrToFuncMap[i].m_pFuncName = "reg";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdRegister;
    ++i;

    /*! \page test
   - <b>adcreg [addr] [val]</b> ??? writes to an adc register \c addr with \c value in hexadecimal format. Only put!
	 */
    descrToFuncMap[i].m_pFuncName = "adcreg";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdRegister;
    ++i;

    /*! \page test
   - <b>setbit</b> ???  Only put!
	 */
    descrToFuncMap[i].m_pFuncName = "setbit";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdRegister;
    ++i;

    /*! \page test
   - <b>clearbit </b> ??? Only put!
	 */
    descrToFuncMap[i].m_pFuncName = "clearbit";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdRegister;
    ++i;

    /*! \page test
   - <b>getbit </b> ??? Only get!
	 */
    descrToFuncMap[i].m_pFuncName = "getbit";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdRegister;
    ++i;

    /* Acquisition and status commands */
    /*! \page acquisition Acquition commands
   Commands to control the acquisition
	 */
    /*! \page acquisition
   - \b acquire blocking acquisition (like calling sls_detector_acquire). Starts receiver and detector, writes and processes the data, stops detector. Only get!
     \c Returns (string)\c "acquire failed" if fails, else \c "Acquired (int)", where int is number of frames caught.
	 */
    descrToFuncMap[i].m_pFuncName = "acquire";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAcquire;
    ++i;

    /*! \page acquisition
   - \b data gets all data from the detector (if any) processes them and writes them to file according to the preferences already setup (Eigerr store in ram only). Only get!
	 */
    descrToFuncMap[i].m_pFuncName = "data";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdData;
    ++i;

    /*! \page config Configuration commands
    Commands to configure the detector. these commands are often left to the configuration file.
	 - \ref configstructure "Data Structure": commands to configure detector data structure
	 - \ref configstatus "Status": commands to configure detector status
	 - \ref configsize "Data Size": commands to configure detector data size
	 - \ref configflags "Flags": commands to configure detector flags
	 - \ref configchip "Chip": commands to configure chip of the detector
	 - \ref configversions "Versions": commands to check version of each subsytem
	 - \ref configspeed "Speed": commands to configure speed of detector
	 - \ref configsettings "Detector Parameters": commands to configure/retrieve configuration of detector
	 */
    /*! \page timing Timing commands
	  Commands to setup the timing
	 */
    /* Detector structure configuration and debugging commands */
    /*! \page config
		\section configstructure Data Structure
   commands to configure detector data structure
	 */

    /*! \page config
   - \b free Free shared memory on the control PC
	 */
    descrToFuncMap[i].m_pFuncName = "free";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdFree;
    ++i;

    /*! \page config
   - <b>virtual [n] [p]</b> \c connects to n virtual detector servers at local host starting at port p \c Returns the list of the hostnames of the multi-detector structure. \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "virtual";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdHostname;
    ++i;


    /*! \page config
   - <b>user</b> \c Returns user details from shared memory. Only allowed at multi detector level.  Cannot put. \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "user";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdUser;
    ++i;

    /*! \page config
		\section configstatus Status
   commands to configure detector status
	 */


    /* detector and data size */
    /*! \page config
		\section configsize Data Size
   commands to configure detector data size
	 */


    /* flags */
    /*! \page config
		\section configflags Flags
   commands to configure detector flags
	 */

    /* fpga */

    /*! \page config
   - <b>copydetectorserver [sname] [phost]</b> copies the detector server sname via tftp from pc with hostname phost and changes respawn server for all detector. Not for Eiger.  Only put! \c Returns \c ("successful", "failed")
	 */
    descrToFuncMap[i].m_pFuncName = "copydetectorserver";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /*! \page config
   - <b>rebootdetpc </b> reboot detector controller blackfin. Only put! Not for Eiger. \c Returns \c ("successful", "failed")
	 */
    descrToFuncMap[i].m_pFuncName = "rebootcontroller";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /*! \page config
   - <b>update [sname] [phost] [file] </b> updates the firmware to file and detector server to sname from phost via tftp and then reboots controller (blackfin). Only put! Not for Eiger. \c Returns \c ("successful", "failed")
	 */
    descrToFuncMap[i].m_pFuncName = "update";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /* chip */
    /*! \page config
		\section configchip Chip
   commands to configure chip of the detector
	 */

    /* versions/ serial numbers  getId */
    /*! \page config
		\section configversions Versions
   Commands to check versions of each subsystem
	 */

    /*! \page config
   - <b>checkdetversion</b> Checks the version compatibility with detector server (if hostname is in shared memory). Only get! Only for Eiger, Jungfrau & Gotthard. \c Returns \c ("compatible", "incompatible")
	 */
    descrToFuncMap[i].m_pFuncName = "checkdetversion";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

    /*! \page config
   - <b>rx_checkversion</b> Checks the version compatibility with receiver server (if rx_hostname is in shared memory). Only get! Only for Eiger, Jungfrau & Gotthard. \c Returns \c ("compatible", "incompatible")
	 */
    descrToFuncMap[i].m_pFuncName = "rx_checkversion";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

  
    /* r/w timers */


    /* read only timers */

   /*! \page timing
   - <b>now</b> Getting actual time of the detector from start. For Jungfrau only. Only get!
	 */
    descrToFuncMap[i].m_pFuncName = "now";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page timing
   - <b>timestamp</b> Getting timestamp. For Jungfrau only. Only get!
	 */
    descrToFuncMap[i].m_pFuncName = "timestamp";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page timing
   - <b>nframes</b> Frames from start run control. Only Jungfrau. Only get! \c Returns \c (long long int)
	 */
    descrToFuncMap[i].m_pFuncName = "nframes";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;


    /* speed */
    /*! \page config
		\section configspeed Speed
   commands to configure speed of detector
	 */


    /* settings dump/retrieve */
    /*! \page config
		\section configsettings Detector Parameters
   commands to configure/retrieve configuration of detector
	 */

    /* settings dump/retrieve */

    descrToFuncMap[i].m_pFuncName = "config";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdConfiguration;
    ++i;

    /* data processing commands */

    /*! \page data Data processing commands
   Commands to setup the data processing
	 */


     	/*! \page data
    //    - <b>threaded [i]</b> Sets/gets the data processing threaded flag. 1 is threaded, 0 unthreaded.
    // 	 */
    // 	descrToFuncMap[i].m_pFuncName="threaded";
    // 	descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdThreaded;
    // 	++i;


    /*! \page settings Detector settings commands
   Commands to setup the settings of the detector
    - \ref settingsdir "Settings, trim & cal Directories": commands to setup settings/trim/cal directories
    - \ref settingssett "Settings and Threshold": commands to configure settings and threshold of detector
    - \ref settingsdacs "DACs": commands to configure DACs of detector
    - \ref settingsadcs "ADCs": commands to readout ADCs of detector
    - \ref settingstmp  "Temp Control": commands to monitor and handle temperature overshoot (only JUNGFRAU)
	 */

    /* trim/cal directories */
    /*! \page settings
		\section settingsdir Settings, trim & cal Directories
   commands to setup settings/trim/cal directories
	 */


    /* settings, threshold */
    /*! \page settings
		\section settingssett Settings and Threshold
   commands to configure settings and threshold of detector
	 */


    /*! \page settings
   - <b>trimbits [fname] </b> loads/stores the trimbits to/from the detector. If no extension is specified, the serial number of each module will be attached. \c Returns \c (string) fname
	 */
    descrToFuncMap[i].m_pFuncName = "trimbits";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSettings;
    ++i;

    /*! \page settings
   - <b>trimval [i]</b> sets all trimbits to i. Used in EIGER only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "trimval";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSettings;
    ++i;

    /* pots */
    /*! \page settings
		\section settingsdacs DACs
   commands to configure DACs of detector
	 */

    /*! \page settings
   - <b>vthreshold [i] [mv]</b> Sets/gets detector threshold voltage for single photon counters. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vthreshold";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcalibration [i] [mv]</b> Sets/gets the voltage of the calibration pulses. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcalibration";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vtrimbit [i] [mv]</b> Sets/gets the voltage to set the width of the trimbits. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vtrimbit";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vpreamp [i] [mv]</b> Sets/gets the voltage to define the preamplifier feedback resistance. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vpreamp";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vshaper1 [i] [mv]</b> Sets/gets the voltage to define the  feedback resistance of the first shaper. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vshaper1";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vshaper [i] [mv]</b> Sets/gets the voltage to define the feedback transistor voltage of the shaper. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vshaper";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vshaper2 [i] [mv]</b> Sets/gets the voltage to define the  feedback resistance of the second shaper. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vshaper2";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vhaperneg [i] [mv]</b> Sets/gets the voltage to define the  feedback transistor voltage of the negative-polarity shaper. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vshaperneg";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vapower [i]</b> Sets/gets the analog power supply for the old chiptest board in DAC units. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vapower";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vddpower [i]</b> Sets/gets the digital power supply for the old chiptest board in DAC units. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vddpower";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vshpower [i]</b> Sets/gets the comparator power supply for the old chiptest board in DAC units. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vshpower";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>viopower [i]</b> Sets/gets the power supply of the FPGA I/Os for the old chiptest board in DAC units. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "viopower";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vrefds [i] [mv]</b> Sets/gets vrefds. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vref_ds";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcascn_pb [i] [mv]</b> Sets/gets vcascn_pb. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcascn_pb";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcasc_pb [i] [mv]</b> Sets/gets vcasc_pb. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcascp_pb";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vout_cm [i] [mv]</b> Sets/gets vout_cm. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vout_cm";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcasc_out [i] [mv]</b> Sets/gets vcasc_out. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcasc_out";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vin_com [i] [mv]</b> Sets/gets vin_com. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vin_cm";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vref_comp [i] [mv]</b> Sets/gets vref_comp. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vref_comp";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>ib_test_c [i] [mv]</b> Sets/gets ib_test_c. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "ib_test_c";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vsvp [i] [mv]</b> Sets/gets vsvp. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vsvp";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vsvn [i] [mv]</b> Sets/gets vsvn. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vsvn";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vtr [i] [mv]</b> Sets/gets vtr. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vtr";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vrf [i] [mv]</b> Sets/gets vrf. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vrf";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vrs [i] [mv]</b> Sets/gets vrs. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vrs";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vtgstv [i] [mv]</b> Sets/gets vtgstv. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vtgstv";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcmp_ll [i] [mv]</b> Sets/gets vcmp_ll. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcmp_ll";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcmp_lr [i] [mv]</b> Sets/gets vcmp_lr. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcmp_lr";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcal_l [i] [mv]</b> Sets/gets vcal_l. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcall";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcomp_rl [i] [mv]</b> Sets/gets vcomp_rl. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcmp_rl";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcomp_rr [i] [mv]</b> Sets/gets vcomp_rr. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcmp_rr";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>rxb_rb [i] [mv]</b> Sets/gets rxb_rb. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "rxb_rb";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>rxb_lb [i] [mv]</b> Sets/gets rxb_lb. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "rxb_lb";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcp [i] [mv]</b> Sets/gets vcp. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcp";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcn [i] [mv]</b> Sets/gets vcn. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcn";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vis [i] [mv]</b> Sets/gets vis. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vis";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>iodelay [i] [mv]</b> Sets/gets iodelay. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "iodelay";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>dac:j [i] [mv]</b> Sets/gets value for DAC number j for the new chiptestboard. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "dac";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vipre</b> Sets/gets dac for the preamplifier's input transistor current for Mythen3. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vipre";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vdcsh</b> Sets/gets dac for the reference (DC) voltage for the shaper for Mythen3. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vdcsh";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vth1</b> Sets/gets first detector threshold voltage for Mythen 3. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vth1";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vth2</b> Sets/gets second detector threshold voltage for Mythen 3. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vth2";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vth3</b> Sets/gets third detector threshold voltage for Mythen 3. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vth3";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vpl</b> Sets/gets dac for the low voltage for analog pulsing for Mythen3. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vpl"; 
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vph</b> Sets/gets dac for the high voltage for analog pulsing for Mythen3. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vph";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vtrim</b> Sets/gets dac for the voltage defining the trim bit size for Mythen3. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vtrim";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>viinsh</b> Sets/gets dac for the bias current for the shaper for Mythen3. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "viinsh";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>cas</b> Sets/gets dac for the preamplifier's cascode voltage for Mythen3. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "cas";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>cassh</b> Sets/gets dac for the shaper's cascode voltage for Mythen3. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "cassh";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vicin</b> Sets/gets dac for the bias current for the comparator for Mythen3. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vicin";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vipreout</b> Sets/gets dac for the preamplifier's output branch current for Mythen3. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vipreout";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;
	
    /*! \page settings
   - <b>vref_h_adc</b> Sets/gets dac for reference voltage high of ADC for Gotthard2. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vref_h_adc";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vb_comp_fe</b> Sets/gets dac for comparator current of analogue front end for Gotthard2. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vb_comp_fe";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;    

    /*! \page settings
   - <b>vb_comp_adc</b> Sets/gets dac for comparator current of ADC for Gotthard2. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vb_comp_adc";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcom_cds</b> Sets/gets dac for common mode voltage of CDS stage for Gotthard2. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcom_cds";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i; 

    /*! \page settings
   - <b>vref_restore</b> Sets/gets dac for reference charging voltage of temparory storage cell in high gain for Gotthard2. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vref_restore";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vb_opa_1st</b> Sets/gets dac for opa current for driving the other DACs in chip for Gotthard2. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vb_opa_1st";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;    

    /*! \page settings
   - <b>vref_comp_fe</b> Sets/gets dac for reference voltage of the comparator of analogue front end for Gotthard2. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vref_comp_fe";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcom_adc1</b> Sets/gets dac for common mode voltage of ADC DAC bank 1 for Gotthard2. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcom_adc1";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;     

    /*! \page settings
   - <b>vref_prech</b> Sets/gets dac for reference votlage for precharing the preamplifier for Gotthard2. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vref_prech";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vref_l_adc</b> Sets/gets dac for reference voltage low for ADC for Gotthard2. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vref_l_adc";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;    

    /*! \page settings
   - <b>vref_cds</b> Sets/gets dac for reference voltage of CDS applied to the temporary storage cell in medium and low gain for Gotthard2. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vref_cds";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vb_cs</b> Sets/gets dac for current injection into preamplifier for Gotthard2. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vb_cs";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i; 

    /*! \page settings
   - <b>vb_opa_fd</b> Sets/gets dac for current for CDS opa stage for Gotthard2. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vb_opa_fd";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcom_adc2</b> Sets/gets dac for common mode voltage of ADC DAC bank 2 for Gotthard2. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcom_adc2";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;    



    /* r/w timers */
    /*! \page settings
		\section settingsadcs ADCs
   commands to readout ADCs of detector
	 */

    /* file name */

    /*! \page output Output settings
   Commands to setup the file destination and format
	 */


    /* communication configuration */

    /*! \page network
   - <b>port [port]</b> sets/gets the port of the client-detector control server TCP interface. Use single-detector command. Default value is 1952 for all detectors. Normally not changed. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "port";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPort;
    ++i;

    /*! \page network
   - <b>stopport [port]</b> sets/gets the port of the client-detector stop server TCP interface. Use single-detector command. Default value is 1953 for all detectors. Normally not changed. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "stopport";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPort;
    ++i;


    /* receiver functions */

    /*! \page receiver Receiver commands
   Commands to configure the receiver.
	 */


    /*! \page receiver
   - <b>resetframescaught [i]</b> resets the number of frames caught to 0. i can be any number. Use this if using status start, instead of acquire (this command is included). Only put! \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "resetframescaught";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page receiver
   - <b>frameindex [i]</b> gets the current frame index of receiver. Average of all for multi-detector command. Only get! \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "frameindex";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;


    /* pattern generator */


    numberOfCommands = i;

    // #ifdef VERBOSE
    //   std::cout << "Number of commands is " << numberOfCommands << std::endl;
    // #endif
}

//-----------------------------------------------------------

/*!
 */

//-----------------------------------------------------------

std::string slsDetectorCommand::executeLine(int narg, const char * const args[], int action, int detPos) {
    if (action == READOUT_ACTION)
        return cmdAcquire(narg, args, action, detPos);

    size_t s = std::string(args[0]).find(':');
    std::string key = std::string(args[0]).substr(0, s); // truncate at :

    if (action == PUT_ACTION && narg < 1)
        action = HELP_ACTION;

    for (int i = 0; i < numberOfCommands; ++i) {

        /* this works only if the command completely matches the key */
        /* otherwise one could try if truncated key is unique */

        if (key == descrToFuncMap[i].m_pFuncName) {
#ifdef VERBOSE
            std::cout << i << " command=" << descrToFuncMap[i].m_pFuncName << " key=" << key << std::endl;
#endif
            cmd = descrToFuncMap[i].m_pFuncName;

            MemFuncGetter memFunc = descrToFuncMap[i].m_pFuncPtr;
            std::string dResult = (this->*memFunc)(narg, args, action, detPos);

            return dResult;
        }
    }
    return cmdUnknown(narg, args, action, detPos);
}

std::string slsDetectorCommand::cmdUnknown(int narg, const char * const args[], int action, int detPos) {
    return std::string("Unknown command, use list to list all commands ");
}
std::string slsDetectorCommand::cmdUnderDevelopment(int narg, const char * const args[], int action, int detPos) {
    return std::string("Must still develop ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
}

std::vector<std::string> slsDetectorCommand::getAllCommands(){
    std::vector<std::string> commands;
    for (int i = 0; i!= numberOfCommands; ++i)
        commands.emplace_back(descrToFuncMap[i].m_pFuncName);
    return commands;
}

std::string slsDetectorCommand::helpLine(int narg, const char * const args[], int action, int detPos) {

    std::ostringstream os;

    if (action == READOUT_ACTION) {
        return helpAcquire(HELP_ACTION);
    }

    if (narg == 0) {
        os << "Command can be: " << std::endl;
        for (int i = 0; i < numberOfCommands; ++i) {
            os << descrToFuncMap[i].m_pFuncName << "\n";
        }
        os << std::endl;
        return os.str();
    }
    return executeLine(narg, args, HELP_ACTION, detPos);
}

std::string slsDetectorCommand::cmdAcquire(int narg, const char * const args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif

    if (action == HELP_ACTION) {
        return helpAcquire(HELP_ACTION);
    }
    if (!myDet->size()) {
        FILE_LOG(logERROR) << "This shared memory has no detectors added. Aborting.";
        return std::string("acquire failed");
    }
    if (detPos >= 0) {
        FILE_LOG(logERROR) << "Individual detectors not allowed for readout. Aborting.";
        return std::string("acquire failed");
    }

    if (myDet->acquire() == FAIL)
        return std::string("acquire failed");
    if (myDet->getUseReceiverFlag(detPos)) {
        char answer[100];
        sprintf(answer, "\nAcquired %d", myDet->getFramesCaughtByReceiver(detPos));
        return std::string(answer);
    }

    return std::string();
}

std::string slsDetectorCommand::helpAcquire(int action) {

    if (action == PUT_ACTION)
        return std::string("");
    std::ostringstream os;
    os << "Usage is " << std::endl
       << "sls_detector_acquire  id " << std::endl;
    os << "where id is the id of the detector " << std::endl;
    os << "the detector will be started, the data acquired, processed and written to file according to the preferences configured " << std::endl;
    return os.str();
}

std::string slsDetectorCommand::cmdData(int narg, const char * const args[], int action, int detPos) {

#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif
    //int b;
    if (action == PUT_ACTION) {
        return std::string("cannot set");
    } else if (action == HELP_ACTION) {
        return helpData(HELP_ACTION);
    } else {
        // b=myDet->setThreadedProcessing(-1);
        // myDet->setThreadedProcessing(0);
        // myDet->readAll(detPos);
        // //processdata in receiver is useful only for gui purposes
        // if(myDet->getUseReceiverFlag(detPos)==OFFLINE_FLAG)
        // 	myDet->processData();
        // myDet->setThreadedProcessing(b);
        return std::string("");
    }
}

std::string slsDetectorCommand::helpData(int action) {

    if (action == PUT_ACTION)
        return std::string("");
    else
        return std::string("data \t gets all data from the detector (if any) processes them and writes them to file according to the preferences already setup\n");
}


std::string slsDetectorCommand::cmdFree(int narg, const char * const args[], int action, int detPos) {

#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif
    if (action == HELP_ACTION) {
        return helpFree(HELP_ACTION);
    }

    return ("Error: Should have been freed before creating constructor\n");
}

std::string slsDetectorCommand::helpFree(int action) {
    return std::string("free \t frees the shared memory\n");
}

std::string slsDetectorCommand::cmdHostname(int narg, const char * const args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif

    if (action == HELP_ACTION) {
        return helpHostname(HELP_ACTION);
    }

    if (action == PUT_ACTION) {
        if (detPos >= 0) {
            return std::string("Wrong usage - setting hostname/virtual only from "
                               "multiDetector level");
        }

        if (cmd == "virtual") {
            int port = -1;
            int numDetectors = 0;
            if (!sscanf(args[1], "%d", &numDetectors)) {
                throw sls::RuntimeError("Cannot scan number of detector servers from virtual command\n");
            }
            if (!sscanf(args[2], "%d", &port)) {
                throw sls::RuntimeError("Cannot scan port from virtual command\n");
            }
            myDet->setVirtualDetectorServers(numDetectors, port);
        } else {
            throw sls::RuntimeError("unknown command\n");
        }
    }
    return myDet->getHostname(detPos);
}

std::string slsDetectorCommand::helpHostname(int action) {
    std::ostringstream os;

    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << std::string("virtual [n] [p]\t connects to n virtual detector servers at local host starting at port p \n");
    }
    return os.str();
}

std::string slsDetectorCommand::cmdUser(int narg, const char * const args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif

    if (action == HELP_ACTION) {
        return helpHostname(HELP_ACTION);
    }
    if (action == PUT_ACTION) {
        return std::string("cannot put");
    }
    if (detPos >= 0) {
        return std::string("Wrong usage - getting user details only from "
                           "multiDetector level");
    }
    return myDet->getUserDetails();
}

std::string slsDetectorCommand::helpUser(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << std::string("user \t returns user details from shared memory without updating shared memory. "
                          "Only allowed at multi detector level.\n");
    }
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << std::string("user \t cannot put\n");
    }
    return os.str();
}

std::string slsDetectorCommand::cmdHelp(int narg, const char * const args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif

    std::cout << narg << std::endl;

    if (narg >= 1)
        return helpLine(narg - 1, args, action, detPos);
    else
        return helpLine(0, args, action, detPos);
}

std::string slsDetectorCommand::cmdExitServer(int narg, const char * const args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif
    if (action == HELP_ACTION) {
        return helpExitServer(action);
    }

    if (action == PUT_ACTION) {
        if (cmd == "exitserver") {
            myDet->exitServer(detPos);
            return std::string("Server shut down.");
        } else if (cmd == "rx_exit") {

            myDet->exitReceiver(detPos);
            return std::string("Receiver shut down\n");
        } else if (cmd == "execcommand") {
            myDet->execCommand(std::string(args[1]), detPos);
            return std::string("Command executed successfully\n");
        } else if (cmd == "rx_execcommand") {

            myDet->execReceiverCommand(std::string(args[1]), detPos);
            return std::string("Command executed successfully\n");
         } else
            return ("cannot decode command\n");
    } else
        return ("cannot get");
}

std::string slsDetectorCommand::helpExitServer(int action) {
    std::ostringstream os;
    os << std::string("exitserver \t shuts down all the detector servers. Don't use it!!!!\n");
    os << std::string("rx_exit \t shuts down all the receiver servers.\n");
    os << std::string("execcommand \t executes command in detector server. Don't use it if you do not know what you are doing.\n");
    os << std::string("rx_execcommand \t executes command in receiver server. Don't use it if you do not know what you are doing.\n");
    return os.str();
}




// std::string slsDetectorCommand::cmdThreaded(int narg, const char * const args[], int action, int detPos){
// 	int ival;
// 	char answer[1000];

// 	if (action==HELP_ACTION)
// 		return helpThreaded(action);

// 	if (action==PUT_ACTION) {
// 		if (sscanf(args[1],"%d",&ival))
// 			myDet->setThreadedProcessing(ival);
// 	}
// 	sprintf(answer,"%d",myDet->setThreadedProcessing());
// 	return std::string(answer);

// }

std::string slsDetectorCommand::helpThreaded(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("threaded \t  returns wether the data processing is threaded. \n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("threaded t \t  sets the threading flag ( 1sets, 0 unsets).\n");

    return os.str();
}



std::string slsDetectorCommand::cmdPort(int narg, const char * const args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpPort(action);
    int val; //ret,
    char ans[MAX_STR_LENGTH];
    if (action == PUT_ACTION) {
        if (sscanf(args[1], "%d", &val))
            ;
        else
            return std::string("could not scan port number") + std::string(args[1]);
    }

    if (cmd == "port") {
        if (action == PUT_ACTION)
            myDet->setControlPort(val, detPos);
        sprintf(ans, "%d", myDet->setControlPort(-1, detPos));
    } else if (cmd == "stopport") {
        if (action == PUT_ACTION)
            myDet->setStopPort(val, detPos);
        sprintf(ans, "%d", myDet->setStopPort(-1, detPos));
    } else
        return std::string("unknown port type ") + cmd;

    return std::string(ans);
}

std::string slsDetectorCommand::helpPort(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "port i \n sets the communication control port" << std::endl;
        os << "stopport i \n sets the communication stop port " << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "port  \n gets the communication control port" << std::endl;
        os << "stopport \n gets the communication stop port " << std::endl;
    }
    return os.str();
}



std::string slsDetectorCommand::cmdSettings(int narg, const char * const args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpSettings(action);
    int val = -1; //ret,
    char ans[1000];

   
    if (cmd == "trimval") {
        if (action == PUT_ACTION) {
            if (sscanf(args[1], "%d", &val))
                myDet->setAllTrimbits(val, detPos);
            else
                return std::string("invalid trimbit value ") + cmd;
        }
        sprintf(ans, "%d", myDet->setAllTrimbits(-1, detPos));
        return ans;
    }
    return std::string("unknown settings command ") + cmd;
}

std::string slsDetectorCommand::helpSettings(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "trimval i \n sets all the trimbits to i" << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
         os << "trimval \n returns the value all trimbits are set to. If they are different, returns -1." << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdSN(int narg, const char * const args[], int action, int detPos) {

    if (action == PUT_ACTION)
        return std::string("cannot set");

    if (action == HELP_ACTION)
        return helpSN(action);


    if (cmd == "checkdetversion") {
        myDet->checkDetectorVersionCompatibility(detPos);
        return std::string("compatible");
    }

    if (cmd == "rx_checkversion") {
        myDet->checkReceiverVersionCompatibility(detPos);
        return std::string("compatible");
    }

    return std::string("unknown id mode ") + cmd;
}

std::string slsDetectorCommand::helpSN(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "checkdetversion \n gets the version compatibility with detector server (if hostname is in shared memory). Only for Eiger, Jungfrau & Gotthard. Prints compatible/ incompatible." << std::endl;
        os << "rx_checkversion \n gets the version compatibility with receiver server (if rx_hostname is in shared memory). Only for Eiger, Jungfrau & Gotthard. Prints compatible/ incompatible." << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdDigiTest(int narg, const char * const args[], int action, int detPos) {

    char answer[1000];

    if (action == HELP_ACTION)
        return helpSN(action);


    if (cmd == "bustest") {
        if (action == PUT_ACTION)
            return std::string("cannot set ") + cmd;
        sprintf(answer, "%d", myDet->digitalTest(DETECTOR_BUS_TEST));
        return std::string(answer);
    }

    else if (cmd == "firmwaretest") {
        if (action == PUT_ACTION)
            return std::string("cannot set ") + cmd;
        sprintf(answer, "%d", myDet->digitalTest(DETECTOR_FIRMWARE_TEST));
        return std::string(answer);
    }


    return std::string("unknown test mode ") + cmd;
}

std::string slsDetectorCommand::helpDigiTest(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "bustest \t performs test of the bus interface between FPGA and embedded Linux system. Can last up to a few minutes. Jungfrau only." << std::endl;
        os << "firmwaretest \t performs the firmware test. Jungfrau only." << std::endl;
    }
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "bustest \t performs test of the bus interface between FPGA and embedded Linux system. Can last up to a few minutes. Jungfrau only." << std::endl;
        os << "firmwaretest \t performs the firmware test. Jungfrau only." << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdRegister(int narg, const char * const args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpRegister(action);

    int addr, val, n;
    char answer[1000];


    // "reg" //

    // "setbit" //

    // "clearbit" //

    // "getbit" //

    if (action == PUT_ACTION) {
        if (cmd == "getbit")
            return std::string("Cannot put");

        if (narg < 3) {
            if (cmd == "reg")
                return std::string("wrong usage: should specify both address and value (hexadecimal fomat) ");
            else
                return std::string("wrong usage: should specify both address (hexadecimal fomat) and bit number");
        }

        if (sscanf(args[1], "%x", &addr))
            ;
        else
            return std::string("Could not scan address  (hexadecimal fomat) ") + std::string(args[1]);

        if (cmd == "reg") {
            if (sscanf(args[2], "%x", &val))
                ;
            else
                return std::string("Could not scan value  (hexadecimal fomat) ") + std::string(args[2]);
            sprintf(answer, "0x%x", myDet->writeRegister(addr, val, detPos));
        } else if (cmd == "adcreg") {
            if (sscanf(args[2], "%x", &val))
                ;
            else
                return std::string("Could not scan value  (hexadecimal fomat) ") + std::string(args[2]);
            myDet->writeAdcRegister(addr, val, detPos);
            sprintf(answer, "%s","successful");
        } else {

            if (sscanf(args[2], "%d", &n))
                ;
            else
                return std::string("Could not scan bit number ") + std::string(args[2]);

            if (n < 0 || n > 31)
                return std::string("Bit number out of range") + std::string(args[2]);

            if (cmd == "setbit")
                sprintf(answer, "0x%x", myDet->setBit(addr, n, detPos));
            if (cmd == "clearbit")
                sprintf(answer, "0x%x", myDet->clearBit(addr, n, detPos));
        }

    } else {
        if (cmd == "setbit")
            return std::string("Cannot get");
        if (cmd == "clearbit")
            return std::string("Cannot get");
        if (cmd == "adcreg")
            return std::string("Cannot get");

        if (cmd == "reg") {
            if (narg < 2)
                return std::string("wrong usage: should specify address  (hexadecimal fomat) ");
            if (sscanf(args[1], "%x", &addr))
                ;
            else
                return std::string("Could not scan address  (hexadecimal fomat) ") + std::string(args[1]);

            sprintf(answer, "0x%x", myDet->readRegister(addr, detPos));
        }

        if (cmd == "getbit") {

            if (narg < 3)
                return std::string("wrong usage: should specify both address (hexadecimal fomat) and bit number");

            if (sscanf(args[1], "%x", &addr))
                ;
            else
                return std::string("Could not scan address  (hexadecimal fomat) ") + std::string(args[1]);

            if (sscanf(args[2], "%d", &n))
                ;
            else
                return std::string("Could not scan bit number ") + std::string(args[2]);

            if (n < 0 || n > 31)
                return std::string("Bit number out of range") + std::string(args[2]);

            sprintf(answer, "%d", (myDet->readRegister(addr, detPos) >> n) & 1);
        }
    }

    return std::string(answer);
}

std::string slsDetectorCommand::helpRegister(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "reg addr val \n writes the register addr with the value val (hexadecimal format)" << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "reg addr \n reads the register addr" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdDAC(int narg, const char * const args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpDAC(action);

    dacIndex dac;
    int val = -1;
    char answer[1000];
    int mode = 0;
    int iarg = 1;

    if (cmd == "dac") {
        int idac = -1;
        if (sscanf(args[iarg], "%d", &idac) != 1) {
            return std::string("Could not scan dac index") + std::string(args[iarg]);
        }
        dac = (dacIndex)idac;
        ++iarg;
        --narg;
    }
    else if (cmd == "vthreshold")
        dac = THRESHOLD;
    else if (cmd == "vcalibration")
        dac = CALIBRATION_PULSE;
    else if (cmd == "vtrimbit")
        dac = TRIMBIT_SIZE;
    else if (cmd == "vpreamp")
        dac = PREAMP;
    else if (cmd == "vshaper1" || cmd == "vshaper")
        dac = SHAPER1;
    else if (cmd == "vshaper2" || cmd == "vshaperneg")
        dac = SHAPER2;
    else if (cmd == "vapower")
        dac = VA_POT;
    else if (cmd == "vddpower")
        dac = VDD_POT;
    else if (cmd == "vshpower")
        dac = VSH_POT;
    else if (cmd == "viopower")
        dac = VIO_POT;
    else if (cmd == "vref_ds")
        dac = G_VREF_DS;
    else if (cmd == "vcascn_pb")
        dac = G_VCASCN_PB;
    else if (cmd == "vcascp_pb")
        dac = G_VCASCP_PB;
    else if (cmd == "vout_cm")
        dac = G_VOUT_CM;
    else if (cmd == "vcasc_out")
        dac = G_VCASC_OUT;
    else if (cmd == "vin_cm")
        dac = G_VIN_CM;
    else if (cmd == "vref_comp")
        dac = G_VREF_COMP;
    else if (cmd == "ib_test_c")
        dac = G_IB_TESTC;

    else if (cmd == "vsvp")
        dac = E_SvP;
    else if (cmd == "vsvn")
        dac = E_SvN;
    else if (cmd == "vtr")
        dac = E_Vtr;
    else if (cmd == "vrf")
        dac = E_Vrf;
    else if (cmd == "vrs")
        dac = E_Vrs;
    else if (cmd == "vtgstv")
        dac = E_Vtgstv;
    else if (cmd == "vcmp_ll")
        dac = E_Vcmp_ll;
    else if (cmd == "vcmp_lr")
        dac = E_Vcmp_lr;
    else if (cmd == "vcall")
        dac = E_cal;
    else if (cmd == "vcmp_rl")
        dac = E_Vcmp_rl;
    else if (cmd == "vcmp_rr")
        dac = E_Vcmp_rr;
    else if (cmd == "rxb_rb")
        dac = E_rxb_rb;
    else if (cmd == "rxb_lb")
        dac = E_rxb_lb;
    else if (cmd == "vcp")
        dac = E_Vcp;
    else if (cmd == "vcn")
        dac = E_Vcn;
    else if (cmd == "vis")
        dac = E_Vis;
    else if (cmd == "iodelay")
        dac = IO_DELAY;
    else if (cmd == "vipre")
        dac = M_vIpre;
    else if (cmd == "viinsh")
        dac = M_vIinSh;
    else if (cmd == "vdcsh")
        dac = M_VdcSh;
    else if (cmd == "vth1")
        dac = THRESHOLD;
    else if (cmd == "vth2")
        dac = M_Vth2;
    else if (cmd == "vth3")
        dac = M_Vth3;
    else if (cmd == "vpl")
        dac = M_VPL;
     else if (cmd == "vph")
        dac = CALIBRATION_PULSE;   
    else if (cmd == "vtrim")
        dac = TRIMBIT_SIZE;
    else if (cmd == "cassh")
        dac = M_casSh;
    else if (cmd == "cas")
        dac = M_cas;
    else if (cmd == "vicin")
        dac = M_vIcin;
    else if (cmd == "vref_h_adc")
        dac = VREF_H_ADC;
    else if (cmd == "vb_comp_fe")
        dac = VB_COMP_FE; 
    else if (cmd == "vb_comp_adc")
        dac = VB_COMP_ADC;
    else if (cmd == "vcom_cds")
        dac = VCOM_CDS;     
    else if (cmd == "vref_restore")
        dac = VREF_RESTORE;
    else if (cmd == "vb_opa_1st")
        dac = VB_OPA_1ST; 
    else if (cmd == "vref_comp_fe")
        dac = VREF_COMP_FE;
    else if (cmd == "vcom_adc1")
        dac = VCOM_ADC1;  
    else if (cmd == "vref_prech")
        dac = VREF_PRECH;
    else if (cmd == "vref_l_adc")
        dac = VREF_L_ADC; 
    else if (cmd == "vref_cds")
        dac = VREF_CDS;
    else if (cmd == "vb_cs")
        dac = VB_CS;     
    else if (cmd == "vb_opa_fd")
        dac = VB_OPA_FD;
    else if (cmd == "vcom_adc2")
        dac = VCOM_ADC2;       
    else
        return std::string("cannot decode dac ") + cmd;


    if (action == PUT_ACTION) {

        if (sscanf(args[iarg], "%d", &val)) 
            ;
        else
            return std::string("cannot scan DAC value ") + std::string(args[iarg]);
        ++iarg;

        if ((narg >= 3) && (!strcasecmp(args[iarg], "mv")))
                mode = 1;

        myDet->setDAC(val, dac, mode, detPos);
    }
   
    // get (dacs in dac units or mV)
    else if ((narg >= 2) && (!strcasecmp(args[iarg], "mv"))) {
             mode = 1;
    }
    sprintf(answer, "%d", myDet->setDAC(-1, dac, mode, detPos));
    if (mode)
        strcat(answer, " mV");
    return std::string(answer);
}

std::string slsDetectorCommand::helpDAC(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "vthreshold dacu\t sets the detector threshold in dac units (0-1024) or mV. The energy is approx 800-15*keV" << std::endl;
        os << std::endl;

        os << "vcalibration "
           << "dacu\t sets the calibration pulse amplitude in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vtrimbit "
           << "dacu\t sets the trimbit amplitude in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vpreamp "
           << "dacu\t sets the preamp feedback voltage in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vshaper1 "
           << "dacu\t sets the shaper1 feedback voltage in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vshaper2 "
           << "dacu\t sets the  shaper2 feedback voltage in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vapower "
           << "dacu\t CHIPTEST BOARD ONLY - sets the analog power supply in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vddpower "
           << "dacu\t CHIPTEST BOARD ONLY - sets the digital power supply in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vshpower "
           << "dacu\t CHIPTEST BOARD ONLY - sets the comparator power supply in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "viopower "
           << "dacu\t CHIPTEST BOARD ONLY - sets the FPGA I/O power supply in dac units (0-1024)." << std::endl;

        os << "vrefds "
           << "dacu\t sets vrefds" << std::endl;
        os << "vcascn_pb "
           << "dacu\t sets vcascn_pb" << std::endl;
        os << "vcascp_pb "
           << "dacu\t sets vcascp_pb" << std::endl;
        os << "vout_cm "
           << "dacu\t sets vout_cm" << std::endl;
        os << "vin_cm "
           << "dacu\t sets vin_cm" << std::endl;
        os << "vcasc_out "
           << "dacu\t sets vcasc_out" << std::endl;
        os << "vref_comp "
           << "dacu\t sets vref_comp" << std::endl;
        os << "ib_test_c "
           << "dacu\t sets ib_test_c" << std::endl;

        os << "vsvp"
           << "dacu\t sets vsvp" << std::endl;
        os << "vsvn"
           << "dacu\t sets vsvn" << std::endl;
        os << "vtr"
           << "dacu\t sets vtr" << std::endl;
        os << "vrf"
           << "dacu\t sets vrf" << std::endl;
        os << "vrs"
           << "dacu\t sets vrs" << std::endl;
        os << "vtgstv"
           << "dacu\t sets vtgstv" << std::endl;
        os << "vcmp_ll"
           << "dacu\t sets vcmp_ll" << std::endl;
        os << "vcmp_lr"
           << "dacu\t sets vcmp_lr" << std::endl;
        os << "vcall"
           << "dacu\t sets vcall" << std::endl;
        os << "vcmp_rl"
           << "dacu\t sets vcmp_rl" << std::endl;
        os << "vcmp_rr"
           << "dacu\t sets vcmp_rr" << std::endl;
        os << "rxb_rb"
           << "dacu\t sets rxb_rb" << std::endl;
        os << "rxb_lb"
           << "dacu\t sets rxb_lb" << std::endl;
        os << "vcp"
           << "dacu\t sets vcp " << std::endl;
        os << "vcn"
           << "dacu\t sets vcn " << std::endl;
        os << "vis"
           << "dacu\t sets vis " << std::endl;

        os << "<dac name> mv <value> if you want in mV else <dac name> <value> in dac units " << std::endl;
    }

    if (action == GET_ACTION || action == HELP_ACTION) {

        os << "vthreshold \t Gets the detector threshold in dac units (0-1024). The energy is approx 800-15*keV" << std::endl;
        os << std::endl;

        os << "vcalibration "
           << "dacu\t gets the calibration pulse amplitude in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vtrimbit "
           << "dacu\t gets the trimbit amplitude in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vpreamp "
           << "dacu\t gets the preamp feedback voltage in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vshaper1 "
           << "dacu\t gets the shaper1 feedback voltage in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vshaper2 "
           << "dacu\t gets the  shaper2 feedback voltage in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vapower "
           << "dacu\t CHIPTEST BOARD ONLY - gets the analog power supply in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vddpower "
           << "dacu\t CHIPTEST BOARD ONLY - gets the digital power supply in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vshpower "
           << "dacu\t CHIPTEST BOARD ONLY - gets the comparator power supply in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "viopower "
           << "dacu\t CHIPTEST BOARD ONLY - gets the FPGA I/O power supply in dac units (0-1024)." << std::endl;
        os << std::endl;

        os << "vrefds "
           << "\t gets vrefds" << std::endl;
        os << "vcascn_pb "
           << "\t gets vcascn_pb" << std::endl;
        os << "vcascp_pb "
           << "\t gets vcascp_pb" << std::endl;
        os << "vout_cm "
           << "\t gets vout_cm" << std::endl;
        os << "vin_cm "
           << "\t gets vin_cm" << std::endl;
        os << "vcasc_out "
           << "\t gets vcasc_out" << std::endl;
        os << "vref_comp "
           << "\t gets vref_comp" << std::endl;
        os << "ib_test_c "
           << "\t gets ib_test_c" << std::endl;

        os << "vsvp"
           << "dacu\t gets vsvp" << std::endl;
        os << "vsvn"
           << "dacu\t gets vsvn" << std::endl;
        os << "vtr"
           << "dacu\t gets vtr" << std::endl;
        os << "vrf"
           << "dacu\t gets vrf" << std::endl;
        os << "vrs"
           << "dacu\t gets vrs" << std::endl;
        os << "vtgstv"
           << "dacu\t gets vtgstv" << std::endl;
        os << "vcmp_ll"
           << "dacu\t gets vcmp_ll" << std::endl;
        os << "vcmp_lr"
           << "dacu\t gets vcmp_lr" << std::endl;
        os << "vcall"
           << "dacu\t gets vcall" << std::endl;
        os << "vcmp_rl"
           << "dacu\t gets vcmp_rl" << std::endl;
        os << "vcmp_rr"
           << "dacu\t gets vcmp_rr" << std::endl;
        os << "rxb_rb"
           << "dacu\t gets rxb_rb" << std::endl;
        os << "rxb_lb"
           << "dacu\t gets rxb_lb" << std::endl;
        os << "vcp"
           << "dacu\t gets vcp " << std::endl;
        os << "vcn"
           << "dacu\t gets vcn " << std::endl;
        os << "vis"
           << "dacu\t gets vis " << std::endl;
    }
    return os.str();
}





std::string slsDetectorCommand::cmdTimeLeft(int narg, const char * const args[], int action, int detPos) {
    timerIndex index;
    int64_t ret;
    double rval;

    char answer[1000];

    if (action == HELP_ACTION)
        return helpTimeLeft(action);

    if (cmd == "now")
        index = ACTUAL_TIME;
    else if (cmd == "timestamp")
        index = MEASUREMENT_TIME;
    else if (cmd == "nframes")
        index = FRAMES_FROM_START;
    else
        return std::string("could not decode timer ") + cmd;

    if (action == PUT_ACTION) {
        return std::string("cannot set ") + std::string(args[1]);
    }


    ret = myDet->getTimeLeft(index, detPos);

    if ((ret != -1) && (index == ACTUAL_TIME || index == MEASUREMENT_TIME)) {
        rval = (double)ret * 1E-9;
        sprintf(answer,"%0.9f",rval);
    } else {
        sprintf(answer,"%lld",(long long int)ret);
    }

    return std::string(answer);
}

std::string slsDetectorCommand::helpTimeLeft(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << std::endl;
    }
    return os.str();
}


std::string slsDetectorCommand::cmdAdvanced(int narg, const char * const args[], int action, int detPos) {

    
    if (action == HELP_ACTION)
        return helpAdvanced(action);
  
    if (cmd == "copydetectorserver") {
        if (action == GET_ACTION)
            return std::string("cannot get");
        if (narg < 3)
        	return ("wrong usage." + helpAdvanced(PUT_ACTION));
        std::string sval = std::string(args[1]);
        std::string pval = std::string(args[2]);
        myDet->copyDetectorServer(sval, pval, detPos);
        return std::string("successful");
    }

    else if (cmd == "rebootcontroller") {
        if (action == GET_ACTION)
            return std::string("cannot get");
        myDet->rebootController(detPos);
        return std::string("successful");
    }

    else if (cmd == "update") {
        if (action == GET_ACTION)
            return std::string("cannot get");
        if (narg < 4)
        	return ("wrong usage." + helpAdvanced(PUT_ACTION));
        // pof
        if (strstr(args[3], ".pof") == nullptr)
            return std::string("wrong usage: programming file should have .pof extension");
        std::string sval = std::string(args[1]);
        std::string pval = std::string(args[2]);
        std::string fval = std::string(args[3]);
        myDet->update(sval, pval, fval, detPos);
        return std::string("successful");
    }


 else
        return std::string("unknown command ") + cmd;
}

std::string slsDetectorCommand::helpAdvanced(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "copydetectorserver s p \t copies the detector server s via tftp from pc with hostname p and changes respawn server. Not for Eiger. " << std::endl;
        os << "rebootcontroller \t reboot controler blackfin of the detector. Not for Eiger." << std::endl;
        os << "update s p f \t updates the firmware to f and detector server to f from host p via tftp and then reboots controller (blackfin). Not for Eiger. " << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdConfiguration(int narg, const char * const args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpConfiguration(action);

    if (cmd == "config") {
        if (action == PUT_ACTION) {
            myDet->readConfigurationFile(std::string(args[1]));
        } 
        return std::string("success");
    } 
    return std::string("could not decode conf mode");
}

std::string slsDetectorCommand::helpConfiguration(int action) {

    std::ostringstream os;
    return os.str();
}

std::string slsDetectorCommand::cmdReceiver(int narg, const char * const args[], int action, int detPos) {
    char answer[100];

    if (action == HELP_ACTION)
        return helpReceiver(action);

    if (cmd == "resetframescaught") {
        if (action == GET_ACTION)
            return std::string("cannot get");
        else {
            myDet->resetFramesCaught(detPos);
            return std::string("successful");
        }
    }

    else if (cmd == "frameindex") {
        if (action == PUT_ACTION)
            return std::string("cannot put");
        else {
            sprintf(answer, "%lu", myDet->getReceiverCurrentFrameIndex(detPos));
            return std::string(answer);
        }
    }


    return std::string("could not decode command");
}

std::string slsDetectorCommand::helpReceiver(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "resetframescaught [any value] \t resets frames caught by receiver" << std::endl;

    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "frameindex \t returns the current frame index of receiver(average for multi)" << std::endl;

    }
    return os.str();
}




