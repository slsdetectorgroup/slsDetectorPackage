#include "qDrawPlot.h"

#include "SlsQt1DPlot.h"
#include "SlsQt2DPlotLayout.h"
#include "detectorData.h"
#include "qCloneWidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QFileDialog>
#include <QPainter>
#include "qwt_symbol.h"
#include <QtConcurrentRun>



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
    if (data2d)
        delete [] data2d;
     if (gainData)
        delete [] gainData;       

    if (plot1d)
        delete plot1d;
    if (plot2d)
        delete plot2d;
    if (gainplot2d)
        delete gainplot2d;

    if (pedestalVals)
        delete [] pedestalVals;
    if (tempPedestalVals)
        delete [] tempPedestalVals;   

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
    } catch (const std::exception &e) {
        qDefs::ExceptionMessage("Could not get file path or file name.", e.what(), "qDrawPlot::SetupWidgetWindow");
        fileSavePath = "/tmp";
        fileSaveName = "Image";
	}   

    SetupPlots();
    SetupStatistics();  
    SetDataCallBack(true);
    myDet->registerAcquisitionFinishedCallback(&(GetAcquisitionFinishedCallBack), this);
    myDet->registerProgressCallback(&(GetProgressCallBack), this);

    // future watcher to watch result of AcquireThread only because it uses signals/slots to handle acquire exception
    acqResultWatcher = new QFutureWatcher<std::string>();

    Initialization();
}

void qDrawPlot::Initialization() {
    connect(this, SIGNAL(UpdateSignal()), this, SLOT(UpdatePlot()));
    connect(acqResultWatcher, SIGNAL(finished()), this, SLOT(AcquireFinished()));
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
        npixelsy_jctb = (myDet->setTimer(slsDetectorDefs::ANALOG_SAMPLES, -1) * 2) /
                        25; // for moench 03
        nPixelsX = npixelsx_jctb;
        nPixelsY = npixelsy_jctb;
    }
    FILE_LOG(logINFO) << "nPixelsX:" << nPixelsX;
    FILE_LOG(logINFO) << "nPixelsY:" << nPixelsY;

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
        datay1d[0][px] = px;
    }
    // add a hist
    SlsQtH1D *h = new SlsQtH1D("", nPixelsX, datax1d, datay1d[0]);
    h->SetLineColor(0);
    SetStyleandSymbol(h);
    hists1d.append(h);
    h->Attach(plot1d);
    plot1d->DisableZoom(true);

    // setup 2d plot
    plot2d = new SlsQt2DPlotLayout(boxPlot);
    // default display data 
    if (data2d)
        delete [] data2d;    
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
                               -0.5, nPixelsY - 0.5, data2d);
    plot2d->setTitle("");
    plot2d->SetXTitle(xTitle2d);
    plot2d->SetYTitle(yTitle2d);
    plot2d->SetZTitle(zTitle2d);
    plot2d->setAlignment(Qt::AlignLeft);

    // gainplot
    gainplot2d = new SlsQt2DPlotLayout(boxPlot);
    if (gainData)
        delete [] gainData;    
    gainData = new double[nPixelsY * nPixelsX];
    for (unsigned int px = 0; px < nPixelsX; ++px)
        for (unsigned int py = 0; py < nPixelsY; ++py)
            gainData[py * nPixelsX + px] =
                sqrt(pow(0 + 1, 2) * pow(double(px) - nPixelsX / 2, 2) /
                         pow(nPixelsX / 2, 2) / pow(1 + 1, 2) +
                     pow(double(py) - nPixelsY / 2, 2) / pow(nPixelsY / 2, 2)) /
                sqrt(2);
    gainplot2d->setFont(QFont("Sans Serif", 9, QFont::Normal));
    gainplot2d->GetPlot()->SetData(nPixelsX, -0.5, nPixelsX - 0.5, nPixelsY,
                                   -0.5, nPixelsY - 0.5, gainData);
    gainplot2d->setTitle("");
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

bool qDrawPlot::GetIsRunning() { 
    return isRunning; 
}

void qDrawPlot::SetRunning(bool enable) {
    isRunning = enable;
}

int qDrawPlot::GetProgress() { 
    return progress; 
}

int64_t qDrawPlot::GetCurrentFrameIndex() { 
    return currentFrame; 
}

