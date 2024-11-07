// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "FebControl.h"
#include "Beb.h"
#include "FebRegisterDefs.h"
#include "clogger.h"
#include "sharedMemory.h"
#include "slsDetectorServer_defs.h"

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <termios.h> // POSIX terminal control definitions(CS8, CREAD, CLOCAL..)
#include <time.h>
#include <unistd.h>

const unsigned int Feb_Control_leftAddress = 0x100;
const unsigned int Feb_Control_rightAddress = 0x200;

int Feb_Control_master = -1;
int Feb_Control_normal = 0;
int Feb_Control_activated = 1;

int Feb_Control_hv_fd = -1;
unsigned int Feb_Control_idelay[4]; // ll,lr,rl,ll
int Feb_Control_counter_bit = 1;
unsigned int Feb_Control_staticBits = 0;
unsigned int Feb_Control_acquireNReadoutMode = 0;
unsigned int Feb_Control_triggerMode = 0;
unsigned int Feb_Control_externalEnableMode = 0;
unsigned int Feb_Control_subFrameMode = 0;
unsigned int Feb_Control_quadMode = 0;

unsigned int Feb_Control_nimages = 0;
double Feb_Control_exposure_time_in_sec = 0;
int64_t Feb_Control_subframe_exposure_time_in_10nsec = 0;
int64_t Feb_Control_subframe_period_in_10nsec = 0;
double Feb_Control_exposure_period_in_sec = 0;

unsigned int Feb_Control_trimbit_size = 0;
unsigned int *Feb_Control_last_downloaded_trimbits = 0;

int64_t Feb_Control_RateTable_Tau_in_nsec = -1;
int64_t Feb_Control_RateTable_Period_in_nsec = -1;
unsigned int Feb_Control_rate_correction_table[1024] = {};
double Feb_Control_rate_meas[16384] = {};
double ratemax = -1;

// setup
void Feb_Control_activate(int activate) { Feb_Control_activated = activate; }

int Feb_Control_FebControl(int normal) {
    Feb_Control_staticBits = 0;
    Feb_Control_acquireNReadoutMode = 0;
    Feb_Control_triggerMode = 0;
    Feb_Control_externalEnableMode = 0;
    Feb_Control_subFrameMode = 0;
    Feb_Control_trimbit_size = 263680;

    Feb_Control_last_downloaded_trimbits =
        malloc(Feb_Control_trimbit_size * sizeof(int));
    if (Feb_Control_last_downloaded_trimbits == NULL) {
        LOG(logERROR,
            ("Could not allocate memory for last downloaded trimbits\n"));
        return 0;
    }

    Feb_Control_normal = normal;
    if (!Feb_Interface_SetAddress(Feb_Control_rightAddress,
                                  Feb_Control_leftAddress))
        return 0;
    if (Feb_Control_activated) {
        return Feb_Interface_SetByteOrder();
    }
    return 1;
}

int Feb_Control_OpenSerialCommunication() {
    LOG(logINFO, ("opening serial communication of hv\n"));
    close(Feb_Control_hv_fd);
    Feb_Control_hv_fd =
        open(SPECIAL9M_HIGHVOLTAGE_PORT, O_RDWR | O_NOCTTY | O_SYNC);
    if (Feb_Control_hv_fd < 0) {
        LOG(logERROR, ("Unable to open port %s to set up high "
                       "voltage serial communciation to the blackfin\n",
                       SPECIAL9M_HIGHVOLTAGE_PORT));
        return 0;
    }
    LOG(logINFO, ("Serial Port opened at %s\n", SPECIAL9M_HIGHVOLTAGE_PORT));
    struct termios serial_conf;
    // reset structure
    memset(&serial_conf, 0, sizeof(serial_conf));
    // control options
    serial_conf.c_cflag = B2400 | CS8 | CREAD | CLOCAL; // 57600 too high
    // input options
    serial_conf.c_iflag = IGNPAR;
    // output options
    serial_conf.c_oflag = 0;
    // line options
    serial_conf.c_lflag = ICANON;
    // flush input
    if (tcflush(Feb_Control_hv_fd, TCIOFLUSH) < 0) {
        LOG(logERROR, ("error from tcflush %d\n", errno));
        return 0;
    }
    // set new options for the port, TCSANOW:changes occur immediately without
    // waiting for data to complete
    if (tcsetattr(Feb_Control_hv_fd, TCSANOW, &serial_conf) < 0) {
        LOG(logERROR, ("error from tcsetattr %d\n", errno));
        return 0;
    }
    if (tcsetattr(Feb_Control_hv_fd, TCSAFLUSH, &serial_conf) < 0) {
        LOG(logERROR, ("error from tcsetattr %d\n", errno));
        return 0;
    }

    // send the first message (which will be garbled up)
    char buffer[SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE];
    memset(buffer, 0, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
    buffer[SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE - 1] = '\n';
    strcpy(buffer, "start");
    LOG(logINFO, ("sending start: '%s'\n", buffer));
    int n = write(Feb_Control_hv_fd, buffer, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
    if (n < 0) {
        LOG(logERROR, ("could not write to i2c bus\n"));
        return 0;
    }
    LOG(logDEBUG1, ("Sent: %d bytes\n", n));
    return 1;
}

void Feb_Control_CloseSerialCommunication() {
    if (Feb_Control_hv_fd != -1)
        close(Feb_Control_hv_fd);
}

int Feb_Control_CheckSetup(int master) {
    LOG(logDEBUG1, ("Checking Set up\n"));

    for (unsigned int j = 0; j < 4; j++) {
        if (Feb_Control_idelay[j] < 0) {
            LOG(logERROR, ("idelay chip %d not set.\n", j));
            return 0;
        }
    }
    int value = 0;
    if ((Feb_Control_master) && (!Feb_Control_GetHighVoltage(&value))) {
        LOG(logERROR, ("high voltage not set.\n"));
        return 0;
    }
    LOG(logDEBUG1, ("Done Checking Set up\n"));
    return 1;
}

unsigned int Feb_Control_AddressToAll() {
    return Feb_Control_leftAddress | Feb_Control_rightAddress;
}

int Feb_Control_SetCommandRegister(unsigned int cmd) {
    if (Feb_Control_activated)
        return Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),
                                           DAQ_REG_CHIP_CMDS, cmd, 0, 0);
    else
        return 1;
}

int Feb_Control_SetStaticBits() {
    if (Feb_Control_activated) {
        // program=1,m4=2,m8=4,test=8,rotest=16,cs_bar_left=32,cs_bar_right=64
        if (!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),
                                         DAQ_REG_STATIC_BITS,
                                         Feb_Control_staticBits, 0, 0) ||
            !Feb_Control_SetCommandRegister(DAQ_SET_STATIC_BIT) ||
            (Feb_Control_StartDAQOnlyNWaitForFinish(5000) != STATUS_IDLE)) {
            LOG(logERROR, ("Could not set static bits\n"));
            return 0;
        }
    }
    return 1;
}

int Feb_Control_SetStaticBits1(unsigned int the_static_bits) {
    Feb_Control_staticBits = the_static_bits;
    return Feb_Control_SetStaticBits();
}

int Feb_Control_SetInTestModeVariable(int on) {
    if (on)
        Feb_Control_staticBits |=
            DAQ_STATIC_BIT_CHIP_TEST; // setting test bit to high
    else
        Feb_Control_staticBits &=
            (~DAQ_STATIC_BIT_CHIP_TEST); // setting test bit to low
    return 1;
}

int Feb_Control_GetTestModeVariable() {
    return Feb_Control_staticBits & DAQ_STATIC_BIT_CHIP_TEST;
}

// idelay
int Feb_Control_SetIDelays(unsigned int ndelay_units) {
    int ret = Feb_Control_SetIDelays1(0, ndelay_units) &&
              Feb_Control_SetIDelays1(1, ndelay_units) &&
              Feb_Control_SetIDelays1(2, ndelay_units) &&
              Feb_Control_SetIDelays1(3, ndelay_units);
    if (ret) {
        LOG(logINFO, ("IODelay set to %d\n", ndelay_units));
    }
    return ret;
}

int Feb_Control_SetIDelays1(
    unsigned int chip_pos,
    unsigned int ndelay_units) { // chip_pos 0=ll,1=lr,0=rl,1=rr
    if (chip_pos > 3) {
        LOG(logERROR, ("SetIDelay chip_pos %d doesn't exist.\n", chip_pos));
        return 0;
    }

    if (chip_pos / 2 == 0) { // left fpga
        if (Feb_Control_SendIDelays(Feb_Control_leftAddress, chip_pos % 2 == 0,
                                    0xffffffff, ndelay_units)) {
            Feb_Control_idelay[chip_pos] = ndelay_units;
        } else {
            LOG(logERROR, ("could not set idelay (left).\n"));
            return 0;
        }
    } else {
        if (Feb_Control_SendIDelays(Feb_Control_rightAddress, chip_pos % 2 == 0,
                                    0xffffffff, ndelay_units)) {
            Feb_Control_idelay[chip_pos] = ndelay_units;
        } else {
            LOG(logERROR, ("could not set idelay (right).\n"));
            return 0;
        }
    }
    return 1;
}

int Feb_Control_SendIDelays(unsigned int dst_num, int chip_lr,
                            unsigned int channels, unsigned int ndelay_units) {
    if (ndelay_units > 0x3ff)
        ndelay_units = 0x3ff;
    // this is global
    unsigned int delay_data_valid_nclks =
        15 - ((ndelay_units & 0x3c0) >> 6); // data valid delay upto 15 clks
    ndelay_units &= 0x3f;

    unsigned int set_left_delay_channels = chip_lr ? channels : 0;
    unsigned int set_right_delay_channels = chip_lr ? 0 : channels;

    LOG(logDEBUG1,
        ("\tSetting delays of %s chips of dst_num %d, "
         "tracks 0x%x to %d, %d clks and %d units.\n",
         ((set_left_delay_channels != 0) ? "left" : "right"), dst_num, channels,
         (((15 - delay_data_valid_nclks) << 6) | ndelay_units),
         delay_data_valid_nclks, ndelay_units));

    if (Feb_Control_activated) {
        if (!Feb_Interface_WriteRegister(
                dst_num, CHIP_DATA_OUT_DELAY_REG2,
                1 << 31 | delay_data_valid_nclks << 16 | ndelay_units, 0,
                0) || // the 1<<31 time enables the setting of the data valid
                      // delays
            !Feb_Interface_WriteRegister(dst_num, CHIP_DATA_OUT_DELAY_REG3,
                                         set_left_delay_channels, 0, 0) ||
            !Feb_Interface_WriteRegister(dst_num, CHIP_DATA_OUT_DELAY_REG4,
                                         set_right_delay_channels, 0, 0) ||
            !Feb_Interface_WriteRegister(dst_num, CHIP_DATA_OUT_DELAY_REG_CTRL,
                                         CHIP_DATA_OUT_DELAY_SET, 1, 1)) {
            LOG(logERROR, ("could not SetChipDataInputDelays(...).\n"));
            return 0;
        }
    }
    return 1;
}

