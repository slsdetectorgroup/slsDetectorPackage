#include "qDrawPlot.h"

#include "SlsQt1DPlot.h"
#include "SlsQt2DPlotLayout.h"
#include "detectorData.h"
#include "qCloneWidget.h"

/*
#include <QFileDialog>
#include <QFont>
#include <QImage>
#include <QPainter>
//#include "qwt_double_interval.h"
#include "qwt_series_data.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
*/

qDrawPlot::qDrawPlot(QWidget *parent, multiSlsDetector *detector)
    : QWidget(parent), myDet(detector) {
    SetupWidgetWindow();
}

qDrawPlot::~qDrawPlot() {

    DetachHists();
    for (QVector<SlsQtH1D *>::iterator h = hists1d.begin();
         h != hists1d.end(); ++h)
        delete *h;
    hists1d.clear();
    if (datax1d)
        delete [] datax1d;
    for (auto &it : datay1d)
        delete [] it;
    if (plot1d)
        delete plot1d;
    
    if (data2d)
        delete [] data2d;
    if (plot2d)
        delete plot2d;

    if (gainImage)
        delete [] gainImage;
    if (gainplot2d)
        delete gainplot2d;

    if (pedestalVals)
        delete [] pedestalVals;
    if (tempPedestalVals)
        delete [] tempPedestalVals;   

    StartOrStopThread(0);

    for (auto &it : cloneWidgets) {
        delete it;
    }

    if (lblFrameIndexTitle1d)
        delete lblFrameIndexTitle1d;
    if (boxPlot)
        delete boxPlot;     
    if (layout)
        delete layout;     
    if (plotLayout)
        delete plotLayout;    
    if (marker)
        delete marker;     
    if (noMarker)
        delete noMarker;     
    if (widgetStatistics)
        delete widgetStatistics;     
    if (lblMinDisp)
        delete lblMinDisp;     
    if (lblMaxDisp)
        delete lblMaxDisp;    
    if (lblSumDisp)
        delete lblSumDisp;          
}

void qDrawPlot::SetupWidgetWindow() {

    detType = myDet->getDetectorTypeAsEnum();
    pthread_mutex_init(&lastImageCompleteMutex, NULL);

    // frame index 1d
    lblFrameIndexTitle1d = new QLabel("");
    lblFrameIndexTitle1d->setFixedHeight(10);

    // marker
    marker = new QwtSymbol();
    marker->setStyle(QwtSymbol::Cross);
    marker->setSize(5, 5);
    noMarker = new QwtSymbol();

    // save
    try {
        std::string temp = myDet->getFilePath();
        fileSavePath = QString(temp.c_str());
        temp = myDet->getFileName();
        fileSaveName = QString(temp.c_str());
    } const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get file path or file name.", e.what(), "qDrawPlot::SetupWidgetWindow");
        fileSavePath = "/tmp";
        fileSaveName = "Image";
	}   

    SetupStatistics();   
    SetupPlots();
    SetDataCallBack(true);

    // other call backs
    myDet->registerAcquisitionFinishedCallback(
        &(GetAcquisitionFinishedCallBack), this);
    myDet->registerMeasurementFinishedCallback(
        &(GetMeasurementFinishedCallBack), this);
    myDet->registerProgressCallback(&(GetProgressCallBack), this);

    Initialization();
}

void qDrawPlot::Initialization() {
    connect(this, SIGNAL(UpdatePlotSignal()), this, SLOT(UpdatePlot()));
}

void qDrawPlot::SetupStatistics() {
    widgetStatistics = new QWidget(this);
    widgetStatistics->setFixedHeight(15);
    QHBoxLayout *hl1 = new QHBoxLayout;
    hl1->setSpacing(0);
    hl1->setContentsMargins(0, 0, 0, 0);
    QLabel *lblMin = new QLabel("Min:  ");
    lblMin->setFixedWidth(40);
    lblMin->setAlignment(Qt::AlignRight);
    QLabel *lblMax = new QLabel("Max:  ");
    lblMax->setFixedWidth(40);
    lblMax->setAlignment(Qt::AlignRight);
    QLabel *lblSum = new QLabel("Sum:  ");
    lblSum->setFixedWidth(40);
    lblSum->setAlignment(Qt::AlignRight);
    lblMinDisp = new QLabel("-");
    lblMinDisp->setAlignment(Qt::AlignLeft);
    lblMaxDisp = new QLabel("-");
    lblMaxDisp->setAlignment(Qt::AlignLeft);
    lblSumDisp = new QLabel("-");
    lblSumDisp->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    lblSumDisp->setAlignment(Qt::AlignLeft);
    hl1->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed));
    hl1->addWidget(lblMin);
    hl1->addWidget(lblMinDisp);
    hl1->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed));
    hl1->addWidget(lblMax);
    hl1->addWidget(lblMaxDisp);
    hl1->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed));
    hl1->addWidget(lblSum);
    hl1->addWidget(lblSumDisp);
    hl1->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed));
    widgetStatistics->setLayout(hl1);
    layout->addWidget(widgetStatistics, 2, 0);
    widgetStatistics->hide();
}

