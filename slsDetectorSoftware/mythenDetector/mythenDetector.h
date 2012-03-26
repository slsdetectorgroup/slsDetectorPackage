


#ifndef MYTHEN_DETECTOR_H
#define MYTHEN_DETECTOR_H

//#include <ostream>
#include "slsDetector.h"
#include "usersFunctions.h"

#define defaultTDead {170,90,750}

//using namespace std;
/**
 \mainpage C++ class with MYTHEN specific functions
 *
 

 @author Anna Bergamaschi

*/



class mythenDetector : public slsDetector{

  
  
 public:
  /** 
      (default) constructor 
  */
  mythenDetector(int id=0, detectorType t=MYTHEN) : slsDetector(t, id){};
  //slsDetector(string  const fname);
  //  ~slsDetector(){while(dataQueue.size()>0){}};
  /** destructor */ 
  virtual ~mythenDetector(){};



};
#endif
