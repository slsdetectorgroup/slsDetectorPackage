// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/Detector.h"
#include "ui_form_tab_debugging.h"

namespace sls {

class qTabDebugging : public QWidget, private Ui::TabDebuggingObject {
    Q_OBJECT

  public:
    qTabDebugging(QWidget *parent, Detector *detector);
    ~qTabDebugging();
    void Refresh();

  private slots:
    void GetInfo();
    void EnableTest();
    void TestDetector();

  private:
    void SetupWidgetWindow();
    void Initialization();
    void PopulateDetectors();
    void GetFirmwareVersion();
    void GetServerSoftwareVersion();
    void GetReceiverVersion();
    void GetDetectorStatus();

    Detector *det;
};

} // namespace sls
