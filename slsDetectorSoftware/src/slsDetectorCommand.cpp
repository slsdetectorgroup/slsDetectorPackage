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
   - <b>imagetest [i]</b> If 1, adds channel intensity with precalculated values. Default is 0. Gotthard only.
	 */
    descrToFuncMap[i].m_pFuncName = "imagetest";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDigiTest;
    ++i;

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
   - <b>  busy i</b> sets/gets acquiring flag. \c 1 the acquisition is active, \c 0 otherwise. Acquire command will set this flag to 1 at the beginning and to 0 at the end. Use this to clear flag if acquisition terminated unexpectedly. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "busy";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdStatus;
    ++i;

    /*! \page acquisition
   - <b> status [s] </b> starts or stops acquisition in detector in non blocking mode. When using stop acquisition and if acquisition is done, it will restream the stop packet from receiver (if data streaming in receiver is on). Eiger can also provide an internal software trigger. \c s: [\c start, \c stop, \c trigger(EIGER only)]. \c Returns the detector status: [\c running, \c error, \c transmitting, \c finished, \c waiting, \c idle]. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "status";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdStatus;
    ++i;

    /*! \page acquisition
   - \b data gets all data from the detector (if any) processes them and writes them to file according to the preferences already setup (Eigerr store in ram only). Only get!
	 */
    descrToFuncMap[i].m_pFuncName = "data";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdData;
    ++i;

    /*! \page acquisition
   - <b>resmat i </b> sets/resets counter bit in detector.gets the counter bit in Eiger
	 */
    descrToFuncMap[i].m_pFuncName = "resmat";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdCounter;
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
   - <b>hostname</b> \c put frees shared memory and sets the hostname (or IP adress). Only allowed at multi detector level. \c Returns the list of the hostnames of the multi-detector structure. \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "hostname";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdHostname;
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

    /*! \page config
    - <b>activate [b] [p]</b> Activates/Deactivates the detector. \c b is 1 for activate, 0 for deactivate. Deactivated detector does not send data. \c p is optional and can be padding (default) or nonpadding for receivers for deactivated detectors. Used for EIGER only. \c Returns \c (int) (string)
	 */
    descrToFuncMap[i].m_pFuncName = "activate";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdOnline;
    ++i;

    /* detector and data size */
    /*! \page config
		\section configsize Data Size
   commands to configure detector data size
	 */

    /*! \page config
   - <b>dr [i]</b> sets/gets the dynamic range of detector. Eiger [4,8,16,32]. Others cannot put! \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "dr";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDetectorSize;
    ++i;

    /*! \page config
   - <b>clearroi </b> resets region of interest of the detector. Used for GOTTHARD only. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "clearroi";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDetectorSize;
    ++i;

    /*! \page config
   - <b>roi [xmin] [xmax] </b> sets region of interest of the detector. Used for GOTTHARD only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "roi";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDetectorSize;
    ++i;

    /*! \page config
   - <b>detsizechan [xmax] [ymax]</b> sets the maximum number of channels in each dimension for complete detector set; 0 is no limit. Use for multi-detector system as first command in config file. \c Returns \c ("int int")
	 */
    descrToFuncMap[i].m_pFuncName = "detsizechan";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDetectorSize;
    ++i;

    /*! \page config
   - <b>quad [i] </b> if 1, sets the detector size to a quad (Specific to an EIGER quad hardware). 0 by default. \c Returns \c (int)
	 */
	descrToFuncMap[i].m_pFuncName="quad"; //
	descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdDetectorSize;
	++i;


    /*! \page config
   - <b>flippeddatax [i]</b> enables/disables data being flipped across x axis. 1 enables, 0 disables. Used for EIGER only. 1 for bottom half-module, 0 for top-half module. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "flippeddatax";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDetectorSize;
    ++i;

    /*! \page config
   - <b>tengiga [i]</b> enables/disables 10GbE in system (detector & receiver). 1 enabled 10GbE, 0 enables 1GbE. Used in EIGER, Moench and ChipTestBoard only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "tengiga";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page config
   - <b>gappixels [i]</b> enables/disables gap pixels in system (detector & receiver). 1 sets, 0 unsets. Used in EIGER only and only in multi detector level command. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "gappixels";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDetectorSize;
    ++i;

    /* flags */
    /*! \page config
		\section configflags Flags
   commands to configure detector flags
	 */

    /*! \page config
   - <b>romode [b]</b> sets/gets the readout flag. Options: analog, digital, analog_digital. Used for CTB only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "romode";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

	/*! \page config
   - <b>interruptsubframe [i]</b> sets/gets the interrupt subframe flag. Setting it to 1 will interrupt the last subframe at the required exposure time. By default, this is disabled and set to 0, ie. it will wait for the last sub frame to finish exposing. Used for EIGER  in 32 bit mode only. \c Returns \c (int).
	 */
	descrToFuncMap[i].m_pFuncName="interruptsubframe";
	descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdAdvanced;
	++i;

    /*! \page config
   - <b>readnlines [i]</b> sets/gets the number of rows to read out per half module. Options: 1 - 256 (Not all values as it depends on dynamic range and 10GbE enabled). Used for EIGER only. \c Returns \c (int) 
	 */
    descrToFuncMap[i].m_pFuncName = "readnlines";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /*! \page config
   - <b>extsig [flag]</b> sets/gets the mode of the external signal. Options: \c trigger_in_rising_edge, \c trigger_in_falling_edge. Used in GOTTHARDonly. \c Returns \c (string)
	*/
    descrToFuncMap[i].m_pFuncName = "extsig"; /* find command! */
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /* fpga */

    /*! \page config
   - <b>programfpga [file]</b> programs the FPGA with file f (with .pof extension). Used for JUNGFRAU only. Only put! \c Returns \c ("successful", "failed")
	 */
    descrToFuncMap[i].m_pFuncName = "programfpga";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /*! \page config
   - <b>resetfpga [f]</b> resets FPGA, where f can be any value. Used for JUNGFRAU only. Only put! \c Returns \c ("successful", "failed")
	 */
    descrToFuncMap[i].m_pFuncName = "resetfpga";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

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

    /*! \page config
   - <b>powerchip [i]</b> Powers on/off the chip. 1 powers on, 0 powers off. Can also get the power status. Used for JUNGFRAU only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "powerchip";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /*! \page config
   - <b>led [i]</b> sets/gets the led status. 1 on, 0 off. Used for MOENCH only ?? \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "led";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /*! \page config
   - <b>diodelay [i] [v]</b> sets the delay for the digital IO pins selected by mask i and delay set by v. mask is upto 64 bits in hex, delay is a max is 775ps, and set in steps of 25 ps. Used for MOENCH/CTB only. Cannot get. \c Returns \c ("successful", failed")
	 */
    descrToFuncMap[i].m_pFuncName = "diodelay";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /*! \page config
   - <b>auto_comp_disable i </b> Currently not implemented. this mode disables the on-chip gain switching comparator automatically after 93.75% of exposure time (only for longer than 100us). 1 enables mode, 0 disables mode. By default, mode is disabled (comparator is enabled throughout). (JUNGFRAU only). \c Returns \c (int)
     */
    descrToFuncMap[i].m_pFuncName = "auto_comp_disable";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /*! \page config
   - <b>pulse [n] [x] [y]</b> pulses pixel at coordinates (x,y) n number of times. Used in EIGER only. Only put! \c Returns \c ("successful", "failed")
	 */
    descrToFuncMap[i].m_pFuncName = "pulse";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPulse;
    ++i;

    /*! \page config
   - <b>pulsenmove [n] [x] [y]</b> pulses pixel n number of times and moves relatively by x value (x axis) and y value(y axis). Used in EIGER only. Only put! \c Returns \c ("successful", "failed")
	 */
    descrToFuncMap[i].m_pFuncName = "pulsenmove";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPulse;
    ++i;

    /*! \page config
   - <b>pulsechip [n]</b>pulses chip n number of times, while n=-1 will reset it to normal mode. Used in EIGER only. Only put! \c Returns \c ("successful", "failed")
	 */
    descrToFuncMap[i].m_pFuncName = "pulsechip";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPulse;
    ++i;

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

    /*! \page config
   - <b>detectornumber</b> Gets the serial number or MAC of detector. Only get! \c Returns \c (long int) in hexadecimal
	 */
    descrToFuncMap[i].m_pFuncName = "detectornumber";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

    /*! \page config
   - <b>detectorversion</b> Gets the firmware version of detector. Only get! \c Returns \c (long int) in hexadecimal
	 */
    descrToFuncMap[i].m_pFuncName = "detectorversion";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

    /*! \page config
   - <b>softwareversion</b> Gets the software version of detector server. Only get! \c Returns \c (long int) in hexadecimal
	 */
    descrToFuncMap[i].m_pFuncName = "softwareversion";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

    /*! \page config
   - <b>thisversion</b> Gets the software version of this client software. Only get! \c Returns \c (long int) in hexadecimal
	 */
    descrToFuncMap[i].m_pFuncName = "thisversion";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

    /*! \page config
   - <b>rx_version</b> Gets the software version of receiver. Only get! \c Returns \c (long int) in hexadecimal
	 */
    descrToFuncMap[i].m_pFuncName = "rx_version";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

    /* r/w timers */

    /*! \page timing
   - <b>timing [mode]</b> sets/gets synchronization mode of the detector. Mode: auto, trigger, ro_trigger, gating, triggered_gating (string)
	*/
    descrToFuncMap[i].m_pFuncName = "timing";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTiming;
    ++i;

    /*! \page timing
   - <b>subdeadtime [i]</b> sets/gets sub frame dead time in s. Subperiod is set in the detector = subexptime + subdeadtime. This value is normally a constant in the config file. Used in EIGER only in 32 bit mode. \c Returns \c (double with 9 decimal digits)
     */
    descrToFuncMap[i].m_pFuncName = "subdeadtime";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>delay [i]</b> sets/gets delay in s. Used in GOTTHARD only. \c Returns \c (double with 9 decimal digits)
	 */
    descrToFuncMap[i].m_pFuncName = "delay";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;


    /*! \page timing
   - <b>startingfnum [i]</b> sets/gets starting frame number for the next acquisition. Only for Jungfrau and Eiger. \c Returns \c (long long int)
	 */
    descrToFuncMap[i].m_pFuncName = "startingfnum";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>cycles [i]</b> sets/gets number of triggers. Timing mode should be set appropriately. \c Returns \c (long long int)
	 */
    descrToFuncMap[i].m_pFuncName = "cycles";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>samples [i]</b> sets/gets number of samples (both analog and digital) expected from the ctb. Used in CHIP TEST BOARD  and MOENCH only. \c Returns \c (long long int)
	 */
    descrToFuncMap[i].m_pFuncName = "samples";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>asamples [i]</b> sets/gets number of analog samples expected from the ctb. Used in CHIP TEST BOARD and MOENCH only. \c Returns \c (long long int)
	 */
    descrToFuncMap[i].m_pFuncName = "asamples";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>dsamples [i]</b> sets/gets number of digital samples expected from the ctb. Used in CHIP TEST BOARD and MOENCH only. \c Returns \c (long long int)
	 */
    descrToFuncMap[i].m_pFuncName = "dsamples";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>storagecells [i]</b> sets/gets number of additional storage cells per acquisition. For very advanced users only! For JUNGFRAU only. Range: 0-15. The #images = #frames * #cycles * (#storagecells +1). \c Returns \c (long long int)
     */
    descrToFuncMap[i].m_pFuncName = "storagecells";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>storagecell_start [i]</b> sets/gets the storage cell that stores the first acquisition of the series. Default is 15(0xf).. For very advanced users only! For JUNGFRAU only. Range: 0-15. \c Returns \c (int)
     */
    descrToFuncMap[i].m_pFuncName = "storagecell_start";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>storagecell_start [i]</b> sets/gets the storage cell that stores the first acquisition of the series. Default is 15(0xf).. For very advanced users only! For JUNGFRAU only. Range: 0-15. \c Returns \c (int)
     */
    descrToFuncMap[i].m_pFuncName = "storagecell_start";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>storagecell_delay [i]</b> sets/gets additional time between 2 storage cells. For very advanced users only! For JUNGFRAU only. Range: 0-1638375 ns (resolution of 25ns). \c Returns \c (int)
     */
    descrToFuncMap[i].m_pFuncName = "storagecell_delay";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /* read only timers */

    /*! \page timing
   - <b>exptimel</b> gets exposure time left. Used in GOTTHARD only. Only get! \c Returns \c (double with 9 decimal digits)
	 */
    descrToFuncMap[i].m_pFuncName = "exptimel";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page timing
   - <b>periodl</b> gets frame period left. Used in GOTTHARD  and Jungfrau only. Only get! \c Returns \c (double with 9 decimal digits)
	 */
    descrToFuncMap[i].m_pFuncName = "periodl";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page timing
   - <b>delayl</b> gets delay left. Used in GOTTHARD, JUNGFRAU, MOENCH and CTB only. Only get! \c Returns \c (double with 9 decimal digits)
	 */
    descrToFuncMap[i].m_pFuncName = "delayl";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page config
   - <b>framesl</b> gets number of frames left. Used in GOTTHARD and Jungfrau only. Only get! \c Returns \c (long long int)
	 */
    descrToFuncMap[i].m_pFuncName = "framesl";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page timing
   - <b>cyclesl</b> gets number of cylces left. Used in GOTTHARD  and Jungfrau only. Only get! \c Returns \c (long long int)
	 */
    descrToFuncMap[i].m_pFuncName = "cyclesl";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

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

    /*! \page timing
   - <b>measuredperiod</b> gets the measured frame period (time between last frame and the previous one) in s. For Eiger only. Makes sense only for acquisitions of more than 1 frame. \c Returns \c  (double with 9 decimal digits)
     */
    descrToFuncMap[i].m_pFuncName = "measuredperiod";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page timing
   - <b>measuredsubperiod</b> gets the measured subframe period (time between last subframe and the previous one) in s. For Eiger only and in 32 bit mode. \c Returns \c  (double with 9 decimal digits)
     */
    descrToFuncMap[i].m_pFuncName = "measuredsubperiod";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /* speed */
    /*! \page config
		\section configspeed Speed
   commands to configure speed of detector
	 */

    /*! \page config
   - <b>clkdivider [i]</b> sets/gets the readout clock divider. EIGER, JUNGFRAU [0(fast speed), 1(half speed), 2(quarter speed)]. Jungfrau also overwrites adcphase to recommended default. For CTB, it is the run clock in MHz. Not for Gotthard. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "clkdivider";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>adcclk [i]</b> sets/gets the ADC clock frequency in MHz. CTB & Moench only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "adcclk";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>dbitclk [i]</b> Sets/gets the clock frequency of the latching of the digital bits in MHz. CTB & Moench only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "dbitclk";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>syncclk </b> Gets the clock frequency of the sync clock in MHz. CTB & Moench only. Get only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "syncclk";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>adcphase [i] [deg]</b> Sets/gets phase of the ADC clock to i. i is the shift or in degrees if deg is used. deg is optional & only for CTB, Moench and Jungfrau. For CTB & Moench, adcphase is reset if adcclk is changed. For Jungfrau, adcphase changed to defaults if clkdivider changed. Jungfrau, CTB & Moench, these are absolute values with limits. Gotthard, relative phase shift. Not for Eiger. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "adcphase";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>dbitphase [i] [deg]</b> Sets/gets phase of the clock for latching of the digital bits to i. i is the shift or in degrees if deg is used. deg is optional. dbitphase is also reset if dbitclk is changed. These are absolute values with limits. for CTB & Moench only.\c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "dbitphase";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>maxadcphaseshift </b> Gets maximum phase shift of the ADC clock. CTB, Moench and Jungfrau only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "maxadcphaseshift";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>maxdbitphaseshift </b> Gets the maximum phase shift of the clock for latching of the digital bits.\c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "maxdbitphaseshift";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>adcpipeline [i]</b> Sets/gets the pipeline of the ADC. For CTB & Moench only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "adcpipeline";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>dbitpipeline [i]</b> Sets/gets the pipeline of the latching of the digital bits. For CTB & Moench only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "dbitpipeline";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /* settings dump/retrieve */
    /*! \page config
		\section configsettings Detector Parameters
   commands to configure/retrieve configuration of detector
	 */

    /*! \page config
   - <b>config [fname]</b> sets/saves detector/receiver to configuration contained in fname. Same as executing sls_detector_put for every line. Normally a one time operation. \c Returns \c (string) fname
	 */
    descrToFuncMap[i].m_pFuncName = "config";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdConfiguration;
    ++i;

    /* settings dump/retrieve */
    /*! \page config
   - <b>rx_printconfig</b> prints the receiver configuration. Only get! \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_printconfig";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdConfiguration;
    ++i;

    /*! \page config
   - <b>parameters [fname]</b> sets/saves detector parameters contained in fname. Normally once per different measurement. \c Returns \c (string) fname
	 */
    descrToFuncMap[i].m_pFuncName = "parameters";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdConfiguration;
    ++i;

    /*! \page config
   - <b>setup [fname]</b> sets/saves detector complete setup contained in fname (extensions automatically generated), including trimfiles, ff coefficients etc.  \c Returns \c (string) fname
	 */
    descrToFuncMap[i].m_pFuncName = "setup";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdConfiguration;
    ++i;

    /* data processing commands */

    /*! \page data Data processing commands
   Commands to setup the data processing
	 */

    /*! \page data
   - <b>ratecorr [ns]</b> Returns the dead time used for rate correections in ns (int). \c put sets the deadtime correction constant in ns, -1  will set it to default tau of settings (0 unset).  \c Returns \c (int). For Eiger only.
	 */
    descrToFuncMap[i].m_pFuncName = "ratecorr";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdRateCorr;
    ++i;

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
    /*! \page settings
   - <b>settingsdir [dir]</b> Sets/gets the directory where the settings files are located. \c Returns \c (string) dir
	 */
    descrToFuncMap[i].m_pFuncName = "settingsdir";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSettingsDir;
    ++i;
    /*! \page settings
   - <b>trimdir [dir]</b> obsolete \c settingsdir. \c Returns \c (string) dir
	 */
    descrToFuncMap[i].m_pFuncName = "trimdir";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSettingsDir;
    ++i;

    /*! \page settings
   - <b>trimen [n e0 e1...e(n-1)]</b> Sets/gets the number of energies n at which the detector has default trim file and their values in eV (int). \c Returns \c (int int...) n e0 e1...e(n-1)
	 */
    descrToFuncMap[i].m_pFuncName = "trimen";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTrimEn;
    ++i;

    /* settings, threshold */
    /*! \page settings
		\section settingssett Settings and Threshold
   commands to configure settings and threshold of detector
	 */

    /*! \page settings
   - <b>settings [s]</b> sets/gets the settings of the detector. Options: \c standard, \c fast, \c highgain, \c dynamicgain, \c lowgain, \c mediumgain, \c veryhighgain,
   \c dynamichg0, \c fixgain1, \c fixgain2, \c forceswitchg1, \c forceswitchg2.
   \n In Eiger, only sets in client shared memory. Use \c threshold or \c thresholdnotb to pass to detector. Gets from detector.  \c Returns \c (string) s
	 */
    descrToFuncMap[i].m_pFuncName = "settings";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSettings;
    ++i;

    /*! \page settings
   - <b>threshold [eV] [sett] </b> sets/gets the detector threshold in eV. sett is optional and if provided also sets the settings. Use this for Eiger instead of \c settings. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "threshold";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSettings;
    ++i;

    /*! \page settings
   - <b>thresholdnotb [eV] [sett] </b> sets/gets the detector threshold in eV without loading trimbits. sett is optional and if provided also sets the settings. Use this for Eiger instead of \c settings. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "thresholdnotb";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSettings;
    ++i;

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
   - <b>vhaper1 [i] [mv]</b> Sets/gets the voltage to define the  feedback resistance of the first shaper. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vshaper1";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vhaper1 [i] [mv]</b> Sets/gets the voltage to define the  feedback resistance of the first shaper. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
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
   - <b>vhaper1 [i] [mv]</b> Sets/gets the voltage to define the  feedback resistance of the first shaper. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vshaperneg";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vhighvoltage [i]</b> Sets/gets the high voltage to the sensor in V. \c Returns \c (int ["mV"]).
	 */
    descrToFuncMap[i].m_pFuncName = "vhighvoltage";
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
   - <b>adcvpp [i] </b> Sets/gets the Vpp of the ADC  0 -> 1V ; 1 -> 1.14V ; 2 -> 1.33V ; 3 -> 1.6V ; 4 -> 2V . \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "adcvpp";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>v_a [i] mv</b> Sets/gets value for Va on the new chiptest board. Must be in mV. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "v_a";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>v_b [i] mv</b> Sets/gets value for Vb on the new chiptest board. Must be in mV. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "v_b";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>v_c [i] mv</b> Sets/gets value for Vc on the new chiptest board. Must be in mV. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "v_c";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>v_d [i] mv</b> Sets/gets value for Vd on the new chiptest board. Must be in mV. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "v_d";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>v_io [i] mv</b> Sets/gets value for Vio on the new chiptest board. Must be in mV. It should be minimum 1200 mV and must be the first power regulator to be set after server start up (fpga reset). To change again, reset fpga first. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "v_io";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>v_chip [i] mv</b> Sets/gets value for Vchip on the new chiptest board. Must be in mV. \c Returns \c (int ["mV"]). Do NOT use it, unless you are completely sure you won't fry the board!
	 */
    descrToFuncMap[i].m_pFuncName = "v_chip";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>v_limit [i] mv</b> Sets/gets a soft limit for the power supplies and the DACs on the new chiptest board. Must be in mV. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "v_limit";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /* MYTHEN 3.0 
	all values are in DACu */

    descrToFuncMap[i].m_pFuncName = "vipre";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "vdcsh";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>Vth1</b> Sets/gets first detector threshold voltage for Mythen 3.01. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vth1";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>Vth1</b> Sets/gets second detector threshold voltage for Mythen 3.0. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vth2";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>Vth1</b> Sets/gets third detector threshold voltage for Mythen 3.0. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vth3";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "vpl";  //baseline for analog pulsing
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "vph";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "vtrim";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "viinsh";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "cas";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "cassh";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "vicin";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "vipreout";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "vrfsh";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "vrfshnpol";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /* r/w timers */
    /*! \page settings
		\section settingsadcs ADCs
   commands to readout ADCs of detector
	 */

    /*! \page settings
   - <b>temp_adc</b> Gets the ADC temperature. \c Returns \c EIGER,JUNGFRAU(double"°C") Others \c (int"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_adc";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_fpga</b> Gets the FPGA temperature. \c Returns \c EIGER,JUNGFRAU(double"°C") Others \c (int"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_fpga";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_fpgaext</b> Gets the external FPGA temperature. Used in EIGER only. \c Returns \c EIGER(double"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_fpgaext";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_10ge</b> Gets the 10Gbe temperature. Used in EIGER only. \c Returns \c EIGER(double"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_10ge";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_dcdc</b> Gets the temperature of the DC/DC converter. Used in EIGER only. \c Returns \c EIGER(double"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_dcdc";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_sodl</b> Gets the temperature of the left so-dimm memory . Used in EIGER only. \c Returns \c EIGER(double"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_sodl";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_sodr</b> Gets the temperature of the right so-dimm memory. Used in EIGER only. \c Returns \c EIGER(double"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_sodr";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>adc:j</b> Gets the values of the slow ADC number j for the new chiptest board. \c Returns \c (int"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "adc";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_fpgal</b> Gets the temperature of the left frontend FPGA. Used in EIGER only. \c Returns \c EIGER(double"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_fpgafl";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_fpgar</b> Gets the temperature of the right frontend FPGA. Used in EIGER only. \c Returns \c EIGER(double"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_fpgafr";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>i_a</b> Gets the current of the power supply a on the new chiptest board. \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "i_a";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>i_b</b> Gets the current of the power supply b on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "i_b";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>i_c</b> Gets the current of the power supply c on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "i_c";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>i_d</b> Gets the current of the power supply d on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "i_d";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>i_io</b> Gets the current of the power supply io on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "i_io";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>vm_a</b> Gets the measured voltage of the power supply a on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "vm_a";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>vm_b</b> Gets the measured voltage of the power supply b on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "vm_b";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>vm_c</b> Gets the measured voltage of the power supply c on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "vm_c";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>vm_d</b> Gets the measured voltage of the power supply d on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "vm_d";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>vm_io</b> Gets the measured voltage of the power supply io on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "vm_io";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /* temperature control */
    /*! \page settings
        \section settingstmp Temp Control
  commands to monitor and handle temperature overshoot (only JUNGFRAU)
     */

    /*! \page settings
   - <b>temp_threshold</b> Sets/gets the threshold temperature. JUNGFRAU ONLY. \c Returns \c (double"°C")
     */
    descrToFuncMap[i].m_pFuncName = "temp_threshold";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTempControl;
    ++i;

    /*! \page settings
   - <b>temp_control</b> Enables/Disables the temperature control. 1 enables, 0 disables.  JUNGFRAU ONLY. \c Returns \c int
     */
    descrToFuncMap[i].m_pFuncName = "temp_control";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTempControl;
    ++i;

    /*! \page settings
   - <b>temp_event</b> Resets/gets over-temperative event. Put only with option 0 to clear event. Gets 1 if temperature went over threshold and control is enabled, else 0. /Disables the temperature control.  JUNGFRAU ONLY. \c Returns \c int
     */
    descrToFuncMap[i].m_pFuncName = "temp_event";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTempControl;
    ++i;

    /* file name */

    /*! \page output Output settings
   Commands to setup the file destination and format
	 */

    /*! \page output
   - <b>fpath [dir]</b> Sets/gets the file output directory. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "fpath";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdOutDir;
    ++i;

    /*! \page output
   - <b>fname [fn]</b> Sets/gets the root of the output file name \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "fname";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdFileName;
    ++i;

    /*! \page output
    - <b>fformat [i]</b> sets/gets the file format for data in receiver. Options: [binary, hdf5]. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "fformat";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdFileName;
    ++i;

    /* communication configuration */

    /*! \page network Network
    Commands to setup the network between client, detector and receiver
    - <b>rx_hostname [s]</b> sets/gets the receiver hostname or IP address, configures detector mac with all network parameters and updates receiver with acquisition parameters. Normally used for single detectors (Can be multi-detector). \c none disables. If used, use as last network command in configuring detector MAC. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_hostname";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>rx_udpsocksize [size]</b> sets/gets the UDP socket buffer size. Already trying to set by default to 100mb, 2gb for Jungfrau. Does not remember in client shared memory, so must be initialized each time after setting receiver hostname in config file.\c Returns \c (int)
     */
    descrToFuncMap[i].m_pFuncName = "rx_udpsocksize";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>rx_realudpsocksize [size]</b> gets the actual UDP socket buffer size. Usually double the set udp socket buffer size due to kernel bookkeeping. Get only. \c Returns \c (int)
     */
    descrToFuncMap[i].m_pFuncName = "rx_realudpsocksize";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>txndelay_left [delay]</b> sets/gets the transmission delay of first packet in an image being streamed out from the detector's left UDP port. Use single-detector command. Used for EIGER only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "txndelay_left";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>txndelay_right [delay]</b> sets/gets the transmission delay of first packet in an image being streamed out from the detector's right UDP port. Use single-detector command. Used for EIGER only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "txndelay_right";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>txndelay_frame [delay]</b> sets/gets the transmission frame period of entire frame being streamed out from the detector for both ports. Use single-detector command. Used for EIGER and JUNGFRAU only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "txndelay_frame";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>flowcontrol_10g [delay]</b> Enables/disables 10 GbE flow control. 1 enables, 0 disables. Used for EIGER and JUNGFRAU only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "flowcontrol_10g";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>zmqport [port]</b> sets/gets the 0MQ (TCP) port of the client to where final data is streamed to (eg. for GUI). The default already connects with rx_zmqport for the GUI. Use single-detector command to set individually or multi-detector command to calculate based on \c port for the rest. Must restart zmq client streaming in gui/external gui \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "zmqport";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>rx_zmqport [port]</b> sets/gets the 0MQ (TCP) port of the receiver from where data is streamed from (eg. to GUI or another process for further processing). Use single-detector command to set individually or multi-detector command to calculate based on \c port for the rest.  put restarts streaming in receiver with new port. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_zmqport";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b> rx_datastream </b>enables/disables data streaming from receiver. 1 enables 0MQ data stream from receiver (creates streamer threads), while 0 disables (destroys streamer threads). Switching to Gui enables data streaming in receiver and switching back to command line acquire will require disabling data streaming in receiver for fast applications \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_datastream";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDataStream;
    ++i;

    /*! \page network
   - <b>zmqip [ip]</b> sets/gets the 0MQ (TCP) ip of the client to where final data is streamed to (eg. for GUI). For Experts only! Default is ip of rx_hostname and works for GUI. This command to change from default can be used from command line when sockets are not already open as the command line is not aware/create the 0mq sockets in the client side. This is usually used to stream in from an external process. . If no custom ip, empty until first time connect to receiver. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "zmqip";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    i++;

    /*! \page network
   - <b>rx_zmqip [ip]</b> sets/gets the 0MQ (TCP) ip of the receiver from where data is streamed from (eg. to GUI or another process for further processing). For Experts only! Default is ip of rx_hostname and works for GUI. This is usually used to stream out to an external process for further processing. . If no custom ip, empty until first time connect to receiver. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_zmqip";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    i++;

    /*! \page network
   - <b>rx_tcpport [port]</b> sets/gets the port of the client-receiver TCP interface. Use single-detector command. Is different for each detector if same \c rx_hostname used. Must be first command to communicate with receiver. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_tcpport";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPort;
    ++i;

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

    /*! \page network
   - <b>lastclient </b> Gets the last client communicating with the detector. Cannot put!. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "lastclient";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdLastClient;
    ++i;

    /* receiver functions */

    /*! \page receiver Receiver commands
   Commands to configure the receiver.
	 */

    /*! \page receiver
   - <b>rx_status [s]</b> starts/stops the receiver to listen to detector packets. Options: [ \c start, \c stop]. \c Returns \c (string) status of receiver[ \c idle, \c running].
	 */
    descrToFuncMap[i].m_pFuncName = "rx_status";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page receiver
   - <b>framescaught</b> gets the number of frames caught by receiver. Average of all for multi-detector command. Only get! \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "framescaught";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

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

    /*! \page receiver
   - <b>rx_lastclient</b> gets the last client communicating with the receiver. Only get! \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_lastclient";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdLastClient;
    ++i;

    /*! \page receiver
    - <b>rx_framesperfile [i]</b> sets/gets the frames per file in receiver to i. 0 means infinite or all frames in a single file. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_framesperfile";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page receiver
    - <b>rx_discardpolicy</b> sets/gets the frame discard policy in the receiver. nodiscard (default) - discards nothing, discardempty - discard only empty frames, discardpartial(fastest) - discards all partial frames. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_discardpolicy";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page receiver
    - <b>rx_jsonaddheader [t]</b> sets/gets additional json header to be streamed out with the zmq from receiver. Default is empty. \c t must be in the format "\"label1\":\"value1\",\"label2\":\"value2\"" etc. Use only if it needs to be processed by an intermediate process. \c Returns \c (string)
     */
    descrToFuncMap[i].m_pFuncName = "rx_jsonaddheader";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    i++;

    /*! \page receiver
    - <b>rx_jsonpara [k] [v]</b> sets/gets  value v for additional json header parameter k to be streamed out with the zmq from receiver. If empty, then no parameter found Use only if it needs to be processed by an intermediate process. \c Returns \c (string)
     */
    descrToFuncMap[i].m_pFuncName = "rx_jsonpara";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    i++;


    /* pattern generator */

    /*! \page prototype Chip Test Board / Moench
	  Commands specific for the chiptest board or moench
	 */

    /*! \page prototype
  - <b>emin [i] </b> Sets/gets detector minimum energy threshold for Moench (soft setting in processor)
     */
    descrToFuncMap[i].m_pFuncName = "emin";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdProcessor;
    ++i;

    /*! \page prototype
  - <b>emax [i] </b> Sets/gets detector maximum energy threshold for Moench (soft setting in processor)
     */
    descrToFuncMap[i].m_pFuncName = "emax";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdProcessor;
    ++i;

    /*! \page prototype
  - <b>framemode [i] </b> Sets/gets frame mode for Moench (soft setting in processor). Options: pedestal, newpedestal, flatfield, newflatfield
     */
    descrToFuncMap[i].m_pFuncName = "framemode";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdProcessor;
    ++i;

    /*! \page prototype
  - <b>detectormode [i] </b> Sets/gets detector mode for Moench (soft setting in processor). Options: counting, interpolating, analog
     */
    descrToFuncMap[i].m_pFuncName = "detectormode";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdProcessor;
    ++i;

    /*! \page prototype
   - <b>adcenable [mask]</b> Sets/gets ADC enable mask (8 digits hex format)
	 */
    descrToFuncMap[i].m_pFuncName = "adcenable";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /** not documenting this, but keeping this for backwards compatibility  */
    descrToFuncMap[i].m_pFuncName = "adcdisable";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>adcinvert [mask]</b> Sets/gets ADC inversion mask (8 digits hex format) CTB or Moench only
	 */
    descrToFuncMap[i].m_pFuncName = "adcinvert";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>extsamplingsrc [i]</b> sets/gets the sampling source signal for digital data. \ci must be between 0 and 63. Advanced!  CTB only \Returns (int)
	 */
    descrToFuncMap[i].m_pFuncName = "extsamplingsrc";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>extsampling [i]</b> enables/disables the external sampling signal to the \c samplingsrc signal for digital data. Advanced!  CTB only \Returns (int)
	 */
    descrToFuncMap[i].m_pFuncName = "extsampling";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>rx_dbitlist [i]</b> sets/gets the list of digital signal bits required for chip in receiver. If set to "all", then all digital bits are enabled. Advanced! CTB only \Returns (string)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_dbitlist";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>rx_dbitoffset [i]</b> sets/gets the offset in bytes in receiver of digital data from chip in receiver. Advanced! CTB only \Returns (int)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_dbitoffset";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;  

    /*! \page prototype
   - <b>pattern fn</b> loads binary pattern file fn
	 */
    descrToFuncMap[i].m_pFuncName = "pattern";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>savepattern fn</b> save pattern to file (ascii). This also executes the pattern.
	 */
    descrToFuncMap[i].m_pFuncName = "savepattern";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patword addr [word]</b> sets/gets 64 bit word at address addr of pattern memory. Both address and word in hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patword";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patioctrl [word]</b> sets/gets 64 bit mask defining input (0) and output (1) signals. hex format.
	 */
    descrToFuncMap[i].m_pFuncName = "patioctrl";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patclkctrl [word]</b> sets/gets 64 bit mask defining if output signal is a clock and runs. hex format. Unused at the moment.
	 */
    descrToFuncMap[i].m_pFuncName = "patclkctrl";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patlimits [addr1 addr2]</b> sets/gets the start and stop limits of the pattern to be executed. hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patlimits";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patloop0 [addr1 addr2]</b> sets/gets the start and stop limits of the level 0 loop. hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patloop0";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patnloop0 [n]</b> sets/gets the number of cyclesof the  level 0 loop (int).
	 */
    descrToFuncMap[i].m_pFuncName = "patnloop0";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patwait0 [addr]</b> sets/gets the address of the level 0 wait point. hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patwait0";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patwaittime0 [n]</b> sets/gets the duration of the witing of the 0 waiting point in clock cycles (int).
	 */
    descrToFuncMap[i].m_pFuncName = "patwaittime0";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patloop1 [addr1 addr2]</b> sets/gets the start and stop limits of the level 1 loop. hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patloop1";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patnloop1 [n]</b> sets/gets the number of cyclesof the  level 1 loop (int).
	 */
    descrToFuncMap[i].m_pFuncName = "patnloop1";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patwait1 [addr]</b> sets/gets the address of the level 1 wait point. hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patwait1";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patwaittime1 [n]</b> sets/gets the duration of the witing of the 1 waiting point in clock cycles (int).
	 */
    descrToFuncMap[i].m_pFuncName = "patwaittime1";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patloop2 [addr1 addr2]</b> sets/gets the start and stop limits of the level 2 loop. hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patloop2";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patnloop2 [n]</b> sets/gets the number of cyclesof the  level 2 loop (int).
	 */
    descrToFuncMap[i].m_pFuncName = "patnloop2";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patwait2 [addr]</b> sets/gets the address of the level 2 wait point. hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patwait2";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patwaittime2 [n]</b> sets/gets the duration of the waiting of the 2 waiting point in clock cycles (int).
	 */
    descrToFuncMap[i].m_pFuncName = "patwaittime2";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patmask [m]</b> sets/gets the 64 bit mask (hex) applied to every pattern. Only the bits from \c patsetbit are selected to mask for the corresponding bit value from \c m mask. Returns \c (uint64_t).
	 */
    descrToFuncMap[i].m_pFuncName = "patmask";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patsetbit [m]</b> selects/gets the 64 bits  (hex) that the patmask will be applied to every pattern. Only the bits from \c m mask are selected to mask for the corresponding bit value from \c patmask. Returns \c (uint64_t).
	 */
    descrToFuncMap[i].m_pFuncName = "patsetbit";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

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

