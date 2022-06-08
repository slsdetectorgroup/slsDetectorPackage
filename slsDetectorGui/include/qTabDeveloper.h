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

  private:
    void SetupWidgetWindow();
    void Initialization();
    void PopulateDetectors();
    slsDetectorDefs::dacIndex getSLSIndex(slsDetectorDefs::detectorType detType,
                                          int index);

    Detector *det;
    std::vector<qDacWidget *> dacWidgets;
    std::vector<qDacWidget *> adcWidgets;
};

} // namespace sls