// highvoltage
// only master gets to call this function
int Feb_Control_SetHighVoltage(int value) {
    LOG(logDEBUG1, (" Setting High Voltage:\t"));
    /*
     * maximum voltage of the hv dc/dc converter:
     * 300 for single module power distribution board
     * 200 for 9M power distribution board
     * but limit is 200V for both
     */
    const float vmin = 0;
    float vmax = 200;
    if (Feb_Control_normal)
        vmax = 300;
    const float vlimit = 200;
    const unsigned int ntotalsteps = 256;
    unsigned int nsteps = ntotalsteps * vlimit / vmax;
    unsigned int dacval = 0;

    // calculate dac value
    if (!Feb_Control_VoltageToDAC(value, &dacval, nsteps, vmin, vlimit)) {
        LOG(logERROR,
            ("SetHighVoltage bad value, %d.  The range is 0 to %d V.\n", value,
             (int)vlimit));
        return -1;
    }
    LOG(logINFO, ("High Voltage set to %dV\n", value));
    LOG(logDEBUG1, ("High Voltage set to (%d dac):\t%dV\n", dacval, value));

    return Feb_Control_SendHighVoltage(dacval);
}

int Feb_Control_GetHighVoltage(int *value) {
    LOG(logDEBUG1, (" Getting High Voltage:\t"));
    unsigned int dacval = 0;

    if (!Feb_Control_ReceiveHighVoltage(&dacval))
        return 0;

    // ok, convert dac to v
    /*
     * maximum voltage of the hv dc/dc converter:
     * 300 for single module power distribution board
     * 200 for 9M power distribution board
     * but limit is 200V for both
     */
    const float vmin = 0;
    float vmax = 200;
    if (Feb_Control_normal)
        vmax = 300;
    const float vlimit = 200;
    const unsigned int ntotalsteps = 256;
    unsigned int nsteps = ntotalsteps * vlimit / vmax;
    *value =
        (int)(Feb_Control_DACToVoltage(dacval, nsteps, vmin, vlimit) + 0.5);
    LOG(logINFO, ("High Voltage read %dV\n", *value));
    LOG(logDEBUG1, ("High Voltage read (%d dac)\t%dV\n", dacval, *value));
    return 1;
}

int Feb_Control_SendHighVoltage(int dacvalue) {
    // normal
    if (Feb_Control_normal) {
        // open file
        FILE *fd = fopen(NORMAL_HIGHVOLTAGE_OUTPUTPORT, "w");
        if (fd == NULL) {
            LOG(logERROR,
                ("Could not open file for writing to set high voltage\n"));
            return 0;
        }
        // convert to string, add 0 and write to file
        fprintf(fd, "%d0\n", dacvalue);
        fclose(fd);
    }

    // 9m
    else {
        /*Feb_Control_OpenSerialCommunication();*/
        if (Feb_Control_hv_fd == -1) {
            LOG(logERROR,
                ("High voltage serial communication not set up for 9m\n"));
            return 0;
        }

        char buffer[SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE];
        memset(buffer, 0, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
        buffer[SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE - 1] = '\n';
        int n;
        sprintf(buffer, "p%d", dacvalue);
        LOG(logINFO, ("Sending HV: '%s'\n", buffer));
        n = write(Feb_Control_hv_fd, buffer, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
        if (n < 0) {
            LOG(logERROR, ("writing to i2c bus\n"));
            return 0;
        }
#ifdef VERBOSEI
        LOG(logINFO, ("Sent %d Bytes\n", n));
#endif
        // ok/fail
        memset(buffer, 0, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
        buffer[SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE - 1] = '\n';
        n = read(Feb_Control_hv_fd, buffer, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
        if (n < 0) {
            LOG(logERROR, ("reading from i2c bus\n"));
            return 0;
        }
#ifdef VERBOSEI
        LOG(logINFO, ("Received %d Bytes\n", n));
#endif
        LOG(logINFO, ("Received HV: '%s'\n", buffer));
        fflush(stdout);
        /*Feb_Control_CloseSerialCommunication();*/
        if (buffer[0] != 's') {
            LOG(logERROR, ("\nError: Failed to set high voltage\n"));
            return 0;
        }
        LOG(logINFO, ("%s\n", buffer));
    }

    return 1;
}

int Feb_Control_ReceiveHighVoltage(unsigned int *value) {

    // normal
    if (Feb_Control_normal) {
        if (readParameterFromFile(NORMAL_HIGHVOLTAGE_INPUTPORT, "high voltage",
                                  value) == FAIL) {
            LOG(logERROR, ("Could not get high voltage\n"));
            return 0;
        }
        *value /= 10;
        LOG(logINFO, ("High Voltage: %d\n", (*value)));
    }

    // 9m
    else {
        /*Feb_Control_OpenSerialCommunication();*/

        if (Feb_Control_hv_fd == -1) {
            LOG(logERROR,
                ("High voltage serial communication not set up for 9m\n"));
            return 0;
        }
        char buffer[SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE];
        buffer[SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE - 1] = '\n';
        int n = 0;
        // request

        strcpy(buffer, "g ");
        LOG(logINFO, ("\nSending HV: '%s'\n", buffer));
        n = write(Feb_Control_hv_fd, buffer, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
        if (n < 0) {
            LOG(logERROR, ("writing to i2c bus\n"));
            return 0;
        }
#ifdef VERBOSEI
        LOG(logINFO, ("Sent %d Bytes\n", n));
#endif

        // ok/fail
        memset(buffer, 0, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
        buffer[SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE - 1] = '\n';
        n = read(Feb_Control_hv_fd, buffer, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
        if (n < 0) {
            LOG(logERROR, ("reading from i2c bus\n"));
            return 0;
        }
#ifdef VERBOSEI
        LOG(logINFO, ("Received %d Bytes\n", n));
#endif
        LOG(logINFO, ("Received HV: '%s'\n", buffer));
        if (buffer[0] != 's') {
            LOG(logERROR, ("failed to read high voltage\n"));
            return 0;
        }

        memset(buffer, 0, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
        buffer[SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE - 1] = '\n';
        n = read(Feb_Control_hv_fd, buffer, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
        if (n < 0) {
            LOG(logERROR, ("reading from i2c bus\n"));
            return 0;
        }
#ifdef VERBOSEI
        LOG(logINFO, ("Received %d Bytes\n", n));
#endif
        LOG(logINFO, ("Received HV: '%s'\n", buffer));
        /*Feb_Control_OpenSerialCommunication();*/
        if (!sscanf(buffer, "%d", value)) {
            LOG(logERROR, ("failed to scan high voltage read\n"));
            return 0;
        }
    }
    return 1;
}

// dacs
int Feb_Control_VoltageToDAC(float value, unsigned int *digital,
                             unsigned int nsteps, float vmin, float vmax) {
    if (value < vmin || value > vmax)
        return 0;
    *digital = (int)(((value - vmin) / (vmax - vmin)) * (nsteps - 1) + 0.5);
    return 1;
}

float Feb_Control_DACToVoltage(unsigned int digital, unsigned int nsteps,
                               float vmin, float vmax) {
    return vmin + (vmax - vmin) * digital / (nsteps - 1);
}

int Feb_Control_SetDAC(unsigned int ch, int value) {
    unsigned int dst_num = Feb_Control_rightAddress;
    if (ch < 0 || ch > 15) {
        LOG(logERROR, ("invalid ch for SetDAC.\n"));
        return 0;
    }

    value &= 0xfff;
    unsigned int dac_ic = (ch < 8) ? 1 : 2;
    unsigned int dac_ch = ch % 8;
    unsigned int r =
        dac_ic << 30 | 3 << 16 | dac_ch << 12 | value; // 3 write and power up

    if (Feb_Control_activated) {
        if (!Feb_Interface_WriteRegister(dst_num, 0, r, 1, 0)) {
            LOG(logERROR, ("trouble setting dac %d voltage.\n", ch));
            return 0;
        }
    }

    float voltage = Feb_Control_DACToVoltage(value, 4096, 0, 2048);
    LOG(logINFO, ("\tset to %d (%.2fmV)\n", value, voltage));
    return 1;
}

int Feb_Control_SetTrimbits(unsigned int *trimbits, int top) {
    LOG(logINFO, ("Setting Trimbits (top:%d)\n", top));
    unsigned int trimbits_to_load_l[1024];
    unsigned int trimbits_to_load_r[1024];

    if (Feb_Control_Reset() == STATUS_ERROR) {
        LOG(logERROR, ("could not reset DAQ.\n"));
    }

    for (int l_r = 0; l_r < 2; l_r++) { // l_r loop
        unsigned int disable_chip_mask =
            l_r ? DAQ_CS_BAR_LEFT : DAQ_CS_BAR_RIGHT;
        if (Feb_Control_activated) {
            if (!(Feb_Interface_WriteRegister(0xfff, DAQ_REG_STATIC_BITS,
                                              disable_chip_mask |
                                                  DAQ_STATIC_BIT_PROGRAM |
                                                  DAQ_STATIC_BIT_M8,
                                              0, 0) &&
                  Feb_Control_SetCommandRegister(DAQ_SET_STATIC_BIT) &&
                  (Feb_Control_StartDAQOnlyNWaitForFinish(5000) ==
                   STATUS_IDLE))) {
                LOG(logERROR, ("Could not select chips\n"));
                return 0;
            }
        }

        for (int row_set = 0; row_set < 16; row_set++) { // 16 rows at a time
            if (row_set == 0) {
                if (!Feb_Control_SetCommandRegister(
                        DAQ_RESET_COMPLETELY | DAQ_SEND_A_TOKEN_IN |
                        DAQ_LOAD_16ROWS_OF_TRIMBITS)) {
                    LOG(logERROR, ("Could not Feb_Control_SetCommandRegister "
                                   "for loading trim bits.\n"));
                    return 0;
                }
            } else {
                if (!Feb_Control_SetCommandRegister(
                        DAQ_LOAD_16ROWS_OF_TRIMBITS)) {
                    LOG(logERROR, ("Could not Feb_Control_SetCommandRegister "
                                   "for loading trim bits.\n"));
                    return 0;
                }
            }

            for (int row = 0; row < 16; row++) { // row loop
                int offset = 2 * 32 * row;
                for (int sc = 0; sc < 32; sc++) { // supercolumn loop sc
                    int super_column_start_position_l =
                        1030 * row + l_r * 258 + sc * 8;
                    int super_column_start_position_r =
                        1030 * row + 516 + l_r * 258 + sc * 8;

                    /*
                      int super_column_start_position_l = 1024*row +       l_r
                      *256 + sc*8; //256 per row, 8 per super column int
                      super_column_start_position_r = 1024*row + 512 + l_r *256
                      + sc*8; //256 per row, 8 per super column
                     */
                    int chip_sc = 31 - sc;
                    trimbits_to_load_l[offset + chip_sc] = 0;
                    trimbits_to_load_r[offset + chip_sc] = 0;
                    trimbits_to_load_l[offset + chip_sc + 32] = 0;
                    trimbits_to_load_r[offset + chip_sc + 32] = 0;
                    for (int i = 0; i < 8; i++) { // column loop i

                        if (top == 1) {
                            trimbits_to_load_l[offset + chip_sc] |=
                                (0x7 &
                                 trimbits[row_set * 16480 +
                                          super_column_start_position_l + i])
                                << ((7 - i) * 4); // low
                            trimbits_to_load_l[offset + chip_sc + 32] |=
                                ((0x38 &
                                  trimbits[row_set * 16480 +
                                           super_column_start_position_l +
                                           i]) >>
                                 3)
                                << ((7 - i) * 4); // upper
                            trimbits_to_load_r[offset + chip_sc] |=
                                (0x7 &
                                 trimbits[row_set * 16480 +
                                          super_column_start_position_r + i])
                                << ((7 - i) * 4); // low
                            trimbits_to_load_r[offset + chip_sc + 32] |=
                                ((0x38 &
                                  trimbits[row_set * 16480 +
                                           super_column_start_position_r +
                                           i]) >>
                                 3)
                                << ((7 - i) * 4); // upper
                        } else {
                            trimbits_to_load_l[offset + chip_sc] |=
                                (0x7 &
                                 trimbits[263679 -
                                          (row_set * 16480 +
                                           super_column_start_position_l + i)])
                                << ((7 - i) * 4); // low
                            trimbits_to_load_l[offset + chip_sc + 32] |=
                                ((0x38 &
                                  trimbits[263679 -
                                           (row_set * 16480 +
                                            super_column_start_position_l +
                                            i)]) >>
                                 3)
                                << ((7 - i) * 4); // upper
                            trimbits_to_load_r[offset + chip_sc] |=
                                (0x7 &
                                 trimbits[263679 -
                                          (row_set * 16480 +
                                           super_column_start_position_r + i)])
                                << ((7 - i) * 4); // low
                            trimbits_to_load_r[offset + chip_sc + 32] |=
                                ((0x38 &
                                  trimbits[263679 -
                                           (row_set * 16480 +
                                            super_column_start_position_r +
                                            i)]) >>
                                 3)
                                << ((7 - i) * 4); // upper
                        }
                    } // end column loop i
                }     // end supercolumn loop sc
            }         // end row loop

            if (Feb_Control_activated) {
                if (!Feb_Interface_WriteMemoryInLoops(Feb_Control_leftAddress,
                                                      0, 0, 1024,
                                                      trimbits_to_load_l) ||
                    !Feb_Interface_WriteMemoryInLoops(Feb_Control_rightAddress,
                                                      0, 0, 1024,
                                                      trimbits_to_load_r) ||
                    (Feb_Control_StartDAQOnlyNWaitForFinish(5000) !=
                     STATUS_IDLE)) {
                    LOG(logERROR, (" some errror in setting trimbits!\n"));
                    return 0;
                }
            }

        } // end row_set loop (groups of 16 rows)
    }     // end l_r loop

    memcpy(Feb_Control_last_downloaded_trimbits, trimbits,
           Feb_Control_trimbit_size * sizeof(unsigned int));

    return Feb_Control_SetStaticBits(); // send the static bits
}

int Feb_Control_SaveAllTrimbitsTo(int value, int top) {
    unsigned int chanregs[Feb_Control_trimbit_size];
    for (int i = 0; i < Feb_Control_trimbit_size; i++)
        chanregs[i] = value;
    return Feb_Control_SetTrimbits(chanregs, top);
}

unsigned int *Feb_Control_GetTrimbits() {
    return Feb_Control_last_downloaded_trimbits;
}

int Feb_Control_AcquisitionInProgress() {
    unsigned int status_reg_r = 0, status_reg_l = 0;
    // deactivated should return end of acquisition
    if (!Feb_Control_activated)
        return STATUS_IDLE;

    if (!(Feb_Control_GetDAQStatusRegister(Feb_Control_rightAddress,
                                           &status_reg_r))) {
        LOG(logERROR, ("Error: Trouble reading Status register (right)"
                       "address\n"));
        return STATUS_ERROR;
    }
    if (!(Feb_Control_GetDAQStatusRegister(Feb_Control_leftAddress,
                                           &status_reg_l))) {
        LOG(logERROR, ("Error: Trouble reading Status register (left)\n"));
        return STATUS_ERROR;
    }
    // running
    if ((status_reg_r | status_reg_l) & DAQ_STATUS_DAQ_RUNNING) {
        return STATUS_RUNNING;
    }
    // idle
    return STATUS_IDLE;
}

int Feb_Control_ProcessingInProgress() {
    unsigned int regr = 0, regl = 0;
    // deactivated should return end of processing
    if (!Feb_Control_activated)
        return STATUS_IDLE;

    if (!Feb_Interface_ReadRegister(Feb_Control_rightAddress, FEB_REG_STATUS,
                                    &regr)) {
        LOG(logERROR, ("Could not read right FEB_REG_STATUS to get feb "
                       "processing status\n"));
        return STATUS_ERROR;
    }
    if (!Feb_Interface_ReadRegister(Feb_Control_leftAddress, FEB_REG_STATUS,
                                    &regl)) {
        LOG(logERROR, ("Could not read left FEB_REG_STATUS to get feb "
                       "processing status\n"));
        return STATUS_ERROR;
    }
    LOG(logDEBUG1, ("regl:0x%x regr:0x%x\n", regl, regr));
    // processing done
    if (regr & regl & FEB_REG_STATUS_ACQ_DONE_MSK) {
        return STATUS_IDLE;
    }
    // processing running
    return STATUS_RUNNING;
}

int Feb_Control_AcquisitionStartedBit() {
    unsigned int status_reg_r = 0, status_reg_l = 0;
    // deactivated should return acquisition started/ready
    if (!Feb_Control_activated)
        return 1;

    if (!(Feb_Control_GetDAQStatusRegister(Feb_Control_rightAddress,
                                           &status_reg_r))) {
        LOG(logERROR, ("Error: Trouble reading Status register (right)\n"));
        return -1;
    }
    if (!(Feb_Control_GetDAQStatusRegister(Feb_Control_leftAddress,
                                           &status_reg_l))) {
        LOG(logERROR, ("Error: Trouble reading Status register (left)\n"));
        return -1;
    }
    // doesnt mean it started, just the bit
    if ((status_reg_r | status_reg_l) & DAQ_STATUS_DAQ_RUN_TOGGLE)
        return 1;
    return 0;
}

int Feb_Control_WaitForStartedFlag(int sleep_time_us, int prev_flag) {
    // deactivated dont wait (otherwise give a toggle value back)
    if (!Feb_Control_activated)
        return 1;

    // did not start
    if (prev_flag == -1)
        return 0;

    int value = prev_flag;
    while (value == prev_flag) {
        usleep(sleep_time_us);
        value = Feb_Control_AcquisitionStartedBit();
    }

    // did not start
    if (value == -1)
        return 0;

    return 1;
}

int Feb_Control_WaitForFinishedFlag(int sleep_time_us, int tempLock) {
    int is_running = Feb_Control_AcquisitionInProgress();
    // unlock for stop server
    if (tempLock) {
        sharedMemory_unlockLocalLink();
    }
    int check_error = 0;

    // it will break out if it is idle or if check_error is more than 5 times
    while (is_running != STATUS_IDLE) {
        usleep(sleep_time_us);
        if (tempLock) {
            sharedMemory_lockLocalLink();
        }
        is_running = Feb_Control_AcquisitionInProgress();
        if (tempLock) {
            sharedMemory_unlockLocalLink();
        }
        // check error only 5 times (ensuring it is not something that happens
        // sometimes)
        if (is_running == STATUS_ERROR) {
            if (check_error == 5)
                break;
            check_error++;
        } // reset check_error for next time
        else
            check_error = 0;
    }
    // lock it again to be unlocked later
    if (tempLock) {
        sharedMemory_lockLocalLink();
    }
    return is_running;
}

int Feb_Control_GetDAQStatusRegister(unsigned int dst_address,
                                     unsigned int *ret_status) {
    // if deactivated, should  be handled earlier and should not get into this
    // function
    if (Feb_Control_activated) {
        if (!Feb_Interface_ReadRegister(dst_address, DAQ_REG_STATUS,
                                        ret_status)) {
            LOG(logERROR, ("Error: reading status register.\n"));
            return 0;
        }
    }
    *ret_status = (0x02FF0000 & *ret_status) >> 16;
    return 1;
}

int Feb_Control_StartDAQOnlyNWaitForFinish(int sleep_time_us) {
    if (Feb_Control_activated) {
        if (!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),
                                         DAQ_REG_CTRL, 0, 0, 0) ||
            !Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),
                                         DAQ_REG_CTRL, DAQ_CTRL_START, 0, 0)) {
            LOG(logERROR, ("could not start.\n"));
            return 0;
        }
    }
    return Feb_Control_WaitForFinishedFlag(sleep_time_us, 0);
}

int Feb_Control_Reset() {
    LOG(logINFO, ("Feb: Reset daq\n"));
    if (Feb_Control_activated) {
        if (!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),
                                         DAQ_REG_CTRL, 0, 0, 0) ||
            !Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),
                                         DAQ_REG_CTRL, DAQ_CTRL_RESET, 0, 0) ||
            !Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),
                                         DAQ_REG_CTRL, 0, 0, 0)) {
            LOG(logERROR, ("Could not reset daq, no response.\n"));
            return 0;
        }
    }
    return Feb_Control_WaitForFinishedFlag(5000, 0);
}

