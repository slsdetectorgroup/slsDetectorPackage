/*
 * qDrawPlot.h
 *
 *  Created on: May 7, 2012
 *      Author: Ian Johnson
 */
#ifndef QDRAWPLOT_H
#define QDRAWPLOT_H

/** Form Header */
#include "ui_form_drawplot.h"
/** Project Class Headers */
class slsDetectorUtils;
/** Qt Project Class Headers */
class SlsQtH1D;
class SlsQt1DPlot;
class SlsQt2DPlotLayout;
class qCloneWidget;
/** Qt Include Headers */
class QTimer;
class QGridLayout;

/**
 *@short Sets up the plot widget
 */
class qDrawPlot:public QWidget, private Ui::DrawPlotObject{
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

public slots:
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


private:
	/** The sls detector object */
	slsDetectorUtils *myDet;
	/**	 Number of Exposures */
	int number_of_exposures;
	/**	 Duration between Exposures */
	double framePeriod;
	/**	 Acquisition Time */
	double acquisitionTime;

/** Widgets needed to plot the clone */
	static const int MAXCloneWindows = 50;
	/**	 */
	qCloneWidget			*winClone[MAXCloneWindows];
	/**	 */
	QTimer* 			plot_update_timer;
	/**	 */
	SlsQt1DPlot* 		plot1D;
	/**	 */
	SlsQt2DPlotLayout* 	plot2D;
	/**	 */
	QVector<SlsQtH1D*> 	plot1D_hists;


/**variables for threads */
	/**	 */
	volatile bool   stop_signal;
	/**	 */
	pthread_mutex_t last_image_complete_mutex;

/**variables for histograms */
	/**	 */
	unsigned int plot_in_scope;
	/**	 */
	unsigned int lastImageNumber;
	/**	 */
	std::string  imageTitle;
	/**	 */
	std::string  imageXAxisTitle;
	/**	 */
	std::string  imageYAxisTitle;
	/**	 */
	std::string  imageZAxisTitle;
	/**	 */
	unsigned int nPixelsX;
	/**	 */
	unsigned int nPixelsY;
	/**	 */
	double*      lastImageArray;
	/**	 */
	unsigned int nHists;
	/**	 */
	std::string  histTitle[10];
	/**	 */
	std::string  histXAxisTitle;
	/**	 */
	std::string  histYAxisTitle;
	/**	 */
	int          histNBins;
	/**	 */
	double*      histXAxis;
	/**	 */
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
