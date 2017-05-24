#ifdef MCB_FUNCS

#ifndef MCB_FUNCS_H
#define MCB_FUNCS_H

#include "sls_detector_defs.h"

int initDetector();
int copyModule(sls_detector_module *destMod, sls_detector_module *srcMod);
int setSettings(int i,int imod);
int initModulebyNumber(sls_detector_module);
int getModulebyNumber(sls_detector_module*);
int getModuleNumber(int modnum);

#endif

#endif
