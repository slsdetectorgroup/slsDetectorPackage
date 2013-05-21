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
// C++ Include Headers
#include <iostream>
#include <string>
#include <sstream>
using namespace std;



//-------------------------------------------------------------------------------------------------------------------------------------------------


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
	delete myDet;
	for(int i=0;i<MAXCloneWindows;i++) if(winClone[i]) delete winClone[i];
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SetupWidgetWindow(){
#ifdef VERBOSE
	cout << "Setting up plot variables" << endl;
#endif

	// Depending on whether the detector is 1d or 2d
	switch(myDet->getDetectorsType()){
	case slsDetectorDefs::MYTHEN:	originally2D = false;	break;
	case slsDetectorDefs::EIGER:	originally2D = true;	break;
	case slsDetectorDefs::GOTTHARD:	originally2D = false; 	break;
	case slsDetectorDefs::MOENCH:	originally2D = true; 	break;
	default:
		cout << "ERROR: Detector Type is Generic" << endl;
		exit(-1);
	}


	data_pause_over = true;//to get the first image

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
	/*imageTitle.assign("Start Image");*/
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
	last_plot_number = 0;

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
	anglePlot = false;
	alreadyDisplayed =  false;

	//filepath and file name
	filePath = QString(myDet->getFilePath().c_str());
	fileName = QString(myDet->getFileName().c_str());

	backwardScanPlot = false;
	fileSaveEnable= myDet->enableWriteToFile();

	//pedestal
	resetPedestal = true;
	pedestalVals = 0;
	pedestalCount = -1;

	if(myDet->getDetectorsType()==slsDetectorDefs::GOTTHARD)
		pedestalCount = 0;

	clientInitiated = false;

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
	plot_update_timer = new QTimer(this);
	connect(plot_update_timer, SIGNAL(timeout()), this, SLOT(UpdatePlot()));
	data_pause_timer = new QTimer(this);
	connect(data_pause_timer, SIGNAL(timeout()), this, SLOT(UpdatePause()));


	// setting default plot titles and settings
	plot1D = new SlsQt1DPlot(boxPlot);

		plot1D->setFont(QFont("Sans Serif",9,QFont::Normal));
		plot1D->SetXTitle(histXAxisTitle.toAscii().constData());
		plot1D->SetYTitle(histYAxisTitle.toAscii().constData());
		plot1D->hide();

		SlsQtH1D*  h;
		histNBins = nPixelsX;
		nHists = 1;
		if(histXAxis)    delete [] histXAxis;	histXAxis    = new double [nPixelsX];
		if(histYAxis[0]) delete [] histYAxis[0];histYAxis[0] = new double [nPixelsX];
		for(unsigned int px=0;px<(int)nPixelsX;px++)	{histXAxis[px]  = px;histYAxis[0][px] = 0;}
		Clear1DPlot();
		plot1D->SetXTitle("X Axis");
		plot1D->SetYTitle("Y Axis");
		plot1D_hists.append(h=new SlsQtH1D("",histNBins,histXAxis,histYAxis[0]));
		h->SetLineColor(1);
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

	qDefs::checkErrorMessage(myDet);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::Initialization(){
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
//#ifdef VERYVERBOSE
	cout << "Entering StartStopDaqToggle(" << stop_if_running << ")" <<endl;
//#endif
	//static bool running = 1;
	if(running){ //stopping
		StartDaq(false);
		running=!running;
	}else if(!stop_if_running){ //then start

		// Reset Current Measurement
		currentMeasurement = 0;
		emit SetCurrentMeasurementSignal(currentMeasurement);


		//to get the first image
		data_pause_over = true;
		//in case of error message
		alreadyDisplayed = false;

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

		qDefs::checkErrorMessage(myDet);
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
		UpdatePlot();
	}else{
#ifdef VERBOSE
		cout << "Start Daq(false) function" << endl;
#endif
		StopDaqForGui();
		StopUpdatePlot();
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

		//refixing all the min and max for all scans
		if (scanArgument == qDefs::None);
		else
			plot2D->GetPlot()->SetZoom(-0.5,startPixel,nPixelsX,endPixel-startPixel);


		cout << "Starting new acquisition thread ...." << endl;
		// Start acquiring data from server
		if(!firstTime) pthread_join(gui_acquisition_thread,NULL);//wait until he's finished, ie. exits
		pthread_create(&gui_acquisition_thread, NULL,DataStartAcquireThread, (void*) this);
		firstTime = false;
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
#ifdef VERBOSE
	cout << "SetScanArgument function:" << scanArg << " running:" << running << endl;
#endif
	scanArgument = scanArg;

	if(plot_in_scope==1) Clear1DPlot();


	LockLastImageArray();


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
	}

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


	UnlockLastImageArray();

	qDefs::checkErrorMessage(myDet);

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SetupMeasurement(){
#ifdef VERBOSE
	cout << "SetupMeasurement function:" << running << endl;
#endif

	LockLastImageArray();

	// Defaults
	if(!running)
		stop_signal = 0;
	currentFrame = 0;
	//for 2d scans
	currentScanDivLevel = 0;
	//if(plot_in_scope==2)
		if(!running)	lastImageNumber = 0;/**Just now */

	//initializing 2d array
	for(int py=0;py<(int)nPixelsY;py++)
		for(int px=0;px<(int)nPixelsX;px++)
			lastImageArray[py*nPixelsX+px] = 0;

	//1d with no scan
	if ((!originally2D) && (scanArgument==qDefs::None)){
		if(!running){
			maxPixelsY = 100;
			minPixelsY = 0;
			startPixel = -0.5;
			endPixel = nPixelsY-0.5;
		}
	}
	else {
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
		}//file index
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
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void* qDrawPlot::DataStartAcquireThread(void *this_pointer){
	((qDrawPlot*)this_pointer)->myDet->acquire(1);
	return this_pointer;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qDrawPlot::GetDataCallBack(detectorData *data, int fIndex, void *this_pointer){
	((qDrawPlot*)this_pointer)->GetData(data,fIndex);
	return 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qDrawPlot::GetData(detectorData *data,int fIndex){
#ifdef VERYVERBOSE
	cout << "******Entering GetDatafunction********" << endl;
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
		if(string(data->fileName).empty()){
			cout << "Received empty file name. Exiting function without updating data for plot." << endl;
			return -1;
		}
#ifdef VERYVERBOSE
		cout << "progress:" << progress << endl;
#endif

		// secondary title necessary to differentiate between frames when not saving data
		char temp_title[2000];
		//findex is used because in the receiver, you cannot know the frame index as many frames are in 1 file.
		if(fIndex!=-1){
			currentFrameIndex=fIndex;
			sprintf(temp_title,"#%d",fIndex);
		}else{
			if(fileSaveEnable)	strcpy(temp_title,"#%d");
			else		sprintf(temp_title,"",currentFrame);
		}


		//Plot Disabled
		if(!plotEnable) 	return 0;


		//angle plotting
		if(anglePlot){
			while(1){
				if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
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
						memcpy(histXAngleAxis,data->angles,nAnglePixelsX*sizeof(double));
						memcpy(histYAngleAxis,data->values,nAnglePixelsX*sizeof(double));
						SetHistXAxisTitle("Angles");
					}
					pthread_mutex_unlock(&(last_image_complete_mutex));
					break;
				}
			}
			currentFrame++;
			return 0;
		}



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
				data_pause_timer->start((int)(PLOT_TIMER_MS/2));
			}
		}




//if scan
		//alframes
		if(scanArgument==qDefs::AllFrames){
			while(1){
				if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
					//set title
					plotTitle=QString(plotTitle_prefix)+QString(data->fileName).section('/',-1);
					//variables
					lastImageNumber= currentFrame+1;
					//title
					imageTitle = temp_title;
					//copy data
					memcpy(lastImageArray+(currentScanDivLevel*nPixelsX),data->values,nPixelsX*sizeof(double));
					pthread_mutex_unlock(&(last_image_complete_mutex));
					break;
				}
			}
			currentFrame++;
			currentScanDivLevel++;
			return 0;
		}
		//file index
		if(scanArgument==qDefs::FileIndex){
			while(1){
				if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
					//set title
					plotTitle=QString(plotTitle_prefix)+QString(data->fileName).section('/',-1);
					//variables
					if(currentFrameIndex == 0) currentScanDivLevel = 0;
					lastImageNumber= currentFrame+1;
					//title
					imageTitle = temp_title;
					//copy data
					for(unsigned int px=0;px<nPixelsX;px++)	lastImageArray[currentScanDivLevel*nPixelsX+px] += data->values[px];
					pthread_mutex_unlock(&(last_image_complete_mutex));
					break;
				}
			}
			currentFrame++;
			currentScanDivLevel++;
			return 0;
		}
		//level0
		if(scanArgument==qDefs::Level0){
			while(1){
				if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
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
					pthread_mutex_unlock(&(last_image_complete_mutex));
					break;
				}
			}
			currentFrame++;
			return 0;
		}
		//level1
		if(scanArgument==qDefs::Level1){
			while(1){
				if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
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
					pthread_mutex_unlock(&(last_image_complete_mutex));
					break;
				}
			}
			currentFrame++;
			return 0;
		}



		//normal measurement or 1d scans
		if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
			//set title
			plotTitle=QString(plotTitle_prefix)+QString(data->fileName).section('/',-1);
			// only if you got the lock, do u need to remember lastimagenumber to plot
			lastImageNumber= currentFrame+1;

			//1d
			if(plot_in_scope==1){
				// Titles
				histTitle[0] = temp_title;
				// Persistency
				if(currentPersistency < persistency)currentPersistency++;
				else currentPersistency=persistency;
				nHists = currentPersistency+1;
				histNBins = nPixelsX;

				// copy data
				for(int i=currentPersistency;i>0;i--)
					memcpy(histYAxis[i],histYAxis[i-1],nPixelsX*sizeof(double));

				//normal data
				if(resetPedestal){
					memcpy(histYAxis[0],data->values,nPixelsX*sizeof(double));
				}else{
					//start adding frames to get to the pedestal value
					if(pedestalCount<NUM_PEDESTAL_FRAMES){
						for(unsigned int px=0;px<nPixelsX;px++)
							pedestalVals[px] += data->values[px];
						memcpy(histYAxis[0],data->values,nPixelsX*sizeof(double));
					}
					//calculate the pedestal value
					else if(pedestalCount==NUM_PEDESTAL_FRAMES){
						cout << "Pedestal Calculated" << endl;
						for(unsigned int px=0;px<nPixelsX;px++)
							pedestalVals[px] = pedestalVals[px]/(double)NUM_PEDESTAL_FRAMES;
						memcpy(histYAxis[0],data->values,nPixelsX*sizeof(double));
					}
					//use this pedestal value henceforth
					else{
						for(unsigned int px=0;px<nPixelsX;px++)
							histYAxis[0][px] = data->values[px] - (pedestalVals[px]);
					}
					pedestalCount++;

				}

			}
			//2d
			else{
				// Titles
				imageTitle = temp_title;
				// manufacture data for now
				/*
				for(unsigned int px=0;px<nPixelsX;px++)
					for(unsigned int py=0;py<nPixelsY;py++)
						lastImageArray[py*nPixelsX+px] = sqrt(pow(currentFrame+1,2)*pow(double(px)-nPixelsX/2,2)/pow(nPixelsX/2,2)/pow(number_of_exposures+1,2) + pow(double(py)-nPixelsY/2,2)/pow(nPixelsY/2,2))/sqrt(2);
						*/
				// copy data
				memcpy(lastImageArray,data->values,nPixelsX*nPixelsY*sizeof(double));
			}
			pthread_mutex_unlock(&(last_image_complete_mutex));
		}