void qDrawPlot::SetupPlots() {
    // default image size
    nPixelsX = myDet->getTotalNumberOfChannelsInclGapPixels(slsDetectorDefs::X);
    nPixelsY = myDet->getTotalNumberOfChannelsInclGapPixels(slsDetectorDefs::Y);
    if (detType == slsDetectorDefs::MOENCH) {
        npixelsy_jctb = (myDet->setTimer(slsDetectorDefs::SAMPLES, -1) * 2) /
                        25; // for moench 03
        nPixelsX = npixelsx_jctb;
        nPixelsY = npixelsy_jctb;
    }
    FILE_LOG(logINFO) << "nPixelsX:" << nPixelsX;
    FILE_LOG(logINFO) << "nPixelsY:" << nPixelsY;
    startPixel = -0.5;
    endPixel = nPixelsY - 0.5;

    // plot layout
    layout = new QGridLayout;
    this->setLayout(layout);
    setFont(QFont("Sans Serif", 9));
    boxPlot = new QGroupBox("");
    layout->addWidget(boxPlot, 1, 0);
    boxPlot->setAlignment(Qt::AlignHCenter);
    boxPlot->setFont(QFont("Sans Serif", 11, QFont::Normal));
    boxPlot->setTitle("Sample Plot");
    boxPlot->setFlat(true);
    boxPlot->setContentsMargins(0, 15, 0, 0);

    // setup 1d plot
    plot1d = new SlsQt1DPlot(boxPlot);
    plot1d->setFont(QFont("Sans Serif", 9, QFont::Normal));
    plot1d->SetXTitle(xTitle1d.toAscii().constData());
    plot1d->SetYTitle(yTitle1d.toAscii().constData());
    plot1d->hide();
    if (title1d.size()) {
        title1d.clear();
    }
    title1d.push_back("");
    // setup data
    if (datax1d)
        delete[] datax1d;
    datax1d = new double[nPixelsX];
    for (auto &it : datay1d) {
        delete[] it;
    }
    if (datay1d.size()) {
        datay1d.clear();
    }
    datay1d.push_back(new double[nPixelsX]);
    // default display data
    for (unsigned int px = 0; px < nPixelsX; ++px) {
        datax1d[px] = px;
        datay1d[0][px] = 0;
    }
    // add a hist
    DetachHists();
    SlsQtH1D *h = new SlsQtH1D("", nPixelsX, datax1d, datay1d[0]);
    h->SetLineColor(0);
    SetStyle(h);
    hists1d.append(h);

    // setup 2d plot
    plot2d = new SlsQt2DPlotLayout(boxPlot);
    // default display data
    data2d = new double[nPixelsY * nPixelsX];
    for (unsigned int px = 0; px < nPixelsX; ++px)
        for (unsigned int py = 0; py < nPixelsY; ++py)
            data2d[py * nPixelsX + px] =
                sqrt(pow(0 + 1, 2) * pow(double(px) - nPixelsX / 2, 2) /
                         pow(nPixelsX / 2, 2) / pow(1 + 1, 2) +
                     pow(double(py) - nPixelsY / 2, 2) / pow(nPixelsY / 2, 2)) /
                sqrt(2);
    plot2d->setFont(QFont("Sans Serif", 9, QFont::Normal));
    plot2d->GetPlot()->SetData(nPixelsX, -0.5, nPixelsX - 0.5, nPixelsY,
                               startPixel, endPixel, data2d);
    plot2d->setTitle(title2d.c_str());
    plot2d->SetXTitle(xTitle2d);
    plot2d->SetYTitle(yTitle2d);
    plot2d->SetZTitle(zTitle2d);
    plot2d->setAlignment(Qt::AlignLeft);

    // gainplot
    gainplot2d = new SlsQt2DPlotLayout(boxPlot);
    gainImage = new double[nPixelsY * nPixelsX];
    for (unsigned int px = 0; px < nPixelsX; ++px)
        for (unsigned int py = 0; py < nPixelsY; ++py)
            gainImage[py * nPixelsX + px] =
                sqrt(pow(0 + 1, 2) * pow(double(px) - nPixelsX / 2, 2) /
                         pow(nPixelsX / 2, 2) / pow(1 + 1, 2) +
                     pow(double(py) - nPixelsY / 2, 2) / pow(nPixelsY / 2, 2)) /
                sqrt(2);
    gainplot2d->setFont(QFont("Sans Serif", 9, QFont::Normal));
    gainplot2d->GetPlot()->SetData(nPixelsX, -0.5, nPixelsX - 0.5, nPixelsY,
                                   startPixel, endPixel, gainImage);
    gainplot2d->setTitle(title2d.c_str());
    gainplot2d->setAlignment(Qt::AlignLeft);
    gainplot2d->GetPlot()->enableAxis(0, false);
    gainplot2d->GetPlot()->enableAxis(1, false);
    gainplot2d->GetPlot()->enableAxis(2, false);
    gainplot2d->hide();

    // layout of plots
    plotLayout = new QGridLayout(boxPlot);
    plotLayout->setContentsMargins(0, 0, 0, 0);
    plotLayout->addWidget(plot1d, 0, 0, 4, 4);
    plotLayout->addWidget(plot2d, 0, 0, 4, 4);
    plotLayout->addWidget(gainplot2d, 0, 4, 1, 1);
}

bool qDrawPlot::isRunning() { 
    return isRunning; 
}

int qDrawPlot::GetProgress() { 
    return progress; 
}

int64_t qDrawPlot::GetCurrentFrameIndex() { 
    return currentFrame; 
}

int64_t qDrawPlot::GetCurrentMeasurementIndex() { 
    return currentMeasurement; 
}

void qDrawPlot::Select1dPlot(bool enable) { 
        LockLastImageArray();
        if (enable) {
        // DetachHists(); it clears the last measurement
        plot1d->SetXTitle(xTitle1d.toAscii().constData());
        plot1d->SetYTitle(yTitle1d.toAscii().constData());
        plot1d->show();
        plot2d->hide();
        boxPlot->setFlat(false);
        is1d = true;
        layout->addWidget(lblFrameIndexTitle1d, 0, 0);
        plotLayout->setContentsMargins(10, 10, 10, 10);
    } else {
        plot2d->SetXTitle(xTitle2d);
        plot2d->SetYTitle(yTitle2d);
        plot2d->SetZTitle(zTitle2d);
        plot1d->hide();
        plot2d->show();
        boxPlot->setFlat(true);
        is1d = false;
        lblFrameIndexTitle1d->setText("");
        layout->removeWidget(lblFrameIndexTitle1d);
        plotLayout->setContentsMargins(0, 0, 0, 0);
    }
    UnlockLastImageArray();
}

void qDrawPlot::SetPlotTitlePrefix(QString title) { 
    LockLastImageArray();
    FILE_LOG(logINFO) << "Setting Title to " << title.toAscii().constData();
    plotTitle_prefix = title; 
    UnlockLastImageArray();
}

void qDrawPlot::SetXAxisTitle(QString title) {
    LockLastImageArray();
    FILE_LOG(logINFO) << "Setting X Axis Title to " << title.toAscii().constData();
    if (is1d) {
        xTitle1d = title;
    } else {
        xTitle2d = title;
    }
    UnlockLastImageArray();
}

void qDrawPlot::SetYAxisTitle(QString title) {
    LockLastImageArray();
    FILE_LOG(logINFO) << "Setting Y Axis Title to " << title.toAscii().constData();
    if (is1d) {
        yTitle1d = title;
    } else {
        yTitle2d = title;
    }
    UnlockLastImageArray();
}

void qDrawPlot::SetZAxisTitle(QString title) { 
    LockLastImageArray();
    FILE_LOG(logINFO) << "Setting Z Axis Title to " << title.toAscii().constData();
    zTitle2d = title; 
    UnlockLastImageArray(); 
}

