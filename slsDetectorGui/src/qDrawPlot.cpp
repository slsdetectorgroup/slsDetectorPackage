/*
 * qDrawPlot.cpp
 *
 *  Created on: May 7, 2012
 *      Author:  Ian Johnson
 */
// Qt Project Class Headers
#include "qDrawPlot.h"
#include "qCloneWidget.h"
#include "slsDetector.h"

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
		QWidget(parent),myDet(detector){
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
	delete[] lastImageArray; lastImageArray=0;
	StartOrStopThread(0);
	delete myDet;
	for(int i=0;i<MAXCloneWindows;i++) delete winClone[i];
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDrawPlot::SetupWidgetWindow(){
#ifdef VERBOSE
	cout << "Setting up plot variables" << endl;
#endif
	number_of_measurements=1;
	currentMeasurement = 0;
	stop_signal = 0;
	pthread_mutex_init(&last_image_complete_mutex,NULL);
	//gui_acquisition_thread_running = 0;
	// Default Plotting
	plot_in_scope   = 0;
	//2d
	lastImageNumber = 0;
	last_plot_number = 0;

	nPixelsX = myDet->getTotalNumberOfChannels();
	nPixelsY = 100;
	nAnglePixelsX = 1;

	minPixelsY = 0;
	//2d
	lastImageArray = 0;
	image_data = 0;
	//1d
	nHists    = 0;
	histNBins = 0;
	histXAxis = 0;
	histXAngleAxis = 0;
	histYAngleAxis = 0;
	persistency = 0;
	currentPersistency = 0;

	progress = 0;
	plotEnable = true;
	anglePlot = false;

	saveAll = false;

	XYRangeChanged = false;
	timerValue = PLOT_TIMER_MS;
	frameFactor=0;
	data_pause_over = true;//to get the first image
	isFrameEnabled = false;
	isTriggerEnabled = false;
	scanArgument = None;
	// This is so that it initially stop and plots
	running = 1;
	for(int i=0;i<MAX_1DPLOTS;i++) {histYAxis[i]=0;yvalues[i]=0; }

	// clone
	for(int i=0;i<MAXCloneWindows;i++) winClone[i]=0;


	// Setting up window
	setFont(QFont("Sans Serif",9));
	layout = new QGridLayout;
		this->setLayout(layout);
	boxPlot = new QGroupBox("Measurement");
		layout->addWidget(boxPlot,1,1);
		boxPlot->setAlignment(Qt::AlignHCenter);
		boxPlot->setFont(QFont("Sans Serif",11,QFont::Normal));
	plot_update_timer = new QTimer(this);
	connect(plot_update_timer, SIGNAL(timeout()), this, SLOT(UpdatePlot()));
	data_pause_timer = new QTimer(this);
	connect(data_pause_timer, SIGNAL(timeout()), this, SLOT(UpdatePause()));

	// Default titles- only for the initial picture
	histXAxisTitle="Channel Number";
	histYAxisTitle="Counts";


	for(int i=0;i<MAX_1DPLOTS;i++){
		histTitle[i] = "";
		//char temp_title[2000];
		//sprintf(temp_title,"Frame -%d",i);
		//histTitle[i] = temp_title;
	}
	imageTitle.assign("Start Image");
	imageXAxisTitle="Pixel";
	imageYAxisTitle="Pixel";
	imageZAxisTitle="Intensity";



	// setting default plot titles and settings
	plot1D = new SlsQt1DPlot(boxPlot);
		plot1D->setFont(QFont("Sans Serif",9,QFont::Normal));
		plot1D->SetXTitle(histXAxisTitle.toAscii().constData());
		plot1D->SetYTitle(histYAxisTitle.toAscii().constData());
		plot1D->hide();

	plot2D = new SlsQt2DPlotLayout(boxPlot);
		plot2D->setFont(QFont("Sans Serif",9,QFont::Normal));
		plot2D->setTitle(GetImageTitle());
		plot2D->SetXTitle(imageXAxisTitle);
		plot2D->SetYTitle(imageYAxisTitle);
		plot2D->SetZTitle(imageZAxisTitle);
		plot2D->setAlignment(Qt::AlignLeft);
	boxPlot->setFlat(true);
	boxPlot->setContentsMargins(0,15,0,0);

	plotLayout =  new QGridLayout(boxPlot);
		plotLayout->addWidget(plot1D,1,1,1,1);
		plotLayout->addWidget(plot2D,1,1,1,1);

	//marker
	lines = true;
	markers = false;
	marker = new QwtSymbol();
	marker->setStyle(QwtSymbol::Cross);
	marker->setSize(5,5);
	noMarker = new QwtSymbol();




}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDrawPlot::Initialization(){
	connect(this, 		SIGNAL(InterpolateSignal(bool)),plot2D, 	SIGNAL(InterpolateSignal(bool)));
	connect(this, 		SIGNAL(ContourSignal(bool)),	plot2D, 	SIGNAL(ContourSignal(bool)));
	connect(this, 		SIGNAL(LogzSignal(bool)),		plot2D, 	SLOT(SetZScaleToLog(bool)));
	connect(this, 		SIGNAL(EnableZRangeSignal(bool)),plot2D, 	SLOT(EnableZRange(bool)));

	connect(this, 		SIGNAL(SetZRangeSignal(double,double)),	plot2D, 	SLOT(SetZRange(double,double)));

	connect(this, 		SIGNAL(AcquisitionErrorSignal()),	this, 	SLOT(ShowAcquisitionErrorMessage()));


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

		//Do the following only once before each n measurements
		// Reset Current Measurement
		currentMeasurement = 0;
		emit SetCurrentMeasurementSignal(currentMeasurement);

		//to get the first image
		data_pause_over = true;


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


		number_of_exposures = number_of_frames * numScan0 * numScan1;
		if(anglePlot) number_of_exposures = numScan0 * numScan1;


		cout << "\tNumber of Exposures:" << number_of_exposures << endl;

		// ExposureTime
		exposureTime= ((double)(myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,-1))*1E-9);
		cout << "\tExposure Time:" << setprecision (10) << exposureTime << endl;
		// Acquisition Period
		acquisitionPeriod= ((double)(myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,-1))*1E-9);
		cout << "\tAcquisition Period:" << setprecision (10) << acquisitionPeriod << endl;

		//to take the first data if frameFactor
		numFactor = 0;

		//for save automatically,
		saveError = false;
		lastSavedFrame = -1;
		lastSavedMeasurement = -1;


		StartDaq(true);
		running=!running;
	}
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
		if(myDet->getRunStatus()==slsDetectorDefs::IDLE)
			cout<<endl<<endl<<"IDLE"<<endl;
		else cout<<endl<<endl<<"ERRORRRRRR: "<<myDet->getRunStatus()<<endl;
		// Defaults
		progress = 0;
		currentFrame = 0;
		stop_signal = 0;
		histTitle[0] = "";

		//for 2d scans
		int currentIndex = myDet->getFileIndex();
		currentScanValue = 0;
		currentScanDivLevel = 0;


		if(scanArgument!=None){
			if(scanArgument==AllFrames){
				maxPixelsY = currentIndex + number_of_exposures - 1;
				minPixelsY = currentIndex;
				nPixelsY = number_of_exposures;
			}else if(scanArgument==FileIndex){
				maxPixelsY = currentIndex + number_of_frames - 1;
				minPixelsY = currentIndex;
				nPixelsY = number_of_frames;
			}else if(scanArgument==Level0){
				//no need to check if numsteps=0,cuz otherwise this mode wont be set in plot tab
				int numSteps = myDet->getScanSteps(0);
				double *values = new double[numSteps];
				myDet->getScanSteps(0,values);

				maxPixelsY = values[numSteps-1];
				minPixelsY = values[0];
				nPixelsY = numSteps;
				currentScanValue = values[0];
			}
			else {
				//no need to check if numsteps=0,cuz otherwise this mode wont be set in plot tab
				int numSteps = myDet->getScanSteps(1);
				double *values = new double[numSteps];
				myDet->getScanSteps(1,values);

				maxPixelsY = values[numSteps-1];
				minPixelsY = values[0];
				nPixelsY = numSteps;
				currentScanValue = values[0];
			}
			pixelWidth = (maxPixelsY -minPixelsY)/(nPixelsY-1);
			startPixel = minPixelsY -(pixelWidth/2);
			endPixel = maxPixelsY + (pixelWidth/2);
		}else{//no scan
			nPixelsY = 100;//number_of_exposures;
			maxPixelsY = 100;
			minPixelsY = 0;
			startPixel = -0.5;
			endPixel = nPixelsY-0.5;
		}
		cout<<"nPixelsY:"<<nPixelsY<<endl;
		cout<<"minPixelsY:"<<minPixelsY<<endl;
		cout<<"maxPixelsY:"<<maxPixelsY<<endl;
		cout<<"startPixel:"<<startPixel<<endl;
		cout<<"endPixel:"<<endPixel<<endl;



		//1d
		if(histXAxis)    delete [] histXAxis;	histXAxis    = new double [nPixelsX];
		if(histYAxis[0]) delete [] histYAxis[0];histYAxis[0] = new double [nPixelsX];

		//2d
		if(lastImageArray) delete [] lastImageArray; lastImageArray = new double[nPixelsY*nPixelsX];

		//sorta useless
		if(yvalues[0]) delete [] yvalues[0];	yvalues[0] = new double [nPixelsX];
		if(image_data) delete [] image_data;	image_data = new double[nPixelsY*nPixelsX];

		//initializing 1d xaxis
		for(unsigned int px=0;px<nPixelsX;px++)	histXAxis[px]  = px;/*+10;*/

		histYAxis[0][4] = 190.56;

		//initializing 2d array
		for(int py=0;py<nPixelsY;py++)
			for(int px=0;px<nPixelsX;px++)
				lastImageArray[py*nPixelsX+px] = 0;



		cout << "Starting new acquisition threadddd ...." << endl;
		// Setting the callback function to get data from detector class
		myDet->registerDataCallback(&(GetDataCallBack),this);
		//Setting the callback function to alert when acquisition finished from detector class
		myDet->registerAcquisitionFinishedCallback(&(GetAcquisitionFinishedCallBack),this);
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