std::string slsDetectorCommand::cmdStatus(int narg, const char * const args[], int action, int detPos) {

#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif

    if (action == HELP_ACTION)
        return helpStatus(action);

    if (cmd == "status") {
        if (action == PUT_ACTION) {
            //myDet->setThreadedProcessing(0);
            if (std::string(args[1]) == "start")
                myDet->startAcquisition(detPos);
            else if (std::string(args[1]) == "stop") {
                myDet->stopAcquisition(detPos);
            } else if (std::string(args[1]) == "trigger") {
                myDet->sendSoftwareTrigger(detPos);
            } else
                return std::string("unknown action");
        }
        runStatus s = myDet->getRunStatus(detPos);
        return myDet->runStatusType(s);
    } else if (cmd == "busy") {
        if (action == PUT_ACTION) {
            int i;
            if (!sscanf(args[1], "%d", &i))
                return std::string("cannot parse busy mode");
            myDet->setAcquiringFlag(i);
        }
        char answer[100];
        sprintf(answer, "%d", myDet->getAcquiringFlag());
        return std::string(answer);
    } else
        return std::string("cannot scan command ") + std::string(cmd);
}

std::string slsDetectorCommand::helpStatus(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << std::string("status \t gets the detector status - can be: running, error, transmitting, finished, waiting or idle\n");
        os << std::string("busy \t gets the status of acquire- can be: 0 or 1. 0 for idle, 1 for running\n");
    }
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << std::string("status \t controls the detector acquisition - can be start or stop or trigger(EIGER only).  When using stop acquisition and if acquisition is done, it will restream the stop packet from receiver (if data streaming in receiver is on). Eiger can also provide an internal software trigger\n");
        os << std::string("busy i\t sets the status of acquire- can be: 0(idle) or 1(running).Command Acquire sets it to 1 at beignning of acquire and back to 0 at the end. Clear Flag for unexpected acquire terminations. \n");
    }
    return os.str();
}

