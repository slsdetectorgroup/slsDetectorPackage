#include "Receiver.h"
#include "string_utils.h"

namespace sls {

size_t Receiver::NUM_RECEIVERS{0};

size_t Receiver::getNumReceivers() {
    return NUM_RECEIVERS;
}


// create shm
Receiver::Receiver(int detector_id, int module_id, int receiver_id, 
    int tcp_port, std::string hostname)
    : receiverId(receiver_id), shm(detector_id, module_id, receiver_id) {

    // ensure shared memory was not created before
    if (shm.IsExisting()) {
        LOG(logWARNING) << "This shared memory should have been "
                                "deleted before! "
                             << shm.GetName() << ". Freeing it again";
        shm.RemoveSharedMemory();
    }
    shm = SharedMemory<sharedReceiver>(detector_id, module_id, receiver_id);
    ++NUM_RECEIVERS;

    // initalize receiver structure
    shm()->shmversion = RECEIVER_SHMVERSION;
    memset(shm()->hostname, 0, MAX_STR_LENGTH);
    shm()->tcpPort = DEFAULT_RX_PORTNO + NUM_RECEIVERS - 1;
    shm()->zmqPort = DEFAULT_ZMQ_RX_PORTNO + NUM_RECEIVERS - 1;
    shm()->zmqIp = IpAddr{};

    // copy port, hostname if given
    if (tcp_port != 0) {
        shm()->tcpPort = tcp_port;
    }
    if (!hostname.empty()) {
        setHostname(hostname);
    }
}

// open shm
Receiver::Receiver(int detector_id, int module_id, int receiver_id, 
    bool verify)
    : receiverId(receiver_id), shm(detector_id, module_id, receiver_id) {
    shm.OpenSharedMemory();
    if (verify && shm()->shmversion != RECEIVER_SHMVERSION) {
        std::ostringstream ss;
        ss << "Receiver shared memory (" << detector_id << "-" << receiverId
            << ":) version mismatch (expected 0x" << std::hex
            << RECEIVER_SHMVERSION << " but got 0x" << shm()->shmversion << ")"
            << std::dec << ". Clear Shared memory to continue.";
        throw SharedMemoryError(ss.str());
    }
}

Receiver::~Receiver() {
    --NUM_RECEIVERS;
}

void Receiver::setHostname(const std::string &ip_port) {
    if (ip_port.empty()) {
        throw RuntimeError("Invalid receiver hostname. Cannot be empty.");
    }
    // parse tcp port from this hostname:port string
    std::string host = ip_port;
    auto res = sls::split(host, ':');
    if (res.size() > 1) {
        host = res[0];
        shm()->tcpPort = std::stoi(res[1]);
    }    
    sls::strcpy_safe(shm()->hostname, host.c_str());

    updateReceiver();
}

void Receiver::updateReceiver() {
    //checkReceiverVersionCompatibility();
}


} // namespace sls