int Feb_Control_ResetChipCompletely() {
    if (!Feb_Control_SetCommandRegister(DAQ_RESET_COMPLETELY) ||
        (Feb_Control_StartDAQOnlyNWaitForFinish(5000) != STATUS_IDLE)) {
        LOG(logERROR, ("could not ResetChipCompletely() with 0x%x.\n",
                       DAQ_RESET_COMPLETELY));
        return 0;
    }
    LOG(logINFO, ("Chip reset completely\n"));
    return 1;
}

int Feb_Control_ResetChipPartially() {
    if (!Feb_Control_SetCommandRegister(DAQ_RESET_PERIPHERY) ||
        (Feb_Control_StartDAQOnlyNWaitForFinish(5000) != STATUS_IDLE)) {
        LOG(logERROR, ("could not ResetChipPartially with periphery\n"));
        return 0;
    }
    LOG(logINFO, ("Chip reset periphery 0x%x\n", DAQ_RESET_PERIPHERY));

    if (!Feb_Control_SetCommandRegister(DAQ_RESET_COLUMN_SELECT) ||
        (Feb_Control_StartDAQOnlyNWaitForFinish(5000) != STATUS_IDLE)) {
        LOG(logERROR, ("could not ResetChipPartially with column select\n"));
        return 0;
    }
    LOG(logINFO, ("Chip reset column select 0x%x\n", DAQ_RESET_COLUMN_SELECT));
    return 1;
}

int Feb_Control_SendBitModeToBebServer() {
    unsigned int just_bit_mode =
        (DAQ_STATIC_BIT_M4 | DAQ_STATIC_BIT_M8) & Feb_Control_staticBits;
    unsigned int bit_mode = 16; // default
    if (just_bit_mode == DAQ_STATIC_BIT_M4)
        bit_mode = 4;
    else if (just_bit_mode == DAQ_STATIC_BIT_M8)
        bit_mode = 8;
    else if (Feb_Control_subFrameMode &
             DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING)
        bit_mode = 32;

    if (!Beb_SetUpTransferParameters(bit_mode)) {
        LOG(logERROR, ("Error: sending bit mode to beb\n"));
        return 0;
    }
    return 1;
}

