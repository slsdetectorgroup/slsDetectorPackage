// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/Detector.h"
#include "ui_form_tab_dataoutput.h"

namespace sls {

class qTabDataOutput : public QWidget, private Ui::TabDataOutputObject {
    Q_OBJECT

  public:
    qTabDataOutput(QWidget *parent, Detector *detector);
    ~qTabDataOutput();
    void Refresh();

  private slots:
    void GetOutputDir();
    void BrowseOutputDir();
    void SetOutputDir(bool force = false);
    void ForceSetOutputDir();
    void SetFileFormat(int format);
    void SetOverwriteEnable(bool enable);
    void SetTenGigaEnable(bool enable);
    void EnableRateCorrection();
    void SetRateCorrection();
    void SetSpeed(int speed);
    void SetParallel(bool enable);

  private:
    void SetupWidgetWindow();
    void Initialization();
    void PopulateDetectors();
    void EnableBrowse();
    void GetFileWrite();
    void GetFileName();
    void GetFileFormat();
    void GetFileOverwrite();
    void GetTenGigaEnable();
    void GetRateCorrection();
    void GetSpeed();
    void GetParallel();

    Detector *det;
    // Button group for radiobuttons for rate
    QButtonGroup *btnGroupRate;
};

} // namespace sls
