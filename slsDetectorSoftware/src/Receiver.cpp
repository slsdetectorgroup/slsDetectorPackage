#include "Receiver.h"
#include "string_utils.h"

namespace sls {


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
    shm()->valid = false;
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

void Receiver::configure() {
    shm()->valid = false;
    LOG(logINFOBLUE) << receiverId  << " configured!";
    //checkReceiverVersionCompatibility();
    shm()->valid = true;
}

int Receiver::getTCPPort() const {
    return shm()->tcpPort;
}

void Receiver::setTCPPort(const int port) {
    if (port >= 0 && port != shm()->tcpPort) {
        if (shm()->valid) {
            // send to receiver to change tcpp port
            shm()->tcpPort = port; // for now
        } else {
            shm()->tcpPort = port;
        }
    }
}

} // namespace sls