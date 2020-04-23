#include "Receiver.h"
#include "ClientSocket.h"
#include "FixedCapacityContainer.h"
#include "string_utils.h"
#include "versionAPI.h"
#include "ToString.h"

namespace sls {

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

    LOG(logINFOBLUE) <<  "reciever " << indexString << " configured!";
    return mac;
}

std::string Receiver::printConfiguration() {
    std::ostringstream oss;
    
    oss << std::endl << std::endl
        << "Receiver " << indexString << std::endl
        << "Hostname : " << shm()->hostname << std::endl
        << "Tcp port : " << shm()->tcpPort << std::endl;

        
    /*
        
    os << "\nReceiver UDP IP:\t" 
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
    oss << "\n";
    return oss.str();
}

int64_t Receiver::getSoftwareVersion() const {
    LOG(logDEBUG1) << "Getting receiver software version";
    return sendToReceiver<int64_t>(F_GET_RECEIVER_VERSION);
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

/** Network Configuration (Detector<->Receiver) */
sls::MacAddr Receiver::setUDPIP(const IpAddr ip) {
    LOG(logDEBUG1) << "Setting udp ip to receier: " << ip;
    if (ip == 0) {
        throw RuntimeError("Invalid destination udp ip address");
    }
    sls::MacAddr retval(0LU);
    sendToReceiver(F_SET_RECEIVER_UDP_IP, ip, retval);
    return retval;
}

void Receiver::setUDPPort(const int port) {
    LOG(logDEBUG1) << "Setting udp port to  receiver: " << port;
    sendToReceiver(F_SET_RECEIVER_UDP_PORT, port, nullptr); 
}

/** ZMQ Streaming Parameters (Receiver<->Client) */
int Receiver::getClientZmqPort() const { 
    return shm()->zmqPort; 
}

void Receiver::setClientZmqPort(const int port) { 
    shm()->zmqPort = port;
}

int Receiver::getZmqPort() const {    
    return sendToReceiver<int>(F_GET_RECEIVER_STREAMING_PORT);
}

void Receiver::setZmqPort(int port) {
    sendToReceiver(F_SET_RECEIVER_STREAMING_PORT, port, nullptr);
}


sls::IpAddr Receiver::getClientZmqIP() { 
    return shm()->zmqIp; 
}

void Receiver::setClientZmqIP(const sls::IpAddr ip) {
    LOG(logDEBUG1) << "Setting client zmq ip to " << ip;
    if (ip == 0) {
        throw RuntimeError("Invalid client zmq ip address");
    } 
    shm()->zmqIp = ip;  
}

sls::IpAddr Receiver::getZmqIP() {
    return sendToReceiver<sls::IpAddr>(F_GET_RECEIVER_STREAMING_SRC_IP);
}

void Receiver::setZmqIP(const sls::IpAddr ip) {
    if (ip == 0) {
        throw RuntimeError("Invalid receiver zmq ip address");
    }
   
    // if client zmqip is empty, update it
    if (shm()->zmqIp == 0) {
        shm()->zmqIp = ip;
    }
    sendToReceiver(F_SET_RECEIVER_STREAMING_SRC_IP, ip, nullptr);
}

/** Receiver Parameters */

int64_t Receiver::getUDPSocketBufferSize() const {
    return sendToReceiver<int64_t>(F_GET_RECEIVER_UDP_SOCK_BUF_SIZE);
}

void Receiver::setUDPSocketBufferSize(int64_t value) {
    LOG(logDEBUG1) << "Sending UDP Socket Buffer size to receiver: "
                        << value;
    sendToReceiver(F_SET_RECEIVER_UDP_SOCK_BUF_SIZE, value, nullptr);
}

int64_t Receiver::getRealUDPSocketBufferSize() const {
    return sendToReceiver<int64_t>(F_GET_RECEIVER_REAL_UDP_SOCK_BUF_SIZE);
}


/** Detector Parameters */

void Receiver::setNumberOfFrames(int64_t value) {
    LOG(logDEBUG1) << "Sending number of frames to Receiver: " << value;
    sendToReceiver(F_RECEIVER_SET_NUM_FRAMES, value, nullptr);   
}

void Receiver::setNumberOfTriggers(int64_t value) {
    LOG(logDEBUG1) << "Sending number of triggers to Receiver: " << value;
    sendToReceiver(F_SET_RECEIVER_NUM_TRIGGERS, value, nullptr);   
}

void Receiver::setNumberOfBursts(int64_t value) {
    LOG(logDEBUG1) << "Sending number of bursts to Receiver: " << value;
    sendToReceiver(F_SET_RECEIVER_NUM_BURSTS, value, nullptr);   
}

void Receiver::setNumberOfAnalogSamples(int value) {
    LOG(logDEBUG1) << "Sending number of analog samples to Receiver: " << value;
    sendToReceiver(F_RECEIVER_SET_NUM_ANALOG_SAMPLES, value, nullptr);   
}

void Receiver::setNumberOfDigitalSamples(int value) {
    LOG(logDEBUG1) << "Sending number of digital samples to Receiver: " << value;
    sendToReceiver(F_RECEIVER_SET_NUM_DIGITAL_SAMPLES, value, nullptr);   
}

void Receiver::setExptime(int64_t value) {
    LOG(logDEBUG1) << "Sending exptime to Receiver: " << value;
    sendToReceiver(F_RECEIVER_SET_EXPTIME, value, nullptr);   
}

void Receiver::setPeriod(int64_t value) {
    LOG(logDEBUG1) << "Sending period to Receiver: " << value;
    sendToReceiver(F_RECEIVER_SET_PERIOD, value, nullptr);   
}

void Receiver::setSubExptime(int64_t value) {
    LOG(logDEBUG1) << "Sending sub exptime to Receiver: " << value;
    sendToReceiver(F_RECEIVER_SET_SUB_EXPTIME, value, nullptr);   
}

void Receiver::setSubDeadTime(int64_t value) {
    LOG(logDEBUG1) << "Sending sub deadtime to Receiver: " << value;
    sendToReceiver(F_RECEIVER_SET_SUB_DEADTIME, value, nullptr);   
}

void Receiver::setTimingMode(timingMode value) {
    LOG(logDEBUG1) << "Sending timing mode to Receiver: " << value;
    sendToReceiver(F_SET_RECEIVER_TIMING_MODE, value, nullptr);   
}

void Receiver::setDynamicRange(int n) {
    int retval = -1;
    LOG(logDEBUG1) << "Sending dynamic range to receiver: " << n;
    sendToReceiver(F_SET_RECEIVER_DYNAMIC_RANGE, n, retval);
}

void Receiver::setReadoutMode(const slsDetectorDefs::readoutMode mode) {
    sendToReceiver(F_RECEIVER_SET_READOUT_MODE, mode, nullptr);
}

void Receiver::setQuad(const bool enable) {
    int value = enable ? 1 : 0;
    LOG(logDEBUG1) << "Setting Quad type to " << value << " in Receiver";
    sendToReceiver(F_SET_RECEIVER_QUAD, value, nullptr);
}

void Receiver::setReadNLines(const int value) {
    LOG(logDEBUG1) << "Setting read n lines to " << value
                        << " in Receiver";
    sendToReceiver(F_SET_RECEIVER_READ_N_LINES, value, nullptr);
}

void Receiver::setADCEnableMask(uint32_t mask) {
    sendToReceiver(F_RECEIVER_SET_ADC_MASK, mask, nullptr);
}

void Receiver::setTenGigaADCEnableMask(uint32_t mask) {
    sendToReceiver(F_RECEIVER_SET_ADC_MASK_10G, mask, nullptr);
}

void Receiver::setBurstMode(slsDetectorDefs::burstMode value) {
    LOG(logDEBUG1) << "Sending burst mode to Receiver: " << value;
    sendToReceiver(F_SET_RECEIVER_BURST_MODE, value, nullptr);   
}

void Receiver::setROI(slsDetectorDefs::ROI arg) {
    std::array<int, 2> args{arg.xmin, arg.xmax};
    LOG(logDEBUG1) << "Sending ROI to receiver";
    sendToReceiver(F_RECEIVER_SET_ROI, args, nullptr);
}

void Receiver::clearROI() {
    LOG(logDEBUG1) << "Clearing ROI";
    slsDetectorDefs::ROI arg;
    arg.xmin = -1;
    arg.xmax = -1;
    setROI(arg);
}

std::vector<int> Receiver::getDbitList() const {
    sls::FixedCapacityContainer<int, MAX_RX_DBIT> retval;
    sendToReceiver(F_GET_RECEIVER_DBIT_LIST, nullptr, retval);
    return retval;    
}

void Receiver::setDbitList(const std::vector<int>& list) {
    LOG(logDEBUG1) << "Setting Receiver Dbit List";
    if (list.size() > 64) {
        throw sls::RuntimeError("Dbit list size cannot be greater than 64\n");
    }
    for (auto &it : list) {
        if (it < 0 || it > 63) {
            throw sls::RuntimeError(
                "Dbit list value must be between 0 and 63\n");
        }
    }
    sls::FixedCapacityContainer<int, MAX_RX_DBIT> arg = list;
    sendToReceiver(F_SET_RECEIVER_DBIT_LIST, arg, nullptr);        
}

int Receiver::getDbitOffset() {
    return sendToReceiver<int>(F_GET_RECEIVER_DBIT_OFFSET);    
}

void Receiver::setDbitOffset(int value) {
    sendToReceiver(F_SET_RECEIVER_DBIT_OFFSET, value, nullptr);        
}

void Receiver::setActivate(const bool enable) {
    int arg = static_cast<int>(enable);
    sendToReceiver(F_RECEIVER_ACTIVATE, arg, nullptr);        
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

std::string Receiver::getAdditionalJsonParameter(const std::string &key) {
    char arg[SHORT_STR_LENGTH]{};
    sls::strcpy_safe(arg, key.c_str());
    char retval[SHORT_STR_LENGTH]{};
    sendToReceiver(F_GET_ADDITIONAL_JSON_PARAMETER, arg, retval); 
    return retval;
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

} // namespace sls