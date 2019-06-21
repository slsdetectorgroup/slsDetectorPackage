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

    Clear1DPlot();
    for (QVector<SlsQtH1D *>::iterator h = hists1d.begin();
         h != hists1d.end(); ++h)
        delete *h;
    hists1d.clear();
    if (x1d)
        delete [] x1d;
    for (auto &it : y1d)
        delete [] it;
    if (plot1d)
        delete plot1d;
    
    if (image2d)
        delete [] image2d;
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

    // layout
    setFont(QFont("Sans Serif", 9));
    layout = new QGridLayout;
    this->setLayout(layout);
    boxPlot = new QGroupBox("");
    layout->addWidget(boxPlot, 1, 0);
    boxPlot->setAlignment(Qt::AlignHCenter);
    boxPlot->setFont(QFont("Sans Serif", 11, QFont::Normal));
    boxPlot->setTitle("Sample Plot");

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

    nPixelsX = myDet->getTotalNumberOfChannelsInclGapPixels(slsDetectorDefs::X);
    nPixelsY = myDet->getTotalNumberOfChannelsInclGapPixels(slsDetectorDefs::Y);
    if (detType == slsDetectorDefs::MOENCH) {
        npixelsy_jctb = (myDet->setTimer(slsDetectorDefs::SAMPLES, -1) * 2) /
                        25; // for moench 03
        nPixelsX = npixelsx_jctb;
        nPixelsY = npixelsy_jctb;
    }
    std::cout << "nPixelsX:" << nPixelsX << '\n';
    std::cout << "nPixelsY:" << nPixelsY << '\n';
    startPixel = -0.5;
    endPixel = nPixelsY - 0.5;
    
    SetupPlots();
    SetDataCallBack(true);

    // other call backs
    myDet->registerAcquisitionFinishedCallback(
        &(GetAcquisitionFinishedCallBack), this);
    myDet->registerMeasurementFinishedCallback(
        &(GetMeasurementFinishedCallBack), this);
    myDet->registerProgressCallback(&(GetProgressCallBack), this);

    Initialization();
    StartStopDaqToggle();
}

void qDrawPlot::Initialization() {
    connect(this, SIGNAL(UpdatePlotSignal()), this, SLOT(UpdatePlot()));
    connect(this, SIGNAL(InterpolateSignal(bool)), plot2d, SIGNAL(InterpolateSignal(bool)));
    connect(this, SIGNAL(ContourSignal(bool)), plot2d, SIGNAL(ContourSignal(bool)));
    connect(this, SIGNAL(LogzSignal(bool)), plot2d, SIGNAL(LogzSignal(bool))));
    connect(this, SIGNAL(LogySignal(bool)), plot1d, SLOT(SetLogY(bool)));
    connect(this, SIGNAL(ResetZMinZMaxSignal(bool, bool, double, double)), plot2d, SLOT(SetZRange(bool, bool, double, double)));
    connect(this, SIGNAL(AcquisitionErrorSignal(QString)), this, SLOT(ShowAcquisitionErrorMessage(QString)));
    connect(this, SIGNAL(GainPlotSignal(bool)), this, SLOT(EnableGainPlot(bool)));
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
    plot1d = new SlsQt1DPlot(boxPlot);
    plot1d->setFont(QFont("Sans Serif", 9, QFont::Normal));
    plot1d->SetXTitle(xTitle1d.toAscii().constData());
    plot1d->SetYTitle(yTitle1d.toAscii().constData());
    plot1d->hide();

    histNBins = nPixelsX;
    nHists = 1;
    if (x1d)
        delete[] x1d;
    x1d = new double[nPixelsX];
    for (auto &it : y1d) {
        delete[] it;
    }
    if (y1d.size()) {
        y1d.clear();
        title1d.clear();
    }
    y1d.push_back(new double[nPixelsX]);
    title1d.push_back("");

    for (unsigned int px = 0; px < nPixelsX; ++px) {
        x1d[px] = px;
        y1d[0][px] = 0;
    }
    Clear1DPlot();
    plot1d->SetXTitle("X Axis");
    plot1d->SetYTitle("Y Axis");

    SlsQtH1D *h;
    hists1d.append(h = new SlsQtH1D("", histNBins, x1d, y1d[0]));
    h->SetLineColor(0);
    SetStyle(h);
    h->Attach(plot1d);
    Clear1DPlot();

    plot2d = new SlsQt2DPlotLayout(boxPlot);
    // default plot
    image2d = new double[nPixelsY * nPixelsX];
    for (unsigned int px = 0; px < nPixelsX; ++px)
        for (unsigned int py = 0; py < nPixelsY; ++py)
            image2d[py * nPixelsX + px] =
                sqrt(pow(0 + 1, 2) * pow(double(px) - nPixelsX / 2, 2) /
                         pow(nPixelsX / 2, 2) / pow(1 + 1, 2) +
                     pow(double(py) - nPixelsY / 2, 2) / pow(nPixelsY / 2, 2)) /
                sqrt(2);
    plot2d->setFont(QFont("Sans Serif", 9, QFont::Normal));
    plot2d->GetPlot()->SetData(nPixelsX, -0.5, nPixelsX - 0.5, nPixelsY,
                               startPixel, endPixel, image2d);
    plot2d->setTitle(title2d.c_str());
    plot2d->SetXTitle(xTitle2d);
    plot2d->SetYTitle(yTitle2d);
    plot2d->SetZTitle(zTitle2d);
    plot2d->setAlignment(Qt::AlignLeft);
    boxPlot->setFlat(true);
    boxPlot->setContentsMargins(0, 15, 0, 0);

    plotLayout = new QGridLayout(boxPlot);
    plotLayout->setContentsMargins(0, 0, 0, 0);
    plotLayout->addWidget(plot1d, 0, 0, 4, 4);
    plotLayout->addWidget(plot2d, 0, 0, 4, 4);

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
    plotLayout->addWidget(gainplot2d, 0, 4, 1, 1);
    gainplot2d->hide();
}


