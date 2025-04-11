// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include "sls/logger.h"
#include "sls/sls_detector_defs.h"

#include <QAbstractButton>
#include <QMessageBox>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>

namespace sls {

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::hours;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::minutes;
using std::chrono::nanoseconds;
using std::chrono::seconds;

#define CATCH_DISPLAY(m, s)                                                    \
    catch (...) {                                                              \
        qDefs::DisplayExceptions(m, s);                                        \
    }
#define CATCH_HANDLE(...)                                                      \
    catch (...) {                                                              \
        qDefs::HandleExceptions(__VA_ARGS__);                                  \
    }

class qDefs : public QWidget {
  public:
    /**
     * Empty Constructor
     */
    qDefs(){};

    static QFont GetDefaultFont() {
        return QFont("Cantarell", 10, QFont::Normal);
    }

    static const int DATA_GAIN_PLOT_RATIO = 5;
    static const int MIN_HEIGHT_GAIN_PLOT_1D = 75;
    static const int GUI_ZMQ_RCV_HWM = 2;

    static void DisplayExceptions(std::string emsg, std::string src) {
        try {
            throw;
        } catch (const SocketError &e) {
            throw;
        } catch (const SharedMemoryError &e) {
            throw;
        } catch (const std::exception &e) {
            ExceptionMessage(emsg, e.what(), src);
        }
    }

    template <class CT> struct NonDeduced {
        using type = CT;
    };
    template <class S, typename RT, typename... CT>
    static void HandleExceptions(const std::string emsg, const std::string src,
                                 S *s, RT (S::*somefunc)(CT...),
                                 typename NonDeduced<CT>::type... Args) {
        try {
            throw;
        } catch (const SocketError &e) {
            throw;
        } catch (const SharedMemoryError &e) {
            throw;
        } catch (const std::exception &e) {
            ExceptionMessage(emsg, e.what(), src);
            (s->*somefunc)(Args...);
        }
    }

    /** function enums */
    enum qFuncNames {
        QF_GET_DETECTOR_STATUS,
        QF_START_ACQUISITION,
        QF_STOP_ACQUISITION,
        QF_START_AND_READ_ALL,
        QF_EXIT_SERVER,
        QF_NUM_FUNCTIONS
    };

    static const char *getQFunctionNameFromEnum(enum qFuncNames func) {
        switch (func) {
        case QF_GET_DETECTOR_STATUS:
            return "QF_GET_DETECTOR_STATUS";
        case QF_START_ACQUISITION:
            return "QF_START_ACQUISITION";
        case QF_STOP_ACQUISITION:
            return "QF_STOP_ACQUISITION";
        case QF_START_AND_READ_ALL:
            return "QF_START_AND_READ_ALL";
        case QF_EXIT_SERVER:
            return "QF_EXIT_SERVER";
        case QF_NUM_FUNCTIONS:
            return "QF_NUM_FUNCTIONS";
        default:
            return "Unknown Function";
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
    enum range { XMIN, XMAX, YMIN, YMAX };

    static std::string getRangeAsString(enum range r) {
        switch (r) {
        case XMIN:
            return "XMIN";
        case XMAX:
            return "XMAX";
        case YMIN:
            return "YMIN";
        case YMAX:
            return "YMAX";
        default:
            return "Unknown";
        }
    };

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

    /** returns string from enabled/disabled
        @param b true or false
        @returns string enabled, disabled
    */
    static std::string stringEnable(bool b) {
        if (b)
            return std::string("enabled");
        else
            return std::string("disabled");
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

    /** returns the time in a user friendly time unit */
    static std::pair<double, timeUnit> getUserFriendlyTime(nanoseconds tns) {
        if (tns < microseconds(1)) {
            return std::make_pair(tns.count(), NANOSECONDS);
        }
        if (tns < milliseconds(1)) {
            return std::make_pair(
                duration_cast<duration<double, std::micro>>(tns).count(),
                MICROSECONDS);
        }
        if (tns < seconds(1)) {
            return std::make_pair(
                duration_cast<duration<double, std::milli>>(tns).count(),
                MILLISECONDS);
        }
        if (tns < minutes(1)) {
            return std::make_pair(duration_cast<duration<double>>(tns).count(),
                                  SECONDS);
        }
        if (tns < hours(1)) {
            return std::make_pair(
                duration_cast<duration<double, std::ratio<60>>>(tns).count(),
                MINUTES);
        }
        return std::make_pair(
            duration_cast<duration<double, std::ratio<3600>>>(tns).count(),
            HOURS);
    }

    /** returns the value in ns */
    static nanoseconds getNSTime(std::pair<double, timeUnit> time) {
        switch (time.second) {
        case HOURS:
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double, std::ratio<3600>>(time.first));
        case MINUTES:
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double, std::ratio<60>>(time.first));
        case SECONDS:
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double>(time.first));
        case MILLISECONDS:
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double, std::milli>(time.first));
        case MICROSECONDS:
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double, std::micro>(time.first));
        default:
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double, std::nano>(time.first));
        }
    }

    /** returns the value in ms */
    static milliseconds getMSTime(std::pair<double, timeUnit> time) {
        switch (time.second) {
        case HOURS:
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::duration<double, std::ratio<3600>>(time.first));
        case MINUTES:
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::duration<double, std::ratio<60>>(time.first));
        case SECONDS:
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::duration<double>(time.first));
        case MILLISECONDS:
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::duration<double, std::milli>(time.first));
        case MICROSECONDS:
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::duration<double, std::micro>(time.first));
        default:
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::duration<double, std::nano>(time.first));
        }
    }

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
        return Message(qDefs::WARNING,
                       message + std::string("\nCaught exception:\n") +
                           exceptionMessage,
                       source);
    }
};

} // namespace sls
