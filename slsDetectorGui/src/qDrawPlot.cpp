#include "qDrawPlot.h"

#include "SlsQt1DPlot.h"
#include "SlsQt2DPlot.h"
#include "detectorData.h"
#include "qCloneWidget.h"

#include "detectorData.h"

#include <QFileDialog>
#include <QPainter>
#include <QtConcurrentRun>
#include <QResizeEvent>


qDrawPlot::qDrawPlot(QWidget *parent, sls::Detector *detector) : QWidget(parent), det(detector) {
    setupUi(this);
    SetupWidgetWindow();
    FILE_LOG(logINFO) << "Plots ready";
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
}

void qDrawPlot::SetupWidgetWindow() {
    detType = det->getDetectorType().squash();
    // save
    try {
        std::string temp = det->getFilePath().squash("/tmp/");
        fileSavePath = QString(temp.c_str());
        temp = det->getFileNamePrefix().squash("xxx");
        fileSaveName = QString(temp.c_str());
    } catch (const std::exception &e) {
        qDefs::ExceptionMessage("Could not get file path or file name.", e.what(), "qDrawPlot::SetupWidgetWindow");
        fileSavePath = "/tmp";
        fileSaveName = "Image";
	}   

    SetupPlots();
    SetDataCallBack(true);
    det->registerAcquisitionFinishedCallback(&(GetAcquisitionFinishedCallBack), this);
    // future watcher to watch result of AcquireThread only because it uses signals/slots to handle acquire exception
    acqResultWatcher = new QFutureWatcher<std::string>();

    Initialization();
}

void qDrawPlot::Initialization() {
    connect(this, SIGNAL(UpdateSignal()), this, SLOT(UpdatePlot()));
    connect(acqResultWatcher, SIGNAL(finished()), this, SLOT(AcquireFinished()));
}

void qDrawPlot::SetupPlots() {
    setFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
 
    // default image size
    slsDetectorDefs::xy res = det->getDetectorSize();
    nPixelsX = res.x;
    nPixelsY = res.y;
    switch(detType) {
    case slsDetectorDefs::MOENCH:
        try{
            npixelsy_jctb = (det->getNumberOfAnalogSamples().tsquash("Inconsistent values for number of analog samples") * 2)/25;// for moench 03
            nPixelsX = npixelsx_jctb;
            nPixelsY = npixelsy_jctb;
        } CATCH_DISPLAY ("Could not get number of analog samples.", "qDrawPlot::SetupPlots")
        break;
    case slsDetectorDefs::EIGER:
        try{
            if (det->getQuad().tsquash("Inconsistent values for quad type")) {
                nPixelsX /= 2;
                nPixelsY *= 2;
                if (nPixelsX != nPixelsY) {
                    --nPixelsX;
                }
            }
        } CATCH_DISPLAY ("Could not get quad.", "qDrawPlot::SetupPlots")
        break;
    default:
        break;
    }
    FILE_LOG(logINFO) << "nPixelsX:" << nPixelsX;
    FILE_LOG(logINFO) << "nPixelsY:" << nPixelsY;

    boxPlot->setFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
    widgetStatistics->hide();

    // setup 1d data
    if (datax1d)
        delete[] datax1d;
    datax1d = new double[nPixelsX];
    if (datay1d.size()) {
        for (auto &it : datay1d) {
            delete[] it;
        }
        datay1d.clear();
    }
   datay1d.push_back(new double[nPixelsX]);
    // default display data
    for (unsigned int px = 0; px < nPixelsX; ++px) {
        datax1d[px] = px;
        datay1d[0][px] = 0;
    }
    // add a hist
    SlsQtH1D *h = new SlsQtH1D("", nPixelsX, datax1d, datay1d[0]);
    h->SetLineColor(0);
    h->setStyleLinesorDots(isLines);
    h->setSymbolMarkers(isMarkers);
    hists1d.append(h);
    // setup 1d plot
    plot1d = new SlsQt1DPlot(boxPlot);
    plot1d->setFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
    plot1d->SetTitleFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
    plot1d->SetXFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
    plot1d->SetYFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
    plot1d->SetTitle("--");
    plot1d->SetXTitle(xTitle1d);
    plot1d->SetYTitle(yTitle1d);
    h->Attach(plot1d);
    plot1d->hide();

    // setup 2d data
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
    // setup 2d plot
    plot2d = new SlsQt2DPlot(boxPlot);
    plot2d->SetData(nPixelsX, -0.5, nPixelsX - 0.5, nPixelsY,
                               -0.5, nPixelsY - 0.5, data2d);
    plot2d->setFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
    plot2d->SetTitleFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
    plot2d->SetXFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
    plot2d->SetYFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
    plot2d->SetZFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));    
    plot2d->setTitle("");
    plot2d->SetXTitle(xTitle2d);
    plot2d->SetYTitle(yTitle2d);
    plot2d->SetZTitle(zTitle2d); 

    gainplot2d = new SlsQt2DPlot(boxPlot);
    gainplot2d->SetData(nPixelsX, -0.5, nPixelsX - 0.5, nPixelsY, -0.5, nPixelsY - 0.5, gainData);
    gainplot2d->setFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
    gainplot2d->setTitle("");
    gainplot2d->enableAxis(0, false);
    gainplot2d->enableAxis(1, false);
    gainplot2d->enableAxis(2, false);
    gainplot2d->hide();

    // layout of plots
    int ratio = qDefs::DATA_GAIN_PLOT_RATIO - 1;
    plotLayout->addWidget(plot1d, 0, 0, ratio, ratio);
    plotLayout->addWidget(plot2d, 0, 0, ratio, ratio);
    plotLayout->addWidget(gainplot2d, 0, ratio, 1, 1, Qt::AlignRight | Qt::AlignTop);
}

