// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include "ui_form_tab_developer.h"
#include <vector>

namespace sls {

class qDacWidget;

class qTabDeveloper : public QWidget, private Ui::TabDeveloperObject {
    Q_OBJECT

  public:
    qTabDeveloper(QWidget *parent, Detector *detector);
    ~qTabDeveloper();

  public slots:
    void Refresh();

  private slots:
    void SetHighVoltage();

  private:
    void SetupWidgetWindow();
    void Initialization();
    void PopulateDetectors();
    void GetHighVoltage();
    slsDetectorDefs::dacIndex getSLSIndex(slsDetectorDefs::detectorType detType,
                                          int index);

    Detector *det;
    std::vector<qDacWidget *> dacWidgets;
    std::vector<qDacWidget *> adcWidgets;

    enum hvVals { HV_0, HV_90, HV_110, HV_120, HV_150, HV_180, HV_200 };
    int hvmin;
    static const int HV_MIN = 60;
    static const int HV_MAX = 200;
};

} // namespace sls
