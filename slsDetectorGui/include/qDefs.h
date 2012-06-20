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
