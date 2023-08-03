// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "qDacWidget.h"
#include "qDefs.h"

namespace sls {

qDacWidget::qDacWidget(QWidget *parent, Detector *detector, bool d,
                       std::string n, slsDetectorDefs::dacIndex i)
    : QWidget(parent), det(detector), isDac(d), index(i) {
    setupUi(this);
    SetupWidgetWindow(n);
}

qDacWidget::~qDacWidget() {}

void qDacWidget::SetupWidgetWindow(std::string name) {
    lblDac->setText(name.c_str());
    if (isDac) {
        spinDac->setDecimals(0);
    } else {
        spinDac->setSuffix(0x00b0 + QString("C"));
        spinDac->setReadOnly(true);
        lblDacmV->setMinimumWidth(0);
        lblDacmV->setMaximumWidth(0);
    }
    Initialization();
    Refresh();
}

void qDacWidget::Initialization() {
    if (isDac) {
        connect(spinDac, SIGNAL(valueChanged(double)), this, SLOT(SetDac()));
    }
}

void qDacWidget::SetDetectorIndex(int id) { detectorIndex = id; }

void qDacWidget::GetDac() {
    LOG(logDEBUG) << "Getting Dac " << index;

    disconnect(spinDac, SIGNAL(valueChanged(double)), this, SLOT(SetDac()));
    try {
        // dac units
        auto retval = det->getDAC(index, 0, {detectorIndex}).squash(-1);
        spinDac->setValue(retval);
        // mv
        retval = det->getDAC(index, 1, {detectorIndex}).squash(-1);
        // -6 is the minimum amt of space it occupies, if more needed, its
        // padded with ' ', negative value for left aligned text
        lblDacmV->setText(QString("%1mV").arg(retval, -6));
    }
    CATCH_DISPLAY(std::string("Could not get dac ") + std::to_string(index),
                  "qDacWidget::GetDac")

    connect(spinDac, SIGNAL(valueChanged(double)), this, SLOT(SetDac()));
}

void qDacWidget::SetDac() {
    int val = (int)spinDac->value();
    LOG(logINFO) << "Setting dac:" << lblDac->text().toLatin1().data() << " : "
                 << val;

    try {
        det->setDAC(index, val, 0, {detectorIndex});
    }
    CATCH_DISPLAY(std::string("Could not set dac ") + std::to_string(index),
                  "qDacWidget::SetDac")

    // update mV anyway
    GetDac();
}

void qDacWidget::GetAdc() {
    LOG(logDEBUG) << "Getting ADC " << index;

    try {
        auto retval = det->getTemperature(index, {detectorIndex}).squash(-1);
        if (retval == -1 && detectorIndex == -1) {
            spinDac->setValue(-1);
        } else {
            spinDac->setValue(retval);
        }
    }
    CATCH_DISPLAY(std::string("Could not get adc ") + std::to_string(index),
                  "qDacWidget::GetAdc")
}

void qDacWidget::Refresh() {
    if (isDac) {
        GetDac();
    } else {
        GetAdc();
    }
}

} // namespace sls
