#ifndef ANGLE_CONVERSION_CONSTANT_H
#define ANGLE_CONVERSION_CONSTANT_H

class angleConversionConstant {

 public:
  
  angleConversionConstant(){};
  angleConversionConstant(double c, double r, double o, double t){center=c; r_conversion=r; offset=o; tilt=t;};

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

  double getCenter(){return center;};
  double getConversion(){return r_conversion;};
  double getOffset(){return offset;};
  double getTilt(){return tilt;};
  
  void setAngConvConstant(angleConversionConstant *acc) {\
    center=acc->center;					\
    ecenter=acc->ecenter;				\
    r_conversion=acc->r_conversion;			\
    er_conversion=acc->er_conversion;			\
    offset=acc->offset;					\
    eoffset=acc->eoffset;				\
    tilt=acc->tilt;					\
    etilt=acc->etilt;					\
  };



};

#endif
