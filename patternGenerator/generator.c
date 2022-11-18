// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/****************************************************************************
usage to generate a patter test.pat from test.p

gcc -DINFILE="\"test.p\"" -DOUTFILE="\"test.pat\"" -o test.exe generator.c ;
./test.exe ; rm test.exe


*************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

#define MAXLOOPS  6
#define MAXTIMERS 6
#define MAXWORDS  8191

uint64_t pat = 0;
uint64_t iopat = 0;
uint64_t clkpat = 0;

unsigned iaddr = 0;
unsigned waitaddr[MAXTIMERS] = {MAXWORDS, MAXWORDS, MAXWORDS,
                                MAXWORDS, MAXWORDS, MAXWORDS};
unsigned startloopaddr[MAXLOOPS] = {MAXWORDS, MAXWORDS, MAXWORDS,
                                    MAXWORDS, MAXWORDS, MAXWORDS};
unsigned stoploopaddr[MAXLOOPS] = {MAXWORDS, MAXWORDS, MAXWORDS,
                                   MAXWORDS, MAXWORDS, MAXWORDS};
unsigned start = 0, stop = 0;
uint64_t waittime[MAXTIMERS] = {0, 0, 0, 0, 0, 0};
unsigned nloop[MAXLOOPS] = {0, 0, 0, 0, 0, 0};

char infile[10000], outfile[10000];

FILE *fd, *fd1;
uint64_t PAT[MAXWORDS];
int iopat_enable = 0;

int i, ii, iii, j, jj, jjj, pixx, pixy, memx, memy, muxout, memclk, colclk,
    rowclk, muxclk, memcol, memrow, loopcounter;

void setstart() { start = iaddr; }

void setstop() { stop = iaddr; }

void setinput(int bit) {
    uint64_t mask = 1;
    mask = mask << bit;
    iopat &= ~mask;
    iopat_enable = 1;
}

void setoutput(int bit) {
    uint64_t mask = 1;
    mask = mask << bit;
    iopat |= mask;
    iopat_enable = 1;
}

void clearbit(int bit) {
    uint64_t mask = 1;
    mask = mask << bit;
    pat &= ~mask;
}
void setbit(int bit) {
    uint64_t mask = 1;
    mask = mask << bit;
    pat |= mask;
}

int checkbit(int bit) {
    uint64_t mask = 1;
    mask = mask << bit;
    return (pat & mask) >> bit;
}

void setstartloop(int iloop) {
    if (iloop >= 0 && iloop < MAXLOOPS) {
        startloopaddr[iloop] = iaddr;
    }
}

void setstoploop(int iloop) {
    if (iloop >= 0 && iloop < MAXLOOPS) {
        stoploopaddr[iloop] = iaddr;
    }
}

void setnloop(int iloop, int n) {
    if (iloop >= 0 && iloop < MAXLOOPS) {
        nloop[iloop] = n;
    }
}

void setwaitpoint(int iloop) {
    if (iloop >= 0 && iloop < MAXTIMERS) {
        waitaddr[iloop] = iaddr;
    }
}

void setwaittime(int iloop, uint64_t t) {
    if (iloop >= 0 && iloop < MAXTIMERS) {
        waittime[iloop] = t;
    }
}

void pw() {
    if (iaddr < MAXWORDS) {
        PAT[iaddr] = pat;
    }
    fprintf(fd, "patword 0x%04x 0x%016llx\n", iaddr, pat);
    iaddr++;
    if (iaddr >= MAXWORDS) {
        printf("ERROR: too many word in the pattern (%d instead of %d)!", iaddr,
               MAXWORDS);
    }
}

int parseCommand(int clk, int cmdbit, int cmd, int length) {
    int ibit;
    clearbit(clk);
    for (ibit = 0; ibit < length; ibit++) {
        if (cmd & (1 >> ibit)) {
            setbit(cmdbit);
        } else {
            clearbit(cmdbit);
        }
        pw();
        /******/
        setbit(clk);
        pw();
        /******/
    }
};

int main() {
    int iloop = 0;
    fd = fopen(OUTFILE, "w");
#include INFILE

    fprintf(fd, "patlimits 0x%04x 0x%04x\n", start, stop);

    if (iopat_enable == 1) {
        fprintf(fd, "patioctrl 0x%016llx\n", iopat);
    }

    for (iloop = 0; iloop < MAXLOOPS; iloop++) {
        if ((startloopaddr[iloop] != MAXWORDS) &&
            (stoploopaddr[iloop] != MAXWORDS)) {
            fprintf(fd, "patloop %d 0x%04x 0x%04x\n", iloop,
                    startloopaddr[iloop], stoploopaddr[iloop]);
            if (stoploopaddr[iloop] <= startloopaddr[iloop]) {
                nloop[iloop] = 0;
            }
            fprintf(fd, "patnloop %d %u\n", iloop, nloop[iloop]);
        }
    }

    for (iloop = 0; iloop < MAXTIMERS; iloop++) {
        if (waitaddr[iloop] != MAXWORDS) {
            fprintf(fd, "patwait %d 0x%04x\n", iloop, waitaddr[iloop]);
            fprintf(fd, "patwaittime %d %llu\n", iloop, waittime[iloop]);
        }
    }

    fclose(fd);
    fd1 = fopen(OUTFILEBIN, "w");
    fwrite(PAT, sizeof(uint64_t), iaddr, fd1);
    fclose(fd1);

    return 0;
}
