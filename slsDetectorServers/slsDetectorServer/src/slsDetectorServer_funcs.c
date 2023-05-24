// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "slsDetectorServer_funcs.h"
#include "clogger.h"
#include "communication_funcs.h"
#include "sharedMemory.h"
#include "sls/sls_detector_funcs.h"
#include "slsDetectorFunctionList.h"

#if defined(CHIPTESTBOARDD) || defined(MYTHEN3D)
#include "Pattern.h"
#include "loadPattern.h"
#endif

#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>

// defined in the detector specific Makefile
#ifdef GOTTHARDD
const enum detectorType myDetectorType = GOTTHARD;
#elif EIGERD
const enum detectorType myDetectorType = EIGER;
#elif JUNGFRAUD
const enum detectorType myDetectorType = JUNGFRAU;
#elif CHIPTESTBOARDD
const enum detectorType myDetectorType = CHIPTESTBOARD;
#elif MOENCHD
const enum detectorType myDetectorType = MOENCH;
#elif MYTHEN3D
const enum detectorType myDetectorType = MYTHEN3;
#elif GOTTHARD2D
const enum detectorType myDetectorType = GOTTHARD2;
#else
const enum detectorType myDetectorType = GENERIC;
#endif

#define LOCALHOSTIP_INT 2130706433

// Global variables from communication_funcs
extern int lockStatus;
extern uint32_t lastClientIP;
extern uint32_t thisClientIP;
extern int differentClients;
extern int isControlServer;
extern int ret;
extern int fnum;
extern char mess[MAX_STR_LENGTH];

// Variables that will be exported
int sockfd = 0;
int debugflag = 0;
int updateFlag = 0;
int checkModuleFlag = 1;
int ignoreConfigFileFlag = 0;

udpStruct udpDetails[MAX_UDP_DESTINATION];
int numUdpDestinations = 1;

int configured = FAIL;
char configureMessage[MAX_STR_LENGTH] = "udp parameters not configured yet";
int maxYMods = -1;
int moduleIndex = -1;

// Local variables
int (*flist[NUM_DET_FUNCTIONS])(int);
pthread_t pthread_tid;

// scan variables
int scan = 0;
int numScanSteps = 0;
int *scanSteps = NULL;
int64_t scanSettleTime_ns = 0;
enum dacIndex scanGlobalIndex = 0;
int scanTrimbits = 0;
char scanErrMessage[MAX_STR_LENGTH] = "";

/* initialization functions */

int updateModeAllowedFunction(int file_des) {
    enum detFuncs allowedFuncs[] = {
        F_EXEC_COMMAND,           F_GET_DETECTOR_TYPE,  F_GET_FIRMWARE_VERSION,
        F_GET_SERVER_VERSION,     F_GET_SERIAL_NUMBER,  F_WRITE_REGISTER,
        F_READ_REGISTER,          F_LOCK_SERVER,        F_GET_LAST_CLIENT_IP,
        F_PROGRAM_FPGA,           F_RESET_FPGA,         F_INITIAL_CHECKS,
        F_REBOOT_CONTROLLER,      F_GET_KERNEL_VERSION, F_UPDATE_KERNEL,
        F_UPDATE_DETECTOR_SERVER, F_GET_UPDATE_MODE,    F_SET_UPDATE_MODE,
        F_GET_NUM_CHANNELS,       F_GET_NUM_INTERFACES, F_ACTIVATE,
        F_GET_HARDWARE_VERSION};
    size_t allowedFuncsSize = sizeof(allowedFuncs) / sizeof(enum detFuncs);

    for (unsigned int i = 0; i < allowedFuncsSize; ++i) {
        if ((unsigned int)fnum == allowedFuncs[i]) {
            return OK;
        }
    }
    ret = FAIL;
    sprintf(mess,
            "Funcion (%s) cannot be executed in update mode. Please disable "
            "update mode to continue.\n",
            getFunctionNameFromEnum((enum detFuncs)fnum));
    LOG(logERROR, (mess));
    Server_SendResult(file_des, INT32, NULL, 0);
    return FAIL;
}

int printSocketReadError() {
    LOG(logERROR, ("Error reading from socket. Possible socket crash.\n"));
    return FAIL;
}

void init_detector() {
    memset(udpDetails, 0, sizeof(udpDetails));
#ifdef VIRTUAL
    udpDetails[0].srcip = LOCALHOSTIP_INT;
    udpDetails[0].srcip2 = LOCALHOSTIP_INT;
#endif
    udpDetails[0].srcport = DEFAULT_UDP_SRC_PORTNO;
    udpDetails[0].dstport = DEFAULT_UDP_DST_PORTNO;
#ifdef EIGERD
    udpDetails[0].dstport2 = DEFAULT_UDP_DST_PORTNO + 1;
#endif
    lockStatus = 0;
    if (isControlServer) {
        basictests();
        initControlServer();
    } else {
        initStopServer();
    }
}

int decode_function(int file_des) {
    ret = FAIL;

    int n = receiveData(file_des, &fnum, sizeof(fnum), INT32);
    if (n <= 0) {
        LOG(logDEBUG3,
            ("ERROR reading from socket n=%d, fnum=%d, file_des=%d, fname=%s\n",
             n, fnum, file_des, getFunctionNameFromEnum((enum detFuncs)fnum)));
        return FAIL;
    } else
        LOG(logDEBUG3, ("Received %d bytes\n", n));

    if (fnum < 0 || fnum >= NUM_DET_FUNCTIONS) {
        LOG(logERROR, ("Unknown function enum %d\n", fnum));
        ret = (M_nofunc)(file_des);
    } else {

        // udpate mode restricted functions, send error (without waitin for
        // arguments)
        if (updateFlag && updateModeAllowedFunction(file_des) == FAIL) {
            return FAIL;
        }

        LOG(logDEBUG1, (" calling function fnum=%d, (%s)\n", fnum,
                        getFunctionNameFromEnum((enum detFuncs)fnum)));
        ret = (*flist[fnum])(file_des);

        if (ret == FAIL) {
            LOG(logDEBUG1, ("Error executing the function = %d (%s)\n", fnum,
                            getFunctionNameFromEnum((enum detFuncs)fnum)));
        } else
            LOG(logDEBUG1,
                ("Function (%s) executed %s\n",
                 getFunctionNameFromEnum((enum detFuncs)fnum), getRetName()));
    }
    return ret;
}

const char *getRetName() {
    switch (ret) {
    case OK:
        return "OK";
    case FAIL:
        return "FAIL";
    case GOODBYE:
        return "GOODBYE";
    case REBOOT:
        return "REBOOT";
    default:
        return "unknown";
    }
}

const char *getRunStateName(enum runStatus ind) {
    switch (ind) {
    case IDLE:
        return "idle";
    case ERROR:
        return "error";
    case WAITING:
        return "waiting";
    case RUN_FINISHED:
        return "run_finished";
    case TRANSMITTING:
        return "transmitting";
    case RUNNING:
        return "running";
    case STOPPED:
        return "stopped";
    default:
        return "unknown";
    }
}

void function_table() {
    flist[F_EXEC_COMMAND] = &exec_command;
    flist[F_GET_DETECTOR_TYPE] = &get_detector_type;
    flist[F_GET_EXTERNAL_SIGNAL_FLAG] = &get_external_signal_flag;
    flist[F_SET_EXTERNAL_SIGNAL_FLAG] = &set_external_signal_flag;
    flist[F_SET_TIMING_MODE] = &set_timing_mode;
    flist[F_GET_FIRMWARE_VERSION] = &get_firmware_version;
    flist[F_GET_SERVER_VERSION] = &get_server_version;
    flist[F_GET_SERIAL_NUMBER] = &get_serial_number;
    flist[F_SET_FIRMWARE_TEST] = &set_firmware_test;
    flist[F_SET_BUS_TEST] = &set_bus_test;
    flist[F_SET_IMAGE_TEST_MODE] = &set_image_test_mode;
    flist[F_GET_IMAGE_TEST_MODE] = &get_image_test_mode;
    flist[F_SET_DAC] = &set_dac;
    flist[F_GET_ADC] = &get_adc;
    flist[F_WRITE_REGISTER] = &write_register;
    flist[F_READ_REGISTER] = &read_register;
    flist[F_SET_MODULE] = &set_module;
    flist[F_SET_SETTINGS] = &set_settings;
    flist[F_GET_THRESHOLD_ENERGY] = &get_threshold_energy;
    flist[F_START_ACQUISITION] = &start_acquisition;
    flist[F_STOP_ACQUISITION] = &stop_acquisition;
    flist[F_GET_RUN_STATUS] = &get_run_status;
    flist[F_START_AND_READ_ALL] = &start_and_read_all;
    flist[F_GET_NUM_FRAMES] = &get_num_frames;
    flist[F_SET_NUM_FRAMES] = &set_num_frames;
    flist[F_GET_NUM_TRIGGERS] = &get_num_triggers;
    flist[F_SET_NUM_TRIGGERS] = &set_num_triggers;
    flist[F_GET_NUM_ADDITIONAL_STORAGE_CELLS] =
        &get_num_additional_storage_cells;
    flist[F_SET_NUM_ADDITIONAL_STORAGE_CELLS] =
        &set_num_additional_storage_cells;
    flist[F_GET_NUM_ANALOG_SAMPLES] = &get_num_analog_samples;
    flist[F_SET_NUM_ANALOG_SAMPLES] = &set_num_analog_samples;
    flist[F_GET_NUM_DIGITAL_SAMPLES] = &get_num_digital_samples;
    flist[F_SET_NUM_DIGITAL_SAMPLES] = &set_num_digital_samples;
    flist[F_GET_EXPTIME] = &get_exptime;
    flist[F_SET_EXPTIME] = &set_exptime;
    flist[F_GET_PERIOD] = &get_period;
    flist[F_SET_PERIOD] = &set_period;
    flist[F_GET_DELAY_AFTER_TRIGGER] = &get_delay_after_trigger;
    flist[F_SET_DELAY_AFTER_TRIGGER] = &set_delay_after_trigger;
    flist[F_GET_SUB_EXPTIME] = &get_sub_exptime;
    flist[F_SET_SUB_EXPTIME] = &set_sub_exptime;
    flist[F_GET_SUB_DEADTIME] = &get_sub_deadtime;
    flist[F_SET_SUB_DEADTIME] = &set_sub_deadtime;
    flist[F_GET_STORAGE_CELL_DELAY] = &get_storage_cell_delay;
    flist[F_SET_STORAGE_CELL_DELAY] = &set_storage_cell_delay;
    flist[F_GET_FRAMES_LEFT] = &get_frames_left;
    flist[F_GET_TRIGGERS_LEFT] = &get_triggers_left;
    flist[F_GET_EXPTIME_LEFT] = &get_exptime_left;
    flist[F_GET_PERIOD_LEFT] = &get_period_left;
    flist[F_GET_DELAY_AFTER_TRIGGER_LEFT] = &get_delay_after_trigger_left;
    flist[F_GET_MEASURED_PERIOD] = &get_measured_period;
    flist[F_GET_MEASURED_SUBPERIOD] = &get_measured_subperiod;
    flist[F_GET_FRAMES_FROM_START] = &get_frames_from_start;
    flist[F_GET_ACTUAL_TIME] = &get_actual_time;
    flist[F_GET_MEASUREMENT_TIME] = &get_measurement_time;
    flist[F_SET_DYNAMIC_RANGE] = &set_dynamic_range;
    flist[F_SET_ROI] = &set_roi;
    flist[F_GET_ROI] = &get_roi;
    flist[F_LOCK_SERVER] = &lock_server;
    flist[F_GET_LAST_CLIENT_IP] = &get_last_client_ip;
    flist[F_ENABLE_TEN_GIGA] = &enable_ten_giga;
    flist[F_SET_ALL_TRIMBITS] = &set_all_trimbits;
    flist[F_SET_PATTERN_IO_CONTROL] = &set_pattern_io_control;
    flist[F_SET_PATTERN_WORD] = &set_pattern_word;
    flist[F_SET_PATTERN_LOOP_ADDRESSES] = &set_pattern_loop_addresses;
    flist[F_SET_PATTERN_LOOP_CYCLES] = &set_pattern_loop_cycles;
    flist[F_SET_PATTERN_WAIT_ADDR] = &set_pattern_wait_addr;
    flist[F_SET_PATTERN_WAIT_TIME] = &set_pattern_wait_time;
    flist[F_SET_PATTERN_MASK] = &set_pattern_mask;
    flist[F_GET_PATTERN_MASK] = &get_pattern_mask;
    flist[F_SET_PATTERN_BIT_MASK] = &set_pattern_bit_mask;
    flist[F_GET_PATTERN_BIT_MASK] = &get_pattern_bit_mask;
    flist[F_WRITE_ADC_REG] = &write_adc_register;
    flist[F_SET_COUNTER_BIT] = &set_counter_bit;
    flist[F_PULSE_PIXEL] = &pulse_pixel;
    flist[F_PULSE_PIXEL_AND_MOVE] = &pulse_pixel_and_move;
    flist[F_PULSE_CHIP] = &pulse_chip;
    flist[F_SET_RATE_CORRECT] = &set_rate_correct;
    flist[F_GET_RATE_CORRECT] = &get_rate_correct;
    flist[F_SET_TEN_GIGA_FLOW_CONTROL] = &set_ten_giga_flow_control;
    flist[F_GET_TEN_GIGA_FLOW_CONTROL] = &get_ten_giga_flow_control;
    flist[F_SET_TRANSMISSION_DELAY_FRAME] = &set_transmission_delay_frame;
    flist[F_GET_TRANSMISSION_DELAY_FRAME] = &get_transmission_delay_frame;
    flist[F_SET_TRANSMISSION_DELAY_LEFT] = &set_transmission_delay_left;
    flist[F_GET_TRANSMISSION_DELAY_LEFT] = &get_transmission_delay_left;
    flist[F_SET_TRANSMISSION_DELAY_RIGHT] = &set_transmission_delay_right;
    flist[F_GET_TRANSMISSION_DELAY_RIGHT] = &get_transmission_delay_right;
    flist[F_PROGRAM_FPGA] = &program_fpga;
    flist[F_RESET_FPGA] = &reset_fpga;
    flist[F_POWER_CHIP] = &power_chip;
    flist[F_ACTIVATE] = &set_activate;
    flist[F_THRESHOLD_TEMP] = &threshold_temp;
    flist[F_TEMP_CONTROL] = &temp_control;
    flist[F_TEMP_EVENT] = &temp_event;
    flist[F_AUTO_COMP_DISABLE] = &auto_comp_disable;
    flist[F_STORAGE_CELL_START] = &storage_cell_start;
    flist[F_INITIAL_CHECKS] = &initial_checks;
    flist[F_SOFTWARE_TRIGGER] = &software_trigger;
    flist[F_LED] = &led;
    flist[F_DIGITAL_IO_DELAY] = &digital_io_delay;
    flist[F_REBOOT_CONTROLLER] = &reboot_controller;
    flist[F_SET_ADC_ENABLE_MASK] = &set_adc_enable_mask;
    flist[F_GET_ADC_ENABLE_MASK] = &get_adc_enable_mask;
    flist[F_SET_ADC_INVERT] = &set_adc_invert;
    flist[F_GET_ADC_INVERT] = &get_adc_invert;
    flist[F_EXTERNAL_SAMPLING_SOURCE] = &set_external_sampling_source;
    flist[F_EXTERNAL_SAMPLING] = &set_external_sampling;
    flist[F_SET_NEXT_FRAME_NUMBER] = &set_next_frame_number;
    flist[F_GET_NEXT_FRAME_NUMBER] = &get_next_frame_number;
    flist[F_SET_QUAD] = &set_quad;
    flist[F_GET_QUAD] = &get_quad;
    flist[F_SET_INTERRUPT_SUBFRAME] = &set_interrupt_subframe;
    flist[F_GET_INTERRUPT_SUBFRAME] = &get_interrupt_subframe;
    flist[F_SET_READ_N_ROWS] = &set_read_n_rows;
    flist[F_GET_READ_N_ROWS] = &get_read_n_rows;
    flist[F_SET_POSITION] = &set_detector_position;
    flist[F_SET_SOURCE_UDP_MAC] = &set_source_udp_mac;
    flist[F_GET_SOURCE_UDP_MAC] = &get_source_udp_mac;
    flist[F_SET_SOURCE_UDP_MAC2] = &set_source_udp_mac2;
    flist[F_GET_SOURCE_UDP_MAC2] = &get_source_udp_mac2;
    flist[F_SET_SOURCE_UDP_IP] = &set_source_udp_ip;
    flist[F_GET_SOURCE_UDP_IP] = &get_source_udp_ip;
    flist[F_SET_SOURCE_UDP_IP2] = &set_source_udp_ip2;
    flist[F_GET_SOURCE_UDP_IP2] = &get_source_udp_ip2;
    flist[F_SET_DEST_UDP_MAC] = &set_dest_udp_mac;
    flist[F_GET_DEST_UDP_MAC] = &get_dest_udp_mac;
    flist[F_SET_DEST_UDP_MAC2] = &set_dest_udp_mac2;
    flist[F_GET_DEST_UDP_MAC2] = &get_dest_udp_mac2;
    flist[F_SET_DEST_UDP_IP] = &set_dest_udp_ip;
    flist[F_GET_DEST_UDP_IP] = &get_dest_udp_ip;
    flist[F_SET_DEST_UDP_IP2] = &set_dest_udp_ip2;
    flist[F_GET_DEST_UDP_IP2] = &get_dest_udp_ip2;
    flist[F_SET_DEST_UDP_PORT] = &set_dest_udp_port;
    flist[F_GET_DEST_UDP_PORT] = &get_dest_udp_port;
    flist[F_SET_DEST_UDP_PORT2] = &set_dest_udp_port2;
    flist[F_GET_DEST_UDP_PORT2] = &get_dest_udp_port2;
    flist[F_SET_NUM_INTERFACES] = &set_num_interfaces;
    flist[F_GET_NUM_INTERFACES] = &get_num_interfaces;
    flist[F_SET_INTERFACE_SEL] = &set_interface_sel;
    flist[F_GET_INTERFACE_SEL] = &get_interface_sel;
    flist[F_SET_PARALLEL_MODE] = &set_parallel_mode;
    flist[F_GET_PARALLEL_MODE] = &get_parallel_mode;
    flist[F_SET_OVERFLOW_MODE] = &set_overflow_mode;
    flist[F_GET_OVERFLOW_MODE] = &get_overflow_mode;
    flist[F_SET_READOUT_MODE] = &set_readout_mode;
    flist[F_GET_READOUT_MODE] = &get_readout_mode;
    flist[F_SET_CLOCK_FREQUENCY] = &set_clock_frequency;
    flist[F_GET_CLOCK_FREQUENCY] = &get_clock_frequency;
    flist[F_SET_CLOCK_PHASE] = &set_clock_phase;
    flist[F_GET_CLOCK_PHASE] = &get_clock_phase;
    flist[F_GET_MAX_CLOCK_PHASE_SHIFT] = &get_max_clock_phase_shift;
    flist[F_SET_CLOCK_DIVIDER] = &set_clock_divider;
    flist[F_GET_CLOCK_DIVIDER] = &get_clock_divider;
    flist[F_SET_ON_CHIP_DAC] = &set_on_chip_dac;
    flist[F_GET_ON_CHIP_DAC] = &get_on_chip_dac;
    flist[F_SET_INJECT_CHANNEL] = &set_inject_channel;
    flist[F_GET_INJECT_CHANNEL] = &get_inject_channel;
    flist[F_SET_VETO_PHOTON] = &set_veto_photon;
    flist[F_GET_VETO_PHOTON] = &get_veto_photon;
    flist[F_SET_VETO_REFERENCE] = &set_veto_reference;
    flist[F_GET_BURST_MODE] = &get_burst_mode;
    flist[F_SET_BURST_MODE] = &set_burst_mode;
    flist[F_SET_ADC_ENABLE_MASK_10G] = &set_adc_enable_mask_10g;
    flist[F_GET_ADC_ENABLE_MASK_10G] = &get_adc_enable_mask_10g;
    flist[F_SET_COUNTER_MASK] = &set_counter_mask;
    flist[F_GET_COUNTER_MASK] = &get_counter_mask;
    flist[F_GET_NUM_BURSTS] = &get_num_bursts;
    flist[F_SET_NUM_BURSTS] = &set_num_bursts;
    flist[F_GET_BURST_PERIOD] = &get_burst_period;
    flist[F_SET_BURST_PERIOD] = &set_burst_period;
    flist[F_GET_CURRENT_SOURCE] = &get_current_source;
    flist[F_SET_CURRENT_SOURCE] = &set_current_source;
    flist[F_GET_TIMING_SOURCE] = &get_timing_source;
    flist[F_SET_TIMING_SOURCE] = &set_timing_source;
    flist[F_GET_NUM_CHANNELS] = &get_num_channels;
    flist[F_UPDATE_RATE_CORRECTION] = &update_rate_correction;
    flist[F_GET_RECEIVER_PARAMETERS] = &get_receiver_parameters;
    flist[F_START_PATTERN] = &start_pattern;
    flist[F_SET_NUM_GATES] = &set_num_gates;
    flist[F_GET_NUM_GATES] = &get_num_gates;
    flist[F_SET_GATE_DELAY] = &set_gate_delay;
    flist[F_GET_GATE_DELAY] = &get_gate_delay;
    flist[F_GET_EXPTIME_ALL_GATES] = &get_exptime_all_gates;
    flist[F_GET_GATE_DELAY_ALL_GATES] = &get_gate_delay_all_gates;
    flist[F_GET_VETO] = &get_veto;
    flist[F_SET_VETO] = &set_veto;
    flist[F_SET_PATTERN] = &set_pattern;
    flist[F_GET_SCAN] = &get_scan;
    flist[F_SET_SCAN] = &set_scan;
    flist[F_GET_SCAN_ERROR_MESSAGE] = &get_scan_error_message;
    flist[F_GET_CDS_GAIN] = &get_cds_gain;
    flist[F_SET_CDS_GAIN] = &set_cds_gain;
    flist[F_GET_FILTER_RESISTOR] = &get_filter_resistor;
    flist[F_SET_FILTER_RESISTOR] = &set_filter_resistor;
    flist[F_GET_ADC_CONFIGURATION] = &get_adc_config;
    flist[F_SET_ADC_CONFIGURATION] = &set_adc_config;
    flist[F_GET_BAD_CHANNELS] = &get_bad_channels;
    flist[F_SET_BAD_CHANNELS] = &set_bad_channels;
    flist[F_RECONFIGURE_UDP] = &reconfigure_udp;
    flist[F_VALIDATE_UDP_CONFIG] = &validate_udp_configuration;
    flist[F_GET_BURSTS_LEFT] = &get_bursts_left;
    flist[F_START_READOUT] = &start_readout;
    flist[F_RESET_TO_DEFAULT_DACS] = &reset_to_default_dacs;
    flist[F_IS_VIRTUAL] = &is_virtual;
    flist[F_GET_PATTERN] = &get_pattern;
    flist[F_LOAD_DEFAULT_PATTERN] = &load_default_pattern;
    flist[F_GET_ALL_THRESHOLD_ENERGY] = &get_all_threshold_energy;
    flist[F_GET_MASTER] = &get_master;
    flist[F_GET_CSR] = &get_csr;
    flist[F_SET_GAIN_CAPS] = &set_gain_caps;
    flist[F_GET_GAIN_CAPS] = &get_gain_caps;
    flist[F_GET_DATASTREAM] = &get_datastream;
    flist[F_SET_DATASTREAM] = &set_datastream;
    flist[F_GET_VETO_STREAM] = &get_veto_stream;
    flist[F_SET_VETO_STREAM] = &set_veto_stream;
    flist[F_GET_VETO_ALGORITHM] = &get_veto_algorithm;
    flist[F_SET_VETO_ALGORITHM] = &set_veto_algorithm;
    flist[F_GET_CHIP_VERSION] = &get_chip_version;
    flist[F_GET_DEFAULT_DAC] = &get_default_dac;
    flist[F_SET_DEFAULT_DAC] = &set_default_dac;
    flist[F_GET_GAIN_MODE] = &get_gain_mode;
    flist[F_SET_GAIN_MODE] = &set_gain_mode;
    flist[F_GET_COMP_DISABLE_TIME] = &get_comp_disable_time;
    flist[F_SET_COMP_DISABLE_TIME] = &set_comp_disable_time;
    flist[F_GET_FLIP_ROWS] = &get_flip_rows;
    flist[F_SET_FLIP_ROWS] = &set_flip_rows;
    flist[F_GET_NUM_FILTER_CELLS] = &get_num_filter_cells;
    flist[F_SET_NUM_FILTER_CELLS] = &set_num_filter_cells;
    flist[F_SET_ADC_PIPELINE] = &set_adc_pipeline;
    flist[F_GET_ADC_PIPELINE] = &get_adc_pipeline;
    flist[F_SET_DBIT_PIPELINE] = &set_dbit_pipeline;
    flist[F_GET_DBIT_PIPELINE] = &get_dbit_pipeline;
    flist[F_GET_MODULE_ID] = &get_module_id;
    flist[F_GET_DEST_UDP_LIST] = &get_dest_udp_list;
    flist[F_SET_DEST_UDP_LIST] = &set_dest_udp_list;
    flist[F_GET_NUM_DEST_UDP] = &get_num_dest_list;
    flist[F_CLEAR_ALL_UDP_DEST] = &clear_all_udp_dst;
    flist[F_GET_UDP_FIRST_DEST] = &get_udp_first_dest;
    flist[F_SET_UDP_FIRST_DEST] = &set_udp_first_dest;
    flist[F_GET_READOUT_SPEED] = &get_readout_speed;
    flist[F_SET_READOUT_SPEED] = &set_readout_speed;
    flist[F_GET_KERNEL_VERSION] = &get_kernel_version;
    flist[F_UPDATE_KERNEL] = &update_kernel;
    flist[F_UPDATE_DETECTOR_SERVER] = &update_detector_server;
    flist[F_GET_UPDATE_MODE] = &get_update_mode;
    flist[F_SET_UPDATE_MODE] = &set_update_mode;
    flist[F_SET_MASTER] = &set_master;
    flist[F_GET_TOP] = &get_top;
    flist[F_SET_TOP] = &set_top;
    flist[F_GET_POLARITY] = &get_polarity;
    flist[F_SET_POLARITY] = &set_polarity;
    flist[F_GET_INTERPOLATION] = &get_interpolation;
    flist[F_SET_INTERPOLATION] = &set_interpolation;
    flist[F_GET_PUMP_PROBE] = &get_pump_probe;
    flist[F_SET_PUMP_PROBE] = &set_pump_probe;
    flist[F_GET_ANALOG_PULSING] = &get_analog_pulsing;
    flist[F_SET_ANALOG_PULSING] = &set_analog_pulsing;
    flist[F_GET_DIGITAL_PULSING] = &get_digital_pulsing;
    flist[F_SET_DIGITAL_PULSING] = &set_digital_pulsing;
    flist[F_GET_MODULE] = &get_module;
    flist[F_GET_SYNCHRONIZATION] = &get_synchronization;
    flist[F_SET_SYNCHRONIZATION] = &set_synchronization;
    flist[F_GET_HARDWARE_VERSION] = &get_hardware_version;
    flist[F_GET_FRONTEND_FIRMWARE_VERSION] = &get_frontend_firmware_version;

    // check
    if (NUM_DET_FUNCTIONS >= RECEIVER_ENUM_START) {
        LOG(logERROR, ("The last detector function enum has reached its "
                       "limit\nGoodbye!\n"));
        exit(EXIT_FAILURE);
    }

    for (int iloop = 0; iloop < NUM_DET_FUNCTIONS; ++iloop) {
        LOG(logDEBUG3, ("function fnum=%d, (%s)\n", iloop,
                        getFunctionNameFromEnum((enum detFuncs)iloop)));
    }
}

void functionNotImplemented() {
    ret = FAIL;
    sprintf(mess, "Function (%s) is not implemented for this detector\n",
            getFunctionNameFromEnum((enum detFuncs)fnum));
    LOG(logERROR, (mess));
}

void modeNotImplemented(char *modename, int mode) {
    ret = FAIL;
    sprintf(mess, "%s (%d) is not implemented for this detector\n", modename,
            mode);
    LOG(logERROR, (mess));
}

int executeCommand(char *command, char *result, enum TLogLevel level) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

    const size_t tempsize = 256;
    char temp[tempsize];
    memset(temp, 0, tempsize);
    memset(result, 0, MAX_STR_LENGTH);

    // copy command
    char cmd[MAX_STR_LENGTH] = {0};
    sprintf(cmd, "%s 2>&1", command);
    LOG(level, ("Executing command:\n[%s]\n", cmd));

    fflush(stdout);
    FILE *sysFile = popen(cmd, "r");
    while (fgets(temp, tempsize, sysFile) != NULL) {
        // size left excludes terminating character
        size_t sizeleft = MAX_STR_LENGTH - strlen(result) - 1;
        // more than the command
        if (tempsize > sizeleft) {
            strncat(result, temp, sizeleft);
            break;
        }
        strncat(result, temp, tempsize);
        memset(temp, 0, tempsize);
    }
    result[MAX_STR_LENGTH - 1] = '\0';
    if (strlen(result) == 0) {
        strcpy(result, "No result");
    }

    int retval = OK;
    int success = pclose(sysFile);
    if (success) {
        retval = FAIL;
        LOG(logERROR, ("Executing cmd[%s]:%s\n", cmd, result));
    } else {
        LOG(level, ("Result:\n[%s]\n", result));
    }

    return retval;
}

int M_nofunc(int file_des) {
    ret = FAIL;
    memset(mess, 0, sizeof(mess));

    sprintf(mess, "%s Function enum %d. Please do not proceed.\n",
            UNRECOGNIZED_FNUM_ENUM, fnum);
    LOG(logERROR, (mess));
    return Server_SendResult(file_des, OTHER, NULL, 0);
}

#if defined(MYTHEN3D) || defined(GOTTHARD2D)
void rebootNiosControllerAndFPGA() { rebootControllerAndFPGA(); }
#endif

int exec_command(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    char cmd[MAX_STR_LENGTH] = {0};
    char retval[MAX_STR_LENGTH] = {0};

    if (receiveData(file_des, cmd, MAX_STR_LENGTH, OTHER) < 0)
        return printSocketReadError();

    // set
    if (Server_VerifyLock() == OK) {
        ret = executeCommand(cmd, retval, logINFO);
    }
    return Server_SendResult(file_des, OTHER, retval, sizeof(retval));
}

