// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

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

int main(int argc, char *argv[]) {

    int iarg;
    char fname[10000];
    uint64_t word;
    int val[64];
    int bit[64];
    FILE *fdin;

    int nb = 2;
    int off = 0;
    int ioff = 0;
    int dr = 24;
    int idr = 0;
    int ib = 0;
    int iw = 0;
    bit[0] = 19;
    bit[1] = 8;
    //  for (iarg=0; iarg<argc; iarg++) printf("%d %s\n",iarg, argv[iarg]);

    if (argc < 2)
        printf("Error: usage is %s fname [dr off  b0 b1 bn]\n");

    if (argc > 2)
        dr = atoi(argv[2]);
    if (argc > 3)
        off = atoi(argv[3]);
    if (argc > 4) {
        for (ib = 0; ib < 64; ib++) {
            if (argc > 4 + ib) {
                bit[ib] = atoi(argv[4 + ib]);
                nb++;
            }
        }
    }

    idr = 0;
    for (ib = 0; ib < nb; ib++) {
        val[ib] = 0;
    }

    fdin = fopen(argv[1], "rb");
    if (fdin == NULL) {
        printf("Cannot open input file %s for reading\n", argv[1]);
        return 200;
    }

    while (fread((void *)&word, 8, 1, fdin)) {
        //   printf("%llx\n",word);
        if (ioff < off)
            ioff++;
        else {

            for (ib = 0; ib < nb; ib++) {
                if (word & (1 << bit[ib]))
                    val[ib] |= (1 << idr);
            }
            idr++;
            if (idr == dr) {
                idr = 0;
                fprintf(stdout, "%d\t", iw++);
                for (ib = 0; ib < nb; ib++) {
#ifdef HEX
                    fprintf(stdout, "%08llx\t", val[ib]);
#else
                    fprintf(stdout, "%lld\t", val[ib]);

#endif

                    val[ib] = 0;
                }
                fprintf(stdout, "\n");
            }
        }
    }
    if (idr != 0) {
        fprintf(stdout, "%d\t", iw++);
        for (ib = 0; ib < nb; ib++) {
#ifdef HEX
            fprintf(stdout, "%08llx\t", val[ib]);
#else
            fprintf(stdout, "%lld\t", val[ib]);

#endif

            val[ib] = 0;
        }
        fprintf(stdout, "\n");
    }

    fclose(fdin);

    return 0;
}
