#include "qTabPlot.h"
#include "qDefs.h"
#include "qDrawPlot.h"
#include <QAbstractButton>
#include <QButtonGroup>
#include <QStackedLayout>
#include <QStandardItemModel>


QString qTabPlot::defaultPlotTitle("");
QString qTabPlot::defaultHistXAxisTitle("Channel Number");
QString qTabPlot::defaultHistYAxisTitle("Counts");
QString qTabPlot::defaultImageXAxisTitle("Pixel");
QString qTabPlot::defaultImageYAxisTitle("Pixel");
QString qTabPlot::defaultImageZAxisTitle("Intensity");

qTabPlot::qTabPlot(QWidget *parent, sls::Detector *detector, qDrawPlot *p)
    : QWidget(parent), det(detector), plot(p), is1d(false) {
    setupUi(this);
    SetupWidgetWindow();
    LOG(logDEBUG) << "Plot ready";
}

qTabPlot::~qTabPlot() { delete btnGroupPlotType; }

void qTabPlot::SetupWidgetWindow() {
    // button group for plot type
    btnGroupPlotType = new QButtonGroup(this);
    btnGroupPlotType->addButton(radioNoPlot, 0);
    btnGroupPlotType->addButton(radioDataGraph, 1);

    // 1D and 2D options
    stackedWidget1D->setCurrentIndex(0);
    stackedWidget2D->setCurrentIndex(0);
    // Plot Axis
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
    case slsDetectorDefs::EIGER:
        chkGapPixels->setEnabled(true);
        break;
    case slsDetectorDefs::JUNGFRAU:
        chkGainPlot->setEnabled(true);
        chkGainPlot->setChecked(true);
        plot->EnableGainPlot(true);
        break;
    default:
        break;
    }

    Select1DPlot(is1d);
    Initialization();
    Refresh();
}

void qTabPlot::Initialization() {
    // Plot arguments box
    connect(btnGroupPlotType, SIGNAL(buttonClicked(int)), this,
            SLOT(SetPlot()));

    // Plotting frequency box
    connect(comboFrequency, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetStreamingFrequency()));
    connect(comboTimeGapUnit, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetStreamingFrequency()));
    connect(spinTimeGap, SIGNAL(editingFinished()), this,
            SLOT(SetStreamingFrequency()));
    connect(spinNthFrame, SIGNAL(editingFinished()), this,
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
    connect(dispXMin, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    connect(dispXMax, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    connect(dispYMin, SIGNAL(editingFinished()), this, SLOT(SetYRange()));
    connect(dispYMax, SIGNAL(editingFinished()), this, SLOT(SetYRange()));
    connect(chkAspectRatio, SIGNAL(toggled(bool)), this,
            SLOT(CheckAspectRatio()));

    connect(chkZMin, SIGNAL(toggled(bool)), this, SLOT(SetZRange()));
    connect(chkZMax, SIGNAL(toggled(bool)), this, SLOT(SetZRange()));
    connect(dispZMin, SIGNAL(editingFinished()), this, SLOT(SetZRange()));
    connect(dispZMax, SIGNAL(editingFinished()), this, SLOT(SetZRange()));
}

void qTabPlot::Select1DPlot(bool enable) {
    LOG(logDEBUG) << "Selecting " << (enable ? "1" : "2") << "D Plot";
    is1d = enable;
    box1D->setEnabled(enable);
    box2D->setEnabled(!enable);
    chkZAxis->setEnabled(!enable);
    dispZAxis->setEnabled(!enable);
    chkZMin->setEnabled(!enable);
    chkZMax->setEnabled(!enable);
    dispZMin->setEnabled(!enable);
    dispZMax->setEnabled(!enable);
    plot->Select1dPlot(enable);
    SetTitles();
    SetXYRange();
    if (!is1d) {
        SetZRange();
    }
}

void qTabPlot::SetPlot() {
    bool plotEnable = false;
    if (radioNoPlot->isChecked()) {
        LOG(logINFO) << "Setting Plot Type: No Plot";
    } else if (radioDataGraph->isChecked()) {
        LOG(logINFO) << "Setting Plot Type: Datagraph";
        plotEnable = true;
    }
    boxFrequency->setEnabled(plotEnable);
    box1D->setEnabled(plotEnable);
    box2D->setEnabled(plotEnable);
    boxSave->setEnabled(plotEnable);
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
    box1D->setTitle(
        QString("1D Plot Options %1").arg(stackedWidget1D->currentIndex() + 1));
}

void qTabPlot::Set1DPlotOptionsLeft() {
    LOG(logDEBUG) << "1D Options Left";
    int i = stackedWidget1D->currentIndex();
    if (i == 0)
        stackedWidget1D->setCurrentIndex(stackedWidget1D->count() - 1);
    else
        stackedWidget1D->setCurrentIndex(i - 1);
    box1D->setTitle(
        QString("1D Plot Options %1").arg(stackedWidget1D->currentIndex() + 1));
}

void qTabPlot::Set2DPlotOptionsRight() {
    LOG(logDEBUG) << "2D Options Right";
    int i = stackedWidget2D->currentIndex();
    if (i == (stackedWidget2D->count() - 1))
        stackedWidget2D->setCurrentIndex(0);
    else
        stackedWidget2D->setCurrentIndex(i + 1);
    box2D->setTitle(
        QString("2D Plot Options %1").arg(stackedWidget2D->currentIndex() + 1));
}

void qTabPlot::Set2DPlotOptionsLeft() {
    LOG(logDEBUG) << "2D Options Left";
    int i = stackedWidget2D->currentIndex();
    if (i == 0)
        stackedWidget2D->setCurrentIndex(stackedWidget2D->count() - 1);
    else
        stackedWidget2D->setCurrentIndex(i - 1);
    box2D->setTitle(
        QString("2D Plot Options %1").arg(stackedWidget2D->currentIndex() + 1));
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
        LOG(logINFO) << "Binary Plot "
                          << (binary1D ? "enabled" : "disabled");
        lblFrom->setEnabled(binary1D);
        lblTo->setEnabled(binary1D);
        spinFrom->setEnabled(binary1D);
        spinTo->setEnabled(binary1D);
        plot->SetBinary(binary1D, spinFrom->value(), spinTo->value());
    } else {
        LOG(logINFO) << "Binary Plot "
                          << (binary2D ? "enabled" : "disabled");
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
            LOG(logDEBUG)
                << "Setting "
                << qDefs::getRangeAsString(static_cast<qDefs::range>(i))
                << " to " << val;
            xyRange[i] = val;
            isRange[i] = true;
            disablezoom = true;
        }
    }

    plot->SetXYRangeChanged(disablezoom, xyRange, isRange);
    emit DisableZoomSignal(disablezoom);
}