void* qDrawPlot::DataStartAcquireThread(void *this_pointer){
	cout << "before acquire ...." << endl;
	((qDrawPlot*)this_pointer)->myDet->acquire(1);
	cout << "after acquire ...." << endl;
	return this_pointer;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

int qDrawPlot::GetDataCallBack(detectorData *data, void *this_pointer){
	((qDrawPlot*)this_pointer)->GetData(data);
	return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

int qDrawPlot::GetData(detectorData *data){
#ifdef VERYVERBOSE
	cout << "Entering GetDatafunction" << endl;
#endif
	if(!stop_signal){

		//set title
		SetPlotTitle(QString(data->fileName).section('/',-1));
		//set progress
		progress=(int)data->progressIndex;



		//Plot Disabled
		if(!plotEnable) 	return 0;


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



		//Not Nth Frame, to check time out(NOT for 2dScans and angle plots)
		else{
			if((scanArgument==None)&&(!anglePlot)){
				//if the time is not over, RETURN
				if(!data_pause_over){
					return 0;
				}
				data_pause_over=false;
				data_pause_timer->start((int)(PLOT_TIMER_MS/2));
			}
		}




		//angle plotting
		if(anglePlot){
			if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
				lastImageNumber= currentFrame+1;
				nAnglePixelsX = data->npoints;
				histNBins = nAnglePixelsX;
				nHists=1;
				if(histXAngleAxis) delete [] histXAngleAxis; histXAngleAxis = new double[nAnglePixelsX];
				if(histYAngleAxis) delete [] histYAngleAxis; histYAngleAxis = new double[nAnglePixelsX];
				memcpy(histXAngleAxis,data->angles,nAnglePixelsX*sizeof(double));
				memcpy(histYAngleAxis,data->values,nAnglePixelsX*sizeof(double));
				pthread_mutex_unlock(&(last_image_complete_mutex));
			}
			currentFrame++;
			return 0;
		}



		//if scan argument is 2d
		if(scanArgument!=None){
			//alframes
			if(scanArgument==AllFrames){
				if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
					//variables
					lastImageNumber= currentFrame+1;
					//title
					char temp_title[2000];
					sprintf(temp_title,"Image Index %d",myDet->getFileIndexFromFileName(string(data->fileName)));
					imageTitle = temp_title;
					//copy data
					memcpy(lastImageArray+(currentScanDivLevel*nPixelsX),data->values,nPixelsX*sizeof(double));
					pthread_mutex_unlock(&(last_image_complete_mutex));
				}
				currentFrame++;
				currentScanDivLevel++;
				return 0;
			}
			//file index
			if(scanArgument==FileIndex){
				if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
					//variables
					int currentIndex = myDet->getFileIndexFromFileName(string(data->fileName));
					if(currentIndex == minPixelsY) currentScanDivLevel = 0;
					lastImageNumber= currentFrame+1;
					//title
					char temp_title[2000];
					sprintf(temp_title,"Image Index %d",currentIndex);
					imageTitle = temp_title;
					//copy data
					for(unsigned int px=0;px<nPixelsX;px++)	lastImageArray[currentScanDivLevel*nPixelsX+px] += data->values[px];
					pthread_mutex_unlock(&(last_image_complete_mutex));
				}
				currentFrame++;
				currentScanDivLevel++;
				return 0;
			}
			//level0
			if(scanArgument==Level0){
				if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
					//get scanvariable0
					int currentIndex = 0, p = 0; double cs0 = 0 , cs1 = 0;
					myDet->getVariablesFromFileName(string(data->fileName), currentIndex, p, cs0, cs1);
					int currentScanVariable0 = (int)cs0;
					//variables
					if(currentScanVariable0!=currentScanValue) currentScanDivLevel++;
					currentScanValue = currentScanVariable0;
					lastImageNumber= currentFrame+1;
					//title
					char temp_title[2000];
					sprintf(temp_title,"Image Index %d",currentIndex);
					imageTitle = temp_title;
					//copy data
					for(unsigned int px=0;px<nPixelsX;px++) lastImageArray[currentScanDivLevel*nPixelsX+px] += data->values[px];
					pthread_mutex_unlock(&(last_image_complete_mutex));
				}
				currentFrame++;
				return 0;
			}
			//level1
			if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
				//get scanvariable1
				int currentIndex = 0, p = 0; double cs0 = 0 , cs1 = 0;
				myDet->getVariablesFromFileName(string(data->fileName), currentIndex, p, cs0, cs1);
				int currentScanVariable1 = (int)cs1;
				//variables
				if(currentScanVariable1!=currentScanValue) currentScanDivLevel++;
				currentScanValue = currentScanVariable1;
				lastImageNumber= currentFrame+1;
				//title
				char temp_title[2000];
				sprintf(temp_title,"Image Index %d",currentIndex);
				imageTitle = temp_title;
				//copy data
				for(unsigned int px=0;px<nPixelsX;px++) lastImageArray[currentScanDivLevel*nPixelsX+px] += data->values[px];
				pthread_mutex_unlock(&(last_image_complete_mutex));
			}
			currentFrame++;
			return 0;

		}

		//normal measurement or 1d scans
		if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
			char temp_title[2000];
			// only if you got the lock, do u need to remember lastimagenumber to plot
			lastImageNumber= currentFrame+1;

			//1d
			if(plot_in_scope==1){
				// Titles changed to "" inside startstopthread
				// Persistency
				if(currentPersistency < persistency)currentPersistency++;
				else currentPersistency=persistency;
				nHists = currentPersistency+1;
				histNBins = nPixelsX;
				// copy data
				for(int i=currentPersistency;i>0;i--)
					memcpy(histYAxis[i],histYAxis[i-1],nPixelsX*sizeof(double));
				memcpy(histYAxis[0],data->values,nPixelsX*sizeof(double));
			}
			//2d
			else{
				// Titles
				sprintf(temp_title,"Image Index %d",myDet->getFileIndexFromFileName(string(data->fileName)));
				imageTitle = temp_title;
				// manufacture data for now
				for(unsigned int px=0;px<nPixelsX;px++)
					for(unsigned int py=0;py<nPixelsY;py++)
						image_data[py*nPixelsX+px] = sqrt(pow(currentFrame+1,2)*pow(double(px)-nPixelsX/2,2)/pow(nPixelsX/2,2)/pow(number_of_exposures+1,2) + pow(double(py)-nPixelsY/2,2)/pow(nPixelsY/2,2))/sqrt(2);
				// copy data
				memcpy(lastImageArray,image_data,nPixelsX*nPixelsY*sizeof(double));
				//SHOULD BE memcpy(lastImageArray,data->values,nPixelsX*nPixelsY*sizeof(double));
			}
			pthread_mutex_unlock(&(last_image_complete_mutex));
		}
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


