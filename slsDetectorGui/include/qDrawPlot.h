/*
 * qDrawPlot.h
 *
 *  Created on: May 7, 2012
 *      Author: Ian Johnson
 */
#ifndef QDRAWPLOT_H
#define QDRAWPLOT_H


/** Project Class Headers */
class multiSlsDetector;
#include "detectorData.h"
/** Qt Project Class Headers */
#include "SlsQt1DPlot.h"
#include "SlsQt2DPlotLayout.h"
#include "qDefs.h"
class SlsQt1DPlot;
class SlsQt2DPlotLayout;
class qCloneWidget;
/** Qt Include Headers */
#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QTimer>
#include <QString>
#include "qwt_symbol.h"

/** C++ Include Headers */


#define MAX_1DPLOTS 10


/**
 *@short Sets up the plot widget
 */
class qDrawPlot:public QWidget{
	Q_OBJECT




public:
	/** \short The constructor	 */
	qDrawPlot(QWidget *parent,multiSlsDetector*& detector);
	/** Destructor	 */
	~qDrawPlot();

	/**is an acquisition running , need it to prevent measurement tab
	 * from being refreshed when switching tabs during acquisition */
	bool isRunning(){return running;};
	/** gets the progress of acquisition to the measurement tab*/
	int GetProgress(){return progress;};

	/**	sets plot Title */
	void SetPlotTitle(QString title)      	{boxPlot->setTitle(title);}
	/**	sets 1D X Axis Title */
	void SetHistXAxisTitle(QString title)   	{histXAxisTitle = title;}
	/**	sets 1D Y Axis Title */
	void SetHistYAxisTitle(QString title)   	{histYAxisTitle = title;}
	/**	sets 2D X Axis Title */
	void SetImageXAxisTitle(QString title)   {imageXAxisTitle = title;}
	/**	sets 2D Y Axis Title */
	void SetImageYAxisTitle(QString title)   {imageYAxisTitle = title;}
	/**	sets 2D Z Axis Title */
	void SetImageZAxisTitle(QString title)   {imageZAxisTitle = title;}
	/** Disables zoom if any of the axes range are checked and fixed with a value */
	void DisableZoom(bool disable);
	/**	Enables plot from the plot tab*/
	void EnablePlot(bool enable);
	/** Enable angle plot */
	void EnableAnglePlot(bool enable){anglePlot = enable;};

	/** Its a reminder to update plot to set the xy range
	 * This is done only when there is a plot to update */
	void SetXYRange(bool changed){XYRangeChanged = changed;};
	/**Sets the min/max for x/y
	 * @param val is the value to be set
	 * @param xy is xmin,xmax,ymin or ymax */
	void SetXYRangeValues(double val,qDefs::range xy){XYRangeValues[xy]=val;};
	/**Sets if min/max for x/y is enabled
	 * @param changed is if this has been changed
	 * @param xy is xmin,xmax,ymin or ymax */
	void IsXYRangeValues(bool changed,qDefs::range xy){IsXYRange[xy]=changed;};

	/** Get minimum Plot timer - between plots */
	double GetMinimumPlotTimer(){return PLOT_TIMER_MS;};
	/** Set Plot timer - between plots in ms*/
	void SetPlotTimer(double time){timerValue = time;};
	/** Set  Plot frame factor - between plots */
	void SetFrameFactor(int frame){frameFactor = frame;};