void qTabPlot::MaintainAspectRatio(int dimension) {
    LOG(logDEBUG) << "Maintaining Aspect Ratio";

    disconnect(chkXMin, SIGNAL(toggled(bool)), this, SLOT(SetXRange()));
    disconnect(chkXMax, SIGNAL(toggled(bool)), this, SLOT(SetXRange()));
    disconnect(chkYMin, SIGNAL(toggled(bool)), this, SLOT(SetYRange()));
    disconnect(chkYMax, SIGNAL(toggled(bool)), this, SLOT(SetYRange()));
    disconnect(dispXMin, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    disconnect(dispXMax, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    disconnect(dispYMin, SIGNAL(editingFinished()), this, SLOT(SetYRange()));
    disconnect(dispYMax, SIGNAL(editingFinished()), this, SLOT(SetYRange()));

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
    LOG(logDEBUG) << "Ideal Aspect ratio: " << idealAspectratio
                       << " for x(" << ranges[qDefs::XMIN] << " - "
                       << ranges[qDefs::XMAX] << "), y(" << ranges[qDefs::YMIN]
                       << " - " << ranges[qDefs::YMAX] << ")";

    // calculate current aspect ratio
    ranges[qDefs::XMIN] = dispXMin->text().toDouble();
    ranges[qDefs::XMAX] = dispXMax->text().toDouble();
    ranges[qDefs::YMIN] = dispYMin->text().toDouble();
    ranges[qDefs::YMAX] = dispYMax->text().toDouble();
    double currentAspectRatio = (ranges[qDefs::XMAX] - ranges[qDefs::XMIN]) /
                                (ranges[qDefs::YMAX] - ranges[qDefs::YMIN]);
    LOG(logDEBUG) << "Current Aspect ratio: " << currentAspectRatio
                       << " for x(" << ranges[qDefs::XMIN] << " - "
                       << ranges[qDefs::XMAX] << "), y(" << ranges[qDefs::YMIN]
                       << " - " << ranges[qDefs::YMAX] << ")";

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
    connect(dispXMin, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    connect(dispXMax, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    connect(dispYMin, SIGNAL(editingFinished()), this, SLOT(SetYRange()));
    connect(dispYMax, SIGNAL(editingFinished()), this, SLOT(SetYRange()));

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
}

void qTabPlot::GetStreamingFrequency() {
    LOG(logDEBUG) << "Getting Streaming Frequency";
    disconnect(comboFrequency, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetStreamingFrequency()));
    disconnect(comboTimeGapUnit, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetStreamingFrequency()));
    disconnect(spinTimeGap, SIGNAL(editingFinished()), this,
               SLOT(SetStreamingFrequency()));
    disconnect(spinNthFrame, SIGNAL(editingFinished()), this,
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
    connect(spinTimeGap, SIGNAL(editingFinished()), this,
            SLOT(SetStreamingFrequency()));
    connect(spinNthFrame, SIGNAL(editingFinished()), this,
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

void qTabPlot::Refresh() {
    LOG(logDEBUG) << "**Updating Plot Tab";

    if (!plot->GetIsRunning()) {
        boxPlotType->setEnabled(true);

        // streaming frequency
        if (!radioNoPlot->isChecked()) {
            boxFrequency->setEnabled(true);
        }
        GetStreamingFrequency();
        // gain plot, gap pixels enable
        switch (det->getDetectorType().squash()) {
        case slsDetectorDefs::EIGER:
            chkGapPixels->setEnabled(true);
            GetGapPixels();
            break;
        case slsDetectorDefs::JUNGFRAU:
            chkGainPlot->setEnabled(true);
            break;
        case slsDetectorDefs::GOTTHARD2:
            chkGainPlot1D->setEnabled(true);
            break;
        default:
            break;
        }
    } else {
        boxPlotType->setEnabled(false);
        boxFrequency->setEnabled(false);
        chkGainPlot->setEnabled(false);
        chkGainPlot1D->setEnabled(false);
        chkGapPixels->setEnabled(false);
    }

    LOG(logDEBUG) << "**Updated Plot Tab";
}
