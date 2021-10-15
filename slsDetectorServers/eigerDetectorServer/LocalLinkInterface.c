// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "LocalLinkInterface.h"
#include "HardwareMMappingDefs.h"
#include "clogger.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

void Local_LocalLinkInterface(struct LocalLinkInterface *ll,
                              unsigned int ll_fifo_badr) {
    LOG(logDEBUG1, ("Initialize PLB LL FIFOs\n"));
    ll->ll_fifo_base = 0;
    ll->ll_fifo_ctrl_reg = 0;
    if (Local_Init(ll, ll_fifo_badr)) {
        Local_Reset(ll);
        LOG(logDEBUG1,
            ("\tFIFO Status : 0x%08x\n\n\n", Local_StatusVector(ll)));
    } else
        LOG(logERROR,
            ("\tCould not map LocalLink : 0x%08x\n\n\n", ll_fifo_badr));
}

int Local_Init(struct LocalLinkInterface *ll, unsigned int ll_fifo_badr) {
    int fd;
    void *plb_ll_fifo_ptr;

    if ((fd = open("/dev/mem", O_RDWR)) < 0) {
        LOG(logERROR, ("Could not open /dev/mem for local link\n"));
        return 0;
    }

    plb_ll_fifo_ptr = mmap(0, getpagesize(), PROT_READ | PROT_WRITE,
                           MAP_FILE | MAP_SHARED, fd, ll_fifo_badr);
    close(fd);

    if (plb_ll_fifo_ptr == MAP_FAILED) {
        LOG(logERROR, ("mmap error for local link\n"));
        return 0;
    }

    ll->ll_fifo_base = (xfs_u32)plb_ll_fifo_ptr;
    ll->ll_fifo_ctrl_reg = 0;

    return 1;
}

int Local_Reset(struct LocalLinkInterface *ll) {
    return Local_Reset1(ll, PLB_LL_FIFO_CTRL_RESET_STD);
}

int Local_Reset1(struct LocalLinkInterface *ll, unsigned int rst_mask) {
    ll->ll_fifo_ctrl_reg |= rst_mask;
    LOG(logDEBUG1, ("\tCTRL Register bits: 0x%08x\n", ll->ll_fifo_ctrl_reg));

    HWIO_xfs_out32(ll->ll_fifo_base + 4 * PLB_LL_FIFO_REG_CTRL,
                   ll->ll_fifo_ctrl_reg);
    HWIO_xfs_out32(ll->ll_fifo_base + 4 * PLB_LL_FIFO_REG_CTRL,
                   ll->ll_fifo_ctrl_reg);
    HWIO_xfs_out32(ll->ll_fifo_base + 4 * PLB_LL_FIFO_REG_CTRL,
                   ll->ll_fifo_ctrl_reg);
    HWIO_xfs_out32(ll->ll_fifo_base + 4 * PLB_LL_FIFO_REG_CTRL,
                   ll->ll_fifo_ctrl_reg);

    ll->ll_fifo_ctrl_reg &= (~rst_mask);

    HWIO_xfs_out32(ll->ll_fifo_base + 4 * PLB_LL_FIFO_REG_CTRL,
                   ll->ll_fifo_ctrl_reg);
    return 1;
}

unsigned int Local_StatusVector(struct LocalLinkInterface *ll) {
    return HWIO_xfs_in32(ll->ll_fifo_base + 4 * PLB_LL_FIFO_REG_STATUS);
}