void qDrawPlot::Select1dPlot(bool enable) { 
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
}

void qDrawPlot::SetPlotTitlePrefix(QString title) { 
    FILE_LOG(logINFO) << "Setting Title to " << title.toAscii().constData();
    plotTitlePrefix = title; 
}

void qDrawPlot::SetXAxisTitle(QString title) {
    FILE_LOG(logINFO) << "Setting X Axis Title to " << title.toAscii().constData();
    if (is1d) {
        xTitle1d = title;
    } else {
        xTitle2d = title;
    }
}

void qDrawPlot::SetYAxisTitle(QString title) {
    FILE_LOG(logINFO) << "Setting Y Axis Title to " << title.toAscii().constData();
    if (is1d) {
        yTitle1d = title;
    } else {
        yTitle2d = title;
    }
}

void qDrawPlot::SetZAxisTitle(QString title) { 
    FILE_LOG(logINFO) << "Setting Z Axis Title to " << title.toAscii().constData();
    zTitle2d = title; 
}

void qDrawPlot::SetXYRangeChanged(bool disable, double* xy, bool* isXY) { 
    std::lock_guard<std::mutex> lock(mPlots);
    FILE_LOG(logINFO) << "XY Range has changed";
    xyRangeChanged = true;  
    std::copy(xy, xy + 4, xyRange);
    std::copy(isXY, isXY + 4, isXYRange);

    FILE_LOG(logDEBUG) << "Setting Disable zoom to " << std::boolalpha << disable << std::noboolalpha;
    disableZoom = disable;
}

