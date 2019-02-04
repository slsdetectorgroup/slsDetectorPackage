//------------------------------------------------------------------------------------------------------
#include <iostream>
#include <vector>
#include <string>

#include <cstdlib>
#include <cstring>

#include <stdint.h>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <sstream>
#include <unistd.h>

#include "sls_receiver_defs.h"
#include "slsReceiverUsers.h"

#include "sls_detector_defs.h"
#include "slsDetectorUsers.h"

#define GOTTHARD_25_TEST
//#define JUNGFRAU_TEST
//#define GOTTHARD_TEST

//======================================================================================================
// test configuration
//======================================================================================================
int      acquisition_nb    = 1; // number of acquisition to make
int      acquisition_nb_ok = 0; // number of correct acquisition
uint64_t last_acquisition_received_frames; // number of received frames during the last acquisition
std::vector <int> acquisition_nb_list;

bool use_trace = false; // activate the acquisition log

//------------------------------------------------------------------------------------------------------
// GOTTHARD 25um
//------------------------------------------------------------------------------------------------------
#ifdef GOTTHARD_25_TEST
    const int receivers_nb = 2; // number of receivers
    const int receivers_rx_tcpport[receivers_nb] = {1954, 1955}; // tcp port for each receiver

    const int detector_id = 0; // detector identifier for slsDetectorUsers constructor
    const std::string detector_config_file_name = "gotthard25.config"; // configuration file name (must be present in the same folder of this application)

    const long        detector_receiver_fifo_depth        = 2500;
    double            detector_exposure_time_sec          = 0.005;
    double            detector_exposure_period_sec        = 0.10;
    const double      detector_delay_after_trigger_sec    = 0.0;
    const std::string detector_trig_mode                  = "auto"; // "auto" or "trigger"
    int64_t           detector_nb_frames_per_cycle        = 10;
    const int64_t     detector_nb_cycles                  = 1;
    int               detector_module_index[receivers_nb] = {0, 1};
#else
//------------------------------------------------------------------------------------------------------
// GOTTHARD
//------------------------------------------------------------------------------------------------------
#ifdef GOTTHARD_TEST
    const int receivers_nb = 1; // number of receivers
    const int receivers_rx_tcpport[receivers_nb] = {1954}; // tcp port for each receiver

    const int detector_id = 0; // detector identifier for slsDetectorUsers constructor
    const std::string detector_config_file_name = "gotthard25.config"; // configuration file name (must be present in the same folder of this application)

    const long        detector_receiver_fifo_depth        = 2500;
    double            detector_exposure_time_sec          = 0.005;
    double            detector_exposure_period_sec        = 0.1;
    const double      detector_delay_after_trigger_sec    = 0.0;
    const std::string detector_trig_mode                  = "auto"; // "auto" or "trigger"
    int64_t           detector_nb_frames_per_cycle        = 10;
    const int64_t     detector_nb_cycles                  = 1;
    int               detector_module_index[receivers_nb] = {0};
#else
//------------------------------------------------------------------------------------------------------
// JUNGFRAU
//------------------------------------------------------------------------------------------------------
#ifdef JUNGFRAU_TEST
    const int receivers_nb = 1; // number of receivers
    const int receivers_rx_tcpport[receivers_nb] = {1954}; // tcp port for each receiver

    const int detector_id = 0; // detector identifier for slsDetectorUsers constructor
    const std::string detector_config_file_name = "jungfrau_nanoscopium_switch.config"; // configuration file name (must be present in the same folder of this application)

    const long        detector_receiver_fifo_depth        = 2500;
    double            detector_exposure_time_sec          = 0.0005;
    double            detector_exposure_period_sec        = 0.001;
    const double      detector_delay_after_trigger_sec    = 0.0;
    const std::string detector_trig_mode                  = "auto"; // "auto" or "trigger"
    int64_t           detector_nb_frames_per_cycle        = 10000;
    const int64_t     detector_nb_cycles                  = 1;
    const int         detector_clock_divider              = 1;
    int               detector_module_index[receivers_nb] = {0};
#endif
#endif
#endif

//------------------------------------------------------------------------------------------------------
// test instances
//------------------------------------------------------------------------------------------------------
std::vector<slsReceiverUsers *> receivers;
slsDetectorUsers * detector = NULL;