void qDrawPlot::SetClientInitiated() { 
    clientInitiated = true; 
}

bool qDrawPlot::GetClientInitiated() { 
    return clientInitiated; 
}

void qDrawPlot::StopAcquisition() { stopSignal = true; };

bool qDrawPlot::isRunning() { 
    return running; 
}

int qDrawPlot::GetProgress() { 
    return progress; 
}

int qDrawPlot::GetCurrentFrameIndex() { 
    return currentFrameIndex; 
}

void qDrawPlot::SetPlotTitlePrefix(QString title) { 
    plotTitle_prefix = title; 
}

void qDrawPlot::SetXAxisTitle(QString title) {
    if (is1d) {
        xTitle1d = title;
    } else {
        xTitle2d = title;
    }
}

void qDrawPlot::SetYAxisTitle(QString title) {
    if (is1d) {
        yTitle1d = title;
    } else {
        yTitle2d = title;
    }
}

void qDrawPlot::SetZAxisTitle(QString title) { 
    zTitle2d = title; 
}

void qDrawPlot::DisableZoom(bool disable) {
    if (is1d)
        plot1d->DisableZoom(disable);
    else
        plot2d->GetPlot()->DisableZoom(disable);
}

void qDrawPlot::SetXYRange(bool changed) { 
    XYRangeChanged = changed; 
}

void qDrawPlot::SetXYRangeValues(double val, qDefs::range xy) {
    XYRangeValues[xy] = val;
}

