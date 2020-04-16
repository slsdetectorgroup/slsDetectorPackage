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
        int getTCPPort() const;
        void setTCPPort(const int port);
        void configure();
        void checkVersionCompatibility();

        private:
            /**
     * Send function parameters to receiver
     * @param fnum function enum
     * @param args argument pointer
     * @param args_size size of argument
     * @param retval return pointers
     * @param retval_size size of return value
     */
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


        const int receiverId{0};
        const int moduleId{0};
        mutable sls::SharedMemory<sharedReceiver> shm{0, 0, 0, true};
    };

}   // sls