	/** Starts or stop acquisition
	 * Calls startDaq() function
	 * @param stop_if_running is 0 to stop acquisition and 1 to start acquisition
	 */
	void StartStopDaqToggle(bool stop_if_running=0);
	/** Set number of measurements
	 *  @param num number of measurements to be set */
	void setNumMeasurements(int num){number_of_measurements = num;};
	/** Set frame enabled
	 *  @param enable enable*/
	void setFrameEnabled(bool enable){isFrameEnabled = enable;};
	/** Set trigger enabled
	 *  @param enable enable */
	void setTriggerEnabled(bool enable){isTriggerEnabled = enable;};





public slots:
/** To select 1D or 2D plot
 @param i is 1 for 1D, else 2D plot */
void SelectPlot(int i=2);
/** To select 1D plot */
void Select1DPlot() {SelectPlot(1);}
/** To select 2D plot */
void Select2DPlot() {SelectPlot(2);}
/** To clear plot */
void Clear1DPlot();
/** Creates a clone of the plot */
void ClonePlot();
/** Closes all the clone plots */
void CloseClones();
/** Saves all the clone plots */
void SaveClones();
/** To Save plot */
void SavePlot();
/** Save all plots **/
void SaveAll(bool enable);
/**	Sets persistency from plot tab */
void SetPersistency(int val);
/**	sets style of plot to lines*/
void SetLines(bool enable){lines = enable;};
/**	sets markers */
void SetMarkers(bool enable){markers = enable;};
/** sets the scan argument to prepare the plot*/
void SetScanArgument(int scanArg){scanArgument = scanArg;};







private:
/** Initializes all its members and the thread */
void Initialization();
/** Sets up the widget */
void SetupWidgetWindow();


/** Gets the image title	 */
const char*  GetImageTitle()      	{return imageTitle.c_str();}
/**	Gets the hist title for a 1D plot */
const char*  GetHistTitle(int i)  	{return (i>=0&&i<MAX_1DPLOTS) ? histTitle[i].c_str():0;} //int for hist number
/**	Gets the y axis value for the hist in 1D plot */
double*      GetHistYAxis(int i)  	{return (i>=0&&i<MAX_1DPLOTS) ? histYAxis[i]:0;} //int for hist number


/**	Locks the image to update plot */
int    LockLastImageArray()			{return pthread_mutex_lock(&last_image_complete_mutex); }
/**	Unocks the image to update plot */
int    UnlockLastImageArray()		{return pthread_mutex_unlock(&last_image_complete_mutex);}
/**	Starts the acquisition */
int    StartDaqForGui() 		  	{return StartOrStopThread(1) ? 1:0;}
/**	Stops the acquisition  */
int    StopDaqForGui() 			  	{return StartOrStopThread(0) ? 0:1;}
/** Starts/stops Acquisition Thread */
bool   StartOrStopThread(bool start);
/**	Resets the acquisition parameter like lastimagenumber */
int    ResetDaqForGui();
/**	The function which is called when start acquisition thread is created */
static void* DataStartAcquireThread(void *this_pointer);
/**	This is called by the detector class to copy the data it jus acquired */
static int GetDataCallBack(detectorData *data, void *this_pointer);
/**	This is called by the GetDataCallBack function to copy the data */
int GetData(detectorData *data);
/** Saves all the plots. All sets saveError to true if not saved.*/
void SavePlotAutomatic();

void SetStyle(SlsQtH1D*  h){
	if(lines) h->setStyle(QwtPlotCurve::Lines); else h->setStyle(QwtPlotCurve::Dots);
	if(markers) h->setSymbol(*marker); 			else h->setSymbol(*noMarker);};





private slots:
/** To update plot */
void UpdatePlot();
/** To stop updating plot */
void StopUpdatePlot();
/** To start or stop acquisition
 * @param start is 1 to start and 0 to stop acquisition */
void StartDaq(bool start);
/** To set the reference to zero after closing a clone
 * @param id is the id of the clone */
void CloneCloseEvent(int id);
/**After a pause, the gui is allowed to collect the data
 * this is called when it is over */
void UpdatePause(){data_pause_over=true;};
/** Shows the first save error message while automatic saving
 * @param fileName file name of the first file that it tried to save.*/
void ShowSaveErrorMessage(QString fileName);




private:
/** The sls detector object */
multiSlsDetector *myDet;



/** Widgets needed to plot the clone */
/**	Max Number of Clone Windows */
static const int MAXCloneWindows = 50;
/**	Array of clone window widget pointers */
qCloneWidget		*winClone[MAXCloneWindows];

/** Widgets needed to set up plot*/
QGroupBox 			*boxPlot;
QGridLayout 		*layout;
QGridLayout 		*plotLayout;
/**	Timer to update plot */
QTimer* 			plot_update_timer;

/** Timer to pause getting data from client */
QTimer*				data_pause_timer;
bool 				data_pause_over;


/**	1D object */
SlsQt1DPlot* 		plot1D;
/**	2D object */
SlsQt2DPlotLayout* 	plot2D;
/**	vector of 1D hist values */
QVector<SlsQtH1D*> 	plot1D_hists;


/** Number of Measurements */
int number_of_measurements;
/** Current Measurement */
int currentMeasurement;
/** currentFrame */
int currentFrame;
/** current Index */
int currentIndex;
/** current Scan Division Level */
int currentScanDivLevel;
/** current scan Value */
double currentScanValue;
/**	 Number of Exposures */
int number_of_exposures;
/** Number of Frames Per Measurement */
int number_of_frames;
/**	 Duration between Exposures */
double acquisitionPeriod;
/**	 Acquisition Time */
double exposureTime;



/**variables for threads */
/**	 */
volatile bool   stop_signal;
/**	 */
pthread_mutex_t last_image_complete_mutex;

/**variables for histograms */
/**	X Axis Title in 2D */
QString  imageXAxisTitle;
/** Y Axis Title in 2D */
QString  imageYAxisTitle;
/**	Z Axis Title in 2D */
QString  imageZAxisTitle;
/**	X Axis Title in 1D */
QString  histXAxisTitle;
/**	Y Axis Title in 1D */
QString  histYAxisTitle;
/**	Title for all the graphs in 1D */
std::string  histTitle[MAX_1DPLOTS];
/**	Title in 2D */
std::string  imageTitle;
/**	1D or 2D */
unsigned int plot_in_scope;
/**	Number of Pixels in X Axis */
unsigned int nPixelsX;
/** Number of angle Pixels in X Axis */
int nAnglePixelsX;
/**	Number of pixel bins in Y Axis */
int nPixelsY;
/** Min Pixel number for Y Axis*/
double minPixelsY;
/** Max Pixel number for Y Axis*/
double maxPixelsY;
/** starting pixel */
double startPixel;
/** end Pixel*/
double endPixel;
/** pixel width */
double pixelWidth;

/**	Current Image Number */
unsigned int lastImageNumber;
int last_plot_number;

/**	Number of graphs in 1D */
unsigned int nHists;
/**	Total Number of X axis values/channels in 1D */
int          histNBins;
/**	X Axis value in 1D */
double*      histXAxis;
/** Y Axis value in 1D  */
double*      histYAxis[MAX_1DPLOTS];
/** X Axis for angles in 1D */
double*	     histXAngleAxis;
/** Y Axis for angles in 1D (no persistency) */
double*	     histYAngleAxis;
/**	Current Image Values in 2D */
double*      lastImageArray;
/**	temporary Y Axis value in 1D */
double* 	 yvalues[MAX_1DPLOTS];
/**	temporary Image Values in 2D */
double* 	 image_data;

/**persistency to be reached*/
int persistency;
/** persistency takes time to reach as it increases per frame
 * this is the current one */
int currentPersistency;
/** to update the progress for each getData() so that
 * measurement tab can request on a timer basis*/
int progress;
/**If plot is enabled from plot tab*/
bool plotEnable;
/**If plot is dotted */
bool lines;
bool markers;
/** Plot marker */
QwtSymbol *marker;
QwtSymbol *noMarker;
/** Save all plots */
bool saveAll;
/** If error, while automatically saving plots, checks this at the end of an acquistion */
bool saveError;
/** index of last saved image for automatic saving*/
int lastSavedFrame;
/** index of measurement number of last saved image for automatic saving*/
int lastSavedMeasurement;
/**if an acquisition is running, so as not to refresh tab
 * and also to update plot only if running (while creating clones)*/
bool running;

/** if the min/max of x and y has been changed,
 * to notify while plotting */
bool XYRangeChanged;
/**the specific min/max of x/y*/
double XYRangeValues[4];
/**if the specific min/max of x/y is enabled */
bool IsXYRange[4];

/** Default timer between plots*/
static const double PLOT_TIMER_MS = 250;
/** Specific timer value between plots */
double timerValue;
/** every nth frame when to plot */
int frameFactor;
/**if frame is enabled in measurement tab */
bool isFrameEnabled;
/**if trigger is enabled in measurement tab */
bool isTriggerEnabled;

/** scan arguments*/
enum scanArgumentList{None,Level0,Level1,FileIndex,AllFrames};
int scanArgument;

/** enable angle plot */
bool anglePlot;



signals:
void UpdatingPlotFinished();
void InterpolateSignal(bool);
void ContourSignal(bool);
void LogzSignal(bool);
void SetZRangeSignal(double,double);
void EnableZRangeSignal(bool);
void SetCurrentMeasurementSignal(int);
void saveErrorSignal(QString);
};



#endif /* QDRAWPLOT_H */
