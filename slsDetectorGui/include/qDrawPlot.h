#pragma once

#include "qDefs.h"
class detectorData;
class SlsQt1DPlot;
class SlsQtH1D;
class SlsQt2DPlotLayout;
class qCloneWidget;

/*
#include "qwt_symbol.h"
#include <QGridLayout>
#include <QGroupBox>
#include <QString>
#include <QTimer>
#include <QWidget>

#include <QVector>
#include <qpen.h>
#include <qwt_column_symbol.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_histogram.h>
#include <qwt_series_data.h>
*/

class qDrawPlot : public QWidget {
    Q_OBJECT

  public:
    /** \short The constructor	 */
    qDrawPlot(QWidget *parent, multiSlsDetector *detector);
    /** Destructor	 */
    ~qDrawPlot();

    bool isRunning();
    // from measurement tabs
    int GetProgress();
    int64_t GetCurrentFrameIndex();
    int64_t GetCurrentMeasurementIndex();
    // from plot tab
    void Select1dPlot(bool enable);
    void SetPlotTitlePrefix(QString title);
    void SetXAxisTitle(QString title);
    void SetYAxisTitle(QString title);
    void SetZAxisTitle(QString title);
    void DisableZoom(bool disable);
    void SetXYRangeChanged();
    void SetXYRangeValues(double val, qDefs::range xy);
    void IsXYRangeValues(bool changed, qDefs::range xy);
    double GetXMinimum();
    double GetXMaximum();
    double GetYMinimum();
    double GetYMaximum();
    void SetZRange(bool isZmin, bool isZmax, double zmin, double zmax);
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

  
    void UpdatePlot();

  signals:
    void UpdatingPlotFinished();
    void UpdatePlotSignal();

  private:
    void SetupWidgetWindow();
    void Initialization();
	  void SetupStatistics();
	  void SetupPlots();
    int LockLastImageArray();
    int UnlockLastImageArray();
	  void SetStyle(SlsQtH1D *h);
    void GetStatistics(double &min, double &max, double &sum, double *array, int size);
    void DetachHists();
    void UpdateXYRange();

 



    bool StartOrStopThread(bool start);
    void SetupMeasurement();
    static void *DataStartAcquireThread(void *this_pointer);
    static int GetDataCallBack(detectorData *data, int fIndex, int subIndex, void *this_pointer);
    int GetData(detectorData *data, int fIndex, int subIndex);
    static int GetAcquisitionFinishedCallBack(double currentProgress, int detectorStatus, void *this_pointer);
    int AcquisitionFinished(double currentProgress, int detectorStatus);
    static int GetMeasurementFinishedCallBack(int currentMeasurementIndex, int fileIndex, void *this_pointer);
    int MeasurementFinished(int currentMeasurementIndex, int fileIndex);
    static int GetProgressCallBack(double currentProgress, void *this_pointer);
    void toDoublePixelData(double *dest, char *source, int size, int databytes, int dr, double *gaindest = NULL);


  	static const int NUM_PEDESTAL_FRAMES = 20;
    multiSlsDetector *myDet;
    slsDetectorDefs::detectorType detType;

    SlsQt1DPlot *plot1d{nullptr};
	  QVector<SlsQtH1D *> hists1d;
    SlsQt2DPlotLayout *plot2d{nullptr};
    SlsQt2DPlotLayout *gainplot2d{nullptr};

    QGridLayout *layout{nullptr};
    QGroupBox *boxPlot{nullptr};
    QGridLayout *plotLayout{nullptr};

    bool is1d{true};
    bool isRunning{false};
    int progress{0};

	// titles
	  QString plotTitle{""};
    QString plotTitle_prefix{""};
    QLabel *lblFrameIndexTitle1d{nullptr};
    std::vector<std::string> title1d;
	  std::string title2d{""};
    QString xTitle1d{"Channel Number"};
    QString yTitle1d{"Counts"};
	  QString xTitle2d{"Pixel"};
    QString yTitle2d{"Pixel"};
    QString zTitle2d{"Intensity"};
    bool XYRangeChanged{false};
    double XYRange[4]{0, 0, 0, 0};
    bool isXYRange[4]{false, false, false, false};

	// data
    unsigned int nHists{1};
    double *datax1d{nullptr};
    std::vector<double *> datay1d;
    double *data2d{nullptr};

	//options
	bool binary{false};
    int binaryFrom{0};
    int binaryTo{0};
    int persistency{0};
    int currentPersistency0};
    bool isLines{true};
    bool isMarkers{false};
    QwtSymbol *marker{nullptr};
    QwtSymbol *noMarker{nullptr};
    bool pedestal{false};
    double *pedestalVals{nullptr};
    double *tempPedestalVals{nullptr};
    int pedestalCount{0};
    bool startPedestalCal{false};
    bool accumulate{false};
    bool resetAccumulate{false};
    QWidget *widgetStatistics{nullptr};
    QLabel *lblMinDisp{nullptr};
    QLabel *lblMaxDisp{nullptr};
    QLabel *lblSumDisp{nullptr};
    bool displayStatistics{false};
    std::vector<qCloneWidget *> cloneWidgets;
    QString fileSavePath{"/tmp"};
    QString fileSaveName{"Image"};
    double *gainImage{nullptr};
    bool gainDataExtracted{false};
    bool gainDataEnable{false};

    unsigned int nPixelsX{0};
    unsigned int nPixelsY{0};
    double minPixelsY{0};
    double maxPixelsY{0};
    double startPixel{0};
    double endPixel{0};
    double pixelWidth{0};

    int64_t currentMeasurement{0};
    int64_t currentFrame{0};
    int64_t lastImageNumber{0};
	  pthread_mutex_t lastImageCompleteMutex;

    const static int npixelsx_jctb = 400;
    int npixelsy_jctb{0};
};
