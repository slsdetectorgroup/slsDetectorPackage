#pragma once
#include "SharedMemory.h"
#include "logger.h"
#include "sls_detector_defs.h"
#include "network_utils.h"

#define RECEIVER_SHMVERSION 0x200414

namespace sls {
    struct sharedReceiver {

    /* FIXED PATTERN FOR STATIC FUNCTIONS. DO NOT CHANGE, ONLY APPEND ------*/

    int shmversion;
    char hostname[MAX_STR_LENGTH];
    int tcpPort;
    bool valid;

    /** END OF FIXED PATTERN -----------------------------------------------*/

    int zmqPort;
    sls::IpAddr zmqIp;

    };

    class Receiver : public virtual slsDetectorDefs {
        public:
        static size_t getNumReceivers();
        // create shm
        explicit Receiver(int detector_id, int module_id, int receiver_id, 
            bool primaryInterface, int tcp_port = 0, std::string hostname = "",
            int zmq_port = 0);
        // open shm
        explicit Receiver(int detector_id, int module_id, int receiver_id, 
            bool primaryInterface, bool verify);

        virtual ~Receiver();


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
        void configure();
        int getTCPPort() const;
        void setTCPPort(const int port);

        private:
        const int receiverId{0};
        const int moduleId{0};
        mutable sls::SharedMemory<sharedReceiver> shm{0, 0, 0, true};
    };

}   // sls