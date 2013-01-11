#ifndef TRIMMING_FUNCS_H
#define TRIMMING_FUNCS_H

#include "sls_detector_defs.h"

int trim_fixed_settings(int countlim, int par2, int imod);
int trim_with_noise(int countlim, int nsigma, int imod);
int trim_with_beam(int countlim, int nsigma, int imod);
int  trim_improve(int maxit, int par2, int imod);
int calcthr_from_vcal(int vcal);
int calccal_from_vthr(int vthr);
int choose_vthresh_and_vtrim(int countlim, int nsigma, int imod);

int choose_vthresh();
int trim_with_level(int countlim, int imod);
int trim_with_median(int stop, int imod);
int calcthr_from_vcal(int vcal);
int calccal_from_vthr(int vthr);

#endif