#ifdef VERYVERBOSE
		cout<<"currentframe:"<<currentFrame<<"\tcurrentframeindex:"<<currentFrameIndex<<endl;
#endif
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
	return 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qDrawPlot::AcquisitionFinished(double currentProgress, int detectorStatus){
//#ifdef VERBOSE
	cout << "\nEntering Acquisition Finished with status " ;
//#endif
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
				string(".</nobr>"),"Dock");
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qDrawPlot::GetMeasurementFinishedCallBack(int currentMeasurementIndex, int fileIndex, void *this_pointer){
	((qDrawPlot*)this_pointer)->MeasurementFinished(currentMeasurementIndex, fileIndex);
	return 0;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qDrawPlot::MeasurementFinished(int currentMeasurementIndex, int fileIndex){
//#ifdef VERBOSE
	cout << "Entering Measurement Finished with currentMeasurement " << currentMeasurementIndex << " and fileIndex " << fileIndex << endl;
//#endif
	//to make sure it plots the last frame before setting lastimagearray all to 0
	//if(plot_in_scope==2)
		usleep(500000);

	currentMeasurement = currentMeasurementIndex+1;
	currentFileIndex = fileIndex;
#ifdef VERBOSE
	cout << "currentMeasurement:" << currentMeasurement << endl;
#endif
	emit SetCurrentMeasurementSignal(currentMeasurement);
	SetupMeasurement();
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
	for(QVector<SlsQtH1D*>::iterator h = plot1D_hists.begin();
			h!=plot1D_hists.end();h++) (*h)->Detach(plot1D); //clear plot
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::UpdatePlot(){
#ifdef VERYVERBOSE
	cout << "Entering UpdatePlot function" << endl;
#endif
	plot_update_timer->stop();

	// only if no plot isnt enabled
	if(plotEnable){
		LockLastImageArray();
		//so that it doesnt plot every single thing
		if(lastImageNumber!=last_plot_number){
			//1-d plot stuff
			if(plot_in_scope==1){
				if(lastImageNumber){
#ifdef VERYVERBOSE
					cout << "Last Image Number:" << lastImageNumber << endl;
#endif
					if(histNBins){
						Clear1DPlot();
						plot1D->SetXTitle(histXAxisTitle.toAscii().constData());
						plot1D->SetYTitle(histYAxisTitle.toAscii().constData());
						for(int hist_num=0;hist_num<(int)nHists;hist_num++){
							SlsQtH1D*  h;
							if(hist_num+1>plot1D_hists.size()){
								if(anglePlot)
									plot1D_hists.append(h=new SlsQtH1D("",histNBins,histXAngleAxis,histYAngleAxis));
								else
									plot1D_hists.append(h=new SlsQtH1D("",histNBins,histXAxis,GetHistYAxis(hist_num)));
								h->SetLineColor(hist_num+1);
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
						}
						// update range if required
						if(XYRangeChanged){
							if(!IsXYRange[qDefs::XMINIMUM])		XYRangeValues[qDefs::XMINIMUM]= plot1D->GetXMinimum();
							if(!IsXYRange[qDefs::XMAXIMUM])		XYRangeValues[qDefs::XMAXIMUM]= plot1D->GetXMaximum();
							if(!IsXYRange[qDefs::YMINIMUM])		XYRangeValues[qDefs::YMINIMUM]= plot1D->GetYMinimum();
							if(!IsXYRange[qDefs::YMAXIMUM])		XYRangeValues[qDefs::YMAXIMUM]= plot1D->GetYMaximum();
							plot1D->SetXMinMax(XYRangeValues[qDefs::XMINIMUM],XYRangeValues[qDefs::XMAXIMUM]);
							plot1D->SetYMinMax(XYRangeValues[qDefs::YMINIMUM],XYRangeValues[qDefs::YMAXIMUM]);
							XYRangeChanged	= false;
						}
						if(saveAll) SavePlotAutomatic();
					}
				}
			}//2-d plot stuff
			else{
				if(lastImageArray){
					if(lastImageNumber&&last_plot_number!=(int)lastImageNumber && //there is a new plot
							nPixelsX>0&&nPixelsY>0){
						//plot2D->GetPlot()->SetData(nPixelsX,-0.5,nPixelsX-0.5,nPixelsY,-0.5,nPixelsY-0.5,lastImageArray);
						plot2D->GetPlot()->SetData(nPixelsX,-0.5,nPixelsX-0.5,nPixelsY,startPixel,endPixel,lastImageArray);
						plot2D->setTitle(GetImageTitle());
						plot2D->SetXTitle(imageXAxisTitle);
						plot2D->SetYTitle(imageYAxisTitle);
						plot2D->SetZTitle(imageZAxisTitle);
						plot2D->UpdateNKeepSetRangeIfSet(); //this will keep a "set" z range, and call Plot()->Update();
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
					if(saveAll) SavePlotAutomatic();
				}
			}
			//}
			last_plot_number=lastImageNumber;

			//set plot title
			boxPlot->setTitle(plotTitle);
		}
		UnlockLastImageArray();
	}

	//if acqq stopped before this line, it continues from here, shouldnt restart plotting timer
	if(!stop_signal){
		if(!frameFactor)
			plot_update_timer->start((int)timerValue);
		else
			plot_update_timer->start((int)PLOT_TIMER_MS);
	}
#ifdef VERYVERBOSE
	cout << "Exiting UpdatePlot function" << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::StopUpdatePlot(){
#ifdef VERYVERBOSE
	cout << "Entering StopUpdatePlot()" << endl;
#endif
	plot_update_timer->stop();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::ClonePlot(){
	int i=0;
	bool found = false;
	for(i=0;i<MAXCloneWindows;i++)
		if(!winClone[i]){
			found=true;
			break;
		}
	// no space for more clone widget references
	if(!found){
		cout << "Too many clones" << endl;
		exit(-1);
	}
	// save height to keep maintain same height of plot
	int preheight = height();

	string sFilePath;
	if(running) sFilePath = filePath.toAscii().constData();
	else {
		sFilePath = myDet->getFilePath();
		qDefs::checkErrorMessage(myDet);
	}



	LockLastImageArray();

	// create clone
	winClone[i] = new qCloneWidget(this,i,boxPlot->title(),(int)plot_in_scope,plot1D,plot2D,sFilePath);
	if(plot_in_scope==1){
		plot1D = new SlsQt1DPlot(boxPlot);
		plot1D->setFont(QFont("Sans Serif",9,QFont::Normal));
		plot1D->SetXTitle(histXAxisTitle.toAscii().constData());
		plot1D->SetYTitle(histYAxisTitle.toAscii().constData());
		plotLayout->addWidget(plot1D,0,0,1,1);
		plotLayout->setContentsMargins(10,10,10,10);
		if(running){
			// update range
			bool found =false;
			for(int index=0;index<4;index++)
				if(IsXYRange[index]){
					found=true;
					break;
				}
			if(found) winClone[i]->SetRange(IsXYRange,XYRangeValues);
			//copy data
			//LockLastImageArray();
			if(!anglePlot)
				winClone[i]->SetCloneHists((int)nHists,histNBins,histXAxis,histYAxis,histTitle,lines,markers);
			else
				winClone[i]->SetCloneHists((int)nHists,histNBins,histXAngleAxis,histYAngleAxis,histTitle,lines,markers);
			//UnlockLastImageArray();
		}
	}
	else{

		plot2D = new SlsQt2DPlotLayout(boxPlot);
		plot2D->GetPlot()->SetData(nPixelsX,-0.5,nPixelsX-0.5,nPixelsY,startPixel,endPixel,lastImageArray);
		plot2D->setFont(QFont("Sans Serif",9,QFont::Normal));
		plot2D->setTitle(GetImageTitle());
		plot2D->SetXTitle(imageXAxisTitle);
		plot2D->SetYTitle(imageYAxisTitle);
		plot2D->SetZTitle(imageZAxisTitle);
		plotLayout->addWidget(plot2D,0,0,1,1);
		plotLayout->setContentsMargins(0,0,0,0);
	}

	UnlockLastImageArray();


	setMinimumHeight(preheight);
	resize(width(),preheight);

	// update the actual plot  only if running, else it doesnt know when its over
	if(running)	UpdatePlot();
	connect(this, 		SIGNAL(InterpolateSignal(bool)),	plot2D, 	SIGNAL(InterpolateSignal(bool)));
	connect(this, 		SIGNAL(ContourSignal(bool)),		plot2D, 	SIGNAL(ContourSignal(bool)));
	connect(this, 		SIGNAL(LogzSignal(bool)),			plot2D, 	SLOT(SetZScaleToLog(bool)));
	connect(this, 		SIGNAL(LogySignal(bool)),			plot1D, 	SLOT(SetLogY(bool)));
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
		qDefs::Message(qDefs::WARNING,errMessage + string("were not saved."),"Dock");
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
		qDefs::checkErrorMessage(myDet);
	}

	if(boxPlot->title().contains('.')){
		fName.append(QString('/')+boxPlot->title());
		fName.replace(".dat",".png");
		fName.replace(".raw",".png");
	}else  fName.append(QString("/Image.png"));

	fName = QFileDialog::getSaveFileName(0,tr("Save Image"),fName,tr("PNG Files (*.png);;XPM Files(*.xpm);;JPEG Files(*.jpg)"),0,QFileDialog::ShowDirsOnly);

	if (!fName.isEmpty())
		if(savedImage.save(fName))
			qDefs::Message(qDefs::INFORMATION,"The Image has been successfully saved","Dock");
		else
			qDefs::Message(qDefs::WARNING,"Attempt to save image failed.\n"
    				"Formats: .png, .jpg, .xpm.","Dock");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SavePlotAutomatic(){
	//no need to save the same plot many times
	if((currentFrame>lastSavedFrame)&&(currentMeasurement>=lastSavedMeasurement)){

		QString qFilePath;
		if(running) qFilePath = filePath;
		else {
			qFilePath = QString(myDet->getFilePath().c_str());
			qDefs::checkErrorMessage(myDet);
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
			string(fileName.toAscii().constData()) + string("\n\nNote: Will not show future file save errors for this acquisition."),"Dock");

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
	//double *temp=0,*trimXAxis=0,*trimYAxis=0;
#ifdef VERBOSE
	if(fromDetector)	cout << "Geting Trimbits from Detector" << endl;
	else				cout << "Getting Trimbits from Shared Memory" << endl;
#endif

	slsDetectorDefs::detectorType detType  = myDet->getDetectorsType();
	if(detType == slsDetectorDefs::MYTHEN){

		//get trimbits
		actualPixelsX = myDet->getTotalNumberOfChannels(slsDetectorDefs::X);
		if(histTrimbits) delete [] histTrimbits; histTrimbits = new double[actualPixelsX];
		ret = myDet->getChanRegs(histTrimbits,fromDetector);
		if(!ret){
			qDefs::Message(qDefs::WARNING,"No Trimbit data found in shared memory.","Dock");
			return qDefs::FAIL;
		}
#ifdef VERBOSE
		cout << "Got Trimbits" << endl;
#endif

		qDefs::checkErrorMessage(myDet);

		//defining axes
		if(Histogram)	nPixelsX = TRIM_HISTOGRAM_XMAX+1;
		else			nPixelsX = actualPixelsX;

		if(histXAxis)		delete [] histXAxis;	histXAxis 	= new double [nPixelsX];
		if(histYAxis[0])	delete [] histYAxis[0]; histYAxis[0]= new double [nPixelsX];
		//initializing
		for(unsigned int px=0;px<(int)nPixelsX;px++)	histXAxis[px] = px;
		for(i=0;i<nPixelsX;i++)							histYAxis[0][i]  = 0;

		//clear/select plot and set titles
		Clear1DPlot();
		Select1DPlot();


		if(!Histogram){
			cout << "Data Graph:" << nPixelsX << endl;
			//data
			memcpy(histYAxis[0],histTrimbits,nPixelsX*sizeof(double));
			//title
			boxPlot->setTitle("Trimbits_Plot_Data Graph");
			plot1D->SetXTitle("Channel Number");
			plot1D->SetYTitle("Trimbits");
			//set plot parameters
			SlsQtH1D*  h;
			plot1D_hists.append(h=new SlsQtH1D("",nPixelsX,histXAxis,histYAxis[0]));
			h->SetLineColor(1);
			h->setTitle(GetHistTitle(0));
			//attach plot
			h->Attach(plot1D);
		}

		else{
			cout << "Histogram: " << nPixelsX-1 << endl;
			//data
			int value =0;
			for(i=0;i<actualPixelsX;i++){
				if((histTrimbits[i]<nPixelsX)&&(histTrimbits[i]>=0)){
					value = (int) histTrimbits[i];
					histYAxis[0][value]++;
				}
				else cout<<"OUT OF BOUNDS:"<<i<<"-"<<histTrimbits[i]<<endl;
			}
			/*		for(i=0;i<TRIM_HISTOGRAM_XMAX;i++)
				if((histYAxis[0][i]<=TRIM_HISTOGRAM_XMAX)&&(histYAxis[0][i]>0))
					cout<<"HIsty["<<i<<"]:"<<histYAxis[0][i]<<endl;*/

			//delete [] histTrimbits;
			//title
			boxPlot->setTitle("Trimbits_Plot_Histogram");
			plot1D->SetXTitle("Trimbits");
			plot1D->SetYTitle("Frequency");
			//set plot parameters
			SlsQtH1D*  h;
			plot1D_hists.append(h=new SlsQtH1D("",nPixelsX,histXAxis,histYAxis[0]));
			h->SetLineColor(1);
			h->setTitle(GetHistTitle(0));
			//attach plot
			h->Attach(plot1D);
		}



#ifdef VERBOSE
		cout << "Trimbits Plot updated" << endl;
#endif
	}



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
			qDefs::Message(qDefs::WARNING,"No Trimbit data found in shared memory.","Dock");
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
	}



	return qDefs::OK;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::ResetPedestal(){
#ifdef VERBOSE
	cout << "Resetting Pedestal" <<  endl;
#endif
	while(1){
		if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
			pedestalVals = 0;
			pedestalCount = 0;
			resetPedestal = true;
			pthread_mutex_unlock(&(last_image_complete_mutex));
			break;
		}
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::CalculatePedestal(){
#ifdef VERBOSE
	cout << "Calculating Pedestal" <<  endl;
#endif
	while(1){
		if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
			//create array
			if(pedestalVals) delete [] pedestalVals; pedestalVals = new double[nPixelsX];
			//reset all values
			for(unsigned int px=0;px<nPixelsX;px++)
				pedestalVals[px] = 0;

			pedestalCount = 0;
			resetPedestal = false;
			pthread_mutex_unlock(&(last_image_complete_mutex));
			break;
		}
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SetFrameFactor(int frame){
	frameFactor = frame;
	if(myDet->setReceiverOnline()==slsDetectorDefs::ONLINE_FLAG){
		frame = myDet->setReadReceiverFrequency(1,frame);
#ifdef VERBOSE
		cout << "Receiver read frequency set to : " << frame << endl;
#endif
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------
