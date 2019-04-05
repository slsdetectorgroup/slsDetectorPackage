#pragma once

#include "sls_detector_defs.h"
#include "multiSlsDetector.h"
#include <string>
#include <ostream>
#include <iostream>
#include <QMessageBox>
#include <QAbstractButton>


class qDefs:public QWidget{
public:

	/** Empty Constructor
	 */
	qDefs(){};


#define GOODBYE 				-200

	enum{
		OK,
		FAIL
	};


	enum MessageIndex{
		WARNING,
		CRITICAL,
		INFORMATION,
		QUESTION
	};

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


	/** range of x and y axes
	 */
	enum range{
		XMINIMUM,
		XMAXIMUM,
		YMINIMUM,
		YMAXIMUM
	};



	/** returns the unit in words
	 * @param unit is the time unit
	 */
	static std::string getUnitString(timeUnit unit){
		switch(unit){
		case HOURS:			return "hrs";
		case MINUTES:		return "min";
		case SECONDS:		return "sec";
		case MILLISECONDS:	return "msec";
		case MICROSECONDS:	return "usec";
		case NANOSECONDS:	return "nsec";
		default:			return "error";
		}
	};


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
		default: break;
		}
		return valueNS;
	};


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
				newVal = value/(double)60;
				intUnit--;
			}
			/** returning the previous value*/
			unit = (timeUnit)(intUnit+1);
			return value;
		}
		/** ms, us, ns */
		else{
			while((value<1)&&(intUnit<(int)NANOSECONDS)){
				value = value*(double)1000;
				intUnit++;
			}
			unit = (timeUnit)(intUnit);
			return value;
		}
	};


	/**displays an warning,error,info message
	 * @param message the message to be displayed
	 * @param source is the tab or the source of the message
	 * */
	static int  Message(MessageIndex index, std::string message, std::string source)
	{
		static QMessageBox* msgBox;
		size_t pos;

		//replace all \n with <br>
		pos = 0;
		while((pos = message.find("\n", pos)) != std::string::npos){
			message.replace(pos, 1, "<br>");
			pos += 1;
		}
		message.append(std::string("<p style=\"font-size:10px;color:grey;\">Source:&nbsp;&nbsp; ") + source + std::string("</p>"));

		switch(index){
		case WARNING:
			msgBox= new QMessageBox(QMessageBox::Warning,"WARNING",tr(message.c_str()),QMessageBox::Ok, msgBox);
			break;
		case CRITICAL:
			msgBox= new QMessageBox(QMessageBox::Critical,"CRITICAL",tr(message.c_str()),QMessageBox::Ok, msgBox);
			break;
		case INFORMATION:
			msgBox= new QMessageBox(QMessageBox::Information,"INFORMATION",tr(message.c_str()),QMessageBox::Ok, msgBox);
			break;
		default:
			msgBox= new QMessageBox(QMessageBox::Question,"QUESTION",tr(message.c_str()),QMessageBox::Ok| QMessageBox::Cancel, msgBox);
			break;
		}
		//msgBox->setDetailedText(QString(source.c_str())); //close button doesnt work with this static function and this
		if(msgBox->exec()==QMessageBox::Ok) return OK; else return FAIL;
	}

};


