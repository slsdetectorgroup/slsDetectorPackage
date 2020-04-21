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
    if (strlen(shm()->hostname) == 0) {
        throw RuntimeError("Reciver not added");
    }
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
    << ", " << args << ", " << sizeof(args) << ", " << typeid(Ret).name() 
    << ", " << sizeof(Ret) << "]";
    Ret retval{};
    sendToReceiver(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << retval;
    return retval;
}

template <typename Ret, typename Arg>
Ret Receiver::sendToReceiver(int fnum, const Arg &args) const{
    LOG(logDEBUG1) << "Sending: [" 
    << getFunctionNameFromEnum(static_cast<slsDetectorDefs::detFuncs>(fnum))
    << ", " << args << ", " << sizeof(args) << ", " << typeid(Ret).name() 
    << ", " << sizeof(Ret) << "]";
    Ret retval{};
    sendToReceiver(fnum, &args, sizeof(args), &retval, sizeof(retval));
    LOG(logDEBUG1) << "Got back: " << retval;
    return retval;
}


// create shm
Receiver::Receiver(int detector_id, int module_id,  int interface_id, 
    int receiver_id, int tcp_port, std::string hostname,
    int zmq_port) :
    receiverId(receiver_id), interfaceId(interface_id), moduleId(module_id), 
    shm(detector_id, module_id, interface_id, receiver_id) {
    createIndexString();   

    // ensure shared memory was not created before
    if (shm.IsExisting()) {
        LOG(logWARNING) << "This shared memory should have been deleted "
        "before! " << shm.GetName() << ". Freeing it again";
        shm.RemoveSharedMemory();
    }
    shm = SharedMemory<sharedReceiver>(detector_id, module_id, interface_id,
        receiver_id);
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
Receiver::Receiver(int detector_id, int module_id,  int interface_id, 
    int receiver_id, bool verify) :
    receiverId(receiver_id), interfaceId(interface_id), moduleId(module_id),
    shm(detector_id, module_id, interface_id, receiver_id) {
    createIndexString();   

    shm.OpenSharedMemory();
    if (verify && shm()->shmversion != RECEIVER_SHMVERSION) {
        std::ostringstream ss;
        ss << "Receiver shared memory (" << detector_id << "-" << indexString
            << ":" << receiverId << ") version mismatch (expected 0x" << std::hex
            << RECEIVER_SHMVERSION << " but got 0x" << shm()->shmversion << ")"
            << std::dec << ". Clear Shared memory to continue.";
        throw SharedMemoryError(ss.str());
    }
}

Receiver::~Receiver() = default;

void Receiver::createIndexString() {
    std::ostringstream oss;
    oss << '(' << moduleId << (char)(interfaceId + 97) << "." << receiverId << ')';
    indexString = oss.str();
}

/** Configuration */

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
    checkVersionCompatibility();
}

int Receiver::getTCPPort() const {
    return shm()->tcpPort;
}

void Receiver::setTCPPort(const int port) {
    LOG(logDEBUG1) << "Setting reciever port to " << port;
    if (port >= 0 && port != shm()->tcpPort) {
        if (strlen(shm()->hostname) != 0) {
            int retval = -1;
            sendToReceiver(F_SET_RECEIVER_PORT, port, retval);
            shm()->tcpPort = retval;
            LOG(logDEBUG1) << "Receiver port: " << retval;
        } else {
            shm()->tcpPort = port;
        }
    }
}

void Receiver::checkVersionCompatibility() {
    int64_t arg = APIRECEIVER;
    LOG(logDEBUG1)
        << "Checking version compatibility with receiver with value "
        << std::hex << arg << std::dec;
    sendToReceiver(F_RECEIVER_CHECK_VERSION, arg, nullptr);
}