void qDrawPlot::DisableZoom(bool disable) {
    LockLastImageArray();
    FILE_LOG(logINFO) << "Setting Disable zoom to " << std::boolalpha << disable << std::noboolalpha;
    if (is1d)
        plot1d->DisableZoom(disable);
    else
        plot2d->GetPlot()->DisableZoom(disable);
    UnlockLastImageArray();     
}

void qDrawPlot::SetXYRangeChanged() { 
    LockLastImageArray();
    FILE_LOG(logINFO) << "XY Range has changed";
    XYRangeChanged = true; 
    UnlockLastImageArray();     
}

void qDrawPlot::SetXYRangeValues(double val, qDefs::range xy) {
    LockLastImageArray();
    FILE_LOG(logDEBUG) << "Setting XY Range [" << static_cast<int>(xy) << "] to " << val;
    XYRange[xy] = val;
    UnlockLastImageArray();     
}

void qDrawPlot::IsXYRangeValues(bool changed, qDefs::range xy) {
    LockLastImageArray();
    FILE_LOG(logDEBUG) << "Setting XY Range Change [" << static_cast<int>(xy) << "] to " << std::boolalpha << changed << std::noboolalpha;;
    isXYRange[xy] = changed;
    UnlockLastImageArray();     
}

double qDrawPlot::GetXMinimum() {
    if (is1d)
        return plot1d->GetXMinimum();
    else
        return plot2d->GetPlot()->GetXMinimum();
}

double qDrawPlot::GetXMaximum() {
    if (is1d)
        return plot1d->GetXMaximum();
    else
        return plot2d->GetPlot()->GetXMaximum();
}

double qDrawPlot::GetYMinimum() {
    if (is1d)
        return plot1d->GetYMinimum();
    else
        return plot2d->GetPlot()->GetYMinimum();
}

double qDrawPlot::GetYMaximum() {
    if (is1d)
        return plot1d->GetYMaximum();
    else
        return plot2d->GetPlot()->GetYMaximum();
}

void qDrawPlot::SetZRange(bool isZmin, bool isZmax, double zmin, double zmax) {
    LockLastImageArray();
    FILE_LOG(logINFO) << std::boolalpha << "Setting Z Range to "
                     "Zmin (" << isZmin << ", " << zmin << ") "
                     "Zmax (" << isZmax << ", " << zmax << ")";
    plot2d->SetZRange(isZmin, isZmax, zmin, zmax);
    UnlockLastImageArray(); 
}

void qDrawPlot::SetDataCallBack(bool enable) {
    LockLastImageArray();
    FILE_LOG(logINFO) << "Setting data call back to " << std::boolalpha << enable << std::noboolalpha;
    if (is1d)
    if (enable) {
        myDet->registerDataCallback(&(GetDataCallBack), this); 
    } else {
        myDet->registerDataCallback(nullptr, this);
    }
    UnlockLastImageArray();     
}

void qDrawPlot::SetBinary(bool enable, int from, int to) {
    LockLastImageArray();
    FILE_LOG(logINFO) << (enable ? "Enabling" : "Disabling") << " Binary output from " << from << " to " << to;
    binary = enable;
    binaryFrom = from;
    binaryTo = to;
    UnlockLastImageArray();    
}

void qDrawPlot::SetPersistency(int val) {
    LockLastImageArray();
    FILE_LOG(logINFO) << "Setting Persistency to " << val;
    for (int i = datay1d.size(); i <= val; ++i)
        datay1d.push_back(new double[nPixelsX]);
    persistency = val;
    UnlockLastImageArray();    
}

void qDrawPlot::SetLines(bool enable) { 
    LockLastImageArray();
    FILE_LOG(logINFO) << "Setting Lines to " << std::boolalpha << enable << std::noboolalpha;
    isLines = enable; 
    UnlockLastImageArray();    
}

void qDrawPlot::SetMarkers(bool enable) { 
    LockLastImageArray();
    FILE_LOG(logINFO) << "Setting Markers to " << std::boolalpha << enable << std::noboolalpha;
    isMarkers = enable;
    UnlockLastImageArray();    
}

void qDrawPlot::Set1dLogY(bool enable) {
    LockLastImageArray();
    FILE_LOG(logINFO) << "Setting Log Y to " << std::boolalpha << enable << std::noboolalpha;
    plot1d->SetLogY(enable);
    UnlockLastImageArray();   
}

void qDrawPlot::SetInterpolate(bool enable) {
    LockLastImageArray();
    FILE_LOG(logINFO) << "Setting Interpolate to " << std::boolalpha << enable << std::noboolalpha;
    plot2d->SetInterpolate();
    UnlockLastImageArray();   
}

void qDrawPlot::SetContour(bool enable) {
    LockLastImageArray();
    FILE_LOG(logINFO) << "Setting Countour to " << std::boolalpha << enable << std::noboolalpha;
    plot2d->SetContour();
    UnlockLastImageArray();   
}

void qDrawPlot::SetLogz(bool enable) {
    LockLastImageArray();
    FILE_LOG(logINFO) << "Setting Log Z to " << std::boolalpha << enable << std::noboolalpha;
    plot2d->SetLogz();
    UnlockLastImageArray();   
}

void qDrawPlot::SetPedestal(bool enable) {
    LockLastImageArray();
    FILE_LOG(logINFO) << (enable ? "Enabling" : "Disabling") << " Pedestal";
    if (enable) {
        isPedestal = true;
        if (pedestalVals == nullptr)
            RecalculatePedestal();
    } else {
        isPedestal = false;
    }
    UnlockLastImageArray();
}

void qDrawPlot::RecalculatePedestal() {
    LockLastImageArray();
    FILE_LOG(logDEBUG) << "Recalculating Pedestal";

    startPedestalCal = true;
    pedestalCount = 0;

    if (pedestalVals != nullptr)
        delete [] pedestalVals;
    int nPixels = nPixelsX * nPixelsY;
    pedestalVals = new double[nPixels];
    std::fill(pedestalVals, pedestalVals + nPixels, 0);

    if (tempPedestalVals != nullptr)
        delete [] tempPedestalVals;
    tempPedestalVals = new double[nPixels];
    std::fill(tempPedestalVals, tempPedestalVals + nPixels, 0);
    UnlockLastImageArray();
}

void qDrawPlot::SetAccumulate(bool enable) {
    LockLastImageArray();
    FILE_LOG(logINFO) << (enable ? "Enabling" : "Disabling") << " Accumulation";
    accumulate = enable;
    UnlockLastImageArray();
}

void qDrawPlot::ResetAccumulate() {
    LockLastImageArray();
    FILE_LOG(logDEBUG) << "Resetting Accumulation";   
    resetAccumulate = true;
    UnlockLastImageArray();
}

