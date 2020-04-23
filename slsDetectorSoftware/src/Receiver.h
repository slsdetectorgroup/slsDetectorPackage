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
    sls::MacAddr setUDPIP(const sls::IpAddr ip);
    void setUDPPort(int udpport);

    /**************************************************
    *                                                *
    *    ZMQ Streaming Parameters (Receiver<->Client)*
    *                                                *
    * ************************************************/
    int getClientZmqPort() const;
    void setClientZmqPort(const int port);
    int getZmqPort() const; 
    void setZmqPort(int port);
    sls::IpAddr getClientZmqIP();
    void setClientZmqIP(const sls::IpAddr ip);
    sls::IpAddr getZmqIP();
    void setZmqIP(const sls::IpAddr ip);

    /**************************************************
    *                                                *
    *    Receiver Parameters                         *
    *                                                *
    * ************************************************/
    int64_t getUDPSocketBufferSize() const;
    void setUDPSocketBufferSize(int64_t value);
    int64_t getRealUDPSocketBufferSize() const;

    /**************************************************
    *                                                *
    *    Detector Parameters                         *
    *                                                *
    * ************************************************/
    void setNumberOfFrames(int64_t value);
    void setNumberOfTriggers(int64_t value);
    void setNumberOfBursts(int64_t value);
    void setNumberOfAnalogSamples(int value);
    void setNumberOfDigitalSamples(int value);
    void setExptime(int64_t value);
    void setPeriod(int64_t value);
    void setSubExptime(int64_t value);
    void setSubDeadTime(int64_t value);
    void setTimingMode(timingMode value);
    void setDynamicRange(int n);
    void setReadoutMode(const readoutMode mode);
    void setQuad(const bool enable);
    void setReadNLines(const int value);
    void setADCEnableMask(uint32_t mask);
    void setTenGigaADCEnableMask(uint32_t mask);
    void setBurstMode(burstMode value);
    void setROI(slsDetectorDefs::ROI arg);
    void clearROI();
    std::vector<int> getDbitList() const;
    /** digital data bits enable (CTB only) */
    void setDbitList(const std::vector<int>& list);
    int getDbitOffset();
    /** Set digital data offset in bytes (CTB only) */
    void setDbitOffset(int value);
    void setActivate(const bool enable);

    std::map<std::string, std::string> getAdditionalJsonHeader();
    /** empty vector deletes entire additional json header */
    void setAdditionalJsonHeader(const std::map<std::string, std::string> &jsonHeader);
    std::string getAdditionalJsonParameter(const std::string &key);
    /** Sets the value for the additional json header parameter key if found, 
    else append it. If value empty, then deletes parameter */
    void setAdditionalJsonParameter(const std::string &key, const std::string &value);


    /**************************************************
    *                                                *
    *    File                                        *
    *                                                *
    * ************************************************/

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