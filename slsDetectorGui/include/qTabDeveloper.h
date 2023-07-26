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
    void setDetectorIndex();

  private:
    void SetupWidgetWindow();
    void Initialization();
    void PopulateDetectors();

    Detector *det;
    std::vector<qDacWidget *> dacWidgets;
    std::vector<qDacWidget *> adcWidgets;

    const std::vector<slsDetectorDefs::dacIndex> eiger_adcs = {
        slsDetectorDefs::TEMPERATURE_FPGA,
        slsDetectorDefs::TEMPERATURE_FPGAEXT,
        slsDetectorDefs::TEMPERATURE_10GE,
        slsDetectorDefs::TEMPERATURE_DCDC,
        slsDetectorDefs::TEMPERATURE_SODL,
        slsDetectorDefs::TEMPERATURE_SODR,
        slsDetectorDefs::TEMPERATURE_FPGA2,
        slsDetectorDefs::TEMPERATURE_FPGA3};
};

} // namespace sls