sls::MacAddr Receiver::configure(slsDetectorDefs::rxParameters arg) {
    // hostname
    memset(arg.hostname, 0, sizeof(arg.hostname));
    strcpy_safe(arg.hostname, shm()->hostname);
    // interface id
    arg.interfaceId = interfaceId;
    // zmqip
    {
        sls::IpAddr ip;
        // Hostname could be ip try to decode otherwise look up the hostname
        ip = sls::IpAddr{shm()->hostname};
        if (ip == 0) {
            ip = HostnameToIp(shm()->hostname);
        }  
        LOG(logINFO) << "Setting default receiver " << indexString 
            << " streaming zmq ip to " << ip;  
        // if client zmqip is empty, update it
        if (shm()->zmqIp == 0) {
            shm()->zmqIp = ip;
        }   
        memcpy(&arg.zmq_ip, &ip, sizeof(ip));
    } 

    LOG(logDEBUG1) 
        << "detType:" << arg.detType << std::endl
        << "detectorSize.x:" << arg.detectorSize.x << std::endl
        << "detectorSize.y:" << arg.detectorSize.y << std::endl
        << "moduleId:" << arg.moduleId << std::endl
        << "hostname:" << arg.hostname << std::endl
        << "interfaceId: " << arg.interfaceId << std::endl
        << "zmq ip:" << arg.zmq_ip << std::endl
        << "udpInterfaces:" << arg.udpInterfaces << std::endl
        << "udp_dstport:" << arg.udp_dstport << std::endl
        << "udp_dstip:" << sls::IpAddr(arg.udp_dstip) << std::endl
        << "udp_dstmac:" << sls::MacAddr(arg.udp_dstmac) << std::endl
        << "udp_dstport2:" << arg.udp_dstport2 << std::endl
        << "udp_dstip2:" << sls::IpAddr(arg.udp_dstip2) << std::endl
        << "udp_dstmac2:" << sls::MacAddr(arg.udp_dstmac2) << std::endl
        << "frames:" << arg.frames << std::endl
        << "triggers:" << arg.triggers << std::endl
        << "bursts:" << arg.bursts << std::endl
        << "analogSamples:" << arg.analogSamples << std::endl
        << "digitalSamples:" << arg.digitalSamples << std::endl
        << "expTimeNs:" << arg.expTimeNs << std::endl
        << "periodNs:" << arg.periodNs << std::endl
        << "subExpTimeNs:" << arg.subExpTimeNs << std::endl
        << "subDeadTimeNs:" << arg.subDeadTimeNs << std::endl
        << "activate:" << arg.activate << std::endl
        << "quad:" << arg.quad << std::endl
        << "dynamicRange:" << arg.dynamicRange << std::endl
        << "timMode:" << arg.timMode << std::endl
        << "tenGiga:" << arg.tenGiga << std::endl
        << "roMode:" << arg.roMode << std::endl
        << "adcMask:" << arg.adcMask << std::endl
        << "adc10gMask:" << arg.adc10gMask << std::endl
        << "roi.xmin:" << arg.roi.xmin << std::endl
        << "roi.xmax:" << arg.roi.xmax << std::endl
        << "countermask:" << arg.countermask << std::endl
        << "burstType:" << arg.burstType << std::endl;

    sls::MacAddr mac;
    {
        sls::MacAddr retval;
        sendToReceiver(F_SETUP_RECEIVER, arg, retval);
        // detector does not have customized udp mac
        if (arg.udp_dstmac == 0) {
            mac = retval;
        }
    } 
    if (arg.detType == MOENCH) {
        setAdditionalJsonParameter("adcmask_1g", std::to_string(arg.adcMask));
        setAdditionalJsonParameter("adcmask_10g", std::to_string(arg.adc10gMask));
    }

    LOG(logINFOBLUE) << receiverId  << " configured!";
    return mac;
}

std::string Receiver::printConfiguration() {
    std::ostringstream os;
    
    os << "\n\nModuler " << indexString 
        << "\nReceiver Hostname:\t"<< shm()->hostname;
    /*
    if (shm()->myDetectorType == JUNGFRAU) {  
        os << "\nNumber of Interfaces:\t" << getNumberofUDPInterfaces() 
        << "\nSelected Interface:\t" << getSelectedUDPInterface();
    }
        
    os << "\nDetector UDP IP:\t"
       << getSourceUDPIP() << "\nDetector UDP MAC:\t" 
       << getSourceUDPMAC() << "\nReceiver UDP IP:\t" 
       << getDestinationUDPIP() << "\nReceiver UDP MAC:\t" << getDestinationUDPMAC();

    if (shm()->myDetectorType == JUNGFRAU) {
        os << "\nDetector UDP IP2:\t" << getSourceUDPIP2() 
       << "\nDetector UDP MAC2:\t" << getSourceUDPMAC2()
       << "\nReceiver UDP IP2:\t" << getDestinationUDPIP2()      
       << "\nReceiver UDP MAC2:\t" << getDestinationUDPMAC2();
    }
    os << "\nReceiver UDP Port:\t" << getDestinationUDPPort();
    if (shm()->myDetectorType == JUNGFRAU || shm()->myDetectorType == EIGER) {
       os << "\nReceiver UDP Port2:\t" << getDestinationUDPPort2();
    }
    */
    os << "\n";
    return os.str();
}