//------------------------------------------------------------------------------------------------------
// tools functions
//------------------------------------------------------------------------------------------------------
/** Define Colors to print data call back in different colors for different recievers */
#define PRINT_IN_COLOR(c,f, ...) 	printf ("\033[%dm" f RESET, 30 + c+1, ##__VA_ARGS__)

#define PRINT_SEPARATOR()  	cprintf(MAGENTA, "============================================\n")

/************************************************************************
 * \brief cleans the shared memory used by the camera
 ************************************************************************/
void clean_shared_memory()
{
    std::string cmd = "rm /dev/shm/slsDetectorPackage*;";
    std::system(cmd.c_str());
}

/*******************************************************************
 * \brief converts a version id to a string
 * \return version in string format (uppercase & hexa)
 *******************************************************************/
std::string convertVersionToString(int64_t in_version)
{
    std::stringstream tempStream;
    tempStream << "0x" << std::uppercase << std::hex << in_version;
    return tempStream.str();
}

//==================================================================
// Related to commands (put & get)
//==================================================================
/*******************************************************************
 * \brief Converts a standard string to args arguments
 * \param in_command command in command line format
 * \param out_argv output c-strings c-array
 * \param out_argc output number of arguments of out_argv
 *******************************************************************/
void convertStringToArgs(const std::string & in_command,
                         char  * *         & out_argv  ,
                         int               & out_argc  )
{
    out_argv = NULL;
    out_argc = 0   ;

    // filling a string vector with the command line elements
    std::vector<std::string> elements;
    std::stringstream ss(in_command);

	while (ss) 
    {
        std::string element;
		ss >> element;

        if(element.size() > 0)
        {
            elements.push_back(element);
        }
	}

    // setting argc value
    out_argc = elements.size();

    // allocating argv array
	out_argv = new char * [out_argc];
    
    // filling argv array
	for (int element_index = 0; element_index < out_argc; element_index++)
    {
        out_argv[element_index] = new char[elements[element_index].size() + 1]; // adding the allocation of end of c-string 
        strcpy(out_argv[element_index], elements[element_index].c_str()); // copying the string including the eos
    }
}

/*******************************************************************
 * \brief Releases args arguments
 * \param in_out_argv output c-strings c-array*(static_cast<int *>(p))
 * \param in_out_argc output number of arguments of out_argv
 *******************************************************************/
void releaseArgs(char * * & in_out_argv  ,
                 int      & in_out_argc  )
{
    if(in_out_argv != NULL)
    {
        // releasing the c_strings array content
        for (int element_index = 0; element_index < in_out_argc; element_index++)
        {
            delete [] in_out_argv[element_index];
        }

        // releasing the c_strings array
        delete [] in_out_argv;

        in_out_argv = NULL;
        in_out_argc = 0   ;
    }
}

/*******************************************************************
 * \brief Executes a set command
 * \param in_command command in command line format
 * \param in_module_index module index
 * \return the command result
 *******************************************************************/
std::string setCmd(const std::string & in_command, int in_module_index=-1)
{
    std::cout << "setCmd - execute set command:\"" << in_command << "\"" << std::endl;

    char  * *   argv  ;
    int         argc  ;
    std::string result;

    convertStringToArgs(in_command, argv, argc);

    if(argc > 0)
    {
        result = detector->putCommand(argc, argv, in_module_index);
    }

    releaseArgs(argv, argc);

	std::cout << "result=\"" << result << "\"" << std::endl;
    return result;
}

/*******************************************************************
 * \brief Executes a get command
 * \param in_command command in command line format
 * \param in_module_index module index
 * \return the command result
 *******************************************************************/
std::string getCmd(const std::string & in_command, int in_module_index=-1)
{
    std::cout << "getCmd - execute get command:\"" << in_command << "\"" << std::endl;

    char  * *   argv  ;
    int         argc  ;
    std::string result;

    convertStringToArgs(in_command, argv, argc);

    if(argc > 0)
    {
        result = detector->getCommand(argc, argv, in_module_index);
    }

    releaseArgs(argv, argc);

	std::cout << "result=\"" << result << "\"" << std::endl;
    return result;
}

//------------------------------------------------------------------------------------------------------
// Receivers callbacks
//------------------------------------------------------------------------------------------------------
/**
 * Start Acquisition Call back
 * slsReceiver writes data if file write enabled.
 * Users get data to write using call back if registerCallBackRawDataReady is registered.
 * @param filepath file path
 * @param filename file name
 * @param fileindex file index
 * @param datasize data size in bytes
 * @param p pointer to object
 * \returns ignored
 */
