/*
 * qDefs.h
 *
 *  Created on: May 4, 2012
 *      Author: l_maliakal_d
 */

#ifndef QDEFS_H
#define QDEFS_H

#include "sls_detector_defs.h"
#include "slsDetector.h"
#include "multiSlsDetector.h"
#include <string>
#include <ostream>
#include <iostream>
#include <QMessageBox>
#include <QAbstractButton>
using namespace std;

class qDefs:public QWidget{
public:
//-------------------------------------------------------------------------------------------------------------------------------------------------

	/** Empty Constructor
	 */
	qDefs(){};
//-------------------------------------------------------------------------------------------------------------------------------------------------

static const int64_t GUI_VERSION=0x20121213;

#define GOODBYE 				-200
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

//-------------------------------------------------------------------------------------------------------------------------------------------------

	/**displays an warning,error,info message
	 * @param message the message to be displayed
	 * @param source is the tab or the source of the message
	 * */
	static int  Message(MessageIndex index, string message,string source)
	{
		static QMessageBox* msgBox;
		size_t pos;

		//replace all \n with <br>
		pos = 0;
		while((pos = message.find("\n", pos)) != string::npos){
			message.replace(pos, 1, "<br>");
			pos += 1;
		}
		message.append(string("<p style=\"font-size:10px;color:grey;\">Source:&nbsp;&nbsp; ") + source + string("</p>"));

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

	/**gets error mask and displays the message if it exists
	 * @param myDet is the multidetector object
	 /returns error message else an empty string
	 * */
	static string checkErrorMessage(multiSlsDetector*& myDet, string title = "Main"){


		int errorLevel= (int)WARNING;
		string retval="";
		size_t pos;


		retval = myDet->getErrorMessage(errorLevel);

		if(!retval.empty()){
			//replace all \n with <br>
			pos = 0;
			while((pos = retval.find("\n", pos)) != string::npos){
				retval.replace(pos, 1, "<br>");
				pos += 1;
			}

			//get rid of the last \n
			if(retval.find_last_of("<br>")==retval.length()-1)
				retval.erase((int)retval.find_last_of("<br>")-3,4);

			retval.insert(0,"<font color=\"darkBlue\">");
			retval.append("</font></nobr>");

			//display message
			qDefs::Message((MessageIndex)errorLevel,retval,title);
		}

		myDet->clearAllErrorMask();
		return retval;
	};


//-------------------------------------------------------------------------------------------------------------------------------------------------

	/**gets error mask and displays the message if it exists
	 * @param myDet is the slsdetector object
	 * @param show to display the error message
		 /returns error message else an empty string
	 * */
	static string checkErrorMessage(slsDetector*& myDet, string title = "Main", bool show = true){


		int errorLevel= (int)WARNING;
		string retval="";
		size_t pos;
		int64_t emask=0;

		emask = myDet->getErrorMask();
		retval = myDet->getErrorMessage(emask);

		if(!retval.empty()){
			//replace all \n with <br>
			pos = 0;
			while((pos = retval.find("\n", pos)) != string::npos){
				retval.replace(pos, 1, "<br>");
				pos += 1;
			}

			//get rid of the last \n
			if(retval.find_last_of("<br>")==retval.length()-1)
				retval.erase((int)retval.find_last_of("<br>")-3,4);

			retval.insert(0,"<font color=\"darkBlue\">");
			retval.append("</font></nobr>");

			//display message
			if(show)
				qDefs::Message((MessageIndex)errorLevel,retval,title);
		}

		myDet->clearErrorMask();

		return retval;
	};


//-------------------------------------------------------------------------------------------------------------------------------------------------


	/** scan arguments*/
	enum scanArgumentList{
		None,
		Level0,
		Level1,
		FileIndex,
		AllFrames
	};


//-------------------------------------------------------------------------------------------------------------------------------------------------


	/** histogram arguments*/
	enum histogramArgumentList{
		Intensity,
		histLevel0,
		histLevel1
	};


//-------------------------------------------------------------------------------------------------------------------------------------------------


};


#endif /* QDEFS_H */