std::string slsDetectorCommand::cmdDataStream(int narg, const char * const args[], int action, int detPos) {

#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif
    int ival = -1;
    char ans[100] = "";

    if (action == HELP_ACTION)
        return helpDataStream(HELP_ACTION);

    if (action == PUT_ACTION) {
        if (!sscanf(args[1], "%d", &ival))
            return std::string("cannot scan rx_datastream mode");
        myDet->enableDataStreamingFromReceiver(ival, detPos);
    }

    sprintf(ans, "%d", myDet->enableDataStreamingFromReceiver(-1, detPos));
    return std::string(ans);
}

std::string slsDetectorCommand::helpDataStream(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("rx_datastream \t enables/disables data streaming from receiver. 1 is 0MQ data stream from receiver enabled, while 0 is 0MQ disabled. -1 for inconsistency between multiple receivers. \n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("rx_datastream i\t enables/disables data streaming from receiver. i is 1 enables 0MQ data stream from receiver (creates streamer threads), while 0 disables (destroys streamer threads). \n");
    return os.str();
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

        char hostname[1000];
        strcpy(hostname, "");
        // if each argument is a hostname
        for (int id = 1; id < narg; ++id) {
            strcat(hostname, args[id]);
            if (narg > 2)
                strcat(hostname, "+");
        }
        if (cmd == "hostname") {
            myDet->setHostname(hostname, detPos);
        } 
        else if (cmd == "virtual") {
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
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << std::string("hostname \t returns the hostname(s) of the multi detector structure.\n");
    }
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << std::string("hostname name [name name]\t frees shared memory and "
                          "sets the hostname (or IP adress). Only allowed at multi detector level.\n");
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

std::string slsDetectorCommand::cmdSettingsDir(int narg, const char * const args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif
    if (action == HELP_ACTION) {
        return helpSettingsDir(action);
    }
    if (action == PUT_ACTION) {
        myDet->setSettingsDir(std::string(args[1]), detPos);
    }
    if (myDet->getSettingsDir(detPos) == "")
        return std::string("undefined");
    return myDet->getSettingsDir(detPos);
}

std::string slsDetectorCommand::helpSettingsDir(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("settingsdir \t  gets the directory where the settings files are located\n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("settingsdir dir \t  sets the directory where the settings files are located\n");
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("trimdir \t  obsolete for settingsdir\n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("trimdir dir \t  obsolete for settingsdir\n");
    return os.str();
}

std::string slsDetectorCommand::cmdTrimEn(int narg, const char * const args[], int action, int detPos) {
    std::vector<int> energies;
    if (action == HELP_ACTION) 
        return helpTrimEn(action);

    if (action == PUT_ACTION) {
        energies.reserve(narg-1);
        for(int i=1; i!=narg; ++i){
            energies.push_back(std::stoi(args[i]));
        }
        myDet->setTrimEn(energies, detPos);
        energies.clear();
    }
    energies = myDet->getTrimEn(detPos);
    std::ostringstream os;
    for(const auto& en : energies){
        os << en << ' ';
    }
    return os.str();
}

std::string slsDetectorCommand::helpTrimEn(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << "trimen ne [e0 e1...ene] \t sets the number of energies at which the detector has default trim files" << std::endl;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << "trimen  \t returns the number of energies at which the detector has default trim files and their values" << std::endl;
    return os.str();
}

std::string slsDetectorCommand::cmdOutDir(int narg, const char * const args[], int action, int detPos) {
    if (action == HELP_ACTION)
        return helpOutDir(action);

    else if (action == PUT_ACTION)
        myDet->setFilePath(std::string(args[1]), detPos);

    return std::string(myDet->getFilePath(detPos));
}

std::string slsDetectorCommand::helpOutDir(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("fpath \t  gets the directory where the output files will be written\n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("fpath dir \t  sets the directory where the output files will be written\n");
    return os.str();
}

std::string slsDetectorCommand::cmdFileName(int narg, const char * const args[], int action, int detPos) {
    if (action == HELP_ACTION)
        return helpFileName(action);
    if (cmd == "fname") {
        if (action == PUT_ACTION)
            myDet->setFileName(std::string(args[1]), detPos);

        return std::string(myDet->getFileName(detPos));
    } else if (cmd == "fformat") {
        if (action == PUT_ACTION) {
            if (std::string(args[1]) == "binary")
                myDet->setFileFormat(BINARY, detPos);
            else if (std::string(args[1]) == "hdf5")
                myDet->setFileFormat(HDF5, detPos);
            else
                return std::string("could not scan file format mode\n");
        }
        return myDet->fileFormats(myDet->getFileFormat(detPos));
    }
    return std::string("unknown command") + cmd;
}

std::string slsDetectorCommand::helpFileName(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << std::string("fname \t  gets the filename for the data without index and extension\n");
        os << std::string("fformat \t  gets the file format for data\n");
    }
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << std::string("fname s \t  sets the filename for the data (index and extension will be automatically appended)\n");
        os << std::string("fformat s \t  sets the file format for the data (binary, hdf5)\n");
    }
    return os.str();
}


std::string slsDetectorCommand::cmdRateCorr(int narg, const char * const args[], int action, int detPos) {

    if (action == HELP_ACTION) {
        return helpRateCorr(action);
    }
    int64_t ival;
    char answer[1000];


    if (action == PUT_ACTION) {
        sscanf(args[1], "%ld", &ival);
        myDet->setRateCorrection(ival, detPos);
    }
    sprintf(answer, "%ld", myDet->getRateCorrection(detPos));
    return std::string(answer);
}

std::string slsDetectorCommand::helpRateCorr(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("ratecorr \t  returns the dead time used for rate correections in ns \n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("ratecorr  ns \t  sets the deadtime correction constant in ns, -1 in Eiger will set it to default tau of settings\n");
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

std::string slsDetectorCommand::cmdCounter(int narg, const char * const args[], int action, int detPos) {
    int ival;
    char answer[100];
    std::string sval;
    int retval = FAIL;
    if (action == HELP_ACTION)
        return helpCounter(HELP_ACTION);
    else if (action == PUT_ACTION)
        ival = atoi(args[1]);

    if (std::string(args[0]) == std::string("resmat")) {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &ival))
                return std::string("Could not scan resmat input ") + std::string(args[1]);
            if (ival >= 0)
                sprintf(answer, "%d", myDet->setCounterBit(ival, detPos));
        } else
            sprintf(answer, "%d", myDet->setCounterBit(-1, detPos));
        return std::string(answer);
    }

    if (retval == OK)
        return std::string("Counter set/reset succesfully");
    else
        return std::string("Counter read/reset failed");
}

std::string slsDetectorCommand::helpCounter(int action) {
    std::ostringstream os;
    os << std::endl;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "resmat i \t  sets/resets counter bit in detector" << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "resmat i \t  gets the counter bit in detector" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdNetworkParameter(int narg, const char * const args[], int action, int detPos) {

	char ans[100] = {0};
    int i;
    if (action == HELP_ACTION)
        return helpNetworkParameter(action);

    if (cmd == "rx_hostname") {
    	  if (action == PUT_ACTION) {
    		  myDet->setReceiverHostname(args[1], detPos);
    	  }
    	  return myDet->getReceiverHostname(detPos);
    }  else if (cmd == "rx_udpsocksize") {
        if (action == PUT_ACTION) {
        	int64_t ival = -1;
            if (!(sscanf(args[1], "%ld", &ival))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setReceiverUDPSocketBufferSize(ival, detPos);
        }
        sprintf(ans, "%ld", myDet->getReceiverUDPSocketBufferSize(detPos));
        return ans;
    } else if (cmd == "rx_realudpsocksize") {
        if (action == PUT_ACTION) {
            return ("cannot put!");
        }
        sprintf(ans, "%ld", myDet->getReceiverRealUDPSocketBufferSize(detPos));
        return ans;
    } else if (cmd == "txndelay_left") {
    	networkParameter t = DETECTOR_TXN_DELAY_LEFT;
        if (action == PUT_ACTION) {
            if (!(sscanf(args[1], "%d", &i))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setDetectorNetworkParameter(t, i, detPos);
        }
        sprintf(ans, "%d", myDet->setDetectorNetworkParameter(t, -1, detPos));
        return ans;
    } else if (cmd == "txndelay_right") {
    	networkParameter t = DETECTOR_TXN_DELAY_RIGHT;
        if (action == PUT_ACTION) {
            if (!(sscanf(args[1], "%d", &i))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setDetectorNetworkParameter(t, i, detPos);
        }
        sprintf(ans, "%d", myDet->setDetectorNetworkParameter(t, -1, detPos));
        return ans;
    } else if (cmd == "txndelay_frame") {
    	networkParameter t = DETECTOR_TXN_DELAY_FRAME;
        if (action == PUT_ACTION) {
            if (!(sscanf(args[1], "%d", &i))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setDetectorNetworkParameter(t, i, detPos);
        }
        sprintf(ans, "%d", myDet->setDetectorNetworkParameter(t, -1, detPos));
        return ans;
    } else if (cmd == "flowcontrol_10g") {
    	networkParameter t = FLOW_CONTROL_10G;
        if (action == PUT_ACTION) {
            if (!(sscanf(args[1], "%d", &i))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setDetectorNetworkParameter(t, i, detPos);
        }
        sprintf(ans, "%d", myDet->setDetectorNetworkParameter(t, -1, detPos));
        return ans;
    } else if (cmd == "zmqport") {
        if (action == PUT_ACTION) {
            if (!(sscanf(args[1], "%d", &i))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setClientDataStreamingInPort(i, detPos);
        }
        sprintf(ans, "%d", myDet->getClientStreamingPort(detPos));
        return ans;
    } else if (cmd == "rx_zmqport") {
        if (action == PUT_ACTION) {
            if (!(sscanf(args[1], "%d", &i))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setReceiverDataStreamingOutPort(i, detPos);
        }
        sprintf(ans, "%d", myDet->getReceiverStreamingPort(detPos));
        return ans;
    } else if (cmd == "zmqip") {
    	  if (action == PUT_ACTION) {
    		  myDet->setClientDataStreamingInIP(args[1], detPos);
    	  }
    	  return myDet->getClientStreamingIP(detPos);
    } else if (cmd == "rx_zmqip") {
  	  if (action == PUT_ACTION) {
  		myDet->setReceiverDataStreamingOutIP(args[1], detPos);
  	  }
  	  return myDet->getReceiverStreamingIP(detPos);
    }

    return ("unknown network parameter") + cmd;
}

std::string slsDetectorCommand::helpNetworkParameter(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {

        os << "rx_hostname name \n sets receiver ip/hostname to name" << std::endl;
        os << "txndelay_left port \n sets detector transmission delay of the left port" << std::endl;
        os << "txndelay_right port \n sets detector transmission delay of the right port" << std::endl;
        os << "txndelay_frame port \n sets detector transmission delay of the entire frame" << std::endl;
        os << "flowcontrol_10g port \n sets flow control for 10g for eiger and jungfrau" << std::endl;
        os << "zmqport port \n sets the 0MQ (TCP) port of the client to where final data is streamed to (eg. for GUI). The default already connects with rx_zmqport for the GUI. "
              "Use single-detector command to set individually or multi-detector command to calculate based on port for the rest."
              "Must restart streaming in client with new port from gui/external gui"
           << std::endl;
        os << "rx_zmqport port \n sets the 0MQ (TCP) port of the receiver from where data is streamed from (eg. to GUI or another process for further processing). "
              "Use single-detector command to set individually or multi-detector command to calculate based on port for the rest."
              "Restarts streaming in receiver with new port"
           << std::endl;
        os << "zmqip ip \n sets the 0MQ (TCP) ip of the client to where final data is streamed to (eg. for GUI). Default is ip of rx_hostname and works for GUI. "
              "This is usually used to stream in from an external process."
              "Must restart streaming in client with new port from gui/external gui. "
           << std::endl;
        os << "rx_zmqip ip \n sets/gets the 0MQ (TCP) ip of the receiver from where data is streamed from (eg. to GUI or another process for further processing). "
              "Default is ip of rx_hostname and works for GUI. This is usually used to stream out to an external process for further processing."
              "restarts streaming in receiver with new port"
           << std::endl;
        os << "rx_udpsocksize [t]\n sets the UDP socket buffer size. Different defaults for Jungfrau. "
              "Does not remember in client shared memory, "
              "so must be initialized each time after setting receiver "
              "hostname in config file."
           << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "rx_hostname \n gets receiver ip " << std::endl;
        os << "txndelay_left \n gets detector transmission delay of the left port" << std::endl;
        os << "txndelay_right \n gets detector transmission delay of the right port" << std::endl;
        os << "txndelay_frame \n gets detector transmission delay of the entire frame" << std::endl;
        os << "flowcontrol_10g \n gets flow control for 10g for eiger and jungfrau" << std::endl;
        os << "zmqport \n gets the 0MQ (TCP) port of the client to where final data is streamed to" << std::endl;
        os << "rx_zmqport \n gets the 0MQ (TCP) port of the receiver from where data is streamed from" << std::endl;
        os << "zmqip \n gets the 0MQ (TCP) ip of the client to where final data is streamed to.If no custom ip, empty until first time connect to receiver" << std::endl;
        os << "rx_zmqip \n gets/gets the 0MQ (TCP) ip of the receiver from where data is streamed from. If no custom ip, empty until first time connect to receiver" << std::endl;
        os << "rx_udpsocksize \n gets the UDP socket buffer size." << std::endl;
        os << "rx_realudpsocksize \n gets the actual UDP socket buffer size. Usually double the set udp socket buffer size due to kernel bookkeeping." << std::endl;
    }
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
    } else if (cmd == "rx_tcpport") {
        if (action == PUT_ACTION)
            myDet->setReceiverPort(val, detPos);
        sprintf(ans, "%d", myDet->setReceiverPort(-1, detPos));
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
        os << "rx_tcpport i \n sets the communication receiver port" << std::endl;
        os << "stopport i \n sets the communication stop port " << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "port  \n gets the communication control port" << std::endl;
        os << "rx_tcpport  \n gets the communication receiver port" << std::endl;
        os << "stopport \n gets the communication stop port " << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdLastClient(int narg, const char * const args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpLastClient(action);

    if (action == PUT_ACTION)
        return std::string("cannot set");

    if (cmd == "lastclient") {
        return myDet->getLastClientIP(detPos);
    }

    else if (cmd == "rx_lastclient") {
        return myDet->getReceiverLastClientIP(detPos);
    }

    return std::string("cannot decode command");
}

std::string slsDetectorCommand::helpLastClient(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "lastclient \n returns the last client communicating with the detector" << std::endl;
        os << "rx_lastclient \n returns the last client communicating with the receiver" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdOnline(int narg, const char * const args[], int action, int detPos) {

    if (action == HELP_ACTION) {
        return helpOnline(action);
    }
    int ival;
    char ans[1000];

    if (cmd == "activate") {

        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &ival))
                return std::string("Could not scan activate mode ") + std::string(args[1]);
            myDet->activate(ival, detPos);
            bool padding = true;
            if (narg > 2) {
                if (std::string(args[2]) == "padding")
                    padding = true;
                else if (std::string(args[2]) == "nopadding")
                    padding = false;
                else
                    return std::string("Could not scan activate mode's padding option " + std::string(args[2]));
                myDet->setDeactivatedRxrPaddingMode(padding, detPos);
            }
        }
        int ret = myDet->setDeactivatedRxrPaddingMode(-1, detPos);
        sprintf(ans, "%d %s", myDet->activate(-1, detPos), ret == 1 ? "padding" : (ret == 0 ? "nopadding" : "unknown"));
    } else {
        return std::string("unknown command");
    }

    return ans;
}

std::string slsDetectorCommand::helpOnline(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "activate i [p]\n sets the detector in  activated (1) or deactivated (0) mode (does not send data).  p is optional and can be padding (default) or nonpadding for receivers for deactivated detectors. Only for Eiger." << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "activate \n gets the detector activated (1) or deactivated (0) mode. And padding or nonpadding for the deactivated receiver. Only for Eiger." << std::endl;
    }
    return os.str();
}


std::string slsDetectorCommand::cmdDetectorSize(int narg, const char * const args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpDetectorSize(action);
    int ret, val = -1;
    char ans[1000];


    if (action == PUT_ACTION) {
        if (cmd != "roi" && !sscanf(args[1], "%d", &val))
            return std::string("could not scan ") + std::string(args[0]) + std::string(" ") + std::string(args[1]);

        if (cmd == "clearroi") {             
            myDet->clearROI(detPos);
        }

        if (cmd == "roi") {
            //debug number of arguments
            if (narg != 3)
                return helpDetectorSize(action);
            ROI roi;
            if (!sscanf(args[1], "%d", &roi.xmin))
                return std::string("cannot parse arguments for roi xmin");
            if (!sscanf(args[2], "%d", &roi.xmax))
                return std::string("cannot parse arguments for roi xmax");                   
            myDet->setROI(roi, detPos);
        }

        if (cmd == "detsizechan") {
            int val2 = 0;
            if ((!sscanf(args[1], "%d", &val)) || (narg <= 2) || (!sscanf(args[2], "%d", &val2))) {
                return std::string("Could not scan det size chan values");
            }
            slsDetectorDefs::xy res;
            res.x = val;
            res.y = val2;
            myDet->setNumberOfChannels(res);
        }

		if(cmd=="quad"){
			if (val >=0 ) {
				myDet->setQuad(val);
			}
		}

        if (cmd == "flippeddatax") {
            if ((!sscanf(args[1], "%d", &val)) || (val != 0 && val != 1))
                return std::string("cannot scan flippeddata x mode: must be 0 or 1");

            myDet->setFlippedDataX(val, detPos);
        }

        if (cmd == "gappixels") {
            if ((!sscanf(args[1], "%d", &val)) || (val != 0 && val != 1))
                return std::string("cannot scan gappixels mode: must be 0 or 1");

            if (detPos < 0) // only in multi detector level to update number of channels etc.
                myDet->enableGapPixels(val, detPos);
        }
    }

    if (cmd == "dr") {
        ret = myDet->setDynamicRange(val, detPos);
    } else if (cmd == "clearroi") {
        if (action == GET_ACTION) {
            return std::string("Cannot get");
        }
        return std::string("successful");
    } else if (cmd == "roi") {
        ROI roi = myDet->getROI(detPos);
        return (std::string("[") + std::to_string(roi.xmin) + std::string(",") + std::to_string(roi.xmax) + std::string("]")); 
    } else if (cmd == "detsizechan") {
        slsDetectorDefs::xy res = myDet->getNumberOfChannels();
        sprintf(ans, "%d %d", res.x, res.y);
        return std::string(ans);
    } 	else if (cmd=="quad") {
		return std::to_string(myDet->getQuad());
    } else if (cmd == "flippeddatax") {
        ret = myDet->getFlippedDataX(detPos);
    } else if (cmd == "gappixels") {
        if (detPos >= 0) // only in multi detector level to update number of channels etc.
            return std::string("Cannot execute this command from slsDetector level. Please use multiSlsDetector level.\n");
        ret = myDet->enableGapPixels(-1, detPos);
    }
    

    else
        return std::string("unknown command ") + cmd;

    sprintf(ans, "%d", ret);

    return std::string(ans);
}

std::string slsDetectorCommand::helpDetectorSize(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "dr i \n sets the dynamic range of the detector" << std::endl;
        os << "clearroi \n resets region of interest" << std::endl;
        os << "roi xmin xmax \n sets region of interest " << std::endl;
        os << "detsizechan x y \n sets the maximum number of channels for complete detector set in both directions; 0 is no limit" << std::endl;
 		os << "quad i \n if i = 1, sets the detector size to a quad (Specific to an EIGER quad hardware). 0 by default."<< std::endl;       
        os << "flippeddatax x \n sets if the data should be flipped on the x axis" << std::endl;
        os << "gappixels i \n enables/disables gap pixels in system (detector & receiver). 1 sets, 0 unsets. Used in EIGER only and multidetector level." << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "dr \n gets the dynamic range of the detector" << std::endl;
        os << "roi \n gets region of interest" << std::endl;
        os << "detsizechan \n gets the maximum number of channels for complete detector set in both directions; 0 is no limit" << std::endl;
        os << "quad \n returns 1 if the detector size is a quad (Specific to an EIGER quad hardware). 0 by default."<< std::endl;
        os << "flippeddatax\n gets if the data will be flipped on the x axis" << std::endl;
        os << "gappixels\n gets if gap pixels is enabled in system. Used in EIGER only and multidetector level." << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdSettings(int narg, const char * const args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpSettings(action);
    int val = -1; //ret,
    char ans[1000];

    //  portType index;
    //     if (sscanf(args[1],"%d",&val))
    //       ;
    //     else
    //       return std::string("could not scan port number")+std::string (args[1]);
    //   }


    if (cmd == "settings") {
        detectorSettings sett = GET_SETTINGS;
        if (action == PUT_ACTION) {
            sett = myDet->getDetectorSettings(std::string(args[1]));
            if (sett == -1)
                return std::string("unknown settings scanned " + std::string(args[1]));
            sett = myDet->setSettings(sett, detPos);
            if (myDet->getDetectorTypeAsEnum(detPos) == EIGER) {
                return myDet->getDetectorSettings(sett);
            }
        }
        return myDet->getDetectorSettings(myDet->getSettings(detPos));
    } else if (cmd == "threshold") {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &val)) {
                return std::string("invalid threshold value");
            }
            detectorType type = myDet->getDetectorTypeAsEnum(detPos);
            if (type != EIGER || (type == EIGER && narg <= 2)) {
                myDet->setThresholdEnergy(val, GET_SETTINGS, 1, detPos);
            } else {
                detectorSettings sett = myDet->getDetectorSettings(std::string(args[2]));
                if (sett == -1)
                    return std::string("invalid settings value");
                myDet->setThresholdEnergy(val, sett, 1, detPos);
            }
        }
        sprintf(ans, "%d", myDet->getThresholdEnergy(detPos));
        return std::string(ans);
    } else if (cmd == "thresholdnotb") {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &val)) {
                return std::string("invalid threshold value");
            }
            detectorType type = myDet->getDetectorTypeAsEnum(detPos);
            if (type != EIGER)
                return std::string("not implemented for this detector");
            if (narg <= 2) {
                myDet->setThresholdEnergy(val, GET_SETTINGS, 0, detPos);
            } else {
                detectorSettings sett = myDet->getDetectorSettings(std::string(args[2]));
                if (sett == -1)
                    return std::string("invalid settings value");
                myDet->setThresholdEnergy(val, sett, 0, detPos);
            }
        }
        sprintf(ans, "%d", myDet->getThresholdEnergy(detPos));
        return std::string(ans);
    } else if (cmd == "trimbits") {
        if (narg >= 2) {
            std::string sval = std::string(args[1]);
#ifdef VERBOSE
            std::cout << " trimfile " << sval << std::endl;
#endif
            if (action == GET_ACTION) {
                //create file names
                myDet->saveSettingsFile(sval, detPos);
            } else if (action == PUT_ACTION) {
                myDet->loadSettingsFile(sval, detPos);
            }
            return sval;
        }
        return std::string("Specify file name for geting settings file");
    } else if (cmd == "trimval") {
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
        os << "settings s \n sets the settings of the detector - can be standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain"
              "dynamichg0,fixgain1,fixgain2,forceswitchg1, forceswitchg2"
           << std::endl;
        os << "threshold eV [sett]\n sets the detector threshold in eV. If sett is provided for eiger, uses settings sett" << std::endl;
        os << "thresholdnotb eV [sett]\n sets the detector threshold in eV without loading trimbits. If sett is provided for eiger, uses settings sett" << std::endl;
        os << "trimbits fname\n loads the trimfile fname to the detector. If no extension is specified, the serial number of each module will be attached." << std::endl;
        os << "trimval i \n sets all the trimbits to i" << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "settings \n gets the settings of the detector" << std::endl;
        os << "threshold V\n gets the detector threshold" << std::endl;
        os << "thresholdnotb V\n gets the detector threshold" << std::endl;
        os << "trimbits [fname]\n returns the trimfile loaded on the detector. If fname is specified the trimbits are saved to file. If no extension is specified, the serial number of each module will be attached." << std::endl;
        os << "trimval \n returns the value all trimbits are set to. If they are different, returns -1." << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdSN(int narg, const char * const args[], int action, int detPos) {

    char answer[1000];

    if (action == PUT_ACTION)
        return std::string("cannot set");

    if (action == HELP_ACTION)
        return helpSN(action);

    if (cmd == "thisversion") {
        int64_t retval = myDet->getClientSoftwareVersion();
        if (retval < 0)
            sprintf(answer, "%d", -1);
        else
            sprintf(answer, "0x%lx", retval);
        return std::string(answer);
    }


    if (cmd == "detectornumber") {
        int64_t retval = myDet->getId(DETECTOR_SERIAL_NUMBER, detPos);
        if (retval < 0)
            sprintf(answer, "%d", -1);
        else
            sprintf(answer, "0x%lx", retval);
        return std::string(answer);
    }

    if (cmd == "detectorversion") {
        int64_t retval = myDet->getId(DETECTOR_FIRMWARE_VERSION, detPos);
        if (retval < 0)
            sprintf(answer, "%d", -1);
        else
            sprintf(answer, "0x%lx", retval);
        return std::string(answer);
    }

    if (cmd == "softwareversion") {
        int64_t retval = myDet->getId(DETECTOR_SOFTWARE_VERSION, detPos);
        if (retval < 0)
            sprintf(answer, "%d", -1);
        else
            sprintf(answer, "0x%lx", retval);
        return std::string(answer);
    }

    if (cmd == "rx_version") {
        int64_t retval = myDet->getReceiverSoftwareVersion(detPos);
        if (retval < 0)
            sprintf(answer, "%d", -1);
        else
            sprintf(answer, "0x%lx", retval);
        return std::string(answer);
    }

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
        os << "detectornumber \n gets the serial number of the detector (MAC)" << std::endl;
        os << "detectorversion \n gets the firmware version of the detector" << std::endl;
        os << "softwareversion \n gets the software version of the detector" << std::endl;
        os << "thisversion \n gets the version of this software" << std::endl;
        os << "rx_version \n gets the version of the receiver" << std::endl;
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

    else if (cmd == "imagetest") {
        if (action == PUT_ACTION) {
            int ival = -1;
            if (!sscanf(args[1], "%d", &ival)) {
                return std::string("could not scan parameter for imagetest\n");
            }
            ival = (ival == 0) ? 0 : 1;
            myDet->digitalTest(IMAGE_TEST, ival, detPos);
        }
        
        return std::to_string(myDet->digitalTest(IMAGE_TEST, -1, detPos));
    }

    return std::string("unknown test mode ") + cmd;
}

std::string slsDetectorCommand::helpDigiTest(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "imagetest i \t If 1, adds channel intensity with precalculated values. Default is 0. Gotthard only." << std::endl;
        os << "bustest \t performs test of the bus interface between FPGA and embedded Linux system. Can last up to a few minutes. Jungfrau only." << std::endl;
        os << "firmwaretest \t performs the firmware test. Jungfrau only." << std::endl;
    }
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "imagetest i \t If 1, adds channel intensity with precalculated values. Default is 0. Gotthard only." << std::endl;
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
    else if (cmd == "adcvpp")
        dac = ADC_VPP;
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
    else if (cmd == "vhighvoltage")
        dac = HIGH_VOLTAGE;
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
    else if (cmd == "v_a") {
        dac = V_POWER_A;
        mode = 1;
    } else if (cmd == "v_b") {
        dac = V_POWER_B;
        mode = 1;
    } else if (cmd == "v_c") {
        dac = V_POWER_C;
        mode = 1;
    } else if (cmd == "v_d") {
        dac = V_POWER_D;
        mode = 1;
    } else if (cmd == "v_io") {
        dac = V_POWER_IO;
        mode = 1;
   } else if (cmd == "v_chip") {
        dac = V_POWER_CHIP;
        mode = 1;
    } else if (cmd == "v_limit")
        dac = V_LIMIT;
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
    else if (cmd == "vipreout")
        dac = M_vIpreOut;
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
        os << "vhighvoltage "
           << "dacu\t CHIPTEST BOARD ONLY - sets the detector HV in dac units (0-1024)." << std::endl;
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
        os << "vhighvoltage "
           << "dacu\t CHIPTEST BOARD ONLY - gets the detector HV in dac units (0-1024)." << std::endl;
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

std::string slsDetectorCommand::cmdADC(int narg, const char * const args[], int action, int detPos) {

    dacIndex adc;
    char answer[1000];

    if (action == HELP_ACTION)
        return helpADC(action);
    else if (action == PUT_ACTION)
        return std::string("cannot set ") + cmd;

    if (cmd == "adc") {
        int idac = -1;
        if (sscanf(args[1], "%d", &idac) != 1) {
            return std::string("Could not scan adc index") + std::string(args[1]);
        }
        if (idac < 0 || idac > SLOW_ADC_TEMP - SLOW_ADC0) {
            return (std::string ("cannot set adc, must be between ") + std::to_string(0) +
                    std::string (" and ") + std::to_string(SLOW_ADC_TEMP - SLOW_ADC0));
        }
        adc = (dacIndex)(idac + SLOW_ADC0);
    }
	else if (cmd=="temp_adc")
		adc=TEMPERATURE_ADC;
	else if (cmd=="temp_fpga")
		adc=TEMPERATURE_FPGA;
	else if (cmd=="temp_fpgaext")
		adc=TEMPERATURE_FPGAEXT;
	else if (cmd=="temp_10ge")
		adc=TEMPERATURE_10GE;
	else if (cmd=="temp_dcdc")
		adc=TEMPERATURE_DCDC;
	else if (cmd=="temp_sodl")
		adc=TEMPERATURE_SODL;
	else if (cmd=="temp_sodr")
		adc=TEMPERATURE_SODR;
	else if (cmd=="temp_fpgafl")
		adc=TEMPERATURE_FPGA2;
	else if (cmd=="temp_fpgafr")
		adc=TEMPERATURE_FPGA3;
	else if (cmd=="i_a")
		adc=I_POWER_A;
	else if (cmd=="i_b")
		adc=I_POWER_B;
	else if (cmd=="i_c")
		adc=I_POWER_C;
	else if (cmd=="i_d")
		adc=I_POWER_D;
	else if (cmd=="vm_a")
		adc=V_POWER_A;
	else if (cmd=="vm_b")
		adc=V_POWER_B;
	else if (cmd=="vm_c")
		adc=V_POWER_C;
	else if (cmd=="vm_d")
		adc=V_POWER_D;
	else if (cmd=="vm_io")
		adc=V_POWER_IO;
	else if (cmd=="i_io")
		adc=I_POWER_IO;
	else
		return std::string("cannot decode adc ")+cmd;

	if (myDet->getDetectorTypeAsEnum(detPos) == EIGER || myDet->getDetectorTypeAsEnum(detPos) == JUNGFRAU){
		int val = myDet->getADC(adc, detPos);
		if (val == -1)
			sprintf(answer,"%d",val);
		else
			sprintf(answer,"%.2f", (double)val/1000.000);
	}
	else sprintf(answer,"%d",myDet->getADC(adc, detPos));

	//if ((adc == TEMPERATURE_ADC) || (adc == TEMPERATURE_FPGA))
	if (adc < 100 || adc == SLOW_ADC_TEMP)
		strcat(answer,"°C");
	else if (adc == I_POWER_A || adc == I_POWER_B || adc == I_POWER_C || adc == I_POWER_D || adc == I_POWER_IO)
	    strcat(answer," mA");
	else
		strcat(answer," mV");

	return std::string(answer);

}

std::string slsDetectorCommand::helpADC(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "temp_adc "
           << "Cannot be set" << std::endl;
        os << "temp_fpga "
           << "Cannot be set" << std::endl;
        os << "temp_fpgaext "
           << "Cannot be set" << std::endl;
        os << "temp_10ge "
           << "Cannot be set" << std::endl;
        os << "temp_dcdc "
           << "Cannot be set" << std::endl;
        os << "temp_sodl "
           << "Cannot be set" << std::endl;
        os << "temp_sodr "
           << "Cannot be set" << std::endl;
        os << "temp_fpgafl "
           << "Cannot be set" << std::endl;
        os << "temp_fpgafr "
           << "Cannot be set" << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "temp_adc "
           << "\t gets the temperature of the adc" << std::endl;
        os << "temp_fpga "
           << "\t gets the temperature of the fpga" << std::endl;
        os << "temp_fpgaext "
           << "\t gets the temperature close to the fpga" << std::endl;
        os << "temp_10ge "
           << "\t gets the temperature close to the 10GE" << std::endl;
        os << "temp_dcdc "
           << "\t gets the temperature close to the dc dc converter" << std::endl;
        os << "temp_sodl "
           << "\t gets the temperature close to the left so-dimm memory" << std::endl;
        os << "temp_sodr "
           << "\t gets the temperature close to the right so-dimm memory" << std::endl;
        os << "temp_fpgafl "
           << "\t gets the temperature of the left front end board fpga" << std::endl;
        os << "temp_fpgafr "
           << "\t gets the temperature of the left front end board fpga" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdTempControl(int narg, const char * const args[], int action, int detPos) {
    char answer[1000] = "";
    int val = -1;

    if (action == HELP_ACTION)
        return helpTempControl(action);


    if (cmd == "temp_threshold") {
        if (action == PUT_ACTION) {
            double fval = 0.0;
            if (!sscanf(args[1], "%lf", &fval))
                return std::string("cannot scan temp control value ") + std::string(args[1]);
            val = fval * 1000;
            myDet->setThresholdTemperature(val, detPos);
        }
        val = myDet->setThresholdTemperature(-1, detPos);
        if (val == -1)
            sprintf(answer, "%d", val);
        else
            sprintf(answer, "%.2f°C", (double)val / 1000.000);
    }

    else if (cmd == "temp_control") {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &val))
                return std::string("cannot scan temp control value ") + std::string(args[1]);
            if ((val != 0) && (val != 1))
                return std::string("temp_control option must be 0 or 1");
            myDet->setTemperatureControl(val, detPos);
        }
        sprintf(answer, "%d", myDet->setTemperatureControl(-1, detPos));
    }

    else if (cmd == "temp_event") {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &val))
                return std::string("cannot scan temp control value ") + std::string(args[1]);
            if (val != 0)
                return std::string("temp_event option must be 0 to clear event");
            myDet->setTemperatureEvent(val, detPos);
        }
        sprintf(answer, "%d", myDet->setTemperatureEvent(-1, detPos));
    }

    else
        return std::string("cannot scan command " + cmd);

    return std::string(answer);
}

