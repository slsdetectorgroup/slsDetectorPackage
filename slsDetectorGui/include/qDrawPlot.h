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

    /** If the gui client has started or stopped */
    void SetClientInitiated();
    bool GetClientInitiated();
    bool isRunning();
    int GetProgress();
    int GetCurrentFrameIndex();
    void SetPlotTitlePrefix(QString title);
    void SetXAxisTitle(QString title);
    void SetYAxisTitle(QString title);
    void SetZAxisTitle(QString title);
    void DisableZoom(bool disable);
    void SetXYRange(bool changed);
    void SetXYRangeValues(double val, qDefs::range xy);
    void IsXYRangeValues(bool changed, qDefs::range xy);
    double GetXMinimum();
    double GetXMaximum();
    double GetYMinimum();
    double GetYMaximum();
    void SetDataCallBack(bool enable);
    void SetBinary(bool enable, int from = 0, int to = 0);

    /** Starts or stop acquisition
     * Calls startDaq() function
     * @param stop_if_running is 0 to stop acquisition and 1 to start
     * acquisition
     */
    void StartStopDaqToggle(bool stop_if_running = 0);

  public slots:
    void StopAcquisition();
    void SetPersistency(int val);
    void SetLines(bool enable);
    void SetMarkers(bool enable);
    void SetPedestal(bool enable);
    void RecalculatePedestal();
    void SetAccumulate(bool enable);
    void ResetAccumulate();
    void DisplayStatistics(bool enable);
    void ClonePlot();
    void CloseClones();
    void SaveClones();
	void SavePlot();
    void Select1DPlot();
    void Select2DPlot();

  private slots:
    void CloneCloseEvent(int id);
  	void EnableGainPlot(bool e);
  
    void UpdatePlot();
    void StartDaq(bool start);
    void ShowAcquisitionErrorMessage(QString status);

  signals:
    void UpdatingPlotFinished();
    void InterpolateSignal(bool);
    void ContourSignal(bool);
    void LogzSignal(bool);
    void LogySignal(bool);
    void ResetZMinZMaxSignal(bool, bool, double, double);
    void SetCurrentMeasurementSignal(int);
    void AcquisitionErrorSignal(QString);
    void UpdatePlotSignal();
    void GainPlotSignal(bool);

  private:
    void SetupWidgetWindow();
    void Initialization();
	void SetupStatistics();
	void SetupPlots();
	const char *GetTitle1d(int histIndex);
    double *GetHistYAxis(int histIndex);
	void SetStyle(SlsQtH1D *h);
    void GetStatistics(double &min, double &max, double &sum, double *array, int size);
    void SelectPlot(int i = 2);
    void Clear1DPlot();


    int LockLastImageArray();
    int UnlockLastImageArray();
    int StartDaqForGui();
    int StopDaqForGui();
    bool StartOrStopThread(bool start);
    void SetupMeasurement();
    int ResetDaqForGui();
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
    bool plotEnable{true};
    bool plotRequired{false};/**?? */
    bool running{false};
    int progress{0};
    volatile bool stopSignal{false};
	bool clientInitiated{false};

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
    double XYRangeValues[4]{0, 0, 0, 0};
    bool isXYRangeEnable[4]{false, false, false, false};

	// data
    unsigned int nHists{0};
    int histNBins{0};
    double *x1d{nullptr};
    std::vector<double *> y1d;
    double *image2d{nullptr};

	//options
	bool binary{false};
    int binaryFrom{0};
    int binaryTo{0};
    int persistency0};
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

    int currentMeasurement{0};
    int currentFrame{0};
    int currentFileIndex{0};
    int currentFrameIndex{0};
	pthread_mutex_t lastImageCompleteMutex;
    unsigned int lastImageNumber{0};
    int numberofFrames{0};
    double acquisitionPeriod{0};
    double exposureTime{0};
 
    /** prevents err msg displaying twice when detector stopped, "transmitting"
     */
    bool alreadyDisplayed{false};

    const static int npixelsx_jctb = 400;
    int npixelsy_jctb{0};
};
