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
    QWidget(parent), myDet(detector), myPlot(plot), is1d(false) {
    setupUi(this);
    SetupWidgetWindow();
    FILE_LOG(logDEBUG) << "Plot ready";
}

qTabPlot::~qTabPlot() {
    if (btnGroupPlotType)
        delete btnGroupPlotType;
}

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
    switch(myDet->getDetectorTypeAsEnum()) {
        case slsDetectorDefs::GOTTHARD:
            is1d = true;
            break;
        case slsDetectorDefs::EIGER:
            chkGapPixels->setEnabled(true);
            break;
        case slsDetectorDefs::JUNGFRAU:
        case slsDetectorDefs::MOENCH:
            chkGainPlot->setEnabled(true);  
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
    connect(btnGroupPlotType, SIGNAL(buttonClicked(int)), this, SLOT(SetPlot()));

    // Plotting frequency box
    connect(comboFrequency, SIGNAL(currentIndexChanged(int)), this, SLOT(SetStreamingFrequency()));
    connect(comboTimeGapUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetStreamingFrequency()));
    connect(spinTimeGap, SIGNAL(editingFinished()), this, SLOT(SetStreamingFrequency()));
    connect(spinNthFrame, SIGNAL(editingFinished()), this, SLOT(SetStreamingFrequency()));

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
    connect(chk1DLog, SIGNAL(toggled(bool)), myPlot, SLOT(Set1dLogY(bool)));
    connect(chkStatistics, SIGNAL(toggled(bool)), myPlot, SLOT(DisplayStatistics(bool)));

    // 2D Plot box
    connect(chkInterpolate, SIGNAL(toggled(bool)), myPlot, SLOT(SetInterpolate(bool)));
    connect(chkContour, SIGNAL(toggled(bool)), myPlot, SLOT(SetContour(bool)));
    connect(chkLogz, SIGNAL(toggled(bool)), myPlot, SLOT(SetLogz(bool)));
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
        connect(chkGainPlot, SIGNAL(toggled(bool)), myPlot, SLOT(EnableGainPlot(bool)));
    // gap pixels
    if (chkGapPixels->isEnabled())
        connect(chkGapPixels, SIGNAL(toggled(bool)), this, SLOT(SetGapPixels(bool)));

    // Save
    connect(btnSave, SIGNAL(clicked()), myPlot, SLOT(SavePlot()));

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
}

void qTabPlot::Select1DPlot(bool enable) {
    FILE_LOG(logDEBUG) << "Selecting " << (enable ? "1" : "2") << "D Plot";
    is1d = enable;
    box1D->setEnabled(enable);
    box2D->setEnabled(!enable);
    chkZAxis->setEnabled(!enable);
    dispZAxis->setEnabled(!enable);
    chkZMin->setEnabled(!enable);
    chkZMax->setEnabled(!enable);
    dispZMin->setEnabled(!enable);
    dispZMax->setEnabled(!enable);
    myPlot->Select1dPlot(enable);
    SetTitles();
    SetXYRange();
    if (!is1d) {
        SetZRange();
    }
}

void qTabPlot::SetPlot() {
    bool plotEnable = false;
    if (radioNoPlot->isChecked()) {
        FILE_LOG(logINFO) << "Setting Plot Type: No Plot";
    } else if (radioDataGraph->isChecked()) {
        FILE_LOG(logINFO) << "Setting Plot Type: Datagraph";
        plotEnable = true;
    }
    boxFrequency->setEnabled(plotEnable);
    box1D->setEnabled(plotEnable);
    box2D->setEnabled(plotEnable);
    boxSave->setEnabled(plotEnable);
    boxSnapshot->setEnabled(plotEnable);
    boxPlotAxis->setEnabled(plotEnable);

    if (plotEnable) {
        SetTitles();
        SetXYRange();
        if (!is1d) {
            SetZRange();
        }
    }
    
    myPlot->SetDataCallBack(plotEnable);
}

void qTabPlot::Set1DPlotOptionsRight() {
    FILE_LOG(logDEBUG) << "1D Options Right";
    int i = stackedWidget1D->currentIndex();
    if (i == (stackedWidget1D->count() - 1))
        stackedWidget1D->setCurrentIndex(0);
    else
        stackedWidget1D->setCurrentIndex(i + 1);
    box1D->setTitle(QString("1D Plot Options %1").arg(stackedWidget1D->currentIndex() + 1));
}

