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

    /** END OF FIXED PATTERN -----------------------------------------------*/

    int zmqPort;
    sls::IpAddr zmqIp;

    };

    class Receiver : public virtual slsDetectorDefs {
        public:
        static size_t getNumReceivers();
        // create shm
        explicit Receiver(int detector_id, int module_id, int receiver_id, 
            int tcp_port = 0, std::string hostname = "");
        // open shm
        explicit Receiver(int detector_id, int module_id, int receiver_id, 
            bool verify);

        virtual ~Receiver();

        void setHostname(const std::string &ip_port);
        void updateReceiver();

        private:
        static size_t NUM_RECEIVERS;
        const int receiverId{0};
        mutable sls::SharedMemory<sharedReceiver> shm{0, 0, 0};
    };

}   // sls