#include "sls_detector_exceptions.h"
#include "logger.h"
namespace sls {
RuntimeError::RuntimeError() : runtime_error("SLS Detector Package Failed") {
    LOG(logERROR) << "SLS Detector Package Failed";
}
RuntimeError::RuntimeError(const std::string &msg) : runtime_error(msg) {
    LOG(logERROR) << msg;
}
RuntimeError::RuntimeError(const char *msg) : runtime_error(msg) {
    LOG(logERROR) << msg;
}
SharedMemoryError::SharedMemoryError(const std::string &msg)
    : RuntimeError(msg) {}
SocketError::SocketError(const std::string &msg) : RuntimeError(msg) {}
ZmqSocketError::ZmqSocketError(const std::string &msg) : RuntimeError(msg) {}
NotImplementedError::NotImplementedError(const std::string &msg)
    : RuntimeError(msg) {}
DetectorError::DetectorError(const std::string &msg) : RuntimeError(msg) {}
ReceiverError::ReceiverError(const std::string &msg) : RuntimeError(msg) {}
GuiError::GuiError(const std::string &msg) : RuntimeError(msg) {}

} // namespace sls