void qDrawPlot::resizeEvent(QResizeEvent *event) {
    if (gainplot2d->isVisible()) {
        gainplot2d->setFixedWidth(plot2d->width() / qDefs::DATA_GAIN_PLOT_RATIO);
        gainplot2d->setFixedHeight(plot2d->height() / qDefs::DATA_GAIN_PLOT_RATIO);
    }
    event->accept();
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
        is1d = true;
        // DetachHists(); it clears the last measurement
        plot1d->SetXTitle(xTitle1d);
        plot1d->SetYTitle(yTitle1d);
        plot1d->show();
        plot2d->hide();
    } else {
        is1d = false;
        plot2d->SetTitle("");
        plot2d->SetXTitle(xTitle2d);
        plot2d->SetYTitle(yTitle2d);
        plot2d->SetZTitle(zTitle2d);
        plot1d->hide();
        plot2d->show();
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
        return plot2d->GetXMinimum();
}

double qDrawPlot::GetXMaximum() {
    if (is1d)
        return plot1d->GetXMaximum();
    else
        return plot2d->GetXMaximum();
}

double qDrawPlot::GetYMinimum() {
    if (is1d)
        return plot1d->GetYMinimum();
    else
        return plot2d->GetYMinimum();
}

double qDrawPlot::GetYMaximum() {
    if (is1d)
        return plot1d->GetYMaximum();
    else
        return plot2d->GetYMaximum();
}

