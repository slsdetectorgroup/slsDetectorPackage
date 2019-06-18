#pragma once

#include "multiSlsDetector.h"
#include "sls_detector_defs.h"

#include <QAbstractButton>
#include <QMessageBox>

#include <iostream>
#include <ostream>
#include <stdint.h>
#include <string>

class qDefs : public QWidget {
  public:
    /**
     * Empty Constructor
     */
    qDefs(){};

#define GOODBYE -200

    /** function enums */
    enum qFuncNames {
        QF_GET_DETECTOR_STATUS,
        QF_START_ACQUISITION,
        QF_STOP_ACQUISITION,
        QF_START_AND_READ_ALL,
        QF_EXIT_SERVER,
        QF_NUM_FUNCTIONS
    };

    const char* getQFunctionNameFromEnum(enum qFuncNames func) {
        switch (func) {
        case QF_GET_DETECTOR_STATUS:	return "QF_GET_DETECTOR_STATUS";
        case QF_START_ACQUISITION:		return "QF_START_ACQUISITION";
        case QF_STOP_ACQUISITION:		return "QF_STOP_ACQUISITION";
        case QF_START_AND_READ_ALL:	    return "QF_START_AND_READ_ALL";
        case QF_EXIT_SERVER:			return "QF_EXIT_SERVER";
        case QF_NUM_FUNCTIONS:			return "QF_NUM_FUNCTIONS";
        default:						return "Unknown Function";
        }
    };

    /** Success or FAIL */
    enum { OK, FAIL };

    /**
     * Message Criticality
     */
    enum MessageIndex { WARNING, CRITICAL, INFORMATION, QUESTION };

    /**
     * unit of time
     */
    enum timeUnit {
        HOURS,        /** hr  */
        MINUTES,      /** min */
        SECONDS,      /** s 	*/
        MILLISECONDS, /** ms 	*/
        MICROSECONDS, /** us 	*/
        NANOSECONDS   /** ns 	*/
    };

    /**
     * range of x and y axes
     */
    enum range { XMINIMUM, XMAXIMUM, YMINIMUM, YMAXIMUM };

    /**
     * function enums for the qServer and qClient
     */
    enum guiFuncs {
        F_GUI_GET_RUN_STATUS,
        F_GUI_START_ACQUISITION,
        F_GUI_STOP_ACQUISITION,
        F_GUI_START_AND_READ_ALL,
        F_GUI_EXIT_SERVER,
        NUM_GUI_FUNCS
    };

    /**
     * returns the unit in words
     * @param unit is the time unit
     */
    static std::string getUnitString(timeUnit unit) {
        switch (unit) {
        case HOURS:
            return std::string("hrs");
        case MINUTES:
            return std::string("min");
        case SECONDS:
            return std::string("sec");
        case MILLISECONDS:
            return std::string("msec");
        case MICROSECONDS:
            return std::string("usec");
        case NANOSECONDS:
            return std::string("nsec");
        default:
            return std::string("error");
        }
    };

    /**
     * returns the value in ns to send to server as the
     * server class slsdetector accepts in ns.
     * @param unit unit of time
     * @param value time
     * returns time value in ns
     */
    static double getNSTime(timeUnit unit, double value) {
        double valueNS = value;
        switch (unit) {
        case HOURS:
            valueNS *= 60;
        case MINUTES:
            valueNS *= 60;
        case SECONDS:
            valueNS *= 1000;
        case MILLISECONDS:
            valueNS *= 1000;
        case MICROSECONDS:
            valueNS *= 1000;
        case NANOSECONDS:
        default:
            break;
        }
        return valueNS;
    };

    /**
     * returns the value in ms
     * @param unit unit of time
     * @param value time
     * returns time value in ms
     */
    static double getMSTime(timeUnit unit, double value) {
        double valueMS = value;
        switch (unit) {
        case NANOSECONDS:  
            valueMS /= 1000;
        case MICROSECONDS:
            valueMS /= 1000;
            return valueMs;

        case HOURS:
            valueMS *= 60;
        case MINUTES:
            valueMS *= 60;
        case SECONDS:
            valueMS *= 1000;
        default:
            break;
        }
        return valueMS;
    };

    /**
     * returns the time in the appropriate time unit
     * @param value time in seconds
     * returns the time in an appropriate time and unit
     */
    static std::pair<double, timeUnit> getCorrectTime(double value) {
        timeUnit unit;
        int intUnit = (int)SECONDS;

        /**0 ms*/
        if (!value) {
            unit = MILLISECONDS;
            return std::make_pair(value, unit);
        }

        /** hr, min, sec */
        if (value >= 1) {
            double newVal = value;
            while ((newVal >= 1) && (intUnit >= (int)HOURS)) {
                /** value retains the old value */
                value = newVal;
                newVal = value / (double)60;
                intUnit--;
            }
            /** returning the previous value*/
            unit = (timeUnit)(intUnit + 1);
            return std::make_pair(value, unit);
        }
        /** ms, us, ns */
        else {
            while ((value < 1) && (intUnit < (int)NANOSECONDS)) {
                value = value * (double)1000;
                intUnit++;
            }
            unit = (timeUnit)(intUnit);
            return std::make_pair(value, unit);
        }
    };

    /**
     * displays an warning,error,info message
     * @param message the message to be displayed
     * @param source is the tab or the source of the message
     * */
    static int Message(MessageIndex index, std::string message,
                       std::string source) {
        static QMessageBox *msgBox;
        size_t pos;

        // replace all \n with <br>
        pos = 0;
        while ((pos = message.find("\n", pos)) != std::string::npos) {
            message.replace(pos, 1, "<br>");
            pos += 1;
        }
        message.append(
            std::string(
                "<p "
                "style=\"font-size:10px;color:grey;\">Source:&nbsp;&nbsp; ") +
            source + std::string("</p>"));

        switch (index) {
        case WARNING:
            msgBox =
                new QMessageBox(QMessageBox::Warning, "WARNING",
                                tr(message.c_str()), QMessageBox::Ok, msgBox);
            break;
        case CRITICAL:
            msgBox =
                new QMessageBox(QMessageBox::Critical, "CRITICAL",
                                tr(message.c_str()), QMessageBox::Ok, msgBox);
            break;
        case INFORMATION:
            msgBox =
                new QMessageBox(QMessageBox::Information, "INFORMATION",
                                tr(message.c_str()), QMessageBox::Ok, msgBox);
            break;
        default:
            msgBox = new QMessageBox(
                QMessageBox::Question, "QUESTION", tr(message.c_str()),
                QMessageBox::Ok | QMessageBox::Cancel, msgBox);
            break;
        }
        // msgBox->setDetailedText(QString(source.c_str())); //close button
        // doesnt work with this static function and this
        if (msgBox->exec() == QMessageBox::Ok)
            return OK;
        else
            return FAIL;
    }

    /**
     * Wrap exception message
     */
    static int ExceptionMessage(std::string message, 
                        std::string exceptionMessage,
                       std::string source) {
        return Message(qDefs::WARNING, message + std::string("\nCaught exception:\n") + exceptionMessage, source);
    } 

};
