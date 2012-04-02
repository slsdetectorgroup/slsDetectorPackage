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


  float angle(float ichan, float encoder, float totalOffset, float conv_r, float center, float offset, float tilt, int direction);
  float get_position();
  int go_to_position(float p);
  int go_to_position_no_wait(float p);
  int connect_channels();
  int disconnect_channels();

  float get_i0();
  




#ifdef __cplusplus
};
#endif


#endif
