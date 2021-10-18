// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include <stdexcept>

namespace sls {

struct RuntimeError : public std::runtime_error {
  public:
    RuntimeError();
    RuntimeError(const std::string &msg);
    RuntimeError(const char *msg);
};

struct SharedMemoryError : public RuntimeError {
  public:
    SharedMemoryError(const std::string &msg);
};

struct SocketError : public RuntimeError {
  public:
    SocketError(const std::string &msg);
};

struct ZmqSocketError : public RuntimeError {
  public:
    ZmqSocketError(const std::string &msg);
};

struct NotImplementedError : public RuntimeError {
  public:
    NotImplementedError(const std::string &msg);
};

struct DetectorError : public RuntimeError {
  public:
    DetectorError(const std::string &msg);
};

struct ReceiverError : public RuntimeError {
  public:
    ReceiverError(const std::string &msg);
};

struct GuiError : public RuntimeError {
  public:
    GuiError(const std::string &msg);
};

} //  namespace sls