unsigned int Feb_Control_ConvertTimeToRegister(float time_in_sec) {
    float n_clk_cycles =
        round(time_in_sec / 10e-9); // 200 MHz ctb clk or 100 MHz feb clk

    unsigned int decoded_time;
    if (n_clk_cycles > (pow(2, 29) - 1) * pow(10, 7)) {
        float max_time = 10e-9 * (pow(2, 28) - 1) * pow(10, 7);
        LOG(logERROR, ("time exceeds (%f) maximum exposure time of %f sec.\n",
                       time_in_sec, max_time));
        LOG(logINFO, ("\t Setting to maximum %f us.\n", max_time));
        decoded_time = 0xffffffff;
    } else {
        int power_of_ten = 0;
        while (n_clk_cycles > pow(2, 29) - 1) {
            power_of_ten++;
            n_clk_cycles = round(n_clk_cycles / 10.0);
        }
        decoded_time = (int)(n_clk_cycles) << 3 | (int)(power_of_ten);
    }
    return decoded_time;
}

int Feb_Control_PrepareForAcquisition() {
    LOG(logINFOBLUE, ("Preparing for Acquisition\n"));
    if (!Feb_Control_PrintAcquisitionSetup()) {
        LOG(logERROR, ("Could not prepare acquisition\n"));
        return 0;
    }

    if (Feb_Control_Reset() == STATUS_ERROR) {
        LOG(logERROR, ("Trouble reseting daq or data stream\n"));
        return 0;
    }

    if (!Feb_Control_SetStaticBits1(Feb_Control_staticBits &
                                    (DAQ_STATIC_BIT_M4 | DAQ_STATIC_BIT_M8))) {
        LOG(logERROR, ("Trouble setting static bits\n"));
        return 0;
    }

    if (!Feb_Control_SendBitModeToBebServer()) {
        LOG(logERROR, ("Trouble sending static bits to server\n"));
        return 0;
    }

    int ret = 0;
    if (Feb_Control_counter_bit)
        ret = Feb_Control_ResetChipCompletely();
    else
        ret = Feb_Control_ResetChipPartially();
    if (!ret) {
        LOG(logERROR, ("Trouble resetting chips\n"));
        return 0;
    }

    unsigned int reg_nums[7];
    unsigned int reg_vals[7];
    reg_nums[0] = DAQ_REG_CTRL;
    reg_vals[0] = 0;
    reg_nums[1] = DAQ_REG_NEXPOSURES;
    reg_vals[1] = Feb_Control_nimages;
    reg_nums[2] = DAQ_REG_EXPOSURE_TIMER;
    reg_vals[2] =
        Feb_Control_ConvertTimeToRegister(Feb_Control_exposure_time_in_sec);
    reg_nums[3] = DAQ_REG_EXPOSURE_REPEAT_TIMER;
    reg_vals[3] =
        Feb_Control_ConvertTimeToRegister(Feb_Control_exposure_period_in_sec);
    reg_nums[4] = DAQ_REG_CHIP_CMDS;
    reg_vals[4] = (Feb_Control_acquireNReadoutMode | Feb_Control_triggerMode |
                   Feb_Control_externalEnableMode | Feb_Control_subFrameMode);
    reg_nums[5] = DAQ_REG_SUBFRAME_EXPOSURES;
    reg_vals[5] = Feb_Control_subframe_exposure_time_in_10nsec;
    reg_nums[6] = DAQ_REG_SUBFRAME_PERIOD;
    reg_vals[6] = Feb_Control_subframe_period_in_10nsec;
    if (Feb_Control_activated) {
        if (!Feb_Interface_WriteRegisters(Feb_Control_AddressToAll(), 7,
                                          reg_nums, reg_vals, 0, 0)) {
            LOG(logERROR, ("Trouble starting acquisition\n"));
            return 0;
        }
    }
    return 1;
}

int Feb_Control_PrintAcquisitionSetup() {
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    int dr = 0;
    if (!Feb_Control_GetDynamicRange(&dr)) {
        LOG(logERROR, ("Could not print acquisition set up\n"));
        return 0;
    }
    LOG(logINFO, ("Starting an exposure: (%s)"
                  "\t Dynamic range nbits: %d\n"
                  "\t Trigger mode: 0x%x\n"
                  "\t Number of exposures: %d\n"
                  "\t Exsposure time (if used): %f seconds.\n"
                  "\t Exsposure period (if used): %f seconds.\n\n",
                  asctime(timeinfo), dr, Feb_Control_triggerMode,
                  Feb_Control_GetNExposures(), Feb_Control_exposure_time_in_sec,
                  Feb_Control_exposure_period_in_sec));

    return 1;
}

int Feb_Control_StartAcquisition() {
    LOG(logINFOBLUE, ("Starting Acquisition\n"));
    unsigned int reg_nums[15];
    unsigned int reg_vals[15];
    for (int i = 0; i < 14; i++) {
        reg_nums[i] = DAQ_REG_CTRL;
        reg_vals[i] = 0;
    }
    reg_nums[14] = DAQ_REG_CTRL;
    reg_vals[14] = ACQ_CTRL_START;
    if (Feb_Control_activated) {
        if (!Feb_Interface_WriteRegisters(Feb_Control_AddressToAll(), 15,
                                          reg_nums, reg_vals, 0, 0)) {
            LOG(logERROR, ("Trouble starting acquisition\n"));
            return 0;
        }
    }
    return 1;
}

int Feb_Control_StopAcquisition() {
    if (Feb_Control_activated) {

        // sends last frames from fifo
        unsigned int orig_value = 0;
        if (!Feb_Interface_ReadRegister(Feb_Control_AddressToAll(),
                                        DAQ_REG_CTRL, &orig_value)) {
            LOG(logERROR, ("Could not read DAQ_REG_CTRL to stop acquisition "
                           "(send complete frames)\n"));
            return 0;
        }
        if (!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),
                                         DAQ_REG_CTRL,
                                         orig_value | DAQ_CTRL_STOP, 0, 0)) {
            LOG(logERROR, ("Could not send last frames.\n"));
            return 0;
        }
        LOG(logINFO, ("Feb: Command to Flush out images from fifo\n"));

        // wait for feb processing to be done
        int is_processing = Feb_Control_ProcessingInProgress();
        int check_error = 0;
        int check_stuck = 0;
        while (is_processing != STATUS_IDLE) {
            usleep(500);
            is_processing = Feb_Control_ProcessingInProgress();

            // check error only 5 times (ensuring it is not something that
            // happens sometimes)
            if (is_processing == STATUS_ERROR) {
                if (check_error == 5)
                    break;
                check_error++;
            } // reset check_error for next time
            else {
                check_error = 0;
            }

            // check stuck only 2000 times (1s)
            if (is_processing == STATUS_RUNNING) {
                if (check_stuck == 2000) {
                    LOG(logERROR,
                        ("Unable to get feb processing done signal\n"));
                    // at least it is idle
                    if (Feb_Control_AcquisitionInProgress() == STATUS_IDLE) {
                        return 1;
                    }
                    LOG(logERROR, ("Unable to get acquisition done signal\n"));
                    return 0;
                }
                check_stuck++;
            } // reset check_stuck for next time
            else {
                check_stuck = 0;
            }
        }
        LOG(logINFO, ("Feb: Processing done (to stop acq)\n"));
    }
    return 1;
}

int Feb_Control_IsReadyForTrigger(int *readyForTrigger) {
    unsigned int addr[2] = {Feb_Control_leftAddress, Feb_Control_rightAddress};
    unsigned int value[2] = {0, 0};

    for (int i = 0; i < 2; ++i) {
        if (!Feb_Interface_ReadRegister(addr[i], FEB_REG_STATUS, &value[i])) {
            LOG(logERROR, ("Could not read %s FEB_REG_STATUS reg\n",
                           (i == 0 ? "left" : "right")));
            return 0;
        }
    }
    *readyForTrigger =
        ((value[0] | value[1]) & FEB_REG_STATUS_WAIT_FOR_TRGGR_MSK);
    return 1;
}

int Feb_Control_SendSoftwareTrigger() {
    // read old value in register
    unsigned int orig_value = 0;
    if (!Feb_Interface_ReadRegister(Feb_Control_AddressToAll(),
                                    DAQ_REG_CHIP_CMDS, &orig_value)) {
        LOG(logERROR, ("Could not read DAQ_REG_CHIP_CMDS to send software "
                       "trigger\n"));
        return 0;
    }
    unsigned int cmd = orig_value | DAQ_REG_CHIP_CMDS_INT_TRIGGER;

    // set trigger bit
    LOG(logDEBUG1, ("Setting Trigger, Register:0x%x\n", cmd));
    if (!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),
                                     DAQ_REG_CHIP_CMDS, cmd, 0, 0)) {
        LOG(logERROR, ("Could not give software trigger\n"));
        return 0;
    }
    // unset trigger bit
    LOG(logDEBUG1, ("Unsetting Trigger, Register:0x%x\n", orig_value));
    if (!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),
                                     DAQ_REG_CHIP_CMDS, orig_value, 0, 0)) {
        LOG(logERROR, ("Could not give software trigger\n"));
        return 0;
    }
    LOG(logDEBUG1, ("Software Internal Trigger Sent!\n"));
    return 1;
}

int Feb_Control_SoftwareTrigger(int block) {
    if (Feb_Control_activated) {
        // cant read reg
        int readyForTrigger = 0;
        if (!Feb_Control_IsReadyForTrigger(&readyForTrigger)) {
            LOG(logERROR, ("Could not read FEB_REG_STATUS reg!\n"));
            return 0;
        }
        // if not ready for trigger, throw
        if (!readyForTrigger) {
            LOG(logWARNING, ("Not yet ready for trigger!\n"));
            return 0;
        }

        // send trigger to both fpgas
        Feb_Control_SendSoftwareTrigger();

        // wait for next trigger ready
        if (block) {
            LOG(logDEBUG1, ("Blocking Software Trigger\n"));
            int readyForTrigger = 0;
            if (!Feb_Control_IsReadyForTrigger(&readyForTrigger)) {
                LOG(logERROR, ("Could not read FEB_REG_STATUS reg after giving "
                               "trigger!\n"));
                return 0;
            }

            while (!readyForTrigger) {
                // end of acquisition (cannot monitor readyForTrigger)
                int status = Feb_Control_AcquisitionInProgress();
                if (status == STATUS_ERROR) {
                    LOG(logERROR,
                        ("Status: ERROR reading DAQ status register\n"));
                    return 0;
                } else if (status == STATUS_IDLE) {
                    break;
                }

                usleep(5000);
                if (!Feb_Control_IsReadyForTrigger(&readyForTrigger)) {
                    LOG(logERROR, ("Could not read FEB_REG_STATUS reg after "
                                   "giving trigger!\n"));
                    return 0;
                }
            }
            LOG(logDEBUG2, ("Done waiting (wait for trigger)!\n"));
        }
        LOG(logINFO,
            ("%s Software Trigger %s\n", (block ? "Blocking" : "Non blocking"),
             (block ? "Acquired" : "Sent")));
        fflush(stdout);
    }

    return 1;
}

