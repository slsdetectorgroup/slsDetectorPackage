/*
 * qDefs.h
 *
 *  Created on: May 4, 2012
 *      Author: l_maliakal_d
 */

#ifndef QDEFS_H
#define QDEFS_H

class qDefs
{
public:
	/** Empty Constructor
	 */
	qDefs(){};

	/** unit of time
	 */
	enum timeUnit{
		HOURS,			/** hr  */
		MINUTES,		/** min */
		SECONDS, 		/** s 	*/
		MILLISECONDS,	/** ms 	*/
		MICROSECONDS,	/** us 	*/
		NANOSECONDS		/** ns 	*/
	};


	/** returns the value in ns to send to server.
	 * @param unit unit of time
	 * @param value time
	 * returns time value in ns
	 */
	static int64_t get64bTime(timeUnit unit, double value){
		int64_t value64=value;
		switch(unit){
		case HOURS:			value64*=60;
		case MINUTES:		value64*=60;
		case SECONDS:		value64*=1000;
		case MILLISECONDS:	value64*=1000;
		case MICROSECONDS:	value64*=1000;
		case NANOSECONDS:
		default:;
		}
		return value64;
	};



};


#endif /* QDEFS_H */
