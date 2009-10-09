#include "usersFunctions.h"
#include <math.h>

float pos;

/* 
   contains the conversion channel-angle for a module channel
   conv_r=pitch/radius
*/


float angle(int ichan, float encoder, float totalOffset, float conv_r, float center, float offset, float tilt, int direction) {
 
 
  float ang;

  ang=180./PI*(center*conv_r+atan((float)(ichan-center)*conv_r))+encoder+totalOffset; 

  return direction*ang;

}

/* reads the encoder and returns the position */

float get_position() {
  return pos;
}

/* moves the encoder to position p */

int go_to_position(float p) {
  pos=p;
}

/* moves the encoder to position p without waiting */

int go_to_position_no_wait(float p) {
  pos=p;
}


/* reads I0 and returns the intensity */

float get_i0() {
  return 100.;
}
  