// parameters
int Feb_Control_SetDynamicRange(int dr) {
    static unsigned int everything_but_bit_mode = DAQ_STATIC_BIT_PROGRAM |
                                                  DAQ_STATIC_BIT_CHIP_TEST |
                                                  DAQ_STATIC_BIT_ROTEST;
    switch (dr) {
    case 4:
        Feb_Control_staticBits =
            DAQ_STATIC_BIT_M4 |
            (Feb_Control_staticBits &
             everything_but_bit_mode); // leave test bits in currernt state
        Feb_Control_subFrameMode &= ~DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING;
        break;
    case 8:
        Feb_Control_staticBits = DAQ_STATIC_BIT_M8 | (Feb_Control_staticBits &
                                                      everything_but_bit_mode);
        Feb_Control_subFrameMode &= ~DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING;
        break;
    case 12:
    case 16:
        Feb_Control_staticBits = DAQ_STATIC_BIT_M12 | (Feb_Control_staticBits &
                                                       everything_but_bit_mode);
        Feb_Control_subFrameMode &= ~DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING;

        // disable 16 bit conversion if 12 bit mode  (enable if 16 bit)
        if (!Feb_Control_Disable16bitConversion(dr == 12))
            return 0;
        break;
    case 32:
        Feb_Control_staticBits = DAQ_STATIC_BIT_M12 | (Feb_Control_staticBits &
                                                       everything_but_bit_mode);
        Feb_Control_subFrameMode |= DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING;
        break;
    default:
        LOG(logERROR,
            ("dynamic range (%d) not valid, not setting bit mode.\n", dr));
        LOG(logINFO, ("Set dynamic range int must equal 4,8 16, or 32.\n"));
        return 0;
    }

    LOG(logINFO, ("Dynamic range set to %d\n", dr));
    return 1;
}

int Feb_Control_GetDynamicRange(int *retval) {
    if (Feb_Control_subFrameMode & DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING) {
        *retval = 32;
    } else if (DAQ_STATIC_BIT_M4 & Feb_Control_staticBits) {
        *retval = 4;
    } else if (DAQ_STATIC_BIT_M8 & Feb_Control_staticBits) {
        *retval = 8;
    } else {
        int disable16 = 0;
        if (!Feb_Control_Get16bitConversionDisabled(&disable16)) {
            LOG(logERROR, ("Could not get dynamic range (12 or 16 bit)\n"));
            return 0;
        }
        if (disable16) {
            *retval = 12;
        } else {
            *retval = 16;
        }
    }

    return 1;
}

int Feb_Control_Disable16bitConversion(int disable) {
    LOG(logINFO, ("%s 16 bit expansion\n", disable ? "Disabling" : "Enabling"));

    uint32_t bitmask = DAQ_REG_HRDWRE_DSBL_16BIT_MSK;
    unsigned int regval = 0;
    if (disable) {
        regval |= bitmask;
    } else {
        regval &= ~bitmask;
    }

    if (!Feb_Control_WriteRegister_BitMask(DAQ_REG_HRDWRE, regval, bitmask,
                                           1)) {
        LOG(logERROR, ("Could not %s 16 bit expansion (bit mode)\n",
                       (disable ? "disable" : "enable")));
        return 0;
    }
    return 1;
}

int Feb_Control_Get16bitConversionDisabled(int *ret) {
    unsigned int regval = 0;
    if (!Feb_Control_ReadRegister_BitMask(DAQ_REG_HRDWRE, &regval,
                                          DAQ_REG_HRDWRE_DSBL_16BIT_MSK)) {
        LOG(logERROR, ("Could not get 16 bit expansion (bit mode)\n"));
        return 0;
    }
    if (regval) {
        *ret = 1;
    } else {
        *ret = 0;
    }
    return 1;
}

int Feb_Control_SetReadoutSpeed(unsigned int readout_speed) {
    // 0->full,1->half,2->quarter or 3->super_slow
    Feb_Control_acquireNReadoutMode &= (~DAQ_CHIP_CONTROLLER_SUPER_SLOW_SPEED);
    if (readout_speed == 1) {
        Feb_Control_acquireNReadoutMode |= DAQ_CHIP_CONTROLLER_HALF_SPEED;
        LOG(logINFO, ("Speed set to half speed (50 MHz)\n"));
    } else if (readout_speed == 2) {
        Feb_Control_acquireNReadoutMode |= DAQ_CHIP_CONTROLLER_QUARTER_SPEED;
        LOG(logINFO, ("Speed set to quarter speed (25 MHz)\n"));
    } else {
        if (readout_speed) {
            LOG(logERROR,
                ("readout speed %d unknown, defaulting to full speed.\n",
                 readout_speed));
            LOG(logINFO, ("full speed, (100 MHz)\n"));
            return 0;
        }
        LOG(logINFO, ("Speed set to full speed (100 MHz)\n"));
    }
    return 1;
}

int Feb_Control_SetReadoutMode(unsigned int readout_mode) {
    // 0->parallel,1->non-parallel,2-> safe_mode
    Feb_Control_acquireNReadoutMode &= (~DAQ_NEXPOSURERS_PARALLEL_MODE);
    if (readout_mode == 1) {
        Feb_Control_acquireNReadoutMode |=
            DAQ_NEXPOSURERS_NORMAL_NONPARALLEL_MODE;
        LOG(logINFO, ("Readout mode set to Non Parallel\n"));
    } else if (readout_mode == 2) {
        Feb_Control_acquireNReadoutMode |=
            DAQ_NEXPOSURERS_SAFEST_MODE_ROW_CLK_BEFORE_MODE;
        LOG(logINFO, ("Readout mode set to Safe (row clk before main clk "
                      "readout sequence)\n"));
    } else {
        Feb_Control_acquireNReadoutMode |= DAQ_NEXPOSURERS_PARALLEL_MODE;
        if (readout_mode) {
            LOG(logERROR,
                ("readout mode %d) unknown, defaulting to parallel readout.\n",
                 readout_mode));
            LOG(logINFO, ("Readout mode set to Parallel\n"));
            return 0;
        }
        LOG(logINFO, ("Readout mode set to Parallel\n"));
    }

    return 1;
}

int Feb_Control_SetTriggerMode(unsigned int trigger_mode) {
    //"00"-> internal exposure time and period,
    //"01"-> external acquistion start and internal exposure time and period,
    //"10"-> external start trigger and internal exposure time,
    //"11"-> external triggered start and stop of exposures
    Feb_Control_triggerMode = (~DAQ_NEXPOSURERS_EXTERNAL_IMAGE_START_AND_STOP);

    if (trigger_mode == 1) {
        Feb_Control_triggerMode = DAQ_NEXPOSURERS_EXTERNAL_ACQUISITION_START;
        LOG(logINFO, ("Trigger mode set to Burst Trigger\n"));
    } else if (trigger_mode == 2) {
        Feb_Control_triggerMode = DAQ_NEXPOSURERS_EXTERNAL_IMAGE_START;
        LOG(logINFO, ("Trigger mode set to Trigger Exposure\n"));
    } else if (trigger_mode == 3) {
        Feb_Control_triggerMode = DAQ_NEXPOSURERS_EXTERNAL_IMAGE_START_AND_STOP;
        LOG(logINFO, ("Trigger mode set to Gated\n"));
    } else {
        Feb_Control_triggerMode = DAQ_NEXPOSURERS_INTERNAL_ACQUISITION;
        if (trigger_mode) {
            LOG(logERROR,
                ("trigger %d) unknown, defaulting to Auto\n", trigger_mode));
        }

        LOG(logINFO, ("Trigger mode set to Auto\n"));
        return trigger_mode == 0;
    }
    // polarity
    int polarity = 1; // hard coded (can be configured in future)
    if (polarity) {
        Feb_Control_triggerMode |= DAQ_NEXPOSURERS_EXTERNAL_TRIGGER_POLARITY;
        LOG(logINFO, ("External trigger polarity set to positive\n"));
    } else {
        Feb_Control_triggerMode &= (~DAQ_NEXPOSURERS_EXTERNAL_TRIGGER_POLARITY);
        LOG(logINFO, ("External trigger polarity set to negitive\n"));
    }
    return 1;
}

int Feb_Control_SetExternalEnableMode(int use_external_enable, int polarity) {
    if (use_external_enable) {
        Feb_Control_externalEnableMode |= DAQ_NEXPOSURERS_EXTERNAL_ENABLING;
        if (polarity) {
            Feb_Control_externalEnableMode |=
                DAQ_NEXPOSURERS_EXTERNAL_ENABLING_POLARITY;
        } else {
            Feb_Control_externalEnableMode &=
                (~DAQ_NEXPOSURERS_EXTERNAL_ENABLING_POLARITY);
        }
        LOG(logINFO, ("External enabling enabled, polarity set to %s\n",
                      (polarity ? "positive" : "negative")));

    } else {
        Feb_Control_externalEnableMode = 0;
        LOG(logINFO, ("External enabling disabled\n"));
    }
    return 1;
}

int Feb_Control_SetNExposures(unsigned int n_images) {
    if (!n_images) {
        LOG(logERROR, ("nimages must be greater than zero.%d\n", n_images));
        return 0;
    }
    Feb_Control_nimages = n_images;
    LOG(logDEBUG1, ("Number of images set to %d\n", Feb_Control_nimages));
    return 1;
}
unsigned int Feb_Control_GetNExposures() { return Feb_Control_nimages; }

int Feb_Control_SetExposureTime(double the_exposure_time_in_sec) {
    Feb_Control_exposure_time_in_sec = the_exposure_time_in_sec;
    LOG(logDEBUG1,
        ("Exposure time set to %fs\n", Feb_Control_exposure_time_in_sec));
    return 1;
}

double Feb_Control_GetExposureTime() {
    return Feb_Control_exposure_time_in_sec;
}

int64_t Feb_Control_GetExposureTime_in_nsec() {
    return (int64_t)(Feb_Control_exposure_time_in_sec * (1E9));
}

int Feb_Control_SetSubFrameExposureTime(
    int64_t the_subframe_exposure_time_in_10nsec) {
    Feb_Control_subframe_exposure_time_in_10nsec =
        the_subframe_exposure_time_in_10nsec;
    LOG(logDEBUG1,
        ("Sub Frame Exposure time set to %lldns\n",
         (long long int)Feb_Control_subframe_exposure_time_in_10nsec * 10));
    return 1;
}

int64_t Feb_Control_GetSubFrameExposureTime() {
    return Feb_Control_subframe_exposure_time_in_10nsec * 10;
}

int Feb_Control_SetSubFramePeriod(int64_t the_subframe_period_in_10nsec) {
    Feb_Control_subframe_period_in_10nsec = the_subframe_period_in_10nsec;
    LOG(logDEBUG1, ("Sub Frame Period set to %lldns\n",
                    (long long int)Feb_Control_subframe_period_in_10nsec * 10));
    return 1;
}
int64_t Feb_Control_GetSubFramePeriod() {
    return Feb_Control_subframe_period_in_10nsec * 10;
}

int Feb_Control_SetExposurePeriod(double the_exposure_period_in_sec) {
    Feb_Control_exposure_period_in_sec = the_exposure_period_in_sec;
    LOG(logDEBUG1,
        ("Exposure period set to %fs\n", Feb_Control_exposure_period_in_sec));
    return 1;
}
double Feb_Control_GetExposurePeriod() {
    return Feb_Control_exposure_period_in_sec;
}