void qDrawPlot::SetDataCallBack(bool enable) {
    FILE_LOG(logINFO) << "Setting data call back to " << std::boolalpha << enable << std::noboolalpha;
    if (enable) {
        isPlot = true;
        det->registerDataCallback(&(GetDataCallBack), this); 
    } else {
        isPlot = false;
        det->registerDataCallback(nullptr, this);
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
    for (int i = 0; i < nHists; ++i) {
        SlsQtH1D* h = hists1d.at(i);
        h->setStyleLinesorDots(isLines);
    }
}

void qDrawPlot::SetMarkers(bool enable) { 
    std::lock_guard<std::mutex> lock(mPlots);
    FILE_LOG(logINFO) << "Setting Markers to " << std::boolalpha << enable << std::noboolalpha;
    isMarkers = enable;
    for (int i = 0; i < nHists; ++i) {
        SlsQtH1D* h = hists1d.at(i);
        h->setSymbolMarkers(isMarkers);
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
    
    SlsQt1DPlot* cloneplot1D = nullptr;
    SlsQt2DPlot* cloneplot2D = nullptr;
    SlsQt2DPlot* clonegainplot2D = nullptr;

    if (is1d) {
        FILE_LOG(logDEBUG) << "Cloning 1D Image";
        cloneplot1D = new SlsQt1DPlot();
        cloneplot1D->setFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
        cloneplot1D->SetTitleFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
        cloneplot1D->SetXFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
        cloneplot1D->SetYFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
        cloneplot1D->SetTitle(plot1d->title().text());
        cloneplot1D->SetXTitle(xTitle1d);
        cloneplot1D->SetYTitle(yTitle1d);
        QVector<SlsQtH1D *> cloneplotHists1D;
        for (int iHist = 0; iHist < nHists; ++iHist) {
            SlsQtH1D *h = new SlsQtH1D("", nPixelsX, datax1d, datay1d[iHist]);
            h->SetLineColor(iHist);
            h->setStyleLinesorDots(isLines);
            h->setSymbolMarkers(isMarkers);
            cloneplotHists1D.append(h);
            h->Attach(cloneplot1D);
        }
    } else  {
        FILE_LOG(logDEBUG) << "Cloning 2D Image";
        cloneplot2D = new SlsQt2DPlot();
        cloneplot2D->setFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
        cloneplot2D->SetTitleFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
        cloneplot2D->SetXFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
        cloneplot2D->SetYFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
        cloneplot2D->SetZFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));    
        cloneplot2D->setTitle(plot2d->title().text());
        cloneplot2D->SetXTitle(xTitle2d);
        cloneplot2D->SetYTitle(yTitle2d);
        cloneplot2D->SetZTitle(zTitle2d); 
        cloneplot2D->SetData(nPixelsX, -0.5, nPixelsX - 0.5, nPixelsY, -0.5, nPixelsY - 0.5, data2d);
        cloneplot2D->SetZRange(isZRange[0], isZRange[1], zRange[0], zRange[1]);

        if (isGainDataExtracted) {
            clonegainplot2D = new SlsQt2DPlot();
            clonegainplot2D->setFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
            clonegainplot2D->enableAxis(0, false);
            clonegainplot2D->enableAxis(1, false);
            clonegainplot2D->enableAxis(2, false);
            clonegainplot2D->SetData(nPixelsX, -0.5, nPixelsX - 0.5, nPixelsY, -0.5, nPixelsY - 0.5, gainData);
            clonegainplot2D->SetTitle(""); 
        }
    }

    qCloneWidget* q = new qCloneWidget(this, cloneplot1D, cloneplot2D, clonegainplot2D,
        boxPlot->title(), fileSavePath, fileSaveName, currentAcqIndex, 
        displayStatistics, lblMinDisp->text(), lblMaxDisp->text(), lblSumDisp->text());
    q->show(); 
}

void qDrawPlot::SavePlot() {
    // render image
    QImage savedImage(size().width(), size().height(), QImage::Format_RGB32);
    QPainter painter(&savedImage);
    render(&painter);

    QString fName = fileSavePath + QString('/') + 
        fileSaveName + 
        QString('_') + (is1d ? plot1d->title().text() : plot2d->title().text()) + 
        QString('_') +  QString("%1").arg(currentAcqIndex) + 
        QString(".png");

        fName = QFileDialog::getSaveFileName(0, tr("Save Image"), fName, tr("PNG Files (*.png);;XPM Files(*.xpm);;JPEG Files(*.jpg)"), 0,
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
    det->clearAcquiringFlag(); // (from previous exit) or if running

    // ensure data streaming in receiver (if plot enabled)
    if (isPlot) {
        try {
           if (!det->getRxZmqDataStream().squash(false)) {
               det->setRxZmqDataStream(true);
           }
        } CATCH_DISPLAY("Could not enable data streaming in Receiver.", "qDrawPlot::StartAcquisition");
    }

    // refixing all the zooming
    {
        std::lock_guard<std::mutex> lock(mPlots);
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
            det->stopAcquisition();
        } CATCH_DISPLAY("Could not stop acquisition and receiver.", "qDrawPlot::AcquireFinished");
        emit AbortSignal();
    }
    FILE_LOG(logDEBUG) << "End of Acquisition Finished";
}

