#ifndef USERS_FUNCTIONS_H
#define USERS_FUNCTIONS_H
/******************************************************************

Functions depending on the experimental setup should be defined here

******************************************************************/

#define PI 3.14159265358979323846



#ifdef __cplusplus
extern "C" {
#endif

  float angle(int ichan, float encoder, float totalOffset, float conv_r, float center, float offset, float tilt, int direction);
  float get_position();
  int go_to_position(float p);
  int go_to_position_no_wait(float p);


  float get_i0();
  




#ifdef __cplusplus
};
#endif


#endif