void qDrawPlot::IsXYRangeValues(bool changed, qDefs::range xy) {
    isXYRangeEnable[xy] = changed;
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

const char *qDrawPlot::GetTitle1d(int histIndex) {
    return (histIndex >= 0 && histIndex < title1d.size()) ? title1d[histIndex].c_str() : 0;
} 

double *qDrawPlot::GetHistYAxis(int histIndex) {
    return (histIndex >= 0 && histIndex < y1d.size()) ? y1d[histIndex] : 0;
} 

void qDrawPlot::SetDataCallBack(bool enable) {
    if (enable) {
        myDet->registerDataCallback(&(GetDataCallBack), this); 
    } else {
        myDet->registerDataCallback(nullptr, this);
    }
}

void qDrawPlot::SetBinary(bool enable, int from, int to) {
    FILE_LOG(logINFO) << (enable ? "Enabling" : "Disabling") << " Binary output from " << from << " to " << to;
    binary = enable;
    binaryFrom = from;
    binaryTo = to;
}

void qDrawPlot::SetPersistency(int val) {
    for (int i = y1d.size(); i <= val; ++i)
        y1d.push_back(new double[nPixelsX]);
    persistency = val;
}

void qDrawPlot::SetLines(bool enable) { 
    isLines = enable; 
}

void qDrawPlot::SetMarkers(bool enable) { 
    isMarkers = enable; 
}

void qDrawPlot::SetStyle(SlsQtH1D *h) {
    if (isLines)
        h->setStyle(QwtPlotCurve::isLines);
    else
        h->setStyle(QwtPlotCurve::Dots);
#if QWT_VERSION < 0x060000
    if (isMarkers)
        h->setSymbol(*marker);
    else
        h->setSymbol(*noMarker);
#else
    if (isMarkers)
        h->setSymbol(marker);
    else
        h->setSymbol(noMarker);
#endif
}

void qDrawPlot::SetPedestal(bool enable) {
    FILE_LOG(logINFO) << (enable ? "Enabling" : "Disabling") << " Pedestal";
    LockLastImageArray();
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
    FILE_LOG(logDEBUG) << "Recalculating Pedestal";
    LockLastImageArray();
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
    FILE_LOG(logINFO) << (enable ? "Enabling" : "Disabling") << " Accumulation";
    LockLastImageArray();
    accumulate = enable;
    UnlockLastImageArray();
}

void qDrawPlot::ResetAccumulate() {
    FILE_LOG(logDEBUG) << "Resetting Accumulation";   
    LockLastImageArray();
    resetAccumulate = true;
    UnlockLastImageArray();
}

void qDrawPlot::DisplayStatistics(bool enable) {
    FILE_LOG(logINFO) << (enable ? "Enabling" : "Disabling") << " Statistics Display";
    LockLastImageArray();
    if (!enable)
       widgetStatistics->hide();
    // shown when calculated
    displayStatistics = enable;
    lblMinDisp->setText("-");
    lblMaxDisp->setText("-");
    lblSumDisp->setText("-");
    UnlockLastImageArray();
}

void qDrawPlot::GetStatistics(double &min, double &max, double &sum, double *array, int size) {
 FILE_LOG(logDEBUG) << "Calculating Statistics";   
    for (int i = 0; i < size; ++i) {
        if (array[i] < min)
            min = array[i];
        if (array[i] > max)
            max = array[i];
        sum += array[i];
    }
}













void qDrawPlot::ClonePlot() {
    int i = cloneWidgets.size();

    // get file path while acquisition runnign without accessing shared memory
    std::string sFilePath;
    if (running)
        sFilePath = fileSavePath.toAscii().constData();
    else {
        sFilePath = myDet->getFilePath();
        qDefs::checkErrorMessage(myDet, "qDrawPlot::ClonePlot");
    }

    LockLastImageArray();

    // create clone & copy data
    if (is1d) {
        qCloneWidget *q = new qCloneWidget(
            this, i, boxPlot->title(), xTitle1d, yTitle1d, "", (is1d ? 1 : 2),
            sFilePath, displayStatistics, lblMinDisp->text(),
            lblMaxDisp->text(), lblSumDisp->text());
        cloneWidgets.push_back(q);
        cloneWidgets[i]->SetCloneHists((int)nHists, histNBins, x1d, y1d,
                                       title1d, isLines, isMarkers);

    } else {
        qCloneWidget *q = new qCloneWidget(
            this, i, boxPlot->title(), xTitle2d, yTitle2d, zTitle2d,
            (is1d ? 1 : 2), sFilePath, displayStatistics, lblMinDisp->text(),
            lblMaxDisp->text(), lblSumDisp->text());
        cloneWidgets.push_back(q);
        cloneWidgets[i]->SetCloneHists2D(nPixelsX, -0.5, nPixelsX - 0.5,
                                         nPixelsY, startPixel, endPixel,
                                         image2d);
    }

    // update range
    found = false;
    for (int index = 0; index < 4; ++index)
        if (isXYRangeEnable[index]) {
            found = true;
            break;
        }
    if (found)
        cloneWidgets[i]->SetRange(isXYRangeEnable, XYRangeValues);

    UnlockLastImageArray();

    cloneWidgets[i]->show();

    // to remember which all clone widgets were closed
    connect(cloneWidgets[i], SIGNAL(CloneClosedSignal(int)), this,
            SLOT(CloneCloseEvent(int)));
}

void qDrawPlot::CloseClones() {
    for (auto &it : cloneWidgets) {
        it->close();
    }

    void qDrawPlot::CloneCloseEvent(int id) {
    cloneWidgets.erase(cloneWidgets.begin() + id;
#ifdef VERBOSE
    std::cout << "Closing Clone Window id:" << id << '\n';
#endif
    }

void qDrawPlot::SaveClones() {
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
    if (success)
        qDefs::Message(
            qDefs::INFORMATION,
            "The Snapshots have all been saved successfully in .png.", "Dock");
    else
        qDefs::Message(qDefs::WARNING,
                       errMessage + std::string("were not saved."),
                       "qDrawPlot::SaveClones");
}


    void qDrawPlot::SavePlot() {
        // render image
        QImage savedImage(size().width(), size().height(),
                          QImage::Format_RGB32);
        QPainter painter(&savedImage);
        render(&painter);

        QString fName;
        if (running)
            fName = fileSavePath;
        else {
            fName = QString(myDet->getFilePath().c_str());
            qDefs::checkErrorMessage(myDet, "qDrawPlot::SavePlot");
        }

        if (boxPlot->title().contains('.')) {
            fName.append(QString('/') + boxPlot->title());
            fName.replace(".dat", ".png");
            fName.replace(".raw", ".png");
        } else
            fName.append(QString("/Image.png"));

        fName = QFileDialog::getSaveFileName(
            0, tr("Save Image"), fName,
            tr("PNG Files (*.png);;XPM Files(*.xpm);;JPEG Files(*.jpg)"), 0,
            QFileDialog::ShowDirsOnly);

        if (!fName.isEmpty()) {
            if (savedImage.save(fName))
                qDefs::Message(qDefs::INFORMATION,
                               "The Image has been successfully saved",
                               "qDrawPlot::SavePlot");
            else
                qDefs::Message(qDefs::WARNING,
                               "Attempt to save image failed.\n"
                               "Formats: .png, .jpg, .xpm.",
                               "qDrawPlot::SavePlot");
        }
    }

void qDrawPlot::Select1DPlot() { SelectPlot(1); }
void qDrawPlot::Select2DPlot() { SelectPlot(2); }

void qDrawPlot::SelectPlot(int i) { // 1 for 1D otherwise 2D
    if (i == 1) {
        // Clear1DPlot(); it clears the last measurement
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
}


    void qDrawPlot::EnableGainPlot(bool e) {
#ifdef VERBOSE
        std::cout << "Setting Gain Data enable to " << e << '\n';
#endif
        gainDataEnable = e;
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






int qDrawPlot::LockLastImageArray() {
    return pthread_mutex_lock(&lastImageCompleteMutex);
}

int qDrawPlot::UnlockLastImageArray() {
    return pthread_mutex_unlock(&lastImageCompleteMutex);
}

int qDrawPlot::StartDaqForGui() { return StartOrStopThread(1) ? 1 : 0; }

int qDrawPlot::StopDaqForGui() { return StartOrStopThread(0) ? 0 : 1; }


void qDrawPlot::StartStopDaqToggle(bool stop_if_running) {
#ifdef VERYVERBOSE
    std::cout << "Entering StartStopDaqToggle(" << stop_if_running << ")\n";
#endif
    // static bool running = 1;
    if (running) { // stopping
        StartDaq(false);
        running = !running;
    } else if (!stop_if_running) { // then start
        // Reset Current Measurement
        currentMeasurement = 0;
        emit SetCurrentMeasurementSignal(currentMeasurement);
        // in case of error message
        alreadyDisplayed = false;

        /*
                // Number of Exposures
                int numFrames =
           (isFrameEnabled)*((int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1));
                int numTriggers =
           (isTriggerEnabled)*((int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,-1));
                numFrames = ((numFrames==0)?1:numFrames);
                numTriggers = ((numTriggers==0)?1:numTriggers);
                numberofFrames = numFrames * numTriggers;
                std::cout << "\tNumber of Frames per Scan/Measurement:" <<
           numberofFrames <<'\n';
                //get #scansets for level 0 and level 1
                int numScan0 = myDet->getScanSteps(0);	numScan0 =
           ((numScan0==0)?1:numScan0); int numScan1 = myDet->getScanSteps(1);
           numScan1 = ((numScan1==0)?1:numScan1); int
           numPos=myDet->getPositions();

                number_of_exposures = numberofFrames * numScan0 * numScan1;
                if(anglePlot) number_of_exposures = numScan0 * numScan1;// *
           numPos; std::cout << "\tNumber of Exposures Per Measurement:" <<
           number_of_exposures <<'\n';
                */

        // ExposureTime
        exposureTime =
            ((double)(myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME, -1)) *
             1E-9);
        std::cout << "\tExposure Time:" << std::setprecision(10) << exposureTime
                  << '\n';
        // Acquisition Period
        acquisitionPeriod =
            ((double)(myDet->setTimer(slsDetectorDefs::FRAME_PERIOD, -1)) *
             1E-9);
        std::cout << "\tAcquisition Period:" << std::setprecision(10)
                  << acquisitionPeriod << '\n';
        std::cout << "\tFile Index:" << myDet->getFileIndex() << '\n';

        // update file path and file name
        fileSavePath = QString(myDet->getFilePath().c_str());
        fileSaveName = QString(myDet->getFileName().c_str());
        // update index
        currentFileIndex = myDet->getFileIndex();
        currentFrameIndex = 0;

        StartDaq(true);
        running = !running;

        qDefs::checkErrorMessage(myDet, "qDrawPlot::StartStopDaqToggle");
    }

    /** if this is set during client initation */
    clientInitiated = false;
}

void qDrawPlot::StartDaq(bool start) {
    if (start) {
#ifdef VERBOSE
        std::cout << "Start Daq(true) function" << '\n';
#endif
        ResetDaqForGui();
        StartDaqForGui();
    } else {
#ifdef VERBOSE
        std::cout << "Start Daq(false) function" << '\n';
#endif
        StopDaqForGui();
    }
}

int qDrawPlot::ResetDaqForGui() {
    if (!StopDaqForGui())
        return 0;
    std::cout << "Resetting image number" << '\n';
    lastImageNumber = 0;
    return 1;
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
        stopSignal = 1; // sorta useless right now
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
              << " running:" << running << '\n';
#endif
    scanArgument = scanArg;

    LockLastImageArray();

    if (is1d)
        Clear1DPlot();

    // Number of Exposures - must be calculated here to get npixelsy for
    // allframes/frameindex scans
    int numFrames = (isFrameEnabled) *
                    ((int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER, -1));
    int numTriggers =
        (isTriggerEnabled) *
        ((int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER, -1));
    int numStoragecells = 0;
    if (detType == slsDetectorDefs::JUNGFRAU)
        numStoragecells =
            (int)myDet->setTimer(slsDetectorDefs::STORAGE_CELL_NUMBER, -1);
    numFrames = ((numFrames == 0) ? 1 : numFrames);
    numTriggers = ((numTriggers == 0) ? 1 : numTriggers);
    numStoragecells = ((numStoragecells <= 0) ? 1 : numStoragecells + 1);
    numberofFrames = numFrames * numTriggers * numStoragecells;
    std::cout << "\tNumber of Frames per Scan/Measurement:" << numberofFrames
              << '\n';
    // get #scansets for level 0 and level 1
    int numScan0 = myDet->getScanSteps(0);
    numScan0 = ((numScan0 == 0) ? 1 : numScan0);
    int numScan1 = myDet->getScanSteps(1);
    numScan1 = ((numScan1 == 0) ? 1 : numScan1);
    // int numPos=myDet->getPositions();

    number_of_exposures = numberofFrames * numScan0 * numScan1;
    if (anglePlot)
        number_of_exposures = numScan0 * numScan1; // * numPos;
    std::cout << "\tNumber of Exposures Per Measurement:" << number_of_exposures
              << '\n';

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

    // cannot do this in between measurements , so update instantly
    if (scanArgument == qDefs::Level0) {
        // no need to check if numsteps=0,cuz otherwise this mode wont be set in
        // plot tab
        int numSteps = myDet->getScanSteps(0);
        double *values = new double[numSteps];
        myDet->getScanSteps(0, values);

        maxPixelsY = values[numSteps - 1];
        minPixelsY = values[0];
        nPixelsY = numSteps;
    } else if (scanArgument == qDefs::Level1) {
        // no need to check if numsteps=0,cuz otherwise this mode wont be set in
        // plot tab
        int numSteps = myDet->getScanSteps(1);
        double *values = new double[numSteps];
        myDet->getScanSteps(1, values);

        maxPixelsY = values[numSteps - 1];
        minPixelsY = values[0];
        nPixelsY = numSteps;
    } else if (scanArgument == qDefs::AllFrames)
        nPixelsY = number_of_exposures;
    else if (scanArgument == qDefs::FileIndex)
        nPixelsY = numberofFrames;

    if (minPixelsY > maxPixelsY) {
        double temp = minPixelsY;
        minPixelsY = maxPixelsY;
        maxPixelsY = temp;
        backwardScanPlot = true;
    } else
        backwardScanPlot = false;

    // 1d
    if (x1d)
        delete[] x1d;
    x1d = new double[nPixelsX];

    for (auto &it : y1d) {
        delete[] it;
    }
    if (y1d.size()) {
        y1d.clear();
        title1d.clear();
    }
    y1d.push_back(new double[nPixelsX]);
    title1d.push_back("");

    // 2d
    if (image2d)
        delete[] image2d;
    image2d = new double[nPixelsY * nPixelsX];
    if (gainImage)
        delete[] gainImage;
    gainImage = new double[nPixelsY * nPixelsX];

    // initializing 1d x axis
    for (unsigned int px = 0; px < nPixelsX; ++px)
        x1d[px] = px; /*+10;*/

    // initializing 2d array

    memset(image2d, 0, nPixelsY * nPixelsX * sizeof(double));
    memset(gainImage, 0, nPixelsY * nPixelsX * sizeof(double));
    /*for(int py=0;py<(int)nPixelsY;++py)
            for(int px=0;px<(int)nPixelsX;++px) {
                    image2d[py*nPixelsX+px] = 0;
                    gainImage[py*nPixelsX+px] = 0;
            }
     */

    // histogram
    if (histogram) {
        int iloop = 0;
        int numSteps = ((histTo - histFrom) / (histSize)) + 1;
        std::cout << "numSteps:" << numSteps << " histFrom:" << histFrom
                  << " histTo:" << histTo << " histSize:" << histSize << endl;
        histogramSamples.resize(numSteps);
        startPixel = histFrom - (histSize / 2);
        std::cout << "startpixel:" << startPixel << endl;
        endPixel = histTo + (histSize / 2);
        std::cout << "endpixel:" << endPixel << endl;
        while (startPixel < endPixel) {
            histogramSamples[iloop].interval.setInterval(
                startPixel, startPixel + histSize, QwtInterval::ExcludeMaximum);
            histogramSamples[iloop].value = 0;
            startPixel += histSize;
            iloop++;
        }

        // print values
        std::cout << "Histogram Intervals:" << '\n';
        for (int j = 0; j < histogramSamples.size(); ++j) {
            std::cout << j
                      << ":\tmin:" << histogramSamples[j].interval.minValue()
                      << ""
                         "\t\tmax:"
                      << histogramSamples[j].interval.maxValue()
                      << "\t\tvalue:" << histogramSamples[j].value << endl;
        }
    }

    UnlockLastImageArray();

    qDefs::checkErrorMessage(myDet, "qDrawPlot::SetScanArgument");
}

void qDrawPlot::SetupMeasurement() {
#ifdef VERYVERBOSE
    std::cout << "SetupMeasurement function:" << running << '\n';
#endif
    LockLastImageArray();
#ifdef VERYVERBOSE
    std::cout << "locklastimagearray\n";
#endif
    // Defaults
    if (!running)
        stopSignal = 0;
    plotRequired = 0;
    currentFrame = 0;

    // if(!is1d)
    if (!running)
        lastImageNumber = 0; /**Just now */
    // initializing 2d array
    memset(image2d, 0, nPixelsY * nPixelsX * sizeof(double));
    memset(gainImage, 0, nPixelsY * nPixelsX * sizeof(double));
    /*
        for(int py=0;py<(int)nPixelsY;++py)
                for(int px=0;px<(int)nPixelsX;++px) {
                        image2d[py*nPixelsX+px] = 0;
                        gainImage[py*nPixelsX+px] = 0;
                        }
         */
    // 1d with no scan
    if ((!originally2D) && (scanArgument == qDefs::None)) {
#ifdef VERYVERBOSE
        std::cout << "1D\n";
#endif
        if (!running) {
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

        // all frames
        else if (scanArgument == qDefs::AllFrames) {
            maxPixelsY = number_of_exposures - 1;
            minPixelsY = 0;
            if (!running)
                nPixelsY = number_of_exposures;
        } // frame index
        else if (scanArgument == qDefs::FileIndex) {
            maxPixelsY = numberofFrames - 1;
            minPixelsY = 0;
            if (!running)
                nPixelsY = numberofFrames;
        } // level0 or level1
        else {
            currentScanValue = minPixelsY;
            if (backwardScanPlot) {
                currentScanValue = maxPixelsY;
                currentScanDivLevel = nPixelsY - 1;
            }
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
    if (((qDrawPlot *)this_pointer)->myDet->setReceiverOnline() ==
        slsDetectorDefs::ONLINE_FLAG) {

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
    if (!stopSignal) {

        // set progress
        progress = (int)data->progressIndex;
        // TODO!
        // currentFrameIndex =
        // fileIOStatic::getIndicesFromFileName(std::string(data->fileSaveName),currentFileIndex);
        currentFileIndex = data->fileIndex;
        // happens if receiver sends a null and empty file name
        /*if(std::string(data->fileSaveName).empty()){
                        std::cout << "Received empty file name. Exiting function
           without updating data for plot." <<'\n'; return -1;
                }*/
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
        LockLastImageArray();
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

            // histogram
            if (histogram) {
                resetAccumulate = false;
                lastImageNumber = currentFrame + 1;

                int numValues = nPixelsX;
                if (originally2D)
                    numValues = nPixelsX * nPixelsY;

                // clean up graph
                if (histogramArgument == qDefs::Intensity) {
                    for (int j = 0; j < histogramSamples.size(); ++j) {
                        histogramSamples[j].value = 0;
                    }
                }

                int val = 0;
                for (int i = 0; i < numValues; ++i) {
                    // frequency of intensity
                    if (histogramArgument == qDefs::Intensity) {
                        // ignore outside limits
                        if ((data->values[i] < histFrom) ||
                            (data->values[i] > histTo))
                            continue;
                        // check for intervals, increment if validates
                        for (int j = 0; j < histogramSamples.size(); ++j) {
                            if (histogramSamples[j].interval.contains(
                                    data->values[i]))
                                histogramSamples[j].value += 1;
                        }
                    }
                    // get sum of data pixels
                    else
                        val += data->values[i];
                }

                if (histogramArgument != qDefs::Intensity) {
                    std::cout << "histogramArgument != qDefs::Intensity\n";
                    // val /= numValues;

                    // //find scan value
                    // int ci = 0, fi = 0;
                    // double cs0 = 0, cs1 = 0;
                    // fileIOStatic::getVariablesFromFileName(std::string(data->fileSaveName),
                    // ci, fi, cs0, cs1);

                    // int scanval = -1;
                    // if (cs0 != -1)
                    //     scanval = cs0;
                    // else
                    //     scanval = cs1;

                    // //ignore outside limits
                    // if ((scanval < histFrom) || (scanval > histTo) ||
                    // (scanval == -1))
                    //     scanval = -1;
                    // //check for intervals, increment if validates
                    // for (int j = 0; j < histogramSamples.size(); ++j) {
                    //     if (histogramSamples[j].interval.contains(scanval)) {
                    //         histogramSamples[j].value = val;
                    //         cout << "j:" << j << " scanval:" << scanval << "
                    //         val:" << val << endl;
                    //     }
                    // }
                }

            }
            // not histogram
            else {
                // Persistency
                if (currentPersistency < persistency)
                    currentPersistency++;
                else
                    currentPersistency = persistency;
                nHists = currentPersistency + 1;
                histNBins = nPixelsX;

                // copy data
                for (int i = currentPersistency; i > 0; i--)
                    memcpy(y1d[i], y1d[i - 1], nPixelsX * sizeof(double));

                // recalculating pedestal
                if (startPedestalCal) {
                    // start adding frames to get to the pedestal value
                    if (pedestalCount < NUM_PEDESTAL_FRAMES) {
                        for (unsigned int px = 0; px < nPixelsX; ++px)
                            tempPedestalVals[px] += data->values[px];
                        memcpy(y1d[0], data->values, nPixelsX * sizeof(double));
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
                    memcpy(y1d[0], data->values, nPixelsX * sizeof(double));
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
                            temp += y1d[0][px];
                        // after all processing
                        y1d[0][px] = temp;
                    }
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
                    memcpy(image2d, data->values,
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
                memcpy(image2d, data->values,
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
                        temp += image2d[px];
                    // after all processing
                    image2d[px] = temp;
                }
            }
        }
        /*	pthread_mutex_unlock(&(lastImageCompleteMutex));
                }*/
        plotRequired = true;
        UnlockLastImageArray();

#ifdef VERYVERBOSE
        cprintf(BLUE, "currentframe:%d \tcurrentframeindex:%d\n", currentFrame,
                currentFrameIndex);
#endif
        currentFrame++;
        emit UpdatePlotSignal();
    }

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
    if ((stopSignal) || (detectorStatus == slsDetectorDefs::ERROR)) {
#ifdef VERBOSE
        std::cout << "Error in Acquisition\n\n";
#endif
        // stopSignal = 1;//just to be sure
        emit AcquisitionErrorSignal(status);
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

    // calculate s curve inflection point
    int l1 = 0, l2 = 0, j;
    if ((histogram) && (histogramArgument != qDefs::Intensity)) {
        for (j = 0; j < histogramSamples.size() - 2; ++j) {
            l1 = histogramSamples[j + 1].value - histogramSamples[j].value;
            l2 = histogramSamples[j + 2].value - histogramSamples[j + 1].value;
            if (l1 > l2) {
                std::cout << "***** s curve inflectionfound at "
                          << histogramSamples[j].interval.maxValue()
                          << ""
                             "or j at "
                          << j << " with l1 " << l1 << " and l2 " << l2 << '\n';
            }
        }
    }

    return 0;
}

int qDrawPlot::GetProgressCallBack(double currentProgress, void *this_pointer) {
    ((qDrawPlot *)this_pointer)->progress = currentProgress;
    return 0;
}

void qDrawPlot::ShowAcquisitionErrorMessage(QString status) {
    if (!alreadyDisplayed) {
        alreadyDisplayed = true;
        qDefs::Message(qDefs::WARNING,
                       std::string("<nobr>The acquisiton has ended abruptly. "
                                   "Current Detector Status: ") +
                           status.toAscii().constData() +
                           std::string(".</nobr>"),
                       "qDrawPlot::ShowAcquisitionErrorMessage");
    }
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
void qDrawPlot::Clear1DPlot() {
    for (QVector<SlsQtH1D *>::iterator h = hists1d.begin();
         h != hists1d.end(); ++h) {
        (*h)->Detach(plot1d);
        // do not delete *h or h.
    }
}

void qDrawPlot::UpdatePlot() {
#ifdef VERYVERBOSE
    std::cout << "Entering UpdatePlot function\n";
#endif
    // only if no plot isnt enabled
    if (plotEnable && plotRequired) {
        LockLastImageArray();
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
            if (histNBins) {
                Clear1DPlot();
                plot1d->SetXTitle(xTitle1d.toAscii().constData());
                plot1d->SetYTitle(yTitle1d.toAscii().constData());

                // histogram
                if (histogram) {
                    plotHistogram->setData(
                        new QwtIntervalSeriesData(histogramSamples));
                    plotHistogram->setPen(QPen(Qt::red));
                    plotHistogram->setBrush(
                        QBrush(Qt::red, Qt::Dense4Pattern)); // Qt::SolidPattern
                    lblFrameIndexTitle1d->setText(GetTitle1d(0));
                    plotHistogram->attach(plot1d);
                    // refixing all the zooming

                    plot1d->SetXMinMax(startPixel, endPixel);
                    plot1d->SetYMinMax(0,
                                       plotHistogram->boundingRect().height());
                    plot1d->SetZoomBase(startPixel, 0, endPixel - startPixel,
                                        plotHistogram->boundingRect().height());

                }
                // not histogram
                else {
                    for (int hist_num = 0; hist_num < (int)nHists; ++hist_num) {
                        SlsQtH1D *h;
                        if (hist_num + 1 > hists1d.size()) {
                            if (anglePlot)
                                hists1d.append(
                                    h = new SlsQtH1D("", histNBins,
                                                     histXAngleAxis,
                                                     histYAngleAxis));
                            else
                                hists1d.append(
                                    h = new SlsQtH1D("", histNBins, x1d,
                                                     GetHistYAxis(hist_num)));
                            h->SetLineColor(hist_num);
                        } else {
                            h = hists1d.at(hist_num);
                            if (anglePlot)
                                h->SetData(histNBins, histXAngleAxis,
                                           histYAngleAxis);
                            else
                                h->SetData(histNBins, x1d,
                                           GetHistYAxis(hist_num));
                        }
                        SetStyle(h);
                        lblFrameIndexTitle1d->setText(GetTitle1d(0));
                        // h->setTitle(GetTitle1d(hist_num));
                        h->Attach(plot1d);
                        // refixing all the zooming
                        // if((firstPlot) || (anglePlot)){
                        /*plot1d->SetXMinMax(h->minXValue(),h->maxXValue());
                                                                        plot1d->SetYMinMax(h->minYValue(),h->maxYValue());
                                                                        plot1d->SetZoomBase(h->minXValue(),h->minYValue(),
                                                                                        h->maxXValue()-h->minXValue(),h->maxYValue()-h->minYValue());*/
                        //	firstPlot = false;
                        //}
                    }

                    /**moved from below (had applied to histograms as well) to
                     * here, */
                    // update range if required
                    if (XYRangeChanged) {
                        if (!isXYRangeEnable[qDefs::XMINIMUM])
                            XYRangeValues[qDefs::XMINIMUM] =
                                plot1d->GetXMinimum();
                        if (!isXYRangeEnable[qDefs::XMAXIMUM])
                            XYRangeValues[qDefs::XMAXIMUM] =
                                plot1d->GetXMaximum();
                        if (!isXYRangeEnable[qDefs::YMINIMUM])
                            XYRangeValues[qDefs::YMINIMUM] =
                                plot1d->GetYMinimum();
                        if (!isXYRangeEnable[qDefs::YMAXIMUM])
                            XYRangeValues[qDefs::YMAXIMUM] =
                                plot1d->GetYMaximum();
                        plot1d->SetXMinMax(XYRangeValues[qDefs::XMINIMUM],
                                           XYRangeValues[qDefs::XMAXIMUM]);
                        plot1d->SetYMinMax(XYRangeValues[qDefs::YMINIMUM],
                                           XYRangeValues[qDefs::YMAXIMUM]);
                        // Should not be reset for histogram,
                        // that is the only way to zoom in (new plots are zoomed
                        // out as its different each time)
                        if (!histogram)
                            XYRangeChanged = false;
                    }
                    /**moved from below (had applied to histograms as well) to
                     * here, */
                    // Display Statistics
                    if (displayStatistics) {
                        double min = 0, max = 0, sum = 0;
                        if (anglePlot)
                            GetStatistics(min, max, sum, histYAngleAxis,
                                          histNBins);
                        else
                            GetStatistics(min, max, sum, y1d[0], histNBins);
                        lblMinDisp->setText(QString("%1").arg(min));
                        lblMaxDisp->setText(QString("%1").arg(max));
                        lblSumDisp->setText(QString("%1").arg(sum));
                         widgetStatistics->show();
                    }
                }
            }
        } // 2-d plot stuff
        else {
            if (image2d) {
                if (nPixelsX > 0 && nPixelsY > 0) {
                    plot2d->GetPlot()->SetData(nPixelsX, -0.5, nPixelsX - 0.5,
                                               nPixelsY, startPixel, endPixel,
                                               image2d);
                    plot2d->setTitle(title2d.c_str());
                    plot2d->SetXTitle(xTitle2d);
                    plot2d->SetYTitle(yTitle2d);
                    plot2d->SetZTitle(zTitle2d);
                    // zmin and zmax of plot already calculated using SetData,
                    // now recalculate if z is set
                    plot2d->KeepZRangeIfSet();
                    if (gainDataExtracted) {
                        gainplot2d->GetPlot()->SetData(
                            nPixelsX, -0.5, nPixelsX - 0.5, nPixelsY,
                            startPixel, endPixel, gainImage);
                        gainplot2d->setTitle(title2d.c_str());
                        gainplot2d->show();
                    } else {
                        gainplot2d->hide();
                    }
                }
                // update range if required
                if (XYRangeChanged) {
                    if (!isXYRangeEnable[qDefs::XMINIMUM])
                        XYRangeValues[qDefs::XMINIMUM] =
                            plot2d->GetPlot()->GetXMinimum();
                    if (!isXYRangeEnable[qDefs::XMAXIMUM])
                        XYRangeValues[qDefs::XMAXIMUM] =
                            plot2d->GetPlot()->GetXMaximum();
                    if (!isXYRangeEnable[qDefs::YMINIMUM])
                        XYRangeValues[qDefs::YMINIMUM] =
                            plot2d->GetPlot()->GetYMinimum();
                    if (!isXYRangeEnable[qDefs::YMAXIMUM])
                        XYRangeValues[qDefs::YMAXIMUM] =
                            plot2d->GetPlot()->GetYMaximum();
                    plot2d->GetPlot()->SetXMinMax(
                        XYRangeValues[qDefs::XMINIMUM],
                        XYRangeValues[qDefs::XMAXIMUM]);
                    plot2d->GetPlot()->SetYMinMax(
                        XYRangeValues[qDefs::YMINIMUM],
                        XYRangeValues[qDefs::YMAXIMUM]);
                    gainplot2d->GetPlot()->SetXMinMax(
                        XYRangeValues[qDefs::XMINIMUM],
                        XYRangeValues[qDefs::XMAXIMUM]);
                    gainplot2d->GetPlot()->SetYMinMax(
                        XYRangeValues[qDefs::YMINIMUM],
                        XYRangeValues[qDefs::YMAXIMUM]);
                    XYRangeChanged = false;
                }
                plot2d->GetPlot()->Update();
                if (gainDataExtracted) {
                    gainplot2d->GetPlot()->Update();
                    gainplot2d->setFixedWidth(plot2d->width() / 4);
                    gainplot2d->setFixedHeight(plot2d->height() / 4);
                    gainplot2d->show();
                } else
                    gainplot2d->hide();
                // Display Statistics
                if (displayStatistics) {
                    double min = 0, max = 0, sum = 0;
                    GetStatistics(min, max, sum, image2d, nPixelsX * nPixelsY);
                    lblMinDisp->setText(QString("%1").arg(min));
                    lblMaxDisp->setText(QString("%1").arg(max));
                    lblSumDisp->setText(QString("%1").arg(sum));
                }
            }
        }
        // set plot title
        boxPlot->setTitle(plotTitle);
        // to notify the measurement finished when its done
        plotRequired = false;
        UnlockLastImageArray();
    }

#ifdef VERYVERBOSE
    std::cout << "Exiting UpdatePlot function\n";
#endif
}