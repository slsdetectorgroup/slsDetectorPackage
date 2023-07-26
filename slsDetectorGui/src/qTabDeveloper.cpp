// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "qTabDeveloper.h"
#include "qDacWidget.h"
#include "qDefs.h"

namespace sls {

qTabDeveloper::qTabDeveloper(QWidget *parent, Detector *detector)
    : QWidget(parent), det(detector) {
    setupUi(this);
    SetupWidgetWindow();
    LOG(logDEBUG) << "Developer ready";
}

qTabDeveloper::~qTabDeveloper() {}

void qTabDeveloper::SetupWidgetWindow() {
    try {
        // dacs
        auto daclist = det->getDacList();
        for (auto it : daclist) {
            dacWidgets.push_back(
                new qDacWidget(this, det, true, sls::ToString(it), it));
        }

        // adcs
        slsDetectorDefs::detectorType detType = det->getDetectorType().squash();
        slsDetectorDefs::dacIndex ind = slsDetectorDefs::DAC_0;
        switch (detType) {
        case slsDetectorDefs::EIGER:
            for (auto it : eiger_adcs) {
                adcWidgets.push_back(
                    new qDacWidget(this, det, false, sls::ToString(it), it));
            }
            break;
        case slsDetectorDefs::GOTTHARD:
            ind = slsDetectorDefs::TEMPERATURE_ADC;
            adcWidgets.push_back(
                new qDacWidget(this, det, false, sls::ToString(ind), ind));
            ind = slsDetectorDefs::TEMPERATURE_FPGA;
            adcWidgets.push_back(
                new qDacWidget(this, det, false, sls::ToString(ind), ind));
            break;

        case slsDetectorDefs::JUNGFRAU:
            ind = slsDetectorDefs::TEMPERATURE_ADC;
            adcWidgets.push_back(
                new qDacWidget(this, det, false, sls::ToString(ind), ind));
            break;

        case slsDetectorDefs::GOTTHARD2:
        case slsDetectorDefs::MYTHEN3:
        case slsDetectorDefs::MOENCH:
            ind = slsDetectorDefs::TEMPERATURE_FPGA;
            adcWidgets.push_back(
                new qDacWidget(this, det, false, sls::ToString(ind), ind));
            break;

        default:
            break;
        }
    }
    CATCH_DISPLAY("Could not get dac/ adc index ",
                  "qTabDeveloper::SetupWidgetWindow")

    for (size_t i = 0; i < dacWidgets.size(); ++i) {
        gridlayoutDac->addWidget(dacWidgets[i], i / 2, i % 2);
    }
    gridlayoutDac->addItem(
        new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Expanding),
        dacWidgets.size(), 0);
    for (size_t i = 0; i < adcWidgets.size(); ++i) {
        gridlayoutAdc->addWidget(adcWidgets[i], i / 2, i % 2);
    }
    gridlayoutAdc->addItem(
        new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Expanding),
        adcWidgets.size(), 0);

    tabWidget->setCurrentIndex(0);

    PopulateDetectors();
    Initialization();
    Refresh();
}

void qTabDeveloper::Initialization() {
    connect(comboDetector, SIGNAL(currentIndexChanged(int)), this,
            SLOT(setDetectorIndex()));
}

void qTabDeveloper::PopulateDetectors() {
    LOG(logDEBUG) << "Populating detectors";

    try {
        comboDetector->clear();
        comboDetector->addItem("All");
        auto res = det->getHostname();
        if (det->size() > 1) {
            for (auto &it : res) {
                comboDetector->addItem(QString(it.c_str()));
            }
        }
        comboDetector->setCurrentIndex(0);
        setDetectorIndex();
    }
    CATCH_DISPLAY("Could not populate readouts for dacs/adcs",
                  "qTabDeveloper::PopulateDetectors")
}

void qTabDeveloper::setDetectorIndex() {
    LOG(logDEBUG) << "set detector index";
    for (const auto &it : dacWidgets) {
        it->SetDetectorIndex(comboDetector->currentIndex() - 1);
    }
    for (const auto &it : adcWidgets) {
        it->SetDetectorIndex(comboDetector->currentIndex() - 1);
    }
    Refresh();
}

void qTabDeveloper::Refresh() {
    LOG(logDEBUG) << "**Updating Developer Tab\n";
    for (const auto &it : dacWidgets) {
        it->Refresh();
    }
    for (const auto &it : adcWidgets) {
        it->Refresh();
    }
    LOG(logDEBUG) << "**Updated Developer Tab";
}

} // namespace sls