void qDrawPlot::DisplayStatistics(bool enable) {
    LockLastImageArray();
    FILE_LOG(logINFO) << (enable ? "Enabling" : "Disabling") << " Statistics Display";
    if (!enable)
       widgetStatistics->hide();
    // shown when calculated
    displayStatistics = enable;
    lblMinDisp->setText("-");
    lblMaxDisp->setText("-");
    lblSumDisp->setText("-");
    UnlockLastImageArray();
}

void qDrawPlot::EnableGainPlot(bool enable) {
    LockLastImageArray();
    FILE_LOG(logINFO) << (enable ? "Enabling" : "Disabling") << " Gain Plot";
    gainDataEnable = enable;
    UnlockLastImageArray();
}

void qDrawPlot::SetSaveFileName(QString val) {
    FILE_LOG(logDEBUG) << "Setting Clone/Save File Name to " << val.toAscii().constData();
    fileSaveName = val;
}

void qDrawPlot::ClonePlot() {
    LockLastImageArray();
    if (is1d) {
        FILE_LOG(logINDO) << "Cloning 1D Image";
        qCloneWidget *q = new qCloneWidget(
            this, cloneWidgets.size(), boxPlot->title(), xTitle1d, yTitle1d, "", 1,
            fileSavePath, fileSaveName, lastImageNumber, displayStatistics, lblMinDisp->text(),
            lblMaxDisp->text(), lblSumDisp->text());
        cloneWidgets.push_back(q);
        cloneWidgets[i]->SetCloneHists(nHists, nPixelsX, datax1d, datay1d,
                                       title1d, isLines, isMarkers);
    } else  {
        FILE_LOG(logINDO) << "Cloning 2D Image";
        qCloneWidget *q = new qCloneWidget(
            this, cloneWidgets.size(), boxPlot->title(), xTitle2d, yTitle2d, zTitle2d, 2, 
            fileSavePath, fileSaveName, lastImageNumber, displayStatistics, lblMinDisp->text(),
            lblMaxDisp->text(), lblSumDisp->text());
        cloneWidgets.push_back(q);
        cloneWidgets[i]->SetCloneHists2D(nPixelsX, -0.5, nPixelsX - 0.5,
                                         nPixelsY, startPixel, endPixel,
                                         data2d);
    }

    if (isXYRange[qDefs::XMIN] || isXYRange[qDefs::XMAX] ||isXYRange[qDefs::YMIN] ||isXYRange[qDefs::YMAX]) {
        cloneWidgets[i]->SetRange(isXYRange, XYRange);
    }
    UnlockLastImageArray();
    cloneWidgets[i]->show();

    // to remember which all clone widgets were closed
    connect(cloneWidgets[i], SIGNAL(CloneClosedSignal(int)), this, SLOT(CloneCloseEvent(int)));
}

void qDrawPlot::CloseClones() {
    FILE_LOG(logDEBUG) << "Closing all Clones"; 
    for (auto &it : cloneWidgets) {
        it->close();
    }
}