int StartAcq(char* filepath, char* filename, uint64_t fileindex, uint32_t datasize, void*p){
	cprintf(BLUE, "#### StartAcq:  filepath:%s  filename:%s fileindex:%llu  datasize:%u ####\n",
			filepath, filename, fileindex, datasize);

	cprintf(BLUE, "--StartAcq: returning 0\n");
    last_acquisition_received_frames = 0LL;
	return 0;
}

/**
 * Acquisition Finished Call back
 * @param frames Number of frames caught
 * @param p pointer to object
 */
void AcquisitionFinished(uint64_t frames, void*p){
	cprintf(BLUE, "#### AcquisitionFinished: frames:%llu ####\n",frames);
    last_acquisition_received_frames = frames;
}

/**
 * Get Receiver Data Call back
 * Prints in different colors(for each receiver process) the different headers for each image call back.
 * @param metadata sls_receiver_header metadata
 * @param datapointer pointer to data
 * @param datasize data size in bytes.
 * @param p pointer to object
 */
void GetData(char* metadata, char* datapointer, uint32_t datasize, void* p)
{
    if(use_trace)
    {
        slsReceiverDefs::sls_receiver_header* header = (slsReceiverDefs::sls_receiver_header*)metadata;
        const slsReceiverDefs::sls_detector_header & detectorHeader = header->detHeader;

        PRINT_IN_COLOR (*(static_cast<int *>(p)),
			        "#### %d GetData: ####\n"
			        "frameNumber: %llu\t\texpLength: %u\t\tpacketNumber: %u\t\tbunchId: %llu"
			        "\t\ttimestamp: %llu\t\tmodId: %u\t\t"
			        "row: %u\t\tcolumn: %u\t\treserved: %u\t\tdebug: %u"
			        "\t\troundRNumber: %u\t\tdetType: %u\t\tversion: %u"
			        //"\t\tpacketsMask:%s"
			        "\t\tfirstbytedata: 0x%x\t\tdatsize: %u\n\n",
                               *(static_cast<int *>(p)),
                                (long long unsigned int)detectorHeader.frameNumber,
                                detectorHeader.expLength, 
                                detectorHeader.packetNumber, 
                                (long long unsigned int)detectorHeader.bunchId,
                                (long long unsigned int)detectorHeader.timestamp,
                                detectorHeader.modId,
			        detectorHeader.row,
                                detectorHeader.column,
                                detectorHeader.reserved,
			        detectorHeader.debug,
                                detectorHeader.roundRNumber,
			        detectorHeader.detType,
			        detectorHeader.version,
			        //header->packetsMask.to_string().c_str(),
			        ((uint8_t)(*((uint8_t*)(datapointer)))),
			        datasize);
    }

    if((datapointer != NULL) && (datasize > 0))
    {
        char * buffer = new char[datasize];
        memcpy(buffer, datapointer, datasize);
        delete [] buffer;
    }
}

//------------------------------------------------------------------------------------------------------
// CreateReceivers
//------------------------------------------------------------------------------------------------------
void CreateReceivers(void)
{
    // preparing the args for receivers creation
    char        temp_port[10];
    const int   argc       = 3;
    char      * args[argc] = {(char*)"slsReceiver", (char*)"--rx_tcpport", temp_port};

    // creating the receivers instances 
    for(int i = 0 ; i < receivers_nb ; i++)
    {
    	int ret = slsReceiverDefs::OK;

        // changing the udp port in the args
        sprintf(temp_port, "%d", receivers_rx_tcpport[i]);

        // creating the receiver using the args
        slsReceiverUsers * receiver = new slsReceiverUsers(argc, args, ret);

        // managing a failed result
        if(ret==slsReceiverDefs::FAIL)
        {
            delete receiver;
            exit(EXIT_FAILURE);
        }

        // adding the receiver to the receivers container
        receivers.push_back(receiver);

        std::cout << "receiver (" << i << ") created - port (" << receivers_rx_tcpport[i] << ")" << std::endl;

        // registering callbacks
        // Call back for start acquisition
        cprintf(BLUE, "Registering StartAcq()\n");
        receiver->registerCallBackStartAcquisition(StartAcq, NULL);

        // Call back for acquisition finished
        cprintf(BLUE, "Registering AcquisitionFinished()\n");
        receiver->registerCallBackAcquisitionFinished(AcquisitionFinished, NULL);

        // Call back for raw data
        cprintf(BLUE, "Registering GetData() \n");
        receiver->registerCallBackRawDataReady(GetData, NULL);//&(detector_module_index[i]));

        // starting tcp server thread
        if (receiver->start() == slsReceiverDefs::FAIL)
        {
            delete receiver;
            cprintf(BLUE,"Could not start receiver (%d)\n", i);
            exit(EXIT_FAILURE);
        }
    }
}

