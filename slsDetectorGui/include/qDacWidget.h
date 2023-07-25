// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include "ui_form_dac.h"
#include <string>

namespace sls {

class qDacWidget : public QWidget, private Ui::WidgetDacObject {
    Q_OBJECT

  public:
    qDacWidget(QWidget *parent, Detector *detector, bool d, std::string n,
               slsDetectorDefs::dacIndex i);
    ~qDacWidget();
    void SetDetectorIndex(int id);
    void Refresh();

  private slots:
    void SetDac();

  private:
    void SetupWidgetWindow(std::string name);
    void Initialization();
    void GetDac();
    void GetAdc();

    Detector *det;
    bool isDac{true};
    slsDetectorDefs::dacIndex index;
    int detectorIndex{-1};
};

} // namespace sls