int Local_Write(struct LocalLinkInterface *ll, unsigned int buffer_len,
                void *buffer) {

    // note: buffer must be word (4 byte) aligned
    // frame_len in byte
    int vacancy = 0;
    int words_send = 0;
    int last_word;
    unsigned int *word_ptr;
    unsigned int fifo_ctrl;
    xfs_u32 status;

    if (buffer_len < 1) {
        return -1;
    }

    last_word = (buffer_len - 1) / 4;
    word_ptr = (unsigned int *)buffer;

    LOG(logDEBUG1, ("LL Write - Len: %2d - If: %X - Data: ", buffer_len,
                    ll->ll_fifo_base));
    for (int i = 0; i < buffer_len / 4; i++)
        LOG(logDEBUG1, ("%.8X ", *(((unsigned *)buffer) + i)));

    while (words_send <= last_word) {
        while (!vacancy) // wait for Fifo to be empty again
        {
            status =
                HWIO_xfs_in32(ll->ll_fifo_base + 4 * PLB_LL_FIFO_REG_STATUS);
            if ((status & PLB_LL_FIFO_STATUS_ALMOSTFULL) == 0)
                vacancy = 1;
            if (vacancy == 0) {
                LOG(logERROR, ("Fifo full!\n"));
            }
        }

        // Just to know: #define PLB_LL_FIFO_ALMOST_FULL_THRESHOLD_WORDS    100
        for (int i = 0; ((i < PLB_LL_FIFO_ALMOST_FULL_THRESHOLD_WORDS) &&
                         (words_send <= last_word));
             i++) {
            fifo_ctrl = 0;
            if (words_send == 0) {
                fifo_ctrl =
                    PLB_LL_FIFO_CTRL_LL_SOF; // announce the start of file
            }

            if (words_send == last_word) {
                fifo_ctrl |=
                    (PLB_LL_FIFO_CTRL_LL_EOF |
                     (((buffer_len - 1) << PLB_LL_FIFO_CTRL_LL_REM_SHIFT) &
                      PLB_LL_FIFO_CTRL_LL_REM));
            }
            Local_ctrl_reg_write_mask(ll, PLB_LL_FIFO_CTRL_LL_MASK, fifo_ctrl);
            HWIO_xfs_out32(ll->ll_fifo_base + 4 * PLB_LL_FIFO_REG_FIFO,
                           word_ptr[words_send++]);
        }
    }

    return buffer_len;
}

int Local_Read(struct LocalLinkInterface *ll, unsigned int buffer_len,
               void *buffer) {

    static unsigned int buffer_ptr = 0;
    // note: buffer must be word (4 byte) aligned
    // frame_len in byte
    int len;
    unsigned int *word_ptr;
    unsigned int status;
    volatile unsigned int fifo_val;
    int sof = 0;

    LOG(logDEBUG1, ("LL Read - If: %X - Data: ", ll->ll_fifo_base));

    word_ptr = (unsigned int *)buffer;
    do {
        status = HWIO_xfs_in32(ll->ll_fifo_base + 4 * PLB_LL_FIFO_REG_STATUS);

        if (!(status & PLB_LL_FIFO_STATUS_EMPTY)) {
            if (status & PLB_LL_FIFO_STATUS_LL_SOF) {
                if (buffer_ptr) {
                    buffer_ptr = 0;
                    return -1; // buffer overflow
                }
                buffer_ptr = 0;
                sof = 1;
            }

            fifo_val = HWIO_xfs_in32(
                ll->ll_fifo_base + 4 * PLB_LL_FIFO_REG_FIFO); // read from fifo

            if ((buffer_ptr > 0) || sof) {
                if ((buffer_len >> 2) > buffer_ptr) {
                    LOG(logDEBUG1, ("%.8X ", fifo_val));
                    word_ptr[buffer_ptr++] = fifo_val; // write to buffer
                } else {
                    buffer_ptr = 0;
                    return -2; // buffer overflow
                }

                if (status & PLB_LL_FIFO_STATUS_LL_EOF) {
                    len = (buffer_ptr << 2) - 3 +
                          ((status & PLB_LL_FIFO_STATUS_LL_REM) >>
                           PLB_LL_FIFO_STATUS_LL_REM_SHIFT);
                    LOG(logDEBUG1, ("Len: %d\n", len));
                    buffer_ptr = 0;
                    return len;
                }
            }
        }
    } while (!(status & PLB_LL_FIFO_STATUS_EMPTY));
    return 0;
}

int Local_ctrl_reg_write_mask(struct LocalLinkInterface *ll, unsigned int mask,
                              unsigned int val) {
    ll->ll_fifo_ctrl_reg &= (~mask);
    ll->ll_fifo_ctrl_reg |= (mask & val);
    HWIO_xfs_out32(ll->ll_fifo_base + 4 * PLB_LL_FIFO_REG_CTRL,
                   ll->ll_fifo_ctrl_reg);
    return 1;
}

int Local_Test(struct LocalLinkInterface *ll, unsigned int buffer_len,
               void *buffer) {

    int len;
    unsigned int rec_buff_len = 4096;
    unsigned int rec_buffer[4097];

    Local_Write(ll, buffer_len, buffer);
    usleep(10000);

    do {
        len = Local_Read(ll, rec_buff_len, rec_buffer);
        LOG(logDEBUG1, ("receive length: %i\n", len));

        if (len > 0) {
            rec_buffer[len] = 0;
            LOG(logINFO, ("%s\n", (char *)rec_buffer));
        }
    } while (len > 0);

    return 1;
}