std::string slsDetectorCommand::helpTempControl(int action) {
    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "temp_threshold t \t sets the threshold temperature. Jungfrau only" << std::endl;
        os << "temp_control t \t Enables/Disables the temperature control. 1 enables, 0 disables. JUNGFRAU ONLY" << std::endl;
        os << "temp_event t \t Resets over-temperative event. Put only with option 0 to clear event. JUNGFRAU ONLY." << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "temp_threshold  \t gets the threshold temperature. Jungfrau only." << std::endl;
        os << "temp_control  \t gets temperature control enable. 1 enabled, 0 disabled. JUNGFRAU ONLY" << std::endl;
        os << "temp_event  \t gets over-temperative event. Gets 1 if temperature went over threshold and control is enabled, else 0. /Disables the temperature control. JUNGFRAU ONLY." << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdTiming(int narg, const char * const args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif

    if (action == HELP_ACTION) {
        return helpTiming(HELP_ACTION);
    }
    if (action == PUT_ACTION) {
        if (myDet->timingModeType(std::string(args[1])) == GET_TIMING_MODE)
            return helpTiming(action);
        myDet->setTimingMode(myDet->timingModeType(std::string(args[1])), detPos);
    }
    return myDet->timingModeType(myDet->setTimingMode(GET_TIMING_MODE, detPos));
}
std::string slsDetectorCommand::helpTiming(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("timing \t gets the timing mode of the detector (auto, trigger, ro_trigger, gating, triggered_gating)\n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("timing mode \t sets synchronization mode of the detector. Can be auto, trigger, ro_trigger, gating, triggered_gating \n");
    return os.str();
}

std::string slsDetectorCommand::cmdTimer(int narg, const char * const args[], int action, int detPos) {
    timerIndex index;
    int64_t t = -1, ret;
    double val, rval;

    char answer[1000];

    if (action == HELP_ACTION)
        return helpTimer(action);

    if (cmd == "exptime")
        index = ACQUISITION_TIME;
    else if (cmd == "subexptime")
        index = SUBFRAME_ACQUISITION_TIME;
    else if (cmd == "period")
        index = FRAME_PERIOD;
    else if (cmd == "subdeadtime")
        index = SUBFRAME_DEADTIME;
    else if (cmd == "delay")
        index = DELAY_AFTER_TRIGGER;
    else if (cmd == "cycles")
        index = CYCLES_NUMBER;
    // also does digital sample
    else if (cmd == "samples") 
        index = ANALOG_SAMPLES; 
    else if (cmd == "asamples")
        index = ANALOG_SAMPLES;
    else if (cmd == "dsamples")
        index = DIGITAL_SAMPLES;
    else if (cmd == "storagecells")
        index = STORAGE_CELL_NUMBER;
    else if (cmd == "storagecell_delay")
        index = STORAGE_CELL_DELAY;
    else if (cmd == "storagecell_start") {
        if (action == PUT_ACTION) {
            int ival = -1;
            if (!sscanf(args[1], "%d", &ival))
                return std::string("cannot scan storage cell start value ") + std::string(args[1]);
            myDet->setStoragecellStart(ival, detPos);
        }
        sprintf(answer, "%d", myDet->setStoragecellStart(-1, detPos));
        return std::string(answer);
    } else if (cmd == "startingfnum") {
        if (action == PUT_ACTION) {
            uint64_t ival = -1;
            if (!sscanf(args[1], "%lu", &ival))
                return std::string("cannot scan starting frame number value ") + std::string(args[1]);
            myDet->setStartingFrameNumber(ival, detPos);
            return std::string(args[1]);
        }
        return std::to_string(myDet->getStartingFrameNumber(detPos));
    } else
        return std::string("could not decode timer ") + cmd;

    if (action == PUT_ACTION) {
        if (sscanf(args[1], "%lf", &val))
            ; //printf("value:%0.9lf\n",val);
        else
            return std::string("cannot scan timer value ") + std::string(args[1]);

        // timer
        if (index == ACQUISITION_TIME || index == SUBFRAME_ACQUISITION_TIME ||
            index == FRAME_PERIOD || index == DELAY_AFTER_TRIGGER ||
            index == SUBFRAME_DEADTIME || index == STORAGE_CELL_DELAY) {
            t = lround(val * 1E9);
        } else
            t = static_cast<int64_t>(val);
    }


    ret = myDet->setTimer(index, t, detPos);

    // samples command does both asamples and dsamples
    if (cmd == "samples" ) {
        int64_t dret = myDet->setTimer(DIGITAL_SAMPLES, t, detPos);
        if (dret != ret) {
            throw sls::RuntimeError("Analog and digital number of samples are different. Check with asamples and dsamples command");
        }
    }

    if ((ret != -1) && (index == ACQUISITION_TIME || index == SUBFRAME_ACQUISITION_TIME ||
    		index == FRAME_PERIOD || index == DELAY_AFTER_TRIGGER ||
                        index == SUBFRAME_DEADTIME || index == STORAGE_CELL_DELAY)) {
        rval = (double)ret * 1E-9;
        sprintf(answer, "%0.9f", rval);
    } else
        sprintf(answer, "%lld", (long long int)ret);

    return std::string(answer);
}

std::string slsDetectorCommand::helpTimer(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "exptime t \t sets the exposure time in s" << std::endl;
        os << "subexptime t \t sets the exposure time of subframe in s" << std::endl;
        os << "period t \t sets the frame period in s" << std::endl;
        os << "delay t \t sets the delay after trigger in s" << std::endl;
        os << "frames t \t sets the number of frames per cycle (e.g. after each trigger)" << std::endl;
        os << "startingfnum t \t sets starting frame number for the next acquisition. Only for Jungfrau and Eiger." << std::endl;
        os << "cycles t \t sets the number of cycles (e.g. number of triggers)" << std::endl;
        os << "samples t \t sets the number of samples (both analog and digital) expected from the ctb" << std::endl;
        os << "asamples t \t sets the number of analog samples expected from the ctb" << std::endl;
        os << "dsamples t \t sets the number of digital samples expected from the ctb" << std::endl;
        os << "storagecells t \t sets number of storage cells per acquisition. For very advanced users only! For JUNGFRAU only. Range: 0-15. The #images = #frames * #cycles * (#storagecells+1)." << std::endl;
        os << "storagecell_start t \t sets the storage cell that stores the first acquisition of the series. Default is 15(0xf). For very advanced users only! For JUNGFRAU only. Range: 0-15." << std::endl;
        os << "storagecell_delay t \t sets additional time to t between 2 storage cells. For very advanced users only! For JUNGFRAU only. Range: 0-1638375 ns (resolution of 25ns).. " << std::endl;
        os << "subdeadtime t \t sets sub frame dead time in s. Subperiod is set in the detector = subexptime + subdeadtime. This value is normally a constant in the config file. Used in EIGER only in 32 bit mode. " << std::endl;
        os << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {

        os << "exptime  \t gets the exposure time in s" << std::endl;
        os << "subexptime  \t gets the exposure time of subframe in s" << std::endl;
        os << "period  \t gets the frame period in s" << std::endl;
        os << "delay  \t gets the delay after trigger in s" << std::endl;
        os << "frames  \t gets the number of frames per cycle (e.g. after each trigger)" << std::endl;
        os << "startingfnum \t gets starting frame number for the next acquisition. Only for Jungfrau and Eiger." << std::endl;
        os << "cycles  \t gets the number of cycles (e.g. number of triggers)" << std::endl;
        os << "samples \t gets the number of samples (both analog and digital) expected from the ctb" << std::endl;
        os << "asamples \t gets the number of analog samples expected from the ctb" << std::endl;
        os << "dsamples \t gets the number of digital samples expected from the ctb" << std::endl;
        os << "storagecells \t gets number of storage cells per acquisition.For JUNGFRAU only." << std::endl;
        os << "storagecell_start \t gets the storage cell that stores the first acquisition of the series." << std::endl;
        os << "storagecell_delay \tgets additional time between 2 storage cells. " << std::endl;
        os << "subperiod \t gets sub frame dead time in s. Used in EIGER in 32 bit only." << std::endl;
        os << std::endl;
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

    if (cmd == "exptimel")
        index = ACQUISITION_TIME;
    else if (cmd == "periodl")
        index = FRAME_PERIOD;
    else if (cmd == "delayl")
        index = DELAY_AFTER_TRIGGER;
    else if (cmd == "framesl")
        index = FRAME_NUMBER;
    else if (cmd == "cyclesl")
        index = CYCLES_NUMBER;
    else if (cmd == "now")
        index = ACTUAL_TIME;
    else if (cmd == "timestamp")
        index = MEASUREMENT_TIME;
    else if (cmd == "nframes")
        index = FRAMES_FROM_START;
    else if (cmd == "measuredperiod")
        index = MEASURED_PERIOD;
    else if (cmd == "measuredsubperiod")
        index = MEASURED_SUBPERIOD;
    else
        return std::string("could not decode timer ") + cmd;

    if (action == PUT_ACTION) {
        return std::string("cannot set ") + std::string(args[1]);
    }


    ret = myDet->getTimeLeft(index, detPos);

    if ((ret != -1) && (index == ACQUISITION_TIME || index == FRAME_PERIOD || index == DELAY_AFTER_TRIGGER || index == ACTUAL_TIME || index == MEASUREMENT_TIME ||
                        index == MEASURED_PERIOD || index == MEASURED_SUBPERIOD)) {
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

        os << "exptimel  \t gets the exposure time left" << std::endl;
        os << "periodl \t gets the frame period left" << std::endl;
        os << "delayl  \t gets the delay left" << std::endl;
        os << "framesl  \t gets the number of frames left" << std::endl;
        os << "cyclesl  \t gets the number of cycles left" << std::endl;
        os << "measuredperiod \t gets the measured frame period (time between last frame and the previous one) in s. For Eiger only. Makes sense only for acquisitions of more than 1 frame." << std::endl;
        os << "measuredsubperiod \t gets the measured subframe period (time between last subframe and the previous one) in s. For Eiger only and in 32 bit mode." << std::endl;
        os << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdSpeed(int narg, const char * const args[], int action, int detPos) {

    speedVariable index;
    int t = -1, ret = 0, mode = 0;

    char answer[1000];

    if (action == HELP_ACTION)
    	return helpSpeed(action);

    if (cmd == "clkdivider") {
    	index = CLOCK_DIVIDER;
    }
    else if (cmd == "adcclk") {
    	index = ADC_CLOCK;
    }
    else if (cmd == "dbitclk") {
    	index = DBIT_CLOCK;
    }
    else if (cmd == "syncclk") {
    	index = SYNC_CLOCK;
        if (action == PUT_ACTION) {
    	    return std::string("cannot put");
    	}    
    }
    else if (cmd == "adcphase") {
    	index = ADC_PHASE;
    	if ((action == PUT_ACTION && narg > 2 && std::string(args[2]) == "deg") ||
    			(action == GET_ACTION && narg > 1 && std::string(args[1]) == "deg")) {
    		mode = 1;
    	}
    }
    else if (cmd == "dbitphase") {
    	index = DBIT_PHASE;
    	if ((action == PUT_ACTION && narg > 2 && std::string(args[2]) == "deg") ||
    			(action == GET_ACTION && narg > 1 && std::string(args[1]) == "deg")) {
    		mode = 1;
    	}
    }
    else if (cmd == "maxadcphaseshift") {
    	index = MAX_ADC_PHASE_SHIFT;
    	 if (action == PUT_ACTION) {
    		 return std::string("cannot put");
    	 }
    }
    else if (cmd == "maxdbitphaseshift") {
    	index = MAX_DBIT_PHASE_SHIFT;
   	 if (action == PUT_ACTION) {
   		 return std::string("cannot put");
   	 }
    }
    else if (cmd == "adcpipeline") {
    	index = ADC_PIPELINE;
    }
    else if (cmd == "dbitpipeline") {
    	index = DBIT_PIPELINE;
    }
    else
        return std::string("could not decode speed variable ") + cmd;

    if (action == PUT_ACTION) {
        if (sscanf(args[1], "%d", &t))
            ;
        else {
            // if parameer is a string (unknown speed will throw)
            if (cmd == "clkdivider") {
                speedLevel lev = getSpeedLevelType(std::string(args[1]));
                t = static_cast<int>(lev);
            } 
            return std::string("cannot scan speed value ") + std::string(args[1]);
        }
    }


    ret = myDet->setSpeed(index, t, mode, detPos);

    sprintf(answer, "%d", ret);
    return std::string(answer);
}

std::string slsDetectorCommand::helpSpeed(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "clkdivider c  \t sets readout clock divider. EIGER, JUNGFRAU [0(fast speed), 1(half speed), 2(quarter speed)].  Jungfrau, full speed is not implemented and overwrites adcphase to recommended default. Not for Gotthard." << std::endl;
        os << "adcclk c \tSets ADC clock frequency in MHz. CTB & Moench only. " << std::endl;
        os << "dbitclk c \tSets the clock frequency of the latching of the digital bits in MHz. CTB & Moench only. " << std::endl;
        os << "adcphase c [deg]\t Sets phase of the ADC clock to i. i is the shift or in degrees if deg is used. deg is optional & only for CTB, Moench & Jungfrau. For CTB & Moench, adcphase is reset if adcclk is changed. For Jungfrau, adcphase changed to defaults if clkdivider changed. Jungfrau, CTB & Moench, these are absolute values with limits. Gotthard, relative phase shift. Not for Eiger." << std::endl;
        os << "dbitphase c [deg]\t Sets the phase of the clock for latching of the digital bits to i. i is the shift or in degrees if deg is used. deg is optional. dbitphase is also reset if dbitclk is changed. These are absolute values with limits. for CTB & Moench only." << std::endl;
        os << "adcpipeline c \t Sets the pipeline of the ADC. For CTB & Moench only." << std::endl;
        os << "dbitpipeline c \t Sets the pipeline of the latching of the digital bits. For CTB & Moench only." << std::endl;
        os << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "clkdivider \t Gets readout clock divider. EIGER, JUNGFRAU [0(fast speed), 1(half speed), 2(quarter speed)].  Jungfrau, full speed is not implemented and overwrites adcphase to recommended default. Not for Gotthard." << std::endl;
        os << "adcclk \tGets ADC clock frequency in MHz. CTB & Moench only. " << std::endl;
        os << "dbitclk \tGets the clock frequency of the latching of the digital bits in MHz. CTB & Moench only. " << std::endl;
        os << "syncclk \t Gets the clock frequency of the sync clock in MHz. CTB & Moench only." << std::endl;
        os << "adcphase [deg]\t Gets phase of the ADC clock. unit is number of shifts or in degrees if deg is used. deg is optional & only for CTB, Moench & Jungfrau. For CTB & Moench, adcphase is reset if adcclk is changed. For Jungfrau, adcphase changed to defaults if clkdivider changed. Jungfrau, CTB & Moench, these are absolute values with limits. Gotthard, relative phase shift. Not for Eiger." << std::endl;
        os << "dbitphase [deg]\t Gets the phase of the clock for  latching of the digital bits. unit is number of shifts or in degrees if deg is used. deg is optional. dbitphase is also reset if dbitclk is changed. These are absolute values with limits. for CTB & Moench only." << std::endl;
        os << "adcpipeline \t Gets the pipeline of the ADC. For CTB & Moench only." << std::endl;
        os << "dbitpipeline \t Gets the pipeline of the latching of the digital bits. For CTB & Moench only." << std::endl;
        os << "maxadcphaseshift \t Gets maximum phase shift of the ADC clock. CTB,Moench and Jungfrau only." << std::endl;
        os << "maxdbitphaseshift \t Gets maximum phase shift of the clock for latching of the digital bits. CTB & Moench only." << std::endl;
        os << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdAdvanced(int narg, const char * const args[], int action, int detPos) {

    
    if (action == HELP_ACTION)
        return helpAdvanced(action);

    if (cmd == "romode") {
        if (action == PUT_ACTION) {
            myDet->setReadoutMode(getReadoutModeType(std::string(args[1])), detPos);
        }
        return getReadoutModeType(myDet->getReadoutMode());
    }

    else if (cmd=="interruptsubframe") {
		if (action==PUT_ACTION) {
			int ival = -1;
			if (!sscanf(args[1],"%d",&ival))
				return std::string("could not scan interrupt sub frame parameter ") + std::string(args[1]);
			myDet->setInterruptSubframe(ival > 0 ? true : false);
		}
		return std::to_string(myDet->getInterruptSubframe());
        
	}  
    
    else if (cmd == "readnlines") {
        if (action == PUT_ACTION) {
                int ival = -1;
                if (!sscanf(args[1],"%d",&ival))
                    return std::string("could not scan readnlines parameter ") + std::string(args[1]);
                myDet->setReadNLines(ival);
            }
            return std::to_string(myDet->getReadNLines());

    } 
    
    else if (cmd == "extsig") {
        externalSignalFlag flag = GET_EXTERNAL_SIGNAL_FLAG;

        if (action == PUT_ACTION) {
            flag = myDet->externalSignalType(args[1]);
            if (flag == GET_EXTERNAL_SIGNAL_FLAG)
                return std::string("could not scan external signal mode ") + std::string(args[1]);
        }
    
        return myDet->externalSignalType(myDet->setExternalSignalFlags(flag, detPos));

    } 
    
    else if (cmd == "programfpga") {
        if (action == GET_ACTION)
            return std::string("cannot get");
        if (strstr(args[1], ".pof") == nullptr)
            return std::string("wrong usage: programming file should have .pof extension");
        std::string sval = std::string(args[1]);
        myDet->programFPGA(sval, detPos);
        return std::string("successful");
    }

    else if (cmd == "resetfpga") {
        if (action == GET_ACTION)
            return std::string("cannot get");
        myDet->resetFPGA(detPos);
        return std::string("successful");
    }

    else if (cmd == "copydetectorserver") {
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

    else if (cmd == "powerchip") {
        char ans[100];
        if (action == PUT_ACTION) {
            int ival = -1;
            if (!sscanf(args[1], "%d", &ival))
                return std::string("could not scan powerchip parameter " + std::string(args[1]));
            myDet->powerChip(ival, detPos);
        }
        sprintf(ans, "%d", myDet->powerChip(-1, detPos));
        return std::string(ans);
    }

    else if (cmd == "led") {
        if (action == PUT_ACTION) {
            int ival = -1;
            if (!sscanf(args[1], "%d", &ival))
                return std::string("could not scan led parameter " + std::string(args[1]));
            myDet->setLEDEnable(ival, detPos);
        }
        return std::to_string(myDet->setLEDEnable(-1, detPos));
    }

    else if (cmd == "diodelay") {
    	if (action == GET_ACTION) {
    		return std::string("Cannot get");
    	}


    	uint64_t pinMask = -1;
    	if (!sscanf(args[1], "%lx", &pinMask))
    		return std::string("could not scan diodelay pin mask(in hex) " + std::string(args[1]));
    	int delay = -1;
    	if (!sscanf(args[2], "%d", &delay))
    		return std::string("could not scan diodelay delay " + std::string(args[2]));

    	myDet->setDigitalIODelay(pinMask, delay, detPos);
      	return std::string("successful");
    }

    else if (cmd == "auto_comp_disable") {
        char ans[100];
        if (action == PUT_ACTION) {
            int ival = -1;
            if (!sscanf(args[1], "%d", &ival))
                return std::string("could not scan auto_comp_control parameter " + std::string(args[1]));
            myDet->setAutoComparatorDisableMode(ival, detPos);
        }
        sprintf(ans, "%d", myDet->setAutoComparatorDisableMode(-1, detPos));
        return std::string(ans);
    } else
        return std::string("unknown command ") + cmd;
}

std::string slsDetectorCommand::helpAdvanced(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {

        os << "extsig mode \t sets the mode of the external signal. can be trigger_out_rising_edge, trigger_out_falling_edge. Gotthard only" << std::endl;
        os << "romode m \t sets the readout flag to m. Options: analog, digital, analog_digital. Used for CTB only." << std::endl;
        os << "interruptsubframe flag \t sets the interrupt subframe flag. Setting it to 1 will interrupt the last subframe at the required exposure time. By default, this is disabled and set to 0, ie. it will wait for the last sub frame to finish exposing. Used for EIGER  in 32 bit mode only." << std::endl;
        os << "readnlines f \t sets the number of rows to read out per half module. Options: 1 - 256 (Not all values as it depends on dynamic range and 10GbE enabled). Used for EIGER only. " << std::endl;
        os << "programfpga f \t programs the fpga with file f (with .pof extension)." << std::endl;
        os << "resetfpga f \t resets fpga, f can be any value" << std::endl;
        os << "copydetectorserver s p \t copies the detector server s via tftp from pc with hostname p and changes respawn server. Not for Eiger. " << std::endl;
        os << "rebootcontroller \t reboot controler blackfin of the detector. Not for Eiger." << std::endl;
        os << "update s p f \t updates the firmware to f and detector server to f from host p via tftp and then reboots controller (blackfin). Not for Eiger. " << std::endl;
        os << "led s \t sets led status (0 off, 1 on)" << std::endl;
        os << "diodelay m v \tsets the delay for the digital IO pins selected by mask m and delay set by v. mask is upto 64 bits in hex, delay max is 775ps, and set in steps of 25 ps. Used for MOENCH/CTB only." << std::endl;
        os << "powerchip i \t powers on or off the chip. i = 1 for on, i = 0 for off" << std::endl;
        os << "auto_comp_disable i \t this mode disables the on-chip gain switching comparator automatically after 93.75% of exposure time (only for longer than 100us). 1 enables mode, 0 disables mode. By default, mode is disabled (comparator is enabled throughout). (JUNGFRAU only). " << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {

        os << "extsig \t gets the mode of the external signal. can be trigger_in_rising_edge, trigger_in_falling_edge. Gotthard only" << std::endl;
        os << "romode \t gets the readout flag. Options: analog, digital, analog_digital. Used for CTB only." << std::endl;
        os << "interruptsubframe \t gets the interrupt subframe flag. Setting it to 1 will interrupt the last subframe at the required exposure time. By default, this is disabled and set to 0, ie. it will wait for the last sub frame to finish exposing. Used for EIGER in 32 bit mode only." << std::endl;
        os << "readnlines \t gets the number of rows to read out per half module. Used for EIGER only. " << std::endl;
        os << "led \t returns led status (0 off, 1 on)" << std::endl;
        os << "powerchip \t gets if the chip has been powered on or off" << std::endl;
        os << "auto_comp_disable \t  gets if the automatic comparator diable mode is enabled/disabled" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdConfiguration(int narg, const char * const args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpConfiguration(action);

    std::string sval;

    if (narg < 2 && cmd != "rx_printconfig")
        return std::string("should specify I/O file");


    if (cmd == "config") {
        if (action == PUT_ACTION) {
            sval = std::string(args[1]);
            myDet->readConfigurationFile(sval);
        } else if (action == GET_ACTION) {
            sval = std::string(args[1]);
            myDet->writeConfigurationFile(sval);
        }
        return sval;
    } else if (cmd == "rx_printconfig") {
        if (action == PUT_ACTION)
            return std::string("cannot put");
        return myDet->printReceiverConfiguration(detPos);
    } else if (cmd == "parameters") {
        if (action == PUT_ACTION) {
            sval = std::string(args[1]);
            myDet->retrieveDetectorSetup(sval, 0);
        } else if (action == GET_ACTION) {
            sval = std::string(args[1]);
            myDet->dumpDetectorSetup(sval, 0);
        }
        return sval;
    } else if (cmd == "setup") {
        if (action == PUT_ACTION) {
            sval = std::string(args[1]);
            myDet->retrieveDetectorSetup(sval, 2);
        } else if (action == GET_ACTION) {
            sval = std::string(args[1]);
            myDet->dumpDetectorSetup(sval, 2);
        }
        return sval;
    }
    return std::string("could not decode conf mode");
}

std::string slsDetectorCommand::helpConfiguration(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {

        os << "config fname \t sets the detector to the configuration contained in fname" << std::endl;
        os << "parameters fname \t sets the detector parameters to those contained in fname" << std::endl;
        os << "setup fname \t sets the detector complete detector setup to that contained in fname (extensions automatically generated), including trimfiles, ff coefficients etc." << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "rx_printconfig \t prints the receiver configuration" << std::endl;
        os << "config fname \t saves the detector to the configuration to fname" << std::endl;
        os << "parameters fname \t saves the detector parameters to  fname" << std::endl;
        os << "setup fname \t saves the detector complete detector setup to  fname (extensions automatically generated), including trimfiles, ff coefficients etc." << std::endl;
    }

    return os.str();
}

std::string slsDetectorCommand::cmdReceiver(int narg, const char * const args[], int action, int detPos) {
    char answer[100];
    int ival = -1;

    if (action == HELP_ACTION)
        return helpReceiver(action);


    if (cmd == "rx_status") {
        if (action == PUT_ACTION) {
            if (!strcasecmp(args[1], "start"))
                myDet->startReceiver(detPos);
            else if (!strcasecmp(args[1], "stop"))
                myDet->stopReceiver(detPos);
            else
                return helpReceiver(action);
        }
        return myDet->runStatusType(myDet->getReceiverStatus(detPos));
    }

    else if (cmd == "framescaught") {
        if (action == PUT_ACTION)
            return std::string("cannot put");
        else {
            sprintf(answer, "%d", myDet->getFramesCaughtByReceiver(detPos));
            return std::string(answer);
        }
    }

    else if (cmd == "resetframescaught") {
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
    else if (cmd == "tengiga") {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &ival))
                return std::string("Could not scan tengiga input ") + std::string(args[1]);
            if (ival >= 0)
                sprintf(answer, "%d", myDet->enableTenGigabitEthernet(ival, detPos));
        } else
            sprintf(answer, "%d", myDet->enableTenGigabitEthernet(-1, detPos));
        return std::string(answer);

    }

    else if (cmd == "rx_framesperfile") {
        if (action == PUT_ACTION) {
            if (sscanf(args[1], "%d", &ival)) {
                myDet->setFramesPerFile(ival, detPos);
            } else
                return std::string("could not scan max frames per file\n");
        }
        memset(answer, 0, 100);
        sprintf(answer, "%d", myDet->getFramesPerFile(detPos));
        return std::string(answer);
    }

    else if (cmd == "rx_discardpolicy") {
        if (action == PUT_ACTION) {
            frameDiscardPolicy f = myDet->getReceiverFrameDiscardPolicy(std::string(args[1]));
            if (f == GET_FRAME_DISCARD_POLICY)
                return std::string("could not scan frame discard policy. Options: nodiscard, discardempty, discardpartial\n");
            myDet->setReceiverFramesDiscardPolicy(f);
        }
        return myDet->getReceiverFrameDiscardPolicy(myDet->setReceiverFramesDiscardPolicy(GET_FRAME_DISCARD_POLICY, detPos));
    }

    else if (cmd == "rx_jsonaddheader") {
        if (action == PUT_ACTION) {
            myDet->setAdditionalJsonHeader(args[1], detPos);
        }
        return myDet->getAdditionalJsonHeader(detPos);
    }

    else if (cmd == "rx_jsonpara") {
        if (action == PUT_ACTION) {
            myDet->setAdditionalJsonParameter(args[1], args[2], detPos);
        }
        return myDet->getAdditionalJsonParameter(args[1], detPos);
    }

    return std::string("could not decode command");
}

std::string slsDetectorCommand::helpReceiver(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "receiver [status] \t starts/stops the receiver to listen to detector packets. - can be start, stop." << std::endl;
        os << "resetframescaught [any value] \t resets frames caught by receiver" << std::endl;
        os << "rx_readfreq \t sets the gui read frequency of the receiver, 0 if gui requests frame, >0 if receiver sends every nth frame to gui. Default : 1" << std::endl;
        os << "tengiga \t sets system to be configure for 10Gbe if set to 1, else 1Gbe if set to 0" << std::endl;
        os << "rx_fifodepth [val]\t sets receiver fifo depth to val" << std::endl;
        os << "rx_silent [i]\t sets receiver in silent mode, ie. it will not print anything during real time acquisition. 1 sets, 0 unsets." << std::endl;
        os << "rx_framesperfile s\t sets the number of frames per file in receiver. 0 means infinite or all frames in a single file." << std::endl;
        os << "rx_discardpolicy s\t sets the frame discard policy in the receiver. nodiscard (default) - discards nothing, discardempty - discard only empty frames, discardpartial(fastest) - discards all partial frames." << std::endl;
        os << "rx_padding s\t enables/disables partial frames to be padded in the receiver. 0 does not pad partial frames(fastest), 1 (default) pads partial frames." << std::endl;
        os << "rx_jsonaddheader [t]\n sets additional json header to be streamed "
              "out with the zmq from receiver. Default is empty. t must be in the format '\"label1\":\"value1\",\"label2\":\"value2\"' etc."
              "Use only if it needs to be processed by an intermediate process." << std::endl;
        os << "rx_jsonpara [k] [v]\n sets value to v for additional json header parameter k to be streamed out with the zmq from receiver." << std::endl;

    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "receiver \t returns the status of receiver - can be running or idle" << std::endl;
        os << "framescaught \t returns the number of frames caught by receiver(average for multi)" << std::endl;
        os << "frameindex \t returns the current frame index of receiver(average for multi)" << std::endl;
        os << "rx_readfreq \t returns the gui read frequency of the receiver. DEfault: 1" << std::endl;
        os << "tengiga \t returns 1 if the system is configured for 10Gbe else 0 for 1Gbe" << std::endl;
        os << "rx_fifodepth \t returns receiver fifo depth" << std::endl;
        os << "rx_silent \t returns receiver silent mode enable. 1 is silent, 0 not silent." << std::endl;
        os << "rx_framesperfile \t gets the number of frames per file in receiver. 0 means infinite or all frames in a single file." << std::endl;
        os << "rx_discardpolicy \t gets the frame discard policy in the receiver. nodiscard (default) - discards nothing, discardempty - discard only empty frames, discardpartial(fastest) - discards all partial frames." << std::endl;
        os << "rx_padding \t gets partial frames padding enable in the receiver. 0 does not pad partial frames(fastest), 1 (default) pads partial frames." << std::endl;
        os << "rx_jsonaddheader \n gets additional json header to be streamed "
              "out with the zmq from receiver." << std::endl;
        os << "rx_jsonpara [k] \n gets value of additional json header parameter k to be streamed out with the zmq from receiver. If empty, then no parameter found." << std::endl;

    }
    return os.str();
}

std::string slsDetectorCommand::helpPattern(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "pattern fname \t loads pattern file" << std::endl;
        os << "patword addr word \t writes pattern word - only very advanced users!" << std::endl;
        os << "patioctrl reg\t configures inputs/outputs of the chiptest board - only advanced users!" << std::endl;
        os << "patclkctrl reg\t configures output clk enable of the chiptest board- only advanced users! " << std::endl;
        os << "patlimits addr1 addr2\t defines pattern limits between addr1 and addr2" << std::endl;
        os << "patloop0 addr1 adrr2 \t configures the limits of the 0 loop " << std::endl;
        os << "patloop1 addr1 adrr2 \t configures the limits of the 1 loop " << std::endl;
        os << "patloop2 addr1 adrr2 \t configures the limits of the 2 loop " << std::endl;
        os << "patnloop0 n \t sets number of cycles of the 0 loop " << std::endl;
        os << "patnloop1 n \t sets number of cycles of the 1 loop " << std::endl;
        os << "patnloop2 n \t sets number of cycles of the 2 loop " << std::endl;
        os << "patwait0 addr \t configures pattern wait 0 address " << std::endl;
        os << "patwait1 addr \t configures pattern wait 1 address " << std::endl;
        os << "patwait2 addr \t configures pattern wait 2 address " << std::endl;
        os << "patwaittime0 nclk \t sets wait 0 waiting time in clock number " << std::endl;
        os << "patwaittime1 nclk \t sets wait 1 waiting time in clock number " << std::endl;
        os << "patwaittime2 nclk \t sets wait 2 waiting time in clock number " << std::endl;
        os << "patmask m \t sets the 64 bit mask (hex) applied to every pattern. Only the bits from patsetbit are selected to mask for the corresponding bit value from m mask" << std::endl;
        os << "patsetbit m \t selects bits (hex) of the 64 bits that the patmask will be applied to every pattern. Only the bits from m mask are selected to mask for the corresponding bit value from patmask." << std::endl;
        os << "adcinvert mask\t sets the adcinversion mask (hex) CTB or Moench only" << std::endl;
        os << "adcenable mask\t sets the adcenable mask (hex) CTB or Moench only" << std::endl;
        os << "extsamplingsrc i\t  sets the external sampling source signal for digital data. i must be between 0 and 63. Advanced! CTB only " << std::endl;
        os << "extsampling i\t enables/disables the external sampling signal to the samplingsrc signal for digital data. Advanced! CTB only" << std::endl;
        os << "rx_dbitlist i..\t  sets the list of digital signal bits required for chip in receiver. If set to 'all', then all digital bits are enabled. Advanced! CTB only " << std::endl;
        os << "rx_dbitoffset i\t  sets the offset in bytes in receiver of digital data from chip in receiver. Advanced! CTB only " << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "pattern \t cannot get" << std::endl;
        os << "patword \t cannot get" << std::endl;
        os << "patioctrl \t returns inputs/outputs of the chiptest board - only advanced users!" << std::endl;
        os << "patclkctrl\t returns output clk enable of the chiptest board- only advanced users! " << std::endl;
        os << "patlimits \t returns pattern limits between addr1 and addr2" << std::endl;
        os << "patloop0  \t returns the limits of the 0 loop " << std::endl;
        os << "patloop1  \t returns the limits of the 1 loop " << std::endl;
        os << "patloop2  \t returns the limits of the 2 loop " << std::endl;
        os << "patnloop0 \t returns the number of cycles of the 0 loop " << std::endl;
        os << "patnloop1 \t returns the number of cycles of the 1 loop " << std::endl;
        os << "patnloop2 \t  returns the number of cycles of the 2 loop " << std::endl;
        os << "patwait0 \t  returns the pattern wait 0 address " << std::endl;
        os << "patwait1 \t  returns the pattern wait 1 address " << std::endl;
        os << "patwait2 \t  returns the pattern wait 2 address " << std::endl;
        os << "patwaittime0 \t  returns the wait 0 waiting time in clock number " << std::endl;
        os << "patwaittime1 \t  returns the wait 1 waiting time in clock number " << std::endl;
        os << "patwaittime2 \t  returns the wait 2 waiting time in clock number " << std::endl;
        os << "patmask \t gets the 64 bit mask (hex) applied to every pattern." << std::endl;
        os << "patsetbit \t gets 64 bit mask (hex) of the selected bits that the patmask will be applied to every pattern. " << std::endl;
        os << "adcinvert \t  returns the adcinversion mask " << std::endl;
        os << "adcenable \t  returns the adcenable mask " << std::endl;
        os << "extsamplingsrc \t  gets the external sampling source signal for digital data. i must be between 0 and 63. Advanced! CTB only " << std::endl;
        os << "extsampling \t gets the external sampling signal enable to the samplingsrc signal for digital data. Advanced! CTB only" << std::endl;
        os << "rx_dbitlist \t  gets the list of digital signal bits required for chip in receiver. If value is 'all', all digital bits are enabled. Advanced! CTB only " << std::endl;
        os << "rx_dbitoffset \t  gets the offset in bytes in receiver of digital data from chip in receiver. Advanced! CTB only " << std::endl;

    }
    return os.str();
}

std::string slsDetectorCommand::cmdPattern(int narg, const char * const args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpPattern(action);
    /********

  Must implement set ctb functions in slsDetector and multiSlsDetector

	 **********/
    std::string fname;
    int addr, start, stop, n;
    uint32_t u32;
    uint64_t word, t;


    std::ostringstream os;
    if (cmd == "pattern") {
        //get fname fron stdin

        if (action == PUT_ACTION) {
            fname = std::string(args[1]);
            myDet->setPattern(fname, detPos);
            os << "successful";
        } else if (action == GET_ACTION)
            os << "Cannot get";
    } else if (cmd == "savepattern") {
        if (narg < 2) {
            return helpPattern(action);
        }
        fname = std::string(args[1]);
        myDet->savePattern(args[1]);
        os << "Pattern executed and saved to " << fname;
    } else if (cmd == "patword") {

        if (action == PUT_ACTION) {
            //get addr, word from stdin

            if (narg < 3)
                return std::string("wrong usage: should specify both address and value (hexadecimal fomat) ");

            if (sscanf(args[1], "%x", &addr))
                ;
            else
                return std::string("Could not scan address (hexadecimal fomat) ") + std::string(args[1]);

            if (sscanf(args[2], "%lx", &word))
                ;
            else
                return std::string("Could not scan value  (hexadecimal fomat) ") + std::string(args[2]);

            os << "0x" << std::setw(4) << std::setfill('0') << std::hex  << addr << " 0x" << std::setw(16)  << myDet->setPatternWord(addr, word, detPos) << std::dec;
        } else if (action == GET_ACTION) {
            if (narg < 2)
                return std::string("wrong usage: should specify address (hexadecimal fomat) ");
            if (!sscanf(args[1], "%x", &addr))
                return std::string("Could not scan address (hexadecimal fomat) ") + std::string(args[1]);
            os << "0x" << std::setw(4) << std::setfill('0') << std::hex  << addr << " 0x" << std::setw(16)  << myDet->setPatternWord(addr, -1, detPos) << std::dec;
        }
    } else if (cmd == "patioctrl") {
        //get word from stdin

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%lx", &word))
                ;
            else
                return std::string("Could not scan value  (hexadecimal fomat) ") + std::string(args[1]);

            myDet->setPatternIOControl(word, detPos);
        }

        os << "0x" << std::setw(16) << std::setfill('0') << std::hex << myDet->setPatternIOControl(-1, detPos) << std::dec;
    } else if (cmd == "patclkctrl") {
        //get word from stdin

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%lx", &word))
                ;
            else
                return std::string("Could not scan value  (hexadecimal fomat) ") + std::string(args[1]);

            myDet->setPatternClockControl(word, detPos);
        }

        os << "0x" << std::setw(16) << std::setfill('0') << std::hex << myDet->setPatternClockControl(-1, detPos) << std::dec;

    } else if (cmd == "patlimits") {
        //get start, stop from stdin
        if (action == PUT_ACTION) {
            if (narg < 3)
                return std::string("wrong usage: should specify both start and stop address (hexadecimal fomat) ");
            n = -1;
            if (sscanf(args[1], "%x", &start))
                ;
            else
                return std::string("Could not scan start address  (hexadecimal fomat) ") + std::string(args[1]);

            if (sscanf(args[2], "%x", &stop))
                ;
            else
                return std::string("Could not scan stop address  (hexadecimal fomat) ") + std::string(args[2]);

            myDet->setPatternLoops(-1, start, stop, n, detPos);
        }

        auto r = myDet->getPatternLoops(-1, detPos);
        os << "0x" << std::setw(4) << std::setfill('0') << std::hex << r[0] << " 0x" << std::setw(4) << std::setfill('0') << r[1];
    } else if (cmd == "patloop0") {
        //get start, stop from stdin

        //get start, stop from stdin
        if (action == PUT_ACTION) {
            if (narg < 3)
                return std::string("wrong usage: should specify both start and stop address (hexadecimal fomat) ");
            n = -1;
            if (sscanf(args[1], "%x", &start))
                ;
            else
                return std::string("Could not scan start address  (hexadecimal fomat) ") + std::string(args[1]);

            if (sscanf(args[2], "%x", &stop))
                ;
            else
                return std::string("Could not scan stop address  (hexadecimal fomat) ") + std::string(args[2]);

            myDet->setPatternLoops(0, start, stop, n, detPos);
        }

        auto r = myDet->getPatternLoops(0, detPos);
        os << "0x" << std::setw(4) << std::setfill('0') << std::hex << r[0] << " 0x" << std::setw(4) << std::setfill('0') << r[1];

    } else if (cmd == "patloop1") {

        //get start, stop from stdin
        if (action == PUT_ACTION) {
            if (narg < 3)
                return std::string("wrong usage: should specify both start and stop address (hexadecimal fomat) ");
            n = -1;
            if (sscanf(args[1], "%x", &start))
                ;
            else
                return std::string("Could not scan start address  (hexadecimal fomat) ") + std::string(args[1]);

            if (sscanf(args[2], "%x", &stop))
                ;
            else
                return std::string("Could not scan stop address  (hexadecimal fomat) ") + std::string(args[2]);

            myDet->setPatternLoops(1, start, stop, n, detPos);
        }

        auto r = myDet->getPatternLoops(1, detPos);
        os << "0x" << std::setw(4) << std::setfill('0') << std::hex << r[0] << " 0x" << std::setw(4) << std::setfill('0') << r[1];

    } else if (cmd == "patloop2") {

        //get start, stop from stdin
        if (action == PUT_ACTION) {
            if (narg < 3)
                return std::string("wrong usage: should specify both start and stop address (hexadecimal fomat) ");
            n = -1;
            if (sscanf(args[1], "%x", &start))
                ;
            else
                return std::string("Could not scan start address  (hexadecimal fomat) ") + std::string(args[1]);

            if (sscanf(args[2], "%x", &stop))
                ;
            else
                return std::string("Could not scan stop address  (hexadecimal fomat) ") + std::string(args[2]);

            myDet->setPatternLoops(2, start, stop, n, detPos);
        }

        auto r = myDet->getPatternLoops(2, detPos);
        os << "0x" << std::setw(4) << std::setfill('0') << std::hex << r[0] << " 0x" << std::setw(4) << std::setfill('0') << r[1];
    } else if (cmd == "patnloop0") {
        start = -1;
        stop = -1;

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%d", &n))
                ;
            else
                return std::string("Could not scan number of loops ") + std::string(args[1]);

            myDet->setPatternLoops(0, start, stop, n, detPos);
        }

        auto r = myDet->getPatternLoops(0, detPos);
        os << std::dec << r[2];
    } else if (cmd == "patnloop1") {

        start = -1;
        stop = -1;

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%d", &n))
                ;
            else
                return std::string("Could not scan number of loops ") + std::string(args[1]);

            myDet->setPatternLoops(1, start, stop, n, detPos);
        }

        auto r = myDet->getPatternLoops(1, detPos);
        os << std::dec << r[2];

    } else if (cmd == "patnloop2") {

        start = -1;
        stop = -1;

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%d", &n))
                ;
            else
                return std::string("Could not scan number of loops ") + std::string(args[1]);

            myDet->setPatternLoops(2, start, stop, n, detPos);
        }

        auto r = myDet->getPatternLoops(2, detPos);
        os << std::dec << r[2];

    } else if (cmd == "patwait0") {

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%x", &addr))
                ;
            else
                return std::string("Could not scan wait address (hex format)") + std::string(args[1]);

            myDet->setPatternWaitAddr(0, addr, detPos);
        }

        os << "0x" << std::setw(4) << std::setfill('0') << std::hex << myDet->setPatternWaitAddr(0, -1, detPos) << std::dec;

    } else if (cmd == "patwait1") {

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%x", &addr))
                ;
            else
                return std::string("Could not scan wait address (hex format)") + std::string(args[1]);

            myDet->setPatternWaitAddr(1, addr, detPos);
        }

        os << "0x" << std::setw(4) << std::setfill('0') << std::hex << myDet->setPatternWaitAddr(1, -1, detPos) << std::dec;

    } else if (cmd == "patwait2") {

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%x", &addr))
                ;
            else
                return std::string("Could not scan wait address (hex format)") + std::string(args[1]);

            myDet->setPatternWaitAddr(2, addr, detPos);
        }

        os << "0x" << std::setw(4) << std::setfill('0') << std::hex << myDet->setPatternWaitAddr(2, -1, detPos) << std::dec;

    } else if (cmd == "patwaittime0") {

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%ld", &t))
                ;
            else
                return std::string("Could not scan wait time") + std::string(args[1]);

            myDet->setPatternWaitTime(0, t, detPos);
        }

        os << myDet->setPatternWaitTime(0, -1, detPos);

    } else if (cmd == "patwaittime1") {

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%ld", &t))
                ;
            else
                return std::string("Could not scan wait time ") + std::string(args[1]);

            myDet->setPatternWaitTime(1, t, detPos);
        }

        os << myDet->setPatternWaitTime(1, -1, detPos);

    } else if (cmd == "patwaittime2") {
        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%ld", &t))
                ;
            else
                return std::string("Could not scan wait time ") + std::string(args[1]);

            myDet->setPatternWaitTime(2, t, detPos);
        }

        os << myDet->setPatternWaitTime(2, -1, detPos);

    }  else if (cmd == "patmask") {
        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%lx", &word))
                ;
            else
                return std::string("Could not scan patmask argument (should be in hex) ") + std::string(args[1]);

            myDet->setPatternMask(word, detPos);
        }

        os << "0x" << std::setw(16) << std::setfill('0') << std::hex << myDet->getPatternMask(detPos) << std::dec;

    } else if (cmd == "patsetbit") {
        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%lx", &word))
                ;
            else
                return std::string("Could not scan patsetbit argument (should be in hex) ") + std::string(args[1]);

            myDet->setPatternBitMask(word, detPos);
        }

        os << "0x" << std::setw(16) << std::setfill('0') << std::hex << myDet->getPatternBitMask(detPos) << std::dec;

    }  else if (cmd == "adcenable") {

        if (action == PUT_ACTION) {
            if (sscanf(args[1], "%x", &u32))
                ;
            else
                return std::string("Could not scan adcenable reg ") + std::string(args[1]);
            
            myDet->setADCEnableMask(u32, detPos);
        }

        os << std::hex << myDet->getADCEnableMask(detPos) << std::dec;
    } 
    // kept only for backwards compatibility, use adcenable
    else if (cmd == "adcdisable") {

        if (action == PUT_ACTION) {
            if (sscanf(args[1], "%x", &u32))
                ;
            else
                return std::string("Could not scan adcdisable reg ") + std::string(args[1]);
            
            // get enable mask from enable mask
            u32 ^= BIT32_MASK;
            myDet->setADCEnableMask(u32, detPos);
        }

        u32 = myDet->getADCEnableMask(detPos);
        // get disable mask
        u32 ^= BIT32_MASK;
        os << std::hex << u32 << std::dec;

    } else if (cmd == "adcinvert") {
        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%x", &u32))
                ;
            else
                return std::string("Could not scan adcinvert reg ") + std::string(args[1]);

            myDet->setADCInvert(u32, detPos);
        }

        os << std::hex << myDet->getADCInvert(detPos) << std::dec;

    } else if (cmd == "extsamplingsrc") {
        if (action == PUT_ACTION) {

            if (!sscanf(args[1], "%d", &addr)) 
                return std::string("Could not scan extsampling src ") + std::string(args[1]);

            if (addr < 0 || addr > 63)
                return std::string("extsamplingsrc must be between 0 and 63. ") + std::string(args[1]);

            myDet->setExternalSamplingSource(addr, detPos); 
        }

        os << myDet->getExternalSamplingSource(detPos); 
   
    } else if (cmd == "extsampling") {
        if (action == PUT_ACTION) {

            if (!sscanf(args[1], "%d", &addr)) 
                return std::string("Could not scan extsampling enable ") + std::string(args[1]);

            myDet->setExternalSampling(addr, detPos); 
        }

        os << myDet->getExternalSampling(detPos);
    
    } else if (cmd == "rx_dbitlist") {


        if (action == PUT_ACTION) {
            std::vector <int> dbitlist;

            // if not all digital bits enabled
            if (std::string(args[1]) != "all") {
                for (int i = 1; i < narg; ++i) {
                    int temp = 0;
                    if (!sscanf(args[i], "%d", &temp))
                        return std::string("Could not scan dbitlist value ") +
                               std::string(args[i]);
                    if (temp < 0 || temp > 63)
                        return std::string("dbitlist value should be between 0 and 63 ") +
                               std::string(args[i]);
                    dbitlist.push_back(temp);
                }
                if (dbitlist.size() > 64) {
                    return std::string("Max number of values for dbitlist is 64 ");
                }
            }

            myDet->setReceiverDbitList(dbitlist, detPos); 
        }
        
        std::vector <int> dbitlist = myDet->getReceiverDbitList(detPos);  
        // all digital bits enabled
        if (dbitlist.empty()) 
            return std::string("all");
        // selective bits
        for (const auto &value : dbitlist) 
            os << value << " ";
    
    } else if (cmd == "rx_dbitoffset") {


        if (action == PUT_ACTION) {

            if (!sscanf(args[1], "%d", &addr)) 
                return std::string("Could not scan rx_dbitoffset enable ") + std::string(args[1]);

            myDet->setReceiverDbitOffset(addr, detPos); 
        }

        os << myDet->getReceiverDbitOffset(detPos);
    
    } 

    else
        return helpPattern(action);

    return os.str();
}