//------------------------------------------------------------------------------------------------------
// ReleaseReceivers
//------------------------------------------------------------------------------------------------------
void ReleaseReceivers(void)
{
    // deleting the receivers instances 
    for(int i = 0 ; i < receivers.size() ; i++)
    {
        slsReceiverUsers * receiver = receivers[i];

		// stoping tcp server thread
        receiver->stop();

        delete receiver;
    }
}

//------------------------------------------------------------------------------------------------------
// CreateDetector
//------------------------------------------------------------------------------------------------------
void CreateDetector(void)
{
    int result;

    // create the detector instance
    detector = new slsDetectorUsers(result, detector_id);

    if(result == slsDetectorDefs::FAIL)
    {
		std::cout << "slsDetectorUsers constructor failed! Could not initialize the camera!" << std::endl;
		exit(EXIT_FAILURE);
    }

    // configuration file is used to properly configure advanced settings in the shared memory
    /*result = detector->readConfigurationFile(detector_config_file_name);

    if(result == slsDetectorDefs::FAIL)
    {
		std::cout << "readConfigurationFile failed! Could not initialize the camera!" << std::endl;
		exit(EXIT_FAILURE);
    }*/

	// set detector in shared memory online (in case no config file was used) */
	detector->setOnline(slsDetectorDefs::ONLINE_FLAG);

	// set receiver in shared memory online (in case no config file was used) */
	detector->setReceiverOnline(slsDetectorDefs::ONLINE_FLAG);

    // disabling the file write by the camera
    detector->enableWriteToFile(slsDetectorDefs::DISABLED);

    // logging some versions informations
    std::cout << "Detector developer        : " << detector->getDetectorDeveloper() << std::endl;
    std::cout << "Detector type             : " << detector->getDetectorType() << std::endl;
    std::cout << "Detector Firmware Version : " << convertVersionToString(detector->getDetectorFirmwareVersion()) << std::endl;
    std::cout << "Detector Software Version : " << convertVersionToString(detector->getDetectorSoftwareVersion()) << std::endl;

	// ensuring detector status is idle
	int status = detector->getDetectorStatus();

    if((status != slsDetectorDefs::IDLE) && (status != slsDetectorDefs::STOPPED))
    {
		std::cout << "Detector not ready: " << slsDetectorUsers::runStatusType(status) << std::endl;
		exit(EXIT_FAILURE);
	}
}

//------------------------------------------------------------------------------------------------------
// ReleaseDetector
//------------------------------------------------------------------------------------------------------
void ReleaseDetector(void)
{
    if(detector != NULL)
    {
        detector->setReceiverOnline(slsDetectorDefs::OFFLINE_FLAG);
        detector->setOnline(slsDetectorDefs::OFFLINE_FLAG);

        delete detector;
        detector = NULL;
    }
}