void qDrawPlot::CloneCloseEvent(int id) {
    FILE_LOG(logDEBUG) << "Closing Clone " << id; 
    cloneWidgets.erase(cloneWidgets.begin() + id;
}

void qDrawPlot::SaveClones() {
    FILE_LOG(logINFO) << "Saving all Clones"; 
    char errID[200];
    std::string errMessage = "The Snapshots with ID's: ";
    bool success = true;
    for (int i = 0; i < cloneWidgets.size(); ++i) {
        if (cloneWidgets[i]->SavePlotAutomatic()) {
            success = false;
            sprintf(errID, "%d", i);
            errMessage.append(std::string(errID) + std::string(", "));
        }
    }
    if (success) {
        qDefs::Message(
            qDefs::INFORMATION,
            "The Snapshots have all been saved successfully in .png.", "Dock");
    } else {
        qDefs::Message(qDefs::WARNING,
                       errMessage + std::string("were not saved."),
                       "qDrawPlot::SaveClones");
        FILE_LOG(logWARNING) << errMessage << "were not saved";
    }
}

void qDrawPlot::SavePlot() {
    // render image
    QImage savedImage(size().width(), size().height(), QImage::Format_RGB32);
    QPainter painter(&savedImage);
    render(&painter);

    QString fName = fileSavePath + Qstring('/') + fileSaveName + Qstring('_') + lastImageNumber +  Qstring('_') + QString(NowTime().c_str()) + QString(".png");
        fName = QFileDialog::getSaveFileName(
        0, tr("Save Image"), fName,
        tr("PNG Files (*.png);;XPM Files(*.xpm);;JPEG Files(*.jpg)"), 0,
        QFileDialog::ShowDirsOnly);

    if (!fName.isEmpty()) {
        if (savedImage.save(fName)) {
            qDefs::Message(qDefs::INFORMATION, "The Image has been successfully saved", "qDrawPlot::SavePlot");
            fileSavePath = fName.section('/', 0, -2);
        } else {
            qDefs::Message(qDefs::WARNING, "Attempt to save image failed.\n Formats: .png, .jpg, .xpm.", "qDrawPlot::SavePlot");
        }
    }
}

int qDrawPlot::LockLastImageArray() {
    return pthread_mutex_lock(&lastImageCompleteMutex);
}

int qDrawPlot::UnlockLastImageArray() {
    return pthread_mutex_unlock(&lastImageCompleteMutex);
}

void qDrawPlot::SetStyle(SlsQtH1D *h) {
    h->setStyle(isLines ? QwtPlotCurve::Lines : QwtPlotCurve::Dots);
#if QWT_VERSION < 0x060000
    h->setSymbol(isMarkers ? *marker : *nomarker);
#else
    h->setSymbol(isMarkers ? marker : nomarker);
#endif
}

void qDrawPlot::GetStatistics(double &min, double &max, double &sum) {
 FILE_LOG(logDEBUG) << "Calculating Statistics";   
    double *array = data2d;
    int size = nPixelsX * nPixelsY;
    if(is1d) {
        array = datay1d[0];
        size = nPixelsX;
    }
    , int size
    for (int i = 0; i < size; ++i) {
        if (array[i] < min)
            min = array[i];
        if (array[i] > max)
            max = array[i];
        sum += array[i];
    }
}

void qDrawPlot::DetachHists() {
    for (QVector<SlsQtH1D *>::iterator h = hists1d.begin(); h != hists1d.end(); ++h) {
        (*h)->Detach(plot1d);
    }
}

void qDrawPlot::UpdateXYRange() {
    if (XYRangeChanged) {
        void* plot = plot1d;
        if (!is1d) {
            plot = plot2d->GetPlot();
        }

        if (!isXYRange[qDefs::XMIN] || !isXYRange[qDefs::XMAX]) {
            plot->EnableXAutoScaling();
        } else {
            if (!isXYRange[qDefs::XMIN])
                XYRange[qDefs::XMIN] = plot->GetXMinimum();
            if (!isXYRange[qDefs::XMAX])
                XYRange[qDefs::XMAX] = plot->GetXMaximum();
            plot->SetXMinMax(XYRange[qDefs::XMIN], XYRange[qDefs::XMAX]);
        } 

        if (!isXYRange[qDefs::YMIN] || !isXYRange[qDefs::YMAX]) {
            plot->EnableYAutoScaling();
        } else {
            if (!isXYRange[qDefs::YMIN])
                XYRange[qDefs::YMIN] = plot->GetYMinimum();
            if (!isXYRange[qDefs::YMAX])
                XYRange[qDefs::YMAX] = plot->GetYMaximum();
            plot->SetYMinMax(XYRange[qDefs::YMIN], XYRange[qDefs::YMAX]);
        } 
        XYRangeChanged = false;
        plot->Update();
    }
}

void qDrawPlot::StartAcquisition() {
    FILE_LOG(logDEBUG) << "Starting Acquisition in qDrawPlot";
    currentMeasurement = 0;
    currentFrame = 0;
    lastImageNumber = 0;
    
}














    void qDrawPlot::toDoublePixelData(double *dest, char *source, int size,
                                      int databytes, int dr, double *gaindest) {
        int ichan = 0;
        int ibyte = 0;
        int halfbyte = 0;
        char cbyte = '\0';

        switch (dr) {

        case 4:
            for (ibyte = 0; ibyte < databytes; ++ibyte) {
                cbyte = source[ibyte];
                for (halfbyte = 1; halfbyte >= 0; --halfbyte) {
                    dest[ichan] = (cbyte >> (halfbyte * 4)) & 0xf;
                    ++ichan;
                }
            }
            break;

        case 8:
            for (ichan = 0; ichan < databytes; ++ichan) {
                dest[ichan] = *((u_int8_t *)source);
                ++source;
            }
            break;

        case 16:
            if (detType == slsDetectorDefs::JUNGFRAU ||
                detType == slsDetectorDefs::MOENCH) {

                // show gain plot
                if (gaindest != NULL) {
                    for (ichan = 0; ichan < size; ++ichan) {
                        if ((*((u_int16_t *)source)) == 0xFFFF) {
                            gaindest[ichan] = 0xFFFF;
                            dest[ichan] = 0xFFFF;
                        } else {
                            gaindest[ichan] =
                                (((*((u_int16_t *)source)) & 0xC000) >> 14);
                            dest[ichan] = ((*((u_int16_t *)source)) & 0x3FFF);
                        }
                        source += 2;
                    }
                }

                // only data plot
                else {
                    for (ichan = 0; ichan < size; ++ichan) {
                        /*if (  (*((u_int16_t*)source)) == 0xFFFF  )
                                                    dest[ichan] = 0xFFFF;
                                            else*/
                        dest[ichan] = ((*((u_int16_t *)source)) & 0x3FFF);
                        source += 2;
                    }
                }
                break;
            }

            // other detectors
            for (ichan = 0; ichan < size; ++ichan) {
                dest[ichan] = *((u_int16_t *)source);
                source += 2;
            }
            break;

        default:
            for (ichan = 0; ichan < size; ++ichan) {
                dest[ichan] = *((u_int32_t *)source);
                source += 4;
            }
            break;
        }
    }





void qDrawPlot::StartStopDaqToggle(bool stop_if_running) {
#ifdef VERYVERBOSE
    std::cout << "Entering StartStopDaqToggle(" << stop_if_running << ")\n";
#endif

    if (isRunning) { 

        StartOrStopThread(0);
        isRunning = !isRunning;
    } else if (!stop_if_running) { 

        currentMeasurement = 0;
        currentFrame = 0;
        lastImageNumber = 0;



        if (!StartOrStopThread(0)) {
            std::cout << "Resetting image number" << '\n';
            lastImageNumber = 0;
        }
        StartOrStopThread(1);

        isRunning = !isRunning;
    }

}



bool qDrawPlot::StartOrStopThread(bool start) {
#ifdef VERYVERBOSE
    std::cout << "StartOrStopThread:" << start << '\n';
#endif
    static bool firstTime = true;
    static bool gui_acquisition_thread_running = 0;
    static pthread_t gui_acquisition_thread;
    static pthread_mutex_t gui_acquisition_start_stop_mutex =
        PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&gui_acquisition_start_stop_mutex);
    // stop part, before start or restart
    if (gui_acquisition_thread_running) {
        std::cout << "Stopping current acquisition thread ...." << '\n';
        gui_acquisition_thread_running = 0;
    }

    // start part
    if (start) {
        progress = 0;
        // sets up the measurement parameters
        SetupMeasurement();

        // refixing all the zooming
        plot2d->GetPlot()->SetXMinMax(-0.5, nPixelsX + 0.5);
        plot2d->GetPlot()->SetYMinMax(startPixel, endPixel);
        plot2d->GetPlot()->SetZoom(-0.5, startPixel, nPixelsX,
                                   endPixel - startPixel);
        if (boxPlot->title() == "Sample Plot")
            plot2d->GetPlot()->UnZoom();
        else
            plot2d->GetPlot()->UnZoom(false);
        /*XYRangeChanged = true;*/
        boxPlot->setTitle("Old_Plot.raw");

        cprintf(BLUE, "Starting new acquisition thread ....\n");
        // Start acquiring data from server
        if (!firstTime)
            pthread_join(gui_acquisition_thread,
                         NULL); // wait until he's finished, ie. exits
        pthread_create(&gui_acquisition_thread, NULL, DataStartAcquireThread,
                       (void *)this);
        // This is set here and later reset to zero when all the plotting is
        // done This is manually done instead of keeping track of thread because
        // this thread returns immediately after executing the acquire command
        gui_acquisition_thread_running = 1;
#ifdef VERYVERBOSE
        std::cout << "Started acquiring thread" << '\n';
#endif
    }
    pthread_mutex_unlock(&gui_acquisition_start_stop_mutex);
    return gui_acquisition_thread_running;
}