void qDrawPlot::SetZRange(double* z, bool* isZ) {
    std::copy(z, z + 2, zRange);
    std::copy(isZ, isZ + 2, isZRange);
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

void qDrawPlot::SetDataCallBack(bool enable) {
    FILE_LOG(logINFO) << "Setting data call back to " << std::boolalpha << enable << std::noboolalpha;
    if (enable) {
        isPlot = true;
        myDet->registerDataCallback(&(GetDataCallBack), this); 
    } else {
        isPlot = false;
        myDet->registerDataCallback(nullptr, this);
    }
  
}

void qDrawPlot::SetBinary(bool enable, int from, int to) {
    FILE_LOG(logINFO) << (enable ? "Enabling" : "Disabling") << " Binary output from " << from << " to " << to;
    binaryFrom = from;
    binaryTo = to;
    isBinary = enable;
}

void qDrawPlot::SetPersistency(int val) {
    FILE_LOG(logINFO) << "Setting Persistency to " << val;
    persistency = val; 
}

void qDrawPlot::SetLines(bool enable) { 
    std::lock_guard<std::mutex> lock(mPlots);
    FILE_LOG(logINFO) << "Setting Lines to " << std::boolalpha << enable << std::noboolalpha;
    isLines = enable; 
    for (unsigned int i = 0; i < nHists; ++i) {
        SlsQtH1D* h = hists1d.at(i);
        SetStyleandSymbol(h);
    }
}

void qDrawPlot::SetMarkers(bool enable) { 
    std::lock_guard<std::mutex> lock(mPlots);
    FILE_LOG(logINFO) << "Setting Markers to " << std::boolalpha << enable << std::noboolalpha;
    isMarkers = enable;
    for (unsigned int i = 0; i < nHists; ++i) {
        SlsQtH1D* h = hists1d.at(i);
        SetStyleandSymbol(h);
    }
}

void qDrawPlot::Set1dLogY(bool enable) {
    std::lock_guard<std::mutex> lock(mPlots);
    FILE_LOG(logINFO) << "Setting Log Y to " << std::boolalpha << enable << std::noboolalpha;
    plot1d->SetLogY(enable);
}

void qDrawPlot::SetInterpolate(bool enable) {
    std::lock_guard<std::mutex> lock(mPlots);
    FILE_LOG(logINFO) << "Setting Interpolate to " << std::boolalpha << enable << std::noboolalpha;
    plot2d->SetInterpolate(enable); 
}

void qDrawPlot::SetContour(bool enable) {
    std::lock_guard<std::mutex> lock(mPlots);
    FILE_LOG(logINFO) << "Setting Countour to " << std::boolalpha << enable << std::noboolalpha;
    plot2d->SetContour(enable);
}

void qDrawPlot::SetLogz(bool enable) {
    std::lock_guard<std::mutex> lock(mPlots);
    FILE_LOG(logINFO) << "Setting Log Z to " << std::boolalpha << enable << std::noboolalpha;
    plot2d->SetLogz(enable, isZRange[0], isZRange[1], zRange[0], zRange[1]);
}

void qDrawPlot::SetPedestal(bool enable) {
    std::lock_guard<std::mutex> lock(mPlots);
    FILE_LOG(logINFO) << (enable ? "Enabling" : "Disabling") << " Pedestal";
    isPedestal = enable;
    resetPedestal = true;
}

void qDrawPlot::RecalculatePedestal() {
    std::lock_guard<std::mutex> lock(mPlots);
    FILE_LOG(logDEBUG) << "Recalculating Pedestal";
    resetPedestal = true;
}

void qDrawPlot::SetAccumulate(bool enable) {
    std::lock_guard<std::mutex> lock(mPlots);
    FILE_LOG(logINFO) << (enable ? "Enabling" : "Disabling") << " Accumulation";
    isAccumulate = enable;
    resetAccumulate = true;
}

void qDrawPlot::ResetAccumulate() {
    std::lock_guard<std::mutex> lock(mPlots);
    FILE_LOG(logDEBUG) << "Resetting Accumulation";   
    resetAccumulate = true;
}

void qDrawPlot::DisplayStatistics(bool enable) {
    FILE_LOG(logINFO) << (enable ? "Enabling" : "Disabling") << " Statistics Display";
    displayStatistics = enable;
}

void qDrawPlot::EnableGainPlot(bool enable) {
    FILE_LOG(logINFO) << (enable ? "Enabling" : "Disabling") << " Gain Plot";
    hasGainData = enable;
}

void qDrawPlot::SetSaveFileName(QString val) {
    FILE_LOG(logDEBUG) << "Setting Clone/Save File Name to " << val.toAscii().constData();
    fileSaveName = val;
}

void qDrawPlot::ClonePlot() {
    std::lock_guard<std::mutex> lock(mPlots);

    int index = 0;
    if (is1d) {
        FILE_LOG(logINFO) << "Cloning 1D Image";
        qCloneWidget *q = new qCloneWidget(
            this, cloneWidgets.size(), boxPlot->title(), xTitle1d, yTitle1d, "", 1,
            fileSavePath, fileSaveName, currentFrame, displayStatistics, lblMinDisp->text(),
            lblMaxDisp->text(), lblSumDisp->text());
        cloneWidgets.push_back(q);
        index = cloneWidgets.size();
        cloneWidgets[index]->SetCloneHists(nHists, nPixelsX, datax1d, datay1d,
                                       lblFrameIndexTitle1d->text(), isLines, isMarkers);                                                            
    } else  {
        FILE_LOG(logINFO) << "Cloning 2D Image";
        qCloneWidget *q = new qCloneWidget(
            this, cloneWidgets.size(), boxPlot->title(), xTitle2d, yTitle2d, zTitle2d, 2, 
            fileSavePath, fileSaveName, currentFrame, displayStatistics, lblMinDisp->text(),
            lblMaxDisp->text(), lblSumDisp->text());
        cloneWidgets.push_back(q);
        index = cloneWidgets.size();
        cloneWidgets[index]->SetCloneHists2D(nPixelsX, -0.5, nPixelsX - 0.5,
                                         nPixelsY, -0.5, nPixelsY - 0.5,
                                         data2d, plot2d->title(), isZRange[0], isZRange[1], zRange[0], zRange[1]);
    }

    cloneWidgets[index]->show();

    // to remember which all clone widgets were closed
    connect(cloneWidgets[index], SIGNAL(CloneClosedSignal(int)), this, SLOT(CloneCloseEvent(int)));
}

void qDrawPlot::CloseClones() {
    FILE_LOG(logDEBUG) << "Closing all Clones"; 
    for (auto &it : cloneWidgets) {
        it->close();
    }
}

void qDrawPlot::CloneCloseEvent(int id) {
    FILE_LOG(logDEBUG) << "Closing Clone " << id; 
    cloneWidgets.erase(cloneWidgets.begin() + id);
}

void qDrawPlot::SaveClones() {
    FILE_LOG(logINFO) << "Saving all Clones"; 
    char errID[200];
    std::string errMessage = "The Snapshots with ID's: ";
    bool success = true;
    for (unsigned int i = 0; i < cloneWidgets.size(); ++i) {
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

    QString fName = fileSavePath + QString('/') + fileSaveName + QString('_') + currentFrame +  QString('_') + QString(NowTime().c_str()) + QString(".png");
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

void qDrawPlot::SetStyleandSymbol(SlsQtH1D *h) {
    h->setStyle(isLines ? QwtPlotCurve::Lines : QwtPlotCurve::Dots);
#if QWT_VERSION < 0x060000
    h->setSymbol(isMarkers ? *marker : *noMarker);
#else
    h->setSymbol(isMarkers ? marker : noMarker);
#endif
}

void qDrawPlot::GetStatistics(double &min, double &max, double &sum) {
 FILE_LOG(logDEBUG) << "Calculating Statistics";   
    double* array = data2d;
    int size = nPixelsX * nPixelsY;
    if(is1d) {
        array = datay1d[0];
        size = nPixelsX;
    }
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

void qDrawPlot::StartAcquisition() {
    FILE_LOG(logDEBUG) << "Starting Acquisition in qDrawPlot";
    progress = 0;
    currentFrame = 0;
    boxPlot->setTitle("Old Plot");
    // check acquiring flag (from previous exit) or if running
    try{
        if (myDet->getAcquiringFlag()) {
            if (myDet->getRunStatus() != slsDetectorDefs::IDLE) {
                qDefs::Message(qDefs::WARNING, "Could not start acquisition as it is already in progress.\nClick start when finished.", "qDrawPlot::StartAcquisition");
                emit AbortSignal();
                return;
            } else {
                myDet->setAcquiringFlag(false);
            }

        }
    } CATCH_DISPLAY("Could not get detector stats.", "qDrawPlot::StartAcquisition");
   
    // ensure data streaming in receiver (if plot enabled)
    if (isPlot) {
        try {
           if (myDet->enableDataStreamingFromReceiver() != 1) {
               myDet->enableDataStreamingFromReceiver(1);
           }
        } CATCH_DISPLAY("Could not enable data streaming in Receiver.", "qDrawPlot::StartAcquisition");
    }

    // refixing all the zooming
    {
        std::lock_guard<std::mutex> lock(mPlots);
        pedestalCount = 0;
        xyRangeChanged = true; 
    }

    // acquisition in another thread
    QFuture<std::string> future = QtConcurrent::run(this, &qDrawPlot::AcquireThread);
    acqResultWatcher->setFuture(future);

    FILE_LOG(logDEBUG) << "End of Starting Acquisition in qDrawPlot";
}

void qDrawPlot::AcquireFinished() {
    FILE_LOG(logDEBUG) << "Acquisition Finished";
    std::string mess = acqResultWatcher->result();
    // exception in acquire will not call acquisition finished call back, so handle it
    if (!mess.empty()) {
        FILE_LOG(logERROR) << "Acquisition Finished with an exception: " << mess;
        qDefs::ExceptionMessage("Acquire unsuccessful.", mess, "qDrawPlot::AcquireFinished");
        try{
            myDet->stopAcquisition();
            myDet->stopReceiver();
        } CATCH_DISPLAY("Could not stop acquisition and receiver.", "qDrawPlot::AcquireFinished");
        emit AbortSignal();
    }
    FILE_LOG(logDEBUG) << "End of Acquisition Finished";
}

std::string qDrawPlot::AcquireThread() {
    FILE_LOG(logDEBUG) << "Acquire Thread";
    try {
        myDet->acquire();
    } catch (const std::exception &e) {
        return std::string(e.what());
    }
    return std::string("");
}

void qDrawPlot::GetProgressCallBack(double currentProgress, void *this_pointer) {
    ((qDrawPlot *)this_pointer)->progress = currentProgress;
    FILE_LOG(logDEBUG) << "Progress Call back successful";
}

void qDrawPlot::GetAcquisitionFinishedCallBack(double currentProgress, int detectorStatus, void *this_pointer) {
    ((qDrawPlot *)this_pointer)->AcquisitionFinished(currentProgress, detectorStatus);
    FILE_LOG(logDEBUG) << "Acquisition Finished Call back successful";
}

void qDrawPlot::GetDataCallBack(detectorData *data, uint64_t frameIndex, uint32_t subFrameIndex, void *this_pointer) {
    ((qDrawPlot *)this_pointer)->GetData(data, frameIndex, subFrameIndex);
    FILE_LOG(logDEBUG) << "Get Data Call back successful";
}

void qDrawPlot::AcquisitionFinished(double currentProgress, int detectorStatus) {
    std::string status = slsDetectorDefs::runStatusType(static_cast<slsDetectorDefs::runStatus>(detectorStatus));
    
    if (detectorStatus == slsDetectorDefs::ERROR) {
        qDefs::Message(qDefs::WARNING, std::string("<nobr>The acquisiton has ended abruptly. Current Detector Status: ") + status + std::string(".</nobr>"), "qDrawPlot::AcquisitionFinished");
        FILE_LOG(logERROR) << "Acquisition finished [Status: ERROR]";
    } else {
        FILE_LOG(logINFO) << "Acquisition finished [ Status:" << status << ", Progress: " << currentProgress << " ]" ;
    }
    emit AcquireFinishedSignal();
}

void qDrawPlot::GetData(detectorData *data, uint64_t frameIndex, uint32_t subFrameIndex) {
    std::lock_guard<std::mutex> lock(mPlots);
    FILE_LOG(logDEBUG)
    << "* GetData Callback *" << std::endl
    << "  frame index: " << frameIndex << std::endl
    << "  sub frame index: " << (((int)subFrameIndex == -1) ? (int)-1 : subFrameIndex) << std::endl  
    << "  Data [" << std::endl  
    << "  \t progress: " << data->progressIndex << std::endl  
    << "  \t file name: " << data->fileName << std::endl  
    << "  \t nx: " << data->nx << std::endl  
    << "  \t ny: " << data->ny << std::endl  
    << "  \t data bytes: " << data->databytes << std::endl  
    << "  \t dynamic range: " << data->dynamicRange << std::endl  
    << "  \t file index: " << data->fileIndex << std::endl
    << "  ]";  
    
    progress = (int)data->progressIndex;
    currentFrame =  frameIndex;
    FILE_LOG(logDEBUG) << "[ Progress:" << progress << ", Frame:" << currentFrame << " ]";

    //FIXME: check npixelsx and npixelsY (change to this new val, if it is not, and look out for sideeffects)

    // convert data to double
    unsigned int nPixels = nPixelsX * (is1d ? 1 : nPixelsY);
    double* rawData = new double[nPixels];
    if (hasGainData) {
        toDoublePixelData(rawData, data->data, nPixels, data->databytes, data->dynamicRange, gainData);
        isGainDataExtracted = true;
    } else {
        toDoublePixelData(rawData, data->data, nPixels, data->databytes, data->dynamicRange);
        isGainDataExtracted = false;
    }

    // title and frame index titles
    plotTitle = plotTitlePrefix + QString(data->fileName.c_str()).section('/', -1);
    indexTitle = QString("%1").arg(frameIndex);
    if ((int)subFrameIndex != -1) {
        indexTitle = QString("%1 %2").arg(frameIndex, subFrameIndex);
    }

    // reset pedestal
    if (resetPedestal) {
        pedestalCount = 0;
        if (pedestalVals != nullptr)
            delete [] pedestalVals;
        pedestalVals = new double[nPixels];
        std::fill(pedestalVals, pedestalVals + nPixels, 0);
        if (tempPedestalVals != nullptr)
            delete [] tempPedestalVals;
        tempPedestalVals = new double[nPixels];
        std::fill(tempPedestalVals, tempPedestalVals + nPixels, 0);
        resetPedestal = false;
    }
    
    if (isPedestal) {
        // add pedestals frames
        if (pedestalCount < NUM_PEDESTAL_FRAMES) {
            for (unsigned int px = 0; px < nPixels; ++px)
                tempPedestalVals[px] += rawData[px];
            pedestalCount++;
        }
        // calculate the pedestal value
        if (pedestalCount == NUM_PEDESTAL_FRAMES) {
            FILE_LOG(logINFO) << "Pedestal Calculated after " << NUM_PEDESTAL_FRAMES << " frames";
            for (unsigned int px = 0; px < nPixels; ++px)
                tempPedestalVals[px] = tempPedestalVals[px] / (double)NUM_PEDESTAL_FRAMES;
            memcpy(pedestalVals, tempPedestalVals, nPixels * sizeof(double));
        }
    }

    if (is1d) {
        Get1dData(rawData);
    } else {   
        Get2dData(rawData);
    }

    FILE_LOG(logDEBUG) << "End of Get Data";
    emit UpdateSignal();
}

void qDrawPlot::Get1dData(double* rawData) {

    // persistency
    if (currentPersistency < persistency)
        currentPersistency++;
    else
        currentPersistency = persistency; // when reducing persistency
    nHists = currentPersistency + 1;
    if (currentPersistency) {
        // allocate
        for(int i = datay1d.size(); i <= persistency; ++i) {
            datay1d.push_back(new double [nPixelsX]);
            SlsQtH1D* h = new SlsQtH1D("", nPixelsX, datax1d, datay1d[i]);
            h->SetLineColor(i);
            SetStyleandSymbol(h);
            hists1d.append(h);
        }
        // copy previous data
        for (int i = currentPersistency; i > 0; --i)
            memcpy(datay1d[i], datay1d[i - 1], nPixelsX * sizeof(double));
    }
    // pedestal
    if (isPedestal) {
        for (unsigned int px = 0; px < nPixelsX; ++px) {
            rawData[px] =- (pedestalVals[px]);
        }
    }
    // accumulate
    if (resetAccumulate) {
        std::fill(datay1d[0], datay1d[0] + nPixelsX, 0);
        resetAccumulate = false;
    }
    if (isAccumulate) {
        for (unsigned int px = 0; px < nPixelsX; ++px) {
            rawData[px] += datay1d[0][px];
        }
    }
    // binary
    if (isBinary) {
        int lBinaryFrom = binaryFrom;
        int lBinaryTo = binaryTo;
        for (unsigned int px = 0; px < nPixelsX; ++px) {
            if ((rawData[px] >= lBinaryFrom) && (rawData[px] <= lBinaryTo))
                rawData[px] = 1;
            else
                rawData[px] = 0;
        }
    }
    memcpy(datay1d[0], rawData, nPixelsX * sizeof(double));
}

void qDrawPlot::Get2dData(double* rawData) {
    unsigned int nPixels = nPixelsX * nPixelsY;
    // pedestal
    if (isPedestal) {
        for (unsigned int px = 0; px < nPixels; ++px) {
            rawData[px] =- (pedestalVals[px]);
        }
    }
    // accumulate
    if (resetAccumulate) {
        std::fill(data2d, data2d + nPixels, 0);
        resetAccumulate = false;
    }
    if (isAccumulate) {
        for (unsigned int px = 0; px < nPixels; ++px) {
            rawData[px] += data2d[px];
        }
    }
    // binary
    if (isBinary) {
        int lBinaryFrom = binaryFrom;
        int lBinaryTo = binaryTo;
        for (unsigned int px = 0; px < nPixels; ++px) {
            if ((rawData[px] >= lBinaryFrom) && (rawData[px] <= lBinaryTo))
                rawData[px] = 1;
            else
                rawData[px] = 0;
        }
    }
    memcpy(data2d, rawData, nPixels * sizeof(double));
}

void qDrawPlot::Update1dPlot() {
    DetachHists();
    plot1d->SetXTitle(xTitle1d.toAscii().constData());
    plot1d->SetYTitle(yTitle1d.toAscii().constData());
    for (unsigned int i = 0; i < nHists; ++i) {
        SlsQtH1D* h = hists1d.at(i);
        h->SetData(nPixelsX, datax1d, datay1d[i]);
        h->Attach(plot1d);
    }
    if (xyRangeChanged) {
        Update1dXYRange();
        xyRangeChanged = false;
    }
    plot1d->DisableZoom(disableZoom);
}

void qDrawPlot::Update2dPlot() {
    plot2d->SetXTitle(xTitle2d);
    plot2d->SetYTitle(yTitle2d);
    plot2d->SetZTitle(zTitle2d);
    plot2d->GetPlot()->SetData(nPixelsX, -0.5, nPixelsX - 0.5,
                                               nPixelsY, -0.5, nPixelsY - 0.5, data2d);
     if (isGainDataExtracted) {
        gainplot2d->GetPlot()->SetData(nPixelsX, -0.5, nPixelsX - 0.5, nPixelsY,
                -0.5, nPixelsY - 0.5, gainData);
        gainplot2d->setFixedWidth(plot2d->width() / 4);
        gainplot2d->setFixedHeight(plot2d->height() / 4);
        gainplot2d->show();
    } else {
        gainplot2d->hide();  
    }
    if (xyRangeChanged) {
        Update2dXYRange();
        xyRangeChanged = false;
    }
    plot2d->GetPlot()->DisableZoom(disableZoom);
    plot2d->SetZRange(isZRange[0], isZRange[1], zRange[0], zRange[1]);
}

void qDrawPlot::Update1dXYRange() {
    if (!isXYRange[qDefs::XMIN] && !isXYRange[qDefs::XMAX]) {
        plot1d->EnableXAutoScaling();
    } else {
        double xmin = (isXYRange[qDefs::XMIN] ? xyRange[qDefs::XMIN] : plot1d->GetXMinimum());
        double xmax = (isXYRange[qDefs::XMAX] ? xyRange[qDefs::XMAX] : plot1d->GetXMaximum());
        plot1d->SetXMinMax(xmin, xmax);
    } 

    if (!isXYRange[qDefs::YMIN] && !isXYRange[qDefs::YMAX]) {
        plot1d->EnableYAutoScaling();
    } else {
        double ymin = (isXYRange[qDefs::YMIN] ? xyRange[qDefs::YMIN] : plot1d->GetYMinimum());
        double ymax = (isXYRange[qDefs::YMAX] ? xyRange[qDefs::YMAX] : plot1d->GetYMaximum());
        plot1d->SetYMinMax(ymin, ymax);
    } 
    plot1d->Update();
}

void qDrawPlot::Update2dXYRange() {
    if (!isXYRange[qDefs::XMIN] && !isXYRange[qDefs::XMAX]) {
        plot2d->GetPlot()->EnableXAutoScaling();
    } else {
        double xmin = (isXYRange[qDefs::XMIN] ? xyRange[qDefs::XMIN] : plot2d->GetPlot()->GetXMinimum());
        double xmax = (isXYRange[qDefs::XMAX] ? xyRange[qDefs::XMAX] : plot2d->GetPlot()->GetXMaximum());
        plot2d->GetPlot()->SetXMinMax(xmin, xmax);
    } 

    if (!isXYRange[qDefs::YMIN] && !isXYRange[qDefs::YMAX]) {
        plot2d->GetPlot()->EnableYAutoScaling();
    } else {
        double ymin = (isXYRange[qDefs::YMIN] ? xyRange[qDefs::YMIN] : plot2d->GetPlot()->GetYMinimum());
        double ymax = (isXYRange[qDefs::YMAX] ? xyRange[qDefs::YMAX] : plot2d->GetPlot()->GetYMaximum());
        plot2d->GetPlot()->SetYMinMax(ymin, ymax);
    } 
    plot2d->GetPlot()->Update();
}

void qDrawPlot::toDoublePixelData(double *dest, char *source, int size, int databytes, int dr, double *gaindest) {
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


void qDrawPlot::UpdatePlot() {
    std::lock_guard<std::mutex> lock(mPlots);
    FILE_LOG(logDEBUG) << "Update Plot";
    
    boxPlot->setTitle(plotTitle);
    if (is1d) {
        lblFrameIndexTitle1d->setText(indexTitle);
        Update1dPlot();
    } else {   
        plot2d->setTitle(indexTitle.toAscii().constData());
        if (isGainDataExtracted)
            gainplot2d->setTitle(indexTitle.toAscii().constData());
        Update2dPlot();
    }

    if (displayStatistics) {
        double min = 0, max = 0, sum = 0;
        GetStatistics(min, max, sum);
        lblMinDisp->setText(QString("%1").arg(min));
        lblMaxDisp->setText(QString("%1").arg(max));
        lblSumDisp->setText(QString("%1").arg(sum));
        widgetStatistics->show();
    } else {
        widgetStatistics->hide();
    }

    FILE_LOG(logDEBUG) << "End of Update Plot";
}