//------------------------------------------------------------------------------------------------------
// RunAcquisition
//------------------------------------------------------------------------------------------------------
int RunAcquisition(void)
{
    std::string trig_mode_label;

    double exposure_time  ;
    double exposure_period;
    double delay_after_trigger;

    int64_t nb_frames_per_cycle;
    int64_t nb_cycles;
    int64_t nb_frames;
#ifdef JUNGFRAU_TEST
    int clock_divider;
#endif
    //----------------------------------------------------------------------------------------------------
    // setting the receiver fifo depth (number of frames in the receiver memory)
    detector->setReceiverFifoDepth(detector_receiver_fifo_depth);

    //----------------------------------------------------------------------------------------------------
    detector->setExposureTime     (detector_exposure_time_sec  , true); // in seconds
    detector->setExposurePeriod   (detector_exposure_period_sec, true); // in seconds
    detector->setDelayAfterTrigger(detector_delay_after_trigger_sec, true); // in seconds

    exposure_time       = detector->setExposureTime     (-1, true); // in seconds
    exposure_period     = detector->setExposurePeriod   (-1, true); // in seconds
    delay_after_trigger = detector->setDelayAfterTrigger(-1, true, 0); // in seconds

    //----------------------------------------------------------------------------------------------------
    // initing the number of frames per cycle and  number of cycles 
    // to avoid problems during the trigger mode change.
    detector->setNumberOfFrames(1);
    detector->setNumberOfCycles(1);

    // conversion of trigger mode label to trigger mode index
    int trigger_mode_index = slsDetectorUsers::getTimingMode(detector_trig_mode);

    // apply the trigger change
    detector->setTimingMode(trigger_mode_index);

    // converting trigger mode index to trigger mode label
    trig_mode_label = slsDetectorUsers::getTimingMode(trigger_mode_index);

    // setting the number of cycles
    nb_cycles = detector->setNumberOfCycles(detector_nb_cycles);

    // setting the number of frames per cycle
    nb_frames_per_cycle = detector->setNumberOfFrames(detector_nb_frames_per_cycle);

    // setting the gain mode
    detector->setSettings(slsDetectorUsers::getDetectorSettings("dynamicgain"));
#ifndef JUNGFRAU_TEST
    detector->setSettings(slsDetectorUsers::getDetectorSettings("mediumgain"));
#else
    detector->setSettings(slsDetectorUsers::getDetectorSettings("dynamichg0"));
#endif
    // computing the number of frames
    nb_frames = nb_cycles * nb_frames_per_cycle;

    //----------------------------------------------------------------------------------------------------
#ifdef JUNGFRAU_TEST
    // clock divider
    detector->setClockDivider(detector_clock_divider);
    clock_divider = detector->setClockDivider(-1);
#endif
    //----------------------------------------------------------------------------------------------------
    std::cout << "receiver fifo depth : " << detector_receiver_fifo_depth << std::endl;
    std::cout << "Exposure time in seconds : " << exposure_time << std::endl;
    std::cout << "Exposure period in seconds : " << exposure_period << std::endl;
    std::cout << "Delay after trigger in seconds : " << delay_after_trigger << std::endl;
    std::cout << "Trigger mode : " << trig_mode_label << std::endl;
    std::cout << "Nb frames per cycle : " << nb_frames_per_cycle << std::endl;
    std::cout << "Nb cycles : " << nb_cycles << std::endl;
    std::cout << "Nb frames : " << nb_frames << std::endl;
#ifdef JUNGFRAU_TEST
    std::cout << "Clock divider : " << clock_divider << std::endl;
#endif
    std::cout << "Estimated frame rate : " << (1.0 / exposure_period) << std::endl;

    //----------------------------------------------------------------------------------------------------
    // reset the number of caught frames in the sdk
    detector->resetFramesCaughtInReceiver();

    //----------------------------------------------------------------------------------------------------
    const unsigned int sleep_time_sec = 1; // sleep the thread in seconds

    // starting receiver listening mode
    if(detector->startReceiver() == slsDetectorDefs::FAIL)
    {
        std::cout << "Could not start the receiver listening mode!" << std::endl;
        return slsDetectorDefs::FAIL;
    }

    // starting real time acquisition in non blocking mode
    // returns OK if all detectors are properly started, FAIL otherwise
    if(detector->startAcquisition() == slsDetectorDefs::FAIL)
    {
        detector->stopReceiver();
        std::cout << "Could not start real time acquisition!" << std::endl;
        return slsDetectorDefs::FAIL;
    }

    for(;;)
    {
        // checking if the hardware acquisition is running
        int status = detector->getDetectorStatus();
        if((status == slsDetectorDefs::IDLE   ) || 
           (status == slsDetectorDefs::STOPPED) ||
           (status == slsDetectorDefs::ERROR  ))
        {
            // we stop the treatment
            break;
        }
        else
        // hardware acquisition is running, we are waiting for new frames not using the cpu during this time
        {
            usleep(sleep_time_sec * 1000 * 1000); // sleep the thread in seconds
        }
    }

    // stopping receiver listening mode
    if(detector->stopReceiver() == slsDetectorDefs::FAIL)
    {
        std::cout << "Could not stop real time acquisition!" << std::endl;
        return slsDetectorDefs::FAIL;
    }
    
    //----------------------------------------------------------------------------------------------------
    PRINT_SEPARATOR();
    std::cout << "receiver fifo depth : " << detector_receiver_fifo_depth << std::endl;
    std::cout << "Exposure time in seconds : " << exposure_time << std::endl;
    std::cout << "Exposure period in seconds : " << exposure_period << std::endl;
    std::cout << "Delay after trigger in seconds : " << delay_after_trigger << std::endl;
    std::cout << "Trigger mode : " << trig_mode_label << std::endl;
    std::cout << "Nb frames per cycle : " << nb_frames_per_cycle << std::endl;
    std::cout << "Nb cyles : " << nb_cycles << std::endl;
    std::cout << "Nb frames : " << nb_frames << std::endl;
#ifdef JUNGFRAU_TEST
    std::cout << "Clock divider : " << clock_divider << std::endl;
#endif
    std::cout << "Estimated frame rate : " << (1.0 / exposure_period) << std::endl;

    if(last_acquisition_received_frames == nb_frames)
    {
        acquisition_nb_ok++;
        return slsDetectorDefs::OK;
    }

    PRINT_SEPARATOR();
    return slsDetectorDefs::FAIL;
}

