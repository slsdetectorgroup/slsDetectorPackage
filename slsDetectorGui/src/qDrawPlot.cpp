/*
 * qDrawPlot.cpp
 *
 *  Created on: May 7, 2012
 *      Author:  Dhanya Maliakal
 */
// Qt Project Class Headers
#include "qDrawPlot.h"
#include "qCloneWidget.h"
#include "slsDetector.h"
#include"fileIOStatic.h"
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
#include "postProcessing.h"
// Qt Include Headers
#include <QFont>
#include <QImage>
#include <QPainter>
#include <QFileDialog>
//#include "qwt_double_interval.h"
#include "qwt_series_data.h"
// C++ Include Headers
#include <iostream>
#include <string>
#include <sstream>
using namespace std;



//-------------------------------------------------------------------------------------------------------------------------------------------------

const double qDrawPlot::PLOT_TIMER_MS = 200;

qDrawPlot::qDrawPlot(QWidget *parent,multiSlsDetector*& detector):
		 QWidget(parent),myDet(detector),plot1D_hists(0){
	SetupWidgetWindow();
	Initialization();
	StartStopDaqToggle(); //as default
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qDrawPlot::~qDrawPlot(){
	// Clear plot
	Clear1DPlot();
	for(QVector<SlsQtH1D*>::iterator h = plot1D_hists.begin();h!=plot1D_hists.end();h++)	delete *h;
	plot1D_hists.clear();
	if(lastImageArray) delete[] lastImageArray; lastImageArray=0;
	StartOrStopThread(0);
	delete myDet; myDet = 0;
	for(int i=0;i<MAXCloneWindows;i++) if(winClone[i]) {delete winClone[i]; winClone[i] = NULL;}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SetupWidgetWindow(){
#ifdef VERBOSE
	cout << "Setting up plot variables" << endl;
#endif

	// Depending on whether the detector is 1d or 2d
	detType = myDet->getDetectorsType();
	switch(detType){
	case slsDetectorDefs::MYTHEN:	originally2D = false;	break;
	case slsDetectorDefs::EIGER:	originally2D = true;	break;
	case slsDetectorDefs::GOTTHARD:	originally2D = false; 	break;
	case slsDetectorDefs::PROPIX:	originally2D = true; 	break;
	case slsDetectorDefs::MOENCH:	originally2D = true; 	break;
	case slsDetectorDefs::JUNGFRAU:	originally2D = true; 	break;
	default:
		cout << "ERROR: Detector Type is Generic" << endl;
		exit(-1);
	}


//initialization
	data_pause_over = true;

	currentMeasurement = 0;
	currentFrame = 0;
	numFactor = 0;
	currentScanDivLevel = 0;
	currentScanValue = 0;
	number_of_exposures = 0;
	number_of_frames = 0;
	acquisitionPeriod = 0;
	exposureTime = 0;
	currentFileIndex = 0;
	currentFrameIndex = 0;

	stop_signal = 0;
	pthread_mutex_init(&last_image_complete_mutex,NULL);

	// Default titles- only for the initial picture
	imageXAxisTitle="Pixel";
	imageYAxisTitle="Pixel";
	imageZAxisTitle="Intensity";
	histXAxisTitle="Channel Number";
	histYAxisTitle="Counts";
	for(int i=0;i<MAX_1DPLOTS;i++){
		histTitle[i] = "";
		//char temp_title[2000];
		//sprintf(temp_title,"Frame -%d",i);
		//histTitle[i] = temp_title;
	}
	imageTitle="";
	plotTitle = "";
	plotTitle_prefix = "";
	plot_in_scope   = 0;
	nPixelsX = myDet->getTotalNumberOfChannels(slsDetectorDefs::X);		cout<<"nPixelsX:"<<nPixelsX<<endl;
	nPixelsY = myDet->getTotalNumberOfChannels(slsDetectorDefs::Y);		cout<<"nPixelsY:"<<nPixelsY<<endl;
	nAnglePixelsX = 1;
	minPixelsY = 0;
	maxPixelsY = 0;
	startPixel=-0.5;
	endPixel=nPixelsY-0.5;
	pixelWidth = 0;

	lastImageNumber = 0;

	nHists    = 0;
	histNBins = 0;
	histXAxis = 0;
	for(int i=0;i<MAX_1DPLOTS;i++)
		histYAxis[i]=0;
	histXAngleAxis = 0;
	histYAngleAxis = 0;
	histTrimbits=0;
	lastImageArray = 0;

	persistency = 0;
	currentPersistency = 0;
	progress = 0;
	plotEnable = true;

	//marker
	lines = true;
	markers = false;
	marker = new QwtSymbol();
	marker->setStyle(QwtSymbol::Cross);
	marker->setSize(5,5);
	noMarker = new QwtSymbol();

	//for save automatically,
	saveAll = false;
	saveError = false;
	lastSavedFrame = -1;
	lastSavedMeasurement = -1;

	// This is so that it initially stop and plots
	running = 1;

	XYRangeChanged = false;
	XYRangeValues[0] = 0;
	XYRangeValues[1] = 0;
	XYRangeValues[2] = 0;
	XYRangeValues[3] = 0;
	IsXYRange[0] = false;
	IsXYRange[1] = false;
	IsXYRange[2] = false;
	IsXYRange[3] = false;

	timerValue = PLOT_TIMER_MS;
	frameFactor=0;
	isFrameEnabled = false;
	isTriggerEnabled = false;

	scanArgument = qDefs::None;
	histogramArgument = qDefs::Intensity;
	anglePlot = false;
	alreadyDisplayed =  false;

	//filepath and file name
	filePath = QString(myDet->getFilePath().c_str());
	fileName = QString(myDet->getFileName().c_str());

	backwardScanPlot = false;
	fileSaveEnable= myDet->enableWriteToFile();

	//pedestal
	pedestal = false;
	pedestalVals = 0;
	tempPedestalVals = 0;
	pedestalCount = 0;
	startPedestalCal = false;

	//accumulate
	accumulate = false;
	resetAccumulate = false;

	clientInitiated = false;

	//binary plot output
	binary = false;
	binaryFrom = 0;
	binaryTo = 0;

	//histogram
	histogram = false;
	histFrom = 0;
	histTo = 0;
	histSize = 0;
	/*
	grid = new QwtPlotGrid;
		grid->enableXMin(true);
		grid->enableYMin(true);
		grid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
		grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
		*/
	plotHistogram = new QwtPlotHistogram();
	plotHistogram->setStyle(QwtPlotHistogram::Columns);//Options:Outline,Columns, Lines


	plotRequired = false;

//widget related initialization

	// clone
	for(int i=0;i<MAXCloneWindows;i++) winClone[i]=0;

	// Setting up window
	setFont(QFont("Sans Serif",9));
	layout = new QGridLayout;
		this->setLayout(layout);

		histFrameIndexTitle= histFrameIndexTitle = new QLabel("");

	boxPlot = new QGroupBox("");
		layout->addWidget(boxPlot,1,0);
		boxPlot->setAlignment(Qt::AlignHCenter);
		boxPlot->setFont(QFont("Sans Serif",11,QFont::Normal));
		boxPlot->setTitle("Sample Plot");
	data_pause_timer = new QTimer(this);
	connect(data_pause_timer, SIGNAL(timeout()), this, SLOT(UpdatePause()));


	//display statistics
	displayStatistics = false;
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
	lblSumDisp->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
	lblSumDisp->setAlignment(Qt::AlignLeft);
	hl1->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	hl1->addWidget(lblMin);
	hl1->addWidget(lblMinDisp);
	hl1->addItem(new QSpacerItem(20,20,QSizePolicy::Expanding,QSizePolicy::Fixed));
	hl1->addWidget(lblMax);
	hl1->addWidget(lblMaxDisp);
	hl1->addItem(new QSpacerItem(20,20,QSizePolicy::Expanding,QSizePolicy::Fixed));
	hl1->addWidget(lblSum);
	hl1->addWidget(lblSumDisp);
	hl1->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	widgetStatistics->setLayout(hl1);
	layout->addWidget(widgetStatistics,2,0);
	widgetStatistics->hide();


	// setting default plot titles and settings
	plot1D = new SlsQt1DPlot(boxPlot);

		plot1D->setFont(QFont("Sans Serif",9,QFont::Normal));
		plot1D->SetXTitle(histXAxisTitle.toAscii().constData());
		plot1D->SetYTitle(histYAxisTitle.toAscii().constData());
		plot1D->hide();

		SlsQtH1D*  h;
		histNBins = nPixelsX;
		nHists = 1;
		if(histXAxis)    delete [] histXAxis; 	histXAxis    = new double [nPixelsX];
		if(histYAxis[0]) delete [] histYAxis[0];histYAxis[0] = new double [nPixelsX];
		for(unsigned int px=0;px<(int)nPixelsX;px++)	{histXAxis[px]  = px;histYAxis[0][px] = 0;}
		Clear1DPlot();
		plot1D->SetXTitle("X Axis");
		plot1D->SetYTitle("Y Axis");
		plot1D_hists.append(h=new SlsQtH1D("",histNBins,histXAxis,histYAxis[0]));
		h->SetLineColor(0);
		SetStyle(h);
		h->Attach(plot1D);
		Clear1DPlot();

	plot2D = new SlsQt2DPlotLayout(boxPlot);
	//default plot
	lastImageArray = new double[nPixelsY*nPixelsX];
	for(unsigned int px=0;px<nPixelsX;px++)
		for(unsigned int py=0;py<nPixelsY;py++)
			lastImageArray[py*nPixelsX+px] = sqrt(pow(0+1,2)*pow(double(px)-nPixelsX/2,2)/pow(nPixelsX/2,2)/pow(1+1,2) + pow(double(py)-nPixelsY/2,2)/pow(nPixelsY/2,2))/sqrt(2);
		plot2D->setFont(QFont("Sans Serif",9,QFont::Normal));
		plot2D->GetPlot()->SetData(nPixelsX,-0.5,nPixelsX-0.5,nPixelsY,startPixel,endPixel,lastImageArray);
		plot2D->setTitle(GetImageTitle());
		plot2D->SetXTitle(imageXAxisTitle);
		plot2D->SetYTitle(imageYAxisTitle);
		plot2D->SetZTitle(imageZAxisTitle);
		plot2D->setAlignment(Qt::AlignLeft);
	boxPlot->setFlat(true);
	boxPlot->setContentsMargins(0,15,0,0);

	plotLayout =  new QGridLayout(boxPlot);
	plotLayout->setContentsMargins(0,0,0,0);
		plotLayout->addWidget(plot1D,0,0,1,1);
		plotLayout->addWidget(plot2D,0,0,1,1);


//callbacks

	// Setting the callback function to get data from detector class
	myDet->registerDataCallback(&(GetDataCallBack),this);
	//Setting the callback function to alert when acquisition finished from detector class
	myDet->registerAcquisitionFinishedCallback(&(GetAcquisitionFinishedCallBack),this);
	//Setting the callback function to alert when each measurement finished from detector class
	myDet->registerMeasurementFinishedCallback(&(GetMeasurementFinishedCallBack),this);
	//Setting the callback function to get progress from detector class(using receivers)
	myDet->registerProgressCallback(&(GetProgressCallBack),this);
	//stream data from receiver to the gui
	if(detType != slsDetectorDefs::MYTHEN)
		myDet->enableDataStreamingFromReceiver(1);


	qDefs::checkErrorMessage(myDet,"qDrawPlot::SetupWidgetWindow");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::Initialization(){
	connect(this, 		SIGNAL(UpdatePlotSignal()),		this, 		SLOT(UpdatePlot()));
	connect(this, 		SIGNAL(InterpolateSignal(bool)),plot2D, 	SIGNAL(InterpolateSignal(bool)));
	connect(this, 		SIGNAL(ContourSignal(bool)),	plot2D, 	SIGNAL(ContourSignal(bool)));
	connect(this, 		SIGNAL(LogzSignal(bool)),		plot2D, 	SLOT(SetZScaleToLog(bool)));
	connect(this, 		SIGNAL(LogySignal(bool)),		plot1D, 	SLOT(SetLogY(bool)));
	connect(this, 		SIGNAL(ResetZMinZMaxSignal(bool,bool,double,double)),plot2D, 	SLOT(ResetZMinZMax(bool,bool,double,double)));

	connect(this, 		SIGNAL(SetZRangeSignal(double,double)),	plot2D, 	SLOT(SetZRange(double,double)));

	connect(this, 		SIGNAL(AcquisitionErrorSignal(QString)),	this, 	SLOT(ShowAcquisitionErrorMessage(QString)));


}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::StartStopDaqToggle(bool stop_if_running){
#ifdef VERYVERBOSE
	cout << "Entering StartStopDaqToggle(" << stop_if_running << ")" <<endl;
#endif
	//static bool running = 1;
	if(running){ //stopping
		StartDaq(false);
		running=!running;
	}else if(!stop_if_running){ //then start
		// Reset Current Measurement
		currentMeasurement = 0;
		emit SetCurrentMeasurementSignal(currentMeasurement);
		data_pause_over = true;
		//in case of error message
		alreadyDisplayed = false;

		/*
		// Number of Exposures
		int numFrames = (isFrameEnabled)*((int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1));
		int numTriggers = (isTriggerEnabled)*((int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,-1));
		numFrames = ((numFrames==0)?1:numFrames);
		numTriggers = ((numTriggers==0)?1:numTriggers);
		number_of_frames = numFrames * numTriggers;
		cout << "\tNumber of Frames per Scan/Measurement:" << number_of_frames << endl;
		//get #scansets for level 0 and level 1
		int numScan0 = myDet->getScanSteps(0);	numScan0 = ((numScan0==0)?1:numScan0);
		int numScan1 = myDet->getScanSteps(1);	numScan1 = ((numScan1==0)?1:numScan1);
		int numPos=myDet->getPositions();

		number_of_exposures = number_of_frames * numScan0 * numScan1;
		if(anglePlot) number_of_exposures = numScan0 * numScan1;// * numPos;
		cout << "\tNumber of Exposures Per Measurement:" << number_of_exposures << endl;
		*/

		// ExposureTime
		exposureTime= ((double)(myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,-1))*1E-9);
		cout << "\tExposure Time:" << setprecision (10) << exposureTime << endl;
		// Acquisition Period
		acquisitionPeriod= ((double)(myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,-1))*1E-9);
		cout << "\tAcquisition Period:" << setprecision (10) << acquisitionPeriod << endl;
		cout << "\tFile Index:" << myDet->getFileIndex() << endl;
		//to take the first data if frameFactor
		numFactor = 0;

		//for save automatically,
		saveError = false;
		lastSavedFrame = -1;
		lastSavedMeasurement = -1;

		//update file path and file name
		filePath = QString(myDet->getFilePath().c_str());
		fileName = QString(myDet->getFileName().c_str());
		//update index
		currentFileIndex = myDet->getFileIndex();
		currentFrameIndex = 0;

		StartDaq(true);
		running=!running;

		qDefs::checkErrorMessage(myDet,"qDrawPlot::StartStopDaqToggle");
	}

	/** if this is set during client initation */
	clientInitiated = false;

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::StartDaq(bool start){
	if(start){
#ifdef VERBOSE
		cout << "Start Daq(true) function" << endl;
#endif
		ResetDaqForGui();
		StartDaqForGui();
	}else{
#ifdef VERBOSE
		cout << "Start Daq(false) function" << endl;
#endif
		StopDaqForGui();
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qDrawPlot::ResetDaqForGui(){
	if(!StopDaqForGui()) return 0;
	cout << "Resetting image number" << endl;
	lastImageNumber = 0;
	return 1;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


bool qDrawPlot::StartOrStopThread(bool start){
#ifdef VERYVERBOSE
	cout << "StartOrStopThread:" << start << endl;
#endif
	static bool firstTime = true;
	static bool             gui_acquisition_thread_running = 0;
	static pthread_t        gui_acquisition_thread;
	static pthread_mutex_t  gui_acquisition_start_stop_mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&gui_acquisition_start_stop_mutex);
	//stop part, before start or restart
	if(gui_acquisition_thread_running){
		cout << "Stopping current acquisition thread ...." << endl;
		stop_signal = 1;//sorta useless right now
		gui_acquisition_thread_running = 0;
	}

	//start part
	if(start){
		progress = 0;
		//sets up the measurement parameters
		SetupMeasurement();

		//refixing all the zooming
		plot2D->GetPlot()->SetXMinMax(-0.5,nPixelsX+0.5);
		plot2D->GetPlot()->SetYMinMax(startPixel,endPixel);
		plot2D->GetPlot()->SetZoom(-0.5,startPixel,nPixelsX,endPixel-startPixel);
		plot2D->GetPlot()->UnZoom();
		/*XYRangeChanged = true;*/
		boxPlot->setTitle("Old_Plot.raw");

		cout << "Starting new acquisition thread ...." << endl;
		// Start acquiring data from server
		if(!firstTime) pthread_join(gui_acquisition_thread,NULL);//wait until he's finished, ie. exits
		pthread_create(&gui_acquisition_thread, NULL,DataStartAcquireThread, (void*) this);
		// This is set here and later reset to zero when all the plotting is done
		// This is manually done instead of keeping track of thread because
		// this thread returns immediately after executing the acquire command
		gui_acquisition_thread_running=1;
		cout << "Started acquiring threaddd:" << endl;
	}
	pthread_mutex_unlock(&gui_acquisition_start_stop_mutex);
	return gui_acquisition_thread_running;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SetScanArgument(int scanArg){
#ifdef VERYVERBOSE
	cout << "SetScanArgument function:" << scanArg << " running:" << running << endl;
#endif
	scanArgument = scanArg;

	LockLastImageArray();

	if(plot_in_scope==1) Clear1DPlot();


	// Number of Exposures - must be calculated here to get npixelsy for allframes/frameindex scans
	int numFrames = (isFrameEnabled)*((int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1));
	int numTriggers = (isTriggerEnabled)*((int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,-1));
	numFrames = ((numFrames==0)?1:numFrames);
	numTriggers = ((numTriggers==0)?1:numTriggers);
	number_of_frames = numFrames * numTriggers;
	cout << "\tNumber of Frames per Scan/Measurement:" << number_of_frames << endl;
	//get #scansets for level 0 and level 1
	int numScan0 = myDet->getScanSteps(0);	numScan0 = ((numScan0==0)?1:numScan0);
	int numScan1 = myDet->getScanSteps(1);	numScan1 = ((numScan1==0)?1:numScan1);
	int numPos=myDet->getPositions();

	number_of_exposures = number_of_frames * numScan0 * numScan1;
	if(anglePlot) number_of_exposures = numScan0 * numScan1;// * numPos;
	cout << "\tNumber of Exposures Per Measurement:" << number_of_exposures << endl;


	maxPixelsY = 0;
	minPixelsY = 0;
	nPixelsX = myDet->getTotalNumberOfChannels(slsDetectorDefs::X);
	nPixelsY = myDet->getTotalNumberOfChannels(slsDetectorDefs::Y);
	//cannot do this in between measurements , so update instantly
	if(scanArgument==qDefs::Level0){
		//no need to check if numsteps=0,cuz otherwise this mode wont be set in plot tab
		int numSteps = myDet->getScanSteps(0);
		double *values = new double[numSteps];
		myDet->getScanSteps(0,values);

		maxPixelsY = values[numSteps-1];
		minPixelsY = values[0];
		nPixelsY = numSteps;
	}else if(scanArgument==qDefs::Level1) {
		//no need to check if numsteps=0,cuz otherwise this mode wont be set in plot tab
		int numSteps = myDet->getScanSteps(1);
		double *values = new double[numSteps];
		myDet->getScanSteps(1,values);

		maxPixelsY = values[numSteps-1];
		minPixelsY = values[0];
		nPixelsY = numSteps;
	}else if(scanArgument==qDefs::AllFrames)
		nPixelsY = number_of_exposures;
	else if(scanArgument==qDefs::FileIndex)
		nPixelsY = number_of_frames;


	if(minPixelsY>maxPixelsY){
		double temp = minPixelsY;
		minPixelsY = maxPixelsY;
		maxPixelsY = temp;
		backwardScanPlot = true;
	}else backwardScanPlot = false;


	//1d
	if(histXAxis)    delete [] histXAxis;	histXAxis    = new double [nPixelsX];

	if(histYAxis[0]) delete [] histYAxis[0]; histYAxis[0] = new double [nPixelsX];

	//2d
	if(lastImageArray) delete [] lastImageArray; lastImageArray = new double[nPixelsY*nPixelsX];

	//initializing 1d x axis
	for(unsigned int px=0;px<(int)nPixelsX;px++)	histXAxis[px]  = px;/*+10;*/

	//initializing 2d array
	for(int py=0;py<(int)nPixelsY;py++)
		for(int px=0;px<(int)nPixelsX;px++)
			lastImageArray[py*nPixelsX+px] = 0;


	//histogram
	if(histogram){
		int iloop = 0;
		int numSteps = ((histTo-histFrom)/(histSize)) + 1;cout<<"numSteps:"<<numSteps<<" histFrom:"<<histFrom<<" histTo:"<<histTo<<" histSize:"<<histSize<<endl;
		histogramSamples.resize(numSteps);
		startPixel = histFrom -(histSize/2);cout<<"startpixel:"<<startPixel<<endl;
		endPixel = histTo + (histSize/2);cout<<"endpixel:"<<endPixel<<endl;
		while(startPixel < endPixel){
			histogramSamples[iloop].interval.setInterval(startPixel,startPixel+histSize,QwtInterval::ExcludeMaximum);
			histogramSamples[iloop].value = 0;
			startPixel += histSize;
			iloop++;
		}

		//print values
		cout << "Histogram Intervals:" << endl;
		for(int j=0;j<histogramSamples.size();j++){
			cout<<j<<":\tmin:"<<histogramSamples[j].interval.minValue()<<""
					"\t\tmax:"<<histogramSamples[j].interval.maxValue()<<"\t\tvalue:"<<histogramSamples[j].value<<endl;
		}
	}

	UnlockLastImageArray();

	qDefs::checkErrorMessage(myDet,"qDrawPlot::SetScanArgument");

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SetupMeasurement(){
#ifdef VERYVERBOSE
	cout << "SetupMeasurement function:" << running << endl;
#endif
	LockLastImageArray();
#ifdef VERYVERBOSE
	cout << "locklastimagearray" << endl;
#endif
	// Defaults
	if(!running)
		stop_signal = 0;
	plotRequired = 0;
	currentFrame = 0;
	//for 2d scans
	currentScanDivLevel = 0;
	//if(plot_in_scope==2)
		if(!running)
		  lastImageNumber = 0;/**Just now */
	//initializing 2d array
	for(int py=0;py<(int)nPixelsY;py++)
		for(int px=0;px<(int)nPixelsX;px++)
			lastImageArray[py*nPixelsX+px] = 0;

	//1d with no scan
	if ((!originally2D) && (scanArgument==qDefs::None)){
#ifdef VERYVERBOSE
	cout << "1D" << endl;
#endif
		if(!running){
			maxPixelsY = 100;
			minPixelsY = 0;
			startPixel = -0.5;
			endPixel = nPixelsY-0.5;
		}
	}
	else {
#ifdef VERYVERBOSE
	cout << "2D" << endl;
#endif
		//2d with no scan
		if ((originally2D) && (scanArgument==qDefs::None)){
			maxPixelsY = nPixelsY;
			minPixelsY = 0;
		}

		//all frames
		else if(scanArgument==qDefs::AllFrames){
			maxPixelsY = number_of_exposures - 1;
			minPixelsY = 0;
			if(!running) nPixelsY = number_of_exposures;
		}//frame index
		else if(scanArgument==qDefs::FileIndex){
			maxPixelsY = number_of_frames - 1;
			minPixelsY = 0;
			if(!running) nPixelsY = number_of_frames;
		}//level0 or level1
		else {
			currentScanValue = minPixelsY;
			if(backwardScanPlot){
				currentScanValue = maxPixelsY;
				currentScanDivLevel = nPixelsY-1;
			}
		}

		//cannot divide by 0
		if(nPixelsY==1){
			pixelWidth = 0;
			startPixel = minPixelsY-0.5;
			endPixel = minPixelsY+0.5;
		}else{
			pixelWidth = (maxPixelsY -minPixelsY)/(nPixelsY-1);
			startPixel = minPixelsY -(pixelWidth/2);
			endPixel = maxPixelsY + (pixelWidth/2);
		}

		if(histogram){
			startPixel = histFrom -(histSize/2);
			endPixel = histTo + (histSize/2);
		}
	}


/*
	cout<<"nPixelsX:"<<nPixelsX<<endl;
	cout<<"nPixelsY:"<<nPixelsY<<endl;
	cout<<"minPixelsY:"<<minPixelsY<<endl;
	cout<<"maxPixelsY:"<<maxPixelsY<<endl;
	cout<<"startPixel:"<<startPixel<<endl;
	cout<<"endPixel:"<<endPixel<<endl<<endl;
*/
	UnlockLastImageArray();


#ifdef VERYVERBOSE
	cout << "locklastimagearray" << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void* qDrawPlot::DataStartAcquireThread(void *this_pointer){
	((qDrawPlot*)this_pointer)->myDet->acquire(1);
	return this_pointer;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qDrawPlot::GetDataCallBack(detectorData *data, int fIndex, int subIndex, void *this_pointer){
	((qDrawPlot*)this_pointer)->GetData(data,fIndex, subIndex);
	return 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qDrawPlot::GetData(detectorData *data,int fIndex, int subIndex){
#ifdef VERYVERBOSE
	cout << "******Entering GetDatafunction********" << endl;
	cout << "fIndex " << fIndex << endl;
	cout << "subIndex " << subIndex << endl;
	cout << "fname " << data->fileName << endl;
	cout << "npoints " << data->npoints << endl;
	cout << "npy " << data->npy << endl;
	cout << "npy " << data->progressIndex << endl;
	cout << "values " << data->values << endl;
	cout << "errors " << data->errors << endl;
	cout << "angle " << data->angles << endl;
#endif
	if(!stop_signal){

		//set progress
		progress=(int)data->progressIndex;
		currentFrameIndex = fileIOStatic::getIndicesFromFileName(string(data->fileName),currentFileIndex);
		//happens if receiver sends a null and empty file name
		/*if(string(data->fileName).empty()){
			cout << "Received empty file name. Exiting function without updating data for plot." << endl;
			return -1;
		}*/
#ifdef VERYVERBOSE
		cout << "progress:" << progress << endl;
#endif
		// secondary title necessary to differentiate between frames when not saving data
		char temp_title[2000];
		//findex is the frame index given by receiver, cannot be derived from file name
		if(fIndex!=-1){
			currentFrameIndex=fIndex;
			sprintf(temp_title,"#%d",fIndex);
			if((detType==slsDetectorDefs::EIGER) && (subIndex != -1))
				sprintf(temp_title,"#%d  %d",fIndex,subIndex);
		}else{
			if(fileSaveEnable)	strcpy(temp_title,"#%d");
			else		sprintf(temp_title,"",currentFrame);
		}
		if(subIndex != -1)
			sprintf(temp_title,"#%d  %d",fIndex,subIndex);

		//Plot Disabled
		if(!plotEnable)
			return 0;


		//angle plotting
		if(anglePlot){
			LockLastImageArray();
			//set title
			plotTitle=QString(plotTitle_prefix)+QString(data->fileName).section('/',-1);
			// Title
			histTitle[0] = temp_title;

			if(data->angles==NULL){
				cout<<"\n\nWARNING:RETURNED NULL instead of angles."<<endl;
				lastImageNumber= currentFrame+1;
				nAnglePixelsX = nPixelsX;
				histNBins = nAnglePixelsX;
				nHists=1;
				memcpy(histXAngleAxis,histXAxis,nAnglePixelsX*sizeof(double));
				memcpy(histYAngleAxis,data->values,nAnglePixelsX*sizeof(double));
				SetHistXAxisTitle("Channel Number");

			}
			else{
				lastImageNumber= currentFrame+1;
				nAnglePixelsX = data->npoints;
				histNBins = nAnglePixelsX;
				nHists=1;
				if(histXAngleAxis) delete [] histXAngleAxis; histXAngleAxis = new double[nAnglePixelsX];
				if(histYAngleAxis) delete [] histYAngleAxis; histYAngleAxis = new double[nAnglePixelsX];
#ifdef CHECKINFERROR
				int k = 0;
				for(int i = 0; i < data->npoints; i++){
					if(isinf(data->values[i])){
						//cout << "*** ERROR: value at " << i << " infinity" << endl;
						k++;
						data->values[i] = -1;
					}
				}
				if (k>0) {
					cout << "*** ERROR: value at " << k << " places have infinity values!" <<  endl;
					double m1,m2,s1;
					GetStatistics(m1,m2,s1,data->angles,nAnglePixelsX);
					cout << "angle min:" << m1 << endl;
					cout << "angle max:" << m2 << endl;
					cout << "angle sum:" << s1 << endl;
					GetStatistics(m1,m2,s1,data->values,nAnglePixelsX);
					cout << "value min:" << m1 << endl;
					cout << "value max:" << m2 << endl;
					cout << "value sum:" << s1 << endl;
				}
#endif
				memcpy(histXAngleAxis,data->angles,nAnglePixelsX*sizeof(double));
				memcpy(histYAngleAxis,data->values,nAnglePixelsX*sizeof(double));
				SetHistXAxisTitle("Angles");
			}
			UnlockLastImageArray();
			currentFrame++;
#ifdef VERYVERBOSE
			cout << "Exiting GetData Function " << endl;
#endif
			emit UpdatePlotSignal();
			plotRequired = true;
			return 0;
		}


		//nth frame or delay decision (data copied later on)
		if (detType == slsDetectorDefs::MYTHEN){
			//Nth Frame
			if(frameFactor){
				//plots if numfactor becomes 0
				if(!numFactor) numFactor=frameFactor-1;
				//return if not
				else{
					numFactor--;
					return 0;
				}
			}

			//Not Nth Frame, to check time out(NOT for Scans and angle plots)
			else{
				if (scanArgument == qDefs::None) {
					//if the time is not over, RETURN
					if(!data_pause_over){
						return 0;
					}
					data_pause_over=false;
					data_pause_timer->start((int)(timerValue));
				}
			}
		}


		//if scan
		//alframes
		if(scanArgument==qDefs::AllFrames){
			LockLastImageArray();
			//set title
			plotTitle=QString(plotTitle_prefix)+QString(data->fileName).section('/',-1);
			//variables
			lastImageNumber= currentFrame+1;
			//title
			imageTitle = temp_title;
			//copy data
			memcpy(lastImageArray+(currentScanDivLevel*nPixelsX),data->values,nPixelsX*sizeof(double));
			UnlockLastImageArray();
			currentFrame++;
			currentScanDivLevel++;
			emit UpdatePlotSignal();
			plotRequired = true;
			return 0;
		}
		//file index
		if(scanArgument==qDefs::FileIndex){
			LockLastImageArray();
			//set title
			plotTitle=QString(plotTitle_prefix)+QString(data->fileName).section('/',-1);
			//variables
			if(currentFrameIndex == 0) currentScanDivLevel = 0;
			lastImageNumber= currentFrame+1;
			//title
			imageTitle = temp_title;
			//copy data
			for(unsigned int px=0;px<nPixelsX;px++)	lastImageArray[currentScanDivLevel*nPixelsX+px] += data->values[px];
			UnlockLastImageArray();
			currentFrame++;
			currentScanDivLevel++;
			emit UpdatePlotSignal();
			plotRequired = true;
			return 0;
		}
		//level0
		if(scanArgument==qDefs::Level0){
			LockLastImageArray();
			//set title
			plotTitle=QString(plotTitle_prefix)+QString(data->fileName).section('/',-1);
			//get scanvariable0
			int ci = 0, fi = 0, p = 0, di = 0; double cs0 = 0 , cs1 = 0;
			fileIOStatic::getVariablesFromFileName(string(data->fileName), ci, fi, p, cs0, cs1, di);
			//variables
			if(cs0!=currentScanValue) {
				if(backwardScanPlot)	currentScanDivLevel--;
				else					currentScanDivLevel++;
			}
			currentScanValue = cs0;
			lastImageNumber= currentFrame+1;
			//title
			imageTitle = temp_title;
			//copy data
			for(unsigned int px=0;px<nPixelsX;px++) lastImageArray[currentScanDivLevel*nPixelsX+px] += data->values[px];
			UnlockLastImageArray();
			currentFrame++;
			emit UpdatePlotSignal();
			plotRequired = true;
			return 0;
		}
		//level1
		if(scanArgument==qDefs::Level1){
			LockLastImageArray();
			//set title
			plotTitle=QString(plotTitle_prefix)+QString(data->fileName).section('/',-1);
			//get scanvariable1
			int ci = 0, fi = 0, p = 0, di = 0; double cs0 = 0 , cs1 = 0;
			fileIOStatic::getVariablesFromFileName(string(data->fileName), ci, fi, p, cs0, cs1, di);
			//variables
			if(cs1!=currentScanValue){
				if(backwardScanPlot)	currentScanDivLevel--;
				else					currentScanDivLevel++;
			}
			currentScanValue = cs1;
			lastImageNumber= currentFrame+1;
			//title
			imageTitle = temp_title;
			//copy data
			for(unsigned int px=0;px<nPixelsX;px++) lastImageArray[currentScanDivLevel*nPixelsX+px] += data->values[px];
			UnlockLastImageArray();
			currentFrame++;
			emit UpdatePlotSignal();
			plotRequired = true;
			return 0;
		}



		//normal measurement or 1d scans
		LockLastImageArray();
		/*if(!pthread_mutex_trylock(&(last_image_complete_mutex))){*/
		//set title
		plotTitle=QString(plotTitle_prefix)+QString(data->fileName).section('/',-1);
		// only if you got the lock, do u need to remember lastimagenumber to plot
		lastImageNumber= currentFrame+1;
		//cout<<"got last imagenumber:"<<lastImageNumber<<endl;
		//1d
		if(plot_in_scope==1){
			// Titles
			histTitle[0] = temp_title;

			//histogram
			if(histogram){
				resetAccumulate = false;
				lastImageNumber= currentFrame+1;

				int numValues = nPixelsX;
				if(originally2D)
					numValues = nPixelsX*nPixelsY;

				//clean up graph
				if(histogramArgument == qDefs::Intensity){
					for(int j=0;j<histogramSamples.size();j++){
						histogramSamples[j].value = 0;

					}
				}

				int val = 0 ;
				for(int i=0;i<numValues;i++){
					//frequency of intensity
					if(histogramArgument == qDefs::Intensity){
						//ignore outside limits
						if ((data->values[i] <  histFrom) || (data->values[i] > histTo))
							continue;
						//check for intervals, increment if validates
						for(int j=0;j<histogramSamples.size();j++){
							if(histogramSamples[j].interval.contains(data->values[i]))
								histogramSamples[j].value += 1;

						}
					}
					//get sum of data pixels
					else
						val += data->values[i];

				}


				if(histogramArgument != qDefs::Intensity){
					val /= numValues;

					//find scan value
					int ci = 0, fi = 0; double cs0 = 0 , cs1 = 0;
					fileIOStatic::getVariablesFromFileName(string(data->fileName), ci, fi,  cs0, cs1);

					int scanval=-1;
					if(cs0 != -1)
						scanval = cs0;
					else scanval = cs1;

					//ignore outside limits
					if ((scanval <  histFrom) || (scanval > histTo) || (scanval == -1))
						scanval = -1;
					//check for intervals, increment if validates
					for(int j=0;j<histogramSamples.size();j++){
						if(histogramSamples[j].interval.contains(scanval)){
							histogramSamples[j].value = val;
							cout << "j:"<<j<<" scanval:"<<scanval<<" val:"<<val<<endl;
						}
					}
				}

			}
			//not histogram
			else{
				// Persistency
				if(currentPersistency < persistency)currentPersistency++;
				else currentPersistency=persistency;
				nHists = currentPersistency+1;
				histNBins = nPixelsX;

				// copy data
				for(int i=currentPersistency;i>0;i--)
					memcpy(histYAxis[i],histYAxis[i-1],nPixelsX*sizeof(double));

				//recalculating pedestal
				if(startPedestalCal){
					//start adding frames to get to the pedestal value
					if(pedestalCount<NUM_PEDESTAL_FRAMES){
						for(unsigned int px=0;px<nPixelsX;px++)
							tempPedestalVals[px] += data->values[px];
						memcpy(histYAxis[0],data->values,nPixelsX*sizeof(double));
						pedestalCount++;
					}
					//calculate the pedestal value
					if(pedestalCount==NUM_PEDESTAL_FRAMES){
						cout << "Pedestal Calculated" << endl;
						for(unsigned int px=0;px<nPixelsX;px++)
							tempPedestalVals[px] = tempPedestalVals[px]/(double)NUM_PEDESTAL_FRAMES;
						memcpy(pedestalVals,tempPedestalVals,nPixelsX*sizeof(double));
						startPedestalCal = 0;
					}
				}

				//normal data
				if(((!pedestal)&(!accumulate)&(!binary))	|| (resetAccumulate)){
					memcpy(histYAxis[0],data->values,nPixelsX*sizeof(double));
					resetAccumulate = false;
				}
				//pedestal or accumulate
				else{
					double temp;//cannot overwrite cuz of accumulate
					for(unsigned int px=0;px<(nPixelsX*nPixelsY);px++){
						temp = data->values[px];
						if(pedestal)
							temp = data->values[px] - (pedestalVals[px]);
						if(binary) {
							if ((temp >= binaryFrom) && (temp <= binaryTo))
								temp = 1;
							else
								temp = 0;
						}
						if(accumulate)
							temp += histYAxis[0][px];
						//after all processing
						histYAxis[0][px] = temp;
					}
				}
			}
		}
		//2d
		else{
			// Titles
			imageTitle = temp_title;
			//recalculating pedestal
			if(startPedestalCal){
				//start adding frames to get to the pedestal value
				if(pedestalCount<NUM_PEDESTAL_FRAMES){
					for(unsigned int px=0;px<(nPixelsX*nPixelsY);px++)
						tempPedestalVals[px] += data->values[px];
					memcpy(lastImageArray,data->values,nPixelsX*nPixelsY*sizeof(double));
					pedestalCount++;
				}
				//calculate the pedestal value
				if(pedestalCount==NUM_PEDESTAL_FRAMES){
					cout << "Pedestal Calculated" << endl;
					for(unsigned int px=0;px<(nPixelsX*nPixelsY);px++)
						tempPedestalVals[px] = tempPedestalVals[px]/(double)NUM_PEDESTAL_FRAMES;
					memcpy(pedestalVals,tempPedestalVals,nPixelsX*nPixelsY*sizeof(double));
					startPedestalCal = 0;
				}
			}

			//normal data
			if(((!pedestal)&(!accumulate)&(!binary))	|| (resetAccumulate)){
				memcpy(lastImageArray,data->values,nPixelsX*nPixelsY*sizeof(double));
				resetAccumulate = false;
			}
			//pedestal or accumulate or binary
			else{
				double temp;
				for(unsigned int px=0;px<(nPixelsX*nPixelsY);px++){
					temp = data->values[px];
					if(pedestal)
						temp = data->values[px] - (pedestalVals[px]);
					if(binary) {
						if ((temp >= binaryFrom) && (temp <= binaryTo))
							temp = 1;
						else
							temp = 0;
					}
					if(accumulate)
						temp += lastImageArray[px];
					//after all processing
					lastImageArray[px] = temp;
				}
			}

		}
		/*	pthread_mutex_unlock(&(last_image_complete_mutex));
		}*/
		UnlockLastImageArray();

#ifdef VERYVERBOSE
		cprintf(BLUE,"currentframe:%d \tcurrentframeindex:%d\n",currentFrame,currentFrameIndex);
#endif
		emit UpdatePlotSignal();
		plotRequired = true;
		currentFrame++;
	}


#ifdef VERYVERBOSE
	cout << "Exiting GetData function" << endl;
#endif
	return 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qDrawPlot::GetAcquisitionFinishedCallBack(double currentProgress,int detectorStatus, void *this_pointer){
	((qDrawPlot*)this_pointer)->AcquisitionFinished(currentProgress,detectorStatus);
#ifdef VERYVERBOSE
	cout << "acquisition finished callback worked ok" << endl;
#endif
	return 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qDrawPlot::AcquisitionFinished(double currentProgress, int detectorStatus){
#ifdef VERBOSE
	cout << "\nEntering Acquisition Finished with status " ;
#endif
	QString status = QString(slsDetectorBase::runStatusType(slsDetectorDefs::runStatus(detectorStatus)).c_str());
#ifdef VERBOSE
  cout << status.toAscii().constData() << " and progress " << currentProgress << endl;
#endif
  //error or stopped
	if((stop_signal)||(detectorStatus==slsDetectorDefs::ERROR)){
#ifdef VERBOSE
		cout << "Error in Acquisition" << endl << endl;
#endif
		//stop_signal = 1;//just to be sure
		emit AcquisitionErrorSignal(status);
	}

	//all measurements are over
	else if(currentProgress==100){
#ifdef VERBOSE
		cout << "Acquisition Finished" << endl << endl;
#endif
	}

	StartStopDaqToggle(true);
	//this lets the measurement tab know its over, and to enable tabs
	emit UpdatingPlotFinished();

	//calculate s curve inflection point
	int l1=0,l2=0,j;
	if((histogram)&&(histogramArgument != qDefs::Intensity)){
		for(j=0;j<histogramSamples.size()-2;j++){
			l1 = histogramSamples[j+1].value - histogramSamples[j].value;
			l2 = histogramSamples[j+2].value - histogramSamples[j+1].value;
			if(l1 > l2){
				cout << "***** s curve inflectionfound at " << histogramSamples[j].interval.maxValue() << ""
						"or j at " << j << " with l1 " << l1 << " and l2 " << l2 << endl;
			}
		}
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qDrawPlot::GetProgressCallBack(double currentProgress, void *this_pointer){
	((qDrawPlot*)this_pointer)->progress= currentProgress;
	return 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::ShowAcquisitionErrorMessage(QString status){
	if(!alreadyDisplayed){
		alreadyDisplayed = true;
		qDefs::Message(qDefs::WARNING,string("<nobr>The acquisiton has ended abruptly. "
				"Current Detector Status: ")+status.toAscii().constData()+
				string(".</nobr>"),"qDrawPlot::ShowAcquisitionErrorMessage");
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qDrawPlot::GetMeasurementFinishedCallBack(int currentMeasurementIndex, int fileIndex, void *this_pointer){
	((qDrawPlot*)this_pointer)->MeasurementFinished(currentMeasurementIndex, fileIndex);
	return 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qDrawPlot::MeasurementFinished(int currentMeasurementIndex, int fileIndex){
#ifdef VERBOSE
	cout << "Entering Measurement Finished with currentMeasurement " << currentMeasurementIndex << " and fileIndex " << fileIndex << endl;
#endif

	//to make sure it plots the last frame
	while(plotRequired){
		usleep(2000);
	}

	currentMeasurement = currentMeasurementIndex+1;
	currentFileIndex = fileIndex;
#ifdef VERBOSE
	cout << "currentMeasurement:" << currentMeasurement << endl;
#endif
	emit SetCurrentMeasurementSignal(currentMeasurement);
	SetupMeasurement();
	/*if((myDet->setReceiverOnline()==slsDetectorDefs::ONLINE_FLAG) && (myDet->getFramesCaughtByReceiver() == 0))
		boxPlot->setTitle("OLD_plot.raw");*/
	return 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SelectPlot(int i){ //1 for 1D otherwise 2D
	if(i==1){
		//Clear1DPlot(); it clears the last measurement
		plot1D->SetXTitle(histXAxisTitle.toAscii().constData());
		plot1D->SetYTitle(histYAxisTitle.toAscii().constData());
		plot1D->show();
		plot2D->hide();
		boxPlot->setFlat(false);
		plot_in_scope=1;
		layout->addWidget(histFrameIndexTitle,0,0);
		plotLayout->setContentsMargins(10,10,10,10);
	}else{
		plot2D->SetXTitle(imageXAxisTitle);
		plot2D->SetYTitle(imageYAxisTitle);
		plot2D->SetZTitle(imageZAxisTitle);
		plot1D->hide();
		plot2D->show();
		boxPlot->setFlat(true);
		plot_in_scope=2;
		histFrameIndexTitle->setText("");
		layout->removeWidget(histFrameIndexTitle);
		plotLayout->setContentsMargins(0,0,0,0);

	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::Clear1DPlot(){
	for(QVector<SlsQtH1D*>::iterator h = plot1D_hists.begin(); h!=plot1D_hists.end();h++){
		(*h)->Detach(plot1D);
		//do not delete *h or h.
	}

	plotHistogram->detach();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::UpdatePlot(){
#ifdef VERYVERBOSE
	cout << "Entering UpdatePlot function" << endl;
#endif
	// only if no plot isnt enabled
	if(plotEnable && plotRequired){
		LockLastImageArray();
		//so that it doesnt plot every single thing
#ifdef VERYVERBOSE
			cprintf(GREEN,"Updating Plot\n");
#endif
			//so as to not plot it again and to let measurment finished know its done plotting it
			//1-d plot stuff
			if(plot_in_scope==1){
#ifdef VERYVERBOSE
					cout << "Last Image Number:" << lastImageNumber << endl;
#endif
					if(histNBins){
						Clear1DPlot();
						plot1D->SetXTitle(histXAxisTitle.toAscii().constData());
						plot1D->SetYTitle(histYAxisTitle.toAscii().constData());

						//histogram
						if(histogram){
							plotHistogram->setData(new QwtIntervalSeriesData(histogramSamples));
							plotHistogram->setPen(QPen(Qt::red));
							plotHistogram->setBrush(QBrush(Qt::red,Qt::Dense4Pattern));//Qt::SolidPattern
							histFrameIndexTitle->setText(GetHistTitle(0));
							plotHistogram->attach(plot1D);
							//refixing all the zooming

							plot1D->SetXMinMax(startPixel,endPixel);
							plot1D->SetYMinMax(0,plotHistogram->boundingRect().height());
							plot1D->SetZoomBase(startPixel,0,endPixel-startPixel,plotHistogram->boundingRect().height());

						}
						//not histogram
						else{
							for(int hist_num=0;hist_num<(int)nHists;hist_num++){
								SlsQtH1D*  h;
								if(hist_num+1>plot1D_hists.size()){
									if(anglePlot)
										plot1D_hists.append(h=new SlsQtH1D("",histNBins,histXAngleAxis,histYAngleAxis));
									else
										plot1D_hists.append(h=new SlsQtH1D("",histNBins,histXAxis,GetHistYAxis(hist_num)));
									h->SetLineColor(hist_num);
								}else{
									h=plot1D_hists.at(hist_num);
									if(anglePlot)
										h->SetData(histNBins,histXAngleAxis,histYAngleAxis);
									else
										h->SetData(histNBins,histXAxis,GetHistYAxis(hist_num));
								}
								SetStyle(h);
								histFrameIndexTitle->setText(GetHistTitle(0));
								//h->setTitle(GetHistTitle(hist_num));
								h->Attach(plot1D);
								//refixing all the zooming
								//if((firstPlot) || (anglePlot)){
									/*plot1D->SetXMinMax(h->minXValue(),h->maxXValue());
									plot1D->SetYMinMax(h->minYValue(),h->maxYValue());
									plot1D->SetZoomBase(h->minXValue(),h->minYValue(),
											h->maxXValue()-h->minXValue(),h->maxYValue()-h->minYValue());*/
								//	firstPlot = false;
								//}
							}

							/**moved from below (had applied to histograms as well) to here, */
							// update range if required
							if(XYRangeChanged){
								if(!IsXYRange[qDefs::XMINIMUM])		XYRangeValues[qDefs::XMINIMUM]= plot1D->GetXMinimum();
								if(!IsXYRange[qDefs::XMAXIMUM])		XYRangeValues[qDefs::XMAXIMUM]= plot1D->GetXMaximum();
								if(!IsXYRange[qDefs::YMINIMUM])		XYRangeValues[qDefs::YMINIMUM]= plot1D->GetYMinimum();
								if(!IsXYRange[qDefs::YMAXIMUM])		XYRangeValues[qDefs::YMAXIMUM]= plot1D->GetYMaximum();
								plot1D->SetXMinMax(XYRangeValues[qDefs::XMINIMUM],XYRangeValues[qDefs::XMAXIMUM]);
								plot1D->SetYMinMax(XYRangeValues[qDefs::YMINIMUM],XYRangeValues[qDefs::YMAXIMUM]);
								//Should not be reset for histogram,
								//that is the only way to zoom in (new plots are zoomed out as its different each time)
								if(!histogram)
									XYRangeChanged	= false;
							}
							/**moved from below (had applied to histograms as well) to here, */
							//Display Statistics
							if(displayStatistics){
								double min=0,max=0,sum=0;
								if(anglePlot)
									GetStatistics(min,max,sum,histYAngleAxis,histNBins);
								else
									GetStatistics(min,max,sum,histYAxis[0],histNBins);
								lblMinDisp->setText(QString("%1").arg(min));
								lblMaxDisp->setText(QString("%1").arg(max));
								lblSumDisp->setText(QString("%1").arg(sum));
							}
						}

						if(saveAll) SavePlotAutomatic();

					}
			}//2-d plot stuff
			else{
				if(lastImageArray){
					if(nPixelsX>0&&nPixelsY>0){
						plot2D->GetPlot()->SetData(nPixelsX,-0.5,nPixelsX-0.5,nPixelsY,startPixel,endPixel,lastImageArray);
						plot2D->setTitle(GetImageTitle());
						plot2D->SetXTitle(imageXAxisTitle);
						plot2D->SetYTitle(imageYAxisTitle);
						plot2D->SetZTitle(imageZAxisTitle);
						plot2D->UpdateNKeepSetRangeIfSet(); //keep a "set" z range, and call Update();
					}
					// update range if required
					if(XYRangeChanged){
						if(!IsXYRange[qDefs::XMINIMUM])			XYRangeValues[qDefs::XMINIMUM]= plot2D->GetPlot()->GetXMinimum();
						if(!IsXYRange[qDefs::XMAXIMUM])			XYRangeValues[qDefs::XMAXIMUM]= plot2D->GetPlot()->GetXMaximum();
						if(!IsXYRange[qDefs::YMINIMUM])			XYRangeValues[qDefs::YMINIMUM]= plot2D->GetPlot()->GetYMinimum();
						if(!IsXYRange[qDefs::YMAXIMUM])			XYRangeValues[qDefs::YMAXIMUM]= plot2D->GetPlot()->GetYMaximum();
						plot2D->GetPlot()->SetXMinMax(XYRangeValues[qDefs::XMINIMUM],XYRangeValues[qDefs::XMAXIMUM]);
						plot2D->GetPlot()->SetYMinMax(XYRangeValues[qDefs::YMINIMUM],XYRangeValues[qDefs::YMAXIMUM]);
						XYRangeChanged	= false;
					}
					plot2D->GetPlot()->Update();
					//Display Statistics
					if(displayStatistics){
						double min=0,max=0,sum=0;
						GetStatistics(min,max,sum,lastImageArray,nPixelsX*nPixelsY);
						lblMinDisp->setText(QString("%1").arg(min));
						lblMaxDisp->setText(QString("%1").arg(max));
						lblSumDisp->setText(QString("%1").arg(sum));
					}
					if(saveAll) SavePlotAutomatic();
				}
			}
			//set plot title
			boxPlot->setTitle(plotTitle);
			//to notify the measurement finished when its done
			plotRequired = false;
		UnlockLastImageArray();
	}

#ifdef VERYVERBOSE
	cout << "Exiting UpdatePlot function" << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::ClonePlot(){
	int i;

	//check for space for more clone widget references
	bool found = false;
	for(i=0;i<MAXCloneWindows;i++)
		if(!winClone[i]){
			found=true;
			break;
		}
	// no space
	if(!found){
		cout << "Too many clones" << endl;
		exit(-1);
	}


	//get file path while acquisition runnign without accessing shared memory
	string sFilePath;
	if(running) sFilePath = filePath.toAscii().constData();
	else {
		sFilePath = myDet->getFilePath();
		qDefs::checkErrorMessage(myDet,"qDrawPlot::ClonePlot");
	}



	LockLastImageArray();


	// create clone & copy data
	if(plot_in_scope==1){
		winClone[i] = new qCloneWidget(this,i,boxPlot->title(),histXAxisTitle,histYAxisTitle,"",
				(int)plot_in_scope,sFilePath,displayStatistics,lblMinDisp->text(),lblMaxDisp->text(),lblSumDisp->text());
		if(!anglePlot)
			winClone[i]->SetCloneHists((int)nHists,histNBins,histXAxis,histYAxis,histTitle,lines,markers);
		else
			winClone[i]->SetCloneHists((int)nHists,histNBins,histXAngleAxis,histYAngleAxis,histTitle,lines,markers);

	}else{
		winClone[i] = new qCloneWidget(this,i,boxPlot->title(),imageXAxisTitle, imageYAxisTitle, imageZAxisTitle,
				(int)plot_in_scope,sFilePath,displayStatistics,lblMinDisp->text(),lblMaxDisp->text(),lblSumDisp->text());
		winClone[i]->SetCloneHists2D(nPixelsX,-0.5,nPixelsX-0.5,nPixelsY,startPixel,endPixel,lastImageArray);
	}

	// update range
	found =false;
	for(int index=0;index<4;index++)
		if(IsXYRange[index]){
			found=true;
			break;
		}
	if(found)
		winClone[i]->SetRange(IsXYRange,XYRangeValues);


	UnlockLastImageArray();

	winClone[i]->show();

	// to remember which all clone widgets were closed
	connect(winClone[i], SIGNAL(CloneClosedSignal(int)),this, SLOT(CloneCloseEvent(int)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SaveClones(){
	char errID[200];
	string errMessage= "The Snapshots with ID's: ";
	bool success = true;
	for(int i=0;i<MAXCloneWindows;i++)
		if(winClone[i]){
			if(winClone[i]->SavePlotAutomatic()){
				success = false;
				sprintf(errID,"%d",i);
				errMessage.append(string(errID)+string(", "));
			}
		}
	if(success)
		qDefs::Message(qDefs::INFORMATION,"The Snapshots have all been saved successfully in .png.","Dock");
	else
		qDefs::Message(qDefs::WARNING,errMessage + string("were not saved."),"qDrawPlot::SaveClones");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::CloseClones(){
	for(int i=0;i<MAXCloneWindows;i++)
		if(winClone[i])
			winClone[i]->close();

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::CloneCloseEvent(int id){
	winClone[id]=0;
#ifdef VERBOSE
	cout << "Closing Clone Window id:" << id << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SavePlot(){
	// render image
	QImage savedImage(size().width(),size().height(),QImage::Format_RGB32);
	QPainter painter(&savedImage);
	render(&painter);

	QString fName;
	if(running) fName = filePath;
	else {
		fName = QString(myDet->getFilePath().c_str());
		qDefs::checkErrorMessage(myDet,"qDrawPlot::SavePlot");
	}

	if(boxPlot->title().contains('.')){
		fName.append(QString('/')+boxPlot->title());
		fName.replace(".dat",".png");
		fName.replace(".raw",".png");
	}else  fName.append(QString("/Image.png"));

	fName = QFileDialog::getSaveFileName(0,tr("Save Image"),fName,tr("PNG Files (*.png);;XPM Files(*.xpm);;JPEG Files(*.jpg)"),0,QFileDialog::ShowDirsOnly);

	if (!fName.isEmpty())
		if(savedImage.save(fName))
			qDefs::Message(qDefs::INFORMATION,"The Image has been successfully saved","qDrawPlot::SavePlot");
		else
			qDefs::Message(qDefs::WARNING,"Attempt to save image failed.\n"
    				"Formats: .png, .jpg, .xpm.","qDrawPlot::SavePlot");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SavePlotAutomatic(){
	//no need to save the same plot many times
	if((currentFrame>lastSavedFrame)&&(currentMeasurement>=lastSavedMeasurement)){

		QString qFilePath;
		if(running) qFilePath = filePath;
		else {
			qFilePath = QString(myDet->getFilePath().c_str());
			qDefs::checkErrorMessage(myDet,"qDrawPlot::SavePlotAutomatic");
		}


		lastSavedFrame = currentFrame;
		lastSavedMeasurement = currentMeasurement;
		char cID[10];
		sprintf(cID,"%d",lastSavedFrame);
		//title
		QString fName = qFilePath;
		if(boxPlot->title().contains('.')){
			fName.append(QString('/')+boxPlot->title());
			fName.replace(".dat",".png");
			fName.replace(".raw",".png");
		}else  fName.append(QString("/Image_unknown_title.png"));
		//save
		QImage img(size().width(),size().height(),QImage::Format_RGB32);
		QPainter painter(&img);
		render(&painter);
		//if error while saving
		if(!img.save(fName)){
			//mention the error only the first time
			if(!saveError){
				//so it doesnt repeat again
				saveError = true;
				connect(this,SIGNAL(saveErrorSignal(QString)),this,SLOT(ShowSaveErrorMessage(QString)));
				emit saveErrorSignal(fName);
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::ShowSaveErrorMessage(QString fileName){
	qDefs::Message(qDefs::WARNING,string("Automatic Saving: Could not save the first file:\n")+
			string(fileName.toAscii().constData()) + string("\n\nNote: Will not show future file save errors for this acquisition."),"qDrawPlot::ShowSaveErrorMessage");

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SetPersistency(int val){
	for(int i=0;i<=val;i++)
		if(!histYAxis[i]) histYAxis[i] = new double [nPixelsX];
	persistency = val;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::EnablePlot(bool enable){
#ifdef VERBOSE
	cout << "Plotting set to:" << enable << endl;
#endif
	plotEnable = enable;
	//if no plot, cant do setting range.
	// not true vice versa where plot was false and now set it to true
	Clear1DPlot();

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::DisableZoom(bool disable){
	if(plot_in_scope==1)
		plot1D->DisableZoom(disable);
	else
		plot2D->GetPlot()->DisableZoom(disable);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qDrawPlot::UpdateTrimbitPlot(bool fromDetector,bool Histogram){
	int ret,i,actualPixelsX;
	double min=0,max=0,sum=0;
#ifdef VERBOSE
	if(fromDetector)	cout << "Geting Trimbits from Detector" << endl;
	else				cout << "Getting Trimbits from Shared Memory" << endl;
#endif

	LockLastImageArray();

	if(detType == slsDetectorDefs::MYTHEN){

		//get trimbits
		actualPixelsX = myDet->getTotalNumberOfChannels(slsDetectorDefs::X);
		if(histTrimbits) delete [] histTrimbits; histTrimbits = new double[actualPixelsX];
		ret = myDet->getChanRegs(histTrimbits,fromDetector);
		//	cout << "got it!" << endl;
		if(!ret){
			qDefs::Message(qDefs::WARNING,"No Trimbit data found in shared memory.","qDrawPlot::UpdateTrimbitPlot");
			UnlockLastImageArray();
			return qDefs::FAIL;
		}
#ifdef VERBOSE
		cout << "Got Trimbits" << endl;
#endif

		qDefs::checkErrorMessage(myDet,"qDrawPlot::UpdateTrimbitPlot");

		//clear/select plot and set titles
		Select1DPlot();
		Clear1DPlot();

		//Display Statistics
		if(displayStatistics){
			GetStatistics(min,max,sum,histTrimbits,actualPixelsX);
			lblMinDisp->setText(QString("%1").arg(min));
			lblMaxDisp->setText(QString("%1").arg(max));
			lblSumDisp->setText(QString("%1").arg(sum));
		}


		if(!Histogram){
			cout << "Data Graph:" << nPixelsX << endl;

			//initialize
			nPixelsX = actualPixelsX;
			if(histXAxis)		delete [] histXAxis;	histXAxis 	= new double [nPixelsX];
			if(histYAxis[0])	delete [] histYAxis[0]; histYAxis[0]= new double [nPixelsX];
			//initializing
			for(unsigned int px=0;px<(int)nPixelsX;px++)	histXAxis[px] = px;
			for(i=0;i<nPixelsX;i++)							histYAxis[0][i]  = 0;

			//data
			memcpy(histYAxis[0],histTrimbits,nPixelsX*sizeof(double));
			//title
			boxPlot->setTitle("Trimbits_Plot_Data Graph");
			plot1D->SetXTitle("Channel Number");
			plot1D->SetYTitle("Trimbits");
			//set plot parameters
			plot1D->SetXMinMax(0,nPixelsX);
			/*plot1D->SetYMinMax(0,plotHistogram->boundingRect().height()); plot1D->SetZoomBase(0,0,nPixelsX,plot1D->GetYMaximum());*/
			//for some reason this plothistogram works as well.
			plot1D->SetZoomBase(0,0,nPixelsX,plotHistogram->boundingRect().height());
			SlsQtH1D*  h;
			plot1D_hists.append(h=new SlsQtH1D("",nPixelsX,histXAxis,histYAxis[0]));
			h->SetLineColor(0);
			histFrameIndexTitle->setText(GetHistTitle(0));
			//attach plot
			h->Attach(plot1D);
			//refixing all the zooming
			/*plot1D->SetXMinMax(h->minXValue(),h->maxXValue());
			plot1D->SetYMinMax(h->minYValue(),h->maxYValue());
			plot1D->SetZoomBase(h->minXValue(),h->minYValue(),
					h->maxXValue()-h->minXValue(),h->maxYValue()-h->minYValue());*/
		}

		else{
			cout << "Histogram: " << TRIM_HISTOGRAM_XMAX << endl;

			//create intervals
			histogramSamples.resize(TRIM_HISTOGRAM_XMAX+1);
			for(i=0; i<TRIM_HISTOGRAM_XMAX+1; i++){
				histogramSamples[i].interval.setInterval(i,i+1);
				histogramSamples[i].value = 0;
			}

			//fill histogram values
			int value = 0;
			for(i=0;i<actualPixelsX;i++){
				if( (histTrimbits[i] <= TRIM_HISTOGRAM_XMAX) && (histTrimbits[i] >= 0)){//if(histogramSamples[j].interval.contains(data->values[i]))
					value = (int) histTrimbits[i];
					histogramSamples[value].value += 1;
				}
				else cout<<"OUT OF BOUNDS:"<<i<<"-"<<histTrimbits[i]<<endl;
			}

			//plot
			boxPlot->setTitle("Trimbits_Plot_Histogram");
			plot1D->SetXTitle("Trimbits");
			plot1D->SetYTitle("Frequency");
			plotHistogram->setData(new QwtIntervalSeriesData(histogramSamples));
			plotHistogram->setPen(QPen(Qt::red));
			plotHistogram->setBrush(QBrush(Qt::red,Qt::Dense4Pattern));//Qt::SolidPattern
			histFrameIndexTitle->setText(GetHistTitle(0));
			plotHistogram->attach(plot1D);
			//refixing all the zooming
			plot1D->SetXMinMax(0,TRIM_HISTOGRAM_XMAX+1);
			plot1D->SetYMinMax(0,plotHistogram->boundingRect().height());
			plot1D->SetZoomBase(0,0,actualPixelsX,plotHistogram->boundingRect().height());
		}
	}


	/**needs to be changed */
	else if(detType == slsDetectorDefs::EIGER){

		//defining axes
		nPixelsX = 100;/**??*/
		nPixelsY = 100;
		if(lastImageArray) delete [] lastImageArray; lastImageArray = new double[nPixelsY*nPixelsX];
		//initializing 2d array
		for(int py=0;py<(int)nPixelsY;py++)
			for(int px=0;px<(int)nPixelsX;px++)
				lastImageArray[py*nPixelsX+px] = 0;
		//get trimbits
		ret = 1;/*myDet->getChanRegs(lastImageArray,fromDetector);*/
		if(!ret){
			qDefs::Message(qDefs::WARNING,"No Trimbit data found in shared memory.","qDrawPlot::UpdateTrimbitPlot");
			UnlockLastImageArray();
			return qDefs::FAIL;
		}
		//clear/select plot and set titles
		Select2DPlot();
		plot2D->GetPlot()->SetData(nPixelsX,-0.5,nPixelsX-0.5,nPixelsY,-0.5,nPixelsY-0.5,lastImageArray);
		plot2D->setTitle("Image");
		plot2D->SetXTitle("Pixel");
		plot2D->SetYTitle("Pixel");
		plot2D->SetZTitle("Trimbits");
		plot2D->UpdateNKeepSetRangeIfSet();
#ifdef VERBOSE
		cout << "Trimbits Plot updated" << endl;
#endif

		//Display Statistics
		if(displayStatistics){
			GetStatistics(min,max,sum,lastImageArray,nPixelsX*nPixelsY);
			lblMinDisp->setText(QString("%1").arg(min));
			lblMaxDisp->setText(QString("%1").arg(max));
			lblSumDisp->setText(QString("%1").arg(sum));
		}

	}

	UnlockLastImageArray();
#ifdef VERBOSE
		cout << "Trimbits Plot updated" << endl;
#endif
	return qDefs::OK;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SetPedestal(bool enable){
#ifdef VERBOSE
	cout << "Setting Pedestal to " << enable <<  endl;
#endif
	if(enable){
		pedestal = true;
		if(pedestalVals == 0)
			RecalculatePedestal();
	}else{
		pedestal = false;
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::RecalculatePedestal(){
#ifdef VERBOSE
	cout << "Recalculating Pedestal" <<  endl;
#endif
	LockLastImageArray();
	startPedestalCal = 1;
	pedestalCount = 0;

	//create array
	if(pedestalVals) 		delete [] pedestalVals;		pedestalVals 		= new double[nPixelsX*nPixelsY];
	if(tempPedestalVals) 	delete [] tempPedestalVals;	tempPedestalVals 	= new double[nPixelsX*nPixelsY];
	//reset all values
	for(unsigned int px=0;px<(nPixelsX*nPixelsY);px++)
		pedestalVals[px] = 0;
	for(unsigned int px=0;px<(nPixelsX*nPixelsY);px++)
		tempPedestalVals[px] = 0;
	UnlockLastImageArray();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SetAccumulate(bool enable){
#ifdef VERBOSE
	cout << "Setting Accumulate to " << enable <<  endl;
#endif
	accumulate = enable;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::ResetAccumulate(){
#ifdef VERBOSE
	cout << "Resetting Accumulation" <<  endl;
#endif
	LockLastImageArray();
	resetAccumulate = true;
	UnlockLastImageArray();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SetPlotTimer(double time){
	timerValue = time;
	if(myDet->setReceiverOnline()==slsDetectorDefs::ONLINE_FLAG){
		time = myDet->setReceiverReadTimer(timerValue);
#ifdef VERBOSE
		cout << "Receiver read timer set to : " << time << endl;
#endif
		qDefs::checkErrorMessage(myDet,"qDrawPlot::SetPlotTimer");
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SetFrameFactor(int frame){
	frameFactor = frame;
	if(myDet->setReceiverOnline()==slsDetectorDefs::ONLINE_FLAG){
		frame = myDet->setReadReceiverFrequency(1,frame);
		/*if(frame > 0) frameFactor = 1;*//**what is this*/
#ifdef VERBOSE
		cout << "Receiver read frequency set to : " << frame << endl;
#endif
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------



void qDrawPlot::UpdateAfterCloning(bool points, bool logy, bool interpolate, bool contour, bool logz){
#ifdef VERBOSE
	cout  << endl << "**Updating Plot After Cloning" << endl;
#endif

	//1d
	if(plot_in_scope == 1){
		SetMarkers(points);
		emit LogySignal(logy);
	}
	//2d
	else{
		emit InterpolateSignal(interpolate);
		emit ContourSignal(contour);
		emit LogzSignal(logz);
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SetBinary(bool enable, int from, int to){
#ifdef VERBOSE
	if(!enable)
		cout << "Disabling Binary output " << endl;
	else
		cout << "Enabling Binary output from " << from << " to " << to << endl;
#endif
	binary = enable;
	binaryFrom = from;
	binaryTo = to;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::DisplayStatistics(bool enable){
#ifdef VERBOSE
	if(!enable)
		cout << "Disabling Statistics Display" << endl;
	else
		cout << "Enabling Statistics Display" << endl;
#endif
	if(enable)	widgetStatistics->show();
	else widgetStatistics->hide();

	displayStatistics = enable;
	lblMinDisp->setText("-");
	lblMaxDisp->setText("-");
	lblSumDisp->setText("-");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::GetStatistics(double &min, double &max, double &sum, double* array, int size){
#ifdef VERYVERBOSE
	cout << "Calculating Statistics" << endl;
#endif

	for(int i=0; i < size; i++){
		//calculate min
		if(array[i] < min)
			min = array[i];
		//calculate max
		if(array[i] > max)
			max = array[i];
		//calculate sum
		sum += array[i];
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------
