#pragma once

#include "qDefs.h"
class detectorData;
class SlsQt1DPlot;
class SlsQtH1D;
class SlsQt2DPlotLayout;
class qCloneWidget;

class QGridLayout;
class QGroupBox;
class QwtSymbol;
#include <QFutureWatcher>

class qDrawPlot : public QWidget {
    Q_OBJECT

  public:
     qDrawPlot(QWidget *parent, multiSlsDetector *detector);
    ~qDrawPlot();
    bool GetIsRunning();
    void SetRunning(bool enable);
    int GetProgress();
    int64_t GetCurrentFrameIndex();
    void Select1dPlot(bool enable);
    void SetPlotTitlePrefix(QString title);
    void SetXAxisTitle(QString title);
    void SetYAxisTitle(QString title);
    void SetZAxisTitle(QString title);
    void SetXYRangeChanged(bool disable, double* xy, bool* isXY);
    void SetZRange(double* z, bool* isZ);
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
    void EnableGainPlot(bool enable);
    void ClonePlot();
    void CloseClones();
    void SaveClones();
	  void SavePlot();

  private slots:
    void SetSaveFileName(QString val);
    void CloneCloseEvent(int id);
    void AcquireFinished();
    void UpdatePlot();
  

  signals:
    void AcquireFinishedSignal();
    void AbortSignal();
    void UpdateSignal();

  private:
    void SetupWidgetWindow();
    void Initialization();
	  void SetupStatistics();
	  void SetupPlots();
    int LockLastImageArray();
    int UnlockLastImageArray();
	  void SetStyleandSymbol(SlsQtH1D *h);
    void GetStatistics(double &min, double &max, double &sum);
    void DetachHists();
    static void GetProgressCallBack(double currentProgress, void *this_pointer);
    static void GetAcquisitionFinishedCallBack(double currentProgress, int detectorStatus, void *this_pointer);
    static void GetDataCallBack(detectorData *data, uint64_t frameIndex, uint32_t subFrameIndex, void *this_pointer);
    std::string AcquireThread();
    void AcquisitionFinished(double currentProgress, int detectorStatus);
    void GetData(detectorData *data, uint64_t frameIndex, uint32_t subFrameIndex);
    void toDoublePixelData(double *dest, char *source, int size, int databytes, int dr, double *gaindest = NULL);
    void Get1dData(double* rawData);
    void Get2dData(double* rawData);
    void Update1dPlot();
    void Update2dPlot();
    void Update1dXYRange();
    void Update2dXYRange();
    
  	static const int NUM_PEDESTAL_FRAMES = 20;
    multiSlsDetector *myDet;
    slsDetectorDefs::detectorType detType;

    SlsQt1DPlot *plot1d{nullptr};
	  QVector<SlsQtH1D *> hists1d;
    SlsQt2DPlotLayout *plot2d{nullptr};
    SlsQt2DPlotLayout *gainplot2d{nullptr};
    QFutureWatcher<std::string> *acqResultWatcher;

    QGridLayout *layout{nullptr};
    QGroupBox *boxPlot{nullptr};
    QGridLayout *plotLayout{nullptr};

    bool is1d{true};
    bool isRunning{false};
   
	// titles
    QString plotTitlePrefix{""};
    QLabel *lblFrameIndexTitle1d{nullptr};
    QString xTitle1d{"Channel Number"};
    QString yTitle1d{"Counts"};
	  QString xTitle2d{"Pixel"};
    QString yTitle2d{"Pixel"};
    QString zTitle2d{"Intensity"};
    QString plotTitle{""};
    QString indexTitle{""};
    bool xyRangeChanged{false};
    double xyRange[4]{0, 0, 0, 0};
    bool isXYRange[4]{false, false, false, false};
    double zRange[2]{0, 1};
    bool isZRange[2]{false, false};

	// data
    unsigned int nHists{1};
    double *datax1d{nullptr};
    std::vector<double *> datay1d;
    double *data2d{nullptr};
    double *gainData{nullptr};

	//options
    bool isPlot{true};
	  bool isBinary{false};
    int binaryFrom{0};
    int binaryTo{0};
    int persistency{0};
    int currentPersistency{0};
    bool isLines{true};
    bool isMarkers{false};
    QwtSymbol *marker{nullptr};
    QwtSymbol *noMarker{nullptr};
    bool isPedestal{false};
    double *pedestalVals{nullptr};
    double *tempPedestalVals{nullptr};
    int pedestalCount{0};
    bool resetPedestal{false};
    bool isAccumulate{false};
    bool resetAccumulate{false};
    QWidget *widgetStatistics{nullptr};
    QLabel *lblMinDisp{nullptr};
    QLabel *lblMaxDisp{nullptr};
    QLabel *lblSumDisp{nullptr};
    bool displayStatistics{false};
    std::vector<qCloneWidget *> cloneWidgets;
    QString fileSavePath{"/tmp"};
    QString fileSaveName{"Image"};
    bool hasGainData{false};
    bool isGainDataExtracted{false};
    bool disableZoom{false};

    int progress{0};
    int64_t currentFrame{0};
    mutable std::mutex mPlots;
	  pthread_mutex_t lastImageCompleteMutex;

    unsigned int nPixelsX{0};
    unsigned int nPixelsY{0};
    const static int npixelsx_jctb = 400;
    int npixelsy_jctb{0};
};
