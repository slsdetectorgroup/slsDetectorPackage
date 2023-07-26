// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include <stdexcept>

namespace sls {

class RuntimeError : public std::runtime_error {
  public:
    RuntimeError();
    explicit RuntimeError(const std::string &msg);
    explicit RuntimeError(const char *msg);
};

class SharedMemoryError : public RuntimeError {
  public:
    explicit SharedMemoryError(const std::string &msg);
};

class SocketError : public RuntimeError {
  public:
    explicit SocketError(const std::string &msg);
};

class ZmqSocketError : public RuntimeError {
  public:
    explicit ZmqSocketError(const std::string &msg);
};

class NotImplementedError : public RuntimeError {
  public:
    explicit NotImplementedError(const std::string &msg);
};

class DetectorError : public RuntimeError {
  public:
    explicit DetectorError(const std::string &msg);
};

class ReceiverError : public RuntimeError {
  public:
    explicit ReceiverError(const std::string &msg);
};

class GuiError : public RuntimeError {
  public:
    explicit GuiError(const std::string &msg);
};

} //  namespace sls
