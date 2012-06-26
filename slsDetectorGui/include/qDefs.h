/*
 * qDefs.h
 *
 *  Created on: May 4, 2012
 *      Author: l_maliakal_d
 */

#ifndef QDEFS_H
#define QDEFS_H

#include <iostream>
#include <QMessageBox>
using namespace std;

class qDefs:public QWidget{
public:
//-------------------------------------------------------------------------------------------------------------------------------------------------

	/** Empty Constructor
	 */
	qDefs(){};

//-------------------------------------------------------------------------------------------------------------------------------------------------

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

//-------------------------------------------------------------------------------------------------------------------------------------------------

	/** returns the value in ns to send to server.
	 * @param unit unit of time
	 * @param value time
	 * returns time value in ns
	 */
	static float getNSTime(timeUnit unit, float value){
		float valueNS=value;
		switch(unit){
		case HOURS:			valueNS*=60;
		case MINUTES:		valueNS*=60;
		case SECONDS:		valueNS*=1000;
		case MILLISECONDS:	valueNS*=1000;
		case MICROSECONDS:	valueNS*=1000;
		case NANOSECONDS:
		default:;
		}
		return valueNS;
	};

//-------------------------------------------------------------------------------------------------------------------------------------------------

	/** returns the time in the appropriate time unit
	 * @param unit unit of time
	 * @param value time in seconds
	 * returns the corresponding time value
	 */
	static float getCorrectTime(timeUnit& unit, float value){
		int intUnit = (int)SECONDS;

		/** hr, min, sec */
		if(value>=1){
			float newVal = value;
			while((newVal>=1)&&(intUnit>=(int)HOURS)){
				/** value retains the old value */
				value = newVal;
				newVal = value/60;
				intUnit--;
			}
			/** returning the previous value*/
			unit = (timeUnit)(intUnit+1);
			return value;
		}
		/** ms, us, ns */
		else{
			while((value<1)&&(intUnit<(int)NANOSECONDS)){
				value = value*1000;
				intUnit++;
			}
			unit = (timeUnit)(intUnit);
			return value;
		}
	};

//-------------------------------------------------------------------------------------------------------------------------------------------------

	/**displays an error message
	 * @param errorMessage the message to be displayed
	 * @param source is the tab or the source of the error
	 * */
	static void  ErrorMessage(string errorMessage,char source[])
	{
		static QMessageBox* msgBox;
		msgBox= new QMessageBox(QMessageBox::Warning,source,tr(errorMessage.c_str()),QMessageBox::Ok, msgBox);
		msgBox->exec();
	}

//-------------------------------------------------------------------------------------------------------------------------------------------------

};


#endif /* QDEFS_H */
