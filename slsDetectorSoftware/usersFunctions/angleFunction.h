#ifndef DEFAULT_ANGLE_FUNCTION_H
#define DEFAULT_ANGLE_FUNCTION_H

/* 
   contains the conversion channel-angle for a module channel
   conv_r=pitch/radius
*/
#define PI 3.14159265358979323846
#include <math.h>

double defaultAngleFunction(double ichan, double encoder, double totalOffset, double conv_r, double center, double offset, double tilt, int direction) {\
  (void) tilt;								\
  return 180./PI*(center*conv_r+direction*atan((double)(ichan-center)*conv_r))+encoder+totalOffset+offset; };

#endif
