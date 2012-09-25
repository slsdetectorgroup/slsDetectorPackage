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

#define GUI_VERSION 0.11

//-------------------------------------------------------------------------------------------------------------------------------------------------

	enum{
		OK,
		FAIL
	};

//-------------------------------------------------------------------------------------------------------------------------------------------------

	enum MessageIndex{
		WARNING,
		CRITICAL,
		INFORMATION,
		QUESTION
	};

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

	/** returns the unit in words
	 * @param unit is the time unit
	 */
	static string getUnitString(timeUnit unit){
		switch(unit){
		case HOURS:			return string("hrs");
		case MINUTES:		return string("min");
		case SECONDS:		return string("sec");
		case MILLISECONDS:	return string("msec");
		case MICROSECONDS:	return string("usec");
		case NANOSECONDS:	return string("nsec");
		default:			return string("error");
		}
	};
//-------------------------------------------------------------------------------------------------------------------------------------------------

	/** returns the value in ns to send to server as the
	 * server class slsdetector accepts in ns.
	 * @param unit unit of time
	 * @param value time
	 * returns time value in ns
	 */
	static double getNSTime(timeUnit unit, double value){
		double valueNS=value;
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
	static double getCorrectTime(timeUnit& unit, double value){
		int intUnit = (int)SECONDS;

		/**0 ms*/
		if(!value){
			unit = MILLISECONDS;
			return value;
		}

		/** hr, min, sec */
		if(value>=1){
			double newVal = value;
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

	/**displays an warning,error,info message
	 * @param message the message to be displayed
	 * @param source is the tab or the source of the message
	 * */
	static int  Message(MessageIndex index, string message,string source)
	{
		static QMessageBox* msgBox;
		switch(index){
		case WARNING:
			source.append(": WARNING");
			msgBox= new QMessageBox(QMessageBox::Warning,source.c_str(),tr(message.c_str()),QMessageBox::Ok, msgBox);
			break;
		case CRITICAL:
			source.append(": CRITICAL");
			msgBox= new QMessageBox(QMessageBox::Critical,source.c_str(),tr(message.c_str()),QMessageBox::Ok, msgBox);
			break;
		case INFORMATION:
			source.append(": INFORMATION");
			msgBox= new QMessageBox(QMessageBox::Information,source.c_str(),tr(message.c_str()),QMessageBox::Ok, msgBox);
			break;
		default:
			source.append(": QUESTION");
			msgBox= new QMessageBox(QMessageBox::Question,source.c_str(),tr(message.c_str()),QMessageBox::Ok| QMessageBox::Cancel, msgBox);
			break;
		}
		if(msgBox->exec()==QMessageBox::Ok) return OK; else return FAIL;
	}


//-------------------------------------------------------------------------------------------------------------------------------------------------

	/** range of x and y axes
	 */
	enum range{
		XMINIMUM,
		XMAXIMUM,
		YMINIMUM,
		YMAXIMUM
	};

//-------------------------------------------------------------------------------------------------------------------------------------------------

};


#endif /* QDEFS_H */
