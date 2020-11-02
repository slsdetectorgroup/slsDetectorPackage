#pragma once

#include <string>
#include <stdexcept>
#include <chrono>

//#include "sls/sls_detector_exceptions.h"
//#include "ansi.h"
#define RED    	 	"\x1b[31m"
#define RESET   	"\x1b[0m"
#define BOLD    	"\x1b[1m"
#define cprintf(code, format, ...) printf(code format RESET, ##__VA_ARGS__)


#define CATCH_DISPLAY(m, s) catch(...) { ctbDefs::DisplayExceptions(m, s); }
#define CATCH_HANDLE(...) catch(...) { ctbDefs::HandleExceptions(__VA_ARGS__); }

class ctbDefs {
  public:
    /**
     * Empty Constructor
     */
    ctbDefs(){};

    // convert double seconds to chrono ns
    static std::chrono::nanoseconds ConvertDoubleStoChronoNS(double timeS) {
        using std::chrono::duration;
        using std::chrono::duration_cast;
        using std::chrono::nanoseconds; 
        return duration_cast<nanoseconds>(duration<double>(timeS));
    }

    // convert chrono ns to doubel s
    static double ConvertChronoNStoDoubleS(std::chrono::nanoseconds timeNs) {
        using std::chrono::duration;
        using std::chrono::duration_cast;    
        return duration_cast<duration<double>>(timeNs).count();
    }

    static void DisplayExceptions(std::string emsg, std::string src) {
        try {
            throw;
        } /* catch (const sls::SocketError &e) {
            throw;
        } catch (const sls::SharedMemoryError &e) {
            throw;
        } */catch (const std::exception &e) {
            ExceptionMessage(emsg, e.what(), src);
        }
    };

    template <class CT> struct NonDeduced { using type = CT; };
    template <class S, typename RT, typename... CT>
    static void HandleExceptions(const std::string emsg, const std::string src, S* s,
                                    RT (S::*somefunc)(CT...),
                                    typename NonDeduced<CT>::type... Args) {
        try {
            throw;
        } /*catch (const sls::SocketError &e) {
            throw;
        } catch (const sls::SharedMemoryError &e) {
            throw;
        } */catch (const std::exception &e) {

            ExceptionMessage(emsg, e.what(), src);
            (s->*somefunc)(Args...);
        }
    };

    static void ExceptionMessage(std::string message, 
                    std::string exceptionMessage,
                    std::string source) {
        // because sls_detector_exceptions cannot be included
      if (exceptionMessage.find("hared memory") != std::string::npos) {
            throw;
        }
      if (exceptionMessage.find("annot connect") != std::string::npos) {
            throw;
        }
        cprintf(RED, "Warning (%s): %s [Caught Exception: %s]\n", source.c_str(), message.c_str(), exceptionMessage.c_str());
        //return Message(qDefs::WARNING, message + std::string("\nCaught exception:\n") + exceptionMessage, source);
    }; 

};
