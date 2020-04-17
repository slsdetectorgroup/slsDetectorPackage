#include "Receiver.h"
#include "ClientSocket.h"
#include "string_utils.h"
#include "versionAPI.h"
#include "ToString.h"

namespace sls {

void Receiver::sendToReceiver(int fnum, const void *args, size_t args_size,
                                 void *retval, size_t retval_size) {
    static_cast<const Receiver &>(*this).sendToReceiver(
        fnum, args, args_size, retval, retval_size);
}

void Receiver::sendToReceiver(int fnum, const void *args, size_t args_size,
                                 void *retval, size_t retval_size) const {
    auto receiver = ReceiverSocket(shm()->hostname, shm()->tcpPort);
    receiver.sendCommandThenRead(fnum, args, args_size, retval, retval_size);
    receiver.close();
}

template <typename Arg, typename Ret>
void Receiver::sendToReceiver(int fnum, const Arg &args, Ret &retval) {
    sendToReceiver(fnum, &args, sizeof(args), &retval, sizeof(retval));
}

template <typename Arg, typename Ret>
void Receiver::sendToReceiver(int fnum, const Arg &args, Ret &retval) const {
    sendToReceiver(fnum, &args, sizeof(args), &retval, sizeof(retval));
}

template <typename Arg>
void Receiver::sendToReceiver(int fnum, const Arg &args, std::nullptr_t) {
    sendToReceiver(fnum, &args, sizeof(args), nullptr, 0);
}

template <typename Arg>
void Receiver::sendToReceiver(int fnum, const Arg &args,
                                 std::nullptr_t) const {
    sendToReceiver(fnum, &args, sizeof(args), nullptr, 0);
}

template <typename Ret>
void Receiver::sendToReceiver(int fnum, std::nullptr_t, Ret &retval) {
    sendToReceiver(fnum, nullptr, 0, &retval, sizeof(retval));
}

template <typename Ret>
void Receiver::sendToReceiver(int fnum, std::nullptr_t, Ret &retval) const {
    sendToReceiver(fnum, nullptr, 0, &retval, sizeof(retval));
}

template <typename Ret>
Ret Receiver::sendToReceiver(int fnum){
    LOG(logDEBUG1) << "Sending: [" 
    << getFunctionNameFromEnum(static_cast<slsDetectorDefs::detFuncs>(fnum))
    << ", nullptr, 0, " << typeid(Ret).name() << ", " << sizeof(Ret) << "]";
    Ret retval{};
    sendToReceiver(fnum, nullptr, 0, &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << retval;
    return retval;
}

template <typename Ret>
Ret Receiver::sendToReceiver(int fnum) const{
    LOG(logDEBUG1) << "Sending: [" 
    << getFunctionNameFromEnum(static_cast<slsDetectorDefs::detFuncs>(fnum))
    << ", nullptr, 0, " << typeid(Ret).name() << ", " << sizeof(Ret) << "]";
    Ret retval{};
    sendToReceiver(fnum, nullptr, 0, &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << retval;
    return retval;
}

template <typename Ret, typename Arg>
Ret Receiver::sendToReceiver(int fnum, const Arg &args){
    LOG(logDEBUG1) << "Sending: [" 
    << getFunctionNameFromEnum(static_cast<slsDetectorDefs::detFuncs>(fnum))
    << ", " << args << ", " << sizeof(args) << ", " << typeid(Ret).name() << ", " << sizeof(Ret) << "]";
    Ret retval{};
    sendToReceiver(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << retval;
    return retval;
}

template <typename Ret, typename Arg>
Ret Receiver::sendToReceiver(int fnum, const Arg &args) const{
    LOG(logDEBUG1) << "Sending: [" 
    << getFunctionNameFromEnum(static_cast<slsDetectorDefs::detFuncs>(fnum))
    << ", " << args << ", " << sizeof(args) << ", " << typeid(Ret).name() << ", " << sizeof(Ret) << "]";
    Ret retval{};
    sendToReceiver(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << retval;
    return retval;
}


// create shm
Receiver::Receiver(int detector_id, int module_id, int receiver_id, 
    bool primaryInterface, int tcp_port, std::string hostname,
    int zmq_port) :
    receiverId(receiver_id), moduleId(module_id), 
    shm(detector_id, module_id, receiver_id, primaryInterface) {

    // ensure shared memory was not created before
    if (shm.IsExisting()) {
        LOG(logWARNING) << "This shared memory should have been "
                                "deleted before! "
                             << shm.GetName() << ". Freeing it again";
        shm.RemoveSharedMemory();
    }
    shm = SharedMemory<sharedReceiver>(detector_id, module_id, receiver_id, 
        primaryInterface);
    shm.CreateSharedMemory();

    // initalize receiver structure
    shm()->shmversion = RECEIVER_SHMVERSION;
    memset(shm()->hostname, 0, MAX_STR_LENGTH);
    shm()->tcpPort = DEFAULT_RX_PORTNO + receiver_id;
    shm()-> stoppedFlag = false;
    shm()->zmqPort = DEFAULT_ZMQ_RX_PORTNO + receiver_id;
    shm()->zmqIp = IpAddr{};

    // copy port, hostname if given
    if (tcp_port != 0) {
        setTCPPort(tcp_port);
    }
    if (zmq_port != 0) {
        shm()->zmqPort = zmq_port;
    }
    if (!hostname.empty()) {
        setHostname(hostname);
    }
}

// open shm
Receiver::Receiver(int detector_id, int module_id, int receiver_id, 
    bool primaryInterface, bool verify) :
    receiverId(receiver_id), moduleId(module_id),
    shm(detector_id, module_id, receiver_id, primaryInterface) {
    shm.OpenSharedMemory();
    if (verify && shm()->shmversion != RECEIVER_SHMVERSION) {
        std::ostringstream ss;
        ss << "Receiver shared memory (" << detector_id << "-" << moduleId
            << ":" << receiverId << ") version mismatch (expected 0x" << std::hex
            << RECEIVER_SHMVERSION << " but got 0x" << shm()->shmversion << ")"
            << std::dec << ". Clear Shared memory to continue.";
        throw SharedMemoryError(ss.str());
    }
}

Receiver::~Receiver() = default;

void Receiver::freeSharedMemory() {
    if (shm.IsExisting()) {
        shm.RemoveSharedMemory();
    }
}

std::string Receiver::getHostname() const {
    return shm()->hostname;
}

void Receiver::setHostname(const std::string &hostname) {
    if (hostname.empty()) {
        throw RuntimeError("Invalid receiver hostname. Cannot be empty.");
    }
    sls::strcpy_safe(shm()->hostname, hostname.c_str());
    configure();
}

int Receiver::getTCPPort() const {
    return shm()->tcpPort;
}

void Receiver::setTCPPort(const int port) {
    if (port >= 0 && port != shm()->tcpPort) {
        if (strlen(shm()->hostname) != 0) {
            // send to receiver to change tcpp port
            shm()->tcpPort = port; // for now
        } else {
            shm()->tcpPort = port;
        }
    }
}

void Receiver::configure() {
    LOG(logINFOBLUE) << receiverId  << " configured!";
    checkVersionCompatibility();
}

void Receiver::checkVersionCompatibility() {
    int64_t arg = APIRECEIVER;
    LOG(logDEBUG1)
        << "Checking version compatibility with receiver with value "
        << std::hex << arg << std::dec;
    sendToReceiver(F_RECEIVER_CHECK_VERSION, arg, nullptr);
}

void Receiver::start() {
    LOG(logDEBUG1) << "Starting Receiver";
    shm()->stoppedFlag = false;
    sendToReceiver(F_START_RECEIVER, nullptr, nullptr);
}

void Receiver::stop() {
    LOG(logDEBUG1) << "Stopping Receiver";
    int arg = static_cast<int>(shm()->stoppedFlag);
    sendToReceiver(F_STOP_RECEIVER, arg, nullptr);
}

slsDetectorDefs::runStatus Receiver::getStatus() const {
    runStatus retval = ERROR;
    LOG(logDEBUG1) << "Getting Receiver Status";
    sendToReceiver(F_GET_RECEIVER_STATUS, nullptr, retval);
    LOG(logDEBUG1) << "Receiver Status: " << ToString(retval);
    return retval;
}


int Receiver::getProgress() const {
    int retval = -1;
    sendToReceiver(F_GET_RECEIVER_PROGRESS, nullptr, retval);
    LOG(logDEBUG1) << "Current Progress of Receiver: " << retval;
    return retval;
}

void Receiver::setStoppedFlag() {
    shm()->stoppedFlag = true;
}

void Receiver::restreamStop() {
    LOG(logDEBUG1) << "Restream stop dummy from Receiver via zmq";
    sendToReceiver(F_RESTREAM_STOP_FROM_RECEIVER, nullptr, nullptr);
}


} // namespace sls