void qDrawPlot::SetScanArgument(int scanArg) {
#ifdef VERYVERBOSE
    std::cout << "SetScanArgument function:" << scanArg
              << " running:" << isRunning << '\n';
#endif
    scanArgument = scanArg;

    LockLastImageArray();

    if (is1d)
        DetachHists();


    maxPixelsY = 0;
    minPixelsY = 0;
    nPixelsX = myDet->getTotalNumberOfChannelsInclGapPixels(slsDetectorDefs::X);
    nPixelsY = myDet->getTotalNumberOfChannelsInclGapPixels(slsDetectorDefs::Y);
    if (detType == slsDetectorDefs::MOENCH) {
        npixelsy_jctb = (myDet->setTimer(slsDetectorDefs::SAMPLES, -1) * 2) /
                        25; // for moench 03
        nPixelsX = npixelsx_jctb;
        nPixelsY = npixelsy_jctb;
    }

    if (minPixelsY > maxPixelsY) {
        double temp = minPixelsY;
        minPixelsY = maxPixelsY;
        maxPixelsY = temp;
        backwardScanPlot = true;
    } else
        backwardScanPlot = false;

    // 1d
    if (datax1d)
        delete[] datax1d;
    datax1d = new double[nPixelsX];

    for (auto &it : datay1d) {
        delete[] it;
    }
    if (datay1d.size()) {
        datay1d.clear();
        title1d.clear();
    }
    datay1d.push_back(new double[nPixelsX]);
    title1d.push_back("");

    // 2d
    if (data2d)
        delete[] data2d;
    data2d = new double[nPixelsY * nPixelsX];
    if (gainImage)
        delete[] gainImage;
    gainImage = new double[nPixelsY * nPixelsX];

    // initializing 1d x axis
    for (unsigned int px = 0; px < nPixelsX; ++px)
        datax1d[px] = px; /*+10;*/

    // initializing 2d array

    memset(data2d, 0, nPixelsY * nPixelsX * sizeof(double));
    memset(gainImage, 0, nPixelsY * nPixelsX * sizeof(double));


    UnlockLastImageArray();

}

void qDrawPlot::SetupMeasurement() {
#ifdef VERYVERBOSE
    std::cout << "SetupMeasurement function:" << isRunning << '\n';
#endif
    LockLastImageArray();
#ifdef VERYVERBOSE
    std::cout << "locklastimagearray\n";
#endif
    // Defaults
    if (!isRunning)
    plotRequired = 0;
    currentFrame = 0;

    // if(!is1d)
    if (!isRunning)
        lastImageNumber = 0; /**Just now */
    // initializing 2d array
    memset(data2d, 0, nPixelsY * nPixelsX * sizeof(double));
    memset(gainImage, 0, nPixelsY * nPixelsX * sizeof(double));

    // 1d with no scan
    if ((!originally2D) && (scanArgument == qDefs::None)) {
#ifdef VERYVERBOSE
        std::cout << "1D\n";
#endif
        if (!isRunning) {
            maxPixelsY = 100;
            minPixelsY = 0;
            startPixel = -0.5;
            endPixel = nPixelsY - 0.5;
        }
    } else {
#ifdef VERYVERBOSE
        std::cout << "2D\n";
#endif
        // 2d with no scan
        if ((originally2D) && (scanArgument == qDefs::None)) {
            maxPixelsY = nPixelsY - 1;
            minPixelsY = 0;
        }

        // cannot divide by 0
        if (nPixelsY == 1) {
            pixelWidth = 0;
            startPixel = minPixelsY - 0.5;
            endPixel = minPixelsY + 0.5;
        } else {
            pixelWidth = (maxPixelsY - minPixelsY) / (nPixelsY - 1);
            startPixel = minPixelsY - (pixelWidth / 2);
            endPixel = maxPixelsY + (pixelWidth / 2);
        }

        if (histogram) {
            startPixel = histFrom - (histSize / 2);
            endPixel = histTo + (histSize / 2);
        }
    }

    /*
        std::cout<<"nPixelsX:"<<nPixelsX<<endl;
        std::cout<<"nPixelsY:"<<nPixelsY<<endl;
        std::cout<<"minPixelsY:"<<minPixelsY<<endl;
        std::cout<<"maxPixelsY:"<<maxPixelsY<<endl;
        std::cout<<"startPixel:"<<startPixel<<endl;
        std::cout<<"endPixel:"<<endPixel<<endl<<endl;
*/
    UnlockLastImageArray();

#ifdef VERYVERBOSE
    std::cout << "locklastimagearray\n";
#endif
}

void *qDrawPlot::DataStartAcquireThread(void *this_pointer) {
    // stream data from receiver to the gui
    if (((qDrawPlot *)this_pointer)->myDet->setReceiverOnline() == slsDetectorDefs::ONLINE_FLAG) {

        // if receiver data up streaming not on, switch it on
        if (((qDrawPlot *)this_pointer)
                ->myDet->enableDataStreamingFromReceiver() != 1) {
            // switch on receiver
            if (((qDrawPlot *)this_pointer)
                    ->myDet->enableDataStreamingFromReceiver(1) != 1) {
                qDefs::checkErrorMessage(((qDrawPlot *)this_pointer)->myDet,
                                         "qDrawPlot::DataStartAcquireThread");
                return this_pointer;
            }
        }
    }

    if (((qDrawPlot *)this_pointer)->myDet->getAcquiringFlag() == true) {
        ((qDrawPlot *)this_pointer)->myDet->setAcquiringFlag(false);
    }
    ((qDrawPlot *)this_pointer)->myDet->acquire();

    return this_pointer;
}

int qDrawPlot::GetDataCallBack(detectorData *data, int fIndex, int subIndex,
                               void *this_pointer) {
    ((qDrawPlot *)this_pointer)->GetData(data, fIndex, subIndex);
    return 0;
}

