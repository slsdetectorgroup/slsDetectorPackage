// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include <stdexcept>

namespace sls {

struct RuntimeError : public std::runtime_error {
  public:
    RuntimeError();
    explicit RuntimeError(const std::string &msg);
    explicit RuntimeError(const char *msg);
};

struct SharedMemoryError : public RuntimeError {
  public:
    explicit SharedMemoryError(const std::string &msg);
};

struct SocketError : public RuntimeError {
  public:
    explicit SocketError(const std::string &msg);
};

struct ZmqSocketError : public RuntimeError {
  public:
    explicit ZmqSocketError(const std::string &msg);
};

struct NotImplementedError : public RuntimeError {
  public:
    explicit NotImplementedError(const std::string &msg);
};

struct DetectorError : public RuntimeError {
  public:
    explicit DetectorError(const std::string &msg);
};

struct ReceiverError : public RuntimeError {
  public:
    explicit ReceiverError(const std::string &msg);
};

struct GuiError : public RuntimeError {
  public:
    explicit GuiError(const std::string &msg);
};

} //  namespace sls