int qDrawPlot::AcquisitionFinished(double currentProgress,int detectorStatus){
#ifdef VERBOSE
	cout << "Entering Acquisition Finished with status " <<
			slsDetectorBase::runStatusType((slsDetectorDefs::runStatus(detectorStatus))) << " and progress " << currentProgress << endl;
#endif

	//error
	if(currentProgress!=100){
		//just to be sure
		stop_signal = 1;
		StartStopDaqToggle(true);
		emit AcquisitionErrorSignal();
	}
	//normal stop
	else if(!stop_signal){
		currentMeasurement++;
		cout<<"currentMeasurement:"<<currentMeasurement<<endl;
		// To start the next measurement
		if(currentMeasurement!=number_of_measurements){
			emit SetCurrentMeasurementSignal(currentMeasurement);
			StopDaqForGui();
			StartDaq(true);
		}// if all the measurements are over
		else{
			StartStopDaqToggle(true);
			emit UpdatingPlotFinished();
		}
	}
	return 0;
}

void qDrawPlot::ShowAcquisitionErrorMessage(){
	int detStatus = (int)myDet->getRunStatus();
	string status = slsDetectorBase::runStatusType(slsDetectorDefs::runStatus(detStatus));
	qDefs::Message(qDefs::WARNING,string("<nobr>The acquisiton has ended abruptly. Current Detector Status: ")+status+string(".</nobr>"),"Dock");
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDrawPlot::SelectPlot(int i){ //1 for 1D otherwise 2D
	if(i==1){
		plot1D->SetXTitle(histXAxisTitle.toAscii().constData());
		plot1D->SetYTitle(histYAxisTitle.toAscii().constData());
		plot1D->show();
		plot2D->hide();
		boxPlot->setFlat(false);
		plot_in_scope=1;
	}else{
		plot2D->SetXTitle(imageXAxisTitle);
		plot2D->SetYTitle(imageYAxisTitle);
		plot2D->SetZTitle(imageZAxisTitle);
		plot1D->hide();
		plot2D->show();
		boxPlot->setFlat(true);
		plot_in_scope=2;
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
		//1-d plot stuff
		if(plot_in_scope==1){
			if(lastImageNumber){
				if(histNBins){
					Clear1DPlot();
					plot1D->SetXTitle(histXAxisTitle.toAscii().constData());
					plot1D->SetYTitle(histYAxisTitle.toAscii().constData());
					for(int hist_num=0;hist_num<(int)nHists;hist_num++){
						SlsQtH1D*  h;
						if(hist_num+1>plot1D_hists.size()){
							if(anglePlot)
								plot1D_hists.append(h=new SlsQtH1D("1d plot",histNBins,histXAngleAxis,histYAngleAxis));
							else
								plot1D_hists.append(h=new SlsQtH1D("1d plot",histNBins,histXAxis,GetHistYAxis(hist_num)));
							h->SetLineColor(hist_num+1);
						}else{
							h=plot1D_hists.at(hist_num);
							if(anglePlot)
								h->SetData(histNBins,histXAngleAxis,histYAngleAxis);
							else
								h->SetData(histNBins,histXAxis,GetHistYAxis(hist_num));
						}
						SetStyle(h);
						h->setTitle(GetHistTitle(hist_num));
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
	}
	last_plot_number=lastImageNumber;

	if(plotEnable) UnlockLastImageArray();

	//if acqq stopped before this line, it continues from here, shouldnt restart plotting timer
	if(!stop_signal){
		if(!frameFactor)
			plot_update_timer->start((int)timerValue);
		else
			plot_update_timer->start((int)PLOT_TIMER_MS);
	}

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

	// create clone
	winClone[i] = new qCloneWidget(this,i,boxPlot->title(),(int)plot_in_scope,plot1D,plot2D,myDet->getFilePath());
	if(plot_in_scope==1){
		plot1D = new SlsQt1DPlot(boxPlot);
		plot1D->setFont(QFont("Sans Serif",9,QFont::Normal));
		plot1D->SetXTitle(histXAxisTitle.toAscii().constData());
		plot1D->SetYTitle(histYAxisTitle.toAscii().constData());
		plotLayout->addWidget(plot1D,1,1,1,1);
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
			LockLastImageArray();
			if(!anglePlot)
				winClone[i]->SetCloneHists((int)nHists,histNBins,histXAxis,histYAxis,histTitle,lines,markers);
			else
				winClone[i]->SetCloneHists((int)nHists,histNBins,histXAngleAxis,histYAngleAxis,histTitle,lines,markers);
			UnlockLastImageArray();
		}
	}
	else{
		plot2D = new SlsQt2DPlotLayout(boxPlot);
		plot2D->setFont(QFont("Sans Serif",9,QFont::Normal));
		plot2D->setTitle(GetImageTitle());
		plot2D->SetXTitle(imageXAxisTitle);
		plot2D->SetYTitle(imageYAxisTitle);
		plot2D->SetZTitle(imageZAxisTitle);
		plotLayout->addWidget(plot2D,1,1,1,1);
	}
	setMinimumHeight(preheight);
	resize(width(),preheight);

	// update the actual plot  only if running, else it doesnt know when its over
	if(running)	UpdatePlot();
	connect(this, 		SIGNAL(InterpolateSignal(bool)),	plot2D, 	SIGNAL(InterpolateSignal(bool)));
	connect(this, 		SIGNAL(ContourSignal(bool)),		plot2D, 	SIGNAL(ContourSignal(bool)));
	connect(this, 		SIGNAL(LogzSignal(bool)),			plot2D, 	SLOT(SetZScaleToLog(bool)));
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

	QString fName = QString(myDet->getFilePath().c_str());
	if(boxPlot->title().contains('.')){
		fName.append(QString('/')+boxPlot->title());
		fName.replace(".dat",".png");
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


void qDrawPlot::SaveAll(bool enable){
	string msg = string("The Files will be saved as:\n")+
				string(myDet->getFilePath().c_str())+string("/")+
				string(myDet->getFileName().c_str())+string("[title].png");
	qDefs::Message(qDefs::INFORMATION,msg,"Dock");
	saveAll = enable;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDrawPlot::SavePlotAutomatic(){
	//no need to save the same plot many times
	if((currentFrame>lastSavedFrame)&&(currentMeasurement>=lastSavedMeasurement)){
		lastSavedFrame = currentFrame;
		lastSavedMeasurement = currentMeasurement;
		char cID[10];
		sprintf(cID,"%d",lastSavedFrame);
		//title
		QString fName = QString(myDet->getFilePath().c_str());
		if(boxPlot->title().contains('.')){
			fName.append(QString('/')+boxPlot->title());
			fName.replace(".dat",".png");
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
	for(int i=0;i<=val;i++){
		if(!yvalues[i]) yvalues[i] = new double [nPixelsX];
		if(!histYAxis[i]) histYAxis[i] = new double [nPixelsX];
	}
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