int get_detector_type(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum detectorType retval = myDetectorType;
    LOG(logDEBUG1, ("Returning detector type %d\n", retval));
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int get_external_signal_flag(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    enum externalSignalFlag retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();

    LOG(logDEBUG1, ("Getting external signal flag (%d)\n", arg));

#if !defined(GOTTHARDD) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // get
    if (arg < 0 || arg >= MAX_EXT_SIGNALS) {
        ret = FAIL;
        sprintf(mess, "Signal index %d can only be between 0 and %d\n", arg,
                MAX_EXT_SIGNALS - 1);
        LOG(logERROR, (mess));
    } else {
        retval = getExtSignal(arg);
        LOG(logDEBUG1, ("External Signal Flag: %d\n", retval));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_external_signal_flag(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[2] = {-1, -1};
    enum externalSignalFlag retval = -1;

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();

    int signalIndex = args[0];
    enum externalSignalFlag flag = args[1];
    LOG(logDEBUG1,
        ("Setting external signal flag [%d] to %d\n", signalIndex, flag));

#if !defined(GOTTHARDD) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    if (Server_VerifyLock() == OK) {
#ifdef MYTHEN3D
        // to be implemented in hardware as busy configurable
        if (signalIndex == 4) {
            ret = FAIL;
            sprintf(mess, "Signal index %d not configurable yet\n",
                    signalIndex);
            LOG(logERROR, (mess));
        } else
#endif
            if (signalIndex < 0 || signalIndex >= MAX_EXT_SIGNALS) {
            ret = FAIL;
            sprintf(mess, "Signal index %d can only be between 0 and %d\n",
                    signalIndex, MAX_EXT_SIGNALS - 1);
            LOG(logERROR, (mess));
        } else {
            switch (flag) {
            case TRIGGER_IN_RISING_EDGE:
            case TRIGGER_IN_FALLING_EDGE:
#ifdef MYTHEN3D
                if (signalIndex > 0) {
                    ret = FAIL;
                    sprintf(mess,
                            "Only Master input trigger signal can edge detect. "
                            "Not signal %d\n",
                            signalIndex);
                    LOG(logERROR, (mess));
                }
#endif
                break;
#ifdef MYTHEN3D
            case INVERSION_ON:
            case INVERSION_OFF:
                if (signalIndex == 0) {
                    ret = FAIL;
                    sprintf(
                        mess,
                        "Master input trigger signal cannot invert. Use "
                        "trigger_in_rising_edge or trigger_in_falling_edge\n");
                    LOG(logERROR, (mess));
                }
                break;
#endif
            default:
                ret = FAIL;
                sprintf(mess, "Unknown flag %d for this detector\n", flag);
                LOG(logERROR, (mess));
            }
        }
        if (ret == OK) {
            setExtSignal(signalIndex, flag);
            retval = getExtSignal(signalIndex);
            validate(&ret, mess, (int)flag, (int)retval,
                     "set external signal flag", DEC);
            LOG(logDEBUG1, ("External Signal Flag: %d\n", retval));
        }
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_timing_mode(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum timingMode arg = AUTO_TIMING;
    enum timingMode retval = AUTO_TIMING;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting external communication mode to %d\n", arg));

    // set
    if (((int)arg != GET_FLAG) && (Server_VerifyLock() == OK)) {
        switch (arg) {
        case AUTO_TIMING:
        case TRIGGER_EXPOSURE:
#ifdef EIGERD
        case GATED:
        case BURST_TRIGGER:
#elif MYTHEN3D
        case GATED:
        case TRIGGER_GATED:
#endif
            setTiming(arg);
            break;
        default:
            modeNotImplemented("Timing mode", (int)arg);
            break;
        }
    }
    // get
    retval = getTiming();
#ifndef MYTHEN3D
    validate(&ret, mess, (int)arg, (int)retval, "set timing mode", DEC);
#endif
    LOG(logDEBUG1, ("Timing Mode: %d\n", retval));

    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int get_firmware_version(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;
    retval = getFirmwareVersion();
    if (retval == 0) {
        ret = FAIL;
        strcpy(mess, "Could not get firmware version\n");
        LOG(logERROR, (mess));
    } else {
        LOG(logDEBUG1,
            ("firmware version retval: 0x%llx\n", (long long int)retval));
    }
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int get_server_version(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    char retvals[MAX_STR_LENGTH];
    memset(retvals, 0, MAX_STR_LENGTH);
    getServerVersion(retvals);
    LOG(logDEBUG1, ("server version retval: %s\n", retvals));
    return Server_SendResult(file_des, OTHER, retvals, sizeof(retvals));
}

int get_serial_number(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;
#ifdef EIGERD
    functionNotImplemented();
#else
    retval = getDetectorNumber();
    LOG(logDEBUG1, ("detector number retval: 0x%llx\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_firmware_test(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    LOG(logDEBUG1, ("Executing firmware test\n"));

#if !defined(GOTTHARDD) && !defined(JUNGFRAUD) && !defined(MOENCHD) &&         \
    !defined(CHIPTESTBOARDD) && !defined(GOTTHARD2D) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    ret = testFpga();
    if (ret == FAIL) {
        strcpy(mess, "FPGA test failed\n");
        LOG(logERROR, (mess));
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int set_bus_test(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    LOG(logDEBUG1, ("Executing bus test\n"));

#if !defined(GOTTHARDD) && !defined(JUNGFRAUD) && !defined(MOENCHD) &&         \
    !defined(CHIPTESTBOARDD) && !defined(GOTTHARD2D) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    ret = testBus();
    if (ret == FAIL) {
        strcpy(mess, "Bus test failed\n");
        LOG(logERROR, (mess));
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int set_image_test_mode(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting image test mode to \n", arg));

#if defined(GOTTHARDD) ||                                                      \
    ((defined(EIGERD) || defined(JUNGFRAUD) || defined(MOENCHD)) &&            \
     defined(VIRTUAL))
    setTestImageMode(arg);
#else
    functionNotImplemented();
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_image_test_mode(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;
    LOG(logDEBUG1, ("Getting image test mode\n"));

#if defined(GOTTHARDD) ||                                                      \
    ((defined(EIGERD) || defined(JUNGFRAUD) || defined(MOENCHD)) &&            \
     defined(VIRTUAL))
    retval = getTestImageMode();
    LOG(logDEBUG1, ("image test mode retval: %d\n", retval));
#else
    functionNotImplemented();
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

enum DACINDEX getDACIndex(enum dacIndex ind) {
    enum DACINDEX serverDacIndex = -1;
    // check if dac exists for this detector
    switch (ind) {
#ifdef GOTTHARDD
    case VREF_DS:
        serverDacIndex = G_VREF_DS;
        break;
    case VCASCN_PB:
        serverDacIndex = G_VCASCN_PB;
        break;
    case VCASCP_PB:
        serverDacIndex = G_VCASCP_PB;
        break;
    case VOUT_CM:
        serverDacIndex = G_VOUT_CM;
        break;
    case VCASC_OUT:
        serverDacIndex = G_VCASC_OUT;
        break;
    case VIN_CM:
        serverDacIndex = G_VIN_CM;
        break;
    case VREF_COMP:
        serverDacIndex = G_VREF_COMP;
        break;
    case IB_TESTC:
        serverDacIndex = G_IB_TESTC;
        break;
#elif EIGERD
    case VTHRESHOLD:
        serverDacIndex = E_VTHRESHOLD;
        break;
    case VSVP:
        serverDacIndex = E_VSVP;
        break;
    case VSVN:
        serverDacIndex = E_VSVN;
        break;
    case VTRIM:
        serverDacIndex = E_VTRIM;
        break;
    case VRPREAMP:
        serverDacIndex = E_VRPREAMP;
        break;
    case VRSHAPER:
        serverDacIndex = E_VRSHAPER;
        break;
    case VTGSTV:
        serverDacIndex = E_VTGSTV;
        break;
    case VCMP_LL:
        serverDacIndex = E_VCMP_LL;
        break;
    case VCMP_LR:
        serverDacIndex = E_VCMP_LR;
        break;
    case VCAL:
        serverDacIndex = E_VCAL;
        break;
    case VCMP_RL:
        serverDacIndex = E_VCMP_RL;
        break;
    case VCMP_RR:
        serverDacIndex = E_VCMP_RR;
        break;
    case RXB_RB:
        serverDacIndex = E_RXB_RB;
        break;
    case RXB_LB:
        serverDacIndex = E_RXB_LB;
        break;
    case VCP:
        serverDacIndex = E_VCP;
        break;
    case VCN:
        serverDacIndex = E_VCN;
        break;
    case VISHAPER:
        serverDacIndex = E_VISHAPER;
        break;
#elif CHIPTESTBOARDD
    case V_POWER_A:
        serverDacIndex = D_PWR_A;
        break;
    case V_POWER_B:
        serverDacIndex = D_PWR_B;
        break;
    case V_POWER_C:
        serverDacIndex = D_PWR_C;
        break;
    case V_POWER_D:
        serverDacIndex = D_PWR_D;
        break;
    case V_POWER_IO:
        serverDacIndex = D_PWR_IO;
        break;
    case V_POWER_CHIP:
        serverDacIndex = D_PWR_CHIP;
        break;
#elif MYTHEN3D
    case VCASSH:
        serverDacIndex = M_VCASSH;
        break;
    case VTH2:
        serverDacIndex = M_VTH2;
        break;
    case VRSHAPER:
        serverDacIndex = M_VRSHAPER;
        break;
    case VRSHAPER_N:
        serverDacIndex = M_VRSHAPER_N;
        break;
    case VIPRE_OUT:
        serverDacIndex = M_VIPRE_OUT;
        break;
    case VTH3:
        serverDacIndex = M_VTH3;
        break;
    case VTH1:
        serverDacIndex = M_VTH1;
        break;
    case VICIN:
        serverDacIndex = M_VICIN;
        break;
    case VCAS:
        serverDacIndex = M_VCAS;
        break;
    case VRPREAMP:
        serverDacIndex = M_VRPREAMP;
        break;
    case VCAL_P:
        serverDacIndex = M_VCAL_P;
        break;
    case VIPRE:
        serverDacIndex = M_VIPRE;
        break;
    case VISHAPER:
        serverDacIndex = M_VISHAPER;
        break;
    case VCAL_N:
        serverDacIndex = M_VCAL_N;
        break;
    case VTRIM:
        serverDacIndex = M_VTRIM;
        break;
    case VDCSH:
        serverDacIndex = M_VDCSH;
        break;
    case VTHRESHOLD:
        serverDacIndex = M_VTHRESHOLD;
        break;
#elif GOTTHARD2D
    case HIGH_VOLTAGE:
        break;
    case VREF_H_ADC:
        serverDacIndex = G2_VREF_H_ADC;
        break;
    case VB_COMP_FE:
        serverDacIndex = G2_VB_COMP_FE;
        break;
    case VB_COMP_ADC:
        serverDacIndex = G2_VB_COMP_ADC;
        break;
    case VCOM_CDS:
        serverDacIndex = G2_VCOM_CDS;
        break;
    case VREF_RSTORE:
        serverDacIndex = G2_VREF_RSTORE;
        break;
    case VB_OPA_1ST:
        serverDacIndex = G2_VB_OPA_1ST;
        break;
    case VREF_COMP_FE:
        serverDacIndex = G2_VREF_COMP_FE;
        break;
    case VCOM_ADC1:
        serverDacIndex = G2_VCOM_ADC1;
        break;
    case VREF_PRECH:
        serverDacIndex = G2_VREF_PRECH;
        break;
    case VREF_L_ADC:
        serverDacIndex = G2_VREF_L_ADC;
        break;
    case VREF_CDS:
        serverDacIndex = G2_VREF_CDS;
        break;
    case VB_CS:
        serverDacIndex = G2_VB_CS;
        break;
    case VB_OPA_FD:
        serverDacIndex = G2_VB_OPA_FD;
        break;
    case VCOM_ADC2:
        serverDacIndex = G2_VCOM_ADC2;
        break;
#elif defined(JUNGFRAUD) || defined(MOENCHD)
    case HIGH_VOLTAGE:
        break;
    case VB_COMP:
        serverDacIndex = J_VB_COMP;
        break;
    case VDD_PROT:
        serverDacIndex = J_VDD_PROT;
        break;
    case VIN_COM:
        serverDacIndex = J_VIN_COM;
        break;
    case VREF_PRECH:
        serverDacIndex = J_VREF_PRECH;
        break;
    case VB_PIXBUF:
        serverDacIndex = J_VB_PIXBUF;
        break;
    case VB_DS:
        serverDacIndex = J_VB_DS;
        break;
    case VREF_DS:
        serverDacIndex = J_VREF_DS;
        break;
    case VREF_COMP:
        serverDacIndex = J_VREF_COMP;
        break;
#endif

    default:
#ifdef CHIPTESTBOARDD
        if (ind < NDAC_ONLY) {
            // For CTB use the index directly, no conversion
            serverDacIndex = (enum DACINDEX)ind;
            break;
        }
#endif
        modeNotImplemented("Dac Index", (int)ind);
        break;
    }
    return serverDacIndex;
}

int validateAndSetDac(enum dacIndex ind, int val, int mV) {
    int retval = -1;
    enum DACINDEX serverDacIndex = 0;

    // valid enums
    switch (ind) {
    case HIGH_VOLTAGE:
#ifdef EIGERD
    case IO_DELAY:
#elif CHIPTESTBOARDD
    case ADC_VPP:
    case V_LIMIT:
#endif
        break;
    default:
        serverDacIndex = getDACIndex(ind);
        break;
    }
    if (ret == FAIL) {
        return retval;
    }
    switch (ind) {
        // adc vpp
#if defined(CHIPTESTBOARDD)
    case ADC_VPP:
        // set
        if (val >= 0) {
            ret = AD9257_SetVrefVoltage(val, mV);
            if (ret == FAIL) {
                sprintf(mess, "Could not set Adc Vpp. Please set a "
                              "proper value\n");
                LOG(logERROR, (mess));
            }
        }
        retval = AD9257_GetVrefVoltage(mV);
        LOG(logDEBUG1,
            ("Adc Vpp retval: %d %s\n", retval, (mV ? "mV" : "mode")));
        // cannot validate (its just a variable and mv gives different
        // value)
        break;
#endif

        // io delay
#ifdef EIGERD
    case IO_DELAY:
        retval = setIODelay(val);
        LOG(logDEBUG1, ("IODelay: %d\n", retval));
        validate(&ret, mess, val, retval, "set iodelay", DEC);
        break;
#endif

    // high voltage
    case HIGH_VOLTAGE:
        retval = setHighVoltage(val);
        LOG(logDEBUG1, ("High Voltage: %d\n", retval));
#if defined(JUNGFRAUD) || defined(MOENCHD) || defined(CHIPTESTBOARDD) ||       \
    defined(GOTTHARD2D) || defined(MYTHEN3D)
        validate(&ret, mess, val, retval, "set high voltage", DEC);
#endif
#ifdef GOTTHARDD
        if (retval == -1) {
            ret = FAIL;
            strcpy(mess, "Invalid Voltage. Valid values are 0, 90, "
                         "110, 120, 150, 180, 200\n");
            LOG(logERROR, (mess));
        } else
            validate(&ret, mess, val, retval, "set high voltage", DEC);
#elif EIGERD
        if ((retval != SLAVE_HIGH_VOLTAGE_READ_VAL) && (retval < 0)) {
            ret = FAIL;
            if (retval == -1)
                sprintf(mess,
                        "Setting high voltage failed. Bad value %d. "
                        "The range is from 0 to 200 V.\n",
                        val);
            else if (retval == -2)
                strcpy(mess, "Setting high voltage failed. "
                             "Serial/i2c communication failed.\n");
            else if (retval == -3)
                strcpy(mess, "Getting high voltage failed. "
                             "Serial/i2c communication failed.\n");
            LOG(logERROR, (mess));
        }
#endif
        break;

        // power, vlimit
#ifdef CHIPTESTBOARDD
    case V_POWER_A:
    case V_POWER_B:
    case V_POWER_C:
    case V_POWER_D:
    case V_POWER_IO:
        if (val != GET_FLAG) {
            if (!mV) {
                ret = FAIL;
                sprintf(mess,
                        "Could not set power. Power regulator %d "
                        "should be in mV and not dac units.\n",
                        ind);
                LOG(logERROR, (mess));
            } else if (checkVLimitCompliant(val) == FAIL) {
                ret = FAIL;
                sprintf(mess,
                        "Could not set power. Power regulator %d "
                        "exceeds voltage limit %d.\n",
                        ind, getVLimit());
                LOG(logERROR, (mess));
            } else if (!isPowerValid(serverDacIndex, val)) {
                ret = FAIL;
                sprintf(
                    mess,
                    "Could not set power. Power regulator %d "
                    "should be between %d and %d mV\n",
                    ind,
                    (serverDacIndex == D_PWR_IO ? VIO_MIN_MV : POWER_RGLTR_MIN),
                    (VCHIP_MAX_MV - VCHIP_POWER_INCRMNT));
                LOG(logERROR, (mess));
            } else {
                setPower(serverDacIndex, val);
            }
        }
        retval = getPower(serverDacIndex);
        LOG(logDEBUG1, ("Power regulator(%d): %d\n", ind, retval));
        validate(&ret, mess, val, retval, "set power regulator", DEC);
        break;

    case V_POWER_CHIP:
        if (val >= 0) {
            ret = FAIL;
            sprintf(mess, "Can not set Vchip. Can only be set "
                          "automatically in the background (+200mV "
                          "from highest power regulator voltage).\n");
            LOG(logERROR, (mess));
            /* restrict users from setting vchip
            if (!mV) {
                ret = FAIL;
                sprintf(mess,"Could not set Vchip. Should be in mV and
            not dac units.\n"); LOG(logERROR,(mess)); } else if
            (!isVchipValid(val)) { ret = FAIL; sprintf(mess,"Could not
            set Vchip. Should be between %d and %d mV\n", VCHIP_MIN_MV,
            VCHIP_MAX_MV); LOG(logERROR,(mess)); } else { setVchip(val);
            }
            */
        }
        retval = getVchip();
        LOG(logDEBUG1, ("Vchip: %d\n", retval));
        if (ret == OK && val != GET_FLAG && val != -100 && retval != val) {
            ret = FAIL;
            sprintf(mess, "Could not set vchip. Set %d, but read %d\n", val,
                    retval);
            LOG(logERROR, (mess));
        }
        break;
#endif

#if defined(CHIPTESTBOARDD)
    case V_LIMIT:
        if (val >= 0) {
            if (!mV) {
                ret = FAIL;
                strcpy(mess, "Could not set power. VLimit should be in "
                             "mV and not dac units.\n");
                LOG(logERROR, (mess));
            } else {
                setVLimit(val);
            }
        }
        retval = getVLimit();
        LOG(logDEBUG1, ("VLimit: %d\n", retval));
        validate(&ret, mess, val, retval, "set vlimit", DEC);
        break;
#endif
        // dacs
    default:
        if (mV && val > DAC_MAX_MV) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set dac %d to value %d. Allowed limits "
                    "(0 - %d mV).\n",
                    ind, val, DAC_MAX_MV);
            LOG(logERROR, (mess));
        } else if (!mV && val > getMaxDacSteps()) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set dac %d to value %d. Allowed limits "
                    "(0 - %d dac units).\n",
                    ind, val, getMaxDacSteps());
            LOG(logERROR, (mess));
        } else {
#if defined(CHIPTESTBOARDD)
            if ((val != GET_FLAG && mV && checkVLimitCompliant(val) == FAIL) ||
                (val != GET_FLAG && !mV &&
                 checkVLimitDacCompliant(val) == FAIL)) {
                ret = FAIL;
                sprintf(mess,
                        "Could not set dac %d to value %d. "
                        "Exceeds voltage limit %d.\n",
                        ind, (mV ? val : dacToVoltage(val)), getVLimit());
                LOG(logERROR, (mess));
            } else
#endif
#ifdef MYTHEN3D
                // ignore counter enable to force vth dac values
                setDAC(serverDacIndex, val, mV, 0);
#else
            setDAC(serverDacIndex, val, mV);
#endif
            retval = getDAC(serverDacIndex, mV);
        }
#ifdef EIGERD
        if (val != GET_FLAG && getSettings() != UNDEFINED) {
            // changing dac changes settings to undefined
            switch (serverDacIndex) {
            case E_VCMP_LL:
            case E_VCMP_LR:
            case E_VCMP_RL:
            case E_VCMP_RR:
            case E_VRPREAMP:
            case E_VCP:
                setSettings(UNDEFINED);
                LOG(logERROR, ("Settings has been changed "
                               "to undefined (changed specific dacs)\n"));
                break;
            default:
                break;
            }
        }
#endif
        // check
        if (ret == OK) {
            if ((abs(retval - val) <= 5) || val == GET_FLAG) {
                ret = OK;
            } else {
                ret = FAIL;
                sprintf(mess, "Setting dac %d : wrote %d but read %d\n",
                        serverDacIndex, val, retval);
                LOG(logERROR, (mess));
            }
        }
        LOG(logDEBUG1, ("Dac (%d): %d %s\n\n", serverDacIndex, retval,
                        (mV ? "mV" : "dac units")));
#ifdef MYTHEN3D
        // changed for setsettings (direct),
        // custom trimbit file (setmodule with myMod.reg as -1),
        // change of dac (direct)
        if (val != GET_FLAG && ret == OK) {
            for (int i = 0; i < NCOUNTERS; ++i) {
                setThresholdEnergy(i, -1);
            }
        }
#endif
        break;
    }
    return retval;
}

int set_dac(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[3] = {-1, -1, -1};
    int retval = -1;

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();

    enum dacIndex ind = args[0];
    int mV = args[1];
    int val = args[2];

    LOG(logDEBUG1,
        ("Setting DAC %d to %d %s\n", ind, val, (mV ? "mV" : "dac units")));
    // set & get
    if ((val == GET_FLAG) || (Server_VerifyLock() == OK)) {
        retval = validateAndSetDac(ind, val, mV);
    }
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int get_adc(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum dacIndex ind = 0;
    int retval = -1;

    if (receiveData(file_des, &ind, sizeof(ind), INT32) < 0)
        return printSocketReadError();

    enum ADCINDEX serverAdcIndex = 0;

    // get
    switch (ind) {
#if defined(MYTHEN3D) || defined(GOTTHARD2D)
    case TEMPERATURE_FPGA:
        serverAdcIndex = TEMP_FPGA;
        break;
#endif
#if defined(GOTTHARDD) || defined(JUNGFRAUD) || defined(MOENCHD)
    case TEMPERATURE_FPGA:
        serverAdcIndex = TEMP_FPGA;
        break;
    case TEMPERATURE_ADC:
        serverAdcIndex = TEMP_ADC;
        break;
#elif EIGERD
    case TEMPERATURE_FPGAEXT:
        serverAdcIndex = TEMP_FPGAEXT;
        break;
    case TEMPERATURE_10GE:
        serverAdcIndex = TEMP_10GE;
        break;
    case TEMPERATURE_DCDC:
        serverAdcIndex = TEMP_DCDC;
        break;
    case TEMPERATURE_SODL:
        serverAdcIndex = TEMP_SODL;
        break;
    case TEMPERATURE_SODR:
        serverAdcIndex = TEMP_SODR;
        break;
    case TEMPERATURE_FPGA:
        serverAdcIndex = TEMP_FPGA;
        break;
    case TEMPERATURE_FPGA2:
        serverAdcIndex = TEMP_FPGAFEBL;
        break;
    case TEMPERATURE_FPGA3:
        serverAdcIndex = TEMP_FPGAFEBR;
        break;
#elif CHIPTESTBOARDD
    case V_POWER_A:
        serverAdcIndex = V_PWR_A;
        break;
    case V_POWER_B:
        serverAdcIndex = V_PWR_B;
        break;
    case V_POWER_C:
        serverAdcIndex = V_PWR_C;
        break;
    case V_POWER_D:
        serverAdcIndex = V_PWR_D;
        break;
    case V_POWER_IO:
        serverAdcIndex = V_PWR_IO;
        break;
    case I_POWER_A:
        serverAdcIndex = I_PWR_A;
        break;
    case I_POWER_B:
        serverAdcIndex = I_PWR_B;
        break;
    case I_POWER_C:
        serverAdcIndex = I_PWR_C;
        break;
    case I_POWER_D:
        serverAdcIndex = I_PWR_D;
        break;
    case I_POWER_IO:
        serverAdcIndex = I_PWR_IO;
        break;
    case SLOW_ADC0:
        serverAdcIndex = S_ADC0;
        break;
    case SLOW_ADC1:
        serverAdcIndex = S_ADC1;
        break;
    case SLOW_ADC2:
        serverAdcIndex = S_ADC2;
        break;
    case SLOW_ADC3:
        serverAdcIndex = S_ADC3;
        break;
    case SLOW_ADC4:
        serverAdcIndex = S_ADC4;
        break;
    case SLOW_ADC5:
        serverAdcIndex = S_ADC5;
        break;
    case SLOW_ADC6:
        serverAdcIndex = S_ADC6;
        break;
    case SLOW_ADC7:
        serverAdcIndex = S_ADC7;
        break;
    case SLOW_ADC_TEMP:
        serverAdcIndex = S_TMP;
        break;
#endif
    default:
        modeNotImplemented("Adc Index", (int)ind);
        break;
    }

    // valid index
    if (ret == OK) {
        LOG(logDEBUG1, ("Getting ADC %d\n", serverAdcIndex));
#if defined(MYTHEN3D) || defined(GOTTHARD2D)
        ret = getADC(serverAdcIndex, &retval);
        if (ret == FAIL) {
            strcpy(mess, "Could not get temperature\n");
            LOG(logERROR, (mess));
        } else {
            LOG(logDEBUG1, ("ADC(%d): %d\n", serverAdcIndex, retval));
        }
#else
        retval = getADC(serverAdcIndex);
        LOG(logDEBUG1, ("ADC(%d): %d\n", serverAdcIndex, retval));
#endif
    }

    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int write_register(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t args[2] = {-1, -1};
    uint32_t retval = -1;

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    uint32_t addr = args[0];
    uint32_t val = args[1];
    LOG(logDEBUG1, ("Writing to register 0x%x, data 0x%x\n", addr, val));

    // only set
    if (Server_VerifyLock() == OK) {
#ifdef GOTTHARDD
        retval = writeRegister16And32(addr, val);
#elif EIGERD
        if (writeRegister(addr, val) == FAIL) {
            ret = FAIL;
            sprintf(mess, "Could not write to register 0x%x.\n", addr);
            LOG(logERROR, (mess));
        } else {
            if (readRegister(addr, &retval) == FAIL) {
                ret = FAIL;
                sprintf(
                    mess,
                    "Could not read register 0x%x or inconsistent values. Try "
                    "to read +0x100 for only left and +0x200 for only right.\n",
                    addr);
                LOG(logERROR, (mess));
            }
        }
#else
        retval = writeRegister(addr, val);
#endif
        // validate
        if (ret == OK && retval != val) {
            ret = FAIL;
            sprintf(
                mess,
                "Could not write to register 0x%x. Wrote 0x%x but read 0x%x\n",
                addr, val, retval);
            LOG(logERROR, (mess));
        }
        LOG(logDEBUG1, ("Write register (0x%x): 0x%x\n", retval));
    }
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int read_register(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t addr = -1;
    uint32_t retval = -1;

    if (receiveData(file_des, &addr, sizeof(addr), INT32) < 0)
        return printSocketReadError();

    LOG(logDEBUG1, ("Reading from register 0x%x\n", addr));

    // get
#ifdef GOTTHARDD
    retval = readRegister16And32(addr);
#elif EIGERD
    if (readRegister(addr, &retval) == FAIL) {
        ret = FAIL;
        sprintf(mess,
                "Could not read register 0x%x or inconsistent values. Try "
                "+0x100 for only left and +0x200 for only right..\n",
                addr);
        LOG(logERROR, (mess));
    }
#else
    retval = readRegister(addr);
#endif
    LOG(logINFO, ("Read register (0x%x): 0x%x\n", addr, retval));

    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int get_module(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

    sls_detector_module module;
    int *myDac = NULL;
    int *myChan = NULL;
    module.dacs = NULL;
    module.chanregs = NULL;

#if !defined(MYTHEN3D) && !defined(EIGERD)
    functionNotImplemented();
#else

    // allocate to receive module structure
    // allocate dacs
    myDac = malloc(getNumberOfDACs() * sizeof(int));
    // error
    if (getNumberOfDACs() > 0 && myDac == NULL) {
        ret = FAIL;
        sprintf(mess, "Could not allocate dacs\n");
        LOG(logERROR, (mess));
    } else
        module.dacs = myDac;

    // allocate chans
    if (ret == OK) {
        myChan = malloc(getTotalNumberOfChannels() * sizeof(int));
        if (getTotalNumberOfChannels() > 0 && myChan == NULL) {
            ret = FAIL;
            strcpy(mess, "Could not allocate chans\n");
            LOG(logERROR, (mess));
        } else
            module.chanregs = myChan;
    }

    // receive module structure
    if (ret == OK) {
        module.nchip = getNumberOfChips();
        module.nchan = getTotalNumberOfChannels();
        module.ndac = getNumberOfDACs();

        // ensure nchan is not 0, else trimbits not copied
        if (module.nchan == 0) {
            strcpy(mess, "Could not get module as the number of channels to "
                         "copy is 0\n");
            LOG(logERROR, (mess));
            return FAIL;
        }
        getModule(&module);
    }
#endif
    Server_SendResult(file_des, INT32, NULL, 0);
    if (ret != FAIL) {
        if (sendModule(file_des, &module) < 0) {
            ret = FAIL;
            strcpy(mess, "Could not send module data\n");
            LOG(logERROR, (mess));
        }
    }
    if (myChan != NULL)
        free(myChan);
    if (myDac != NULL)
        free(myDac);
    return ret;
}

int set_module(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

#if !(defined(MYTHEN3D) || defined(EIGERD))
    functionNotImplemented();
#else

    sls_detector_module module;
    int *myDac = NULL;
    int *myChan = NULL;
    module.dacs = NULL;
    module.chanregs = NULL;

    // allocate to receive arguments
    // allocate dacs
    myDac = malloc(getNumberOfDACs() * sizeof(int));
    // error
    if (getNumberOfDACs() > 0 && myDac == NULL) {
        ret = FAIL;
        strcpy(mess, "Could not allocate dacs\n");
        LOG(logERROR, (mess));
    } else
        module.dacs = myDac;

    // allocate chans
    if (ret == OK) {
        myChan = malloc(getTotalNumberOfChannels() * sizeof(int));
        if (getTotalNumberOfChannels() > 0 && myChan == NULL) {
            ret = FAIL;
            strcpy(mess, "Could not allocate chans\n");
            LOG(logERROR, (mess));
        } else
            module.chanregs = myChan;
    }
    // receive arguments
    if (ret == OK) {
        module.nchip = getNumberOfChips();
        module.nchan = getTotalNumberOfChannels();
        module.ndac = getNumberOfDACs();
        int ts = receiveModule(file_des, &module);
        if (ts < 0) {
            free(myChan);
            free(myDac);
            return printSocketReadError();
        }
        LOG(logDEBUG1, ("module register is %d, nchan %d, nchip %d, "
                        "ndac %d, iodelay %d, tau %d, eV %d\n",
                        module.reg, module.nchan, module.nchip, module.ndac,
                        module.iodelay, module.tau, module.eV[0]));
        // should at least have a dac
        if (ts <= (int)sizeof(sls_detector_module)) {
            ret = FAIL;
            strcpy(mess, "Cannot set module. Received incorrect number of "
                         "dacs or channels\n");
            LOG(logERROR, (mess));
        }
    }

    // only set
    if (ret == OK && Server_VerifyLock() == OK) {
        // check index

// setsettings
#ifndef MYTHEN3D
        // m3 uses reg for chip (not settings)
        validate_settings((enum detectorSettings)(module.reg));
#endif
        ret = setModule(module, mess);
        enum detectorSettings retval = getSettings();
#ifndef MYTHEN3D
        validate(&ret, mess, module.reg, (int)retval, "set module (settings)",
                 DEC);
#endif
        LOG(logDEBUG1, ("Settings: %d\n", retval));
    }
    if (myChan != NULL)
        free(myChan);
    if (myDac != NULL)
        free(myDac);
#endif

    return Server_SendResult(file_des, INT32, NULL, 0);
}

void validate_settings(enum detectorSettings sett) {
    // check index
    switch (sett) {
#ifdef EIGERD
    case STANDARD:
#elif defined(JUNGFRAUD)
    case GAIN0:
    case HIGHGAIN0:
#elif GOTTHARDD
    case DYNAMICGAIN:
    case HIGHGAIN:
    case LOWGAIN:
    case MEDIUMGAIN:
    case VERYHIGHGAIN:
#elif GOTTHARD2D
    case DYNAMICGAIN:
    case FIXGAIN1:
    case FIXGAIN2:
#elif MYTHEN3D
    case STANDARD:
    case FAST:
    case HIGHGAIN:
#endif
        break;
    default:
        modeNotImplemented("Settings Index", (int)sett);
        break;
    }
}

int set_settings(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum detectorSettings isett = STANDARD;
    enum detectorSettings retval = STANDARD;

    if (receiveData(file_des, &isett, sizeof(isett), INT32) < 0)
        return printSocketReadError();

#ifdef CHIPTESTBOARDD
    functionNotImplemented();
#else
    LOG(logDEBUG1, ("Setting settings %d\n", isett));

    // set & get
    if (((int)isett == GET_FLAG) || (Server_VerifyLock() == OK)) {

        if ((int)isett != GET_FLAG) {
#ifdef EIGERD
            ret = FAIL;
            strcpy(mess, "Cannot set settings via SET_SETTINGS, use "
                         "SET_MODULE\n");
            LOG(logERROR, (mess));
#else
            validate_settings(isett);
#endif
            if (ret == OK) {
                setSettings(isett);
            }
        }
        retval = getSettings();
        LOG(logDEBUG1, ("Settings: %d\n", retval));

        if ((int)isett != GET_FLAG) {
            validate(&ret, mess, (int)isett, (int)retval, "set settings", DEC);
#ifdef GOTTHARDD
            if (ret == OK) {
                ret = resetToDefaultDacs(0);
                if (ret == FAIL) {
                    strcpy(mess, "Could change settings, but could not set to "
                                 "default dacs\n");
                    LOG(logERROR, (mess));
                }
            }
#endif
#ifdef MYTHEN3D
            // changed for setsettings (direct),
            // custom trimbit file (setmodule with myMod.reg as -1),
            // change of dac (direct)
            if (ret == OK) {
                for (int i = 0; i < NCOUNTERS; ++i) {
                    setThresholdEnergy(i, -1);
                }
            }
#endif
        }
    }
#endif

    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int get_threshold_energy(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting Threshold energy\n"));
#ifndef EIGERD
    functionNotImplemented();
#else
    // only get
    retval = getThresholdEnergy();
    LOG(logDEBUG1, ("Threshold energy: %d eV\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int acquire(int blocking, int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    if (blocking) {
        LOG(logINFOBLUE, ("Blocking Acquisition\n"));
    } else {
        LOG(logINFOBLUE, ("Unblocking Acquisition\n"));
    }
    // only set
    if (Server_VerifyLock() == OK) {
#if defined(JUNGFRAUD)
        // chipv1.1 has to be configured before acquisition
        if (getChipVersion() == 11 && !isChipConfigured()) {
            ret = FAIL;
            strcpy(mess, "Could not start acquisition. Chip is not configured. "
                         "Power it on to configure it.\n");
            LOG(logERROR, (mess));
        } else
#endif
#ifdef CHIPTESTBOARDD
            if ((getReadoutMode() == ANALOG_AND_DIGITAL ||
                 getReadoutMode() == ANALOG_ONLY) &&
                (getNumAnalogSamples() <= 0)) {
            ret = FAIL;
            sprintf(mess,
                    "Could not start acquisition. Invalid number of analog "
                    "samples: %d.\n",
                    getNumAnalogSamples());
            LOG(logERROR, (mess));
        } else if ((getReadoutMode() == ANALOG_AND_DIGITAL ||
                    getReadoutMode() == DIGITAL_ONLY) &&
                   (getNumDigitalSamples() <= 0)) {
            ret = FAIL;
            sprintf(mess,
                    "Could not start acquisition. Invalid number of digital "
                    "samples: %d.\n",
                    getNumDigitalSamples());
            LOG(logERROR, (mess));
        } else
#endif
#ifdef EIGERD
            // check for hardware mac and hardware ip
            if (udpDetails[0].srcmac != getDetectorMAC()) {
            ret = FAIL;
            uint64_t sourcemac = getDetectorMAC();
            char src_mac[MAC_ADDRESS_SIZE];
            getMacAddressinString(src_mac, MAC_ADDRESS_SIZE, sourcemac);
            sprintf(mess,
                    "Invalid udp source mac address for this detector. Must be "
                    "same as hardware detector mac address %s\n",
                    src_mac);
            LOG(logERROR, (mess));
        } else if (!enableTenGigabitEthernet(GET_FLAG) &&
                   (udpDetails[0].srcip != getDetectorIP())) {
            ret = FAIL;
            uint32_t sourceip = getDetectorIP();
            char src_ip[INET_ADDRSTRLEN];
            getIpAddressinString(src_ip, sourceip);
            sprintf(mess,
                    "Invalid udp source ip address for this detector. Must be "
                    "same as hardware detector ip address %s in 1G readout "
                    "mode \n",
                    src_ip);
            LOG(logERROR, (mess));
        } else
#endif
            if (configured == FAIL) {
            ret = FAIL;
            strcpy(mess, "Could not start acquisition because ");
            strcat(mess, configureMessage);
            LOG(logERROR, (mess));
        } else if (sharedMemory_getScanStatus() == RUNNING) {
            ret = FAIL;
            strcpy(mess, "Could not start acquisition because a scan is "
                         "already running!\n");
            LOG(logERROR, (mess));
        } else {
            memset(scanErrMessage, 0, MAX_STR_LENGTH);
            sharedMemory_setScanStop(0);
            sharedMemory_setScanStatus(IDLE); // if it was error
            if (pthread_create(&pthread_tid, NULL, &start_state_machine,
                               &blocking)) {
                ret = FAIL;
                strcpy(mess, "Could not start acquisition thread!\n");
                LOG(logERROR, (mess));
            } else {
                // only does not wait for non blocking and scan
                if (blocking || !scan) {
                    pthread_join(pthread_tid, NULL);
                }
            }
        }
    }
    return Server_SendResult(file_des, INT32, NULL, 0);
}

void *start_state_machine(void *arg) {
    int *blocking = (int *)arg;
    int times = 1;
    // start of scan
    if (scan) {
        sharedMemory_setScanStatus(RUNNING);
        times = numScanSteps;
    }
    for (int i = 0; i != times; ++i) {
        // normal acquisition
        if (scan == 0) {
            LOG(logINFOBLUE, ("Normal Acquisition (not scan)\n"));
        }
        // scan
        else {
            // check scan stop
            if (sharedMemory_getScanStop()) {
                LOG(logINFORED, ("Scan manually stopped!\n"));
                sharedMemory_setScanStatus(IDLE);
                break;
            }
            // trimbits scan
            if (scanTrimbits) {
                LOG(logINFOBLUE,
                    ("Trimbits scan %d/%d: [%d]\n", i, times, scanSteps[i]));
                validateAndSetAllTrimbits(scanSteps[i]);
                if (ret == FAIL) {
                    sprintf(scanErrMessage, "Cannot scan trimbit %d. ",
                            scanSteps[i]);
                    strcat(scanErrMessage, mess);
                    sharedMemory_setScanStatus(ERROR);
                    break;
                }
            }
            // dac scan
            else {
                LOG(logINFOBLUE, ("Dac [%d] scan %d/%d: [%d]\n",
                                  scanGlobalIndex, i, times, scanSteps[i]));
                validateAndSetDac(scanGlobalIndex, scanSteps[i], 0);
                if (ret == FAIL) {
                    sprintf(scanErrMessage, "Cannot scan dac %d at %d. ",
                            scanGlobalIndex, scanSteps[i]);
                    strcat(scanErrMessage, mess);
                    sharedMemory_setScanStatus(ERROR);
                    break;
                }
            }
            // check scan stop
            if (sharedMemory_getScanStop()) {
                LOG(logINFORED, ("Scan manually stopped!\n"));
                sharedMemory_setScanStatus(IDLE);
                break;
            }
            usleep(scanSettleTime_ns / 1000);
        }
        ret = startStateMachine();
        LOG(logDEBUG2, ("Starting Acquisition ret: %d\n", ret));
        if (ret == FAIL) {
#if defined(CHIPTESTBOARDD) || defined(VIRTUAL)
            sprintf(mess, "Could not start acquisition. Could not create udp "
                          "socket in server. Check udp_dstip & udp_dstport.\n");
#else
            sprintf(mess, "Could not start acquisition\n");
#endif
            LOG(logERROR, (mess));
            if (scan) {
                sprintf(scanErrMessage, "Cannot scan at %d. ", scanSteps[i]);
                strcat(scanErrMessage, mess);
                sharedMemory_setScanStatus(ERROR);
            }
            break;
        }

#if defined(CHIPTESTBOARDD)
        readFrames(&ret, mess);
        if (ret == FAIL && scan) {
            sprintf(scanErrMessage, "Cannot scan at %d. ", scanSteps[i]);
            strcat(scanErrMessage, mess);
            sharedMemory_setScanStatus(ERROR);
            break;
        }
#endif
        // blocking or scan
        if (*blocking || times > 1) {
#ifdef EIGERD
            waitForAcquisitionEnd(&ret, mess);
            if (ret == FAIL && scan) {
                sprintf(scanErrMessage, "Cannot scan at %d. ", scanSteps[i]);
                strcat(scanErrMessage, mess);
                sharedMemory_setScanStatus(ERROR);
                break;
            }
#else
            waitForAcquisitionEnd();
#endif
        }
    }
    // end of scan
    if (scan && sharedMemory_getScanStatus() != ERROR) {
        sharedMemory_setScanStatus(IDLE);
    }
    return NULL;
}

int start_acquisition(int file_des) { return acquire(0, file_des); }

int stop_acquisition(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

    LOG(logDEBUG1, ("Stopping Acquisition\n"));
    // only set
    if (Server_VerifyLock() == OK) {
        ret = stopStateMachine();
        if (ret == FAIL) {
            sprintf(mess, "Could not stop acquisition\n");
            LOG(logERROR, (mess));
        }
        LOG(logDEBUG1, ("Stopping Acquisition ret: %d\n", ret));
    }
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_run_status(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum runStatus retval = ERROR;

    LOG(logDEBUG1, ("Getting status\n"));
    // only get
    retval = getRunStatus();
    LOG(logDEBUG1, ("Status: %d\n", retval));
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int start_and_read_all(int file_des) { return acquire(1, file_des); }

int get_num_frames(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

    // get only
    if (!scan) {
        retval = getNumFrames();
        LOG(logDEBUG1, ("retval num frames %lld\n", (long long int)retval));
    } else {
        retval = numScanSteps;
        LOG(logDEBUG1, ("retval num frames (num scan steps) %lld\n",
                        (long long int)retval));
    }
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_num_frames(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting number of frames %lld\n", (long long int)arg));

    // only set
    if (Server_VerifyLock() == OK) {
        // only set number of frames if normal mode (not scan)
        if (scan) {
            if (arg != numScanSteps) {
                ret = FAIL;
                sprintf(mess,
                        "Could not set number of frames %lld. In scan mode, it "
                        "is number of steps %d\n",
                        (long long unsigned int)arg, numScanSteps);
                LOG(logERROR, (mess));
            }
        } else {
#ifdef GOTTHARD2D
            // validate #frames in burst mode
            enum burstMode mode = getBurstMode();
            if ((mode == BURST_INTERNAL || mode == BURST_EXTERNAL) &&
                arg > MAX_FRAMES_IN_BURST_MODE) {
                ret = FAIL;
                sprintf(mess,
                        "Could not set number of frames %lld. Must be less "
                        "than equal to %d in "
                        "burst mode.\n",
                        (long long unsigned int)arg, MAX_FRAMES_IN_BURST_MODE);
                LOG(logERROR, (mess));
            }
#endif
            if (ret == OK) {
                setNumFrames(arg);
                int64_t retval = getNumFrames();
                LOG(logDEBUG1,
                    ("retval num frames %lld\n", (long long int)retval));
                validate64(&ret, mess, arg, retval, "set number of frames",
                           DEC);
            }
        }
    }
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_num_triggers(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

    // get only
    retval = getNumTriggers();
    LOG(logDEBUG1, ("retval num triggers %lld\n", (long long int)retval));
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_num_triggers(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting number of triggers %lld\n", (long long int)arg));

    // only set
    if (Server_VerifyLock() == OK) {
        setNumTriggers(arg);
        int64_t retval = getNumTriggers();
        LOG(logDEBUG1, ("retval num triggers %lld\n", (long long int)retval));
        validate64(&ret, mess, arg, retval, "set number of triggers", DEC);
    }
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_num_additional_storage_cells(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

#if !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // get only
    retval = getNumAdditionalStorageCells();
    LOG(logDEBUG1, ("retval num addl. storage cells %d\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_num_additional_storage_cells(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting number of addl. storage cells %d\n", arg));

#if !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (getChipVersion() == 11) {
            ret = FAIL;
            sprintf(mess,
                    "Cannot set addl. number of storage cells for chip v1.1\n");
            LOG(logERROR, (mess));
        } else if (arg > getMaxStoragecellStart()) {
            ret = FAIL;
            sprintf(mess, "Max Storage cell number should not exceed %d\n",
                    getMaxStoragecellStart());
            LOG(logERROR, (mess));
        } else {
            setNumAdditionalStorageCells(arg);
            int retval = getNumAdditionalStorageCells();
            LOG(logDEBUG1, ("retval num addl. storage cells %d\n", retval));
            validate(&ret, mess, arg, retval,
                     "set number of additional storage cells", DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_num_analog_samples(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

#if !defined(CHIPTESTBOARDD)
    functionNotImplemented();
#else
    // get only
    retval = getNumAnalogSamples();
    LOG(logDEBUG1, ("retval num analog samples %d\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_num_analog_samples(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting number of analog samples %d\n", arg));

#if !defined(CHIPTESTBOARDD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (ret == OK) {
            ret = setNumAnalogSamples(arg);
            if (ret == FAIL) {
                sprintf(mess,
                        "Could not set number of analog samples to %d. Could "
                        "not allocate RAM\n",
                        arg);
                LOG(logERROR, (mess));
            } else {
                int retval = getNumAnalogSamples();
                LOG(logDEBUG1, ("retval num analog samples %d\n", retval));
                validate(&ret, mess, arg, retval,
                         "set number of analog samples", DEC);
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_num_digital_samples(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

#if !defined(CHIPTESTBOARDD)
    functionNotImplemented();
#else
    // get only
    retval = getNumDigitalSamples();
    LOG(logDEBUG1, ("retval num digital samples %d\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_num_digital_samples(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting number of digital samples %d\n", arg));

#if !defined(CHIPTESTBOARDD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        ret = setNumDigitalSamples(arg);
        if (ret == FAIL) {
            sprintf(mess,
                    "Could not set number of digital samples to %d. Could not "
                    "allocate RAM\n",
                    arg);
            LOG(logERROR, (mess));
        } else {
            int retval = getNumDigitalSamples();
            LOG(logDEBUG1, ("retval num digital samples %d\n", retval));
            validate(&ret, mess, arg, retval, "set number of digital samples",
                     DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_exptime(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int gateIndex = -1;
    int64_t retval = -1;

    if (receiveData(file_des, &gateIndex, sizeof(gateIndex), INT32) < 0)
        return printSocketReadError();

        // get only
#ifdef MYTHEN3D
    if (gateIndex < 0 || gateIndex > 2) {
        ret = FAIL;
        sprintf(mess,
                "Could not get exposure time. Invalid gate index %d. "
                "Options [0-2]\n",
                gateIndex);
        LOG(logERROR, (mess));
    } else {
        retval = getExpTime(gateIndex);
        LOG(logDEBUG1, ("retval exptime %lld ns\n", (long long int)retval));
    }
#else
    if (gateIndex != -1) {
        ret = FAIL;
        sprintf(mess, "Could not get exposure time. Gate index not implemented "
                      "for this detector\n");
        LOG(logERROR, (mess));
    } else {
        retval = getExpTime();
        LOG(logDEBUG1, ("retval exptime %lld ns\n", (long long int)retval));
    }
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_exptime(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t args[2] = {-1, -1};

    if (receiveData(file_des, args, sizeof(args), INT64) < 0)
        return printSocketReadError();
    int gateIndex = args[0];
    int64_t val = args[1];
    LOG(logDEBUG1, ("Setting exptime %lld ns (gateIndex:%d)\n",
                    (long long int)val, gateIndex));

    // only set
    if (Server_VerifyLock() == OK) {
#ifdef MYTHEN3D
        if (gateIndex < -1 || gateIndex > 2) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set exposure time. Invalid gate index %d. "
                    "Options [-1, 0-2]\n",
                    gateIndex);
            LOG(logERROR, (mess));
        } else {
            // specific gate index
            if (gateIndex != -1) {
                ret = setExpTime(gateIndex, val);
                int64_t retval = getExpTime(gateIndex);
                LOG(logDEBUG1,
                    ("retval exptime %lld ns\n", (long long int)retval));
                if (ret == FAIL) {
                    sprintf(mess,
                            "Could not set exposure time. Set %lld ns, read "
                            "%lld ns.\n",
                            (long long int)val, (long long int)retval);
                    LOG(logERROR, (mess));
                }
            }
            // all gate indices
            else {
                for (int i = 0; i != 3; ++i) {
                    ret = setExpTime(i, val);
                    int64_t retval = getExpTime(i);
                    LOG(logDEBUG1, ("retval exptime %lld ns (index:%d)\n",
                                    (long long int)retval, i));
                    if (ret == FAIL) {
                        sprintf(mess,
                                "Could not set exptime. Set %lld ns, read %lld "
                                "ns.\n",
                                (long long int)val, (long long int)retval);
                        LOG(logERROR, (mess));
                        break;
                    }
                }
            }
        }
#else
        if (gateIndex != -1) {
            ret = FAIL;
            sprintf(mess,
                    "Could not get exposure time. Gate index not implemented "
                    "for this detector\n");
            LOG(logERROR, (mess));
        } else {
            ret = setExpTime(val);
            int64_t retval = getExpTime();
            LOG(logDEBUG1, ("retval exptime %lld ns\n", (long long int)retval));
            if (ret == FAIL) {
                sprintf(mess,
                        "Could not set exposure time. Set %lld ns, read "
                        "%lld ns.\n",
                        (long long int)val, (long long int)retval);
                LOG(logERROR, (mess));
            }
        }
#endif
    }
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_period(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

    // get only
    retval = getPeriod();
    LOG(logDEBUG1, ("retval period %lld ns\n", (long long int)retval));
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_period(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting period %lld ns\n", (long long int)arg));

    // only set
    if (Server_VerifyLock() == OK) {
        ret = setPeriod(arg);
        int64_t retval = getPeriod();
        LOG(logDEBUG1, ("retval period %lld ns\n", (long long int)retval));
        if (ret == FAIL) {
            sprintf(mess, "Could not set period. Set %lld ns, read %lld ns.\n",
                    (long long int)arg, (long long int)retval);
            LOG(logERROR, (mess));
        }
    }
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_delay_after_trigger(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(GOTTHARDD) &&         \
    !defined(CHIPTESTBOARDD) && !defined(MYTHEN3D) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // get only
    retval = getDelayAfterTrigger();
    LOG(logDEBUG1,
        ("retval delay after trigger %lld ns\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_delay_after_trigger(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logDEBUG1,
        ("Setting delay after trigger %lld ns\n", (long long int)arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(GOTTHARDD) &&         \
    !defined(CHIPTESTBOARDD) && !defined(MYTHEN3D) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        ret = setDelayAfterTrigger(arg);
        int64_t retval = getDelayAfterTrigger();
        LOG(logDEBUG1,
            ("retval delay after trigger %lld ns\n", (long long int)retval));
        if (ret == FAIL) {
            sprintf(mess,
                    "Could not set delay after trigger. Set %lld ns, read %lld "
                    "ns.\n",
                    (long long int)arg, (long long int)retval);
            LOG(logERROR, (mess));
        }
    }
#endif
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_sub_exptime(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#ifndef EIGERD
    functionNotImplemented();
#else
    // get only
    retval = getSubExpTime();
    LOG(logDEBUG1, ("retval subexptime %lld ns\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_sub_exptime(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting subexptime %lld ns\n", (long long int)arg));

#ifndef EIGERD
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (arg > ((int64_t)MAX_SUBFRAME_EXPOSURE_VAL_IN_10NS * 10)) {
            ret = FAIL;
            sprintf(mess,
                    "Sub Frame exposure time should not exceed %lf seconds\n",
                    ((double)((int64_t)MAX_SUBFRAME_EXPOSURE_VAL_IN_10NS * 10) /
                     (double)(1E9)));
            LOG(logERROR, (mess));
        } else {
            ret = setSubExpTime(arg);
            int64_t retval = getSubExpTime();
            LOG(logDEBUG1,
                ("retval subexptime %lld ns\n", (long long int)retval));
            if (ret == FAIL) {
                sprintf(mess,
                        "Could not set subframe exposure time. Set %lld ns, "
                        "read %lld ns.\n",
                        (long long int)arg, (long long int)retval);
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_sub_deadtime(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#ifndef EIGERD
    functionNotImplemented();
#else
    // get only
    retval = getSubDeadTime();
    LOG(logDEBUG1, ("retval subdeadtime %lld ns\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_sub_deadtime(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting subdeadtime %lld ns\n", (long long int)arg));

#ifndef EIGERD
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        int64_t subexptime = getSubExpTime();
        if ((arg + subexptime) >
            ((int64_t)MAX_SUBFRAME_EXPOSURE_VAL_IN_10NS * 10)) {
            ret = FAIL;
            sprintf(
                mess,
                "Sub Frame Period should not exceed %lf seconds. "
                "So sub frame dead time should not exceed %lf seconds "
                "(subexptime = %lf seconds)\n",
                ((double)((int64_t)MAX_SUBFRAME_EXPOSURE_VAL_IN_10NS * 10) /
                 (double)(1E9)),
                ((double)(((int64_t)MAX_SUBFRAME_EXPOSURE_VAL_IN_10NS * 10) -
                          subexptime) /
                 (double)1E9),
                ((double)subexptime / (double)1E9));
            LOG(logERROR, (mess));
        } else {
            ret = setSubDeadTime(arg);
            int64_t retval = getSubDeadTime();
            LOG(logDEBUG1,
                ("retval subdeadtime %lld ns\n", (long long int)retval));
            if (ret == FAIL) {
                sprintf(mess,
                        "Could not set subframe dead time. Set %lld ns, read "
                        "%lld ns.\n",
                        (long long int)arg, (long long int)retval);
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_storage_cell_delay(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#if !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // get only
    if (getChipVersion() == 11) {
        ret = FAIL;
        strcpy(mess, "Storage cell delay is not applicable for chipv 1.1\n");
        LOG(logERROR, (mess));
    } else {
        retval = getStorageCellDelay();
        LOG(logDEBUG1,
            ("retval storage cell delay %lld ns\n", (long long int)retval));
    }
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_storage_cell_delay(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logDEBUG1,
        ("Setting storage cell delay %lld ns\n", (long long int)arg));

#if !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (getChipVersion() == 11) {
            ret = FAIL;
            strcpy(mess,
                   "Storage cell delay is not applicable for chipv 1.1\n");
            LOG(logERROR, (mess));
        } else if (arg > MAX_STORAGE_CELL_DLY_NS_VAL) {
            ret = FAIL;
            sprintf(mess,
                    "Max Storage cell delay value should not exceed %lld ns\n",
                    (long long unsigned int)MAX_STORAGE_CELL_DLY_NS_VAL);
            LOG(logERROR, (mess));
        } else {
            ret = setStorageCellDelay(arg);
            int64_t retval = getStorageCellDelay();
            LOG(logDEBUG1,
                ("retval storage cell delay %lld ns\n", (long long int)retval));
            if (ret == FAIL) {
                sprintf(mess,
                        "Could not set storage cell delay. Set %lld ns, read "
                        "%lld ns.\n",
                        (long long int)arg, (long long int)retval);
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_frames_left(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(GOTTHARDD) &&         \
    !defined(CHIPTESTBOARDD) && !defined(MYTHEN3D) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // get only
    retval = getNumFramesLeft();
    LOG(logDEBUG1, ("retval num frames left %lld\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int get_triggers_left(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(GOTTHARDD) &&         \
    !defined(CHIPTESTBOARDD) && !defined(MYTHEN3D) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // get only
    retval = getNumTriggersLeft();
    LOG(logDEBUG1, ("retval num triggers left %lld\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int get_exptime_left(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#ifndef GOTTHARDD
    functionNotImplemented();
#else
    // get only
    retval = getExpTimeLeft();
    LOG(logDEBUG1, ("retval exptime left %lld ns\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int get_period_left(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#if !defined(JUNGFRAUD) && !defined(MOENCHD) &&                                \
    !defined(GOTTHARDD) /* && !defined(CHIPTESTBOARDD)                         \
&& !defined(MYTHEN3D) && !defined(GOTTHARD2D)*/
    functionNotImplemented();
#else
    // get only
    retval = getPeriodLeft();
    LOG(logDEBUG1, ("retval period left %lld ns\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int get_delay_after_trigger_left(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#if !defined(JUNGFRAUD) && !defined(MOENCHD) &&                                \
    !defined(GOTTHARDD) /* && !defined(CHIPTESTBOARDD)                         \
&& !defined(MYTHEN3D) && !defined(GOTTHARD2D)*/
    functionNotImplemented();
#else
    // get only
    retval = getDelayAfterTriggerLeft();
    LOG(logDEBUG1,
        ("retval delay after trigger left %lld ns\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int get_measured_period(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#ifndef EIGERD
    functionNotImplemented();
#else
    // get only
    retval = getMeasuredPeriod();
    LOG(logDEBUG1, ("retval measured period %lld ns\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int get_measured_subperiod(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#ifndef EIGERD
    functionNotImplemented();
#else
    // get only
    retval = getMeasuredSubPeriod();
    LOG(logDEBUG1,
        ("retval measured sub period %lld ns\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int get_frames_from_start(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(CHIPTESTBOARDD) &&    \
    !defined(MYTHEN3D) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // get only
    retval = getFramesFromStart();
    LOG(logDEBUG1, ("retval frames from start %lld\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int get_actual_time(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(CHIPTESTBOARDD) &&    \
    !defined(MYTHEN3D) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // get only
    retval = getActualTime();
    LOG(logDEBUG1, ("retval actual time %lld ns\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int get_measurement_time(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(CHIPTESTBOARDD) &&    \
    !defined(MYTHEN3D) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // get only
    retval = getMeasurementTime();
    LOG(logDEBUG1,
        ("retval measurement time %lld ns\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_dynamic_range(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int dr = -1;
    int retval = -1;

    if (receiveData(file_des, &dr, sizeof(dr), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting dr to %d\n", dr));

    // set & get
    if ((dr == GET_FLAG) || (Server_VerifyLock() == OK)) {
        // check dr
        switch (dr) {
        case GET_FLAG:
/*#ifdef MYTHEN3D TODO:Not implemented in firmware yet
        case 1:
#endif*/
#ifdef EIGERD
        case 4:
#endif
#if defined(EIGERD) || defined(MYTHEN3D)
        case 8:
#ifdef EIGERD
        case 12:
#endif
        case 16:
        case 32:
#endif
#if defined(GOTTHARDD) || defined(JUNGFRAUD) || defined(MOENCHD) ||            \
    defined(CHIPTESTBOARDD) || defined(GOTTHARD2D)
        case 16:
#endif
            if (dr >= 0) {
                ret = setDynamicRange(dr);
                if (ret == FAIL) {
                    sprintf(mess, "Could not set dynamic range to %d\n", dr);
                    LOG(logERROR, (mess));
                }
            }

            // get
            if (ret == OK) {
                ret = getDynamicRange(&retval);
                if (ret == FAIL) {
                    strcpy(mess, "Could not get dynamic range\n");
                    LOG(logERROR, (mess));
                } else {
                    LOG(logDEBUG1, ("Dynamic range: %d\n", retval));
                    validate(&ret, mess, dr, retval, "set dynamic range", DEC);
                }
            }
            break;
        default:
            modeNotImplemented("Dynamic range", dr);
            break;
        }
    }
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_roi(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    ROI arg;

    // receive ROI
    if (receiveData(file_des, &arg.xmin, sizeof(int), INT32) < 0)
        return printSocketReadError();
    if (receiveData(file_des, &arg.xmax, sizeof(int), INT32) < 0)
        return printSocketReadError();
    if (receiveData(file_des, &arg.ymin, sizeof(int), INT32) < 0)
        return printSocketReadError();
    if (receiveData(file_des, &arg.ymax, sizeof(int), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Set ROI: [%d, %d, %d, %d]\n", arg.xmin, arg.xmax, arg.ymin,
                    arg.ymax));

#ifndef GOTTHARDD
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        ret = setROI(arg);
        if (ret == FAIL) {
            sprintf(mess, "Could not set ROI. Invalid xmin or xmax\n");
            LOG(logERROR, (mess));
        }
        // old firmware requires a redo configure mac
        else {
            configure_mac();
        }
    }
#endif

    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_roi(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    ROI retval;

#ifndef GOTTHARDD
    functionNotImplemented();
#else
    // only get
    retval = getROI();
    LOG(logDEBUG1, ("nRois: (%d, %d, %d, %d)\n", retval.xmin, retval.xmax,
                    retval.ymin, retval.ymax));
#endif

    Server_SendResult(file_des, INT32, NULL, 0);
    if (ret != FAIL) {
        sendData(file_des, &retval.xmin, sizeof(int), INT32);
        sendData(file_des, &retval.xmax, sizeof(int), INT32);
        sendData(file_des, &retval.ymin, sizeof(int), INT32);
        sendData(file_des, &retval.ymax, sizeof(int), INT32);
    }
    return ret;
}

int lock_server(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int lock = 0;

    if (receiveData(file_des, &lock, sizeof(lock), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Locking Server to %d\n", lock));

    // set
    if (lock >= 0) {
        if (!lockStatus || // if it was unlocked, anyone can lock
            (lastClientIP == thisClientIP) || // if it was locked, need same ip
            (lastClientIP == 0u)) { // if it was locked, must be by "none"
            lockStatus = lock;
            if (lock) {
                char buf[INET_ADDRSTRLEN] = "";
                getIpAddressinString(buf, lastClientIP);
                LOG(logINFO, ("Server lock to %s\n", buf));
            } else {
                LOG(logINFO, ("Server unlocked\n"));
            }
            lastClientIP = thisClientIP;
        } else {
            Server_LockedError();
        }
    }
    int retval = lockStatus;
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int get_last_client_ip(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t retval = lastClientIP;
    retval = __builtin_bswap32(retval);
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int enable_ten_giga(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG, ("Setting 10GbE: %d\n", arg));

#if defined(JUNGFRAUD) || defined(MOENCHD) || defined(GOTTHARDD) ||            \
    defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // set & get
    if ((arg == GET_FLAG) || (Server_VerifyLock() == OK)) {
        if (arg >= 0 && enableTenGigabitEthernet(GET_FLAG) != arg) {
            enableTenGigabitEthernet(arg);
#ifdef EIGERD
            uint64_t hardwaremac = getDetectorMAC();
            if (udpDetails[0].srcmac != hardwaremac) {
                LOG(logINFOBLUE, ("Updating udp source mac\n"));
                for (int iRxEntry = 0; iRxEntry != MAX_UDP_DESTINATION;
                     ++iRxEntry) {
                    udpDetails[iRxEntry].srcmac = hardwaremac;
                }
            }
            uint32_t hardwareip = getDetectorIP();
            if (arg == 0 && udpDetails[0].srcip != hardwareip) {
                LOG(logINFOBLUE, ("Updating udp source ip\n"));
                for (int iRxEntry = 0; iRxEntry != MAX_UDP_DESTINATION;
                     ++iRxEntry) {
                    udpDetails[iRxEntry].srcip = hardwareip;
                }
            }
#endif
            configure_mac();
        }
        retval = enableTenGigabitEthernet(GET_FLAG);
        LOG(logDEBUG1, ("10GbE: %d\n", retval));
        validate(&ret, mess, arg, retval, "enable/disable 10GbE", DEC);
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int validateAndSetAllTrimbits(int arg) {
    int retval = -1;

#if !defined(EIGERD) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // set
    if (arg >= 0) {
        if (arg > MAX_TRIMBITS_VALUE) {
            ret = FAIL;
            sprintf(mess, "Cannot set all trimbits. Range: 0 - %d\n",
                    MAX_TRIMBITS_VALUE);
            LOG(logERROR, (mess));
        } else {
            ret = setAllTrimbits(arg);
            if (ret == FAIL) {
                strcpy(mess, "Could not set all trimbits\n");
                LOG(logERROR, (mess));
            }
#ifdef EIGERD
            // changes settings to undefined
            if (getSettings() != UNDEFINED) {
                setSettings(UNDEFINED);
                LOG(logERROR,
                    ("Settings has been changed to undefined (change all "
                     "trimbits)\n"));
            }
#endif
        }
    }
    // get
    retval = getAllTrimbits();
    LOG(logDEBUG1, ("All trimbits: %d\n", retval));
    validate(&ret, mess, arg, retval, "set all trimbits", DEC);
#endif
    return retval;
}

int set_all_trimbits(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Set all trmbits to %d\n", arg));

    if ((arg >= 0 && Server_VerifyLock() == OK) || arg < 0) {
        retval = validateAndSetAllTrimbits(arg);
    }

    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_pattern_io_control(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t arg = -1;
    uint64_t retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
#if !defined(CHIPTESTBOARDD)
    functionNotImplemented();
#else
    LOG(logDEBUG1,
        ("Setting Pattern IO Control to 0x%llx\n", (long long int)arg));
    if (((int64_t)arg == GET_FLAG) || (Server_VerifyLock() == OK)) {
        if ((int64_t)arg != GET_FLAG) {
            ret = validate_writePatternIOControl(mess, arg);
        }
        retval = validate_readPatternIOControl();
    }
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_pattern_word(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t args[2] = {-1, -1};
    uint64_t retval = -1;

    if (receiveData(file_des, args, sizeof(args), INT64) < 0)
        return printSocketReadError();
#if !defined(CHIPTESTBOARDD) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    int addr = (int)args[0];
    uint64_t word = args[1];
    if (word != (uint64_t)-1) {
        LOG(logINFO, ("Setting Pattern Word (addr:0x%x, word:0x%llx\n", addr,
                      (long long int)word));
    }
    if (Server_VerifyLock() == OK) {
        if (word != (uint64_t)-1) {
            ret = validate_writePatternWord(mess, addr, word);
        } else {
            ret = validate_readPatternWord(mess, addr, &retval);
        }
    }
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_pattern_loop_addresses(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[3] = {-1, -1, -1};
    int retvals[2] = {-1, -1};

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
#if !defined(CHIPTESTBOARDD) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    int loopLevel = args[0];
    int startAddr = args[1];
    int stopAddr = args[2];
    LOG(logDEBUG1, ("Setting Pattern loop addresses(loopLevel:%d "
                    "startAddr:0x%x stopAddr:0x%x)\n",
                    loopLevel, startAddr, stopAddr));
    if ((startAddr == GET_FLAG) || (stopAddr == GET_FLAG) ||
        (Server_VerifyLock() == OK)) {
        // loop limits
        if (loopLevel == -1) {
            // set
            if (startAddr >= 0 && stopAddr >= 0) {
                ret = validate_setPatternLoopLimits(mess, startAddr, stopAddr);
            }
            // get
            validate_getPatternLoopLimits(&retvals[0], &retvals[1]);
        }
        // loop addresses
        else {
            // set
            if (startAddr >= 0 && stopAddr >= 0) {
                ret = validate_setPatternLoopAddresses(mess, loopLevel,
                                                       startAddr, stopAddr);
            }
            // get
            if (ret == OK) {
                ret = validate_getPatternLoopAddresses(
                    mess, loopLevel, &retvals[0], &retvals[1]);
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, retvals, sizeof(retvals));
}

int set_pattern_loop_cycles(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[2] = {-1, -1};
    int retval = -1;

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
#if !defined(CHIPTESTBOARDD) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    int loopLevel = args[0];
    int numLoops = args[1];
    LOG(logDEBUG1, ("Setting Pattern loop cycles (loopLevel:%d numLoops:%d)\n",
                    loopLevel, numLoops));
    if ((numLoops == GET_FLAG) || (Server_VerifyLock() == OK)) {
        // set
        if (numLoops != GET_FLAG) {
            ret = validate_setPatternLoopCycles(mess, loopLevel, numLoops);
        }
        // get
        if (ret == OK) {
            ret = validate_getPatternLoopCycles(mess, loopLevel, &retval);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_pattern_wait_addr(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[2] = {-1, -1};
    int retval = -1;

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
#if !defined(CHIPTESTBOARDD) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    int loopLevel = args[0];
    int addr = args[1];
    LOG(logDEBUG1, ("Setting Pattern wait address (loopLevel:%d addr:0x%x)\n",
                    loopLevel, addr));
    if ((addr == GET_FLAG) || (Server_VerifyLock() == OK)) {
        // set
        if (addr != GET_FLAG) {
            ret = validate_setPatternWaitAddresses(mess, loopLevel, addr);
        }
        // get
        if (ret == OK) {
            ret = validate_getPatternWaitAddresses(mess, loopLevel, &retval);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_pattern_wait_time(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t args[2] = {-1, -1};
    uint64_t retval = -1;

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
#if !defined(CHIPTESTBOARDD) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    int loopLevel = (int)args[0];
    uint64_t timeval = args[1];
    LOG(logDEBUG1, ("Setting Pattern wait time (loopLevel:%d timeval:0x%llx)\n",
                    loopLevel, (long long int)timeval));
    if (((int64_t)timeval == GET_FLAG) || (Server_VerifyLock() == OK)) {
        // set
        if ((int64_t)timeval != GET_FLAG) {
            ret = validate_setPatternWaitTime(mess, loopLevel, timeval);
        }
        // get
        if (ret == OK) {
            ret = validate_getPatternWaitTime(mess, loopLevel, &retval);
        }
    }
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_pattern_mask(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Set Pattern Mask to %d\n", arg));

#if !defined(CHIPTESTBOARDD) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        setPatternMask(arg);
        uint64_t retval64 = getPatternMask();
        LOG(logDEBUG1,
            ("Pattern mask: 0x%llx\n", (long long unsigned int)retval64));
        validate64(&ret, mess, arg, retval64, "set Pattern Mask", HEX);
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_pattern_mask(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t retval64 = -1;

    LOG(logDEBUG1, ("Get Pattern Mask\n"));

#if !defined(CHIPTESTBOARDD) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // only get
    retval64 = getPatternMask();
    LOG(logDEBUG1,
        ("Get Pattern mask: 0x%llx\n", (long long unsigned int)retval64));

#endif
    return Server_SendResult(file_des, INT64, &retval64, sizeof(retval64));
}

int set_pattern_bit_mask(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Set Pattern Bit Mask to %d\n", arg));

#if !defined(CHIPTESTBOARDD) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        setPatternBitMask(arg);
        uint64_t retval64 = getPatternBitMask();
        LOG(logDEBUG1,
            ("Pattern bit mask: 0x%llx\n", (long long unsigned int)retval64));
        validate64(&ret, mess, arg, retval64, "set Pattern Bit Mask", HEX);
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_pattern_bit_mask(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t retval64 = -1;

    LOG(logDEBUG1, ("Get Pattern Bit Mask\n"));

#if !defined(CHIPTESTBOARDD) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // only get
    retval64 = getPatternBitMask();
    LOG(logDEBUG1,
        ("Get Pattern Bitmask: 0x%llx\n", (long long unsigned int)retval64));

#endif
    return Server_SendResult(file_des, INT64, &retval64, sizeof(retval64));
}

int write_adc_register(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t args[2] = {-1, -1};

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    uint32_t addr = args[0];
    uint32_t val = args[1];
    LOG(logDEBUG1, ("Writing 0x%x to ADC Register 0x%x\n", val, addr));

#if defined(EIGERD) || defined(GOTTHARD2D) || defined(MYTHEN3D)
    functionNotImplemented();
#else
#ifndef VIRTUAL
    // only set
    if (Server_VerifyLock() == OK) {
#if defined(JUNGFRAUD) || defined(MOENCHD) || defined(CHIPTESTBOARDD)
        AD9257_Set(addr, val);
#elif GOTTHARDD
        if (isHardwareVersion_1_0()) {
            AD9252_Set(addr, val);
        } else {
            AD9257_Set(addr, val);
        }
#endif
    }
#endif
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int set_counter_bit(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Set counter bit with value: %d\n", arg));

#ifndef EIGERD
    functionNotImplemented();
#else

    // set
    if (arg >= 0 && Server_VerifyLock() == OK) {
        setCounterBit(arg);
    }
    // get
    retval = setCounterBit(GET_FLAG);
    LOG(logDEBUG1, ("Set counter bit retval: %d\n", retval));
    validate(&ret, mess, arg, retval, "set counter bit", DEC);
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int pulse_pixel(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[3] = {-1, -1, -1};

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1,
        ("Pulse pixel, n: %d, x: %d, y: %d\n", args[0], args[1], args[2]));

#ifndef EIGERD
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        ret = pulsePixel(args[0], args[1], args[2]);
        if (ret == FAIL) {
            strcpy(mess, "Could not pulse pixel\n");
            LOG(logERROR, (mess));
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int pulse_pixel_and_move(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[3] = {-1, -1, -1};

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Pulse pixel and move, n: %d, x: %d, y: %d\n", args[0],
                    args[1], args[2]));

#ifndef EIGERD
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        ret = pulsePixelNMove(args[0], args[1], args[2]);
        if (ret == FAIL) {
            strcpy(mess, "Could not pulse pixel and move\n");
            LOG(logERROR, (mess));
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int pulse_chip(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Pulse chip: %d\n", arg));

#ifndef EIGERD
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        ret = pulseChip(arg);
        if (ret == FAIL) {
            strcpy(mess, "Could not pulse chip\n");
            LOG(logERROR, (mess));
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int set_rate_correct(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t tau_ns = -1;

    if (receiveData(file_des, &tau_ns, sizeof(tau_ns), INT64) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Set rate correct with tau %lld\n", (long long int)tau_ns));

#ifndef EIGERD
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        ret = validateAndSetRateCorrection(tau_ns, mess);
        int64_t retval = getCurrentTau(); // to update eiger_tau_ns (for
                                          // update rate correction)
        if (ret == FAIL) {
            strcpy(mess, "Rate correction failed\n");
            LOG(logERROR, (mess));
        } else {
            validate64(&ret, mess, tau_ns, retval, "set rate correction", DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_rate_correct(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

    LOG(logDEBUG1, ("Getting rate correction\n"));
#ifndef EIGERD
    functionNotImplemented();
#else
    retval = getCurrentTau();
    LOG(logDEBUG1, ("Tau: %lld\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_ten_giga_flow_control(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting ten giga flow control: %d\n", arg));

#if !defined(EIGERD) && !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        ret = setTenGigaFlowControl(arg);
        if (ret == FAIL) {
            strcpy(mess, "Could not set ten giga flow control.\n");
            LOG(logERROR, (mess));
        } else {
            int retval = getTenGigaFlowControl();
            LOG(logDEBUG1, ("ten giga flow control retval: %d\n", retval));
            validate(&ret, mess, arg, retval, "set ten giga flow control", DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_ten_giga_flow_control(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting ten giga flow control\n"));

#if !defined(EIGERD) && !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    // get only
    retval = getTenGigaFlowControl();
    LOG(logDEBUG1, ("ten giga flow control retval: %d\n", retval));
    if (retval == -1) {
        strcpy(mess, "Could not get ten giga flow control.\n");
        LOG(logERROR, (mess));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_transmission_delay_frame(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting transmission delay frame: %d\n", arg));

#if !defined(EIGERD) && !defined(JUNGFRAUD) && !defined(MOENCHD) &&            \
    !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
#if defined(JUNGFRAUD) || defined(MYTHEN3D)
        if (arg > MAX_TIMESLOT_VAL) {
            ret = FAIL;
            sprintf(mess, "Transmission delay %d should be in range: 0 - %d\n",
                    arg, MAX_TIMESLOT_VAL);
            LOG(logERROR, (mess));
        }
#endif
        if (ret == OK) {
            ret = setTransmissionDelayFrame(arg);
            if (ret == FAIL) {
                strcpy(mess, "Could not set transmission delay frame.\n");
                LOG(logERROR, (mess));
            } else {
                int retval = getTransmissionDelayFrame();
                LOG(logDEBUG1,
                    ("transmission delay frame retval: %d\n", retval));
                validate(&ret, mess, arg, retval,
                         "set transmission delay frame", DEC);
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_transmission_delay_frame(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting transmission delay frame\n"));

#if !defined(EIGERD) && !defined(JUNGFRAUD) && !defined(MOENCHD) &&            \
    !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // get only
    retval = getTransmissionDelayFrame();
    LOG(logDEBUG1, ("transmission delay frame retval: %d\n", retval));
    if (retval == -1) {
        strcpy(mess, "Could not get transmission delay frame.\n");
        LOG(logERROR, (mess));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_transmission_delay_left(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting transmission delay left: %d\n", arg));

#ifndef EIGERD
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        ret = setTransmissionDelayLeft(arg);
        if (ret == FAIL) {
            strcpy(mess, "Could not set transmission delay left.\n");
            LOG(logERROR, (mess));
        } else {
            int retval = getTransmissionDelayLeft();
            LOG(logDEBUG1, ("transmission delay left retval: %d\n", retval));
            validate(&ret, mess, arg, retval, "set transmission delay left",
                     DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_transmission_delay_left(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting transmission delay left\n"));

#ifndef EIGERD
    functionNotImplemented();
#else
    // get only
    retval = getTransmissionDelayLeft();
    LOG(logDEBUG1, ("transmission delay left: %d\n", retval));
    if (retval == -1) {
        strcpy(mess, "Could not get transmission delay left.\n");
        LOG(logERROR, (mess));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_transmission_delay_right(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting transmission delay right: %d\n", arg));

#ifndef EIGERD
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        ret = setTransmissionDelayRight(arg);
        if (ret == FAIL) {
            strcpy(mess, "Could not set transmission delay right.\n");
            LOG(logERROR, (mess));
        } else {
            int retval = getTransmissionDelayRight();
            LOG(logDEBUG1, ("transmission delay right retval: %d\n", retval));
            validate(&ret, mess, arg, retval, "set transmission delay right",
                     DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_transmission_delay_right(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting transmission delay right\n"));

#ifndef EIGERD
    functionNotImplemented();
#else
    // get only
    retval = getTransmissionDelayRight();
    LOG(logDEBUG1, ("transmission delay right retval: %d\n", retval));
    if (retval == -1) {
        strcpy(mess, "Could not get transmission delay right.\n");
        LOG(logERROR, (mess));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int program_fpga(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

#if defined(EIGERD) || defined(GOTTHARDD)
    functionNotImplemented();
    return Server_SendResult(file_des, INT32, NULL, 0);
#else
    receive_program(file_des, PROGRAM_FPGA);
#endif
    return ret;
}

int reset_fpga(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

    LOG(logDEBUG1, ("Reset FPGA\n"));
#if defined(EIGERD) || defined(GOTTHARDD) || defined(GOTTHARD2D) ||            \
    defined(MYTHEN3D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (isControlServer) {
            basictests(); // mapping of control server at least
            char *message = NULL;
            if (getInitResult(&message) == FAIL) {
                ret = FAIL;
                strcpy(mess, message);
                LOG(logERROR, (mess));
            } else {
                initControlServer();
            }
        } else {
            initStopServer(); // remapping of stop server
        }
        if (ret == OK) {
            char *message = NULL;
            if (getInitResult(&message) == FAIL) {
                ret = FAIL;
                strcpy(mess, message);
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int power_chip(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Powering chip to %d\n", arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(MYTHEN3D) &&          \
    !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // set & get
    if ((arg == GET_FLAG) || (Server_VerifyLock() == OK)) {
#if defined(MYTHEN3D) || defined(GOTTHARD2D)
        // check only when powering on
        if (arg != -1 && arg != 0) {
            if (!checkModuleFlag) {
                LOG(logINFOBLUE,
                    ("In No-Module mode: Ignoring module type. Continuing.\n"));
            } else {
                ret = checkDetectorType(mess);
                if (ret == FAIL) {
                    LOG(logERROR, ("Could not power on chip.\n"));
                }
            }
        }
#endif
        if (ret == OK) {
            retval = powerChip(arg);
            LOG(logDEBUG1, ("Power chip: %d\n", retval));
        }
        validate(&ret, mess, arg, retval, "power on/off chip", DEC);
#if defined(JUNGFRAUD) || defined(MOENCHD)
        // narrow down error when powering on
        if (ret == FAIL && arg > 0) {
            if (setTemperatureEvent(GET_FLAG) == 1)
                sprintf(mess,
                        "Powering chip failed due to over-temperature event. "
                        "Clear event & power chip again. Set %d, read %d \n",
                        arg, retval);
            LOG(logERROR, (mess));
        }
#endif
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_activate(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting activate mode to %d\n", arg));

#ifndef EIGERD
    functionNotImplemented();
#else
    // set & get
    if ((arg == GET_FLAG) || (Server_VerifyLock() == OK)) {
        if (arg >= 0) {
            if (setActivate(arg) == FAIL) {
                ret = FAIL;
                sprintf(mess, "Could not %s\n",
                        (arg == 0 ? "deactivate" : "activate"));
                LOG(logERROR, (mess));
            }
        }
        if (ret == OK) {
            if (getActivate(&retval) == FAIL) {
                ret = FAIL;
                sprintf(mess, "Could not get activate flag\n");
                LOG(logERROR, (mess));
            } else {
                LOG(logDEBUG1, ("Activate: %d\n", retval));
                validate(&ret, mess, arg, retval, "set/get activate", DEC);
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

// stop server
int threshold_temp(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting threshold temperature to %d\n", arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    // set & get
    if ((arg == GET_FLAG) || (Server_VerifyLock() == OK)) {
        if (arg > MAX_THRESHOLD_TEMP_VAL) {
            ret = FAIL;
            sprintf(mess, "Threshold Temp %d should be in range: 0 - %d\n", arg,
                    MAX_THRESHOLD_TEMP_VAL);
            LOG(logERROR, (mess));
        }
        // valid temp
        else {
            retval = setThresholdTemperature(arg);
            LOG(logDEBUG1, ("Threshold temperature: %d\n", retval));
            validate(&ret, mess, arg, retval, "set threshold temperature", DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

// stop server
int temp_control(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting temperature control to %d\n", arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    // set & get
    if ((arg == GET_FLAG) || (Server_VerifyLock() == OK)) {
        retval = setTemperatureControl(arg);
        LOG(logDEBUG1, ("Temperature control: %d\n", retval));
        validate(&ret, mess, arg, retval, "set temperature control", DEC);
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

// stop server
int temp_event(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting temperature event to %d\n", arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    // set & get
    if ((arg == GET_FLAG) || (Server_VerifyLock() == OK)) {
        retval = setTemperatureEvent(arg);
        LOG(logDEBUG1, ("Temperature event: %d\n", retval));
        validate(&ret, mess, arg, retval, "set temperature event", DEC);
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int auto_comp_disable(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting  Auto comp disable to %d\n", arg));

#if !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // set & get
    if ((arg == GET_FLAG) || (Server_VerifyLock() == OK)) {
        retval = autoCompDisable(arg);
        LOG(logDEBUG1, ("Auto comp disable: %d\n", retval));
        validate(&ret, mess, arg, retval, "set auto comp disable", DEC);
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int storage_cell_start(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting Storage cell start to %d\n", arg));

#if !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // set & get
    if ((arg == GET_FLAG) || (Server_VerifyLock() == OK)) {
        if (arg > getMaxStoragecellStart()) {
            ret = FAIL;
            sprintf(mess, "Max Storage cell number should not exceed %d\n",
                    getMaxStoragecellStart());
            LOG(logERROR, (mess));
        } else {
            retval = selectStoragecellStart(arg);
            LOG(logDEBUG1, ("Storage cell start: %d\n", retval));
            validate(&ret, mess, arg, retval, "set storage cell start", DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int initial_checks(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

    // check software- firmware compatibility and basic tests
    LOG(logDEBUG1, ("Checking software-firmware compatibility and basic "
                    "test result\n"));

    // check if firmware check is done
    if (!isInitCheckDone()) {
        usleep(3 * 1000 * 1000);
        if (!isInitCheckDone()) {
            ret = FAIL;
            strcpy(mess, "Server Initialization still not done done in server. "
                         "Unexpected.\n");
            LOG(logERROR, (mess));
        }
    }

    // check firmware check result
    if (ret == OK) {
        char *firmware_message = NULL;
        if (getInitResult(&firmware_message) == FAIL) {
            ret = FAIL;
            strcpy(mess, firmware_message);
            LOG(logERROR, (mess));
        }
    }
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int software_trigger(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Software Trigger (block: %d\n", arg));

#if !defined(EIGERD) && !defined(MYTHEN3D) && !defined(JUNGFRAUD) &&           \
    !defined(MOENCHD)
    functionNotImplemented();
#else
    if (arg && myDetectorType == MYTHEN3) {
        ret = FAIL;
        strcpy(mess, "Blocking trigger not implemented for this detector. "
                     "Please use "
                     "non blocking trigger.\n");
        LOG(logERROR, (mess));
    }
    // only set
    else if (Server_VerifyLock() == OK) {
#ifdef MYTHEN3D
        ret = softwareTrigger();
#else
        ret = softwareTrigger(arg);
#endif
        if (ret == FAIL) {
            strcpy(mess, "Could not send software trigger\n");
            LOG(logERROR, (mess));
        }
        LOG(logDEBUG1, ("Software trigger successful\n"));
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int led(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting led enable to %d\n", arg));

#if (!defined(CHIPTESTBOARDD))
    functionNotImplemented();
#else
    // set & get
    if ((arg == GET_FLAG) || (Server_VerifyLock() == OK)) {
        retval = setLEDEnable(arg);
        LOG(logDEBUG1, ("LED Enable: %d\n", retval));
        validate(&ret, mess, arg, retval, "enable/disable LED", DEC);
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int digital_io_delay(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t args[2] = {-1, -1};

    if (receiveData(file_des, args, sizeof(args), INT64) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Digital IO Delay, pinMask: 0x%llx, delay:%d ps\n", args[0],
                    (int)args[1]));

#if (!defined(CHIPTESTBOARDD))
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        int delay = (int)args[1];
        if (delay < 0 || delay > DIGITAL_IO_DELAY_MAXIMUM_PS) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set digital IO delay. Delay maximum is %d ps\n",
                    DIGITAL_IO_DELAY_MAXIMUM_PS);
            LOG(logERROR, (mess));
        } else {
            setDigitalIODelay(args[0], delay);
            LOG(logDEBUG1, ("Digital IO Delay successful\n"));
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int reboot_controller(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

#ifdef EIGERD
    functionNotImplemented();
#elif VIRTUAL
    ret = GOODBYE;
#elif defined(MYTHEN3D) || defined(GOTTHARD2D)
    if (isHardwareVersion_1_0()) {
        ret = FAIL;
        strcpy(mess, "Old board version, reboot by yourself please!\n");
        LOG(logINFORED, (mess));
        Server_SendResult(file_des, INT32, NULL, 0);
        return GOODBYE;
    }
    ret = REBOOT;
#else
    ret = REBOOT;
#endif

    Server_SendResult(file_des, INT32, NULL, 0);
    return ret;
}

int set_adc_enable_mask(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Seting 1Gb ADC Enable Mask to %u\n", arg));

#if (!defined(CHIPTESTBOARDD))
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (arg == 0u) {
            ret = FAIL;
            sprintf(mess,
                    "Not allowed to set adc mask of 0 due to data readout. \n");
            LOG(logERROR, (mess));
        } else {
            ret = setADCEnableMask(arg);
            if (ret == FAIL) {
                sprintf(mess, "Could not set 1Gb ADC Enable mask to 0x%x.\n",
                        arg);
                LOG(logERROR, (mess));
            } else {
                uint32_t retval = getADCEnableMask();
                if (arg != retval) {
                    ret = FAIL;
                    sprintf(
                        mess,
                        "Could not set 1Gb ADC Enable mask. Set 0x%x, but read "
                        "0x%x\n",
                        arg, retval);
                    LOG(logERROR, (mess));
                }
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_adc_enable_mask(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t retval = -1;

    LOG(logDEBUG1, ("Getting 1Gb ADC Enable Mask \n"));

#if (!defined(CHIPTESTBOARDD))
    functionNotImplemented();
#else
    // get
    retval = getADCEnableMask();
    LOG(logDEBUG1, ("1Gb ADC Enable Mask retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_adc_enable_mask_10g(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Seting 10Gb ADC Enable Mask to %u\n", arg));

#if (!defined(CHIPTESTBOARDD))
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (arg == 0u) {
            ret = FAIL;
            sprintf(mess,
                    "Not allowed to set adc mask of 0 due to data readout \n");
            LOG(logERROR, (mess));
        } else {
            setADCEnableMask_10G(arg);
            uint32_t retval = getADCEnableMask_10G();
            if (arg != retval) {
                ret = FAIL;
                sprintf(mess,
                        "Could not set 10Gb ADC Enable mask. Set 0x%x, but "
                        "read 0x%x\n",
                        arg, retval);
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_adc_enable_mask_10g(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t retval = -1;

    LOG(logDEBUG1, ("Getting 10Gb ADC Enable Mask\n"));

#if (!defined(CHIPTESTBOARDD))
    functionNotImplemented();
#else
    // get
    retval = getADCEnableMask_10G();
    LOG(logDEBUG1, ("10Gb ADC Enable Mask retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_adc_invert(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Seting ADC Invert to %u\n", arg));

#if !defined(CHIPTESTBOARDD) && !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        setADCInvertRegister(arg);
        uint32_t retval = getADCInvertRegister();
        if (arg != retval) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set ADC Invert register. Set 0x%x, but read "
                    "0x%x\n",
                    arg, retval);
            LOG(logERROR, (mess));
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_adc_invert(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t retval = -1;

    LOG(logDEBUG1, ("Getting ADC Invert register \n"));

#if !defined(CHIPTESTBOARDD) && !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    // get
    retval = getADCInvertRegister();
    LOG(logDEBUG1, ("ADC Invert register retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_external_sampling_source(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting external sampling source to %d\n", arg));

#ifndef CHIPTESTBOARDD
    functionNotImplemented();
#else
    // set & get
    if ((arg == GET_FLAG) || (Server_VerifyLock() == OK)) {
        if (arg < -1 || arg > 63) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set external sampling source to %d. Value must "
                    "be 0-63.\n",
                    arg);
            LOG(logERROR, (mess));
        } else {
            retval = setExternalSamplingSource(arg);
            LOG(logDEBUG1, ("External Sampling source: %d\n", retval));
            validate(&ret, mess, arg, retval, "set external sampling source",
                     DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_external_sampling(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting external sampling enable to %d\n", arg));

#ifndef CHIPTESTBOARDD
    functionNotImplemented();
#else
    // set & get
    if ((arg == GET_FLAG) || (Server_VerifyLock() == OK)) {
        arg = (arg > 0) ? 1 : arg;
        retval = setExternalSampling(arg);
        LOG(logDEBUG1, ("External Sampling enable: %d\n", retval));
        validate(&ret, mess, arg, retval, "set external sampling enable", DEC);
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_next_frame_number(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting next frame number to %llu\n", arg));

#if !defined(EIGERD) && !defined(JUNGFRAUD) && !defined(MOENCHD) &&            \
    !defined(CHIPTESTBOARDD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (arg == 0) {
            ret = FAIL;
            sprintf(mess, "Could not set next frame number. Cannot be 0.\n");
            LOG(logERROR, (mess));
        }
#if (defined(EIGERD)) || (defined(CHIPTESTBOARDD))
        else if (arg > UDP_HEADER_MAX_FRAME_VALUE) {
            ret = FAIL;
#ifdef VIRTUAL
            sprintf(mess,
                    "Could not set next frame number. Must be less then "
                    "%ld (0x%lx)\n",
                    UDP_HEADER_MAX_FRAME_VALUE, UDP_HEADER_MAX_FRAME_VALUE);
#else
            sprintf(mess,
                    "Could not set next frame number. Must be less then "
                    "%lld (0x%llx)\n",
                    UDP_HEADER_MAX_FRAME_VALUE, UDP_HEADER_MAX_FRAME_VALUE);
#endif
            LOG(logERROR, (mess));
        }
#endif
        else {
            ret = setNextFrameNumber(arg);
            if (ret == FAIL) {
                sprintf(
                    mess, "Could not set next frame number. %s\n",
                    (myDetectorType == EIGER ? "Failed to map address" : ""));
                LOG(logERROR, (mess));
            }
            if (ret == OK) {
                uint64_t retval = 0;
                ret = getNextFrameNumber(&retval);
                if (ret == FAIL) {
                    sprintf(mess, "Could not set next frame number. %s\n",
                            (myDetectorType == EIGER ? "Failed to map address"
                                                     : ""));
                    LOG(logERROR, (mess));
                } else if (ret == -2) {
                    sprintf(mess, "Inconsistent next frame number from "
                                  "left and right FPGA. Please set it.\n");
                    LOG(logERROR, (mess));
                } else {
                    if (arg != retval) {
                        ret = FAIL;
#ifdef VIRTUAL
                        sprintf(mess,
                                "Could not set next frame number. Set "
                                "0x%lx, but read 0x%lx\n",
                                arg, retval);
#else
                        sprintf(mess,
                                "Could not set next frame number. Set "
                                "0x%llx, but read 0x%llx\n",
                                arg, retval);
#endif
                        LOG(logERROR, (mess));
                    }
                }
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_next_frame_number(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t retval = -1;

    LOG(logDEBUG1, ("Getting next frame number \n"));

#if !defined(EIGERD) && !defined(JUNGFRAUD) && !defined(MOENCHD) &&            \
    !defined(CHIPTESTBOARDD)
    functionNotImplemented();
#else
    // get
    ret = getNextFrameNumber(&retval);
    if (ret == FAIL) {
        sprintf(mess, "Could not set next frame number. %s\n",
                (myDetectorType == EIGER ? "Failed to map address" : ""));
        LOG(logERROR, (mess));
    } else if (ret == -2) {
        sprintf(mess, "Inconsistent next frame number from left and right "
                      "FPGA. Please set it.\n");
        LOG(logERROR, (mess));
    } else {
        LOG(logDEBUG1, ("next frame number retval: %u\n", retval));
    }
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_quad(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting quad: %u\n", arg));

#ifndef EIGERD
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (setQuad(arg) == FAIL) {
            ret = FAIL;
            sprintf(mess, "Could not set quad.\n");
            LOG(logERROR, (mess));
        } else {
            int retval = getQuad();
            if (arg != retval) {
                ret = FAIL;
                sprintf(mess, "Could not set quad. Set %d, but read %d\n",
                        retval, arg);
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_quad(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting Quad\n"));

#ifndef EIGERD
    functionNotImplemented();
#else
    // get only
    retval = getQuad();
    LOG(logDEBUG1, ("Quad retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_interrupt_subframe(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting interrupt subframe: %u\n", arg));

#ifndef EIGERD
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (setInterruptSubframe(arg) == FAIL) {
            ret = FAIL;
            sprintf(mess, "Could not set Intertupt Subframe in FEB.\n");
            LOG(logERROR, (mess));
        } else {
            int retval = getInterruptSubframe();
            if (arg != retval) {
                ret = FAIL;
                sprintf(mess,
                        "Could not set Intertupt Subframe. Set %d, but "
                        "read %d\n",
                        retval, arg);
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_interrupt_subframe(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting interrupt subframe\n"));

#ifndef EIGERD
    functionNotImplemented();
#else
    // get only
    retval = getInterruptSubframe();
    if (retval == -1) {
        ret = FAIL;
        sprintf(mess, "Could not get Intertupt Subframe or inconsistent values "
                      "between left and right. \n");
        LOG(logERROR, (mess));
    } else {
        LOG(logDEBUG1, ("Interrupt subframe retval: %u\n", retval));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_read_n_rows(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting number of rows: %u\n", arg));

#if !defined(EIGERD) && !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (arg < MIN_ROWS_PER_READOUT || arg > MAX_ROWS_PER_READOUT) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set read n rows. Must be between %d "
                    "and %d\n",
                    MIN_ROWS_PER_READOUT, MAX_ROWS_PER_READOUT);
            LOG(logERROR, (mess));
        } else {
#ifdef EIGERD
            int dr = 0;
            ret = getDynamicRange(&dr);
            int isTenGiga = enableTenGigabitEthernet(GET_FLAG);
            unsigned int maxnl = MAX_ROWS_PER_READOUT;
            unsigned int maxnp = (isTenGiga ? 4 : 16) * dr;
            // get dr fail
            if (ret == FAIL) {
                strcpy(mess,
                       "Could not read n rows (failed to get dynamic range)\n");
                LOG(logERROR, (mess));
            } else if ((arg * maxnp) % maxnl) {
                ret = FAIL;
                sprintf(mess,
                        "Could not set number of rows to %d. For %d bit "
                        "mode and 10 giga %s, (%d (num "
                        "rows) x %d (max num packets for this mode)) must be "
                        "divisible by %d\n",
                        arg, dr, isTenGiga ? "enabled" : "disabled", arg, maxnp,
                        maxnl);
                LOG(logERROR, (mess));
            } else
#elif defined(JUNGFRAUD) || defined(MOENCHD)
            if ((check_detector_idle("set number of rows") == OK) &&
                (arg % READ_N_ROWS_MULTIPLE != 0)) {
                ret = FAIL;
                sprintf(mess,
                        "Could not set number of rows. %d must be a multiple "
                        "of %d\n",
                        arg, READ_N_ROWS_MULTIPLE);
                LOG(logERROR, (mess));
            } else if (isHardwareVersion_1_0()) {
                ret = FAIL;
                strcpy(mess, "Could not set number of rows. Only available for "
                             "Hardware Board version 2.0.\n");
                LOG(logERROR, (mess));
            } else
#endif
            {
                if (setReadNRows(arg) == FAIL) {
                    ret = FAIL;
                    sprintf(mess, "Could not set number of rows to %d.\n", arg);
                    LOG(logERROR, (mess));
                } else {
                    int retval = getReadNRows();
                    if (arg != retval) {
                        ret = FAIL;
                        sprintf(mess,
                                "Could not set number of rows. Set %d, but "
                                "read %d\n",
                                retval, arg);
                        LOG(logERROR, (mess));
                    }
                }
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_read_n_rows(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting number of rows\n"));

#if !defined(EIGERD) && !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    retval = getReadNRows();
    if (retval == -1) {
        ret = FAIL;
        sprintf(mess, "Could not get number of rows. \n");
        LOG(logERROR, (mess));
    } else {
        LOG(logDEBUG1, ("number of rows retval: %u\n", retval));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

void calculate_and_set_position() {
    if (maxYMods == -1 || moduleIndex == -1) {
        ret = FAIL;
        sprintf(mess,
                "Could not set detector position (did not get multi size).\n");
        LOG(logERROR, (mess));
        return;
    }

    // calculating new position
    int pos[2] = {0, 0};

    int portGeometry[2] = {1, 1};
    // position does change for eiger and jungfrau/moench (2 interfaces)
#if defined(EIGERD)
    portGeometry[X] = getNumberofUDPInterfaces(); // horz
#elif defined(JUNGFRAUD) || defined(MOENCHD)
    portGeometry[Y] = getNumberofUDPInterfaces(); // vert
#endif
    LOG(logDEBUG1, ("moduleIndex:%d maxymods:%d portGeo.x:%d portgeo.y:%d\n",
                    moduleIndex, maxYMods, portGeometry[X], portGeometry[Y]));
    pos[Y] = (moduleIndex % maxYMods) * portGeometry[Y];
    pos[X] = (moduleIndex / maxYMods) * portGeometry[X];
    LOG(logINFO, ("Setting Positions (%d,%d) #(col, row)\n", pos[X], pos[Y]));
    if (setDetectorPosition(pos) == FAIL) {
        ret = FAIL;
        sprintf(mess, "Could not set detector position.\n");
        LOG(logERROR, (mess));
    }
    // to redo the detector mac (depends on positions)
    else {
        // create detector mac from x and y
        if (udpDetails[0].srcmac == 0) {
            char dmac[MAC_ADDRESS_SIZE];
            memset(dmac, 0, MAC_ADDRESS_SIZE);
            sprintf(dmac, "aa:bb:cc:dd:%02x:%02x", pos[X] & 0xFF,
                    pos[Y] & 0xFF);
            LOG(logINFO, ("Udp source mac address created: %s\n", dmac));
            unsigned char a[6];
            sscanf(dmac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &a[0], &a[1], &a[2],
                   &a[3], &a[4], &a[5]);
            udpDetails[0].srcmac = 0;
            for (int i = 0; i < 6; ++i) {
                udpDetails[0].srcmac = (udpDetails[0].srcmac << 8) + a[i];
            }
            for (int iRxEntry = 1; iRxEntry != MAX_UDP_DESTINATION;
                 ++iRxEntry) {
                udpDetails[iRxEntry].srcmac = udpDetails[0].srcmac;
            }
        }
#if defined(JUNGFRAUD) || defined(MOENCHD) || defined(GOTTHARD2D)
        if (getNumberofUDPInterfaces() > 1) {
            if (udpDetails[0].srcmac2 == 0) {
                char dmac2[MAC_ADDRESS_SIZE];
                memset(dmac2, 0, MAC_ADDRESS_SIZE);
                sprintf(dmac2, "aa:bb:cc:dd:%02x:%02x", (pos[X] + 1) & 0xFF,
                        pos[Y] & 0xFF);
                LOG(logINFO, ("Udp source mac address2 created: %s\n", dmac2));
                unsigned char a[6];
                sscanf(dmac2, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &a[0], &a[1],
                       &a[2], &a[3], &a[4], &a[5]);
                udpDetails[0].srcmac2 = 0;
                for (int i = 0; i < 6; ++i) {
                    udpDetails[0].srcmac2 = (udpDetails[0].srcmac2 << 8) + a[i];
                }
                for (int iRxEntry = 1; iRxEntry != MAX_UDP_DESTINATION;
                     ++iRxEntry) {
                    udpDetails[iRxEntry].srcmac2 = udpDetails[0].srcmac2;
                }
            }
        }
#endif
        configure_mac();
    }
    // no need to do a get (also jungfrau/moench gives bigger set for second)
}

int set_detector_position(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[2] = {0, 0};

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG, ("Setting detector positions: [maxy:%u, modIndex:%u]\n",
                   args[0], args[1]));

    // only set
    if (Server_VerifyLock() == OK) {
        // if in update mode, there is no need to do this (also detector not set
        // up)
        if (!updateFlag && check_detector_idle("configure mac") == OK) {
            maxYMods = args[0];
            moduleIndex = args[1];
            calculate_and_set_position();
        }
    }
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int check_detector_idle(const char *s) {
    enum runStatus status = getRunStatus();
    if (status != IDLE && status != RUN_FINISHED && status != STOPPED &&
        status != ERROR) {
        ret = FAIL;
        sprintf(mess,
                "Cannot %s when detector is not idle. Detector at "
                "%s state\n",
                s, getRunStateName(status));
        LOG(logERROR, (mess));
    }
    return ret;
}

int is_udp_configured() {
    for (int i = 0; i != numUdpDestinations; ++i) {
        if (udpDetails[i].dstip == 0) {
            sprintf(configureMessage,
                    "udp destination ip not configured [entry:%d]\n", i);
            LOG(logWARNING, ("%s", configureMessage));
            return FAIL;
        }
        if (udpDetails[i].srcip == 0) {
            sprintf(configureMessage,
                    "udp source ip not configured [entry:%d]\n", i);
            LOG(logWARNING, ("%s", configureMessage));
            return FAIL;
        }
        if (udpDetails[i].srcmac == 0) {
            sprintf(configureMessage,
                    "udp source mac not configured [entry:%d]\n", i);
            LOG(logWARNING, ("%s", configureMessage));
            return FAIL;
        }
        // virtual: no check (can be eth name: lo, ip: 127.0.0.1)
#ifndef VIRTUAL
        if (udpDetails[i].dstmac == 0) {
            sprintf(configureMessage,
                    "udp destination mac not configured [entry:%d]\n", i);
            LOG(logWARNING, ("%s", configureMessage));
            return FAIL;
        }
#endif
#if defined(JUNGFRAUD) || defined(MOENCHD) || defined(GOTTHARD2D)
        if (getNumberofUDPInterfaces() == 2) {
            if (udpDetails[i].srcip2 == 0) {
                sprintf(configureMessage,
                        "udp source ip2 not configured [entry:%d]\n", i);
                LOG(logWARNING, ("%s", configureMessage));
                return FAIL;
            }
            if (udpDetails[i].dstip2 == 0) {
                sprintf(configureMessage,
                        "udp destination ip2 not configured [entry:%d]\n", i);
                LOG(logWARNING, ("%s", configureMessage));
                return FAIL;
            }
            if (udpDetails[i].srcmac2 == 0) {
                sprintf(configureMessage,
                        "udp source mac2 not configured [entry:%d]\n", i);
                LOG(logWARNING, ("%s", configureMessage));
                return FAIL;
            }
#ifndef VIRTUAL
            if (udpDetails[i].dstmac2 == 0) {
                sprintf(configureMessage,
                        "udp destination mac2 not configured [entry:%d]\n", i);
                LOG(logWARNING, ("%s", configureMessage));
                return FAIL;
            }
#endif
        }
#endif
    }
    return OK;
}

void configure_mac() {
    if (isControlServer) {
        if (is_udp_configured() == OK) {
            ret = configureMAC();
            if (ret != OK) {
#if defined(CHIPTESTBOARDD)
                if (ret == -1) {
                    sprintf(mess, "Could not allocate RAM\n");
                } else {
                    sprintf(mess,
                            "Could not configure mac because of incorrect "
                            "udp 1G destination IP and port\n");
                }
#else
                sprintf(mess, "Configure Mac failed\n");
#endif
                strcpy(configureMessage, mess);
                LOG(logERROR, (mess));
            } else {
                LOG(logINFOGREEN, ("\tConfigure MAC successful\n"));
                configured = OK;
                return;
            }
        }
    }
    configured = FAIL;
    LOG(logWARNING, ("Configure FAIL, not all parameters configured yet\n"));
}

int set_source_udp_ip(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    arg = __builtin_bswap32(arg);
    LOG(logINFO, ("Setting udp source ip: 0x%x\n", arg));

    // only set
    if (Server_VerifyLock() == OK) {
        if (check_detector_idle("configure mac") == OK) {
            if (udpDetails[0].srcip != arg) {
                for (int iRxEntry = 0; iRxEntry != MAX_UDP_DESTINATION;
                     ++iRxEntry) {
                    udpDetails[iRxEntry].srcip = arg;
                }
                configure_mac();
            }
        }
    }
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_source_udp_ip(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t retval = -1;
    LOG(logDEBUG1, ("Getting udp source ip\n"));

    // get only
    retval = udpDetails[0].srcip;
    retval = __builtin_bswap32(retval);
    LOG(logDEBUG1, ("udp soure ip retval: 0x%x\n", retval));

    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_source_udp_ip2(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    arg = __builtin_bswap32(arg);
    LOG(logINFO, ("Setting udp source ip2: 0x%x\n", arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (check_detector_idle("configure mac") == OK) {
            if (udpDetails[0].srcip2 != arg) {
                for (int iRxEntry = 0; iRxEntry != MAX_UDP_DESTINATION;
                     ++iRxEntry) {
                    udpDetails[iRxEntry].srcip2 = arg;
                }
                configure_mac();
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_source_udp_ip2(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t retval = -1;
    LOG(logDEBUG1, ("Getting udp source ip2\n"));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // get only
    retval = udpDetails[0].srcip2;
    retval = __builtin_bswap32(retval);
    LOG(logDEBUG1, ("udp soure ip2 retval: 0x%x\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_dest_udp_ip(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    arg = __builtin_bswap32(arg);
    LOG(logINFO, ("Setting udp destination ip: 0x%x\n", arg));

    // only set
    if (Server_VerifyLock() == OK) {
        if (check_detector_idle("configure mac") == OK) {
            if (udpDetails[0].dstip != arg) {
                udpDetails[0].dstip = arg;
                configure_mac();
            }
        }
    }
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_dest_udp_ip(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t retval = -1;
    LOG(logDEBUG1, ("Getting destination ip\n"));

    // get only
    retval = udpDetails[0].dstip;
    retval = __builtin_bswap32(retval);
    LOG(logDEBUG1, ("udp destination ip retval: 0x%x\n", retval));

    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_dest_udp_ip2(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    arg = __builtin_bswap32(arg);
    LOG(logINFO, ("Setting udp destination ip2: 0x%x\n", arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (check_detector_idle("configure mac") == OK) {
            if (udpDetails[0].dstip2 != arg) {
                udpDetails[0].dstip2 = arg;
                configure_mac();
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_dest_udp_ip2(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t retval = -1;
    LOG(logDEBUG1, ("Getting udp destination ip2\n"));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // get only
    retval = udpDetails[0].dstip2;
    retval = __builtin_bswap32(retval);
    LOG(logDEBUG1, ("udp destination ip2 retval: 0x%x\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_source_udp_mac(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting udp source mac: 0x%lx\n", arg));

    // only set
    if (Server_VerifyLock() == OK) {
        if (check_detector_idle("configure mac") == OK) {
            if (udpDetails[0].srcmac != arg) {
                // multicast (LSB of first octet = 1)
                if ((arg >> 40) & 0x1) {
                    ret = FAIL;
                    sprintf(mess,
                            "Cannot set source mac address. Must be a unicast "
                            "address (LSB of first octet should be 0).");
                    LOG(logERROR, (mess));
                } else {
                    for (int iRxEntry = 0; iRxEntry != MAX_UDP_DESTINATION;
                         ++iRxEntry) {
                        udpDetails[iRxEntry].srcmac = arg;
                    }
                    configure_mac();
                }
            }
        }
    }
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_source_udp_mac(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t retval = -1;
    LOG(logDEBUG1, ("Getting udp source mac\n"));

    // get only
    retval = udpDetails[0].srcmac;
    LOG(logDEBUG1, ("udp soure mac retval: 0x%lx\n", retval));

    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_source_udp_mac2(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting udp source mac2: 0x%lx\n", arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (check_detector_idle("configure mac") == OK) {
            if (udpDetails[0].srcmac2 != arg) {
                for (int iRxEntry = 0; iRxEntry != MAX_UDP_DESTINATION;
                     ++iRxEntry) {
                    udpDetails[iRxEntry].srcmac2 = arg;
                }
                configure_mac();
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_source_udp_mac2(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t retval = -1;
    LOG(logDEBUG1, ("Getting udp source mac2\n"));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // get only
    retval = udpDetails[0].srcmac2;
    LOG(logDEBUG1, ("udp soure mac2 retval: 0x%lx\n", retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_dest_udp_mac(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting udp destination mac: 0x%lx\n", arg));

    // only set
    if (Server_VerifyLock() == OK) {
        if (check_detector_idle("configure mac") == OK) {
            if (udpDetails[0].dstmac != arg) {
                udpDetails[0].dstmac = arg;
                configure_mac();
            }
        }
    }
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_dest_udp_mac(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t retval = -1;
    LOG(logDEBUG1, ("Getting udp destination mac\n"));

    // get only
    retval = udpDetails[0].dstmac;
    LOG(logDEBUG1, ("udp destination mac retval: 0x%lx\n", retval));

    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_dest_udp_mac2(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting udp destination mac2: 0x%lx\n", arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (check_detector_idle("configure mac") == OK) {
            if (udpDetails[0].dstmac2 != arg) {
                udpDetails[0].dstmac2 = arg;
                configure_mac();
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_dest_udp_mac2(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t retval = -1;
    LOG(logDEBUG1, ("Getting udp destination mac2\n"));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // get only
    retval = udpDetails[0].dstmac2;
    LOG(logDEBUG1, ("udp destination mac2 retval: 0x%lx\n", retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_dest_udp_port(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting udp destination port: %u\n", arg));

    // only set
    if (Server_VerifyLock() == OK) {
        if (check_detector_idle("configure mac") == OK) {
            if (udpDetails[0].dstport != arg) {
                udpDetails[0].dstport = arg;
                configure_mac();
            }
        }
    }
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_dest_udp_port(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;
    LOG(logDEBUG1, ("Getting destination port"));

    // get only
    retval = udpDetails[0].dstport;
    LOG(logDEBUG, ("udp destination port retval: %u\n", retval));

    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_dest_udp_port2(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting udp destination port2: %u\n", arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(EIGERD) &&            \
    !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (check_detector_idle("configure mac") == OK) {
            if (udpDetails[0].dstport2 != arg) {
                udpDetails[0].dstport2 = arg;
                configure_mac();
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_dest_udp_port2(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;
    LOG(logDEBUG1, ("Getting destination port2\n"));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(EIGERD) &&            \
    !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // get only
    retval = udpDetails[0].dstport2;
    LOG(logDEBUG1, ("udp destination port2 retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_num_interfaces(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting number of interfaces: %d\n", arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(GOTTHARD2D)
    // fixed number of udp interfaces
    int num_interfaces = getNumberofUDPInterfaces();
    if (arg != num_interfaces) {
        ret = FAIL;
        sprintf(mess,
                "Could not set number of interfaces. Invalid value: %d. Must "
                "be %d\n",
                arg, num_interfaces);
        LOG(logERROR, (mess));
    }
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (arg < 1 || arg > 2) {
            ret = FAIL;
            sprintf(mess,
                    "Could not number of interfaces to %d. Options[1, 2]\n",
                    arg);
            LOG(logERROR, (mess));
        } else if (check_detector_idle("configure mac") == OK) {
            if (getNumberofUDPInterfaces() != arg) {
                setNumberofUDPInterfaces(arg);
                for (int iRxEntry = 0; iRxEntry != numUdpDestinations;
                     ++iRxEntry) {
                    if (arg == 1) {
                        udpDetails[iRxEntry].srcport2 = 0;
                        udpDetails[iRxEntry].srcip2 = 0;
                        udpDetails[iRxEntry].srcmac2 = 0;
                        udpDetails[iRxEntry].dstport2 = 0;
                        udpDetails[iRxEntry].dstip2 = 0;
                        udpDetails[iRxEntry].dstmac2 = 0;
                    } else {
                        // if still 0, set defaults
                        udpDetails[iRxEntry].srcport2 =
                            DEFAULT_UDP_SRC_PORTNO + 1;
                        if (udpDetails[iRxEntry].dstport2 == 0) {
                            udpDetails[iRxEntry].dstport2 =
                                2 * iRxEntry + 1 + DEFAULT_UDP_DST_PORTNO;
                        }
                        // if still 0, copy from entry 0
                        if (iRxEntry != 0) {
                            udpDetails[iRxEntry].srcip2 = udpDetails[0].srcip2;
                            udpDetails[iRxEntry].srcmac2 =
                                udpDetails[0].srcmac2;
                            if (udpDetails[iRxEntry].dstip2 == 0) {
                                udpDetails[iRxEntry].dstip2 =
                                    udpDetails[0].dstip2;
                            }
                            if (udpDetails[iRxEntry].dstmac2 == 0) {
                                udpDetails[iRxEntry].dstmac2 =
                                    udpDetails[0].dstmac2;
                            }
                        }
                    }
                }
                calculate_and_set_position(); // aleady configures mac
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_num_interfaces(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;
    LOG(logDEBUG1, ("Getting number of udp interfaces\n"));

    // get only
    retval = getNumberofUDPInterfaces();

    LOG(logDEBUG1, ("Number of udp interfaces retval: %u\n", retval));
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_interface_sel(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting selected interface: %d\n", arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (arg < 0 || arg > 1) {
            ret = FAIL;
            sprintf(mess, "Could not set primary interface %d. Options[0, 1]\n",
                    arg);
            LOG(logERROR, (mess));
        } else if (check_detector_idle("configure mac") == OK) {
            if (getPrimaryInterface() != arg) {
                selectPrimaryInterface(arg);
                configure_mac();
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_interface_sel(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;
    LOG(logDEBUG1, ("Getting selected interface\n"));

#if !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    // get only
    retval = getPrimaryInterface();
    LOG(logDEBUG1, ("Selected interface retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_parallel_mode(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting parallel mode: %u\n", arg));

#if !defined(EIGERD) && !defined(MYTHEN3D) && !defined(GOTTHARD2D) &&          \
    !defined(MOENCHD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (setParallelMode(arg) == FAIL) {
            ret = FAIL;
            sprintf(mess, "Could not set parallel mode\n");
            LOG(logERROR, (mess));
        } else {
            int retval = getParallelMode();
            if (arg != retval) {
                ret = FAIL;
                sprintf(mess,
                        "Could not set parallel mode. Set %d, but read %d\n",
                        retval, arg);
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_parallel_mode(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting parallel mode\n"));

#if !defined(EIGERD) && !defined(MYTHEN3D) && !defined(GOTTHARD2D) &&          \
    !defined(MOENCHD)
    functionNotImplemented();
#else
    // get only
    retval = getParallelMode();
    LOG(logDEBUG1, ("parallel mode retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_overflow_mode(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting overflow mode: %u\n", arg));

#ifndef EIGERD
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (setOverFlowMode(arg) == FAIL) {
            ret = FAIL;
            sprintf(mess, "Could not set overflow mode\n");
            LOG(logERROR, (mess));
        } else {
            int retval = getOverFlowMode();
            if (arg != retval) {
                ret = FAIL;
                sprintf(mess,
                        "Could not set overflow mode. Set %d, but read %d\n",
                        retval, arg);
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_overflow_mode(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting overflow mode\n"));

#ifndef EIGERD
    functionNotImplemented();
#else
    // get only
    retval = getOverFlowMode();
    LOG(logDEBUG1, ("overflow mode retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_readout_mode(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting readout mode: %u\n", arg));

#ifndef CHIPTESTBOARDD
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        switch (arg) {
        case ANALOG_ONLY:
        case DIGITAL_ONLY:
        case ANALOG_AND_DIGITAL:
            break;
        default:
            modeNotImplemented("Readout mode", (int)arg);
            break;
        }
        if (ret == OK) {
            if (setReadoutMode(arg) == FAIL) {
                ret = FAIL;
                sprintf(mess, "Could not set readout mode. Check #samples or "
                              "memory allocation\n");
                LOG(logERROR, (mess));
            } else {
                int retval = getReadoutMode();
                if (retval == -1) {
                    ret = FAIL;
                    sprintf(mess, "Could not get readout mode\n");
                    LOG(logERROR, (mess));
                } else {
                    LOG(logDEBUG1, ("readout mode retval: %u\n", retval));
                }
                validate(&ret, mess, arg, retval, "set readout mode", DEC);
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_readout_mode(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting readout mode\n"));

#ifndef CHIPTESTBOARDD
    functionNotImplemented();
#else
    // get only
    retval = getReadoutMode();
    if (retval == -1) {
        ret = FAIL;
        sprintf(mess, "Could not get readout mode\n");
        LOG(logERROR, (mess));
    } else {
        LOG(logDEBUG1, ("readout mode retval: %u\n", retval));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_clock_frequency(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[2] = {-1, -1};

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting clock (%d) frequency : %u\n", args[0], args[1]));

#if !defined(CHIPTESTBOARDD)
    functionNotImplemented();
#else

    // only set
    if (Server_VerifyLock() == OK) {
        int ind = args[0];
        int val = args[1];
        enum CLKINDEX c = 0;
        switch (ind) {
        case ADC_CLOCK:
            c = ADC_CLK;
            break;
#ifdef CHIPTESTBOARDD
        case DBIT_CLOCK:
            c = DBIT_CLK;
            break;
#endif
        case RUN_CLOCK:
            c = RUN_CLK;
            break;
        case SYNC_CLOCK:
            ret = FAIL;
            sprintf(mess, "Cannot set sync clock frequency.\n");
            LOG(logERROR, (mess));
            break;
        default:
            modeNotImplemented("clock index (frequency set)", ind);
            break;
        }

        if (ret != FAIL) {
            char *clock_names[] = {CLK_NAMES};
            char modeName[50] = "";
            sprintf(modeName, "%s clock (%d) frequency", clock_names[c],
                    (int)c);

            if (getFrequency(c) == val) {
                LOG(logINFO, ("Same %s: %d %s\n", modeName, val,
                              myDetectorType == GOTTHARD2 ? "Hz" : "MHz"));
            } else {
                setFrequency(c, val);
                int retval = getFrequency(c);
                LOG(logDEBUG1, ("retval %s: %d %s\n", modeName, retval,
                                myDetectorType == GOTTHARD2 ? "Hz" : "MHz"));
                validate(&ret, mess, val, retval, modeName, DEC);
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_clock_frequency(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Getting clock (%d) frequency\n", arg));

#if !defined(CHIPTESTBOARDD) && !defined(GOTTHARD2D) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // get only
    enum CLKINDEX c = 0;
    switch (arg) {
#if defined(CHIPTESTBOARDD)
    case ADC_CLOCK:
        c = ADC_CLK;
        break;
#ifdef CHIPTESTBOARDD
    case DBIT_CLOCK:
        c = DBIT_CLK;
        break;
#endif
    case RUN_CLOCK:
        c = RUN_CLK;
        break;
    case SYNC_CLOCK:
        c = SYNC_CLK;
        break;
#endif
    default:
#if defined(GOTTHARD2D) || defined(MYTHEN3D)
        if (arg < NUM_CLOCKS) {
            c = (enum CLKINDEX)arg;
            break;
        }
#endif
        modeNotImplemented("clock index (frequency get)", arg);
        break;
    }
    if (ret == OK) {
        retval = getFrequency(c);
        char *clock_names[] = {CLK_NAMES};
        LOG(logDEBUG1,
            ("retval %s clock (%d) frequency: %d %s\n", clock_names[c], (int)c,
             retval,
             myDetectorType == GOTTHARD2 || myDetectorType == MYTHEN3 ? "Hz"
                                                                      : "MHz"));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_clock_phase(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[3] = {-1, -1, -1};

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting clock (%d) phase: %u %s\n", args[0], args[1],
                    (args[2] == 0 ? "" : "degrees")));

#if !defined(CHIPTESTBOARDD) && !defined(JUNGFRAUD) && !defined(MOENCHD) &&    \
    !defined(GOTTHARDD) && !defined(GOTTHARD2D) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        int ind = args[0];
        int val = args[1];
        int inDegrees = args[2] == 0 ? 0 : 1;
        enum CLKINDEX c = 0;
        switch (ind) {
#if defined(CHIPTESTBOARDD) || defined(JUNGFRAUD) || defined(MOENCHD) ||       \
    defined(GOTTHARDD)
        case ADC_CLOCK:
            c = ADC_CLK;
            break;
#endif
#if defined(CHIPTESTBOARDD) || defined(JUNGFRAUD)
        case DBIT_CLOCK:
            c = DBIT_CLK;
            break;
#endif
        default:
#if defined(GOTTHARD2D) || defined(MYTHEN3D)
            if (ind < NUM_CLOCKS) {
                c = (enum CLKINDEX)ind;
                break;
            }
#endif
            modeNotImplemented("clock index (phase set)", ind);
            break;
        }
        if (ret != FAIL) {
            char *clock_names[] = {CLK_NAMES};
            char modeName[50] = "";
            sprintf(modeName, "%s clock (%d) phase %s", clock_names[c], (int)c,
                    (inDegrees == 0 ? "" : "(degrees)"));

            // gotthard1d doesnt take degrees and cannot get phase
#ifdef GOTTHARDD
            if (inDegrees != 0) {
                ret = FAIL;
                strcpy(mess,
                       "Cannot set phase in degrees for this detector.\n");
                LOG(logERROR, (mess));
            }
#else
            if (getPhase(c, inDegrees) == val) {
                LOG(logINFO, ("Same %s: %d\n", modeName, val));
            } else if (inDegrees && (val < 0 || val > 359)) {
                ret = FAIL;
                sprintf(mess,
                        "Cannot set %s to %d degrees. Phase outside limits (0 "
                        "- 359°C)\n",
                        modeName, val);
                LOG(logERROR, (mess));
            } else if (!inDegrees && (val < 0 || val > getMaxPhase(c) - 1)) {
                ret = FAIL;
                sprintf(mess,
                        "Cannot set %s to %d. Phase outside limits (0 - %d "
                        "phase shifts)\n",
                        modeName, val, getMaxPhase(c) - 1);
                LOG(logERROR, (mess));
            }
#endif
            else {
                int ret = setPhase(c, val, inDegrees);
                if (ret == FAIL) {
                    sprintf(mess, "Could not set %s to %d.\n", modeName, val);
                    LOG(logERROR, (mess));
                }

                // gotthard1d doesnt take degrees and cannot get phase
#ifndef GOTTHARDD
                else {
                    int retval = getPhase(c, inDegrees);
                    LOG(logDEBUG1, ("retval %s : %d\n", modeName, retval));
                    if (!inDegrees) {
                        validate(&ret, mess, val, retval, modeName, DEC);
                    } else {
                        ret = validatePhaseinDegrees(c, val, retval);
                        if (ret == FAIL) {
                            sprintf(mess,
                                    "Could not set %s. Set %d degrees, got %d "
                                    "degrees\n",
                                    modeName, val, retval);
                            LOG(logERROR, (mess));
                        }
                    }
                }
#endif
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_clock_phase(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[2] = {-1, -1};
    int retval = -1;

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    LOG(logINFOBLUE, ("Getting clock (%d) phase %s \n", args[0],
                      (args[1] == 0 ? "" : "in degrees")));

#if !defined(CHIPTESTBOARDD) && !defined(JUNGFRAUD) && !defined(MOENCHD) &&    \
    !defined(GOTTHARD2D) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // get only
    int ind = args[0];
    int inDegrees = args[1] == 0 ? 0 : 1;
    enum CLKINDEX c = 0;
    switch (ind) {
#if defined(CHIPTESTBOARDD) || defined(JUNGFRAUD) || defined(MOENCHD)
    case ADC_CLOCK:
        c = ADC_CLK;
        break;
#endif
#if defined(CHIPTESTBOARDD) || defined(JUNGFRAUD)
    case DBIT_CLOCK:
        c = DBIT_CLK;
        break;
#endif
    default:
#if defined(GOTTHARD2D) || defined(MYTHEN3D)
        if (ind < NUM_CLOCKS) {
            c = (enum CLKINDEX)ind;
            LOG(logINFOBLUE, ("NUMclocks:%d c:%d\n", NUM_CLOCKS, c));
            break;
        }
#endif
        modeNotImplemented("clock index (phase get)", ind);
        break;
    }
    if (ret == OK) {
        retval = getPhase(c, inDegrees);
        char *clock_names[] = {CLK_NAMES};
        LOG(logDEBUG1, ("retval %s clock (%d) phase: %d %s\n", clock_names[c],
                        (int)c, retval, (inDegrees == 0 ? "" : "degrees")));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int get_max_clock_phase_shift(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Getting clock (%d) max phase shift\n", arg));

#if !defined(CHIPTESTBOARDD) && !defined(JUNGFRAUD) && !defined(MOENCHD) &&    \
    !defined(GOTTHARD2D) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // get only
    enum CLKINDEX c = 0;
    switch (arg) {
#if defined(CHIPTESTBOARDD) || defined(JUNGFRAUD) || defined(MOENCHD)
    case ADC_CLOCK:
        c = ADC_CLK;
        break;
#endif
#if defined(CHIPTESTBOARDD) || defined(JUNGFRAUD)
    case DBIT_CLOCK:
        c = DBIT_CLK;
        break;
#endif
    default:
#if defined(GOTTHARD2D) || defined(MYTHEN3D)
        if (arg < NUM_CLOCKS) {
            c = (enum CLKINDEX)arg;
            break;
        }
#endif
        modeNotImplemented("clock index (max phase get)", arg);
        break;
    }
    if (ret == OK) {
        retval = getMaxPhase(c);
        char *clock_names[] = {CLK_NAMES};
        LOG(logDEBUG1, ("retval %s clock (%d) max phase shift: %d\n",
                        clock_names[c], (int)c, retval));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_clock_divider(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[2] = {-1, -1};

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting clock (%d) divider: %u\n", args[0], args[1]));

#if !defined(GOTTHARD2D) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {

#ifdef MYTHEN3D
        if (args[0] >= NUM_CLOCKS_TO_SET) {
#else
        if (args[0] >= NUM_CLOCKS) {
#endif
            modeNotImplemented("clock index (divider set)", args[0]);
        }
        // TODO: if value between to set and num clocks, msg = "cannot set"
        enum CLKINDEX c = 0;
        int val = args[1];
        if (ret == OK) {
            c = (enum CLKINDEX)args[0];
            // validate val range
            if (val < 2 || val > getMaxClockDivider()) {
                char *clock_names[] = {CLK_NAMES};
                ret = FAIL;
                sprintf(mess,
                        "Cannot set %s clock(%d) to %d. Value should be in "
                        "range [2-%d]\n",
                        clock_names[c], (int)c, val, getMaxClockDivider());
                LOG(logERROR, (mess));
            }
        }

        if (ret != FAIL) {
            char modeName[50];
            char *clock_names[] = {CLK_NAMES};
            sprintf(modeName, "%s clock (%d) divider", clock_names[c], (int)c);
            if (getClockDivider(c) == val) {
                LOG(logINFO, ("Same %s: %d\n", modeName, val));
            } else {
                int ret = setClockDivider(c, val);
                if (ret == FAIL) {
                    sprintf(mess, "Could not set %s to %d.\n", modeName, val);
                    LOG(logERROR, (mess));
                } else {
                    int retval = getClockDivider(c);
                    LOG(logDEBUG1, ("retval %s : %d\n", modeName, retval));
                    validate(&ret, mess, val, retval, modeName, DEC);
                }
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_clock_divider(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Getting clock (%d) divider\n", arg));

#if !defined(GOTTHARD2D) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // get only
    if (arg >= NUM_CLOCKS) {
        modeNotImplemented("clock index (divider get)", arg);
    }
    if (ret == OK) {
        enum CLKINDEX c = (enum CLKINDEX)arg;
        retval = getClockDivider(c);
        char *clock_names[] = {CLK_NAMES};
        LOG(logDEBUG1, ("retval %s clock (%d) divider: %d\n", clock_names[c],
                        (int)c, retval));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_on_chip_dac(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[3] = {-1, -1, -1};

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting On chip dac (%d), chip %d: 0x%x\n", args[0],
                    args[1], args[2]));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else

    // only set
    if (Server_VerifyLock() == OK) {
        int ind = args[0];
        int chipIndex = args[1];
        int val = args[2];
        enum ONCHIP_DACINDEX dacIndex = 0;
        switch (ind) {
        case VB_COMP_FE:
            dacIndex = G2_VCHIP_COMP_FE;
            break;
        case VB_OPA_1ST:
            dacIndex = G2_VCHIP_OPA_1ST;
            break;
        case VB_OPA_FD:
            dacIndex = G2_VCHIP_OPA_FD;
            break;
        case VB_COMP_ADC:
            dacIndex = G2_VCHIP_COMP_ADC;
            break;
        case VREF_COMP_FE:
            dacIndex = G2_VCHIP_REF_COMP_FE;
            break;
        case VB_CS:
            dacIndex = G2_VCHIP_CS;
            break;
        default:
            modeNotImplemented("on chip dac index", ind);
            break;
        }

        if (ret != FAIL) {
            char *names[] = {ONCHIP_DAC_NAMES};
            char modeName[50] = "";
            sprintf(modeName, "on-chip-dac (%s, %d, chip:%d)", names[dacIndex],
                    (int)dacIndex, chipIndex);
            if (chipIndex < -1 || chipIndex >= NCHIP) {
                ret = FAIL;
                sprintf(mess,
                        "Could not set %s to %d. Invalid Chip Index. "
                        "Options[-1, 0 - %d]\n",
                        modeName, val, NCHIP - 1);
                LOG(logERROR, (mess));
            } else if (val < 0 || val > ONCHIP_DAC_MAX_VAL) {
                ret = FAIL;
                sprintf(mess,
                        "Could not set %s to 0x%x. Invalid value. Options:[0 - "
                        "0x%x]\n",
                        modeName, val, ONCHIP_DAC_MAX_VAL);
                LOG(logERROR, (mess));
            } else {
                ret = setOnChipDAC(dacIndex, chipIndex, val);
                if (ret == FAIL) {
                    sprintf(mess, "Could not set %s to 0x%x.\n", modeName, val);
                    LOG(logERROR, (mess));
                } else {
                    int retval = getOnChipDAC(dacIndex, chipIndex);
                    LOG(logDEBUG1, ("retval %s: 0x%x\n", modeName, retval));
                    validate(&ret, mess, val, retval, modeName, DEC);
                }
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_on_chip_dac(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[2] = {-1, -1};
    int retval = -1;

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Getting On chip dac (%d), chip %d\n", args[0], args[1]));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // get only
    int ind = args[0];
    int chipIndex = args[1];
    enum ONCHIP_DACINDEX dacIndex = 0;
    switch (ind) {
    case VB_COMP_FE:
        dacIndex = G2_VCHIP_COMP_FE;
        break;
    case VB_OPA_1ST:
        dacIndex = G2_VCHIP_OPA_1ST;
        break;
    case VB_OPA_FD:
        dacIndex = G2_VCHIP_OPA_FD;
        break;
    case VB_COMP_ADC:
        dacIndex = G2_VCHIP_COMP_ADC;
        break;
    case VREF_COMP_FE:
        dacIndex = G2_VCHIP_REF_COMP_FE;
        break;
    case VB_CS:
        dacIndex = G2_VCHIP_CS;
        break;
    default:
        modeNotImplemented("on chip dac index", ind);
        break;
    }
    if (ret == OK) {
        char *names[] = {ONCHIP_DAC_NAMES};
        char modeName[50] = "";
        sprintf(modeName, "on-chip-dac (%s, %d, chip:%d)", names[dacIndex],
                (int)dacIndex, chipIndex);
        if (chipIndex < -1 || chipIndex >= NCHIP) {
            ret = FAIL;
            sprintf(mess,
                    "Could not get %s. Invalid Chip Index. Options[-1, 0 - "
                    "%d]\n",
                    modeName, NCHIP - 1);
            LOG(logERROR, (mess));
        } else {
            retval = getOnChipDAC(dacIndex, chipIndex);
            LOG(logDEBUG1, ("retval %s: 0x%x\n", modeName, retval));
        }
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_inject_channel(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[2] = {-1, -1};

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting inject channel: [%d, %d]\n", args[0], args[1]));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        int offset = args[0];
        int increment = args[1];
        if (offset < 0 || increment < 1) {
            ret = FAIL;
            sprintf(mess,
                    "Could not inject channel. Invalid offset %d or "
                    "increment %d\n",
                    offset, increment);
            LOG(logERROR, (mess));
        } else {
            ret = setInjectChannel(offset, increment);
            if (ret == FAIL) {
                strcpy(mess, "Could not inject channel\n");
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_inject_channel(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retvals[2] = {-1, -1};

    LOG(logDEBUG1, ("Getting injected channels\n"));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // get only
    int offset = -1, increment = -1;
    getInjectedChannels(&offset, &increment);
    LOG(logDEBUG1, ("Get Injected channels: [offset:%d, increment:%d]\n",
                    offset, increment));
    retvals[0] = offset;
    retvals[1] = increment;
#endif
    return Server_SendResult(file_des, INT32, retvals, sizeof(retvals));
}

int set_veto_photon(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

    int args[2] = {-1, -1};
    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    const int chipIndex = args[0];
    const int numChannels = args[1];

    int *gainIndices = malloc(sizeof(int) * numChannels);
    if (receiveData(file_des, gainIndices, sizeof(int) * numChannels, INT32) <
        0) {
        free(gainIndices);
        return printSocketReadError();
    }

    int *values = malloc(sizeof(int) * numChannels);
    if (receiveData(file_des, values, sizeof(int) * numChannels, INT32) < 0) {
        free(gainIndices);
        free(values);
        return printSocketReadError();
    }

    LOG(logINFO, ("Setting Veto Photon: [chipIndex:%d, nch:%d]\n", chipIndex,
                  numChannels));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (numChannels != NCHAN) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set veto photon. Invalid number of channels %d. "
                    "Expected %d\n",
                    numChannels, NCHAN);
            LOG(logERROR, (mess));
        } else if (chipIndex < -1 || chipIndex >= NCHIP) {
            ret = FAIL;
            sprintf(mess, "Could not set veto photon. Invalid chip index %d\n",
                    chipIndex);
            LOG(logERROR, (mess));
        } else {
            for (int i = 0; i < NCHAN; ++i) {
                if (gainIndices[i] < 0 || gainIndices[i] > 2) {
                    ret = FAIL;
                    sprintf(mess,
                            "Could not set veto photon. Invalid gain index %d "
                            "for channel %d.\n",
                            gainIndices[i], i);
                    LOG(logERROR, (mess));
                    break;
                }
                if (values[i] > ADU_MAX_VAL) {
                    ret = FAIL;
                    sprintf(mess,
                            "Could not set veto photon. Invalid ADU value 0x%x "
                            "for channel %d, must be 12 bit.\n",
                            values[i], i);
                    LOG(logERROR, (mess));
                    break;
                }
            }
            if (ret == OK) {
                ret = setVetoPhoton(chipIndex, gainIndices, values);
                if (ret == FAIL) {
                    sprintf(mess,
                            "Could not set veto photon for chip index %d\n",
                            chipIndex);
                    LOG(logERROR, (mess));
                }
            }
        }
    }
#endif
    if (gainIndices != NULL) {
        free(gainIndices);
    }
    if (values != NULL) {
        free(values);
    }
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_veto_photon(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    int *retvals = NULL;
    int *gainRetvals = NULL;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Getting veto photon [chip Index:%d]\n", arg));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    retvals = malloc(sizeof(int) * NCHAN);
    gainRetvals = malloc(sizeof(int) * NCHAN);
    memset(retvals, 0, sizeof(int) * NCHAN);
    memset(gainRetvals, 0, sizeof(int) * NCHAN);

    if (retvals == NULL || gainRetvals == NULL) {
        ret = FAIL;
        strcpy(
            mess,
            "Could not get veto photon. Could not allocate memory in server\n");
        LOG(logERROR, (mess));
    } else {
        // get only
        int chipIndex = arg;
        if (chipIndex < -1 || chipIndex >= NCHIP) {
            ret = FAIL;
            sprintf(mess, "Could not get veto photon. Invalid chip index %d\n",
                    chipIndex);
            LOG(logERROR, (mess));
        } else {
            ret = getVetoPhoton(chipIndex, retvals, gainRetvals);
            if (ret == FAIL) {
                strcpy(mess,
                       "Could not get veto photon for chipIndex -1. Not the "
                       "same for all chips. Select specific chip index "
                       "instead.\n");
                LOG(logERROR, (mess));
            } else {
                for (int i = 0; i < NCHAN; ++i) {
                    LOG(logDEBUG1,
                        ("%d:[%d, %d]\n", i, retvals[i], gainRetvals[i]));
                }
            }
        }
    }
#endif
    Server_SendResult(file_des, INT32, NULL, 0);
    if (ret != FAIL) {
        int nch = NCHAN;
        sendData(file_des, &nch, sizeof(nch), INT32);
        sendData(file_des, gainRetvals, sizeof(int) * NCHAN, INT32);
        sendData(file_des, retvals, sizeof(int) * NCHAN, INT32);
    }
    if (retvals != NULL) {
        free(retvals);
    }
    if (gainRetvals != NULL) {
        free(gainRetvals);
    }
    return ret;
}

int set_veto_reference(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[2] = {-1, -1};

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO,
        ("Setting Veto Reference: [G%d, value:%d]\n", args[0], args[1]));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        int gainIndex = args[0];
        int value = args[1];
        if (gainIndex < 0 || gainIndex > 2) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set veto reference. Invalid gain index %d\n",
                    gainIndex);
            LOG(logERROR, (mess));
        } else if (value > ADU_MAX_VAL) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set veto reference. Invalid ADU value %d, "
                    "must be 12 bit.\n",
                    value);
            LOG(logERROR, (mess));
        } else {
            ret = setVetoReference(gainIndex, value);
            if (ret == FAIL) {
                sprintf(mess, "Could not set veto reference\n");
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int set_burst_mode(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum burstMode arg = BURST_INTERNAL;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting burst mode: %d\n", arg));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        switch (arg) {
        case BURST_INTERNAL:
        case BURST_EXTERNAL:
        case CONTINUOUS_INTERNAL:
        case CONTINUOUS_EXTERNAL:
            break;
        default:
            modeNotImplemented("Burst mode", (int)arg);
            break;
        }
        if (ret == OK) {
            setBurstMode(arg);
            enum burstMode retval = getBurstMode();
            LOG(logDEBUG, ("burst mode retval: %d\n", retval));
            if (retval != arg) {
                ret = FAIL;
                sprintf(mess, "Could not set burst type. Set %d, got %d\n", arg,
                        retval);
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_burst_mode(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum burstMode retval = BURST_INTERNAL;

    LOG(logDEBUG1, ("Getting burst mode\n"));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // get only
    retval = getBurstMode();
    LOG(logDEBUG1, ("Get burst mode retval:%d\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_counter_mask(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();

    LOG(logINFO, ("Setting Counter mask:0x%x\n", arg));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (arg == 0) {
            ret = FAIL;
            sprintf(mess, "Could not set counter mask. Cannot set it to 0.\n");
            LOG(logERROR, (mess));
        } else if (arg > MAX_COUNTER_MSK) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set counter mask. Invalid counter bit enabled. "
                    "Max number of counters: %d\n",
                    NCOUNTERS);
            LOG(logERROR, (mess));
        } else {
            setCounterMask(arg);
            uint32_t retval = getCounterMask();
            LOG(logDEBUG, ("counter mask retval: 0x%x\n", retval));
            if (retval != arg) {
                ret = FAIL;
                sprintf(mess,
                        "Could not set counter mask. Set 0x%x mask, got 0x%x "
                        "mask\n",
                        arg, retval);
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_counter_mask(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t retval = -1;
    LOG(logDEBUG1, ("Getting counter mask\n"));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    // get only
    retval = getCounterMask();
    LOG(logDEBUG, ("counter mask retval: 0x%x\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int get_num_bursts(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // get only
    retval = getNumBursts();
    LOG(logDEBUG1, ("retval num bursts %lld\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_num_bursts(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting number of bursts %lld\n", (long long int)arg));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        setNumBursts(arg);
        int64_t retval = getNumBursts();
        LOG(logDEBUG1, ("retval num bursts %lld\n", (long long int)retval));
        validate64(&ret, mess, arg, retval, "set number of bursts", DEC);
    }
#endif
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_burst_period(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // get only
    retval = getBurstPeriod();
    LOG(logDEBUG1, ("retval burst period %lld ns\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_burst_period(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting burst period %lld ns\n", (long long int)arg));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        ret = setBurstPeriod(arg);
        int64_t retval = getBurstPeriod();
        LOG(logDEBUG1,
            ("retval burst period %lld ns\n", (long long int)retval));
        if (ret == FAIL) {
            sprintf(mess,
                    "Could not set burst period. Set %lld ns, read %lld ns.\n",
                    (long long int)arg, (long long int)retval);
            LOG(logERROR, (mess));
        }
    }
#endif
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int set_current_source(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint64_t select = 0;
    int args[3] = {-1, -1, -1};
    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    if (receiveData(file_des, &select, sizeof(select), INT64) < 0)
        return printSocketReadError();
    int enable = args[0];
    int fix = args[1];
    int normal = args[2];

    LOG(logDEBUG1, ("Setting current source [enable:%d, fix:%d, select:%lld, "
                    "normal:%d]\n",
                    enable, fix, (long long int)select, normal));

#if !defined(GOTTHARD2D) && !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (enable != 0 && enable != 1) {
            ret = FAIL;
            strcpy(mess, "Could not enable/disable current source. Enable can "
                         "be 0 or 1 only.\n");
            LOG(logERROR, (mess));
        }
        // disable
        else if (enable == 0 && (fix != -1 || normal != -1)) {
            ret = FAIL;
            strcpy(
                mess,
                "Could not disable current source. Requires no parameters.\n");
            LOG(logERROR, (mess));
        }
        // enable
        else if (enable == 1) {
#ifdef GOTTHARD2D
            // no parameters allowed
            if (fix != -1 || normal != -1) {
                ret = FAIL;
                strcpy(mess, "Could not enable current source. Fix and normal "
                             "are invalid parameters for this detector.\n");
                LOG(logERROR, (mess));
            }
#else
            int chipVersion = getChipVersion();
            if (ret == OK) {
                if (chipVersion == 11) {
                    // require both
                    if ((fix != 0 && fix != 1) ||
                        (normal != 0 && normal != 1)) {
                        ret = FAIL;
                        strcpy(mess, "Could not enable current source. Invalid "
                                     "or insufficient parameters (fix or "
                                     "normal). or Options: 0 or 1.\n");
                        LOG(logERROR, (mess));
                    }
                }
                // chipv1.0
                else {
                    // require only fix
                    if (fix != 0 && fix != 1) {
                        ret = FAIL;
                        strcpy(mess,
                               "Could not enable current source. Invalid value "
                               "for parameter (fix). Options: 0 or 1.\n");
                        LOG(logERROR, (mess));
                    } else if (normal != -1) {
                        ret = FAIL;
                        strcpy(mess, "Could not enable current source. Invalid "
                                     "parmaeter (normal). Require only fix and "
                                     "select for chipv1.0.\n");
                        LOG(logERROR, (mess));
                    }
                    // select can only be 0-63
                    else if (select > MAX_SELECT_CHIP10_VAL) {
                        ret = FAIL;
                        strcpy(mess,
                               "Could not enable current source. Invalid value "
                               "for parameter (select). Options: 0-63.\n");
                        LOG(logERROR, (mess));
                    }
                }
            }
#endif
        }

        if (ret == OK) {
#if defined(JUNGFRAUD)
            if (enable == 0) {
                disableCurrentSource();
            } else {
                enableCurrentSource(fix, select, normal);
            }
#else
            setCurrentSource(enable);
#endif
            int retval = getCurrentSource();
            LOG(logDEBUG1, ("current source enable retval: %u\n", retval));
            validate(&ret, mess, enable, retval, "set current source enable",
                     DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_current_source(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retvals[3] = {-1, -1, -1};
    uint64_t retval_select = 0;

    LOG(logDEBUG1, ("Getting current source\n"));

#if !defined(GOTTHARD2D) && !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // get only
    retvals[0] = getCurrentSource();
    LOG(logDEBUG1, ("current source enable retval: %u\n", retvals[0]));
#if defined(JUNGFRAUD)
    if (retvals[0]) {
        retvals[1] = getFixCurrentSource();
        retvals[2] = getNormalCurrentSource();
        retval_select = getSelectCurrentSource();
    }
    LOG(logDEBUG1, ("current source parameters retval: [enable:%d fix:%d, "
                    "normal:%d, select:%lld]\n",
                    retvals[0], retvals[1], retvals[2], retval_select));
#endif
#endif
    Server_SendResult(file_des, INT32, NULL, 0);
    if (ret != FAIL) {
        sendData(file_des, retvals, sizeof(retvals), INT32);
        sendData(file_des, &retval_select, sizeof(retval_select), INT64);
    }
    return ret;
}

int set_timing_source(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum timingSourceType arg = TIMING_INTERNAL;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting timing source: %d\n", arg));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        switch (arg) {
        case TIMING_INTERNAL:
        case TIMING_EXTERNAL:
            break;
        default:
            modeNotImplemented("timing source", (int)arg);
            break;
        }
        if (ret == OK) {
            setTimingSource(arg);
            enum timingSourceType retval = getTimingSource();
            LOG(logDEBUG, ("timing source retval: %d\n", retval));
            if (retval != arg) {
                ret = FAIL;
                sprintf(mess, "Could not set timing source. Set %d, got %d\n",
                        arg, retval);
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_timing_source(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum timingSourceType retval = TIMING_INTERNAL;

    LOG(logDEBUG1, ("Getting timing source\n"));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // get only
    retval = getTimingSource();
    LOG(logDEBUG1, ("Get timing source retval:%d\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int get_num_channels(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retvals[2] = {-1, -1};

    LOG(logDEBUG1, ("Getting number of channels\n"));

#if !defined(CHIPTESTBOARDD)
    functionNotImplemented();
#else
    // get only
    getNumberOfChannels(&retvals[0], &retvals[1]);
    LOG(logDEBUG1,
        ("Get number of channels sretval:[%d, %d]\n", retvals[0], retvals[1]));
#endif
    return Server_SendResult(file_des, INT32, retvals, sizeof(retvals));
}

int update_rate_correction(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

#ifndef EIGERD
    functionNotImplemented();
#else
    LOG(logINFO, ("Update Rate Correction\n"));
    // only set
    if (Server_VerifyLock() == OK) {
        ret = updateRateCorrection(mess);
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_receiver_parameters(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

    LOG(logDEBUG1, ("Getting receiver parameters\n"));
    // get only
    Server_SendResult(file_des, INT32, NULL, 0);

    int n = 0;
    int i32 = 0;
    int64_t i64 = 0;
    uint32_t u32 = 0;
    uint64_t u64 = 0;

    // send fake parameters needed for shared memory
    // (so that client can receive a struct)
    // detector type
    i32 = 0;
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();
    // numberOfDetector
    i32 = 0;
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();
    i32 = 0;
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();
    // detId
    i32 = 0;
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();
    // hostname
    {
        char hostname[MAX_STR_LENGTH];
        memset(hostname, 0, MAX_STR_LENGTH);
        n += sendData(file_des, hostname, MAX_STR_LENGTH, OTHER);
        if (n < 0)
            return printSocketReadError();
    }
    // end of shared memory variables in struct

    // sending real detector parameters
    // udp interfaces
    i32 = getNumberofUDPInterfaces();
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

    // udp dst port
    i32 = udpDetails[0].dstport;
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

    // udp dst ip
    u32 = udpDetails[0].dstip;
    u32 = __builtin_bswap32(u32);
    n += sendData(file_des, &u32, sizeof(u32), INT32);
    if (n < 0)
        return printSocketReadError();

    // udp dst mac
    u64 = udpDetails[0].dstmac;
    n += sendData(file_des, &u64, sizeof(u64), INT64);
    if (n < 0)
        return printSocketReadError();

    // udp dst port2
    i32 = udpDetails[0].dstport2;
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

    // udp dst ip2
    u32 = udpDetails[0].dstip2;
    u32 = __builtin_bswap32(u32);
    n += sendData(file_des, &u32, sizeof(u32), INT32);
    if (n < 0)
        return printSocketReadError();

    // udp dst mac2
    u64 = udpDetails[0].dstmac2;
    n += sendData(file_des, &u64, sizeof(u64), INT64);
    if (n < 0)
        return printSocketReadError();

    // frames
    if (!scan) {
        i64 = getNumFrames();
    } else {
        i64 = numScanSteps;
    }
    n += sendData(file_des, &i64, sizeof(i64), INT64);
    if (n < 0)
        return printSocketReadError();

    // triggers
    i64 = getNumTriggers();
    n += sendData(file_des, &i64, sizeof(i64), INT64);
    if (n < 0)
        return printSocketReadError();

        // bursts
#ifdef GOTTHARD2D
    i64 = getNumBursts();
#else
    i64 = 0;
#endif
    n += sendData(file_des, &i64, sizeof(i64), INT64);
    if (n < 0)
        return printSocketReadError();

        // additional storage cells
#if defined(JUNGFRAUD)
    i32 = getNumAdditionalStorageCells();
#else
    i32 = 0;
#endif
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

        // analog samples
#if defined(CHIPTESTBOARDD)
    i32 = getNumAnalogSamples();
#else
    i32 = 0;
#endif
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

        // digital samples
#ifdef CHIPTESTBOARDD
    i32 = getNumDigitalSamples();
#else
    i32 = 0;
#endif
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

        // exptime
#ifdef MYTHEN3D
    i64 = 0;
#else
    i64 = getExpTime();
#endif
    n += sendData(file_des, &i64, sizeof(i64), INT64);
    if (n < 0)
        return printSocketReadError();

    // period
    i64 = getPeriod();
    n += sendData(file_des, &i64, sizeof(i64), INT64);
    if (n < 0)
        return printSocketReadError();

        // sub exptime
#ifdef EIGERD
    i64 = getSubExpTime();
#else
    i64 = 0;
#endif
    n += sendData(file_des, &i64, sizeof(i64), INT64);
    if (n < 0)
        return printSocketReadError();

        // sub deadtime
#ifdef EIGERD
    i64 = getSubDeadTime();
#else
    i64 = 0;
#endif
    n += sendData(file_des, &i64, sizeof(i64), INT64);
    if (n < 0)
        return printSocketReadError();

        // activate
#ifdef EIGERD
    i32 = 0;
    getActivate(&i32);
#else
    i32 = 0;
#endif
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

        // data stream left
#ifdef EIGERD
    i32 = 0;
    getDataStream(LEFT, &i32);
#else
    i32 = 0;
#endif
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

        // data stream right
#ifdef EIGERD
    i32 = 0;
    getDataStream(RIGHT, &i32);
#else
    i32 = 0;
#endif
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

        // quad
#ifdef EIGERD
    i32 = getQuad();
#else
    i32 = 0;
#endif
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

        // ReadNRows
#if defined(EIGERD) || defined(JUNGFRAUD) || defined(MOENCHD)
    i32 = getReadNRows();
#else
    i32 = 0;
#endif
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

    // threshold ev
    {
        int i32s[3] = {0, 0, 0};
#ifdef EIGERD
        i32s[0] = getThresholdEnergy();
#elif MYTHEN3D
        for (int i = 0; i < NCOUNTERS; ++i) {
            i32s[i] = getThresholdEnergy(i);
        }
#endif
        n += sendData(file_des, i32s, sizeof(i32s), INT32);
        if (n < 0)
            return printSocketReadError();
    }

    // dynamic range
    ret = getDynamicRange(&i32);
    if (ret == FAIL) {
        i32 = 0;
    }
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

    // timing mode
    i32 = (int)getTiming();
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

        // 10 gbe
#if defined(EIGERD) || defined(CHIPTESTBOARDD) || defined(MYTHEN3D)
    i32 = enableTenGigabitEthernet(GET_FLAG);
#else
    i32 = 0;
#endif
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

        // readout mode
#ifdef CHIPTESTBOARD
    i32 = getReadoutMode();
#else
    i32 = 0;
#endif
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

        // adc mask
#if defined(CHIPTESTBOARDD)
    u32 = getADCEnableMask();
#else
    u32 = 0;
#endif
    n += sendData(file_des, &u32, sizeof(u32), INT32);
    if (n < 0)
        return printSocketReadError();

        // 10g adc mask
#if defined(CHIPTESTBOARDD)
    u32 = getADCEnableMask_10G();
#else
    u32 = 0;
#endif
    n += sendData(file_des, &u32, sizeof(u32), INT32);
    if (n < 0)
        return printSocketReadError();

    // roi
    {
        ROI roi;
#ifdef GOTTHARDD
        roi = getROI();
#else
        roi.xmin = -1;
        roi.xmax = -1;
        roi.ymin = -1;
        roi.ymax = -1;
#endif
        n += sendData(file_des, &roi.xmin, sizeof(int), INT32);
        if (n < 0)
            return printSocketReadError();
        n += sendData(file_des, &roi.xmax, sizeof(int), INT32);
        if (n < 0)
            return printSocketReadError();
        n += sendData(file_des, &roi.ymin, sizeof(int), INT32);
        if (n < 0)
            return printSocketReadError();
        n += sendData(file_des, &roi.ymax, sizeof(int), INT32);
        if (n < 0)
            return printSocketReadError();
    }

    // counter mask
#ifdef MYTHEN3D
    u32 = getCounterMask();
#else
    u32 = 0;
#endif
    n += sendData(file_des, &u32, sizeof(u32), INT32);
    if (n < 0)
        return printSocketReadError();

        // burst mode
#ifdef GOTTHARD2D
    i32 = (int)getBurstMode();
#else
    i32 = 0;
#endif
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

        // exptime1
#ifdef MYTHEN3D
    i64 = getExpTime(0);
#else
    i64 = 0;
#endif
    n += sendData(file_des, &i64, sizeof(i64), INT64);
    if (n < 0)
        return printSocketReadError();

        // exptime2
#ifdef MYTHEN3D
    i64 = getExpTime(1);
#else
    i64 = 0;
#endif
    n += sendData(file_des, &i64, sizeof(i64), INT64);
    if (n < 0)
        return printSocketReadError();

        // exptime3
#ifdef MYTHEN3D
    i64 = getExpTime(2);
#else
    i64 = 0;
#endif
    n += sendData(file_des, &i64, sizeof(i64), INT64);
    if (n < 0)
        return printSocketReadError();

        // gatedelay1
#ifdef MYTHEN3D
    i64 = getGateDelay(0);
#else
    i64 = 0;
#endif
    n += sendData(file_des, &i64, sizeof(i64), INT64);
    if (n < 0)
        return printSocketReadError();

        // gatedelay2
#ifdef MYTHEN3D
    i64 = getGateDelay(1);
#else
    i64 = 0;
#endif
    n += sendData(file_des, &i64, sizeof(i64), INT64);
    if (n < 0)
        return printSocketReadError();

        // gatedelay3
#ifdef MYTHEN3D
    i64 = getGateDelay(2);
#else
    i64 = 0;
#endif
    n += sendData(file_des, &i64, sizeof(i64), INT64);
    if (n < 0)
        return printSocketReadError();

        // gates
#ifdef MYTHEN3D
    i32 = getNumGates();
#else
    i32 = 0;
#endif
    n += sendData(file_des, &i32, sizeof(i32), INT32);
    if (n < 0)
        return printSocketReadError();

    // scan parameters
    // scan enable, dac, start, stop, step
    // scan dac settle time
    int i32s[5] = {0, 0, 0, 0, 0};
    i64 = 0;
    i32s[0] = scan;
    if (scan) {
        i32s[1] = scanGlobalIndex;
        i32s[2] = scanSteps[0];
        i32s[3] = scanSteps[numScanSteps - 1];
        i32s[4] = scanSteps[1] - scanSteps[0];
        i64 = scanSettleTime_ns;
    }
    n += sendData(file_des, i32s, sizeof(i32s), INT32);
    if (n < 0)
        return printSocketReadError();
    n += sendData(file_des, &i64, sizeof(i64), INT64);
    if (n < 0)
        return printSocketReadError();

    LOG(logINFO, ("Sent %d bytes for receiver parameters\n", n));

    return OK;
}

int start_pattern(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

    LOG(logDEBUG1, ("Starting Pattern\n"));
#ifndef MYTHEN3D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        startPattern();
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int set_num_gates(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting number of gates %d\n", arg));

#if !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        setNumGates(arg);
        int retval = getNumGates();
        LOG(logDEBUG1, ("retval num gates %d\n", retval));
        validate(&ret, mess, arg, retval, "set number of gates", DEC);
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_num_gates(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

#if !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // get only
    retval = getNumGates();
    LOG(logDEBUG1, ("retval num gates %d\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_gate_delay(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t args[2] = {-1, -1};

    if (receiveData(file_des, args, sizeof(args), INT64) < 0)
        return printSocketReadError();
    int gateIndex = args[0];
    int64_t val = args[1];
    LOG(logDEBUG1, ("Setting gate delay %lld ns (gateIndex:%d)\n",
                    (long long int)val, gateIndex));

#if !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (gateIndex < -1 || gateIndex > 2) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set gate delay. Invalid gate index %d. "
                    "Options [-1, 0-2]\n",
                    gateIndex);
            LOG(logERROR, (mess));
        } else {
            // specific gate index
            if (gateIndex != -1) {
                ret = setGateDelay(gateIndex, val);
                int64_t retval = getGateDelay(gateIndex);
                LOG(logDEBUG1,
                    ("retval exptime %lld ns\n", (long long int)retval));
                if (ret == FAIL) {
                    sprintf(mess,
                            "Could not set gate delay. Set %lld ns, read %lld "
                            "ns.\n",
                            (long long int)val, (long long int)retval);
                    LOG(logERROR, (mess));
                }
            }
            // all gate indices
            else {
                for (int i = 0; i != 3; ++i) {
                    ret = setGateDelay(i, val);
                    int64_t retval = getGateDelay(i);
                    LOG(logDEBUG1, ("retval gate delay %lld ns (index:%d)\n",
                                    (long long int)retval, i));
                    if (ret == FAIL) {
                        sprintf(mess,
                                "Could not set gate delay. Set %lld ns, "
                                "read %lld "
                                "ns.\n",
                                (long long int)val, (long long int)retval);
                        LOG(logERROR, (mess));
                        break;
                    }
                }
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_gate_delay(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int gateIndex = -1;
    int64_t retval = -1;

    if (receiveData(file_des, &gateIndex, sizeof(gateIndex), INT32) < 0)
        return printSocketReadError();

#if !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // get only
    if (gateIndex < 0 || gateIndex > 2) {
        ret = FAIL;
        sprintf(mess,
                "Could not set gate delay. Invalid gate index %d. "
                "Options [0-2]\n",
                gateIndex);
        LOG(logERROR, (mess));
    } else {
        retval = getGateDelay(gateIndex);
        LOG(logDEBUG1, ("retval gate delay %lld ns\n", (long long int)retval));
    }
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int get_exptime_all_gates(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retvals[3] = {-1, -1, -1};

#if !defined(MYTHEN3D)
    functionNotImplemented();
#else
    for (int i = 0; i != 3; ++i) {
        retvals[i] = getExpTime(i);
        LOG(logINFO, ("retval exptime %lld ns (index:%d)\n",
                      (long long int)retvals[i], i));
    }
#endif
    return Server_SendResult(file_des, INT64, retvals, sizeof(retvals));
}

int get_gate_delay_all_gates(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retvals[3] = {-1, -1, -1};

#if !defined(MYTHEN3D)
    functionNotImplemented();
#else
    for (int i = 0; i != 3; ++i) {
        retvals[i] = getGateDelay(i);
        LOG(logDEBUG1, ("retval gate delay %lld ns (index:%d)\n",
                        (long long int)retvals[i], i));
    }
#endif
    return Server_SendResult(file_des, INT64, retvals, sizeof(retvals));
}

int get_veto(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting veto\n"));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // get only
    retval = getVeto();
    LOG(logDEBUG1, ("veto mode retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_veto(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting veto mode: %u\n", arg));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        setVeto(arg);
        int retval = getVeto();
        LOG(logDEBUG1, ("veto mode retval: %u\n", retval));
        validate(&ret, mess, arg, retval, "set veto mode", DEC);
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int set_pattern(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

#if !defined(CHIPTESTBOARDD) && !defined(MYTHEN3D)
    functionNotImplemented();
#else

    patternParameters *pat = malloc(sizeof(patternParameters));
    memset(pat, 0, sizeof(patternParameters));
    // ignoring endianness for eiger
    if (receiveData(file_des, pat, sizeof(patternParameters), INT32) < 0) {
        if (pat != NULL)
            free(pat);
        return printSocketReadError();
    }

    if (Server_VerifyLock() == OK) {
        LOG(logINFO, ("Setting Pattern from structure\n"));
        ret = loadPattern(mess, logINFO, pat);
    }
    if (pat != NULL)
        free(pat);
#endif

    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_pattern(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

#if !defined(CHIPTESTBOARDD) && !defined(MYTHEN3D)
    functionNotImplemented();
    return Server_SendResult(file_des, INT32, NULL, 0);
#else

    patternParameters *pat = malloc(sizeof(patternParameters));
    memset(pat, 0, sizeof(patternParameters));

    if (Server_VerifyLock() == OK) {
        ret = getPattern(mess, pat);
    }

    // ignoring endianness for eiger
    int ret =
        Server_SendResult(file_des, INT32, pat, sizeof(patternParameters));
    if (pat != NULL)
        free(pat);
    return ret;
#endif
}

int get_scan(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retvals[5] = {0, 0, 0, 0, 0};
    int64_t retvals_dacTime = 0;

    LOG(logDEBUG1, ("Getting scan\n"));

    // get only
    retvals[0] = scan;
    if (scan) {
        retvals[1] = scanGlobalIndex;
        retvals[2] = scanSteps[0];
        retvals[3] = scanSteps[numScanSteps - 1];
        retvals[4] = scanSteps[1] - scanSteps[0];
        retvals_dacTime = scanSettleTime_ns;
    }
    LOG(logDEBUG1, ("scan retval: [%s, dac:%d, start:%d, stop:%d, step:%d, "
                    "dacTime:%lldns]\n",
                    retvals[0] ? "enabled" : "disabled", retvals[1], retvals[2],
                    retvals[3], retvals[4], (long long int)retvals_dacTime));
    Server_SendResult(file_des, INT32, NULL, 0);
    if (ret != FAIL) {
        sendData(file_des, retvals, sizeof(retvals), INT32);
        sendData(file_des, &retvals_dacTime, sizeof(retvals_dacTime), INT64);
    }
    return ret;
}

int set_scan(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[5] = {-1, -1, -1, -1, -1};
    int64_t dacTime = -1;
    int64_t retval = -1;

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    if (receiveData(file_des, &dacTime, sizeof(dacTime), INT64) < 0)
        return printSocketReadError();

    // only set
    if (Server_VerifyLock() == OK) {
        int enable = args[0];
        enum dacIndex index = args[1];
        int start = args[2];
        int stop = args[3];
        int step = args[4];

        // disable scan
        if (enable == 0) {
            LOG(logINFOBLUE, ("Disabling scan"));
            scan = 0;
            numScanSteps = 0;
            // setting number of frames to 1
            int64_t arg = 1;
            setNumFrames(arg);
            retval = getNumFrames();
            LOG(logDEBUG1, ("retval num frames %lld\n", (long long int)retval));
            validate64(&ret, mess, arg, retval, "set number of frames", DEC);
        }
        // enable scan
        else {
            if ((start < stop && step <= 0) || (stop < start && step >= 0)) {
                ret = FAIL;
                sprintf(mess, "Invalid scan parameters\n");
                LOG(logERROR, (mess));
            } else {
                // trimbit scan
                if (index == TRIMBIT_SCAN) {
                    LOG(logINFOBLUE, ("Trimbit scan enabled\n"));
                    scanTrimbits = 1;
                    scanGlobalIndex = index;
                    scanSettleTime_ns = dacTime;
                }
                // dac scan
                else {
                    // validate index
                    getDACIndex(index);
                    if (ret == OK) {
                        LOG(logINFOBLUE, ("Dac [%d] scan enabled\n", index));
                        scanTrimbits = 0;
                        scanGlobalIndex = index;
                        scanSettleTime_ns = dacTime;
                    }
                }
            }
            // valid scan
            if (ret == OK) {
                scan = 1;
                numScanSteps = (abs(stop - start) / abs(step)) + 1;
                if (scanSteps != NULL) {
                    free(scanSteps);
                }
                scanSteps = malloc(numScanSteps * sizeof(int));
                for (int i = 0; i != numScanSteps; ++i) {
                    scanSteps[i] = start + i * step;
                    LOG(logDEBUG1, ("scansteps[%d]:%d\n", i, scanSteps[i]));
                }
                LOG(logINFOBLUE, ("Enabling scan for %s, start[%d], stop[%d], "
                                  "step[%d], nsteps[%d]\n",
                                  scanTrimbits == 1 ? "trimbits" : "dac", start,
                                  stop, step, numScanSteps));

                // setting number of frames to scansteps
                int64_t arg = 1;
                setNumFrames(arg);
                retval = getNumFrames();
                LOG(logDEBUG1,
                    ("retval num frames %lld\n", (long long int)retval));
                validate64(&ret, mess, arg, retval, "set number of frames",
                           DEC);
                retval = numScanSteps;
            }
        }
    }
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int get_scan_error_message(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    char retvals[MAX_STR_LENGTH];
    memset(retvals, 0, MAX_STR_LENGTH);

    LOG(logDEBUG1, ("Getting scan error message\n"));

    // get only
    strcpy(retvals, scanErrMessage);
    LOG(logDEBUG1, ("scan retval err message: [%s]\n", retvals));

    return Server_SendResult(file_des, OTHER, retvals, sizeof(retvals));
}

int get_cds_gain(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting cds gain enable\n"));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // get only
    retval = getCDSGain();
    LOG(logDEBUG1, ("cds gain enable retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_cds_gain(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting cds gain enable: %u\n", arg));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (arg != 0 && arg != 1) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set CDS gain. Invalid value %d. "
                    "Options [0-1]\n",
                    arg);
            LOG(logERROR, (mess));
        } else {
            setCDSGain(arg);
            int retval = getCDSGain();
            LOG(logDEBUG1, ("cds gain enable retval: %u\n", retval));
            validate(&ret, mess, arg, retval, "set cds gain enable", DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_filter_resistor(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting filter resistor\n"));

#if !defined(GOTTHARD2D) && !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // get only
#if defined(JUNGFRAUD)
    if (getChipVersion() == 10) {
        ret = FAIL;
        strcpy(mess, "Could not get filter cell. Not available for this chip "
                     "version 1.0.\n");
        LOG(logERROR, (mess));
    }
#endif
    if (ret == OK) {
        retval = getFilterResistor();
        LOG(logDEBUG1, ("filter resistor retval: %u\n", retval));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_filter_resistor(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting filter resistor: %u\n", arg));

#if !defined(GOTTHARD2D) && !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (arg < 0 || arg > ASIC_FILTER_MAX_RES_VALUE) {
            ret = FAIL;
            sprintf(
                mess,
                "Could not set filter resistor. Invalid filter argument %d. "
                "Options [0-%d]\n",
                arg, ASIC_FILTER_MAX_RES_VALUE);
            LOG(logERROR, (mess));
        }
#if defined(JUNGFRAUD)
        else if (getChipVersion() == 10) {
            ret = FAIL;
            strcpy(mess, "Could not set filter cell. Not available for this "
                         "chip version 1.0.\n");
            LOG(logERROR, (mess));
        }
#endif
        else {
            ret = setFilterResistor(arg);
            if (ret == FAIL) {
                strcpy(mess, "Could not set filter resistor.\n");
                LOG(logERROR, (mess));
            }
#if defined(GOTTHARD2D)
            // jungfrau might take time to update status register if acquiring
            int retval = getFilterResistor();
            LOG(logDEBUG1, ("filter resistor retval: %u\n", retval));
            validate(&ret, mess, arg, retval, "set filter resistor", DEC);
#endif
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_adc_config(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[2] = {-1, -1};
    int retval = -1;

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Getting adc configuration [chipIndex:%d, adcIndex:%d]\n",
                    args[0], args[1]));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // get only
    int chipIndex = args[0];
    int adcIndex = args[1];
    if (chipIndex < -1 || chipIndex >= NCHIP) {
        ret = FAIL;
        sprintf(mess,
                "Could not get adc configuration. Invalid chip index %d\n",
                chipIndex);
        LOG(logERROR, (mess));
    } else if (adcIndex < -1 || adcIndex >= NADC) {
        ret = FAIL;
        sprintf(mess, "Could not get adc configuration. Invalid adc index %d\n",
                adcIndex);
        LOG(logERROR, (mess));
    } else {
        retval = getADCConfiguration(chipIndex, adcIndex);
        LOG(logDEBUG1, ("adc config retval: %u\n", retval));
        if (retval == -1) {
            ret = FAIL;
            sprintf(mess,
                    "Could not get a single adc configuration. Different "
                    "values for "
                    "selected adc (%d) and chip (%d) range.\n",
                    chipIndex, adcIndex);
            LOG(logERROR, (mess));
        }
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_adc_config(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[3] = {-1, -1, -1};

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1,
        ("Setting adc configuration [chipIndex:%d, adcIndex:%d, value:0x%x]\n",
         args[0], args[1], args[2]));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        int chipIndex = args[0];
        int adcIndex = args[1];
        int value = args[2];
        if (chipIndex < -1 || chipIndex >= NCHIP) {
            ret = FAIL;
            sprintf(mess,
                    "Could not get adc configuration. Invalid chip index %d. "
                    "Options: [-1 to %d]\n",
                    chipIndex, NCHIP);
            LOG(logERROR, (mess));
        } else if (adcIndex < -1 || adcIndex >= NADC) {
            ret = FAIL;
            sprintf(mess,
                    "Could not get adc configuration. Invalid adc index %d. "
                    "Options: [-1 to %d]\n",
                    adcIndex, NADC);
            LOG(logERROR, (mess));
        } else if (value < 0 || value > ASIC_ADC_MAX_VAL) {
            ret = FAIL;
            sprintf(mess,
                    "Could not get adc configuration. Invalid value 0x%x. "
                    "Options: [0 to 0x%x]\n",
                    value, ASIC_ADC_MAX_VAL);
            LOG(logERROR, (mess));
        } else {
            ret = setADCConfiguration(chipIndex, adcIndex, value);
            if (ret == FAIL) {
                sprintf(mess,
                        "Could not set adc configuration in chip (chipIndex: "
                        "%d, adcIndex: %d, value:0x%x).\n",
                        chipIndex, adcIndex, value);
                LOG(logERROR, (mess));
            } else {
                int retval = getADCConfiguration(chipIndex, adcIndex);
                LOG(logDEBUG1, ("adc config retval: %u\n", retval));
                validate(&ret, mess, value, retval, "configure adc", HEX);
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_bad_channels(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int nretvals = 0;
    int *retvals = NULL;

    LOG(logDEBUG1, ("Getting bad channels\n"));

#if !defined(GOTTHARD2D) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // get only
    retvals = getBadChannels(&nretvals);
    if (nretvals == -1) {
        ret = FAIL;
        strcpy(mess, "Could not get bad channels. Memory allcoation error\n");
        LOG(logERROR, (mess));
    }
#endif
    Server_SendResult(file_des, INT32, NULL, 0);
    if (ret != FAIL) {
        sendData(file_des, &nretvals, sizeof(nretvals), INT32);
        if (nretvals > 0) {
            sendData(file_des, retvals, sizeof(int) * nretvals, INT32);
        }
    }
    if (retvals != NULL) {
        free(retvals);
    }
    return ret;
}

int set_bad_channels(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int nargs = 0;
    int *args = NULL;

    if (receiveData(file_des, &nargs, sizeof(nargs), INT32) < 0)
        return printSocketReadError();

    if (nargs > 0) {
        args = malloc(nargs * sizeof(int));
        if (receiveData(file_des, args, nargs * sizeof(int), INT32) < 0)
            return printSocketReadError();
    }

    LOG(logDEBUG1, ("Setting %d bad channels\n", nargs));

#if !defined(GOTTHARD2D) && !defined(MYTHEN3D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        // validate bad channel number
        int maxChannel = NCHAN * NCHIP;
#ifdef MYTHEN3D
        maxChannel = NCHAN_1_COUNTER * NCHIP;
#endif
        for (int i = 0; i < nargs; ++i) {
            LOG(logDEBUG1, ("\t[%d]:%d\n", i, args[i]));
            if (args[i] < 0 || args[i] >= maxChannel) {
                ret = FAIL;
                sprintf(mess,
                        "Could not set bad channels. Invalid bad channel "
                        "number %d. Options [0-%d]\n",
                        args[i], maxChannel - 1);
                LOG(logERROR, (mess));
                break;
            }
        }
        if (ret == OK) {
            ret = setBadChannels(nargs, args);
            if (ret == FAIL) {
                strcpy(mess, "Could not set bad channels.\n");
                LOG(logERROR, (mess));
            } else {
                int nretvals = 0;
                int *retvals = getBadChannels(&nretvals);
                if (nretvals == -1) {
                    ret = FAIL;
                    strcpy(mess, "Could not get bad channels. Memory "
                                 "allcoation error\n");
                    LOG(logERROR, (mess));
                } else if (nretvals != nargs) {
                    ret = FAIL;
                    sprintf(mess,
                            "Could not set bad channels. Set %d channels, but "
                            "read %d "
                            "channels\n",
                            nargs, nretvals);
                    LOG(logERROR, (mess));
                }
                if (retvals != NULL) {
                    free(retvals);
                }
            }
        }
    }
    if (args != NULL) {
        free(args);
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int reconfigure_udp(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

    if (Server_VerifyLock() == OK) {
        LOG(logINFO, ("Reconfiguring UDP\n"));
        if (check_detector_idle("configure mac") == OK) {
            configure_mac();
            if (configured == FAIL) {
                ret = FAIL;
                strcpy(mess, "Invalid UDP Configuration because ");
                strcat(mess, configureMessage);
                LOG(logERROR, (mess));
            }
        }
    }
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int validate_udp_configuration(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

    LOG(logINFO, ("Validating UDP Configuration\n"));
    if (configured == FAIL) {
        ret = FAIL;
        strcpy(mess, "Invalid UDP Configuration because ");
        strcat(mess, configureMessage);
        LOG(logERROR, (mess));
    }

    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_bursts_left(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // get only
    retval = getNumBurstsLeft();
    LOG(logDEBUG1, ("retval num bursts left %lld\n", (long long int)retval));
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int start_readout(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
#ifndef MYTHEN3D
    functionNotImplemented();
#else
    if (Server_VerifyLock() == OK) {
        enum runStatus s = getRunStatus();
        if (s == RUNNING || s == WAITING) {
            ret = FAIL;
            strcpy(mess, "Could not start readout because the detector is "
                         "already running!\n");
            LOG(logERROR, (mess));
        } else if (configured == FAIL) {
            ret = FAIL;
            strcpy(mess, "Could not start readout because ");
            strcat(mess, configureMessage);
            LOG(logERROR, (mess));
        } else {
            memset(scanErrMessage, 0, MAX_STR_LENGTH);
            sharedMemory_setScanStop(0);
            sharedMemory_setScanStatus(IDLE); // if it was error
            // start readout
            ret = startReadOut();
            if (ret == FAIL) {
#ifdef VIRTUAL
                sprintf(mess,
                        "Could not start readout. Could not create udp "
                        "socket in server. Check udp_dstip & udp_dstport.\n");
#else
                sprintf(mess, "Could not start readout\n");
#endif
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int reset_to_default_dacs(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Resetting dacs to defaults (hard reset: %d)\n", arg));

#ifdef CHIPTESTBOARDD
    functionNotImplemented();
#else
    if (Server_VerifyLock() == OK) {
        if (resetToDefaultDacs(arg) == FAIL) {
            ret = FAIL;
            sprintf(mess, "Could not %s reset default dacs",
                    (arg == 1 ? "hard" : ""));
            LOG(logERROR, (mess));
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int is_virtual(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = 0;
#ifdef VIRTUAL
    retval = 1;
#endif
    LOG(logDEBUG1, ("is virtual retval: %d\n", retval));
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int load_default_pattern(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

#if !defined(MYTHEN3D)
    functionNotImplemented();
#else
    if (Server_VerifyLock() == OK) {
        ret = loadPatternFile(DEFAULT_PATTERN_FILE, mess);
        if (ret == FAIL) {
            LOG(logERROR, (mess));
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_all_threshold_energy(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retvals[3] = {-1, -1, -1};

    LOG(logDEBUG1, ("Getting all threshold energy\n"));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    for (int i = 0; i < NCOUNTERS; ++i) {
        retvals[i] = getThresholdEnergy(i);
        LOG(logDEBUG, ("eV[%d]: %deV\n", i, retvals[i]));
    }
#endif
    return Server_SendResult(file_des, INT32, retvals, sizeof(retvals));
}

int get_master(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting master\n"));

#if !defined(MYTHEN3D) && !defined(EIGERD) && !defined(GOTTHARDD) &&           \
    !defined(GOTTHARD2D) && !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    ret = isMaster(&retval);
    if (ret == FAIL) {
        strcpy(mess, "Could not get master\n");
        LOG(logERROR, (mess));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_master(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting master: %u\n", (int)arg));

#if !defined(EIGERD) && !defined(GOTTHARD2D) && !defined(JUNGFRAUD) &&         \
    !defined(MOENCHD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if ((check_detector_idle("set master") == OK) &&
            (arg != 0 && arg != 1)) {
            ret = FAIL;
            sprintf(mess, "Could not set master. Invalid argument %d.\n", arg);
            LOG(logERROR, (mess));
        } else {
            ret = setMaster(arg == 1 ? OW_MASTER : OW_SLAVE);
            if (ret == FAIL) {
                strcpy(mess, "Could not set master\n");
                LOG(logERROR, (mess));
            } else {
                int retval = 0;
                ret = isMaster(&retval);
                if (ret == FAIL) {
                    strcpy(mess, "Could not get master\n");
                    LOG(logERROR, (mess));
                } else {
                    LOG(logDEBUG1, ("master retval: %u\n", retval));
                    validate(&ret, mess, arg, retval, "set master", DEC);
                }
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_csr(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting csr\n"));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    retval = getChipStatusRegister();
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_gain_caps(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting gain caps to: %u\n", arg));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    if (Server_VerifyLock() == OK) {
        ret = setGainCaps(arg);
        if (ret == FAIL) {
            strcpy(mess, "Could not set gain caps.\n");
            LOG(logERROR, (mess));
        } else {
            int retval = getGainCaps();
            validate(&ret, mess, (int)arg, (int)retval, "set gain caps", DEC);
            LOG(logDEBUG1, ("gain caps retval: %u\n", retval));
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_gain_caps(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;
    LOG(logDEBUG1, ("Getting gain caps\n"));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    retval = getGainCaps();
    LOG(logDEBUG1, ("Gain caps: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int get_datastream(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum portPosition arg = LEFT;
    int retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Getting data stream enable [port:%d]\n", arg));

#ifndef EIGERD
    functionNotImplemented();
#else
    // get only
    if (arg != LEFT && arg != RIGHT) {
        ret = FAIL;
        sprintf(mess,
                "Could not get data stream enable. Invalid port position %d. "
                "Only left and right allowed\n",
                arg);
        LOG(logERROR, (mess));
    } else {
        ret = getDataStream(arg, &retval);
        LOG(logDEBUG1, ("datastream (%s) retval: %u\n",
                        (arg == LEFT ? "left" : "right"), retval));
        if (ret == FAIL) {
            sprintf(mess, "Could not get %s data stream enable.\n",
                    (arg == LEFT ? "left" : "right"));
            LOG(logERROR, (mess));
        }
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_datastream(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[2] = {-1, -1};

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting data stream enable [left:%d, enable:%d]\n",
                    args[0], args[1]));

#ifndef EIGERD
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        enum portPosition port = args[0];
        int enable = args[1];
        char msg[256];
        memset(msg, 0, sizeof(msg));
        sprintf(msg, "%s %s fpga datastream", (enable ? "enable" : "disable"),
                (port == LEFT ? "left" : "right"));
        if (port != LEFT && port != RIGHT) {
            ret = FAIL;
            sprintf(mess,
                    "Could not %s. Invalid port position %d. Only left and "
                    "right allowed\n",
                    msg, port);
            LOG(logERROR, (mess));
        } else if (enable != 0 && enable != 1) {
            ret = FAIL;
            sprintf(mess, "Could not %s. Invalid enable %d. \n", msg, enable);
            LOG(logERROR, (mess));
        } else {
            ret = setDataStream(port, enable);
            if (ret == FAIL) {
                sprintf(mess, "Could not %s\n", msg);
                LOG(logERROR, (mess));
            } else {
                int retval = -1;
                ret = getDataStream(port, &retval);
                LOG(logDEBUG1, ("%s retval: %u\n", msg, retval));
                if (ret == FAIL) {
                    sprintf(mess, "Could not get %s data stream enable.\n",
                            (port == LEFT ? "left" : "right"));
                    LOG(logERROR, (mess));
                }
                validate(&ret, mess, enable, retval, msg, DEC);
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_veto_stream(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum streamingInterface retval = NONE;

    LOG(logDEBUG1, ("Getting veto stream\n"));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // get only
    retval = getVetoStream();
    LOG(logDEBUG1, ("vetostream retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_veto_stream(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum streamingInterface arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting vetostream: %u\n", (int)arg));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {

        if (arg != 0 && arg != 1) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set vetostream 3GbE. Invalid argument %d.\n",
                    arg);
            LOG(logERROR, (mess));
        } else {
            setVetoStream(arg);
            int retval = getVetoStream();
            LOG(logDEBUG1, ("vetostream retval: %u\n", retval));
            validate(&ret, mess, arg, retval, "set veto stream", DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_veto_algorithm(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum streamingInterface arg = NONE;
    enum vetoAlgorithm retval = ALG_HITS;
    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();

    LOG(logDEBUG1, ("Getting veto algorithm for interface %d\n", arg));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // get only
    if (arg != LOW_LATENCY_LINK && arg != ETHERNET_10GB) {
        ret = FAIL;
        sprintf(mess, "Could not get vetoalgorithm. Invalid interface %d.\n",
                arg);
        LOG(logERROR, (mess));
    } else {
        retval = getVetoAlgorithm(arg);
        LOG(logDEBUG1, ("vetoalgorithm retval: %u\n", retval));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_veto_algorithm(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[2] = {-1, -1};
    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();

    enum vetoAlgorithm alg = args[0];
    enum streamingInterface interface = args[1];
    LOG(logDEBUG1, ("Setting vetoalgorithm (interface: %d): %u\n",
                    (int)interface, (int)alg));

#ifndef GOTTHARD2D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (interface != LOW_LATENCY_LINK && interface != ETHERNET_10GB) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set vetoalgorithm. Invalid interface %d.\n",
                    interface);
            LOG(logERROR, (mess));
        } else {
            switch (alg) {
            case ALG_HITS:
            case ALG_RAW:
                break;
            default:
                modeNotImplemented("Veto Algorithm index", (int)alg);
                break;
            }
        }
        if (ret == OK) {
            setVetoAlgorithm(alg, interface);
            int retval = getVetoAlgorithm(interface);
            LOG(logDEBUG1, ("vetoalgorithm retval: %u\n", retval));
            validate(&ret, mess, alg, retval, "set veto algorithm", DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_chip_version(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;
#if !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    retval = getChipVersion();
#endif
    LOG(logDEBUG1, ("chip version retval: %d\n", retval));
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int get_default_dac(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[2] = {-1, -1};
    int retval = -1;
    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();

    enum dacIndex dacindex = args[0];
    enum detectorSettings sett = args[1];
    LOG(logDEBUG1,
        ("Getting default dac [dacindex:%d, settings: %d]\n", dacindex, sett));

#ifdef CHIPTESTBOARDD
    functionNotImplemented();
#else
    // get only
    enum DACINDEX idac = getDACIndex(dacindex);
    if (ret == OK) {
        // to allow for default dacs (without settings)
        if (sett != UNDEFINED) {
            validate_settings(sett);
        }
        if (ret == OK) {
            ret = getDefaultDac(idac, sett, &retval);
            if (ret == FAIL) {
                sprintf(mess, "Could not get default dac %d %s\n", (int)idac,
                        (sett != UNDEFINED ? "for this setting" : ""));
                LOG(logERROR, (mess));
            } else {
                LOG(logDEBUG1,
                    ("default dac retval [dacindex:%d, setting:%d]: %u\n",
                     (int)dacindex, (int)sett, retval));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_default_dac(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int args[3] = {-1, -1, -1};
    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();

    enum dacIndex dacindex = args[0];
    enum detectorSettings sett = args[1];
    int value = args[2];
    LOG(logDEBUG1, ("Setting default dac [dacindex: %d, settings: %d] to %d\n",
                    (int)dacindex, (int)sett, value));

#ifdef CHIPTESTBOARDD
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        enum DACINDEX idac = getDACIndex(dacindex);
        if (ret == OK) {
            // to allow for default dacs (without settings)
            if (sett != UNDEFINED) {
                validate_settings(sett);
            }
            if (ret == OK) {
                ret = setDefaultDac(idac, sett, value);
                if (ret == FAIL) {
                    sprintf(mess, "Could not set default dac %d %s\n",
                            (int)idac,
                            (sett != UNDEFINED ? "for this setting" : ""));
                    LOG(logERROR, (mess));
                } else {
                    int retval = -1;
                    ret = getDefaultDac(idac, sett, &retval);
                    if (ret == FAIL) {
                        sprintf(mess, "Could not get default dac %d %s\n",
                                (int)idac,
                                (sett != UNDEFINED ? "for this setting" : ""));
                        LOG(logERROR, (mess));
                    } else {
                        LOG(logDEBUG1, ("default dac retval [dacindex:%d, "
                                        "setting:%d]: %u\n",
                                        (int)dacindex, (int)sett, retval));
                    }
                }
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_gain_mode(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum gainMode retval = DYNAMIC;
    LOG(logDEBUG1, ("Getting gain mode\n"));

#if !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // get only
    retval = getGainMode();
    LOG(logDEBUG1, ("gainmode retval: %u\n", retval));
    if ((int)retval == -1) {
        ret = FAIL;
        strcpy(mess, "Could not get gain mode.\n");
        LOG(logERROR, (mess));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_gain_mode(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;
    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    enum gainMode gainmode = arg;
    LOG(logDEBUG1, ("Setting gain mode %d\n", (int)gainmode));

#if !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        switch (gainmode) {
        case DYNAMIC:
        case FORCE_SWITCH_G1:
        case FORCE_SWITCH_G2:
        case FIX_G1:
        case FIX_G2:
        case FIX_G0:
            break;
        default:
            modeNotImplemented("Gain Mode Index", (int)gainmode);
            break;
        }

        setGainMode(gainmode);
        int retval = getGainMode();
        LOG(logDEBUG1, ("gainmode retval: %u\n", retval));
        if (retval == -1) {
            ret = FAIL;
            strcpy(mess, "Could not get gain mode.\n");
            LOG(logERROR, (mess));
        }
        validate(&ret, mess, arg, retval, "set gain mode", DEC);
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_comp_disable_time(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t retval = -1;
#if !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // get only
    if (getChipVersion() != 11) {
        ret = FAIL;
        strcpy(mess,
               "Cannot get comparator disable time. Only valid for chipv1.1\n");
        LOG(logERROR, (mess));
    } else {
        retval = getComparatorDisableTime();
        LOG(logDEBUG1,
            ("retval comp disable time %lld ns\n", (long long int)retval));
    }
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}

int set_comp_disable_time(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int64_t arg = -1;
    if (receiveData(file_des, &arg, sizeof(arg), INT64) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting comp disable time %lld ns\n", (long long int)arg));

#if !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (getChipVersion() != 11) {
            ret = FAIL;
            strcpy(mess, "Cannot get comparator disable time. Only valid for "
                         "chipv1.1\n");
            LOG(logERROR, (mess));
        } else {
            ret = setComparatorDisableTime(arg);
            int64_t retval = getComparatorDisableTime();
            LOG(logDEBUG1, ("retval get comp disable time %lld ns\n",
                            (long long int)retval));
            if (ret == FAIL) {
                sprintf(mess,
                        "Could not set comp disable time. Set %lld ns, read "
                        "%lld ns.\n",
                        (long long int)arg, (long long int)retval);
                LOG(logERROR, (mess));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT64, NULL, 0);
}

int get_flip_rows(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting flip rows\n"));

#if !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    // get only
    if (isHardwareVersion_1_0()) {
        ret = FAIL;
        strcpy(mess, "Could not get flip rows. Only available for "
                     "Hardware Board version 2.0.\n");
        LOG(logERROR, (mess));
    } else {
        retval = getFlipRows();
        LOG(logDEBUG1, ("flip rows retval: %u\n", retval));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_flip_rows(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting flip rows: %u\n", (int)arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if ((check_detector_idle("set flip rows") == OK) &&
            (arg != 0 && arg != 1)) {
            ret = FAIL;
            sprintf(mess, "Could not set flip rows. Invalid argument %d.\n",
                    arg);
            LOG(logERROR, (mess));
        } else if (isHardwareVersion_1_0()) {
            ret = FAIL;
            strcpy(mess, "Could not set flip rows. Only available for "
                         "Hardware Board version 2.0.\n");
            LOG(logERROR, (mess));
        } else if (getNumberofUDPInterfaces() == 1) {
            ret = FAIL;
            strcpy(mess, "Could not set flip rows. Number of udp "
                         "interfaces is still 1.\n");
            LOG(logERROR, (mess));
        } else {
            setFlipRows(arg);
            int retval = getFlipRows();
            LOG(logDEBUG1, ("flip rows retval: %u\n", retval));
            validate(&ret, mess, arg, retval, "set flip rows", DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_num_filter_cells(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting number of filter cellsn"));

#if !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // get only
    // only for chipv1.1
    if (getChipVersion() == 10) {
        ret = FAIL;
        strcpy(mess, "Could not get number of filter cells. Only available for "
                     "chip version 1.1\n");
        LOG(logERROR, (mess));
    } else {
        retval = getNumberOfFilterCells();
        LOG(logDEBUG1, ("num filter cells retval: %u\n", retval));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_num_filter_cells(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting number of filter cells: %u\n", (int)arg));

#if !defined(JUNGFRAUD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {

        if (arg < 0 || arg > MAX_FILTER_CELL_VAL) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set number of filter cells. Invalid argument "
                    "%d. Options: "
                    "0 - %d\n",
                    arg, MAX_FILTER_CELL_VAL);
            LOG(logERROR, (mess));
        }
        // only for chipv1.1
        else if (getChipVersion() == 10) {
            ret = FAIL;
            strcpy(mess,
                   "Could not set number of filter cells. Only available for "
                   "chip version 1.1\n");
            LOG(logERROR, (mess));
        } else {
            setNumberOfFilterCells(arg);
            // no validation as it might take time to update status register if
            // acquiring
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int set_adc_pipeline(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting adc pipeline : %u\n", arg));

#if !defined(CHIPTESTBOARDD)
    functionNotImplemented();
#else

    // only set
    if (Server_VerifyLock() == OK) {
        setADCPipeline(arg);
        int retval = getADCPipeline();
        LOG(logDEBUG1, ("retval adc pipeline: %d\n", retval));
        validate(&ret, mess, arg, retval, "set adc pipeline", DEC);
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_adc_pipeline(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting adc pipeline\n"));

#if !defined(CHIPTESTBOARDD)
    functionNotImplemented();
#else
    // get only
    retval = getADCPipeline();
    LOG(logDEBUG1, ("retval adc pipeline: %d\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_dbit_pipeline(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting dbit pipeline : %u\n", arg));

#if !defined(CHIPTESTBOARDD) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else

    // only set
    if (Server_VerifyLock() == OK) {
        setDBITPipeline(arg);
        int retval = getDBITPipeline();
        LOG(logDEBUG1, ("retval dbit pipeline: %d\n", retval));
        validate(&ret, mess, arg, retval, "set dbit pipeline", DEC);
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_dbit_pipeline(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;
    LOG(logDEBUG1, ("Getting dbit pipeline\n"));

#if !defined(CHIPTESTBOARDD) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // get only
    retval = getDBITPipeline();
    LOG(logDEBUG1, ("retval dbit pipeline: %d\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int get_module_id(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;
#if !(defined(GOTTHARD2D) || defined(EIGERD) || defined(MYTHEN3D) ||           \
      defined(JUNGFRAUD) || defined(MOENCHD))
    functionNotImplemented();
#else
    retval = getModuleId(&ret, mess);
    LOG(logDEBUG1, ("module id retval: 0x%x\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int get_dest_udp_list(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t arg = 0;
    uint32_t retvals[5] = {};
    uint64_t retvals64[2] = {};

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Getting udp destination list for entry %d\n", arg));

#if !defined(EIGERD) && !defined(JUNGFRAUD) && !defined(MOENCHD) &&            \
    !defined(MYTHEN3D) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    if (arg >= MAX_UDP_DESTINATION) {
        ret = FAIL;
        sprintf(
            mess,
            "Could not set udp destination. Invalid entry. Options: 0 - %d\n",
            MAX_UDP_DESTINATION - 1);
        LOG(logERROR, (mess));
    } else {
        retvals[0] = arg;
        retvals[1] = udpDetails[arg].dstport;
        retvals[2] = udpDetails[arg].dstport2;
        retvals[3] = udpDetails[arg].dstip;
        retvals[4] = udpDetails[arg].dstip2;
        retvals64[0] = udpDetails[arg].dstmac;
        retvals64[1] = udpDetails[arg].dstmac2;

        // swap ip
        retvals[3] = __builtin_bswap32(retvals[3]);
        retvals[4] = __builtin_bswap32(retvals[4]);

        // convert to string
        char ip[INET_ADDRSTRLEN], ip2[INET_ADDRSTRLEN];
        getIpAddressinString(ip, retvals[3]);
        getIpAddressinString(ip2, retvals[4]);
        char mac[MAC_ADDRESS_SIZE], mac2[MAC_ADDRESS_SIZE];
        getMacAddressinString(mac, MAC_ADDRESS_SIZE, retvals64[0]);
        getMacAddressinString(mac2, MAC_ADDRESS_SIZE, retvals64[1]);
        LOG(logDEBUG1,
            ("Udp Dest. retval [%d]: [port %d, port2 %d, ip %s, ip2 %s, "
             "mac %s, mac2 %s]\n",
             retvals[0], retvals[1], retvals[2], ip, ip2, mac, mac2));
    }
#endif
    Server_SendResult(file_des, INT32, NULL, 0);
    if (ret != FAIL) {
        sendData(file_des, retvals, sizeof(retvals), INT32);
        sendData(file_des, retvals64, sizeof(retvals64), INT64);
    }
    return ret;
}

int set_dest_udp_list(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    uint32_t args[5] = {};
    uint64_t args64[2] = {};

    if (receiveData(file_des, args, sizeof(args), INT32) < 0)
        return printSocketReadError();
    if (receiveData(file_des, args64, sizeof(args64), INT64) < 0)
        return printSocketReadError();

    // swap ip
    args[3] = __builtin_bswap32(args[3]);
    args[4] = __builtin_bswap32(args[4]);

    // convert to string
    char ip[INET_ADDRSTRLEN], ip2[INET_ADDRSTRLEN];
    getIpAddressinString(ip, args[3]);
    getIpAddressinString(ip2, args[4]);
    char mac[MAC_ADDRESS_SIZE], mac2[MAC_ADDRESS_SIZE];
    getMacAddressinString(mac, MAC_ADDRESS_SIZE, args64[0]);
    getMacAddressinString(mac2, MAC_ADDRESS_SIZE, args64[1]);

#if !defined(EIGERD) && !defined(JUNGFRAUD) && !defined(MOENCHD) &&            \
    !defined(MYTHEN3D) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        int entry = args[0];
        LOG(logINFOBLUE,
            ("Setting udp dest. [%d]: [port %d, port2 %d, ip %s, ip2 %s, "
             "mac %s, mac2 %s]\n",
             entry, args[1], args[2], ip, ip2, mac, mac2));

        if (entry < 1 || entry >= MAX_UDP_DESTINATION) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set udp destination. Invalid entry. Options: 1 "
                    "- %d\n",
                    MAX_UDP_DESTINATION - 1);
            LOG(logERROR, (mess));
        }
#if defined(EIGERD) || defined(MYTHEN3D)
        else if (args[4] != 0 || args64[1] != 0) {
            ret = FAIL;
            strcpy(mess, "Could not set udp destination. ip2 and mac2 not "
                         "implemented for this detector.\n");
            LOG(logERROR, (mess));
        }
#endif
        else {
            if (check_detector_idle("set udp destination list entries") == OK) {
                if (args[1] != 0) {
                    udpDetails[entry].dstport = args[1];
                }
                if (args[2] != 0) {
                    udpDetails[entry].dstport2 = args[2];
                }
                if (args[3] != 0) {
                    udpDetails[entry].dstip = args[3];
                }
                if (args[4] != 0) {
                    udpDetails[entry].dstip2 = args[4];
                }
                if (args64[0] != 0) {
                    udpDetails[entry].dstmac = args64[0];
                }
                if (args64[1] != 0) {
                    udpDetails[entry].dstmac2 = args64[1];
                }

                // if still 0, set defaults
                int twoInterfaces = 0;
#if defined(JUNGFRAUD) || defined(MOENCHD) || defined(GOTTHARD2D)
                twoInterfaces = getNumberofUDPInterfaces() == 2 ? 1 : 0;
#endif
                udpDetails[entry].srcport = DEFAULT_UDP_SRC_PORTNO;
                if (udpDetails[entry].dstport == 0) {
                    udpDetails[entry].dstport =
                        2 * entry + DEFAULT_UDP_DST_PORTNO;
                }
                if (myDetectorType == EIGER || twoInterfaces) {
                    udpDetails[entry].srcport2 = DEFAULT_UDP_SRC_PORTNO + 1;
                    if (udpDetails[entry].dstport2 == 0) {
                        udpDetails[entry].dstport2 =
                            2 * entry + 1 + DEFAULT_UDP_DST_PORTNO;
                    }
                }
                // if still 0, copy from entry 0
                if (entry != 0) {
                    udpDetails[entry].srcip = udpDetails[0].srcip;
                    udpDetails[entry].srcmac = udpDetails[0].srcmac;
                    if (udpDetails[entry].dstip == 0) {
                        udpDetails[entry].dstip = udpDetails[0].dstip;
                    }
                    if (udpDetails[entry].dstmac == 0) {
                        udpDetails[entry].dstmac = udpDetails[0].dstmac;
                    }
                    if (twoInterfaces) {
                        udpDetails[entry].srcip2 = udpDetails[0].srcip2;
                        udpDetails[entry].srcmac2 = udpDetails[0].srcmac2;
                        if (udpDetails[entry].dstip2 == 0) {
                            udpDetails[entry].dstip2 = udpDetails[0].dstip2;
                        }
                        if (udpDetails[entry].dstmac2 == 0) {
                            udpDetails[entry].dstmac2 = udpDetails[0].dstmac2;
                        }
                    }
                }
                // find number of destinations
                int numdest = 0;
                for (int i = MAX_UDP_DESTINATION - 1; i >= 0; --i) {
                    if (udpDetails[i].dstip != 0) {
                        numdest = i + 1;
                        break;
                    }
                }
                // atleast 1 destination
                if (numdest == 0) {
                    numdest = 1;
                }
                // set number of destinations
                if (setNumberofDestinations(numdest) == FAIL) {
                    ret = FAIL;
                    strcpy(mess, "Could not set number of udp destinations.\n");
                    LOG(logERROR, (mess));
                } else {
                    numUdpDestinations = numdest;
                    LOG(logINFOBLUE, ("Number of UDP Destinations: %d\n",
                                      numUdpDestinations));
                    configure_mac();
                }
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_num_dest_list(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(EIGERD) &&            \
    !defined(MYTHEN3D) && !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    retval = numUdpDestinations;
    LOG(logDEBUG1, ("numUdpDestinations retval: 0x%x\n", retval));
    int retval1 = 0;
    if (getNumberofDestinations(&retval1) == FAIL || retval1 != retval) {
        ret = FAIL;
        sprintf(mess,
                "Could not get number of udp destinations. (server reads %d, "
                "fpga reads %d).\n",
                retval1, retval);
        LOG(logERROR, (mess));
    }

#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int clear_all_udp_dst(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));

    LOG(logINFO, ("Clearing all udp destinations\n"));
    if (Server_VerifyLock() == OK) {
        if (check_detector_idle("clear all udp destinations") == OK) {
            memset(udpDetails, 0, sizeof(udpDetails));
            // minimum 1 destination in fpga
            int numdest = 1;
            // set number of destinations
#if defined(JUNGFRAUD) || defined(MOENCHD) || defined(EIGERD) ||               \
    defined(MYTHEN3D) || defined(GOTTHARD2D)
            if (setNumberofDestinations(numdest) == FAIL) {
                ret = FAIL;
                strcpy(mess, "Could not clear udp destinations to 1 entry.\n");
                LOG(logERROR, (mess));
            } else
#endif
            {
                numUdpDestinations = numdest;
                LOG(logINFOBLUE,
                    ("Number of UDP Destinations: %d\n", numUdpDestinations));
                ret = configureMAC();
                if (ret == FAIL) {
                    strcpy(mess,
                           "Could not clear all destinations in the fpga.\n");
                    LOG(logERROR, (mess));
                }
            }
        }
    }
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_udp_first_dest(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;
#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(MYTHEN3D) &&          \
    !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    retval = getFirstUDPDestination();
    LOG(logDEBUG1, ("first udp destination retval: 0x%x\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_udp_first_dest(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting first udp destination to %d\n", arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(MYTHEN3D) &&          \
    !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (arg < 0 || arg >= numUdpDestinations) {
            ret = FAIL;
            sprintf(mess, "Could not set first destination. Options: 0-%d\n",
                    numUdpDestinations - 1);
            LOG(logERROR, (mess));
        } else {
            if (check_detector_idle("set first udp destination") == OK) {
                setFirstUDPDestination(arg);
                int retval = getFirstUDPDestination();
                validate(&ret, mess, arg, retval, "set udp first destination",
                         DEC);
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_readout_speed(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;
    LOG(logDEBUG1, ("Getting readout speed\n"));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(EIGERD) &&            \
    !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // get only
    ret = getReadoutSpeed(&retval);
    LOG(logDEBUG1, ("retval readout speed: %d\n", retval));
    if (ret == FAIL) {
        strcpy(mess, "Could not get readout speed\n");
        LOG(logERROR, (mess));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_readout_speed(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting readout speed : %u\n", arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(EIGERD) &&            \
    !defined(GOTTHARD2D)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
#if defined(JUNGFRAUD) || defined(MOENCHD)
        if (arg == (int)FULL_SPEED && isHardwareVersion_1_0()) {
            ret = FAIL;
            strcpy(
                mess,
                "Full speed not implemented for this board version (v1.0).\n");
            LOG(logERROR, (mess));
        }
#endif
        if (ret == OK) {
            switch (arg) {
#if defined(EIGERD) || defined(JUNGFRAUD) || defined(MOENCHD)
            case FULL_SPEED:
#ifndef MOENCHD
            case HALF_SPEED:
            case QUARTER_SPEED:
#endif
#elif GOTTHARD2D
            case G2_108MHZ:
            case G2_144MHZ:
#endif
                break;
            default:
                modeNotImplemented("readout speed index", arg);
                break;
            }
            if (ret == OK) {
                ret = setReadoutSpeed(arg);
                if (ret == FAIL) {
                    sprintf(mess, "Could not set readout speed to %d.\n", arg);
                    LOG(logERROR, (mess));
                } else {
                    int retval = 0;
                    ret = getReadoutSpeed(&retval);
                    LOG(logDEBUG1, ("retval readout speed: %d\n", retval));
                    if (ret == FAIL) {
                        strcpy(mess, "Could not get readout speed\n");
                        LOG(logERROR, (mess));
                    }
                    validate(&ret, mess, arg, retval, "set readout speed", DEC);
                }
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_kernel_version(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    char retvals[MAX_STR_LENGTH];
    memset(retvals, 0, MAX_STR_LENGTH);

    LOG(logDEBUG1, ("Getting kernel version\n"));

    // get only
    ret = getKernelVersion(retvals);
    if (ret == FAIL) {
        if (snprintf(mess, MAX_STR_LENGTH, "Could not get kernel version. %s\n",
                     retvals) >= MAX_STR_LENGTH) {
            strcpy(mess,
                   "Could not get kernel version. Reason too long to copy\n");
        }
        LOG(logERROR, (mess));
    } else {
        LOG(logDEBUG1, ("kernel version: [%s]\n", retvals));
    }
    return Server_SendResult(file_des, OTHER, retvals, sizeof(retvals));
}

int update_kernel(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
#ifdef EIGERD
    functionNotImplemented();
    return Server_SendResult(file_des, INT32, NULL, 0);
#else
    receive_program(file_des, PROGRAM_KERNEL);
#endif
    return ret;
}

int update_detector_server(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    return receive_program(file_des, PROGRAM_SERVER);
}

int receive_program(int file_des, enum PROGRAM_INDEX index) {
    // only set
    if (Server_VerifyLock() == OK) {
        char functionType[SHORT_STR_LENGTH] = {0};
        switch (index) {
        case PROGRAM_FPGA:
            strcpy(functionType, "Update Firmware");
            break;
        case PROGRAM_KERNEL:
            strcpy(functionType, "Update Kernel");
            break;
        case PROGRAM_SERVER:
            strcpy(functionType, "Update Server");
            break;
        }
        LOG(logINFOBLUE, ("%s ...\n", functionType));

        // filesize
        uint64_t filesize = 0;
        if (receiveData(file_des, &filesize, sizeof(filesize), INT64) < 0)
            return printSocketReadError();
        LOG(logINFO, ("\tProgram size: %lld\n", (long long int)filesize));

        // client checksum
        char checksum[MAX_STR_LENGTH] = {0};
        if (receiveData(file_des, checksum, MAX_STR_LENGTH, OTHER) < 0)
            return printSocketReadError();
        LOG(logINFO, ("\tChecksum: %s\n", checksum));

        // server name
        char serverName[MAX_STR_LENGTH] = {0};
        if (index == PROGRAM_SERVER) {
            if (receiveData(file_des, serverName, MAX_STR_LENGTH, OTHER) < 0)
                return printSocketReadError();
            LOG(logINFO, ("\tServer Name: %s\n", serverName));
        }

#if !defined(GOTTHARD2D) && !defined(MYTHEN3D) && !defined(EIGERD)
        int forceDeleteNormalFile = 0;
        if (receiveData(file_des, &forceDeleteNormalFile,
                        sizeof(forceDeleteNormalFile), INT32) < 0)
            return printSocketReadError();
        LOG(logINFO, ("\tForce Delete Normal File flag? %s\n",
                      (forceDeleteNormalFile ? "Y" : "N")));
#endif

        // ensure the name is not the same as the linked name
        if (!strcmp(serverName, LINKED_SERVER_NAME)) {
            ret = FAIL;
            strcpy(mess, "Server name is the same as the symbolic link. Please "
                         "use a different server name\n");
            LOG(logERROR, (mess));
        }

        // in same folder as current process (will also work for virtual then
        // with write permissions)
        {
            const int fileNameSize = 128;
            char fname[fileNameSize];
            if (getAbsPath(fname, fileNameSize, serverName) == FAIL) {
                ret = FAIL;
                sprintf(mess,
                        "Could not %s. Could not get abs path of current "
                        "process\n",
                        functionType);
                LOG(logERROR, (mess));
                Server_SendResult(file_des, INT32, NULL, 0);
            } else {
                strcpy(serverName, fname);
            }
        }

        if (ret == OK) {
#if defined(GOTTHARD2D) || defined(MYTHEN3D) || defined(EIGERD)
            receive_program_default(file_des, index, functionType, filesize,
                                    checksum, serverName);
#else
            receive_program_via_blackfin(file_des, index, functionType,
                                         filesize, checksum, serverName,
                                         forceDeleteNormalFile);
#endif
        }

        if (ret == OK) {
            LOG(logINFOGREEN, ("%s completed successfully\n", functionType));
        } else {
            LOG(logERROR, ("%s FAIL!\n", functionType));
        }
    }

    return ret;
}

void receive_program_via_blackfin(int file_des, enum PROGRAM_INDEX index,
                                  char *functionType, uint64_t filesize,
                                  char *checksum, char *serverName,
                                  int forceDeleteNormalFile) {

#if !defined(JUNGFRAUD) && !defined(MOENCHD) && !defined(CHIPTESTBOARDD) &&    \
    !defined(GOTTHARDD)
    ret = FAIL;
    sprintf(mess,
            "Could not %s. program via blackfin not implmented for this "
            "detector.\n",
            functionType);
    LOG(logERROR, (mess));
#else
    // only when writing to kernel flash or root directory
    if (index != PROGRAM_FPGA) {
        // check update is allowed  (Non Amd OR AMD + current kernel)
        ret = allowUpdate(mess, functionType);
        if (ret == FAIL) {
            Server_SendResult(file_des, INT32, NULL, 0);
            return;
        }
    }

    // open file and allocate memory for part program
    FILE *fd = NULL;
    ret = preparetoCopyProgram(mess, functionType, &fd, filesize);
    char *src = NULL;
    if (ret == OK) {
        src = malloc(MAX_BLACKFIN_PROGRAM_SIZE);
        if (src == NULL) {
            fclose(fd);
            struct sysinfo info;
            sysinfo(&info);
            sprintf(mess,
                    "Could not %s. Memory allocation failure. Free "
                    "space: %d MB\n",
                    functionType, (int)(info.freeram / (1024 * 1024)));
            LOG(logERROR, (mess));
            ret = FAIL;
        }
    }
    Server_SendResult(file_des, INT32, NULL, 0);
    if (ret == FAIL) {
        return;
    }

    // copying program part by part
    uint64_t totalsize = filesize;
    while (ret == OK && filesize) {
        uint64_t unitprogramsize = MAX_BLACKFIN_PROGRAM_SIZE;
        if (unitprogramsize > filesize)
            unitprogramsize = filesize;
        LOG(logDEBUG1, ("unit size to receive is:%lld [filesize:%lld]\n",
                        (long long unsigned int)unitprogramsize,
                        (long long unsigned int)filesize));

        // receive part of program
        if (receiveData(file_des, src, unitprogramsize, OTHER) < 0) {
            printSocketReadError();
            break;
        }
        filesize -= unitprogramsize;

        // copy program
        if (fwrite((void *)src, sizeof(char), unitprogramsize, fd) !=
            unitprogramsize) {
            ret = FAIL;
            sprintf(
                mess,
                "Could not %s. Could not copy program to /var/tmp (size:%ld)\n",
                functionType, (long int)unitprogramsize);
            LOG(logERROR, (mess));
        }
        Server_SendResult(file_des, INT32, NULL, 0);
        if (ret == FAIL) {
            break;
        }
        // print progress
        LOG(logINFO,
            ("\t%d%%\r",
             (int)(((double)(totalsize - filesize) / totalsize) * 100)));
        fflush(stdout);
    }
    free(src);
    fclose(fd);

    // checksum of copied program
    if (ret == OK) {
        ret = verifyChecksumFromFile(mess, functionType, checksum,
                                     TEMP_PROG_FILE_NAME);
    }
    Server_SendResult(file_des, INT32, NULL, 0);
    if (ret == FAIL) {
        return;
    }

    // appropriate functions
    switch (index) {
    case PROGRAM_FPGA:
    case PROGRAM_KERNEL:
        ret = eraseAndWriteToFlash(mess, index, functionType, checksum,
                                   totalsize, forceDeleteNormalFile);
        break;
    case PROGRAM_SERVER:
        // a fail here is not a show stopper (just for memory)
        deleteOldServers(mess, serverName, "update detector server");
        ret = moveBinaryFile(mess, serverName, TEMP_PROG_FILE_NAME,
                             "update detector server");
        if (ret == OK) {
            ret = setupDetectorServer(mess, serverName);
        }
        break;
    default:
        modeNotImplemented("Program index", (int)index);
        break;
    }

    // erase and copy to flash
    Server_SendResult(file_des, INT32, NULL, 0);
#endif
}

void receive_program_default(int file_des, enum PROGRAM_INDEX index,
                             char *functionType, uint64_t filesize,
                             char *checksum, char *serverName) {
#if !defined(GOTTHARD2D) && !defined(MYTHEN3D) && !defined(EIGERD)
    ret = FAIL;
    sprintf(mess,
            "Could not %s. program via blackfin not implmented for this "
            "detector.\n",
            functionType);
    LOG(logERROR, (mess))
#else
#if defined(GOTTHARD2D) || defined(MYTHEN3D)
    // validate file size
    if (filesize > NIOS_MAX_APP_IMAGE_SIZE) {
        ret = FAIL;
        sprintf(mess,
                "Could not %s. File size 0x%llx "
                "exceeds max size 0x%llx. Forgot Compression?\n",
                functionType, (long long unsigned int)filesize,
                (long long unsigned int)NIOS_MAX_APP_IMAGE_SIZE);
        LOG(logERROR, (mess));
    }
#endif

    // memory allocation
    char *src = NULL;
    if (ret == OK) {
        src = malloc(filesize);
        if (src == NULL) {
            struct sysinfo info;
            sysinfo(&info);
            sprintf(mess,
                    "Could not %s. Memory allocation failure. Free "
                    "space: %d MB\n",
                    functionType, (int)(info.freeram / (1024 * 1024)));
            LOG(logERROR, (mess));
            ret = FAIL;
        }
    }
    Server_SendResult(file_des, INT32, NULL, 0);
    if (ret == FAIL) {
        return;
    }

    // receive program
    if (receiveData(file_des, src, filesize, OTHER) < 0) {
        free(src);
        ret = printSocketReadError();
        return;
    }

    // checksum of copied program
    if (ret == OK) {
        ret = verifyChecksumFromBuffer(mess, functionType, checksum, src,
                                       filesize);
    }
    Server_SendResult(file_des, INT32, NULL, 0);
    if (ret == FAIL) {
        return;
    }

    // appropriate functions
    switch (index) {
#if defined(GOTTHARD2D) || defined(MYTHEN3D)
    case PROGRAM_FPGA:
    case PROGRAM_KERNEL:
        ret = eraseAndWriteToFlash(mess, index, functionType, checksum, src,
                                   filesize);
        break;
#endif
#if defined(GOTTHARD2D) || defined(MYTHEN3D) || defined(EIGERD)
    case PROGRAM_SERVER:
        ret = writeBinaryFile(mess, TEMP_PROG_FILE_NAME, src, filesize,
                              "update detector server");
        // extra step to write to temp and move to real file as
        // fopen will give text busy if opening same name as process name
        if (ret == OK) {
            ret = moveBinaryFile(mess, serverName, TEMP_PROG_FILE_NAME,
                                 "update detector server");
        }
        if (ret == OK) {
            ret = verifyChecksumFromFile(mess, functionType, checksum,
                                         serverName);
        }
        if (ret == OK) {
            ret = setupDetectorServer(mess, serverName);
        }
        break;
#endif
    default:
        modeNotImplemented("Program index", (int)index);
        break;
    }
    // send result
    Server_SendResult(file_des, INT32, NULL, 0);

    // free resources
    free(src);
#endif
}

int get_update_mode(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;
    LOG(logDEBUG1, ("Getting update mode\n"));

    retval = updateFlag;
    LOG(logDEBUG1, ("update mode retval: %d\n", retval));

    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_update_mode(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting update mode to \n", arg));

#ifdef BLACKFIN_DEFINED
    // check update is allowed  (Non Amd OR AMD + current kernel)
    ret = allowUpdate(mess, "set/unset update mode");
#endif

    if (ret == OK) {
        switch (arg) {
        case 0:
            ret = deleteFile(mess, UPDATE_FILE, "unset update mode");
            break;
        case 1:
            ret = createEmptyFile(mess, UPDATE_FILE, "set update mode");
            break;
        default:
            ret = FAIL;
            sprintf(mess, "Could not set updatemode. Options: 0 or 1\n");
            LOG(logERROR, (mess));
            break;
        }
    }

    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_top(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;
    LOG(logDEBUG1, ("Getting top\n"));

#ifndef EIGERD
    functionNotImplemented();
#else
    // get only
    ret = isTop(&retval);
    if (ret == FAIL) {
        strcpy(mess, "Could not get Top\n");
        LOG(logERROR, (mess));
    } else {
        LOG(logDEBUG1, ("retval top: %d\n", retval));
    }
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_top(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting top : %u\n", arg));

#ifndef EIGERD
    functionNotImplemented();
#else

    // only set
    if (Server_VerifyLock() == OK) {
        if (arg != 0 && arg != 1) {
            ret = FAIL;
            sprintf(
                mess,
                "Could not set top mode. Invalid value: %d. Must be 0 or 1\n",
                arg);
            LOG(logERROR, (mess));
        } else {
            ret = setTop(arg == 1 ? OW_TOP : OW_BOTTOM);
            if (ret == FAIL) {
                sprintf(mess, "Could not set %s\n",
                        (arg == 1 ? "Top" : "Bottom"));
                LOG(logERROR, (mess));
            } else {
                int retval = -1;
                ret = isTop(&retval);
                if (ret == FAIL) {
                    strcpy(mess, "Could not get Top mode\n");
                    LOG(logERROR, (mess));
                } else {
                    LOG(logDEBUG1, ("retval top: %d\n", retval));
                    validate(&ret, mess, arg, retval, "set top mode", DEC);
                }
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_polarity(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum polarity retval = POSITIVE;

    LOG(logDEBUG1, ("Getting negativepolarity\n"));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    // get only
    retval = getNegativePolarity() ? NEGATIVE : POSITIVE;
    LOG(logDEBUG1, ("negative polarity retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_polarity(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum polarity arg = POSITIVE;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting negative polarity: %u\n", (int)arg));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        switch (arg) {
        case POSITIVE:
            ret = setNegativePolarity(0);
            break;
        case NEGATIVE:
            ret = setNegativePolarity(1);
            break;
        default:
            modeNotImplemented("Polarity index", (int)arg);
            break;
        }
        if (ret == FAIL) {
            sprintf(mess, "Could not set polarity\n");
            LOG(logERROR, (mess));
        } else {
            enum polarity retval = getNegativePolarity() ? NEGATIVE : POSITIVE;
            validate(&ret, mess, (int)arg, (int)retval, "set polarity", DEC);
            LOG(logDEBUG1, ("negative polarity retval: %u\n", retval));
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_interpolation(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting interpolation\n"));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    // get only
    retval = getInterpolation();
    LOG(logDEBUG1, ("interpolation retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_interpolation(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting interpolation: %u\n", arg));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (getPumpProbe() && arg) {
            ret = FAIL;
            sprintf(mess, "Could not set interpolation. Disable pump probe "
                          "mode first.\n");
            LOG(logERROR, (mess));
        } else {
            ret = setInterpolation(arg);
            if (ret == FAIL) {
                if (arg)
                    sprintf(mess, "Could not set interpolation or enable all "
                                  "counters for it.\n");
                else
                    sprintf(mess, "Could not set interpolation\n");
                LOG(logERROR, (mess));
            } else {
                int retval = getInterpolation();
                validate(&ret, mess, (int)arg, (int)retval, "set interpolation",
                         DEC);
                LOG(logDEBUG1, ("interpolation retval: %u\n", retval));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_pump_probe(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting pump probe\n"));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    // get only
    retval = getPumpProbe();
    LOG(logDEBUG1, ("pump probe retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_pump_probe(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting pump probe: %u\n", arg));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if (getInterpolation() && arg) {
            ret = FAIL;
            sprintf(mess, "Could not set pump probe mode. Disable "
                          "interpolation mode first.\n");
            LOG(logERROR, (mess));
        } else {
            ret = setPumpProbe(arg);
            if (ret == FAIL) {
                sprintf(mess, "Could not set pump probe\n");
                LOG(logERROR, (mess));
            } else {
                int retval = getPumpProbe();
                validate(&ret, mess, (int)arg, (int)retval, "set pump probe",
                         DEC);
                LOG(logDEBUG1, ("pump probe retval: %u\n", retval));
            }
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_analog_pulsing(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting analog pulsing\n"));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    // get only
    retval = getAnalogPulsing();
    LOG(logDEBUG1, ("analog pulsing retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_analog_pulsing(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting analog pulsing: %u\n", arg));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        ret = setAnalogPulsing(arg);
        if (ret == FAIL) {
            sprintf(mess, "Could not set analog pulsing\n");
            LOG(logERROR, (mess));
        } else {
            int retval = getAnalogPulsing();
            validate(&ret, mess, (int)arg, (int)retval, "set analog pulsing",
                     DEC);
            LOG(logDEBUG1, ("analog pulsing retval: %u\n", retval));
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_digital_pulsing(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting digital pulsing\n"));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    // get only
    retval = getDigitalPulsing();
    LOG(logDEBUG1, ("digital pulsing retval: %u\n", retval));
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_digital_pulsing(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = 0;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logINFO, ("Setting digital pulsing: %u\n", arg));

#ifndef MYTHEN3D
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        ret = setDigitalPulsing(arg);
        if (ret == FAIL) {
            sprintf(mess, "Could not set digital pulsing\n");
            LOG(logERROR, (mess));
        } else {
            int retval = getDigitalPulsing();
            validate(&ret, mess, (int)arg, (int)retval, "set digital pulsing",
                     DEC);
            LOG(logDEBUG1, ("digital pulsing retval: %u\n", retval));
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_synchronization(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int retval = -1;

    LOG(logDEBUG1, ("Getting synchronization\n"));

#if !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    retval = getSynchronization();
#endif
    return Server_SendResult(file_des, INT32, &retval, sizeof(retval));
}

int set_synchronization(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    int arg = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Setting synchronization: %u\n", (int)arg));

#if !defined(JUNGFRAUD) && !defined(MOENCHD)
    functionNotImplemented();
#else
    // only set
    if (Server_VerifyLock() == OK) {
        if ((check_detector_idle("set synchronization") == OK) &&
            (arg != 0 && arg != 1)) {
            ret = FAIL;
            sprintf(mess,
                    "Could not set synchronization. Invalid argument %d.\n",
                    arg);
            LOG(logERROR, (mess));
        } else {
            setSynchronization(arg);
            int retval = getSynchronization();
            LOG(logDEBUG1, ("synchronization retval: %u\n", retval));
            validate(&ret, mess, arg, retval, "set synchronization", DEC);
        }
    }
#endif
    return Server_SendResult(file_des, INT32, NULL, 0);
}

int get_hardware_version(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    char retvals[MAX_STR_LENGTH];
    memset(retvals, 0, MAX_STR_LENGTH);
    getHardwareVersion(retvals);
    LOG(logDEBUG1, ("hardware version retval: %s\n", retvals));
    return Server_SendResult(file_des, OTHER, retvals, sizeof(retvals));
}

int get_frontend_firmware_version(int file_des) {
    ret = OK;
    memset(mess, 0, sizeof(mess));
    enum fpgaPosition arg = FRONT_LEFT;
    int64_t retval = -1;

    if (receiveData(file_des, &arg, sizeof(arg), INT32) < 0)
        return printSocketReadError();
    LOG(logDEBUG1, ("Getting front end firmware version: %s\n",
                    (arg == FRONT_LEFT ? "left" : "right")));

#if !defined(EIGERD)
    functionNotImplemented();
#else
    switch (arg) {
    case FRONT_LEFT:
    case FRONT_RIGHT:
        break;
    default:
        modeNotImplemented("Fpga position Index", (int)arg);
        break;
    }
    if (ret == OK) {
        retval = getFrontEndFirmwareVersion(arg);
        if (retval == 0) {
            ret = FAIL;
            strcpy(mess, "Could not get febl/r firmware version\n");
            LOG(logERROR, (mess));
        } else {
            LOG(logDEBUG1, ("Front %s version retval: 0x%llx\n",
                            (arg == FRONT_LEFT ? "left" : "right"),
                            (long long int)retval));
        }
    }
#endif
    return Server_SendResult(file_des, INT64, &retval, sizeof(retval));
}