int qDrawPlot::GetData(detectorData *data, int fIndex, int subIndex) {
#ifdef VERYVERBOSE
    std::cout << "******Entering GetDatafunction********\n";
    std::cout << "fIndex " << fIndex << '\n';
    std::cout << "subIndex " << subIndex << '\n';
    std::cout << "fname " << data->fileSaveName << '\n';
    std::cout << "npoints " << data->npoints << '\n';
    std::cout << "npy " << data->npy << '\n';
    std::cout << "progress " << data->progressIndex << '\n';
    if (data->values != NULL)
        std::cout << "values " << data->values << '\n';
    std::cout << "databytes " << data->databytes << '\n';
    std::cout << "dynamicRange " << data->dynamicRange << '\n';
    std::cout << "fileIndex " << data->fileIndex << '\n';
#endif

        // set progress
        progress = (int)data->progressIndex;
        // currentFrameIndex = fileIOStatic::getIndicesFromFileName(std::string(data->fileSaveName),currentFileIndex);
        currentFileIndex = data->fileIndex;

#ifdef VERYVERBOSE
        std::cout << "progress:" << progress << '\n';
#endif
        // secondary title necessary to differentiate between frames when not
        // saving data
        char temp_title[2000];
        // findex is the frame index given by receiver, cannot be derived from
        // file name
        if (fIndex != -1) {
            currentFrameIndex = fIndex;
            sprintf(temp_title, "#%d", fIndex);
            if ((detType == slsDetectorDefs::EIGER) && (subIndex != -1))
                sprintf(temp_title, "#%d  %d", fIndex, subIndex);
        } else {
            
                sprintf(temp_title, "#%d", currentFrame);
        }
        if (subIndex != -1)
            sprintf(temp_title, "#%d  %d", fIndex, subIndex);

        // Plot Disabled
        if (!plotEnable)
            return 0;

        // convert char* to double
        if (data->values == NULL) {
            data->values = new double[nPixelsX * nPixelsY];
            if (gainDataEnable) {
                data->dgainvalues = new double[nPixelsX * nPixelsY];
                toDoublePixelData(data->values, data->cvalues,
                                  nPixelsX * nPixelsY, data->databytes,
                                  data->dynamicRange, data->dgainvalues);
            } else
                toDoublePixelData(data->values, data->cvalues,
                                  nPixelsX * nPixelsY, data->databytes,
                                  data->dynamicRange);
        }

        // normal measurement or 1d scans
        LockLastImageArray();//fixme: lock from beginning
        /*if(!pthread_mutex_trylock(&(lastImageCompleteMutex))){*/
        // set title
        plotTitle = QString(plotTitle_prefix) +
                    QString(data->fileSaveName).section('/', -1);
        // only if you got the lock, do u need to remember lastimagenumber to
        // plot
        lastImageNumber = currentFrame + 1;
        // cout<<"got last imagenumber:"<<lastImageNumber<<endl;
        // 1d
        if (is1d) {
            // Titles
            title1d[0] = temp_title;

            // Persistency
            if (currentPersistency < persistency)
                currentPersistency++;
            else
                currentPersistency = persistency;
            nHists = currentPersistency + 1;

            // copy data
            for (int i = currentPersistency; i > 0; i--)
                memcpy(datay1d[i], datay1d[i - 1], nPixelsX * sizeof(double));

            // recalculating pedestal
            if (startPedestalCal) {
                // start adding frames to get to the pedestal value
                if (pedestalCount < NUM_PEDESTAL_FRAMES) {
                    for (unsigned int px = 0; px < nPixelsX; ++px)
                        tempPedestalVals[px] += data->values[px];
                    memcpy(datay1d[0], data->values, nPixelsX * sizeof(double));
                    pedestalCount++;
                }
                // calculate the pedestal value
                if (pedestalCount == NUM_PEDESTAL_FRAMES) {
                    cout << "Pedestal Calculated" << '\n';
                    for (unsigned int px = 0; px < nPixelsX; ++px)
                        tempPedestalVals[px] = tempPedestalVals[px] /
                                                (double)NUM_PEDESTAL_FRAMES;
                    memcpy(pedestalVals, tempPedestalVals,
                            nPixelsX * sizeof(double));
                    startPedestalCal = 0;
                }
            }

            // normal data
            if (((!isPedestal) & (!accumulate) & (!binary)) ||
                (resetAccumulate)) {
                memcpy(datay1d[0], data->values, nPixelsX * sizeof(double));
                resetAccumulate = false;
            }
            // pedestal or accumulate
            else {
                double temp; // cannot overwrite cuz of accumulate
                for (unsigned int px = 0; px < (nPixelsX * nPixelsY);
                        ++px) {
                    temp = data->values[px];
                    if (isPedestal)
                        temp = data->values[px] - (pedestalVals[px]);
                    if (binary) {
                        if ((temp >= binaryFrom) && (temp <= binaryTo))
                            temp = 1;
                        else
                            temp = 0;
                    }
                    if (accumulate)
                        temp += datay1d[0][px];
                    // after all processing
                    datay1d[0][px] = temp;
                }
            }
        }
        // 2d
        else {
            // Titles
            title2d = temp_title;

            // jungfrau mask gain
            if (data->dgainvalues != NULL) {
                memcpy(gainImage, data->dgainvalues,
                       nPixelsX * nPixelsY * sizeof(double));
                gainDataExtracted = true;
            } else
                gainDataExtracted = false;

            // recalculating pedestal
            if (startPedestalCal) {
                // start adding frames to get to the pedestal value
                if (pedestalCount < NUM_PEDESTAL_FRAMES) {
                    for (unsigned int px = 0; px < (nPixelsX * nPixelsY); ++px)
                        tempPedestalVals[px] += data->values[px];
                    memcpy(data2d, data->values,
                           nPixelsX * nPixelsY * sizeof(double));
                    pedestalCount++;
                }
                // calculate the pedestal value
                if (pedestalCount == NUM_PEDESTAL_FRAMES) {
                    std::cout << "Pedestal Calculated" << '\n';
                    for (unsigned int px = 0; px < (nPixelsX * nPixelsY); ++px)
                        tempPedestalVals[px] =
                            tempPedestalVals[px] / (double)NUM_PEDESTAL_FRAMES;
                    memcpy(pedestalVals, tempPedestalVals,
                           nPixelsX * nPixelsY * sizeof(double));
                    startPedestalCal = 0;
                }
            }

            // normal data
            if (((!isPedestal) & (!accumulate) & (!binary)) ||
                (resetAccumulate)) {
                memcpy(data2d, data->values,
                       nPixelsX * nPixelsY * sizeof(double));
                resetAccumulate = false;
            }
            // pedestal or accumulate or binary
            else {
                double temp;
                for (unsigned int px = 0; px < (nPixelsX * nPixelsY); ++px) {
                    temp = data->values[px];
                    if (isPedestal)
                        temp = data->values[px] - (pedestalVals[px]);
                    if (binary) {
                        if ((temp >= binaryFrom) && (temp <= binaryTo))
                            temp = 1;
                        else
                            temp = 0;
                    }
                    if (accumulate)
                        temp += data2d[px];
                    // after all processing
                    data2d[px] = temp;
                }
            }
        }
        /*	pthread_mutex_unlock(&(lastImageCompleteMutex));
                }*/
        plotRequired = true;
        //UnlockLastImageArray(); // fixme: do not unlock, let it plot.

#ifdef VERYVERBOSE
        cprintf(BLUE, "currentframe:%d \tcurrentframeindex:%d\n", currentFrame,
                currentFrameIndex);
#endif
        currentFrame++;
        emit UpdatePlotSignal();
    

#ifdef VERYVERBOSE
    std::cout << "Exiting GetData function" << '\n';
#endif
    return 0;
}

