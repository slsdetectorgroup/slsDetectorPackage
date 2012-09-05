#ifndef ANGLE_CONVERSION_CONSTANT_H
#define ANGLE_CONVERSION_CONSTANT_H

class angleConversionConstant {

 public:
//typedef struct  {
  double center;  /**< center of the module (channel at which the radius is perpendicular to the module surface) */
  double ecenter; /**< error in the center determination */
  double r_conversion;  /**<  detector pixel size (or strip pitch) divided by the diffractometer radius */
  double er_conversion;  /**< error in the r_conversion determination */
  double offset; /**< the module offset i.e. the position of channel 0 with respect to the diffractometer 0 */
  double eoffset; /**< error in the offset determination */
  double tilt; /**< ossible tilt in the orthogonal direction (unused)*/
  double etilt; /**< error in the tilt determination */
//} angleConversionConstant;
};

#endif