void Feb_Control_Set_Counter_Bit(int value) { Feb_Control_counter_bit = value; }

int Feb_Control_Get_Counter_Bit() { return Feb_Control_counter_bit; }

int Feb_Control_SetInterruptSubframe(int val) {
    LOG(logINFO, ("Setting Interrupt Subframe to %d\n", val));
    // they need to be written separately because the left and right registers
    // have different values for this particular register
    uint32_t offset = DAQ_REG_HRDWRE;
    uint32_t regVal = 0;
    char side[2][10] = {"right", "left"};
    unsigned int addr[2] = {Feb_Control_rightAddress, Feb_Control_leftAddress};
    for (int iloop = 0; iloop < 2; ++iloop) {
        // get previous value to keep it
        if (!Feb_Interface_ReadRegister(addr[iloop], offset, &regVal)) {
            LOG(logERROR,
                ("Could not read %s interrupt subframe\n", side[iloop]));
            return 0;
        }
        uint32_t data = ((val == 0) ? (regVal & ~DAQ_REG_HRDWRE_INTRRPT_SF_MSK)
                                    : (regVal | DAQ_REG_HRDWRE_INTRRPT_SF_MSK));
        if (!Feb_Interface_WriteRegister(addr[iloop], offset, data, 0, 0)) {
            LOG(logERROR,
                ("Could not write 0x%x to %s interrupt subframe addr 0x%x\n",
                 data, side[iloop], offset));
            return 0;
        }
    }
    return 1;
}

int Feb_Control_GetInterruptSubframe() {
    // they need to be written separately because the left and right registers
    // have different values for this particular register
    uint32_t offset = DAQ_REG_HRDWRE;
    uint32_t regVal = 0;

    char side[2][10] = {"right", "left"};
    unsigned int addr[2] = {Feb_Control_rightAddress, Feb_Control_leftAddress};
    uint32_t value[2] = {0, 0};
    for (int iloop = 0; iloop < 2; ++iloop) {
        if (!Feb_Interface_ReadRegister(addr[iloop], offset, &regVal)) {
            LOG(logERROR,
                ("Could not read back %s interrupt subframe\n", side[iloop]));
            return -1;
        }
        value[iloop] = (regVal & DAQ_REG_HRDWRE_INTRRPT_SF_MSK) >>
                       DAQ_REG_HRDWRE_INTRRPT_SF_OFST;
    }
    // inconsistent
    if (value[0] != value[1]) {
        LOG(logERROR, ("Inconsistent values of interrupt subframe betweeen "
                       "right %d and left %d\n",
                       value[0], value[1]));
        return -1;
    }
    return value[0];
}

int Feb_Control_SetTop(enum TOPINDEX ind, int left, int right) {
    uint32_t offset = DAQ_REG_HRDWRE;
    unsigned int addr[2] = {0, 0};
    if (left) {
        addr[0] = Feb_Control_leftAddress;
    }
    if (right) {
        addr[1] = Feb_Control_rightAddress;
    }
    char *top_names[] = {TOP_NAMES};
    for (int i = 0; i < 2; ++i) {
        if (addr[i] == 0) {
            continue;
        }
        uint32_t value = 0;
        if (!Feb_Interface_ReadRegister(addr[i], offset, &value)) {
            LOG(logERROR, ("Could not read %s Feb reg to set Top flag\n",
                           (i == 0 ? "left" : "right")));
            return 0;
        }
        switch (ind) {
        case TOP_HARDWARE:
            value &= ~DAQ_REG_HRDWRE_OW_TOP_MSK;
            break;
        case OW_TOP:
            value |= DAQ_REG_HRDWRE_OW_TOP_MSK;
            value |= DAQ_REG_HRDWRE_TOP_MSK;
            break;
        case OW_BOTTOM:
            value |= DAQ_REG_HRDWRE_OW_TOP_MSK;
            value &= ~DAQ_REG_HRDWRE_TOP_MSK;
            break;
        default:
            LOG(logERROR, ("Unknown top index in Feb: %d\n", ind));
            return 0;
        }
        if (!Feb_Interface_WriteRegister(addr[i], offset, value, 0, 0)) {
            LOG(logERROR, ("Could not set Top flag to %s in %s Feb\n",
                           top_names[ind], (i == 0 ? "left" : "right")));
            return 0;
        }
    }
    if (left && right) {
        LOG(logINFOBLUE, ("%s Top flag to %s Feb\n",
                          (ind == TOP_HARDWARE ? "Resetting" : "Overwriting"),
                          top_names[ind]));
    }
    return 1;
}

int Feb_Control_SetMaster(enum MASTERINDEX ind) {

    uint32_t offset = DAQ_REG_HRDWRE;
    unsigned int addr[2] = {Feb_Control_leftAddress, Feb_Control_rightAddress};
    char *master_names[] = {MASTER_NAMES};
    for (int i = 0; i < 2; ++i) {
        uint32_t value = 0;
        if (!Feb_Interface_ReadRegister(addr[i], offset, &value)) {
            LOG(logERROR, ("Could not read %s Feb reg to set Master flag\n",
                           (i == 0 ? "left" : "right")));
            return 0;
        }
        switch (ind) {
        case MASTER_HARDWARE:
            value &= ~DAQ_REG_HRDWRE_OW_MASTER_MSK;
            break;
        case OW_MASTER:
            value |= DAQ_REG_HRDWRE_OW_MASTER_MSK;
            value |= DAQ_REG_HRDWRE_MASTER_MSK;
            break;
        case OW_SLAVE:
            value |= DAQ_REG_HRDWRE_OW_MASTER_MSK;
            value &= ~DAQ_REG_HRDWRE_MASTER_MSK;
            break;
        default:
            LOG(logERROR, ("Unknown master index in Feb: %d\n", ind));
            return 0;
        }

        if (!Feb_Interface_WriteRegister(addr[i], offset, value, 0, 0)) {
            LOG(logERROR, ("Could not set Master flag to %s in %s Feb\n",
                           master_names[ind], (i == 0 ? "left" : "right")));
            return 0;
        }
    }
    LOG(logINFOBLUE, ("%s Master flag to %s Feb\n",
                      (ind == MASTER_HARDWARE ? "Resetting" : "Overwriting"),
                      master_names[ind]));

    return 1;
}

int Feb_Control_SetMasterEffects(int master, int controlServer) {
    int prevMaster = Feb_Control_master;

    Feb_Control_master = master;
    // change in master for 9m
    if (controlServer && prevMaster != Feb_Control_master &&
        !Feb_Control_normal) {
        if (prevMaster) {
            Feb_Control_CloseSerialCommunication();
        }
        if (Feb_Control_master) {
            if (!Feb_Control_OpenSerialCommunication()) {
                LOG(logERROR, ("Could not intitalize feb control serial "
                               "communication\n"));
                return FAIL;
            }
        }
    }
    return OK;
}

int Feb_Control_SetQuad(int val) {
    LOG(logINFO, ("Setting Quad to %d in Feb\n", val));
    Feb_Control_quadMode = val;
    // only setting on the right feb if quad
    return Feb_Control_SetTop(val == 0 ? TOP_HARDWARE : OW_BOTTOM, 0, 1);
}

int Feb_Control_SetChipSignalsToTrimQuad(int enable) {
    if (Feb_Control_quadMode) {
        LOG(logINFO, ("%s chip signals to trim quad\n",
                      enable ? "Enabling" : "Disabling"));
        unsigned int regval = 0;
        // right fpga only
        uint32_t righOffset = DAQ_REG_HRDWRE + Feb_Control_rightAddress;
        if (!Feb_Control_ReadRegister(righOffset, &regval)) {
            LOG(logERROR, ("Could not set chip signals to trim quad\n"));
            return 0;
        }
        if (enable) {
            regval |= (DAQ_REG_HRDWRE_PROGRAM_MSK | DAQ_REG_HRDWRE_M8_MSK);
        } else {
            regval &= ~(DAQ_REG_HRDWRE_PROGRAM_MSK | DAQ_REG_HRDWRE_M8_MSK);
        }

        if (!Feb_Control_WriteRegister(righOffset, regval, 1)) {
            LOG(logERROR, ("Could not set chip signals to trim quad\n"));
            return 0;
        }
    }
    return 1;
}

int Feb_Control_SetReadNRows(int value) {
    LOG(logINFO, ("Setting number of rows to %d\n", value));
    if (!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),
                                     DAQ_REG_READ_N_ROWS, value, 0, 0)) {
        LOG(logERROR, ("Could not write %d to number of rows reg\n", value));
        return 0;
    }
    return 1;
}

int Feb_Control_GetReadNRows() {
    uint32_t regVal = 0;
    if (!Feb_Interface_ReadRegister(Feb_Control_AddressToAll(),
                                    DAQ_REG_READ_N_ROWS, &regVal)) {
        LOG(logERROR, ("Could not read back ReadNRows reg\n"));
        return -1;
    }
    LOG(logDEBUG1, ("Retval ReadNRows: %d\n", regVal));
    return regVal;
}

int Feb_Control_WriteRegister(uint32_t offset, uint32_t data, int validate) {
    if (!Feb_Control_WriteRegister_BitMask(offset, data, BIT32_MSK, validate))
        return 0;
    return 1;
}

int Feb_Control_ReadRegister(uint32_t offset, uint32_t *retval) {
    return Feb_Control_ReadRegister_BitMask(offset, retval, BIT32_MASK);
}

int Feb_Control_WriteRegister_BitMask(uint32_t offset, uint32_t data,
                                      uint32_t bitmask, int validate) {
    uint32_t actualOffset = offset;
    char side[2][10] = {"right", "left"};
    unsigned int addr[2] = {Feb_Control_rightAddress, Feb_Control_leftAddress};

    int run[2] = {0, 0};
    // both registers
    if (offset < Feb_Control_leftAddress) {
        run[0] = 1;
        run[1] = 1;
    }
    // right registers only
    else if (offset >= Feb_Control_rightAddress) {
        run[0] = 1;
        actualOffset = offset - Feb_Control_rightAddress;
    }
    // left registers only
    else {
        run[1] = 1;
        actualOffset = offset - Feb_Control_leftAddress;
    }

    for (int iloop = 0; iloop < 2; ++iloop) {
        if (run[iloop]) {
            LOG(logDEBUG1, ("Writing 0x%x to %s 0x%x (mask:0x%x)\n", data,
                            side[iloop], actualOffset, bitmask));

            uint32_t writeVal = 0;
            if (!Feb_Interface_ReadRegister(addr[iloop], actualOffset,
                                            &writeVal)) {
                LOG(logERROR, ("Could not read %s addr 0x%x register\n",
                               side[iloop], actualOffset));
                return 0;
            }
            // set only the bits in the mask
            writeVal &= ~(bitmask);
            writeVal |= (data & bitmask);

            LOG(logDEBUG1, ("writing 0x%x to 0x%x\n", writeVal, actualOffset));
            if (!Feb_Interface_WriteRegister(addr[iloop], actualOffset,
                                             writeVal, 0, 0)) {
                LOG(logERROR, ("Could not write 0x%x to %s addr 0x%x\n",
                               writeVal, side[iloop], actualOffset));
                return 0;
            }

            if (validate) {

                uint32_t readVal = 0;
                if (!Feb_Interface_ReadRegister(addr[iloop], actualOffset,
                                                &readVal)) {
                    return 0;
                }
                readVal &= bitmask;
                writeVal &= bitmask;
                if (writeVal != readVal) {
                    LOG(logERROR,
                        ("Could not write %s addr 0x%x register. Wrote "
                         "0x%x, read 0x%x (mask:0x%x)\n",
                         side[iloop], actualOffset, writeVal, readVal,
                         bitmask));
                    return 0;
                }
            }
        }
    }

    return 1;
}