/** Acquisition */

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

/** Detector Specific */
// Moench

void Receiver::setAdditionalJsonHeader(const std::map<std::string, std::string> &jsonHeader) {
    for (auto &it : jsonHeader) {
        if (it.first.empty() || it.first.length() > SHORT_STR_LENGTH ||
            it.second.length() > SHORT_STR_LENGTH ) {
            throw RuntimeError(it.first + " or " + it.second + " pair has invalid size. "
            "Key cannot be empty. Both can have max 20 characters");
        }
    }
    const int size = jsonHeader.size();
    int fnum = F_SET_ADDITIONAL_JSON_HEADER;
    int ret = FAIL;
    LOG(logDEBUG) << "Sending to receiver additional json header " << ToString(jsonHeader);
    auto client = ReceiverSocket(shm()->hostname, shm()->tcpPort);
    client.Send(&fnum, sizeof(fnum));
    client.Send(&size, sizeof(size));
    if (size > 0) {
        char args[size * 2][SHORT_STR_LENGTH];
        memset(args, 0, sizeof(args));
        int iarg = 0;
        for (auto &it : jsonHeader) {
            sls::strcpy_safe(args[iarg], it.first.c_str());
            sls::strcpy_safe(args[iarg + 1], it.second.c_str());
            iarg += 2;
        }
        client.Send(args, sizeof(args));
    }
    client.Receive(&ret, sizeof(ret));
    if (ret == FAIL) {
        char mess[MAX_STR_LENGTH]{};
        client.Receive(mess, MAX_STR_LENGTH);
        throw RuntimeError("Receiver " + std::to_string(moduleId) +
                           " returned error: " + std::string(mess));
    }
}

std::map<std::string, std::string> Receiver::getAdditionalJsonHeader() {
    int fnum = F_GET_ADDITIONAL_JSON_HEADER;
    int ret = FAIL;
    int size = 0;
    auto client = ReceiverSocket(shm()->hostname, shm()->tcpPort);
    client.Send(&fnum, sizeof(fnum));
    client.Receive(&ret, sizeof(ret));
    if (ret == FAIL) {
        char mess[MAX_STR_LENGTH]{};
        client.Receive(mess, MAX_STR_LENGTH);
        throw RuntimeError("Receiver " + std::to_string(moduleId) +
                           " returned error: " + std::string(mess));
    } else {
        client.Receive(&size, sizeof(size));
        std::map<std::string, std::string> retval;
        if (size > 0) {
            char retvals[size * 2][SHORT_STR_LENGTH];
            memset(retvals, 0, sizeof(retvals));
            client.Receive(retvals, sizeof(retvals));
            for (int i = 0; i < size; ++i) {
                retval[retvals[2 * i]] = retvals[2 * i + 1];
            }
        }
        LOG(logDEBUG) << "Getting additional json header " << ToString(retval);
        return retval;
    }
}

void Receiver::setAdditionalJsonParameter(const std::string &key, const std::string &value) {
    if (key.empty() || key.length() > SHORT_STR_LENGTH ||
        value.length() > SHORT_STR_LENGTH ) {
        throw RuntimeError(key + " or " + value + " pair has invalid size. "
        "Key cannot be empty. Both can have max 2 characters");
    }
    char args[2][SHORT_STR_LENGTH]{};
    sls::strcpy_safe(args[0], key.c_str());
    sls::strcpy_safe(args[1], value.c_str());
    sendToReceiver(F_SET_ADDITIONAL_JSON_PARAMETER, args, nullptr);
}

std::string Receiver::getAdditionalJsonParameter(const std::string &key) {
    char arg[SHORT_STR_LENGTH]{};
    sls::strcpy_safe(arg, key.c_str());
    char retval[SHORT_STR_LENGTH]{};
    sendToReceiver(F_GET_ADDITIONAL_JSON_PARAMETER, arg, retval); 
    return retval;
}

} // namespace sls