//------------------------------------------------------------------------------------------------------
// test
//------------------------------------------------------------------------------------------------------
void Test(void)
{
    try
    {
        PRINT_SEPARATOR();
        std::cout << "CreateReceivers" << std::endl;
        PRINT_SEPARATOR();

        CreateReceivers();
        
        PRINT_SEPARATOR();
        std::cout << "CreateDetector" << std::endl;
        PRINT_SEPARATOR();

        CreateDetector();

        PRINT_SEPARATOR();
        std::cout << "RunAcquisition" << std::endl;
        PRINT_SEPARATOR();

        for(int acquisition_index = 0 ; acquisition_index < acquisition_nb ; acquisition_index++)
        {
            cprintf(MAGENTA, "Acquisition number : %d\n", acquisition_index);
            if (RunAcquisition() == slsDetectorDefs::FAIL) {
                acquisition_nb_list.push_back(acquisition_index);
            }
        }
        
        PRINT_SEPARATOR();
        std::cout << "ReleaseDetector" << std::endl;
        PRINT_SEPARATOR();

        ReleaseDetector();
        
        PRINT_SEPARATOR();
        std::cout << "ReleaseReceivers" << std::endl;
        PRINT_SEPARATOR();

        ReleaseReceivers();

        PRINT_SEPARATOR();
	if (acquisition_nb - acquisition_nb_ok)
		cprintf(BOLD RED, "Correct acquisition(s) %d/%d\n", acquisition_nb_ok, acquisition_nb);
	else
		cprintf(BOLD GREEN, "Correct acquisition(s) %d/%d\n", acquisition_nb_ok, acquisition_nb);
        if (acquisition_nb - acquisition_nb_ok) {
            cprintf(RED, "Acquisition(s) gone wrong :\n");
            for (int list_index = 0; list_index < acquisition_nb_list.size(); ++list_index) {
		cprintf(RED, "%d\n", acquisition_nb_list[list_index]);
            }
        }
        PRINT_SEPARATOR();
    }
    catch (...)
    {
        std::cout << "unknown exception!" << std::endl;
		exit(EXIT_FAILURE);
    }
}

std::string roi_result = 
"detector 0:\n"
"0       255     -1      -1\n"
"detector 1:\n"
"1024    1279    -1      -1\n"
"\n"
"xmin    xmax    ymin    ymax\n"
"0       255     -1      -1\n"
"2304    2559    -1      -1\n"
"roi 2\n";

#include <vector>

// use example :
// std::vector<slsReceiverDefs::ROI> rois;
// get_rois_from_string(roi_result, rois);
/*******************************************************************
 * \brief Cuts the string in pieces
 * \param[in] in_string source string
 * \param[in] in_delimitor line delimitor
 * \param[out] out_lines line container result
 *******************************************************************/
void split_string_line(const std::string & in_string, const char in_delimitor, std::vector<std::string> & out_lines)
{
    std::stringstream ss(in_string);
    std::string sub_string;

    while (getline(ss, sub_string, in_delimitor))
    {
        out_lines.push_back(sub_string);
    }
}

/*******************************************************************
 * \brief retrieve the ROIs from a string
 * \param[in] in_rois_string string from "get roi" command
 * \param[out] out_rois ROI container result (empty if no set ROI)
 *******************************************************************/
