/*
 * qDrawPlot.h
 *
 *  Created on: May 7, 2012
 *      Author: Ian Johnson
 */
#ifndef QDRAWPLOT_H
#define QDRAWPLOT_H


/** Project Class Headers */
class slsDetectorUtils;
/** Qt Project Class Headers */
class SlsQtH1D;
class SlsQt1DPlot;
class SlsQt2DPlotLayout;
class qCloneWidget;
/** Qt Include Headers */

#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QTimer>


/**
 *@short Sets up the plot widget
 */
class qDrawPlot:public QWidget{
	Q_OBJECT

public:
	/** \short The constructor
	 */
	qDrawPlot(QWidget *parent,slsDetectorUtils*& detector);

	/** Destructor
	 */
	~qDrawPlot();

	/** Starts or stop acquisition
	 * Calls startDaq() function
	 * @param stop_if_running is 0 to stop acquisition and 1 to start acquisition
	 */
	void StartStopDaqToggle(bool stop_if_running=0);

private:
	/** The sls detector object */
	slsDetectorUtils *myDet;

	/** Number of Measurements */
	int numberOfMeasurements;
	/**	 Number of Exposures */
	int number_of_exposures;
	/**	 Duration between Exposures */
	double framePeriod;
	/**	 Acquisition Time */
	double acquisitionTime;

/** Widgets needed to plot the clone */
	/**	Max Number of Clone Windows */
	static const int MAXCloneWindows = 50;
	/**	Array of clone window widget pointers */
	qCloneWidget			*winClone[MAXCloneWindows];

/** Widgets needed to set up plot*/
	QGroupBox 			*boxPlot;
	QGridLayout 		*layout;
	QGridLayout 		*plotLayout;
	/**	Timer to update plot */
	QTimer* 			plot_update_timer;
	/**	1D object */
	SlsQt1DPlot* 		plot1D;
	/**	2D object */
	SlsQt2DPlotLayout* 	plot2D;
	/**	1D hist values */
	QVector<SlsQtH1D*> 	plot1D_hists;


/**variables for threads */
	/**	 */
	volatile bool   stop_signal;
	/**	 */
	pthread_mutex_t last_image_complete_mutex;

/**variables for histograms */
	/**	1D or 2D */
	unsigned int plot_in_scope;
	/**	Current Image Number */
	unsigned int lastImageNumber;
	/**	Title in 2D */
	std::string  imageTitle;
	/**	X Axis Title in 2D */
	std::string  imageXAxisTitle;
	/** Y Axis Title in 2D */
	std::string  imageYAxisTitle;
	/**	Z Axis Title in 2D */
	std::string  imageZAxisTitle;
	/**	Number of Pixels in X Axis */
	unsigned int nPixelsX;
	/**	Number of Pixels in Y Axis */
	unsigned int nPixelsY;
	/**	Current Image Values in 1D */
	double*      lastImageArray;
	/**	Number of graphs in 1D */
	unsigned int nHists;
	/**	Title for all the graphs in 1D */
	std::string  histTitle[10];
	/**	X Axis Title in 1D */
	std::string  histXAxisTitle;
	/**	Y Axis Title in 1D */
	std::string  histYAxisTitle;
	/**	Total Number of X axis values/channels in 1D */
	int          histNBins;
	/**	X Axis value in 1D */
	double*      histXAxis;
	/** Y Axis value in 1D  */
	double*      histYAxis[10];


	/**	 */
	int    LockLastImageArray()			{return pthread_mutex_lock(&last_image_complete_mutex); }
	/**	 */
	int    UnlockLastImageArray()		{return pthread_mutex_unlock(&last_image_complete_mutex);}
	/**	 */
	SlsQt1DPlot*       Get1DPlotPtr() 	{return plot1D;}
	/**	 */
	SlsQt2DPlotLayout* Get2DPlotPtr() 	{return plot2D;}
	/**	 */
	int    StartDaqForGui() 		  	{return StartOrStopThread(1) ? 1:0;}
	/**	 */
	int    StopDaqForGui() 			  	{return StartOrStopThread(0) ? 0:1;}
	/**	 */
	unsigned int PlotInScope()        	{return plot_in_scope;}
	/**	 */
	unsigned int GetLastImageNumber() 	{return lastImageNumber;}
	/**	 */
	const char*  GetImageTitle()      	{return imageTitle.c_str();}
	/**	 */
	const char*  GetImageXAxisTitle() 	{return imageXAxisTitle.c_str();}
	/**	 */
	const char*  GetImageYAxisTitle() 	{return imageYAxisTitle.c_str();}
	/**	 */
	const char*  GetImageZAxisTitle() 	{return imageZAxisTitle.c_str();}
	/**	 */
	unsigned int GetNPixelsX()        	{return nPixelsX;}
	/**	 */
	unsigned int GetNPixelsY()        	{return nPixelsY;}
	/**	 */
	double*      GetLastImageArray()  	{return lastImageArray;}
	/**	 */
	unsigned int GetNHists()          	{return nHists;}
	/**	 */
	const char*  GetHistTitle(int i)  	{return (i>=0&&i<10) ? histTitle[i].c_str():0;} //int for hist number
	/**	 */
	const char*  GetHistXAxisTitle()  	{return histXAxisTitle.c_str();}
	/**	 */
	const char*  GetHistYAxisTitle()  	{return histYAxisTitle.c_str();}
	/**	 */
	unsigned int GetHistNBins()       	{return histNBins;}
	/**	 */
	double*      GetHistXAxis()       	{return histXAxis;}
	/**	 */
	double*      GetHistYAxis(int i)  	{return (i>=0&&i<10) ? histYAxis[i]:0;} //int for hist number

	/** Initializes all its members and the thread */
	void Initialization();

	/** Sets up the widget */
	void SetupWidgetWindow();

	/**	 */
	int    ResetDaqForGui();

	/**acquisition thread stuff */
	/**	 */
	bool         StartOrStopThread(bool start);
	/**	 */
	static void* DataAcquisionThread(void *this_pointer);
	/**	 */
	void*        AcquireImages();



public slots:
/** Set number of measurements
 *  @param num number of measurements to be set
 */
void setNumMeasurements(int num);

/** To select 1D or 2D plot
 * @param i is 1 for 1D, else 2D plot
 */
void SelectPlot(int i=2);

/** To select 1D plot
 */
void Select1DPlot() {SelectPlot(1);}

/** To select 2D plot
 */
void Select2DPlot() {SelectPlot(2);}

/** To clear plot
 */
void Clear1DPlot();

/** Creates a clone of the plot
 * */
void ClonePlot();

/** Closes all the clone plots
 * */
void CloseClones();



private slots:
/** To update plot
 */
void UpdatePlot();

/** To stop updating plot
 */
void StopUpdatePlot();

/** To start or stop acquisition
 * @param start is 1 to start and 0 to stop acquisition
 */
void StartDaq(bool start);

void CloneCloseEvent(int id);
signals:

void UpdatingPlotFinished();
void InterpolateSignal(bool);
void ContourSignal(bool);
void LogzSignal(bool);

};



#endif /* QDRAWPLOT_H */