int qDrawPlot::GetAcquisitionFinishedCallBack(double currentProgress,
                                              int detectorStatus,
                                              void *this_pointer) {
    ((qDrawPlot *)this_pointer)
        ->AcquisitionFinished(currentProgress, detectorStatus);
#ifdef VERYVERBOSE
    std::cout << "acquisition finished callback worked ok\n";
#endif
    return 0;
}

int qDrawPlot::AcquisitionFinished(double currentProgress, int detectorStatus) {
#ifdef VERBOSE
    std::cout << "\nEntering Acquisition Finished with status ";
#endif
    QString status = QString(slsDetectorDefs::runStatusType(
                                 slsDetectorDefs::runStatus(detectorStatus))
                                 .c_str());
#ifdef VERBOSE
    std::cout << status.toAscii().constData() << " and progress "
              << currentProgress << '\n';
#endif
    // error or stopped
    if (detectorStatus == slsDetectorDefs::ERROR) {
#ifdef VERBOSE
        std::cout << "Error in Acquisition\n\n";
#endif
        qDefs::Message(qDefs::WARNING,
                std::string("<nobr>The acquisiton has ended abruptly. "
                            "Current Detector Status: ") +
                    status.toAscii().constData() +
                    std::string(".</nobr>"),
                "qDrawPlot::ShowAcquisitionErrorMessage");
    }
#ifdef VERBOSE
    // all measurements are over
    else if (currentProgress == 100) {
        std::cout << "Acquisition Finished\n";
    }
#endif
    StartStopDaqToggle(true);
    // this lets the measurement tab know its over, and to enable tabs
    emit UpdatingPlotFinished();

    return 0;
}

int qDrawPlot::GetProgressCallBack(double currentProgress, void *this_pointer) {
    ((qDrawPlot *)this_pointer)->progress = currentProgress;
    return 0;
}


int qDrawPlot::GetMeasurementFinishedCallBack(int currentMeasurementIndex,
                                              int fileIndex,
                                              void *this_pointer) {
    ((qDrawPlot *)this_pointer)
        ->MeasurementFinished(currentMeasurementIndex, fileIndex);
    return 0;
}

int qDrawPlot::MeasurementFinished(int currentMeasurementIndex, int fileIndex) {
#ifdef VERBOSE
    std::cout << "Entering Measurement Finished with currentMeasurement "
              << currentMeasurementIndex << " and fileIndex " << fileIndex
              << '\n';
#endif

    // to make sure it plots the last frame
    while (plotRequired) {
        usleep(2000);
    }

    currentMeasurement = currentMeasurementIndex + 1;
    currentFileIndex = fileIndex;
#ifdef VERBOSE
    std::cout << "currentMeasurement:" << currentMeasurement << '\n';
#endif
    emit SetCurrentMeasurementSignal(currentMeasurement);
    SetupMeasurement();
    /*if((myDet->setReceiverOnline()==slsDetectorDefs::ONLINE_FLAG) &&
       (myDet->getFramesCaughtByReceiver() == 0))
                boxPlot->setTitle("OLD_plot.raw");*/
    return 0;
}

void qDrawPlot::UpdatePlot() {
#ifdef VERYVERBOSE
    std::cout << "Entering UpdatePlot function\n";
#endif
    if (!plotEnable || !plotRequired) {
        UnlockLastImageArray();
        return;
    }
        // so that it doesnt plot every single thing
#ifdef VERYVERBOSE
        cprintf(GREEN, "Updating Plot\n");
#endif
        // so as to not plot it again and to let measurment finished know its
        // done plotting it 1-d plot stuff
        if (is1d) {
#ifdef VERYVERBOSE
            std::cout << "Last Image Number:" << lastImageNumber << '\n';
#endif
            if (nPixelsX) {
                DetachHists();
                plot1d->SetXTitle(xTitle1d.toAscii().constData());
                plot1d->SetYTitle(yTitle1d.toAscii().constData());

                for (int hist_num = 0; hist_num < (int)nHists; ++hist_num) {
                    SlsQtH1D *h;
                    if (hist_num + 1 > hists1d.size()) {
                        hists1d.append(h = new SlsQtH1D("", nPixelsX, datax1d, datay1d[0]));
                        h->SetLineColor(hist_num);
                    } else {
                        h = hists1d.at(hist_num);
                        h->SetData(nPixelsX, datax1d, datay1d[hist_num]);
                    }
                    SetStyle(h);
                    lblFrameIndexTitle1d->setText(title1d[0].c_str());
                    h->Attach(plot1d);
                }
            }
        } // 2-d plot stuff
        else {
                    plot2d->GetPlot()->SetData(nPixelsX, -0.5, nPixelsX - 0.5,
                                               nPixelsY, startPixel, endPixel,data2d);
                    plot2d->setTitle(title2d.c_str());
                    plot2d->SetXTitle(xTitle2d);
                    plot2d->SetYTitle(yTitle2d);
                    plot2d->SetZTitle(zTitle2d);
                    // recalculate if z is set
                    plot2d->KeepZRangeIfSet();
                if (gainDataExtracted) {
                    gainplot2d->GetPlot()->SetData(nPixelsX, -0.5, nPixelsX - 0.5, nPixelsY,
                            startPixel, endPixel, gainImage);
                    gainplot2d->setTitle(title2d.c_str());
                    gainplot2d->setFixedWidth(plot2d->width() / 4);
                    gainplot2d->setFixedHeight(plot2d->height() / 4);
                    gainplot2d->show();
                } else
                    gainplot2d->hide();
        }
        UpdateXYRange();
        // Display Statistics
        if (displayStatistics) {
            double min = 0, max = 0, sum = 0;
            GetStatistics(min, max, sum);
            lblMinDisp->setText(QString("%1").arg(min));
            lblMaxDisp->setText(QString("%1").arg(max));
            lblSumDisp->setText(QString("%1").arg(sum));
        }

        // set plot title
        boxPlot->setTitle(plotTitle);
        // to notify the measurement finished when its done
        plotRequired = false;
        UnlockLastImageArray();
    

#ifdef VERYVERBOSE
    std::cout << "Exiting UpdatePlot function\n";
#endif
}