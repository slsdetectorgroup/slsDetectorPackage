#include "qTabPlot.h"
#include "qDefs.h"
#include "qDrawPlot.h"

#include <QStandardItemModel>
#include <QStackedLayout>
#include <QButtonGroup>
#include <QAbstractButton>

#include <iostream>
#include <math.h>
#include <string>

QString qTabPlot::defaultPlotTitle("");
QString qTabPlot::defaultHistXAxisTitle("Channel Number");
QString qTabPlot::defaultHistYAxisTitle("Counts");
QString qTabPlot::defaultImageXAxisTitle("Pixel");
QString qTabPlot::defaultImageYAxisTitle("Pixel");
QString qTabPlot::defaultImageZAxisTitle("Intensity");


qTabPlot::qTabPlot(QWidget *parent, multiSlsDetector *detector, qDrawPlot *plot) : 
    QWidget(parent), myDet(detector), myPlot(plot), isOneD(false), isOriginallyOneD(false), wrongInterval(0), 
    stackedLayout(nullptr), spinNthFrame(nullptr), spinTimeGap(nullptr), comboTimeGapUnit(nullptr),
     btnGroupPlotType(0) {
    setupUi(this);
    SetupWidgetWindow();
    FILE_LOG(logDEBUG) << "Plot ready";
}

qTabPlot::~qTabPlot() {}

void qTabPlot::SetupWidgetWindow() {
    // button group for plot type
    btnGroupPlotType = new QButtonGroup(this);
    btnGroupPlotType->addButton(radioNoPlot, 0);
    btnGroupPlotType->addButton(radioDataGraph, 1);
    btnGroupPlotType->addButton(radioHistogram, 2);
    // Plotting Frequency
    stackedLayout = new QStackedLayout;
    stackedLayout->setSpacing(0);
    spinNthFrame = new QSpinBox;
    spinNthFrame->setMinimum(1);
    spinNthFrame->setMaximum(2000000000);
    spinNthFrame->setValue(1);
    spinTimeGap = new QDoubleSpinBox;
    spinTimeGap->setMinimum(0);
    spinTimeGap->setDecimals(3);
    spinTimeGap->setMaximum(999999);
    spinTimeGap->setValue(DEFAULT_STREAMING_TIMER_IN_MS);
    comboTimeGapUnit = new QComboBox;
    comboTimeGapUnit->addItem("hr");
    comboTimeGapUnit->addItem("min");
    comboTimeGapUnit->addItem("s");
    comboTimeGapUnit->addItem("ms");
    comboTimeGapUnit->setCurrentIndex(3);
    QWidget *wTimeInterval = new QWidget;
    QHBoxLayout *h1 = new QHBoxLayout;
    wTimeInterval->setLayout(h1);
    h1->setContentsMargins(0, 0, 0, 0);
    h1->setSpacing(3);
    h1->addWidget(spinTimeGap);
    h1->addWidget(comboTimeGapUnit);
    stackedLayout->addWidget(wTimeInterval);
    stackedLayout->addWidget(spinNthFrame);
    stackWidget->setLayout(stackedLayout);
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
    myPlot->SetPlotTitlePrefix("");
    dispXAxis->setText(defaultHistXAxisTitle);
    dispYAxis->setText(defaultHistYAxisTitle);
    myPlot->SetHistXAxisTitle(defaultHistXAxisTitle);
    myPlot->SetHistYAxisTitle(defaultHistYAxisTitle);
    dispXAxis->setText(defaultImageXAxisTitle);
    dispYAxis->setText(defaultImageYAxisTitle);
    dispZAxis->setText(defaultImageZAxisTitle);
    myPlot->SetImageXAxisTitle(defaultImageXAxisTitle);
    myPlot->SetImageYAxisTitle(defaultImageYAxisTitle);
    myPlot->SetImageZAxisTitle(defaultImageZAxisTitle);

	// enabling according to det type
	switch(myDet->getDetectorTypeAsEnum()) {
    case slsDetectorDefs::GOTTHARD:
        isOriginallyOneD = true;
        break;
    case slsDetectorDefs::EIGER:
        isOriginallyOneD = false;
        pagePedestal->setEnabled(false);
        pagePedestal_2->setEnabled(false);
        chkGapPixels->setEnabled(true);
        break;
    case slsDetectorDefs::JUNGFRAU:
    case slsDetectorDefs::MOENCH:
        isOriginallyOneD = false;
        chkGainPlot->setEnabled(true);
        break;
    default:
        break;
    }

    Select1DPlot(isOriginallyOneD);

    Initialization();

    Refresh();
}

