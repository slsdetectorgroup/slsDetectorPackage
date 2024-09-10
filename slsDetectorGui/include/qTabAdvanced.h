// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/Detector.h"
#include "ui_form_tab_advanced.h"

namespace sls {

class qDrawPlot;

class qTabAdvanced : public QWidget, private Ui::TabAdvancedObject {
    Q_OBJECT

  public:
    qTabAdvanced(QWidget *parent, Detector *detector, qDrawPlot *p);
    ~qTabAdvanced();

  public slots:
    void Refresh();

  private slots:
    void SetDetector();
    void SetControlPort(int port);
    void SetStopPort(int port);
    void SetDetectorUDPIP(bool force = false);
    void ForceSetDetectorUDPIP();
    void SetDetectorUDPMAC(bool force = false);
    void ForceSetDetectorUDPMAC();
    void SetCltZMQPort(int port);
    void SetCltZMQIP(bool force = false);
    void ForceSetCltZMQIP();
    void SetRxrHostname(bool force = false);
    void ForceSetRxrHostname();
    void SetRxrTCPPort(int port);
    void SetRxrUDPPort(int port);
    void SetRxrUDPIP(bool force = false);
    void ForceSetRxrUDPIP();
    void SetRxrUDPMAC(bool force = false);
    void ForceSetRxrUDPMAC();
    void SetRxrZMQPort(int port);
    void GetROI();
    void ClearROI();
    void SetROI();
    void SetAllTrimbits();
    void SetNumStoragecells(int value);
    void SetSubExposureTime();
    void SetSubDeadTime();
    void SetGateIndex(int value);
    void SetExposureTime();
    void SetGateDelay();

  private:
    void SetupWidgetWindow();
    void Initialization();
    void PopulateDetectors();

    void GetControlPort();
    void GetStopPort();
    void GetDetectorUDPIP();
    void GetDetectorUDPMAC();
    void GetCltZMQPort();
    void GetCltZMQIP();
    void GetRxrHostname();
    void GetRxrTCPPort();
    void GetRxrUDPPort();
    void GetRxrUDPIP();
    void GetRxrUDPMAC();
    void GetRxrZMQPort();
    void GetAllTrimbits();
    void GetNumStoragecells();
    void GetSubExposureTime();
    void GetSubDeadTime();
    void GetExposureTime();
    void GetGateDelay();

    Detector *det;
    qDrawPlot *plot;
};

} // namespace sls
