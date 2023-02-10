// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "qTabPlot.h"
#include "qDefs.h"
#include "qDrawPlot.h"
#include <QStackedLayout>
#include <QStandardItemModel>

namespace sls {

QString qTabPlot::defaultPlotTitle("");
QString qTabPlot::defaultHistXAxisTitle("Channel Number");
QString qTabPlot::defaultHistYAxisTitle("Counts");
QString qTabPlot::defaultImageXAxisTitle("Pixel");
QString qTabPlot::defaultImageYAxisTitle("Pixel");
QString qTabPlot::defaultImageZAxisTitle("Intensity");

qTabPlot::qTabPlot(QWidget *parent, Detector *detector, qDrawPlot *p)
    : QWidget(parent), det(detector), plot(p) {
    setupUi(this);
    SetupWidgetWindow();
    LOG(logDEBUG) << "Plot ready";
}

qTabPlot::~qTabPlot() {}

void qTabPlot::SetupWidgetWindow() {
    // 1D and 2D options
    stackedWidget1D->setCurrentIndex(0);
    stackedWidget2D->setCurrentIndex(0);
    // Plot Axis
    // its not spinboxes to not take value when checkbox checked
    dispXMin->setValidator(new QDoubleValidator(dispXMin));
    dispYMin->setValidator(new QDoubleValidator(dispYMin));
    dispZMin->setValidator(new QDoubleValidator(dispZMin));
    dispXMax->setValidator(new QDoubleValidator(dispXMax));
    dispYMax->setValidator(new QDoubleValidator(dispYMax));
    dispZMax->setValidator(new QDoubleValidator(dispZMax));
    // Plot titles
    dispTitle->setText("");
    dispXAxis->setText(defaultHistXAxisTitle);
    dispYAxis->setText(defaultHistYAxisTitle);
    dispXAxis->setText(defaultImageXAxisTitle);
    dispYAxis->setText(defaultImageYAxisTitle);
    dispZAxis->setText(defaultImageZAxisTitle);

    // enabling according to det type
    is1d = false;
    switch (det->getDetectorType().squash()) {
    case slsDetectorDefs::GOTTHARD:
    case slsDetectorDefs::MYTHEN3:
        is1d = true;
        break;
    case slsDetectorDefs::GOTTHARD2:
        is1d = true;
        chkGainPlot1D->setEnabled(true);
        chkGainPlot1D->setChecked(true);
        plot->EnableGainPlot(true);
        break;
    case slsDetectorDefs::JUNGFRAU:
        chkGainPlot->setEnabled(true);
        chkGainPlot->setChecked(true);
        plot->EnableGainPlot(true);
        break;
    default:
        break;
    }
    isGapPixelsAllowed = VerifyGapPixelsAllowed();
    chkGapPixels->setEnabled(isGapPixelsAllowed);

    Select1DPlot(is1d);
    Initialization();
    Refresh();

    // update both zmq high water mark to GUI_ZMQ_RCV_HWM (2)
    comboHwm->setCurrentIndex(SND_HWM);
    spinHwm->setValue(qDefs::GUI_ZMQ_RCV_HWM);
    comboHwm->setCurrentIndex(RX_HWM);
    spinHwm->setValue(qDefs::GUI_ZMQ_RCV_HWM);

    if (chkGapPixels->isEnabled()) {
        chkGapPixels->setChecked(true);
    }
}

void qTabPlot::Initialization() {
    // Plotting frequency box
    connect(chkNoPlot, SIGNAL(toggled(bool)), this, SLOT(SetPlot()));
    connect(comboHwm, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SelectHwm(int)));
    connect(spinHwm, SIGNAL(valueChanged(int)), this, SLOT(SetHwm(int)));
    connect(comboFrequency, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetStreamingFrequency()));
    connect(comboTimeGapUnit, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetStreamingFrequency()));
    connect(spinTimeGap, SIGNAL(valueChanged(double)), this,
            SLOT(SetStreamingFrequency()));
    connect(spinNthFrame, SIGNAL(valueChanged(int)), this,
            SLOT(SetStreamingFrequency()));

    // navigation buttons for options
    connect(btnRight1D, SIGNAL(clicked()), this, SLOT(Set1DPlotOptionsRight()));
    connect(btnLeft1D, SIGNAL(clicked()), this, SLOT(Set1DPlotOptionsLeft()));
    connect(btnRight2D, SIGNAL(clicked()), this, SLOT(Set2DPlotOptionsRight()));
    connect(btnLeft2D, SIGNAL(clicked()), this, SLOT(Set2DPlotOptionsLeft()));

    // 1D options
    connect(chkSuperimpose, SIGNAL(toggled(bool)), this,
            SLOT(EnablePersistency(bool)));
    connect(spinPersistency, SIGNAL(valueChanged(int)), plot,
            SLOT(SetPersistency(int)));
    connect(chkPoints, SIGNAL(toggled(bool)), plot, SLOT(SetMarkers(bool)));
    connect(chkLines, SIGNAL(toggled(bool)), plot, SLOT(SetLines(bool)));
    connect(chk1DLog, SIGNAL(toggled(bool)), plot, SLOT(Set1dLogY(bool)));
    connect(chkStatistics, SIGNAL(toggled(bool)), plot,
            SLOT(DisplayStatistics(bool)));

    // 2D Plot box
    connect(chkInterpolate, SIGNAL(toggled(bool)), plot,
            SLOT(SetInterpolate(bool)));
    connect(chkContour, SIGNAL(toggled(bool)), plot, SLOT(SetContour(bool)));
    connect(chkLogz, SIGNAL(toggled(bool)), plot, SLOT(SetLogz(bool)));
    connect(chkStatistics_2, SIGNAL(toggled(bool)), plot,
            SLOT(DisplayStatistics(bool)));
    // pedstal
    connect(chkPedestal, SIGNAL(toggled(bool)), plot, SLOT(SetPedestal(bool)));
    connect(btnRecalPedestal, SIGNAL(clicked()), plot,
            SLOT(RecalculatePedestal()));
    connect(chkPedestal_2, SIGNAL(toggled(bool)), plot,
            SLOT(SetPedestal(bool)));
    connect(btnRecalPedestal_2, SIGNAL(clicked()), plot,
            SLOT(RecalculatePedestal()));
    // accumulate
    connect(chkAccumulate, SIGNAL(toggled(bool)), plot,
            SLOT(SetAccumulate(bool)));
    connect(btnResetAccumulate, SIGNAL(clicked()), plot,
            SLOT(ResetAccumulate()));
    connect(chkAccumulate_2, SIGNAL(toggled(bool)), plot,
            SLOT(SetAccumulate(bool)));
    connect(btnResetAccumulate_2, SIGNAL(clicked()), plot,
            SLOT(ResetAccumulate()));
    // binary
    connect(chkBinary, SIGNAL(toggled(bool)), this, SLOT(SetBinary()));
    connect(chkBinary_2, SIGNAL(toggled(bool)), this, SLOT(SetBinary()));
    connect(spinFrom, SIGNAL(valueChanged(int)), this, SLOT(SetBinary()));
    connect(spinFrom_2, SIGNAL(valueChanged(int)), this, SLOT(SetBinary()));
    connect(spinTo, SIGNAL(valueChanged(int)), this, SLOT(SetBinary()));
    connect(spinTo_2, SIGNAL(valueChanged(int)), this, SLOT(SetBinary()));
    // gainplot
    if (chkGainPlot->isEnabled())
        connect(chkGainPlot, SIGNAL(toggled(bool)), plot,
                SLOT(EnableGainPlot(bool)));
    if (chkGainPlot1D->isEnabled())
        connect(chkGainPlot1D, SIGNAL(toggled(bool)), plot,
                SLOT(EnableGainPlot(bool)));
    // gap pixels
    if (chkGapPixels->isEnabled())
        connect(chkGapPixels, SIGNAL(toggled(bool)), this,
                SLOT(SetGapPixels(bool)));

    // Save, clone
    connect(btnSave, SIGNAL(clicked()), plot, SLOT(SavePlot()));
    connect(btnClone, SIGNAL(clicked()), plot, SLOT(ClonePlot()));

    // Plot Axis
    connect(chkTitle, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(chkXAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(chkYAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(chkZAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(dispTitle, SIGNAL(textChanged(const QString &)), this,
            SLOT(SetTitles()));
    connect(dispXAxis, SIGNAL(textChanged(const QString &)), this,
            SLOT(SetTitles()));
    connect(dispYAxis, SIGNAL(textChanged(const QString &)), this,
            SLOT(SetTitles()));
    connect(dispZAxis, SIGNAL(textChanged(const QString &)), this,
            SLOT(SetTitles()));

    connect(chkXMin, SIGNAL(toggled(bool)), this, SLOT(SetXRange()));
    connect(chkXMax, SIGNAL(toggled(bool)), this, SLOT(SetXRange()));
    connect(chkYMin, SIGNAL(toggled(bool)), this, SLOT(SetYRange()));
    connect(chkYMax, SIGNAL(toggled(bool)), this, SLOT(SetYRange()));
    connect(dispXMin, SIGNAL(editingFinished()), this, SLOT(isXMinModified()));
    connect(dispXMax, SIGNAL(editingFinished()), this, SLOT(isXMaxModified()));
    connect(dispYMin, SIGNAL(editingFinished()), this, SLOT(isYMinModified()));
    connect(dispYMax, SIGNAL(editingFinished()), this, SLOT(isYMaxModified()));
    connect(chkAspectRatio, SIGNAL(toggled(bool)), this,
            SLOT(CheckAspectRatio()));

    connect(chkZMin, SIGNAL(toggled(bool)), this, SLOT(SetZRange()));
    connect(chkZMax, SIGNAL(toggled(bool)), this, SLOT(SetZRange()));
    connect(dispZMin, SIGNAL(editingFinished()), this, SLOT(isZMinModified()));
    connect(dispZMax, SIGNAL(editingFinished()), this, SLOT(isZMaxModified()));
}

bool qTabPlot::VerifyGapPixelsAllowed() {
    try {
        switch (det->getDetectorType().squash()) {
        case slsDetectorDefs::JUNGFRAU:
            return true;
        case slsDetectorDefs::EIGER:
            if (det->getQuad().squash(false)) {
                return true;
            }
            // full modules
            if (det->getModuleGeometry().y % 2 == 0) {
                return true;
            }
            return false;
        default:
            return false;
        }
    }
    CATCH_DISPLAY("Could not verify if gap pixels allowed.",
                  "qTabPlot::VerifyGapPixelsAllowed")
    return false;
}

void qTabPlot::Select1DPlot(bool enable) {
    LOG(logDEBUG) << "Selecting " << (enable ? "1" : "2") << "D Plot";
    is1d = enable;
    stackedPlotOptions->setCurrentIndex(is1d ? 0 : 1);
    chkZAxis->setEnabled(!is1d);
    dispZAxis->setEnabled(!is1d);
    chkZMin->setEnabled(!is1d);
    chkZMax->setEnabled(!is1d);
    dispZMin->setEnabled(!is1d);
    dispZMax->setEnabled(!is1d);
    plot->Select1dPlot(is1d);
    SetTitles();
    SetXYRange();
    if (!is1d) {
        SetZRange();
    }
}

void qTabPlot::SetPlot() {
    bool plotEnable = false;
    if (chkNoPlot->isChecked()) {
        LOG(logINFO) << "Setting Plot Type: No Plot";
    } else {
        LOG(logINFO) << "Setting Plot Type: Datagraph";
        plotEnable = true;
    }
    comboFrequency->setEnabled(plotEnable);
    comboHwm->setEnabled(plotEnable);
    spinHwm->setEnabled(plotEnable);
    stackedTimeInterval->setEnabled(plotEnable);
    stackedPlotOptions->setEnabled(plotEnable);
    btnSave->setEnabled(plotEnable);
    btnClone->setEnabled(plotEnable);
    boxPlotAxis->setEnabled(plotEnable);

    if (plotEnable) {
        SetTitles();
        SetXYRange();
        if (!is1d) {
            SetZRange();
        }
    }

    plot->SetDataCallBack(plotEnable);
}

void qTabPlot::Set1DPlotOptionsRight() {
    LOG(logDEBUG) << "1D Options Right";
    int i = stackedWidget1D->currentIndex();
    if (i == (stackedWidget1D->count() - 1))
        stackedWidget1D->setCurrentIndex(0);
    else
        stackedWidget1D->setCurrentIndex(i + 1);
}

void qTabPlot::Set1DPlotOptionsLeft() {
    LOG(logDEBUG) << "1D Options Left";
    int i = stackedWidget1D->currentIndex();
    if (i == 0)
        stackedWidget1D->setCurrentIndex(stackedWidget1D->count() - 1);
    else
        stackedWidget1D->setCurrentIndex(i - 1);
}

void qTabPlot::Set2DPlotOptionsRight() {
    LOG(logDEBUG) << "2D Options Right";
    int i = stackedWidget2D->currentIndex();
    if (i == (stackedWidget2D->count() - 1))
        stackedWidget2D->setCurrentIndex(0);
    else
        stackedWidget2D->setCurrentIndex(i + 1);
}

void qTabPlot::Set2DPlotOptionsLeft() {
    LOG(logDEBUG) << "2D Options Left";
    int i = stackedWidget2D->currentIndex();
    if (i == 0)
        stackedWidget2D->setCurrentIndex(stackedWidget2D->count() - 1);
    else
        stackedWidget2D->setCurrentIndex(i - 1);
}

void qTabPlot::EnablePersistency(bool enable) {
    LOG(logINFO) << "Superimpose " << (enable ? "enabled" : "disabled");
    lblPersistency->setEnabled(enable);
    spinPersistency->setEnabled(enable);
    if (enable)
        plot->SetPersistency(spinPersistency->value());
    else
        plot->SetPersistency(0);
}

void qTabPlot::SetBinary() {
    bool binary1D = chkBinary->isChecked();
    bool binary2D = chkBinary_2->isChecked();
    if (is1d) {
        LOG(logINFO) << "Binary Plot " << (binary1D ? "enabled" : "disabled");
        lblFrom->setEnabled(binary1D);
        lblTo->setEnabled(binary1D);
        spinFrom->setEnabled(binary1D);
        spinTo->setEnabled(binary1D);
        plot->SetBinary(binary1D, spinFrom->value(), spinTo->value());
    } else {
        LOG(logINFO) << "Binary Plot " << (binary2D ? "enabled" : "disabled");
        lblFrom_2->setEnabled(binary2D);
        lblTo_2->setEnabled(binary2D);
        spinFrom_2->setEnabled(binary2D);
        spinTo_2->setEnabled(binary2D);
        plot->SetBinary(binary2D, spinFrom_2->value(), spinTo_2->value());
    }
}

void qTabPlot::GetGapPixels() {
    LOG(logDEBUG) << "Getting gap pixels";
    disconnect(chkGapPixels, SIGNAL(toggled(bool)), this,
               SLOT(SetGapPixels(bool)));
    try {
        auto retval = det->getGapPixelsinCallback();
        chkGapPixels->setChecked(retval);
    }
    CATCH_DISPLAY("Could not get gap pixels enable.", "qTabPlot::GetGapPixels")
    connect(chkGapPixels, SIGNAL(toggled(bool)), this,
            SLOT(SetGapPixels(bool)));
}

void qTabPlot::SetGapPixels(bool enable) {
    LOG(logINFO) << "Setting Gap Pixels Enable to " << enable;
    try {
        det->setGapPixelsinCallback(enable);
        plot->SetGapPixels(enable);
    }
    CATCH_HANDLE("Could not set gap pixels enable.", "qTabPlot::SetGapPixels",
                 this, &qTabPlot::GetGapPixels)
}

void qTabPlot::SetTitles() {
    LOG(logDEBUG) << "Setting Plot Titles";
    disconnect(chkTitle, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    disconnect(chkXAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    disconnect(chkYAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    disconnect(chkZAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    disconnect(dispTitle, SIGNAL(textChanged(const QString &)), this,
               SLOT(SetTitles()));
    disconnect(dispXAxis, SIGNAL(textChanged(const QString &)), this,
               SLOT(SetTitles()));
    disconnect(dispYAxis, SIGNAL(textChanged(const QString &)), this,
               SLOT(SetTitles()));
    disconnect(dispZAxis, SIGNAL(textChanged(const QString &)), this,
               SLOT(SetTitles()));

    // title
    if (!chkTitle->isChecked() || dispTitle->text().isEmpty()) {
        plot->SetPlotTitlePrefix("");
        dispTitle->setText("");
    } else {
        plot->SetPlotTitlePrefix(dispTitle->text());
    }
    // x
    if (!chkXAxis->isChecked() || dispXAxis->text().isEmpty()) {
        dispXAxis->setText(is1d ? defaultHistXAxisTitle
                                : defaultImageXAxisTitle);
        plot->SetXAxisTitle(is1d ? defaultHistXAxisTitle
                                 : defaultImageXAxisTitle);
    } else {
        plot->SetXAxisTitle(dispXAxis->text());
    }
    // y
    if (!chkYAxis->isChecked() || dispYAxis->text().isEmpty()) {
        dispYAxis->setText(is1d ? defaultHistYAxisTitle
                                : defaultImageYAxisTitle);
        plot->SetYAxisTitle(is1d ? defaultHistYAxisTitle
                                 : defaultImageYAxisTitle);
    } else {
        plot->SetYAxisTitle(dispYAxis->text());
    }
    // z
    if (!chkZAxis->isChecked() || dispZAxis->text().isEmpty()) {
        plot->SetZAxisTitle(defaultImageZAxisTitle);
        dispZAxis->setText(defaultImageZAxisTitle);
    } else {
        plot->SetZAxisTitle(dispZAxis->text());
    }

    connect(chkTitle, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(chkXAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(chkYAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(chkZAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(dispTitle, SIGNAL(textChanged(const QString &)), this,
            SLOT(SetTitles()));
    connect(dispXAxis, SIGNAL(textChanged(const QString &)), this,
            SLOT(SetTitles()));
    connect(dispYAxis, SIGNAL(textChanged(const QString &)), this,
            SLOT(SetTitles()));
    connect(dispZAxis, SIGNAL(textChanged(const QString &)), this,
            SLOT(SetTitles()));
}

void qTabPlot::isXMinModified() {
    if (dispXMin->isModified()) {
        dispXMin->setModified(false);
        SetXRange();
    }
}

void qTabPlot::isXMaxModified() {
    if (dispXMax->isModified()) {
        dispXMax->setModified(false);
        SetXRange();
    }
}

void qTabPlot::isYMinModified() {
    if (dispYMin->isModified()) {
        dispYMin->setModified(false);
        SetYRange();
    }
}

void qTabPlot::isYMaxModified() {
    if (dispYMax->isModified()) {
        dispYMax->setModified(false);
        SetYRange();
    }
}

void qTabPlot::isZMinModified() {
    if (dispZMin->isModified()) {
        dispZMin->setModified(false);
        SetZRange();
    }
}

void qTabPlot::isZMaxModified() {
    if (dispZMax->isModified()) {
        dispZMax->setModified(false);
        SetZRange();
    }
}

void qTabPlot::SetXRange() {
    LOG(logDEBUG) << "Enable X axis range";

    if (chkAspectRatio->isChecked()) {
        MaintainAspectRatio(static_cast<int>(slsDetectorDefs::Y));
    } else {
        SetXYRange();
    }
}

void qTabPlot::SetYRange() {
    LOG(logDEBUG) << "Enable Y axis range";

    if (chkAspectRatio->isChecked()) {
        MaintainAspectRatio(static_cast<int>(slsDetectorDefs::X));
    } else {
        SetXYRange();
    }
}

void qTabPlot::CheckAspectRatio() {
    if (chkAspectRatio->isChecked()) {
        MaintainAspectRatio(-1);
    } else {
        SetXYRange();
    }
}

void qTabPlot::SetXYRange() {
    LOG(logDEBUG) << "Set XY Range";
    bool disablezoom = false;
    bool isRange[4]{false, false, false, false};
    double xyRange[4]{0, 0, 0, 0};

    QString dispVal[4]{dispXMin->text(), dispXMax->text(), dispYMin->text(),
                       dispYMax->text()};
    bool chkVal[4]{chkXMin->isChecked(), chkXMax->isChecked(),
                   chkYMin->isChecked(), chkYMax->isChecked()};

    for (int i = 0; i < 4; ++i) {
        if (chkVal[i] && !dispVal[i].isEmpty()) {
            double val = dispVal[i].toDouble();
            LOG(logDEBUG) << "Setting "
                          << qDefs::getRangeAsString(
                                 static_cast<qDefs::range>(i))
                          << " to " << val;
            xyRange[i] = val;
            isRange[i] = true;
            disablezoom = true;
        }
    }

    plot->SetXYRangeChanged(disablezoom, xyRange, isRange);
    plot->UpdatePlot();
    emit DisableZoomSignal(disablezoom);
}

void qTabPlot::MaintainAspectRatio(int dimension) {
    LOG(logDEBUG) << "Maintaining Aspect Ratio";

    disconnect(chkXMin, SIGNAL(toggled(bool)), this, SLOT(SetXRange()));
    disconnect(chkXMax, SIGNAL(toggled(bool)), this, SLOT(SetXRange()));
    disconnect(chkYMin, SIGNAL(toggled(bool)), this, SLOT(SetYRange()));
    disconnect(chkYMax, SIGNAL(toggled(bool)), this, SLOT(SetYRange()));
    disconnect(dispXMin, SIGNAL(editingFinished()), this,
               SLOT(isXMinModified()));
    disconnect(dispXMax, SIGNAL(editingFinished()), this,
               SLOT(isXMaxModified()));
    disconnect(dispYMin, SIGNAL(editingFinished()), this,
               SLOT(isYMinModified()));
    disconnect(dispYMax, SIGNAL(editingFinished()), this,
               SLOT(isYMaxModified()));

    // check all, fill all
    chkXMin->setChecked(true);
    chkXMax->setChecked(true);
    chkYMin->setChecked(true);
    chkYMax->setChecked(true);
    if (dispXMin->text().isEmpty())
        dispXMin->setText(QString::number(plot->GetXMinimum()));
    if (dispXMax->text().isEmpty())
        dispXMax->setText(QString::number(plot->GetXMaximum()));
    if (dispYMin->text().isEmpty())
        dispYMin->setText(QString::number(plot->GetYMinimum()));
    if (dispYMax->text().isEmpty())
        dispYMax->setText(QString::number(plot->GetYMaximum()));

    // calculate ideal aspect ratio with previous limits
    double ranges[4];
    ranges[qDefs::XMIN] = plot->GetXMinimum();
    ranges[qDefs::XMAX] = plot->GetXMaximum();
    ranges[qDefs::YMIN] = plot->GetYMinimum();
    ranges[qDefs::YMAX] = plot->GetYMaximum();
    double idealAspectratio = (ranges[qDefs::XMAX] - ranges[qDefs::XMIN]) /
                              (ranges[qDefs::YMAX] - ranges[qDefs::YMIN]);
    LOG(logDEBUG) << "Ideal Aspect ratio: " << idealAspectratio << " for x("
                  << ranges[qDefs::XMIN] << " - " << ranges[qDefs::XMAX]
                  << "), y(" << ranges[qDefs::YMIN] << " - "
                  << ranges[qDefs::YMAX] << ")";

    // calculate current aspect ratio
    ranges[qDefs::XMIN] = dispXMin->text().toDouble();
    ranges[qDefs::XMAX] = dispXMax->text().toDouble();
    ranges[qDefs::YMIN] = dispYMin->text().toDouble();
    ranges[qDefs::YMAX] = dispYMax->text().toDouble();
    double currentAspectRatio = (ranges[qDefs::XMAX] - ranges[qDefs::XMIN]) /
                                (ranges[qDefs::YMAX] - ranges[qDefs::YMIN]);
    LOG(logDEBUG) << "Current Aspect ratio: " << currentAspectRatio << " for x("
                  << ranges[qDefs::XMIN] << " - " << ranges[qDefs::XMAX]
                  << "), y(" << ranges[qDefs::YMIN] << " - "
                  << ranges[qDefs::YMAX] << ")";

    if (currentAspectRatio != idealAspectratio) {
        // dimension: 1(x changed: y adjusted), 0(y changed: x adjusted),
        // -1(aspect ratio clicked: larger one adjusted)
        if (dimension == -1) {
            dimension = ((ranges[qDefs::XMAX] - ranges[qDefs::XMIN]) >
                         (ranges[qDefs::YMAX] - ranges[qDefs::YMIN]))
                            ? static_cast<int>(slsDetectorDefs::X)
                            : static_cast<int>(slsDetectorDefs::Y);
        }

        // calculate new value to maintain aspect ratio
        // adjust x
        double newval = 0;
        if (dimension == static_cast<int>(slsDetectorDefs::X)) {
            newval =
                idealAspectratio * (ranges[qDefs::YMAX] - ranges[qDefs::YMIN]) +
                ranges[qDefs::XMIN];
            if (newval <= plot->GetXMaximum()) {
                ranges[qDefs::XMAX] = newval;
                dispXMax->setText(QString::number(newval));
                LOG(logDEBUG) << "New XMax: " << newval;
            } else {
                newval = ranges[qDefs::XMAX] -
                         (idealAspectratio *
                          (ranges[qDefs::YMAX] - ranges[qDefs::YMIN]));
                ranges[qDefs::XMIN] = newval;
                dispXMin->setText(QString::number(newval));
                LOG(logDEBUG) << "New XMin: " << newval;
            }
        }
        // adjust y
        else {
            newval = ((ranges[qDefs::XMAX] - ranges[qDefs::XMIN]) /
                      idealAspectratio) +
                     ranges[qDefs::YMIN];
            if (newval <= plot->GetYMaximum()) {
                ranges[qDefs::YMAX] = newval;
                dispYMax->setText(QString::number(newval));
                LOG(logDEBUG) << "New YMax: " << newval;
            } else {
                newval = ranges[qDefs::YMAX] -
                         ((ranges[qDefs::XMAX] - ranges[qDefs::XMIN]) /
                          idealAspectratio);
                ranges[qDefs::YMIN] = newval;
                dispYMin->setText(QString::number(newval));
                LOG(logDEBUG) << "New YMax: " << newval;
            }
        }
    }

    connect(chkXMin, SIGNAL(toggled(bool)), this, SLOT(SetXRange()));
    connect(chkXMax, SIGNAL(toggled(bool)), this, SLOT(SetXRange()));
    connect(chkYMin, SIGNAL(toggled(bool)), this, SLOT(SetYRange()));
    connect(chkYMax, SIGNAL(toggled(bool)), this, SLOT(SetYRange()));
    connect(dispXMin, SIGNAL(editingFinished()), this, SLOT(isXMinModified()));
    connect(dispXMax, SIGNAL(editingFinished()), this, SLOT(isXMaxModified()));
    connect(dispYMin, SIGNAL(editingFinished()), this, SLOT(isYMinModified()));
    connect(dispYMax, SIGNAL(editingFinished()), this, SLOT(isYMaxModified()));

    bool isRange[4]{true, true, true, true};
    plot->SetXYRangeChanged(true, ranges, isRange);
    emit DisableZoomSignal(true);
}

void qTabPlot::SetZRange() {
    bool isZRange[2]{chkZMin->isChecked(), chkZMax->isChecked()};
    double zRange[2]{0, 0};

    if (isZRange[0] && !dispZMin->text().isEmpty()) {
        double val = dispZMin->text().toDouble();
        LOG(logDEBUG) << "Setting zmin to " << val;
        zRange[0] = val;
    }
    if (isZRange[1] && !dispZMax->text().isEmpty()) {
        double val = dispZMax->text().toDouble();
        LOG(logDEBUG) << "Setting zmax to " << val;
        zRange[1] = val;
    }
    plot->SetZRange(zRange, isZRange);
    plot->UpdatePlot();
}

void qTabPlot::GetStreamingFrequency() {
    LOG(logDEBUG) << "Getting Streaming Frequency";
    disconnect(comboFrequency, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetStreamingFrequency()));
    disconnect(comboTimeGapUnit, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetStreamingFrequency()));
    disconnect(spinTimeGap, SIGNAL(valueChanged(double)), this,
               SLOT(SetStreamingFrequency()));
    disconnect(spinNthFrame, SIGNAL(valueChanged(int)), this,
               SLOT(SetStreamingFrequency()));
    try {
        int freq = det->getRxZmqFrequency().tsquash(
            "Inconsistent receiver zmq streaming frequency for all detectors.");
        // time interval
        if (freq == 0) {
            comboFrequency->setCurrentIndex(0);
            stackedTimeInterval->setCurrentIndex(0);
            try {
                int timeMs = det->getRxZmqTimer().tsquash(
                    "Inconsistent receiver zmq streaming timer for all "
                    "detectors.");
                auto timeNS = qDefs::getNSTime(std::make_pair(
                    static_cast<double>(timeMs), qDefs::MILLISECONDS));
                auto time = qDefs::getUserFriendlyTime(timeNS);
                spinTimeGap->setValue(time.first);
                comboTimeGapUnit->setCurrentIndex(
                    static_cast<int>(time.second));
            }
            CATCH_DISPLAY("Could not get streaming timer.",
                          "qTabPlot::GetStreamingFrequency")
        }
        // every nth frame
        else {
            comboFrequency->setCurrentIndex(1);
            stackedTimeInterval->setCurrentIndex(1);
            spinNthFrame->setValue(freq);
        }
    }
    CATCH_DISPLAY("Could not get streaming frequency.",
                  "qTabPlot::GetStreamingFrequency")
    connect(comboFrequency, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetStreamingFrequency()));
    connect(comboTimeGapUnit, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetStreamingFrequency()));
    connect(spinTimeGap, SIGNAL(valueChanged(double)), this,
            SLOT(SetStreamingFrequency()));
    connect(spinNthFrame, SIGNAL(valueChanged(int)), this,
            SLOT(SetStreamingFrequency()));
}

void qTabPlot::SetStreamingFrequency() {
    bool frequency = (comboFrequency->currentIndex() == 0) ? 0 : 1;
    auto freqVal = spinNthFrame->value();
    auto timeVal = spinTimeGap->value();
    auto timeUnit =
        static_cast<qDefs::timeUnit>(comboTimeGapUnit->currentIndex());
    stackedTimeInterval->setCurrentIndex(comboFrequency->currentIndex());
    try {
        if (frequency) {
            LOG(logINFO) << "Setting Streaming Frequency to " << freqVal;
            det->setRxZmqFrequency(freqVal);
        } else {
            LOG(logINFO) << "Setting Streaming Frequency to " << 0
                         << " (timer)";
            det->setRxZmqFrequency(0);
            LOG(logINFO) << "Setting Streaming Timer to " << timeVal << " "
                         << qDefs::getUnitString(timeUnit);
            auto timeMS = qDefs::getMSTime(std::make_pair(timeVal, timeUnit));
            det->setRxZmqTimer(timeMS.count());
        }
    }
    CATCH_HANDLE("Could not set streaming frequency/ timer.",
                 "qTabPlot::SetStreamingFrequency", this,
                 &qTabPlot::GetStreamingFrequency)
}

void qTabPlot::SelectHwm(int value) { GetHwm(); }

void qTabPlot::GetHwm() {
    if (comboHwm->currentIndex() == SND_HWM)
        GetStreamingHwm();
    else
        GetReceivingHwm();
}

void qTabPlot::SetHwm(int value) {
    if (comboHwm->currentIndex() == SND_HWM)
        SetStreamingHwm(value);
    else
        SetReceivingHwm(value);
}

void qTabPlot::GetStreamingHwm() {
    LOG(logDEBUG) << "Getting Streaming Hwm for receiver";
    disconnect(spinHwm, SIGNAL(valueChanged(int)), this, SLOT(SetHwm(int)));
    try {
        int value = det->getRxZmqHwm().tsquash(
            "Inconsistent streaming hwm for all receivers.");
        LOG(logDEBUG) << "Got streaming hwm for receiver " << value;
        spinHwm->setValue(value);
    }
    CATCH_DISPLAY("Could not get streaming hwm for receiver.",
                  "qTabPlot::GetStreamingHwm")
    connect(spinHwm, SIGNAL(valueChanged(int)), this, SLOT(SetHwm(int)));
}

void qTabPlot::SetStreamingHwm(int value) {
    LOG(logINFO) << "Setting Streaming Hwm for receiver to " << value;
    try {
        det->setRxZmqHwm(value);
    }
    CATCH_HANDLE("Could not set streaming hwm for receiver.",
                 "qTabPlot::SetStreamingHwm", this, &qTabPlot::GetHwm)
}

void qTabPlot::GetReceivingHwm() {
    LOG(logDEBUG) << "Getting Receiving Hwm for client";
    disconnect(spinHwm, SIGNAL(valueChanged(int)), this, SLOT(SetHwm(int)));
    try {
        int value = det->getClientZmqHwm();
        LOG(logDEBUG) << "Got receiving hwm for client " << value;
        spinHwm->setValue(value);
    }
    CATCH_DISPLAY("Could not get receiving hwm for client.",
                  "qTabPlot::GetReceivingHwm")
    connect(spinHwm, SIGNAL(valueChanged(int)), this, SLOT(SetHwm(int)));
}

void qTabPlot::SetReceivingHwm(int value) {
    LOG(logINFO) << "Setting Streaming Hwm to " << value;
    try {
        det->setClientZmqHwm(value);
    }
    CATCH_HANDLE("Could not set receiving hwm from client.",
                 "qTabPlot::SetReceivingHwm", this, &qTabPlot::GetHwm)
}

void qTabPlot::Refresh() {
    LOG(logDEBUG) << "**Updating Plot Tab";

    if (!plot->GetIsRunning()) {
        boxFrequency->setEnabled(true);
        GetStreamingFrequency();
        GetHwm();
        // gain plot
        switch (det->getDetectorType().squash()) {
        case slsDetectorDefs::JUNGFRAU:
            chkGainPlot->setEnabled(true);
            GetGapPixels();
            break;
        case slsDetectorDefs::GOTTHARD2:
            chkGainPlot1D->setEnabled(true);
            break;
        default:
            break;
        }
        // gap pixels
        if (isGapPixelsAllowed) {
            chkGapPixels->setEnabled(true);
            GetGapPixels();
        }
    } else {
        boxFrequency->setEnabled(false);
        chkGainPlot->setEnabled(false);
        chkGainPlot1D->setEnabled(false);
        chkGapPixels->setEnabled(false);
    }

    LOG(logDEBUG) << "**Updated Plot Tab";
}

} // namespace sls