int Feb_Control_ReadRegister_BitMask(uint32_t offset, uint32_t *retval,
                                     uint32_t bitmask) {

    uint32_t actualOffset = offset;
    char side[2][10] = {"right", "left"};
    unsigned int addr[2] = {Feb_Control_rightAddress, Feb_Control_leftAddress};
    uint32_t value[2] = {0, 0};
    int run[2] = {0, 0};
    // both registers
    if (offset < Feb_Control_leftAddress) {
        run[0] = 1;
        run[1] = 1;
    }
    // right registers only
    else if (offset >= Feb_Control_rightAddress) {
        run[0] = 1;
        actualOffset = offset - Feb_Control_rightAddress;
    }
    // left registers only
    else {
        run[1] = 1;
        actualOffset = offset - Feb_Control_leftAddress;
    }

    for (int iloop = 0; iloop < 2; ++iloop) {
        if (run[iloop]) {
            if (!Feb_Interface_ReadRegister(addr[iloop], actualOffset,
                                            &value[iloop])) {
                LOG(logERROR, ("Could not read from %s addr 0x%x\n",
                               side[iloop], actualOffset));
                return 0;
            }
            value[iloop] &= bitmask;
            LOG(logDEBUG1, ("Read 0x%x from %s 0x%x (mask:0x%x)\n",
                            value[iloop], side[iloop], actualOffset, bitmask));
            *retval = value[iloop];
            // if not the other (left, not right OR right, not left), return the
            // value
            if (!run[iloop ? 0 : 1]) {
                return 1;
            }
        }
    }
    // Inconsistent values when reading both registers
    if ((run[0] & run[1]) & (value[0] != value[1])) {
        LOG(logERROR, ("Inconsistent values read from %s: 0x%x and %s: 0x%x\n",
                       side[0], value[0], side[1], value[1]));
        return 0;
    }
    return 1;
}

// pulsing
int Feb_Control_Pulse_Pixel(int npulses, int x, int y) {
    // this function is not designed for speed
    int pulse_multiple = 0; // has to be 0 or 1

    if (x < 0) {
        x = -x;
        pulse_multiple = 1;
        LOG(logINFO,
            ("Pulsing pixel %d in all super columns below number %d.\n", x % 8,
             x / 8));
    }
    if (x < 0 || x > 255 || y < 0 || y > 255) {
        LOG(logERROR, ("Pixel out of range.\n"));
        return 0;
    }

    //  y = 255 - y;
    int nrowclocks = 0;
    nrowclocks += (Feb_Control_staticBits & DAQ_STATIC_BIT_M4) ? 0 : 2 * y;
    nrowclocks += (Feb_Control_staticBits & DAQ_STATIC_BIT_M8) ? 0 : y;

    Feb_Control_SetInTestModeVariable(1); // on
    Feb_Control_SetStaticBits();
    Feb_Control_SetCommandRegister(DAQ_RESET_PERIPHERY |
                                   DAQ_RESET_COLUMN_SELECT);
    if (Feb_Control_StartDAQOnlyNWaitForFinish(5000) != STATUS_IDLE) {
        LOG(logERROR, ("could not pulse pixel as status not idle\n"));
        return 0;
    }

    unsigned int serial_in = 8 << (4 * (7 - x % 8));
    if (!Feb_Control_Shift32InSerialIn(serial_in)) {
        LOG(logERROR,
            ("ChipController::PulsePixel: could shift in the initail 32.\n"));
        return 0;
    }

    if (!pulse_multiple)
        serial_in = 0;
    for (int i = 0; i < x / 8; i++)
        Feb_Control_Shift32InSerialIn(serial_in);

    Feb_Control_SendTokenIn();
    Feb_Control_ClockRowClock(nrowclocks);
    Feb_Control_PulsePixelNMove(npulses, 0, 0);
    return 1;
}

int Feb_Control_PulsePixelNMove(int npulses, int inc_x_pos, int inc_y_pos) {
    unsigned int c = DAQ_SEND_N_TEST_PULSES;
    c |= (inc_x_pos) ? DAQ_CLK_MAIN_CLK_TO_SELECT_NEXT_PIXEL : 0;
    c |= (inc_y_pos) ? DAQ_CLK_ROW_CLK_TO_SELECT_NEXT_ROW : 0;

    if (Feb_Control_activated) {
        if (!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),
                                         DAQ_REG_SEND_N_TESTPULSES, npulses, 0,
                                         0) ||
            !Feb_Control_SetCommandRegister(c) ||
            (Feb_Control_StartDAQOnlyNWaitForFinish(5000) != STATUS_IDLE)) {
            LOG(logERROR, ("could not PulsePixelNMove(...).\n"));
            return 0;
        }
    }
    return 1;
}

int Feb_Control_Shift32InSerialIn(unsigned int value_to_shift_in) {
    if (Feb_Control_activated) {
        if (!Feb_Control_SetCommandRegister(DAQ_SERIALIN_SHIFT_IN_32) ||
            !Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),
                                         DAQ_REG_SHIFT_IN_32, value_to_shift_in,
                                         0, 0) ||
            (Feb_Control_StartDAQOnlyNWaitForFinish(5000) != STATUS_IDLE)) {
            LOG(logERROR, ("could not shift in 32.\n"));
            return 0;
        }
    }
    return 1;
}

int Feb_Control_SendTokenIn() {
    if (!Feb_Control_SetCommandRegister(DAQ_SEND_A_TOKEN_IN) ||
        (Feb_Control_StartDAQOnlyNWaitForFinish(5000) != STATUS_IDLE)) {
        LOG(logERROR, ("could not SendTokenIn().\n"));
        return 0;
    }
    return 1;
}

int Feb_Control_ClockRowClock(unsigned int ntimes) {
    if (ntimes > 1023) {
        LOG(logERROR, ("Clock row clock ntimes (%d) exceeds the maximum value "
                       "of 1023.\n\t Setting ntimes to 1023.\n",
                       ntimes));
        ntimes = 1023;
    }
    if (Feb_Control_activated) {
        if (!Feb_Control_SetCommandRegister(DAQ_CLK_ROW_CLK_NTIMES) ||
            !Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),
                                         DAQ_REG_CLK_ROW_CLK_NTIMES, ntimes, 0,
                                         0) ||
            (Feb_Control_StartDAQOnlyNWaitForFinish(5000) != STATUS_IDLE)) {
            LOG(logERROR, ("could not clock row clock.\n"));
            return 0;
        }
    }
    return 1;
}

int Feb_Control_PulseChip(int npulses) {
    int on = 1;
    if (npulses == -1) {
        on = 0;
        LOG(logINFO, ("\nResetting to normal mode\n"));
    } else {
        LOG(logINFO, ("\n\nPulsing Chip.\n")); // really just toggles the enable
        LOG(logINFO, ("Vcmp should be set to 2.0 and Vtrim should be 2.\n"));
    }

    Feb_Control_SetInTestModeVariable(on);
    Feb_Control_SetStaticBits(); // toggle the enable 2x times
    Feb_Control_ResetChipCompletely();

    for (int i = 0; i < npulses; i++) {
        if (!Feb_Control_SetCommandRegister(
                DAQ_CHIP_CONTROLLER_SUPER_SLOW_SPEED | DAQ_RESET_PERIPHERY |
                DAQ_RESET_COLUMN_SELECT)) {
            LOG(logERROR, ("some set command register error\n"));
        }
        if ((Feb_Control_StartDAQOnlyNWaitForFinish(5000) != STATUS_IDLE)) {
            LOG(logERROR, ("some wait error\n"));
        }
    }
    Feb_Control_SetExternalEnableMode(on, 1);
    Feb_Control_counter_bit = (on ? 0 : 1);
    LOG(logINFO, ("Feb_Control_counter_bit:%d\n", Feb_Control_counter_bit));
    if (on) {
        LOG(logINFO, ("Pulse chip success\n\n"));
    } else {
        LOG(logINFO, ("Reset to normal mode success\n\n"));
    }
    return 1;
}

// rate correction
int64_t Feb_Control_Get_RateTable_Tau_in_nsec() {
    return Feb_Control_RateTable_Tau_in_nsec;
}

int64_t Feb_Control_Get_RateTable_Period_in_nsec() {
    return Feb_Control_RateTable_Period_in_nsec;
}