void qTabPlot::Set1DPlotOptionsLeft() {
    FILE_LOG(logDEBUG) << "1D Options Left";
    int i = stackedWidget1D->currentIndex();
    if (i == 0)
        stackedWidget1D->setCurrentIndex(stackedWidget1D->count() - 1);
    else
        stackedWidget1D->setCurrentIndex(i - 1);
    box1D->setTitle(QString("1D Plot Options %1").arg(stackedWidget1D->currentIndex() + 1));
}

void qTabPlot::Set2DPlotOptionsRight() {
    FILE_LOG(logDEBUG) << "2D Options Right";
    int i = stackedWidget2D->currentIndex();
    if (i == (stackedWidget2D->count() - 1))
        stackedWidget2D->setCurrentIndex(0);
    else
        stackedWidget2D->setCurrentIndex(i + 1);
    box2D->setTitle(QString("2D Plot Options %1").arg(stackedWidget2D->currentIndex() + 1));
}

void qTabPlot::Set2DPlotOptionsLeft() {
    FILE_LOG(logDEBUG) << "2D Options Left";
    int i = stackedWidget2D->currentIndex();
    if (i == 0)
        stackedWidget2D->setCurrentIndex(stackedWidget2D->count() - 1);
    else
        stackedWidget2D->setCurrentIndex(i - 1);
    box2D->setTitle(QString("2D Plot Options %1").arg(stackedWidget2D->currentIndex() + 1));
}

void qTabPlot::EnablePersistency(bool enable) {
    FILE_LOG(logINFO) << "Superimpose " << (enable ? "enabled" : "disabled");
    lblPersistency->setEnabled(enable);
    spinPersistency->setEnabled(enable);
    if (enable)
        myPlot->SetPersistency(spinPersistency->value());
    else
        myPlot->SetPersistency(0);
}

void qTabPlot::SetBinary() {
    bool binary1D = chkBinary->isChecked();
    bool binary2D = chkBinary_2->isChecked(); 
    if (is1d) {
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
    } CATCH_DISPLAY ("Could not get gap pixels enable.", "qTabPlot::GetGapPixels")

    connect(chkGapPixels, SIGNAL(toggled(bool)), this, SLOT(SetGapPixels(bool)));
}