std::string qDrawPlot::AcquireThread() {
    FILE_LOG(logDEBUG) << "Acquire Thread";
    try {
        det->acquire();
    } catch (const std::exception &e) {
        return std::string(e.what());
    }
    return std::string("");
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
    progress = currentProgress;
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
    currentAcqIndex = data->fileIndex;
    currentFrame =  frameIndex;
    FILE_LOG(logDEBUG) << "[ Progress:" << progress << ", Frame:" << currentFrame << " ]";

    // 2d (only image, not gain data, not pedestalvals), 
    // check if npixelsX and npixelsY is the same (quad is different)
    if (!is1d && (static_cast<int>(nPixelsX) != data->nx || static_cast<int>(nPixelsY) != data->ny)) {
        nPixelsX = data->nx;
        nPixelsY = data->ny;
        FILE_LOG(logINFO) << "Change in Detector Shape:\n\tnPixelsX:" << nPixelsX << " nPixelsY:" << nPixelsY;
        if (data2d)
            delete [] data2d;    
        data2d = new double[nPixelsY * nPixelsX];
        std::fill(data2d, data2d + nPixelsX * nPixelsY, 0);
    }

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
    
    if (isPedestal && pedestalCount <= NUM_PEDESTAL_FRAMES) {
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
            pedestalCount++;
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
        }
        // copy previous data
        for (int i = currentPersistency; i > 0; --i)
            memcpy(datay1d[i], datay1d[i - 1], nPixelsX * sizeof(double));
    }
    // pedestal
    if (isPedestal) {
        for (unsigned int px = 0; px < nPixelsX; ++px) {
            rawData[px] -= (pedestalVals[px]);
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
            rawData[px] -= (pedestalVals[px]);
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
    plot1d->SetTitle(indexTitle);
    plot1d->SetXTitle(xTitle1d);
    plot1d->SetYTitle(yTitle1d);
    for (int i = 0; i < nHists; ++i) {
        if (i < hists1d.size()) {
            SlsQtH1D* h = hists1d.at(i);
            h->SetData(nPixelsX, datax1d, datay1d[i]);
            h->Attach(plot1d);
        } else {
            SlsQtH1D* h = new SlsQtH1D("", nPixelsX, datax1d, datay1d[i]);
            h->SetLineColor(i);
            h->setStyleLinesorDots(isLines);
            h->setSymbolMarkers(isMarkers);
            hists1d.append(h);
            h->Attach(plot1d);
        }
    }
    if (xyRangeChanged) {
        Update1dXYRange();
        xyRangeChanged = false;
    }
    plot1d->DisableZoom(disableZoom);
}

void qDrawPlot::Update2dPlot() {
    plot2d->SetTitle(indexTitle);
    plot2d->SetXTitle(xTitle2d);
    plot2d->SetYTitle(yTitle2d);
    plot2d->SetZTitle(zTitle2d);
    plot2d->SetData(nPixelsX, -0.5, nPixelsX - 0.5, nPixelsY, -0.5, nPixelsY - 0.5, data2d);
     if (isGainDataExtracted) {
        gainplot2d->SetData(nPixelsX, -0.5, nPixelsX - 0.5, nPixelsY, -0.5, nPixelsY - 0.5, gainData);
        if (!gainplot2d->isVisible()) {
            gainplot2d->setFixedWidth(plot2d->width() / qDefs::DATA_GAIN_PLOT_RATIO);
            gainplot2d->setFixedHeight(plot2d->height() / qDefs::DATA_GAIN_PLOT_RATIO);
            gainplot2d->show();
          }
    } else if (gainplot2d->isVisible()) {
        gainplot2d->hide();  
    }
    if (xyRangeChanged) {
        Update2dXYRange();
        xyRangeChanged = false;
    }
    plot2d->DisableZoom(disableZoom);
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
        plot2d->EnableXAutoScaling();
    } else {
        double xmin = (isXYRange[qDefs::XMIN] ? xyRange[qDefs::XMIN] : plot2d->GetXMinimum());
        double xmax = (isXYRange[qDefs::XMAX] ? xyRange[qDefs::XMAX] : plot2d->GetXMaximum());
        plot2d->SetXMinMax(xmin, xmax);
    } 

    if (!isXYRange[qDefs::YMIN] && !isXYRange[qDefs::YMAX]) {
        plot2d->EnableYAutoScaling();
    } else {
        double ymin = (isXYRange[qDefs::YMIN] ? xyRange[qDefs::YMIN] : plot2d->GetYMinimum());
        double ymax = (isXYRange[qDefs::YMAX] ? xyRange[qDefs::YMAX] : plot2d->GetYMaximum());
        plot2d->SetYMinMax(ymin, ymax);
    } 
    plot2d->Update();
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
        Update1dPlot();
    } else {   
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
