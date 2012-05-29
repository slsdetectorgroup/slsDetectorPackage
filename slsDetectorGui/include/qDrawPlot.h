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
#include <QString>

#define MAX_1DPLOTS 10


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

	/**	sets plot Title */
	void  		SetPlotTitle(QString title)      		{boxPlot->setTitle(title);}
	/**	sets 1D X Axis Title */
	void  		SetHistXAxisTitle(QString title)      	{histXAxisTitle = title;}
	/**	sets 1D Y Axis Title */
	void  		SetHistYAxisTitle(QString title)      	{histYAxisTitle = title;}
	/**	sets 2D X Axis Title */
	void  		SetImageXAxisTitle(QString title)      	{imageXAxisTitle = title;}
	/**	sets 2D Y Axis Title */
	void  		SetImageYAxisTitle(QString title)      	{imageYAxisTitle = title;}
	/**	sets 2D Z Axis Title */
	void  		SetImageZAxisTitle(QString title)      	{imageZAxisTitle = title;}


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
	qCloneWidget		*winClone[MAXCloneWindows];

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
	/**	vector of 1D hist values */
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
	QString  imageXAxisTitle;
	/** Y Axis Title in 2D */
	QString  imageYAxisTitle;
	/**	Z Axis Title in 2D */
	QString  imageZAxisTitle;
	/**	Number of Pixels in X Axis */
	unsigned int nPixelsX;
	/**	Number of Pixels in Y Axis */
	unsigned int nPixelsY;
	/**	Current Image Values in 1D */
	double*      lastImageArray;
	/**	Number of graphs in 1D */
	unsigned int nHists;
	/**	Title for all the graphs in 1D */
	std::string  histTitle[MAX_1DPLOTS];
	/**	X Axis Title in 1D */
	QString  histXAxisTitle;
	/**	Y Axis Title in 1D */
	QString  histYAxisTitle;
	/**	Total Number of X axis values/channels in 1D */
	int          histNBins;
	/**	X Axis value in 1D */
	double*      histXAxis;
	/** Y Axis value in 1D  */
	double*      histYAxis[MAX_1DPLOTS];


	/**	 */
	int    LockLastImageArray()			{return pthread_mutex_lock(&last_image_complete_mutex); }
	/**	 */
	int    UnlockLastImageArray()		{return pthread_mutex_unlock(&last_image_complete_mutex);}
	/**	 */
	int    StartDaqForGui() 		  	{return StartOrStopThread(1) ? 1:0;}
	/**	 */
	int    StopDaqForGui() 			  	{return StartOrStopThread(0) ? 0:1;}

	/**	 */
	const char*  GetImageTitle()      	{return imageTitle.c_str();}
	/**	 */
	const char*  GetHistTitle(int i)  	{return (i>=0&&i<MAX_1DPLOTS) ? histTitle[i].c_str():0;} //int for hist number
	/**	 */
	double*      GetHistYAxis(int i)  	{return (i>=0&&i<MAX_1DPLOTS) ? histYAxis[i]:0;} //int for hist number



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

/** To Save plot
 * @param FName full name of file
 * */
void SavePlot(QString FName);

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

/** To set the reference to zero after closing a clone
 * @param id is the id of the clone
 */
void CloneCloseEvent(int id);
signals:

void UpdatingPlotFinished();
void InterpolateSignal(bool);
void ContourSignal(bool);
void LogzSignal(bool);


};



#endif /* QDRAWPLOT_H */
