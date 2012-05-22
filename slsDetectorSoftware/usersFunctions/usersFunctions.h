#ifndef USERS_FUNCTIONS_H
#define USERS_FUNCTIONS_H
/******************************************************************

Functions depending on the experimental setup should be defined here

******************************************************************/

#define PI 3.14159265358979323846


#ifdef EPICS
#include <cadef.h>
#include <epicsEvent.h>
#endif
#include "detectorData.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EPICS
  int connect_channel(const char *name,  chid *ch_id);
  int disconnect_channel(chid ch_id);
  int caget(chid ch_id,  double *value);
  int caputq(chid ch_id,  double value);
  void put_callback(struct event_handler_args args);
  int caput(chid ch_id,  double value);
#endif


  float defaultAngleFunction(float ichan, float encoder, float totalOffset, float conv_r, float center, float offset, float tilt, int direction);
  float defaultGetPosition();
  int defaultGoToPosition(float p);
  int defaultGoToPositionNoWait(float p);
  int defaultConnectChannels();
  int defaultDisconnectChannels();
  float defaultGetI0(int t);
  
  int defaultDataReadyFunc(detectorData* d);



#ifdef __cplusplus
};
#endif


#endif
