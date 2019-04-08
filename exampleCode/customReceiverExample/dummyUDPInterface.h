#ifndef DUMMYUDPINTERFACE_H
#define DUMMYUDPINTERFACE_H

/***********************************************
 * @file UDPInterface.h
 * @short Base class with all the functions for the UDP inteface of the receiver
 ***********************************************/
/**
 * \mainpage Base class with all the functions for the UDP inteface of the receiver
 */

/**
 * @short Base class with all the functions for the UDP inteface of the receiver
 */

#include "UDPBaseImplementation.h"


class dummyUDPInterface : public virtual slsReceiverDefs, public UDPBaseImplementation {
	

	
 public:
	
	/** cosntructor & destructor */
  dummyUDPInterface() { cout << "New dummy UDP Interface" << endl;};
   ~dummyUDPInterface() {cout << "Destroying  dummy UDP Interface" << endl;};


 protected:
	
 private:

};

#endif  /* #ifndef DUMMYUDPINTERFACE_H */
