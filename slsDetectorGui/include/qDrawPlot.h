// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "qDefs.h"
#include "sls/Detector.h"
#include "ui_form_plot.h"
#include <mutex>

class QResizeEvent;

namespace sls {

class SlsQt1DPlot;
class SlsQtH1D;
class SlsQt2DPlot;
class qCloneWidget;
class detectorData;

class qDrawPlot : public QWidget, private Ui::PlotObject {
    Q_OBJECT

  public:
    qDrawPlot(QWidget *parent, Detector *detector);
    ~qDrawPlot();
    bool GetIsRunning();
    void SetRunning(bool enable);
    double GetProgress();
    int64_t GetCurrentFrameIndex();
    void Select1dPlot(bool enable);
    void SetPlotTitlePrefix(QString title);
    void SetXAxisTitle(QString title);
    void SetYAxisTitle(QString title);
    void SetZAxisTitle(QString title);
    void SetXYRangeChanged(bool disable, double *xy, bool *isXY);
    void SetZRange(double *z, bool *isZ);
    double GetXMinimum();
    double GetXMaximum();
    double GetYMinimum();
    double GetYMaximum();
    void SetDataCallBack(bool enable);
    void SetBinary(bool enable, int from = 0, int to = 0);
    void StartAcquisition();

  public slots:
    void SetPersistency(int val);
    void SetLines(bool enable);
    void SetMarkers(bool enable);
    void Set1dLogY(bool enable);
    void SetInterpolate(bool enable);
    void SetContour(bool enable);
    void SetLogz(bool enable);
    void SetPedestal(bool enable);
    void RecalculatePedestal();
    void SetAccumulate(bool enable);
    void ResetAccumulate();
    void DisplayStatistics(bool enable);
    void SetNumDiscardBits(int value);
    void EnableGainPlot(bool enable);
    void ClonePlot();
    void SavePlot();
    void SetGapPixels(bool enable);
    void UpdatePlot();

  protected:
    void resizeEvent(QResizeEvent *event);

  private slots:
    void Zoom1DGainPlot(const QRectF &rect);
    void Zoom2DGainPlot(const QRectF &rect);
    void SetSaveFileName(QString val);

  signals:
    void AcquireFinishedSignal();
    void AbortSignal(QString);
    void UpdateSignal();

  private:
    void SetupWidgetWindow();
    void Initialization();
    void SetupPlots();
    void GetStatistics(double &min, double &max, double &sum);
    void DetachHists();
    void AcquireThread();
    static void GetAcquisitionFinishedCallBack(double currentProgress,
                                               int detectorStatus,
                                               void *this_pointer);
    static void GetDataCallBack(detectorData *data, uint64_t frameIndex,
                                uint32_t subFrameIndex, void *this_pointer);
    void AcquisitionFinished(double currentProgress, int detectorStatus);
    void GetData(detectorData *data, uint64_t frameIndex,
                 uint32_t subFrameIndex);
    void toDoublePixelData(double *dest, char *source, int size, int databytes,
                           int dr, double *gaindest = NULL);
    void Get1dData(double *rawData);
    void Get2dData(double *rawData);
    void Update1dPlot();
    void Update2dPlot();
    void Update1dXYRange();
    void Update2dXYRange();
    void rearrangeGotthard25data(double *data);

    static const int NUM_PEDESTAL_FRAMES = 20;
    static const int NUM_GOTTHARD25_CHANS = 1280;
    Detector *det;
    slsDetectorDefs::detectorType detType;

    SlsQt1DPlot *plot1d{nullptr};
    QVector<SlsQtH1D *> hists1d;
    SlsQt1DPlot *gainplot1d{nullptr};
    SlsQtH1D *gainhist1d{nullptr};
    SlsQt2DPlot *plot2d{nullptr};
    SlsQt2DPlot *gainplot2d{nullptr};

    bool is1d{true};
    bool isRunning{false};

    // titles
    QString plotTitlePrefix{""};
    QString xTitle1d{"Channel Number"};
    QString yTitle1d{"Counts"};
    QString xTitle2d{"Pixel"};
    QString yTitle2d{"Pixel"};
    QString zTitle2d{"Intensity"};
    QString plotTitle{""};
    QString indexTitle{""};
    bool completeImage{true};
    bool xyRangeChanged{false};
    double xyRange[4]{0, 0, 0, 0};
    bool isXYRange[4]{false, false, false, false};
    double zRange[2]{0, 1};
    bool isZRange[2]{false, false};

    // data
    int nHists{1};
    double *datax1d{nullptr};
    std::vector<double *> datay1d;
    double *gainDatay1d{nullptr};
    double *data2d{nullptr};
    double *gainData{nullptr};

    // options
    bool isPlot{true};
    bool isBinary{false};
    int binaryFrom{0};
    int binaryTo{0};
    int persistency{0};
    int currentPersistency{0};
    bool isLines{true};
    bool isMarkers{false};
    bool isPedestal{false};
    double *pedestalVals{nullptr};
    double *tempPedestalVals{nullptr};
    int pedestalCount{0};
    bool resetPedestal{false};
    bool isAccumulate{false};
    bool resetAccumulate{false};
    bool displayStatistics{false};
    QString fileSavePath{"/tmp"};
    QString fileSaveName{"Image"};
    bool hasGainData{false};
    bool isGainDataExtracted{false};
    bool disableZoom{false};
    int numDiscardBits{0};

    double progress{0};
    int64_t currentFrame{0};
    mutable std::mutex mPlots;
    int64_t currentAcqIndex{0};
    slsDetectorDefs::ROI rxRoi{};
    bool isRxRoiDisplayed{false};
    bool isGapPixels{false};

    unsigned int nPixelsX{0};
    unsigned int nPixelsY{0};
    uint32_t pixelMask{0};
    uint32_t gainMask{0};
    int gainOffset{0};
    bool gotthard25;
};

} // namespace sls