void qTabPlot::SetGapPixels(bool enable) {
	FILE_LOG(logINFO) << "Setting Gap Pixels Enable to " << enable;

	try {
        myDet->enableGapPixels(enable);
    } CATCH_HANDLE("Could not set gap pixels enable.", "qTabPlot::SetGapPixels", this, &qTabPlot::GetGapPixels)
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
        dispXAxis->setText(is1d ? defaultHistXAxisTitle : defaultImageXAxisTitle);
        myPlot->SetXAxisTitle(is1d ? defaultHistXAxisTitle : defaultImageXAxisTitle);
    } else {
        myPlot->SetXAxisTitle(dispXAxis->text());
    } 
    // y
    if (!chkYAxis->isChecked() || dispYAxis->text().isEmpty()) {
        dispYAxis->setText(is1d ? defaultHistYAxisTitle : defaultImageYAxisTitle);
        myPlot->SetYAxisTitle(is1d ? defaultHistYAxisTitle : defaultImageYAxisTitle);
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
    FILE_LOG(logDEBUG) << "Set XY Range";
    disconnect(dispXMin, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    disconnect(dispXMax, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    disconnect(dispYMin, SIGNAL(editingFinished()), this, SLOT(SetYRange()));
    disconnect(dispYMax, SIGNAL(editingFinished()), this, SLOT(SetYRange()));

    bool disablezoom = false;

    // xmin
    // if unchecked, empty or invalid (set to false so it takes the min/max of plot)
    if (!chkXMin->isChecked() || dispXMin->text().isEmpty()) {
        myPlot->IsXYRangeValues(false, qDefs::XMIN);
    } else if (dispXMin->text().toDouble() < myPlot->GetXMinimum()) {
        qDefs::Message(qDefs::WARNING, "XMin Outside Plot Range", "qTabPlot::SetXRange");
        FILE_LOG(logWARNING) << "Xmin entered " << dispXMin->text().toDouble() << " outside xmin range " << myPlot->GetXMinimum();
        dispXMin->setText("");
        myPlot->IsXYRangeValues(false, qDefs::XMIN);
    } else {
        myPlot->SetXYRangeValues(dispXMin->text().toDouble(), qDefs::XMIN);
        myPlot->IsXYRangeValues(true, qDefs::XMIN);
        disablezoom = true;
    } 

    //xmax
    if (!chkXMax->isChecked() || dispXMax->text().isEmpty()) {
        myPlot->IsXYRangeValues(false, qDefs::XMAX);
    } else if (dispXMax->text().toDouble() > myPlot->GetXMaximum()) {
        qDefs::Message(qDefs::WARNING, "XMax Outside Plot Range", "qTabPlot::SetXYRange");
        FILE_LOG(logWARNING) << "Xmax entered " << dispXMax->text().toDouble() << " outside xmax range " << myPlot->GetXMaximum();
        dispXMax->setText("");
        myPlot->IsXYRangeValues(false, qDefs::XMAX);
    } else {
        myPlot->SetXYRangeValues(dispXMax->text().toDouble(), qDefs::XMAX);
        myPlot->IsXYRangeValues(true, qDefs::XMAX);
        disablezoom = true;
    } 
    
    // ymin
    if (!chkYMin->isChecked() || dispYMin->text().isEmpty()) {
        myPlot->IsXYRangeValues(false, qDefs::YMIN);
    } else if (dispYMin->text().toDouble() < myPlot->GetYMinimum()) {
        qDefs::Message(qDefs::WARNING, "YMin Outside Plot Range", "qTabPlot::SetXYRange");
                FILE_LOG(logWARNING) << "Ymin entered " << dispYMin->text().toDouble() << " outside ymin range " << myPlot->GetYMinimum();
        dispYMin->setText("");
        myPlot->IsXYRangeValues(false, qDefs::YMIN);
    } else {
        myPlot->SetXYRangeValues(dispYMin->text().toDouble(), qDefs::YMIN);
        myPlot->IsXYRangeValues(true, qDefs::YMIN);
        disablezoom = true;
    } 

    //ymax
    if (!chkYMax->isChecked() || dispYMax->text().isEmpty()) {
        myPlot->IsXYRangeValues(false, qDefs::YMAX);
    } else if (dispYMax->text().toDouble() > myPlot->GetYMaximum()) {
        qDefs::Message(qDefs::WARNING, "YMax Outside Plot Range", "qTabPlot::SetXYRange");
                FILE_LOG(logWARNING) << "Ymax entered " << dispYMax->text().toDouble() << " outside ymax range " << myPlot->GetYMaximum();
        dispYMax->setText("");
        myPlot->IsXYRangeValues(false, qDefs::YMAX);
    } else {
        myPlot->SetXYRangeValues(dispYMax->text().toDouble(), qDefs::YMAX);
        myPlot->IsXYRangeValues(true, qDefs::YMAX);
        disablezoom = true;
    } 

    connect(dispXMin, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    connect(dispXMax, SIGNAL(editingFinished()), this, SLOT(SetXRange()));
    connect(dispYMin, SIGNAL(editingFinished()), this, SLOT(SetYRange()));
    connect(dispYMax, SIGNAL(editingFinished()), this, SLOT(SetYRange()));

    // to update plot with range
    myPlot->SetXYRangeChanged();
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
    ranges[qDefs::XMIN] = myPlot->GetXMinimum();
    ranges[qDefs::XMAX] = myPlot->GetXMaximum();
    ranges[qDefs::YMIN] = myPlot->GetYMinimum();
    ranges[qDefs::YMAX] = myPlot->GetYMaximum();
    double idealAspectratio = (ranges[qDefs::XMAX] - ranges[qDefs::XMIN]) / (ranges[qDefs::YMAX] - ranges[qDefs::YMIN]);
    FILE_LOG(logDEBUG) << "Ideal Aspect ratio: " << idealAspectratio << " for x(" << ranges[qDefs::XMIN] << " - " << ranges[qDefs::XMAX] << "), y(" << ranges[qDefs::YMIN] << " - " << ranges[qDefs::YMAX] << ")";
  
    // calculate current aspect ratio
    ranges[qDefs::XMIN] = dispXMin->text().toDouble();
    ranges[qDefs::XMAX] = dispXMax->text().toDouble();
    ranges[qDefs::YMIN] = dispYMin->text().toDouble();
    ranges[qDefs::YMAX] = dispYMax->text().toDouble();
    double currentAspectRatio = (ranges[qDefs::XMAX] - ranges[qDefs::XMIN]) / (ranges[qDefs::YMAX] - ranges[qDefs::YMIN]);
    FILE_LOG(logDEBUG) << "Current Aspect ratio: " << currentAspectRatio << " for x(" << ranges[qDefs::XMIN] << " - " << ranges[qDefs::XMAX] << "), y(" << ranges[qDefs::YMIN] << " - " << ranges[qDefs::YMAX] << ")";
 
    if (currentAspectRatio != idealAspectratio) {
        // dimension: 1(x changed: y adjusted), 0(y changed: x adjusted), -1(aspect ratio clicked: larger one adjusted)
        if (dimension == -1) {
            dimension = ((ranges[qDefs::XMAX] - ranges[qDefs::XMIN]) > (ranges[qDefs::YMAX] - ranges[qDefs::YMIN])) 
            ? static_cast<int>(slsDetectorDefs::X) : static_cast<int>(slsDetectorDefs::Y);
        }

        // calculate new value to maintain aspect ratio
        // adjust x
        double newval = 0;
        if (dimension == static_cast<int>(slsDetectorDefs::X)) {
            newval = idealAspectratio * (ranges[qDefs::YMAX] - ranges[qDefs::YMIN]) + ranges[qDefs::XMIN];
            if (newval <= myPlot->GetXMaximum()) {
                dispXMax->setText(QString::number(newval));
                FILE_LOG(logDEBUG) << "New XMax: " << newval;
            } else {
                newval = ranges[qDefs::XMAX] - (idealAspectratio * (ranges[qDefs::YMAX] - ranges[qDefs::YMIN]));
                dispXMin->setText(QString::number(newval));
                FILE_LOG(logDEBUG) << "New XMin: " << newval;
            }
        }
        // adjust y
        else  {
            newval = ((ranges[qDefs::XMAX] - ranges[qDefs::XMIN]) / idealAspectratio) + ranges[qDefs::YMIN];
            if (newval <= myPlot->GetYMaximum()) {
                dispYMax->setText(QString::number(newval));
                FILE_LOG(logDEBUG) << "New YMax: " << newval;
            } else {
                newval = ranges[qDefs::YMAX] - ((ranges[qDefs::XMAX] - ranges[qDefs::XMIN]) / idealAspectratio);
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
    myPlot->SetXYRangeValues(dispXMin->text().toDouble(), qDefs::XMIN);
    myPlot->SetXYRangeValues(dispXMax->text().toDouble(), qDefs::XMAX);
    myPlot->SetXYRangeValues(dispYMin->text().toDouble(), qDefs::YMIN);
    myPlot->SetXYRangeValues(dispYMax->text().toDouble(), qDefs::YMAX);

    myPlot->IsXYRangeValues(true, qDefs::XMIN);
    myPlot->IsXYRangeValues(true, qDefs::XMAX);
    myPlot->IsXYRangeValues(true, qDefs::YMIN);
    myPlot->IsXYRangeValues(true, qDefs::YMAX);

    // to update plot with range
    myPlot->SetXYRangeChanged();
    myPlot->DisableZoom(true);
    emit DisableZoomSignal(true);
}

void qTabPlot::SetZRange() {
    bool isZmin = chkZMin->isChecked();
    bool isZmax = chkZMax->isChecked();
    double zmin = 0, zmax = 1;
    if (!dispZMin->text().isEmpty()) {
        zmin = dispZMin->text().toDouble();
    }
    if (!dispZMax->text().isEmpty()) {
        zmax = dispZMax->text().toDouble();
    }  
    myPlot->SetZRange(isZmin, isZmax, zmin, zmax);
}

void qTabPlot::GetStreamingFrequency() {
	FILE_LOG(logDEBUG) << "Getting Streaming Frequency";	
    disconnect(comboFrequency, SIGNAL(currentIndexChanged(int)), this, SLOT(SetStreamingFrequency()));
    disconnect(comboTimeGapUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetStreamingFrequency()));
    disconnect(spinTimeGap, SIGNAL(editingFinished()), this, SLOT(SetStreamingFrequency()));
    disconnect(spinNthFrame, SIGNAL(editingFinished()), this, SLOT(SetStreamingFrequency()));

	try {
		int freq = myDet->setReceiverStreamingFrequency(-1);
        if (freq < 0) {
            qDefs::Message(qDefs::WARNING, "Streaming frequency is inconsistent for all detectors.", "qTabPlot::GetStreamingFrequency");
        } 
        // time interval
        else if (freq == 0) {
            comboFrequency->setCurrentIndex(0);
            stackedTimeInterval->setCurrentIndex(0);
            try {
                int timeMs = myDet->setReceiverStreamingTimer(-1);
                if (freq < 0) {
                    qDefs::Message(qDefs::WARNING, "Streaming timer is inconsistent for all detectors.", "qTabPlot::GetStreamingFrequency");
                } else {
                    double timeS = static_cast<double>(timeMs) / 1000.00;
                    auto time = qDefs::getCorrectTime(timeS);
                    spinTimeGap->setValue(time.first);
                    comboTimeGapUnit->setCurrentIndex(static_cast<int>(time.second));
                }
            } CATCH_DISPLAY ("Could not get streaming timer.", "qTabPlot::GetStreamingFrequency")
        }
        // every nth frame
        else {
            comboFrequency->setCurrentIndex(1);
            stackedTimeInterval->setCurrentIndex(1);
            spinNthFrame->setValue(freq);
        }
    } CATCH_DISPLAY ("Could not get streaming frequency.", "qTabPlot::GetStreamingFrequency")

	connect(comboFrequency, SIGNAL(currentIndexChanged(int)), this, SLOT(SetStreamingFrequency()));
    connect(comboTimeGapUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetStreamingFrequency()));
    connect(spinTimeGap, SIGNAL(editingFinished()), this, SLOT(SetStreamingFrequency()));
    connect(spinNthFrame, SIGNAL(editingFinished()), this, SLOT(SetStreamingFrequency()));

}

void qTabPlot::SetStreamingFrequency() {
    bool frequency = (comboFrequency->currentIndex() == 0) ? 0 : 1;
    auto freqVal = spinNthFrame->value();
    auto timeVal = spinTimeGap->value();
    auto timeUnit = static_cast<qDefs::timeUnit>(comboTimeGapUnit->currentIndex());
    stackedTimeInterval->setCurrentIndex(comboFrequency->currentIndex());
	try {
        if (frequency) {
            FILE_LOG(logINFO) << "Setting Streaming Frequency to " << freqVal;
            myDet->setReceiverStreamingFrequency(freqVal);
        } else {
            FILE_LOG(logINFO) << "Setting Streaming Timer to " << timeVal << " " << qDefs::getUnitString(timeUnit);
            double timeMS = qDefs::getMSTime(timeUnit, timeVal);
            myDet->setReceiverStreamingTimer(timeMS);
        }
    } CATCH_HANDLE("Could not set streaming frequency/ timer.", "qTabPlot::SetStreamingFrequency", this, &qTabPlot::GetStreamingFrequency)
}

void qTabPlot::Refresh() {
    FILE_LOG(logDEBUG) << "**Updating Plot Tab";

    if (!myPlot->GetIsRunning()) {
        boxPlotType->setEnabled(true);

        // streaming frequency
        if (!radioNoPlot->isChecked()) {
            boxFrequency->setEnabled(true);
        }
        GetStreamingFrequency();
        // gain plot, gap pixels enable
        switch(myDet->getDetectorTypeAsEnum()) {
            case slsDetectorDefs::EIGER:
                chkGapPixels->setEnabled(true);
                GetGapPixels();
                break;
            case slsDetectorDefs::JUNGFRAU:
            case slsDetectorDefs::MOENCH:
                chkGainPlot->setEnabled(true);  
                break;
            default:
                break;             
        }
    } else {
        boxPlotType->setEnabled(false);
        boxFrequency->setEnabled(false);
        chkGainPlot->setEnabled(false);
        chkGapPixels->setEnabled(false);
    }

    FILE_LOG(logDEBUG) << "**Updated Plot Tab";
}