std::string slsDetectorCommand::helpPulse(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "pulse [n] [x] [y] \t pulses pixel at coordinates (x,y) n number of times" << std::endl;
        os << "pulsenmove [n] [x] [y]\t pulses pixel n number of times and moves relatively by x value (x axis) and y value(y axis)" << std::endl;
        os << "pulsechip [n] \t pulses chip n number of times, while n=-1 will reset it to normal mode" << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "pulse \t cannot get" << std::endl;
        os << "pulsenmove \t cannot get" << std::endl;
        os << "pulsechip \t cannot get" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdPulse(int narg, const char * const args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpPulse(action);
    else if (action == GET_ACTION)
        return std::string("cannot get ") + cmd;


    int ival1 = -1;
    if (!sscanf(args[1], "%d", &ival1))
        return std::string("Could not scan 1st argument ") + std::string(args[1]);

    if (std::string(args[0]) == std::string("pulsechip"))
        myDet->pulseChip(ival1, detPos);

    else {
        //next commands requires 3 addnl. arguments
        int ival2 = -1, ival3 = -1;
        if (narg < 4)
            return std::string("insufficient arguments:\n" + helpPulse(action));
        if (!sscanf(args[2], "%d", &ival2))
            return std::string("Could not scan 2nd argument ") + std::string(args[2]);
        if (!sscanf(args[3], "%d", &ival3))
            return std::string("Could not scan 3rd argument ") + std::string(args[3]);

        if (std::string(args[0]) == std::string("pulse"))
            myDet->pulsePixel(ival1, ival2, ival3, detPos);

        else if (std::string(args[0]) == std::string("pulsenmove"))
            myDet->pulsePixelNMove(ival1, ival2, ival3, detPos);

        else
            std::string("could not decode command") + cmd;
    }

    return std::string(" successful");
}


std::string slsDetectorCommand::helpProcessor(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "emin [n] \t Sets detector minimum energy threshold to x for Moench (soft setting in processor)" << std::endl;
        os << "emax [n] \t Sets detector maximum energy threshold to x for Moench (soft setting in processor)" << std::endl;
        os << "framemode [n] \t Sets frame mode for Moench (soft setting in processor). Options: pedestal, newpedestal, flatfield, newflatfield" << std::endl;
        os << "detectormode [n] \t Sets  detector mode for Moench (soft setting in processor). Options: counting, interpolating, analog" << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "emin [n] \t Gets detector minimum energy threshold to x for Moench (soft setting in processor)" << std::endl;
        os << "emax [n] \t Gets detector maximum energy threshold to x for Moench (soft setting in processor)" << std::endl;
        os << "framemode [n] \t Gets frame mode for Moench (soft setting in processor). Options: pedestal, newpedestal, flatfield, newflatfield" << std::endl;
        os << "detectormode [n] \t Gets  detector mode for Moench (soft setting in processor). Options: counting, interpolating, analog" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdProcessor(int narg, const char * const args[], int action, int detPos) {
    if (action == HELP_ACTION)
        return helpProcessor(action);


    if (cmd == "emin" || cmd == "emax") {
        if (action == PUT_ACTION) {
            int ival = -1;
            if(!sscanf(args[1],"%d",&ival))
                return std::string("cannot parse emin/emax value");
            myDet->setDetectorMinMaxEnergyThreshold((cmd == "emin" ? 0 : 1), ival, detPos);
        }
        return std::to_string(myDet->setDetectorMinMaxEnergyThreshold(0, -1, detPos));
    }

    else if (cmd == "framemode") {
        if (action == PUT_ACTION) {
            frameModeType ival = getFrameModeType(args[1]);
            if (ival == GET_FRAME_MODE)
                return std::string("cannot parse frame mode value");
            myDet->setFrameMode(ival, detPos);
        }
        return getFrameModeType(frameModeType(myDet->setFrameMode(GET_FRAME_MODE, detPos)));
    }

    else if (cmd == "detectormode") {
        if (action == PUT_ACTION) {
            detectorModeType ival = getDetectorModeType(args[1]);
            if (ival == GET_DETECTOR_MODE)
                return std::string("cannot parse detector mode value");
            myDet->setDetectorMode(ival, detPos);
        }
        return getDetectorModeType(detectorModeType(myDet->setDetectorMode(GET_DETECTOR_MODE, detPos)));
    }
    return std::string("unknown command");
}