void qTabPlot::Initialization() {
    // Plot arguments box
    connect(btnGroupPlotType, SIGNAL(buttonClicked(int)), this, SLOT(SetPlot()));

    // Plotting frequency box
    connect(comboFrequency, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFrequency()));
    connect(comboTimeGapUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFrequency()));
    connect(spinTimeGap, SIGNAL(editingFinished()), this, SLOT(SetFrequency()));
    connect(spinNthFrame, SIGNAL(editingFinished()), this, SLOT(SetFrequency()));

    // navigation buttons for options
    connect(btnRight1D, SIGNAL(clicked()), this, SLOT(Set1DPlotOptionsRight()));
    connect(btnLeft1D, SIGNAL(clicked()), this, SLOT(Set1DPlotOptionsLeft()));
    connect(btnRight2D, SIGNAL(clicked()), this, SLOT(Set2DPlotOptionsRight()));
    connect(btnLeft2D, SIGNAL(clicked()), this, SLOT(Set2DPlotOptionsLeft()));
    
    // 1D options
    connect(chkSuperimpose, SIGNAL(toggled(bool)), this, SLOT(EnablePersistency(bool)));
    connect(spinPersistency, SIGNAL(valueChanged(int)), myPlot, SLOT(SetPersistency(int)));
    connect(chkPoints, SIGNAL(toggled(bool)), myPlot, SLOT(SetMarkers(bool)));
    connect(chkLines, SIGNAL(toggled(bool)), myPlot, SLOT(SetLines(bool)));
    connect(chk1DLog, SIGNAL(toggled(bool)), myPlot, SIGNAL(LogySignal(bool)));
    connect(chkStatistics, SIGNAL(toggled(bool)), myPlot, SLOT(DisplayStatistics(bool)));

    // 2D Plot box
    connect(chkInterpolate, SIGNAL(toggled(bool)), myPlot, SIGNAL(InterpolateSignal(bool)));
    connect(chkContour, SIGNAL(toggled(bool)), myPlot, SIGNAL(ContourSignal(bool)));
    connect(chkLogz, SIGNAL(toggled(bool)), myPlot, SIGNAL(LogzSignal(bool)));
    connect(chkStatistics_2, SIGNAL(toggled(bool)), myPlot, SLOT(DisplayStatistics(bool)));
    //pedstal
    connect(chkPedestal, SIGNAL(toggled(bool)), myPlot, SLOT(SetPedestal(bool)));
    connect(btnRecalPedestal, SIGNAL(clicked()), myPlot, SLOT(RecalculatePedestal()));
    connect(chkPedestal_2, SIGNAL(toggled(bool)), myPlot, SLOT(SetPedestal(bool)));
    connect(btnRecalPedestal_2, SIGNAL(clicked()), myPlot, SLOT(RecalculatePedestal()));
    //accumulate
    connect(chkAccumulate, SIGNAL(toggled(bool)), myPlot, SLOT(SetAccumulate(bool)));
    connect(btnResetAccumulate, SIGNAL(clicked()), myPlot, SLOT(ResetAccumulate()));
    connect(chkAccumulate_2, SIGNAL(toggled(bool)), myPlot, SLOT(SetAccumulate(bool)));
    connect(btnResetAccumulate_2, SIGNAL(clicked()), myPlot, SLOT(ResetAccumulate()));
    //binary
    connect(chkBinary, SIGNAL(toggled(bool)), this, SLOT(SetBinary()));
    connect(chkBinary_2, SIGNAL(toggled(bool)), this, SLOT(SetBinary()));
    connect(spinFrom, SIGNAL(valueChanged(int)), this, SLOT(SetBinary()));
    connect(spinFrom_2, SIGNAL(valueChanged(int)), this, SLOT(SetBinary()));
    connect(spinTo, SIGNAL(valueChanged(int)), this, SLOT(SetBinary()));
    connect(spinTo_2, SIGNAL(valueChanged(int)), this, SLOT(SetBinary()));
    //gainplot
    if (chkGainPlot->isEnabled())
        connect(chkGainPlot, SIGNAL(toggled(bool)), myPlot, SIGNAL(GainPlotSignal(bool)));
    // gap pixels
    if (chkGapPixels->isEnabled())
        connect(chkGapPixels, SIGNAL(toggled(bool)), this, SLOT(SetGapPixels(bool)));

    // Save
    connect(btnSave, SIGNAL(clicked()), myPlot, SLOT(SavePlot()));
    connect(chkSaveAll, SIGNAL(toggled(bool)), myPlot, SLOT(SaveAll(bool)));

    // Snapshot box
    connect(btnClone, SIGNAL(clicked()), myPlot, SLOT(ClonePlot()));
    connect(btnCloseClones, SIGNAL(clicked()), myPlot, SLOT(CloseClones()));
    connect(btnSaveClones, SIGNAL(clicked()), myPlot, SLOT(SaveClones()));

    // Plot Axis
    connect(chkTitle, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(chkXAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(chkYAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(chkZAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(dispTitle, SIGNAL(textChanged(const QString &)), this, SLOT(SetTitles()));
    connect(dispXAxis, SIGNAL(textChanged(const QString &)), this, SLOT(SetTitles()));
    connect(dispYAxis, SIGNAL(textChanged(const QString &)), this, SLOT(SetTitles()));
    connect(dispZAxis, SIGNAL(textChanged(const QString &)), this, SLOT(SetTitles()));

    connect(chkXMin, SIGNAL(toggled(bool)), this, SLOT(SetXRange()));
    connect(chkXMax, SIGNAL(toggled(bool)), this, SLOT(SetXRange()));
    connect(chkYMin, SIGNAL(toggled(bool)), this, SLOT(SetYRange()));
    connect(chkYMax, SIGNAL(toggled(bool)), this, SLOT(SetYRange()));
    connect(dispXMin, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    connect(dispXMax, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    connect(dispYMin, SIGNAL(editingFinished()), this, SLOT(SetYRange()));
    connect(dispYMax, SIGNAL(editingFinished()), this, SLOT(SetYRange()));
    connect(chkAspectRatio, SIGNAL(toggled(bool)), this, SLOT(CheckAspectRatio()));

    connect(chkZMin, SIGNAL(toggled(bool)), this, SLOT(SetZRange()));
    connect(chkZMax, SIGNAL(toggled(bool)), this, SLOT(SetZRange()));
    connect(dispZMin, SIGNAL(editingFinished()), this, SLOT(SetZRange()));
    connect(dispZMax, SIGNAL(editingFinished()), this, SLOT(SetZRange()));
    connect(this, SIGNAL(ResetZMinZMaxSignal(bool, bool, double, double)), myPlot, SIGNAL(ResetZMinZMaxSignal(bool, bool, double, double)));
}

xxxxxxxxxxxxxxx

void qTabPlot::Set1DPlotOptionsRight() {
    FILE_LOG(logDEBUG) << "1D Options Right";
    int i = stackedWidget->currentIndex();
    if (i == (stackedWidget->count() - 1))
        stackedWidget->setCurrentIndex(0);
    else
        stackedWidget->setCurrentIndex(i + 1);
    box1D->setTitle(QString("1D Plot Options %1").arg(stackedWidget->currentIndex() + 1));
}

void qTabPlot::Set1DPlotOptionsLeft() {
    FILE_LOG(logDEBUG) << "1D Options Left";
    int i = stackedWidget->currentIndex();
    if (i == 0)
        stackedWidget->setCurrentIndex(stackedWidget->count() - 1);
    else
        stackedWidget->setCurrentIndex(i - 1);
    box1D->setTitle(QString("1D Plot Options %1").arg(stackedWidget->currentIndex() + 1));
}

void qTabPlot::Set2DPlotOptionsRight() {
    FILE_LOG(logDEBUG) << "2D Options Right";
    int i = stackedWidget_2->currentIndex();
    if (i == (stackedWidget_2->count() - 1))
        stackedWidget_2->setCurrentIndex(0);
    else
        stackedWidget_2->setCurrentIndex(i + 1);
    box2D->setTitle(QString("2D Plot Options %1").arg(stackedWidget_2->currentIndex() + 1));
}

void qTabPlot::Set2DPlotOptionsLeft() {
    FILE_LOG(logDEBUG) << "2D Options Left";
    int i = stackedWidget_2->currentIndex();
    if (i == 0)
        stackedWidget_2->setCurrentIndex(stackedWidget_2->count() - 1);
    else
        stackedWidget_2->setCurrentIndex(i - 1);
    box2D->setTitle(QString("2D Plot Options %1").arg(stackedWidget_2->currentIndex() + 1));
}

void qTabPlot::EnablePersistency(bool enable) {
    FILE_LOG(logINFO) << "Superimpose " << (enable ? "enabled" : "disabled");
    lblPersistency->setEnabled(val);
    spinPersistency->setEnabled(val);
    if (enable)
        myPlot->SetPersistency(spinPersistency->value());
    else
        myPlot->SetPersistency(0);
}

void qTabPlot::SetBinary() {
    bool binary1D = chkBinary->isChecked();
    bool binary2D = chkBinary_2->isChecked(); 
    if (isOneD) {
        FILE_LOG(logINFO) << "Binary Plot " << (binary1D ? "enabled" : "disabled");
        lblFrom->setEnabled(binary1D);
        lblTo->setEnabled(binary1D);
        spinFrom->setEnabled(binary1D);
        spinTo->setEnabled(binary1D);
        myPlot->SetBinary(binary1D, spinFrom->value(), spinTo->value());
    } else {
        FILE_LOG(logINFO) << "Binary Plot " << (binary2D ? "enabled" : "disabled");
        lblFrom_2->setEnabled(binary2D);
        lblTo_2->setEnabled(binary2D);
        spinFrom_2->setEnabled(binary2D);
        spinTo_2->setEnabled(binary2D);
        myPlot->SetBinary(binary2D, spinFrom_2->value(), spinTo_2->value());
    }
}

void qTabPlot::GetGapPixels() {
    FILE_LOG(logDEBUG) << "Getting gap pixels";
    disconnect(chkGapPixels, SIGNAL(toggled(bool)), this, SLOT(SetGapPixels(bool)));

	try {
        auto retval = myDet->enableGapPixels(-1);
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Gap pixels enable is inconsistent for all detectors.", "qTabPlot::GetGapPixels");
		} else {
			chkGapPixels->setChecked(retval == 0 ? false : true);
		}
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get gap pixels enable.", e.what(), "qTabPlot::GetGapPixels");
    }

    connect(chkGapPixels, SIGNAL(toggled(bool)), this, SLOT(SetGapPixels(bool)));
}

void qTabPlot::SetGapPixels(bool enable) {
	FILE_LOG(logINFO) << "Setting Gap Pixels Enable to " << enable;

	try {
        myDet->enableGapPixels(enable);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set gap pixels enable.", e.what(), "qTabPlot::SetGapPixels");
        GetGapPixels();
    }
}

void qTabPlot::SetTitles() {
    FILE_LOG(logDEBUG) << "Setting Plot Titles";
    disconnect(chkTitle, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    disconnect(chkXAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    disconnect(chkYAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    disconnect(chkZAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    disconnect(dispTitle, SIGNAL(textChanged(const QString &)), this, SLOT(SetTitles()));
    disconnect(dispXAxis, SIGNAL(textChanged(const QString &)), this, SLOT(SetTitles()));
    disconnect(dispYAxis, SIGNAL(textChanged(const QString &)), this, SLOT(SetTitles()));
    disconnect(dispZAxis, SIGNAL(textChanged(const QString &)), this, SLOT(SetTitles()));

    // title
    if (!chkTitle->isChecked() || dispTitle->text().isEmpty()) {
        myPlot->SetPlotTitlePrefix("");
        dispTitle->setText("");   
    } else {
        myPlot->SetPlotTitlePrefix(dispTitle->text());
    }
    // x
    if (!chkXAxis->isChecked() || dispXAxis->text().isEmpty()) {
        dispXAxis->setText(isOneD ? defaultHistXAxisTitle : defaultImageXAxisTitle);
        myPlot->SetXAxisTitle(isOneD ? defaultHistXAxisTitle : defaultImageXAxisTitle);
    } else {
        myPlot->SetXAxisTitle(dispXAxis->text());
    } 
    // y
    if (!chkYAxis->isChecked() || dispYAxis->text().isEmpty()) {
        dispYAxis->setText(isOneD ? defaultHistYAxisTitle : defaultImageYAxisTitle);
        myPlot->SetYAxisTitle(isOneD ? defaultHistYAxisTitle : defaultImageYAxisTitle);
    } else {
        myPlot->SetYAxisTitle(dispYAxis->text());
    }  
    // z
    if (!chkZAxis->isChecked() || dispZAxis->text().isEmpty()) {
        myPlot->SetZAxisTitle(defaultImageZAxisTitle);
        dispZAxis->setText(defaultImageZAxisTitle);
    } else {
        myPlot->SetZAxisTitle(dispZAxis->text());
    }

    connect(chkTitle, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(chkXAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(chkYAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(chkZAxis, SIGNAL(toggled(bool)), this, SLOT(SetTitles()));
    connect(dispTitle, SIGNAL(textChanged(const QString &)), this, SLOT(SetTitles()));
    connect(dispXAxis, SIGNAL(textChanged(const QString &)), this, SLOT(SetTitles()));
    connect(dispYAxis, SIGNAL(textChanged(const QString &)), this, SLOT(SetTitles()));
    connect(dispZAxis, SIGNAL(textChanged(const QString &)), this, SLOT(SetTitles()));
}

void qTabPlot::SetXRange() {
    FILE_LOG(logDEBUG) << "Enable X axis range";

    if (chkAspectRatio->isChecked()) {
        MaintainAspectRatio(static_cast<int>(slsDetectorDefs::Y));
    } else {
        SetXYRange();
    }
}

void qTabPlot::SetYRange() {
    FILE_LOG(logDEBUG) << "Enable Y axis range";

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
    FILE_LOG(LOGDEBUG) << "Set XY Range";
    disconnect(dispXMin, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    disconnect(dispXMax, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    disconnect(dispYMin, SIGNAL(editingFinished()), this, SLOT(SetYRange()));
    disconnect(dispYMax, SIGNAL(editingFinished()), this, SLOT(SetYRange()));

    bool disablezoom = false;

    // xmin
    // if unchecked, empty or invalid (set to false so it takes the min/max of plot)
    if (!chkXMin->isChecked() || dispXMin->text().isEmpty()) {
        myPlot->IsXYRangeValues(false, qDefs::XMINIMUM);
    } else if (dispXMin->text().toDouble() < myPlot->GetXMinimum()) {
        qDefs::Message(qDefs::WARNING, "XMin Outside Plot Range", "qTabPlot::SetXRange");
        dispXMin->setText("");
        myPlot->IsXYRangeValues(false, qDefs::XMINIMUM);
    } else {
        myPlot->SetXYRangeValues(dispXMin->text().toDouble(), qDefs::XMINIMUM);
        myPlot->IsXYRangeValues(true, qDefs::XMINIMUM);
        disablezoom = true;
    } 

    //xmax
    if (!chkXMax->isChecked() || dispXMax->text().isEmpty()) {
        myPlot->IsXYRangeValues(false, qDefs::XMAXIMUM);
    } else if (dispXMax->text().toDouble() < myPlot->GetXMaximum()) {
        qDefs::Message(qDefs::WARNING, "XMax Outside Plot Range", "qTabPlot::SetXYRange");
        dispXMax->setText("");
        myPlot->IsXYRangeValues(false, qDefs::XMAXIMUM);
    } else {
        myPlot->SetXYRangeValues(dispXMax->text().toDouble(), qDefs::XMAXIMUM);
        myPlot->IsXYRangeValues(true, qDefs::XMAXIMUM);
        disablezoom = true;
    } 
    
    // ymin
    if (!chkYMin->isChecked() || dispYMin->text().isEmpty()) {
        myPlot->IsXYRangeValues(false, qDefs::YMINIMUM);
    } else if (dispYMin->text().toDouble() < myPlot->GetYMinimum()) {
        qDefs::Message(qDefs::WARNING, "YMin Outside Plot Range", "qTabPlot::SetXYRange");
        dispYMin->setText("");
        myPlot->IsXYRangeValues(false, qDefs::YMINIMUM);
    } else {
        myPlot->SetXYRangeValues(dispYMin->text().toDouble(), qDefs::YMINIMUM);
        myPlot->IsXYRangeValues(true, qDefs::YMINIMUM);
        disablezoom = true;
    } 

    //ymax
    if (!chkYMax->isChecked() || dispYMax->text().isEmpty()) {
        myPlot->IsXYRangeValues(false, qDefs::YMAXIMUM);
    } else if (dispYMax->text().toDouble() < myPlot->GetYMaximum()) {
        qDefs::Message(qDefs::WARNING, "YMax Outside Plot Range", "qTabPlot::SetXYRange");
        dispYMax->setText("");
        myPlot->IsXYRangeValues(false, qDefs::YMAXIMUM);
    } else {
        myPlot->SetXYRangeValues(dispYMax->text().toDouble(), qDefs::YMAXIMUM);
        myPlot->IsXYRangeValues(true, qDefs::YMAXIMUM);
        disablezoom = true;
    } 

    connect(dispXMin, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    connect(dispXMax, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    connect(dispYMin, SIGNAL(editingFinished()), this, SLOT(SetYRange()));
    connect(dispYMax, SIGNAL(editingFinished()), this, SLOT(SetYRange()));

    // to update plot with range
    myPlot->SetXYRange(true);
    myPlot->DisableZoom(disablezoom);
    emit DisableZoomSignal(disablezoom);
}

void qTabPlot::MaintainAspectRatio(int dimension) {
    FILE_LOG(logDEBUG) << "Maintaining Aspect Ratio";

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
        dispXMin->setText(QString::number(myPlot->GetXMinimum()));
	if (dispXMax->text().isEmpty()) 
        dispXMax->setText(QString::number(myPlot->GetXMaximum()));
	if (dispYMin->text().isEmpty()) 
        dispYMin->setText(QString::number(myPlot->GetYMinimum()));
	if (dispYMax->text().isEmpty()) 
        dispYMax->setText(QString::number(myPlot->GetYMaximum()));

    // calculate ideal aspect ratio with previous limits
    double ranges[4];
    ranges[qDefs::XMINIMUM] = myPlot->GetXMinimum();
    ranges[qDefs::XMAXIMUM] = myPlot->GetXMaximum();
    ranges[qDefs::YMINIMUM] = myPlot->GetYMinimum();
    ranges[qDefs::YMAXIMUM] = myPlot->GetYMaximum();
    double idealAspectratio = (ranges[qDefs::XMAXIMUM] - ranges[qDefs::XMINIMUM]) / (ranges[qDefs::YMAXIMUM] - ranges[qDefs::YMINIMUM]);
    FILE_LOG(logDEBUG) << "Ideal Aspect ratio: %f for x(%f - %f), y(%f - %f)\n", idealAspectratio, ranges[qDefs::XMINIMUM], ranges[qDefs::XMAXIMUM], ranges[qDefs::YMINIMUM], ranges[qDefs::YMAXIMUM]);
  
    // calculate current aspect ratio
    ranges[qDefs::XMINIMUM] = dispXMin->text().toDouble();
    ranges[qDefs::XMAXIMUM] = dispXMax->text().toDouble();
    ranges[qDefs::YMINIMUM] = dispYMin->text().toDouble();
    ranges[qDefs::YMAXIMUM] = dispYMax->text().toDouble();
    double currentAspectRatio = (ranges[qDefs::XMAXIMUM] - ranges[qDefs::XMINIMUM]) / (ranges[qDefs::YMAXIMUM] - ranges[qDefs::YMINIMUM]);
    FILE_LOG(logDEBUG) << "Current Aspect ratio: %f for x(%f - %f), y(%f - %f)\n", currentAspectRatio, ranges[qDefs::XMINIMUM], ranges[qDefs::XMAXIMUM], ranges[qDefs::YMINIMUM], ranges[qDefs::YMAXIMUM]);

    if (newAspectRatio != idealAspectratio) {
        // dimension: 1(x changed: y adjusted), 0(y changed: x adjusted), -1(aspect ratio clicked: larger one adjusted)
        if (dimension == -1) {
            dimension = ((ranges[qDefs::XMAXIMUM] - ranges[qDefs::XMINIMUM]) > (ranges[qDefs::YMAXIMUM] - ranges[qDefs::YMINIMUM])) 
            ? static_cast<int>(slsDetectorDefs::X) : static_cast<int>(slsDetectorDefs::Y);
        }

        // calculate new value to maintain aspect ratio
        // adjust x
        double newval = 0;
        if (dimension == static_cast<int>(slsDetectorDefs::X)) {
            newval = idealAspectratio * (ranges[qDefs::YMAXIMUM] - ranges[qDefs::YMINIMUM]) + ranges[qDefs::XMINIMUM];
            if (newval <= myPlot->GetXMaximum()) {
                dispXMax->setText(QString::number(newval));
                FILE_LOG(logDEBUG) << "New XMax: " << newval;
            } else {
                newval = ranges[qDefs::XMAXIMUM] - (idealAspectratio * (ranges[qDefs::YMAXIMUM] - ranges[qDefs::YMINIMUM]));
                dispXMin->setText(QString::number(newval));
                FILE_LOG(logDEBUG) << "New XMin: " << newval;
            }
        }
        // adjust y
        else  {
            newval = ((ranges[qDefs::XMAXIMUM] - ranges[qDefs::XMINIMUM]) / idealAspectratio) + ranges[qDefs::YMINIMUM];
            if (newval <= myPlot->GetYMaximum()) {
                dispYMax->setText(QString::number(newval));
                FILE_LOG(logDEBUG) << "New YMax: " << newval;
            } else {
                newval = ranges[qDefs::YMAXIMUM] - ((ranges[qDefs::XMAXIMUM] - ranges[qDefs::XMINIMUM]) / idealAspectratio);
                dispYMin->setText(QString::number(newval));
                FILE_LOG(logDEBUG) << "New YMax: " << newval;
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

    // set XY values in plot
    myPlot->SetXYRangeValues(dispXMin->text().toDouble(), qDefs::XMINIMUM);
    myPlot->SetXYRangeValues(dispXMax->text().toDouble(), qDefs::XMAXIMUM);
    myPlot->SetXYRangeValues(dispYMin->text().toDouble(), qDefs::YMINIMUM);
    myPlot->SetXYRangeValues(dispYMax->text().toDouble(), qDefs::YMAXIMUM);

    myPlot->IsXYRangeValues(true, qDefs::XMINIMUM);
    myPlot->IsXYRangeValues(true, qDefs::XMAXIMUM);
    myPlot->IsXYRangeValues(true, qDefs::YMINIMUM);
    myPlot->IsXYRangeValues(true, qDefs::YMAXIMUM);

    // to update plot with range
    myPlot->SetXYRange(true);
    myPlot->DisableZoom(true);
    emit DisableZoomSignal(true);
}


void qTabPlot::SetZRange() {
    bool isZmin = chkZMin->isChecked();
    bool isZmax = chkZMax->isChecked();
    double zmin = 0, zmax = 1;
    if (!dispZMin->text().empty()) {
        zmin = dispZMin->text().toDouble();
    }
    if (!dispZMax->text().empty()) {
        zmax = dispZMax->text().toDouble();
    }  
    emit ResetZMinZMaxSignal(isZmin, isZmax, zmin, zmax);
}















































































void qTabPlot::Select1DPlot(bool b) {
#ifdef VERBOSE
    if (b)
        cout << "Selecting 1D Plot" << endl;
    else
        cout << "Selecting 2D Plot" << endl;
#endif
    isOneD = b;
    lblFrom->setEnabled(false);
    lblTo->setEnabled(false);
    lblFrom_2->setEnabled(false);
    lblTo_2->setEnabled(false);
    spinFrom->setEnabled(false);
    spinFrom_2->setEnabled(false);
    spinTo->setEnabled(false);
    spinTo_2->setEnabled(false);
    if (b) {
        box1D->show();
        box2D->hide();
        chkZAxis->setEnabled(false);
        chkZMin->setEnabled(false);
        chkZMax->setEnabled(false);
        myPlot->Select1DPlot();
    } else {
        box1D->hide();
        box2D->show();
        chkZAxis->setEnabled(true);
        chkZMin->setEnabled(true);
        chkZMax->setEnabled(true);
        myPlot->Select2DPlot();
    }
}














void qTabPlot::SetPlot() {
#ifdef VERBOSE
    cout << "Entering Set Plot()";
#endif
    if (radioNoPlot->isChecked()) {
        cout << " - No Plot" << endl;

        boxScan->show();
        boxHistogram->hide();
        myPlot->EnablePlot(false);
        boxSnapshot->setEnabled(false);
        boxSave->setEnabled(false);
        boxFrequency->setEnabled(false);
        boxPlotAxis->setEnabled(false);
        boxScan->setEnabled(false);

    } else if (radioDataGraph->isChecked()) {
        cout << " - DataGraph" << endl;

        boxScan->show();
        boxHistogram->hide();
        myPlot->EnablePlot(true);
        Select1DPlot(isOriginallyOneD);
        boxSnapshot->setEnabled(true);
        boxSave->setEnabled(true);
        boxFrequency->setEnabled(true);
        boxPlotAxis->setEnabled(true);
        // if(!myPlot->isRunning())
        // 	EnableScanBox();
        //  To remind the updateplot in qdrawplot to set range after updating plot
        myPlot->SetXYRange(true);
    } else {
        //histogram and 2d scans dont work
        if (boxScan->isChecked()) {
            qDefs::Message(qDefs::WARNING, "<nobr>Histogram cannot be used together with 2D Scan Plots.</nobr><br>"
                                           "<nobr>Uncheck <b>2D Scan</b> plots to plot <b>Histograms</b></nobr>",
                           "qTabPlot::SetPlot");
            radioDataGraph->setChecked(true);
            boxScan->show();
            boxHistogram->hide();
            return;
        }

        cout << " - Histogram" << endl;

        if (radioHistIntensity->isChecked()) {
            pageHistogram->setEnabled(true);
            pageHistogram_2->setEnabled(true);
        } else {
            pageHistogram->setEnabled(false);
            pageHistogram_2->setEnabled(false);
        }
        boxScan->hide();
        boxHistogram->show();
        myPlot->EnablePlot(true);
        Select1DPlot(isOriginallyOneD);
        boxSnapshot->setEnabled(true);
        boxSave->setEnabled(true);
        boxFrequency->setEnabled(true);
        boxPlotAxis->setEnabled(true);
        // if(!myPlot->isRunning())
        // 	EnableScanBox();

        //qDefs::Message(qDefs::INFORMATION,"<nobr>Please check the <b>Plot Histogram Options</b> below "
        //		"before <b>Starting Acquitision</b></nobr>","qTabPlot::SetPlot");
    }
}


void qTabPlot::SetFrequency() {
#ifdef VERBOSE
    cout << "Setting Plot Interval Frequency" << endl;
#endif
    disconnect(comboTimeGapUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFrequency()));
    disconnect(spinTimeGap, SIGNAL(editingFinished()), this, SLOT(SetFrequency()));
    disconnect(spinNthFrame, SIGNAL(editingFinished()), this, SLOT(SetFrequency()));
    disconnect(comboFrequency, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFrequency()));
    double timeMS, acqPeriodMS;
    double minPlotTimer = myPlot->GetMinimumPlotTimer();
    char cMin[200];
    sprintf(cMin, "%f ms", minPlotTimer);

    acqPeriodMS = (myDet->setTimer(slsDetectorDefs::FRAME_PERIOD, -1) * (1E-6));
    //if period is 0, check exptime, if that is also 0, give warning and set to min timer
    if (acqPeriodMS == 0) {
        acqPeriodMS = (myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME, -1) * (1E-6));

        if (acqPeriodMS == 0) {
            //to reduce the warnings displayed
            if ((comboFrequency->currentIndex() == 0) && (spinTimeGap->value() == minPlotTimer))
                ;
            else
                qDefs::Message(qDefs::WARNING, "<nobr>Interval between Plots:</nobr><br><nobr>"
                                               "<b>Every Nth Image</b>: Period betwen Frames and Exposure Time cannot both be 0 ms.</nobr><br><nobr>"
                                               "Resetting to minimum plotting time interval",
                               "qTabPlot::SetFrequency");
            comboFrequency->setCurrentIndex(0);
            stackedLayout->setCurrentIndex(comboFrequency->currentIndex());
            spinTimeGap->setValue(minPlotTimer);
            comboTimeGapUnit->setCurrentIndex(qDefs::MILLISECONDS);
            timeMS = minPlotTimer;
            //This is done so that its known which one was selected
            myPlot->SetFrameFactor(0);
            // Setting the timer value(ms) between plots
            myPlot->SetPlotTimer(timeMS);

            connect(comboTimeGapUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFrequency()));
            connect(spinTimeGap, SIGNAL(editingFinished()), this, SLOT(SetFrequency()));
            connect(spinNthFrame, SIGNAL(editingFinished()), this, SLOT(SetFrequency()));
            connect(comboFrequency, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFrequency()));
            qDefs::checkErrorMessage(myDet, "qTabPlot::SetFrequency");
            return;
        }
    }

    stackedLayout->setCurrentIndex(comboFrequency->currentIndex());
    switch (comboFrequency->currentIndex()) {
    case 0:
        // Get the time interval from gui in ms
        timeMS = (qDefs::getNSTime((qDefs::timeUnit)comboTimeGapUnit->currentIndex(), spinTimeGap->value())) / (1e6);

        if ((int)timeMS == 0) {
            qDefs::Message(qDefs::WARNING, "<nobr>Interval between Plots:</nobr><br><nobr>"
                                           "Time Interval must be atleast >= 1 ms. Resetting to minimum plotting time interval.",
                           "qTabPlot::SetFrequency");
            spinTimeGap->setValue(minPlotTimer);
            comboTimeGapUnit->setCurrentIndex(qDefs::MILLISECONDS);
            timeMS = minPlotTimer;
        }

        //show red if min interval<minplottimer
        if (timeMS < minPlotTimer) {
            //qDefs::Message(qDefs::WARNING,"<nobr>Interval between Plots: You might be losing Images!</nobr>","Plot");
            boxFrequency->setPalette(*red);
            boxFrequency->setTitle("Interval between Plots*");
            QString errTip = intervalTip + QString("<br><br><font color=\"red\"><nobr>"
                                                   "<b>Time Interval</b> Condition: min of ") +
                             QString("%1").arg(minPlotTimer) +
                             QString("ms.</nobr><br><nobr>You might be losing images!</nobr></font>");
            boxFrequency->setToolTip(errTip);
        }
        //show red if acqPeriod<minInterval
        else if ((acqPeriodMS + 1) < timeMS) {
            cout << "\nacqPeriodMS:" << acqPeriodMS << "\ttimeMS:" << timeMS << endl;
            //qDefs::Message(qDefs::WARNING,"<nobr>Interval between Plots: You might be losing Images!</nobr>","Plot");
            boxFrequency->setPalette(*red);
            boxFrequency->setTitle("Interval between Plots*");
            QString errTip = intervalTip + QString("<br><br><font color=\"red\"><nobr>"
                                                   "<b>Time Interval</b> Acquisition Period should be >= Time Interval between plots.</nobr><br><nobr>"
                                                   "You might be losing images!</nobr></font>");
            boxFrequency->setToolTip(errTip);
        }
        //correct
        else {
            boxFrequency->setPalette(boxSnapshot->palette());
            boxFrequency->setTitle("Interval between Plots");
            boxFrequency->setToolTip(intervalTip);
        }

        //This is done so that its known which one was selected
        myPlot->SetFrameFactor(0);
        // Setting the timer value(ms) between plots
        myPlot->SetPlotTimer(timeMS);
#ifdef VERBOSE
        cout << "Plotting Frequency: Time Gap - " << spinTimeGap->value() << qDefs::getUnitString((qDefs::timeUnit)comboTimeGapUnit->currentIndex()) << endl;
#endif
        break;

    case 1:

        // gets the acq period * number of nth frames
        timeMS = (spinNthFrame->value()) * acqPeriodMS;

        //Show red to make sure the period between plotting is not less than minimum plot timer in  ms
        if (timeMS < minPlotTimer) {
            int minFrame = (int)(ceil)(minPlotTimer / acqPeriodMS);
            //qDefs::Message(qDefs::WARNING,"<nobr>Interval between Plots: You might be losing Images!</nobr>","Plot");
            boxFrequency->setPalette(*red);
            boxFrequency->setTitle("Interval between Plots*");
            QString errTip = intervalTip + QString("<br><br><font color=\"red\"><nobr>"
                                                   "<b>Every nth Image</b> Condition: min nth Image for this time period: ") +
                             QString("%1").arg(minFrame) +
                             QString(".</nobr><br><nobr>You might be losing images!</nobr></font>");
            boxFrequency->setToolTip(errTip);
        } else {
            boxFrequency->setPalette(boxSnapshot->palette());
            boxFrequency->setTitle("Interval between Plots");
            boxFrequency->setToolTip(intervalTip);
        }

        // Setting the timer value (nth frames) between plots
        myPlot->SetFrameFactor(spinNthFrame->value());

#ifdef VERBOSE
        cout << "Plotting Frequency: Nth Frame - " << spinNthFrame->value() << endl;
#endif
        break;
    }

    connect(comboTimeGapUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFrequency()));
    connect(spinTimeGap, SIGNAL(editingFinished()), this, SLOT(SetFrequency()));
    connect(spinNthFrame, SIGNAL(editingFinished()), this, SLOT(SetFrequency()));
    connect(comboFrequency, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFrequency()));
    qDefs::checkErrorMessage(myDet, "qTabPlot::SetFrequency");
}

 void qTabPlot::EnableScanBox(){
 #ifdef VERBOSE
 	cout << "Entering Enable Scan Box"<< endl;
 #endif
 	disconnect(btnGroupPlotType,SIGNAL(buttonClicked(int)),this, SLOT(SetPlot()));
 	disconnect(boxScan,	  	SIGNAL(toggled(bool)),		this, SLOT(EnableScanBox()));

 	int oldfreqvalue = myDet->setReadReceiverFrequency();

 	int mode0 = myDet->getScanMode(0);
 	int mode1 = myDet->getScanMode(1);

 	radioHistLevel0->setEnabled(mode0);
 	radioHistLevel1->setEnabled(mode1);
 	int ang;
 	bool angConvert = myDet->getAngularConversion(ang);
 	myPlot->EnableAnglePlot(angConvert);

 	radioDataGraph->setEnabled(true);
 	radioHistogram->setEnabled(true);
 	chkSuperimpose->setEnabled(true);
 	pageAccumulate->setEnabled(true);
 	pageAccumulate_2->setEnabled(true);
 	if((myDet->getDetectorsType() == slsDetectorDefs::GOTTHARD) ||
 			(myDet->getDetectorsType() == slsDetectorDefs::PROPIX) ||
 			(myDet->getDetectorsType() == slsDetectorDefs::JUNGFRAU) ||
 			(myDet->getDetectorsType() == slsDetectorDefs::MOENCH)){
 		pagePedestal->setEnabled(true);
 		pagePedestal_2->setEnabled(true);
 		chkBinary->setEnabled(true);
 		chkBinary_2->setEnabled(true);
 	}

 	//if angle plot or originally 2d, uncheck and disable scanbox
 	if ((angConvert) || (!isOriginallyOneD)){
 		boxScan->setChecked(false);
 		boxScan->setEnabled(false);

 		/**Newly added*/
 		// To remind the updateplot in qdrawplot to set range after updating plot
 		if(!isOriginallyOneD)
 			myPlot->SetXYRange(true);

 		//2d scans read every frame, not compulsory, but for historgrams
 		if((!isOriginallyOneD) && (mode0 || mode1)){
 			//read every frame
 			disconnect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
 			disconnect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
 			comboFrequency->setCurrentIndex(1);
 			spinNthFrame->setValue(1);
 			SetFrequency();
 			connect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
 			connect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
 		}

 		//persistency, accumulate, pedestal, binary
 		if(angConvert){
 			if(chkSuperimpose->isChecked())	chkSuperimpose->setChecked(false);
 			if(chkPedestal->isChecked())	chkPedestal->setChecked(false);
 			if(chkPedestal_2->isChecked())	chkPedestal_2->setChecked(false);
 			if(chkAccumulate->isChecked())	chkAccumulate->setChecked(false);
 			if(chkAccumulate_2->isChecked())chkAccumulate_2->setChecked(false);
 			if(chkBinary->isChecked())		chkBinary->setChecked(false);
 			if(chkBinary_2->isChecked())	chkBinary_2->setChecked(false);
 			pagePedestal->setEnabled(false);
 			pagePedestal_2->setEnabled(false);
 			chkBinary->setEnabled(false);
 			chkBinary_2->setEnabled(false);
 			pageAccumulate->setEnabled(false);
 			pageAccumulate_2->setEnabled(false);
 		}

 		if(angConvert){
 			boxScan->setToolTip("<nobr>Only 1D Plots enabled for Angle Plots</nobr>");
 			//disable histogram
 			if(radioHistogram->isChecked()){
 				radioDataGraph->setChecked(true);
 				radioHistogram->setEnabled(false);
 				//  To remind the updateplot in qdrawplot to set range after updating plot
 				myPlot->SetXYRange(true);
 				boxScan->show();
 				boxHistogram->hide();
 			}
 		}
 	}

 	//originally1d && not angle plot
 	else{
 		boxScan->setToolTip("");
 		boxScan->setEnabled(true);
 		/*if(mode0 || mode1)
 			boxScan->setChecked(true);*/

 		//2d enabled with boxscan
 		if(boxScan->isChecked()){

 			//2d for 1d detctors and histogram dont go
 			if(radioHistogram->isChecked()){
 				radioDataGraph->setChecked(true);
 				//  To remind the updateplot in qdrawplot to set range after updating plot
 				myPlot->SetXYRange(true);
 				boxScan->show();
 				boxHistogram->hide();
 			}

 			//read every frame
 			disconnect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
 			disconnect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
 			comboFrequency->setCurrentIndex(1);
 			spinNthFrame->setValue(1);
 			SetFrequency();
 			connect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
 			connect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));

 			//enabling options
 			radioFileIndex->setEnabled(mode0||mode1);
 			if(mode0 && mode1){
 				radioLevel0->setEnabled(false);
 				radioLevel1->setEnabled(false);
 			}else{
 				radioLevel0->setEnabled(mode0);
 				radioLevel1->setEnabled(mode1);
 			}
 			//default is allframes if checked button is disabled
 			if(!btnGroupScan->checkedButton()->isEnabled())
 				radioAllFrames->setChecked(true);
 		}
 	}

 	//histogram
 	if(radioHistogram->isChecked()){
 		if(radioHistIntensity->isChecked()){
 			pageHistogram->setEnabled(true);
 			pageHistogram_2->setEnabled(true);
 		}else{
 			pageHistogram->setEnabled(false);
 			pageHistogram_2->setEnabled(false);
 		}
 		stackedWidget->setCurrentIndex(stackedWidget->count()-1);
 		stackedWidget_2->setCurrentIndex(stackedWidget_2->count()-1);
 		box1D->setTitle(QString("1D Plot Options %1 - Histogram").arg(stackedWidget->currentIndex()+1));
 		box2D->setTitle(QString("2D Plot Options %1 - Histogram").arg(stackedWidget_2->currentIndex()+1));

 		if(chkSuperimpose->isChecked())	chkSuperimpose->setChecked(false);
 		if(chkPedestal->isChecked())	chkPedestal->setChecked(false);
 		if(chkPedestal_2->isChecked())	chkPedestal_2->setChecked(false);
 		if(chkAccumulate->isChecked())	chkAccumulate->setChecked(false);
 		if(chkAccumulate_2->isChecked())chkAccumulate_2->setChecked(false);
 		if(chkBinary->isChecked())		chkBinary->setChecked(false);
 		if(chkBinary_2->isChecked())	chkBinary_2->setChecked(false);
 		pagePedestal->setEnabled(false);
 		pagePedestal_2->setEnabled(false);
 		chkBinary->setEnabled(false);
 		chkBinary_2->setEnabled(false);
 		pageAccumulate->setEnabled(false);
 		pageAccumulate_2->setEnabled(false);

 		//read every frame
 		disconnect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
 		disconnect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
 		comboFrequency->setCurrentIndex(1);
 		spinNthFrame->setValue(1);
 		SetFrequency();
 		connect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
 		connect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));

 	}else{
 		pageHistogram->setEnabled(false);
 		pageHistogram_2->setEnabled(false);
 	}

 	// if it was set to read every frame
 	if (oldfreqvalue != 0 && (comboFrequency->currentIndex() != 1 || spinNthFrame->value() != oldfreqvalue)) {
 		disconnect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
 		disconnect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
 		comboFrequency->setCurrentIndex(1);
 		spinNthFrame->setValue(1);
 		SetFrequency();
 		connect(spinNthFrame,	SIGNAL(editingFinished()),			this, SLOT(SetFrequency()));
 		connect(comboFrequency, SIGNAL(currentIndexChanged(int)),	this, SLOT(SetFrequency()));
 	}

 	connect(btnGroupPlotType,SIGNAL(buttonClicked(int)),this, SLOT(SetPlot()));
 	connect(boxScan,	 	SIGNAL(toggled(bool)),	this, SLOT(EnableScanBox()));

 }


 void qTabPlot::SetScanArgument(){
 #ifdef VERYVERBOSE
 	cout << "Entering qTabPlot::SetScanArgument()" << endl;
 #endif

 	//1d
 	if(isOriginallyOneD){
 		dispXAxis->setText(defaultHistXAxisTitle);
 		dispYAxis->setText(defaultHistYAxisTitle);
 		myPlot->SetHistXAxisTitle(defaultHistXAxisTitle);
 		myPlot->SetHistYAxisTitle(defaultHistYAxisTitle);
 		Select1DPlot(true);
 	}
 	//2d
 	else{
 		dispXAxis->setText(defaultImageXAxisTitle);
 		dispYAxis->setText(defaultImageYAxisTitle);
 		dispZAxis->setText(defaultImageZAxisTitle);
 		myPlot->SetImageXAxisTitle(defaultImageXAxisTitle);
 		myPlot->SetImageYAxisTitle(defaultImageYAxisTitle);
 		myPlot->SetImageZAxisTitle(defaultImageZAxisTitle);
 		Select1DPlot(false);
 	}

 	//histogram default  - set before setscanargument
 	int min = spinHistFrom->value();
 	int max = spinHistTo->value();
 	double size = spinHistSize->value();
 	int histArg = qDefs::Intensity;
 	if(radioHistogram->isChecked()){
 		if(!radioHistIntensity->isChecked()){

 			int mode = 0;
 			histArg = qDefs::histLevel0;
 			if(radioHistLevel1->isChecked()){
 				mode = 1;
 				histArg = qDefs::histLevel1;
 			}

 			int numSteps = myDet->getScanSteps(mode);
 			double *values = NULL;
 			min = 0;max = 1;size = 1;

 			if(numSteps > 0){
 				values = new double[numSteps];
 				myDet->getScanSteps(mode,values);
 				min = values[0];
 				max = values[numSteps - 1];
 				size = (max - min)/(numSteps - 1);
 			}
 		}

 	}

 	//cout <<"min:"<<min<<" max:"<<max<<" size:"<<size<<endl;
 	myPlot->SetHistogram(radioHistogram->isChecked(),histArg,min,max,size);

 	if(radioHistogram->isChecked()){
 		if(radioHistIntensity->isChecked())
 			dispXAxis->setText("Intensity");
 		else if (radioHistLevel0->isChecked())
 			dispXAxis->setText("Level 0");
 		else
 			dispXAxis->setText("Level 1");
 		dispYAxis->setText("Frequency");
 		myPlot->SetHistXAxisTitle("Intensity");
 		myPlot->SetHistYAxisTitle("Frequency");
 		Select1DPlot(true);
 	}

 	//angles (1D)
 	int ang;
 	if(myDet->getAngularConversion(ang)){
 		dispXAxis->setText("Angles");
 		myPlot->SetHistXAxisTitle("Angles");
 		Select1DPlot(true);
 	}

 	//1d with scan
 	if(boxScan->isChecked()){
 		myPlot->SetScanArgument(btnGroupScan->checkedId()+1);

 		switch(btnGroupScan->checkedId()){
 		case 0://level0
 			dispYAxis->setText("Scan Level 0");
 			myPlot->SetImageYAxisTitle("Scan Level 0");
 			break;
 		case 1://level1
 			dispYAxis->setText("Scan Level 1");
 			myPlot->SetImageYAxisTitle("Scan Level 1");
 			break;
 			break;
 		case 2://file index
 			dispYAxis->setText("Frame Index");
 			myPlot->SetImageYAxisTitle("Frame Index");
 			break;
 		case 3://all frames
 			dispYAxis->setText("All Frames");
 			myPlot->SetImageYAxisTitle("All Frames");
 			break;
 		}
 		Select1DPlot(false);
 	}else
 		myPlot->SetScanArgument(qDefs::None);

 	//update the from and to labels to be enabled
 	SetBinary();

 	qDefs::checkErrorMessage(myDet,"qTabPlot::SetScanArgument");

 }





void qTabPlot::SetHistogramOptions() {
    if (radioHistIntensity->isChecked()) {
        pageHistogram->setEnabled(true);
        pageHistogram_2->setEnabled(true);
    } else {
        pageHistogram->setEnabled(false);
        pageHistogram_2->setEnabled(false);
    }
}



void qTabPlot::Refresh() {



#ifdef VERBOSE
    cout << endl
         << "**Updating Plot Tab" << endl;
#endif
    if (!myPlot->isRunning()) {
        if (!radioNoPlot->isChecked())
            boxFrequency->setEnabled(true);
        SetFrequency();
        
        if (chkGapPixels->isEnabled()) {
            GetGapPixels();
        }

    } else {
        boxFrequency->setEnabled(false);
        disconnect(boxScan, SIGNAL(toggled(bool)), this, SLOT(EnableScanBox()));
        boxScan->setEnabled(false);
        pageHistogram->setEnabled(false);
        pageHistogram_2->setEnabled(false);
        if (radioHistogram->isChecked())
            radioDataGraph->setEnabled(false);
        else
            radioHistogram->setEnabled(false);
    }
#ifdef VERBOSE
    cout << "**Updated Plot Tab" << endl
         << endl;
#endif
}