void get_rois_from_string(const std::string & in_rois_string, std::vector<slsReceiverDefs::ROI> & out_rois)
{
    out_rois.clear();

    try
    {
        // cuts the string in lines
        std::vector<std::string> lines;
        split_string_line(in_rois_string, '\n', lines);

        if(lines.size() >= 1)
        {
            // checks if no ROI ?
            if(lines[0] != "roi 0")
            {
                for(int roi_index = 0 ; roi_index < 2 ; roi_index++)
                {
                    if(lines.size() >= ((roi_index + 1) * 2)) // two lines per ROI definition
                    {
                        std::stringstream detector_name;
                        detector_name << "detector " << roi_index << ":";
                        
                        // checks the first line
                        if(lines[roi_index * 2] == detector_name.str())
                        {
                            std::stringstream ss(lines[(roi_index * 2) + 1]);

                            slsReceiverDefs::ROI roi;
                            ss >> roi.xmin;
                            ss >> roi.xmax;  
                            ss >> roi.ymin;
                            ss >> roi.ymax;

                            out_rois.push_back(roi);
                        }
                    }
                }
            }
        }
    }
    catch(...)
    {
        out_rois.clear();
    }
}

//------------------------------------------------------------------------------------------------------
// read_simple_option
//------------------------------------------------------------------------------------------------------
bool read_simple_option(int argc, char* argv[], const char * in_option_name)
{
    int option_index = 1;

    while(option_index < argc)
    {
        if (strcmp(argv[option_index], in_option_name) == 0)
        {
            std::cout << "Found option:" << in_option_name << std::endl;
            return true;
        }

        option_index++;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------
// read_option_value
//------------------------------------------------------------------------------------------------------
template <typename T> bool read_option_value(int argc, char* argv[], const char * in_option_name, T & out_option_value)
{
    int option_index = 1;

    while(option_index < argc)
    {
        if (strcmp(argv[option_index], in_option_name) == 0)
        {
            option_index++;

            if(option_index < argc)
            {
                std::stringstream ss(std::string(argv[option_index]));
                ss >> out_option_value;
                std::cout << "Found option: " << in_option_name << " " << out_option_value << std::endl;
                return true;
            }
        }

        option_index++;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    if(read_simple_option(argc, argv, "-help") || read_simple_option(argc, argv, "--help"))
    {
        PRINT_SEPARATOR();
        std::cout << "Options:" << std::endl;
        std::cout << "-clean -> clean shared memory" << std::endl;
        std::cout << "-trace -> activate acquisition log" << std::endl;
        std::cout << "-exp <value> -> set exposure time value in seconds (for example: -exp 0.0005)" << std::endl;
        std::cout << "-period <value> -> set period time value in seconds (for example: -period 0.001)" << std::endl;
        std::cout << "-frames <value> -> set number of frames (for example: -frames 10000)" << std::endl;
        std::cout << "-acq <value> -> set number of acquisition (for example: -acq 10)" << std::endl;
        std::cout << std::endl;
        std::cout << "example: ./manual-acq -clean -trace -acq 1 -exp 0.0005 -period 0.001 -frames 1000" << std::endl;
        PRINT_SEPARATOR();
        return 0;
    }

    if(read_simple_option(argc, argv, "-clean"))
    {
        PRINT_SEPARATOR();
        std::cout << "Cleaning shared memory" << std::endl;
        PRINT_SEPARATOR();

        clean_shared_memory();
    }

    if(read_simple_option(argc, argv, "-trace"))
    {
        PRINT_SEPARATOR();
        std::cout << "Activating acquisition log..." << std::endl;
        PRINT_SEPARATOR();

        use_trace = true;
    }

    int64_t frames_value;

    if(read_option_value(argc, argv, "-frames", frames_value))
    {
        detector_nb_frames_per_cycle = frames_value;
    }

    double exp_value;

    if(read_option_value(argc, argv, "-exp", exp_value))
    {
        detector_exposure_time_sec = exp_value;
    }

    double period_value;

    if(read_option_value(argc, argv, "-period", period_value))
    {
        detector_exposure_period_sec = period_value;
    }

    int acq_nb;

    if(read_option_value(argc, argv, "-acq", acq_nb))
    {
        acquisition_nb = acq_nb;
    }

    Test();

    std::cout << "====================== ENDING ======================" << std::endl;

    return 0;
}

//------------------------------------------------------------------------------------------------------
