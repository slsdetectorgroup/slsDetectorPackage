#pragma once
#include "SharedMemory.h"
#include "logger.h"
#include "sls_detector_defs.h"
#include "network_utils.h"

#include <map>

#define RECEIVER_SHMVERSION 0x200421

namespace sls {
    struct sharedReceiver {

    /* FIXED PATTERN FOR STATIC FUNCTIONS. DO NOT CHANGE, ONLY APPEND ------*/
    int shmversion;
    char hostname[MAX_STR_LENGTH];
    int tcpPort;
    /** END OF FIXED PATTERN -----------------------------------------------*/

    int stoppedFlag;
    int zmqPort;
    sls::IpAddr zmqIp;

    };

    class Receiver : public virtual slsDetectorDefs {
        public:
        static size_t getNumReceivers();
        // create shm
        explicit Receiver(int detector_id, int module_id, int interface_id,
            int receiver_id, int tcp_port = 0, std::string hostname = "",
            int zmq_port = 0);
        // open shm
        explicit Receiver(int detector_id, int module_id, int interface_id,
            int receiver_id, bool verify);

        virtual ~Receiver();

        void createIndexString();

        /**************************************************
        *                                                *
        *    Configuration                               *
        *                                                *
        * ************************************************/
        /**
        * Free shared memory and delete shared memory structure
        * occupied by the sharedReceiver structure
        * Is only safe to call if one deletes the Receiver object afterward
        * and frees multi shared memory/updates
        * thisMultiDetector->numberOfReceivers
        */
        void freeSharedMemory();
        std::string getHostname() const;
        void setHostname(const std::string &hostname);
        sls::MacAddr configure(slsDetectorDefs::rxParameters arg);
        int getTCPPort() const;
        void setTCPPort(const int port);
        std::string printConfiguration();
        int64_t getSoftwareVersion() const;


        /**************************************************
        *                                                *
        *    Acquisition Parameters                      *
        *                                                *
        * ************************************************/
        void setNumberOfFrames(int64_t value);


        /**************************************************
        *                                                *
        *    Acquisition                                 *
        *                                                *
        * ************************************************/
        void start();
        void stop();
        slsDetectorDefs::runStatus getStatus() const;
        int getProgress() const;
        void setStoppedFlag();
        void restreamStop();

        /**************************************************
        *                                                 *
        *    Network Configuration (Detector<->Receiver)  *
        *                                                 *
        * ************************************************/

        /**************************************************
        *                                                *
        *    File                                        *
        *                                                *
        * ************************************************/
        /**************************************************
        *                                                *
        *    ZMQ Streaming Parameters (Receiver<->Client)*
        *                                                *
        * ************************************************/
        
        /**************************************************
        *                                                *
        *    Detector Specific                           *
        *                                                *
        * ************************************************/
        // Eiger 
        void setQuad(const bool enable);
        void setReadNLines(const int value);

        // Moench
        /** empty vector deletes entire additional json header */
        void setAdditionalJsonHeader(const std::map<std::string, std::string> &jsonHeader);
        std::map<std::string, std::string> getAdditionalJsonHeader();

        /** Sets the value for the additional json header parameter key if found, 
        else append it. If value empty, then deletes parameter */
        void setAdditionalJsonParameter(const std::string &key, const std::string &value);
        std::string getAdditionalJsonParameter(const std::string &key);


        private:
        void sendToReceiver(int fnum, const void *args, size_t args_size,
                            void *retval, size_t retval_size);

        void sendToReceiver(int fnum, const void *args, size_t args_size,
                            void *retval, size_t retval_size) const;

        template <typename Arg, typename Ret>
        void sendToReceiver(int fnum, const Arg &args, Ret &retval);

        template <typename Arg, typename Ret>
        void sendToReceiver(int fnum, const Arg &args, Ret &retval) const;

        template <typename Arg>
        void sendToReceiver(int fnum, const Arg &args, std::nullptr_t);

        template <typename Arg>
        void sendToReceiver(int fnum, const Arg &args, std::nullptr_t) const;

        template <typename Ret>
        void sendToReceiver(int fnum, std::nullptr_t, Ret &retval);

        template <typename Ret>
        void sendToReceiver(int fnum, std::nullptr_t, Ret &retval) const;

        template <typename Ret>
        Ret sendToReceiver(int fnum);

        template <typename Ret>
        Ret sendToReceiver(int fnum) const;

        template <typename Ret, typename Arg>
        Ret sendToReceiver(int fnum, const Arg &args);

        template <typename Ret, typename Arg>
        Ret sendToReceiver(int fnum, const Arg &args) const;

        void checkVersionCompatibility();
        const int receiverId{0};
        const int interfaceId{0};
        const int moduleId{0};
        std::string indexString;
        mutable sls::SharedMemory<sharedReceiver> shm{0, 0, 0, 0};
    };

}   // sls