int Feb_Control_SetRateCorrectionTau(int64_t tau_in_Nsec) {
    // period = exptime if 16bit, period = subexptime if 32 bit
    int dr = 0;
    if (!Feb_Control_GetDynamicRange(&dr)) {
        LOG(logERROR, ("Could not set rate correction tau\n"));
        return 0;
    }
    double period_in_sec =
        (double)(Feb_Control_GetSubFrameExposureTime()) / (double)1e9;
    if (dr == 16)
        period_in_sec = Feb_Control_GetExposureTime();

    double tau_in_sec = (double)tau_in_Nsec / (double)1e9;
    LOG(logINFO, (" tau %lf %lf ", (double)tau_in_Nsec, (double)tau_in_sec));

    unsigned int np = 16384; // max slope 16 * 1024
    double b0[1024];
    double m[1024];

    if (tau_in_sec < 0 || period_in_sec < 0) {
        if (dr == 32) {
            LOG(logERROR,
                ("tau %lf and sub_exposure_time %lf must be greater than 0.\n",
                 tau_in_sec, period_in_sec));
        } else {
            LOG(logERROR,
                ("tau %lf and exposure_time %lf must be greater than 0.\n",
                 tau_in_sec, period_in_sec));
        }
        return 0;
    }

    LOG(logINFO, ("Changing Rate Correction Table tau:%0.8f sec, period:%f sec",
                  tau_in_sec, period_in_sec));
    LOG(logINFO, ("\tCalculating table for tau of %lld ns.\n", tau_in_Nsec));
    for (int i = 0; i < np; i++) {
        Feb_Control_rate_meas[i] = i * exp(-i / period_in_sec * tau_in_sec);
        if (Feb_Control_rate_meas[i] > ratemax)
            ratemax = Feb_Control_rate_meas[i];
    }

    /*
      b  :  index/address of block ram/rate correction table
      b0 :  base in vhdl
      m  :  slope in vhdl

      Firmware:
      data_in(11..2) -> memory address  --> memory
      data_in( 1..0) -> lsb

      mem_data_out(13.. 0) -> base
      mem_data_out(17..14) -> slope

      delta = slope*lsb
      corr  = base+delta
     */

    int next_i = 0;
    double beforemax;
    b0[0] = 0;
    m[0] = 1;

    Feb_Control_rate_correction_table[0] =
        (((int)(m[0] + 0.5) & 0xf) << 14) | ((int)(b0[0] + 0.5) & 0x3fff);

    for (int b = 1; b < 1024; b++) {
        if (m[b - 1] < 15) {
            double s = 0, sx = 0, sy = 0, sxx = 0, sxy = 0;
            for (;; next_i++) {
                if (next_i >= np) {
                    for (; b < 1024; b++) {
                        if (beforemax > ratemax)
                            b0[b] = beforemax;
                        else
                            b0[b] = ratemax;
                        m[b] = 15;
                        Feb_Control_rate_correction_table[b] =
                            (((int)(m[b] + 0.5) & 0xf) << 14) |
                            ((int)(b0[b] + 0.5) & 0x3fff);
                    }
                    b = 1024;
                    break;
                }

                double x = Feb_Control_rate_meas[next_i] - b * 4;
                double y = next_i;
                /*LOG(logDEBUG1, ("Start Loop  x: %f,\t y: %f,\t  s: %f,\t  sx:
            %f,\t  sy: %f,\t  sxx: %f,\t  sxy: %f,\t  " "next_i: %d,\t  b: %d,\t
            Feb_Control_rate_meas[next_i]: %f\n", x, y, s, sx, sy, sxx, sxy,
            next_i, b, Feb_Control_rate_meas[next_i]));*/

                if (x < -0.5)
                    continue;
                if (x > 3.5)
                    break;
                s += 1;
                sx += x;
                sy += y;
                sxx += x * x;
                sxy += x * y;
                /*LOG(logDEBUG1, ("End   Loop  x: %f,\t y: %f,\t  s: %f,\t  sx:
            %f,\t  sy: %f,\t  sxx: %f,\t  sxy: %f,\t  " "next_i: %d,\t  b: %d,\t
            Feb_Control_rate_meas[next_i]: %f\n", x, y, s, sx, sy, sxx, sxy,
            next_i, b, Feb_Control_rate_meas[next_i]));*/
            }
            double delta = s * sxx - sx * sx;
            b0[b] = (sxx * sy - sx * sxy) / delta;
            m[b] = (s * sxy - sx * sy) / delta;
            beforemax = b0[b];

            if (m[b] < 0 || m[b] > 15) {
                m[b] = 15;
                if (beforemax > ratemax)
                    b0[b] = beforemax;
                else
                    b0[b] = ratemax;
            }
            /*LOG(logDEBUG1, ("After Loop  s: %f,\t  sx: %f,\t  sy: %f,\t  sxx:
              %f,\t  sxy: %f,\t  " "next_i: %d,\t  b: %d,\t
              Feb_Control_rate_meas[next_i]: %f\n",
              s, sx, sy, sxx, sxy, next_i, b, Feb_Control_rate_meas[next_i]));*/
            //	cout<<s<<"   "<<sx<<"   "<<sy<<"   "<<sxx<<"   "<<"   "<<sxy<<"
            //"<<delta<<"   "<<m[b]<<"    "<<b0[b]<<endl;
        } else {
            if (beforemax > ratemax)
                b0[b] = beforemax;
            else
                b0[b] = ratemax;
            m[b] = 15;
        }
        Feb_Control_rate_correction_table[b] =
            (((int)(m[b] + 0.5) & 0xf) << 14) | ((int)(b0[b] + 0.5) & 0x3fff);
        /*LOG(logDEBUG1, ("After Loop  4*b: %d\tbase:%d\tslope:%d\n",4*b,
         * (int)(b0[b]+0.5), (int)(m[b]+0.5) ));*/
    }

    if (Feb_Control_SetRateCorrectionTable(Feb_Control_rate_correction_table)) {
        Feb_Control_RateTable_Tau_in_nsec = tau_in_Nsec;
        Feb_Control_RateTable_Period_in_nsec = period_in_sec * 1e9;
        return 1;
    } else {
        Feb_Control_RateTable_Tau_in_nsec = -1;
        Feb_Control_RateTable_Period_in_nsec = -1;
        return 0;
    }
}

int Feb_Control_SetRateCorrectionTable(unsigned int *table) {
    if (!table) {
        LOG(logERROR,
            ("Error: could not set rate correction table, point is zero.\n"));
        Feb_Control_SetRateCorrectionVariable(0);
        return 0;
    }

    LOG(logINFO, ("Setting rate correction table. %d %d %d %d ....\n", table[0],
                  table[1], table[2], table[3]));

    // was added otherwise after an acquire, startdaqonlywatiforfinish waits
    // forever
    if (!Feb_Control_SetCommandRegister(DAQ_RESET_COMPLETELY)) {
        LOG(logERROR, ("Could not Feb_Control_SetCommandRegister for loading "
                       "trim bits.\n"));
        return 0;
    }
    LOG(logINFO, ("daq reset completely\n"));

    if (Feb_Control_activated) {
        if (!Feb_Interface_WriteMemoryInLoops(
                Feb_Control_leftAddress, 1, 0, 1024,
                Feb_Control_rate_correction_table) ||
            !Feb_Interface_WriteMemoryInLoops(
                Feb_Control_rightAddress, 1, 0, 1024,
                Feb_Control_rate_correction_table) ||
            (Feb_Control_StartDAQOnlyNWaitForFinish(5000) != STATUS_IDLE)) {
            LOG(logERROR, ("could not write to memory (top) "
                           "::Feb_Control_SetRateCorrectionTable\n"));
            return 0;
        }
    }
    return 1;
}

int Feb_Control_GetRateCorrectionVariable() {
    return (Feb_Control_subFrameMode &
            DAQ_NEXPOSURERS_ACTIVATE_RATE_CORRECTION);
}

void Feb_Control_SetRateCorrectionVariable(int activate_rate_correction) {
    if (activate_rate_correction) {
        Feb_Control_subFrameMode |= DAQ_NEXPOSURERS_ACTIVATE_RATE_CORRECTION;
        LOG(logINFO, ("Rate correction activated\n"));
    } else {
        Feb_Control_subFrameMode &= ~DAQ_NEXPOSURERS_ACTIVATE_RATE_CORRECTION;
        LOG(logINFO, ("Rate correction deactivated\n"));
    }
}

int Feb_Control_PrintCorrectedValues() {
    int delta, slope, base, lsb, corr;
    for (int i = 0; i < 4096; i++) {
        lsb = i & 3;
        base = Feb_Control_rate_correction_table[i >> 2] & 0x3fff;
        slope = ((Feb_Control_rate_correction_table[i >> 2] & 0x3c000) >> 14);

        delta = slope * lsb;
        corr = delta + base;
        if (slope == 15)
            corr = 3 * slope + base;

        LOG(logDEBUG1,
            ("Readout Input: "
             "%d,\tBase:%d,\tSlope:%d,\tLSB:%d,\tDelta:%d\tResult:%d\tReal:%"
             "lf\n",
             i, base, slope, lsb, delta, corr, Feb_Control_rate_meas[i]));
    }
    return 1;
}

// adcs
// So if software says now 40.00 you neeed to convert to mdegrees 40000(call it
// A1) and then A1/65536/0.00198421639-273.15
int Feb_Control_GetLeftFPGATemp() {
    if (!Feb_Control_activated) {
        return 0;
    }
    unsigned int value = 0;
    if (!Feb_Interface_ReadRegister(Feb_Control_leftAddress, FEB_REG_STATUS,
                                    &value)) {
        LOG(logERROR, ("Trouble reading FEB_REG_STATUS reg to get left feb "
                       "temperature\n"));
        return 0;
    }
    unsigned int temperature =
        ((value & FEB_REG_STATUS_TEMP_MSK) >> FEB_REG_STATUS_TEMP_OFST);
    temperature =
        ((((float)(temperature) / 65536.0f) / 0.00198421639f) - 273.15f) *
        1000; // Static conversation, copied from xps sysmon standalone driver
    // division done in client to send int over network
    return (int)temperature;
}

int Feb_Control_GetRightFPGATemp() {
    if (!Feb_Control_activated) {
        return 0;
    }
    unsigned int temperature = 0;
    if (!Feb_Interface_ReadRegister(Feb_Control_rightAddress, FEB_REG_STATUS,
                                    &temperature)) {
        LOG(logERROR, ("Trouble reading FEB_REG_STATUS reg to get right feb "
                       "temperature\n"));
        return 0;
    }
    temperature = temperature >> 16;
    temperature =
        ((((float)(temperature) / 65536.0f) / 0.00198421639f) - 273.15f) *
        1000; // Static conversation, copied from xps sysmon standalone driver
    // division done in client to send int over network
    return (int)temperature;
}

int Feb_Control_GetFPGAHardwareVersion(int *retval) {
    if (!Feb_Control_activated) {
        return 0;
    }
    unsigned int value = 0;
    if (!Feb_Control_ReadRegister_BitMask(FEB_REG_STATUS, &value,
                                          FEB_REG_STATUS_FX30_MSK)) {
        LOG(logERROR,
            ("Trouble reading FEB_REG_STATUS reg to feb hardware version\n"));
        return 0;
    }
    *retval = (value >> FEB_REG_STATUS_FX30_OFST);
    return 1;
}

int64_t Feb_Control_GetFrontLeftFirmwareVersion() {
    if (!Feb_Control_activated) {
        return 0;
    }
    unsigned int value = 0;
    if (!Feb_Interface_ReadRegister(Feb_Control_leftAddress, FEB_REG_STATUS,
                                    &value)) {
        LOG(logERROR, ("Trouble reading FEB_REG_STATUS reg to get left feb "
                       "fw version\n"));
        return 0;
    }
    return ((value & FEB_REG_STATUS_FW_VERSION_MSK) >>
            FEB_REG_STATUS_FW_VERSION_OFST);
}

int64_t Feb_Control_GetFrontRightFirmwareVersion() {
    if (!Feb_Control_activated) {
        return 0;
    }
    unsigned int value = 0;
    if (!Feb_Interface_ReadRegister(Feb_Control_rightAddress, FEB_REG_STATUS,
                                    &value)) {
        LOG(logERROR, ("Trouble reading FEB_REG_STATUS reg to get right feb "
                       "fw version\n"));
        return 0;
    }
    return ((value & FEB_REG_STATUS_FW_VERSION_MSK) >>
            FEB_REG_STATUS_FW_VERSION_OFST);
}

int64_t Feb_Control_GetMeasuredPeriod() {
    if (!Feb_Control_activated) {
        return 0;
    }
    unsigned int value = 0;
    if (!Feb_Interface_ReadRegister(Feb_Control_leftAddress, MEAS_PERIOD_REG,
                                    &value)) {
        LOG(logERROR,
            ("Trouble reading MEAS_PERIOD_REG reg to get measured period\n"));
        return 0;
    }
    return (int64_t)value * 10;
}

int64_t Feb_Control_GetSubMeasuredPeriod() {
    if (!Feb_Control_activated) {
        return 0;
    }
    unsigned int value = 0;
    if (!Feb_Interface_ReadRegister(Feb_Control_leftAddress, MEAS_SUBPERIOD_REG,
                                    &value)) {
        LOG(logERROR, ("Trouble reading MEAS_SUBPERIOD_REG reg to get measured "
                       "sub period\n"));
        return 0;
    }
    return (int64_t)value * 10;
}
