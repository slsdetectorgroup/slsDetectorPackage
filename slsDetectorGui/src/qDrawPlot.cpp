/*
 * qDrawPlot.cpp
 *
 *  Created on: May 7, 2012
 *      Author:  Ian Johnson
 */
/** Qt Project Class Headers */
#include "qDrawPlot.h"
#include "qCloneWidget.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
#include "postProcessing.h"
/** Qt Include Headers */
#include <QFont>
#include <QImage>
#include <QPainter>
/** C++ Include Headers */
#include <iostream>
#include <string>
#include <sstream>
using namespace std;


#define Detector_Index 0




//int numberOfMeasurements;
int qDrawPlot::currentFrame;
int qDrawPlot::number_of_exposures;
//double framePeriod;
//double acquisitionTime;
pthread_mutex_t qDrawPlot::last_image_complete_mutex;
//std::string  imageTitle;

unsigned int qDrawPlot::plot_in_scope;
unsigned int qDrawPlot::nPixelsX;
unsigned int qDrawPlot::nPixelsY;
unsigned int qDrawPlot::lastImageNumber;

string  	 qDrawPlot::histTitle[MAX_1DPLOTS];
unsigned int qDrawPlot::nHists;
int          qDrawPlot::histNBins;
double*      qDrawPlot::histXAxis;
double* 	 qDrawPlot::yvalues[MAX_1DPLOTS];
double*  	 qDrawPlot::histYAxis[MAX_1DPLOTS];

string  	 qDrawPlot::imageTitle;
double*      qDrawPlot::lastImageArray;
double* 	 qDrawPlot::image_data;

bool     	 qDrawPlot::gui_acquisition_thread_running;
int    		 qDrawPlot::persistency;
int    		 qDrawPlot::currentPersistency;
int 		 qDrawPlot::progress;
bool		 qDrawPlot::plotEnable;


qDrawPlot::qDrawPlot(QWidget *parent,slsDetectorUtils*& detector):QWidget(parent),myDet(detector),numberOfMeasurements(1){
	if(myDet)	{
		SetupWidgetWindow();
		Initialization();
		StartStopDaqToggle(); //as default
	}
}


qDrawPlot::~qDrawPlot(){
	/** Clear plot*/
	Clear1DPlot();
	for(QVector<SlsQtH1D*>::iterator h = plot1D_hists.begin();h!=plot1D_hists.end();h++)	delete *h;
	plot1D_hists.clear();
	delete[] lastImageArray; lastImageArray=0;
	StartOrStopThread(0);
	delete myDet;
	for(int i=0;i<MAXCloneWindows;i++) delete winClone[i];
}


void qDrawPlot::SetupWidgetWindow(){
#ifdef VERBOSE
	cout<<"Setting up plot variables"<<endl;
#endif
	stop_signal = 0;
	pthread_mutex_init(&last_image_complete_mutex,NULL);
	gui_acquisition_thread_running = 0;
	/** Default Plotting*/
	plot_in_scope   = 0;
	/**2d*/
	lastImageNumber = 0;


	nPixelsX = 1280; nPixelsY = 100;

	lastImageArray = 0;
	image_data = 0;
	/**1d*/
	nHists    = 0;
	histNBins = 0;
	histXAxis = 0;
	persistency = 0;
	currentPersistency = 0;
	progress = 0;
	plotEnable=true;
	for(int i=0;i<MAX_1DPLOTS;i++) {histYAxis[i]=0;yvalues[i]=0; }

	/*clone*/
	for(int i=0;i<MAXCloneWindows;i++) winClone[i]=0;


	/** Setting up window*/
	setFont(QFont("Sans Serif",9));
	layout = new QGridLayout;
		this->setLayout(layout);
	boxPlot = new QGroupBox("Measurement");
		layout->addWidget(boxPlot,1,1);
		boxPlot->setAlignment(Qt::AlignHCenter);
		boxPlot->setFont(QFont("Sans Serif",11,QFont::Normal));
	plot_update_timer = new QTimer(this);
	connect(plot_update_timer, SIGNAL(timeout()), this, SLOT(UpdatePlot()));


	/** Default titles- only for the initial picture*/
	histXAxisTitle="Channel Number";
	histYAxisTitle="Counts";

	char temp_title[2000];
	for(int i=0;i<MAX_1DPLOTS;i++){
		sprintf(temp_title,"Frame -%d",i);
		histTitle[i] = temp_title;
	}
	imageTitle.assign("Start Image");
	imageXAxisTitle="Pixel";
	imageYAxisTitle="Pixel";
	imageZAxisTitle="Intensity";



	/** setting default plot titles and settings*/
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
}




void qDrawPlot::Initialization(){
	connect(this, 		SIGNAL(InterpolateSignal(bool)),plot2D, 	SIGNAL(InterpolateSignal(bool)));
	connect(this, 		SIGNAL(ContourSignal(bool)),	plot2D, 	SIGNAL(ContourSignal(bool)));
	connect(this, 		SIGNAL(LogzSignal(bool)),		plot2D, 	SLOT(SetZScaleToLog(bool)));
	connect(this, 		SIGNAL(EnableZRangeSignal(bool)),plot2D, 	SLOT(EnableZRange(bool)));

	connect(this, 		SIGNAL(SetZRangeSignal(double,double)),	plot2D, 	SLOT(SetZRange(double,double)));

}




void qDrawPlot::StartStopDaqToggle(bool stop_if_running){
	static bool running = 1;
	if(running){ //stopping
		StartDaq(0);
		running=!running;
	}else if(!stop_if_running){ //then start
		StartDaq(1);
		running=!running;
	}
}




void qDrawPlot::StartDaq(bool start){
	if(start){
#ifdef VERBOSE
		cout<<"Start Daq(1) function"<<endl;
#endif
		number_of_exposures= (int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1);
		cout<<"\tNumber of Exposures:"<<number_of_exposures<<endl;

		acquisitionTime= ((double)(myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,-1))*1E-9);
		cout<<"\tAcquisition Time:"<<setprecision (10)<<acquisitionTime<<endl;

		framePeriod= ((double)(myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,-1))*1E-9);
		cout<<"\tFrame Period:"<<setprecision (10)<<framePeriod<<endl;

		ResetDaqForGui();
		StartDaqForGui();
		UpdatePlot();
	}else{
#ifdef VERBOSE
		cout<<"Start Daq(0) function"<<endl;
#endif
		StopDaqForGui();
		StopUpdatePlot();
	}
}





int qDrawPlot::ResetDaqForGui(){
	if(!StopDaqForGui()) return 0;
	lastImageNumber = 0;
	return 1;
}







bool qDrawPlot::StartOrStopThread(bool start){
	static pthread_t        gui_acquisition_thread;
	static pthread_mutex_t  gui_acquisition_start_stop_mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&gui_acquisition_start_stop_mutex);
	//stop part, before start or restart
	if(gui_acquisition_thread_running){
		cout<<"Stopping current acquisition thread ...."<<endl;
		stop_signal = 1;
		myDet->stopAcquisition();
		pthread_join(gui_acquisition_thread,NULL); //wait until he's finished, ie. exits
		gui_acquisition_thread_running = 0;
	}

	//start part
	if(start){
		/** Defaults */
		currentFrame = 0;
		stop_signal = 0;
		histNBins = nPixelsX;
		if(!image_data) image_data = new double[nPixelsX*nPixelsY];
		if(!lastImageArray) lastImageArray = new double[nPixelsX*nPixelsY];
		if(!histXAxis)    histXAxis    = new double [nPixelsX];
		for(unsigned int px=0;px<nPixelsX;px++)	histXAxis[px]  = px+10;
		if(!yvalues[0]) yvalues[0] = new double [nPixelsX];
		if(!histYAxis[0]) histYAxis[0] = new double [nPixelsX];


		if(plot_in_scope==1) Clear1DPlot();
		cout<<"Starting new acquisition thread ...."<<endl;
		/** Setting the callback function to get data from software client*/
		myDet->registerDataCallback(&(GetDataCallBack));
		/** Start acquiring data from server */
		pthread_create(&gui_acquisition_thread, NULL,DataStartAcquireThread, (void*) this);
		/** This is later reset to zero when all the plotting is done */
		gui_acquisition_thread_running=1;
		cout<<"Started acquiring"<<endl;
	}
	pthread_mutex_unlock(&gui_acquisition_start_stop_mutex);
	return gui_acquisition_thread_running;
}



void* qDrawPlot::DataStartAcquireThread(void *this_pointer){
	((qDrawPlot*)this_pointer)->myDet->acquire(1);
	return this_pointer;
}




int qDrawPlot::GetDataCallBack(detectorData *data){
#ifdef VERYVERBOSE
	cout<<"Entering GetDataCallBack function"<<endl;
#endif
	progress=(int)data->progressIndex;

	if(!plotEnable) {
		lastImageNumber= currentFrame+1;
		currentFrame++;
		return 0;
	}

	/** Get data from client */
	/**1d*/
	if(plot_in_scope==1){
		/** Persistency */
		if(currentPersistency < persistency)currentPersistency++;
		else currentPersistency=persistency;
		for(int i=currentPersistency;i>0;i--)
			memcpy(yvalues[i],yvalues[i-1],nPixelsX*sizeof(double));
		nHists = currentPersistency+1;
		memcpy(yvalues[0],data->values,nPixelsX*sizeof(double));
	}
	/**2d*/
	else{
		for(unsigned int px=0;px<nPixelsX;px++)
			for(unsigned int py=0;py<nPixelsY;py++)
				image_data[py*nPixelsX+px] = sqrt(pow(currentFrame+1,2)*pow(double(px)-nPixelsX/2,2)/pow(nPixelsX/2,2)/pow(number_of_exposures+1,2) + pow(double(py)-nPixelsY/2,2)/pow(nPixelsY/2,2))/sqrt(2);
	}


	if((currentFrame)<(number_of_exposures)){
#ifdef VERYVERBOSE
		cout<<"Reading in image: "<<currentFrame+1<<endl;
#endif
		if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
			char temp_title[2000];
			/** only if you got the lock, do u need to remember lastimagenumber to plot*/
			lastImageNumber= currentFrame+1;

			/**1d*/
			if(plot_in_scope==1){
				/** Titles*/
				sprintf(temp_title,"Frame %d",currentFrame);    histTitle[0] = temp_title;
				/** copy data*/
				//memcpy(histXAxis,    xvalues,nPixelsX*sizeof(double));
				for(int i=currentPersistency;i>0;i--)
					memcpy(histYAxis[i],histYAxis[i-1],nPixelsX*sizeof(double));
				memcpy(histYAxis[0],yvalues[0],nPixelsX*sizeof(double));
			}
			/**2d*/
			else{
				sprintf(temp_title,"Image Number %d",currentFrame);    imageTitle = temp_title;
				memcpy(lastImageArray,image_data,nPixelsX*nPixelsY*sizeof(double));
			}
			pthread_mutex_unlock(&(last_image_complete_mutex));
		}

		currentFrame++;
	}
#ifdef VERYVERBOSE
	cout<<"Exiting GetDataCallBack function"<<endl;
#endif
	return 0;
}



void qDrawPlot::setNumMeasurements(int num){
	numberOfMeasurements = num;
}




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




void qDrawPlot::Clear1DPlot(){
	for(QVector<SlsQtH1D*>::iterator h = plot1D_hists.begin();
			h!=plot1D_hists.end();h++) (*h)->Detach(plot1D); //clear plot
}




void qDrawPlot::UpdatePlot(){
	static int last_plot_number = 0;

	plot_update_timer->stop();
	if(plotEnable){
		LockLastImageArray();
		//1-d plot stuff
		if(lastImageNumber){
			if(histNBins){
#ifdef VERYVERBOSE
				cout<<"Last Image Number: "<<lastImageNumber<<endl;
#endif
				Clear1DPlot();
				plot1D->SetXTitle(histXAxisTitle.toAscii().constData());
				plot1D->SetYTitle(histYAxisTitle.toAscii().constData());
				for(int hist_num=0;hist_num<(int)nHists;hist_num++){
					SlsQtH1D*  h;
					if(hist_num+1>plot1D_hists.size()){
						plot1D_hists.append(h=new SlsQtH1D("1d plot",histNBins,histXAxis,GetHistYAxis(hist_num)));
						h->SetLineColor(hist_num+1);
					}else{
						h=plot1D_hists.at(hist_num);
						h->SetData(histNBins,histXAxis,GetHistYAxis(hist_num));
					}
					h->setTitle(GetHistTitle(hist_num));
					h->Attach(plot1D);

				}
			}
		}

		//2-d plot stuff
		if(lastImageArray){
			if(lastImageNumber&&last_plot_number!=(int)lastImageNumber && //there is a new plot
					nPixelsX>0&&nPixelsY>0){
				plot2D->GetPlot()->SetData(nPixelsX,-0.5,nPixelsX-0.5,nPixelsY,-0.5,nPixelsY-0.5,lastImageArray);
				//as it inherits a widget
				plot2D->setTitle(GetImageTitle());
				plot2D->SetXTitle(imageXAxisTitle);
				plot2D->SetYTitle(imageYAxisTitle);
				plot2D->SetZTitle(imageZAxisTitle);
				plot2D->UpdateNKeepSetRangeIfSet(); //this will keep a "set" z range, and call Plot()->Update();
			}
		}
	}
	last_plot_number=lastImageNumber;
	if(plotEnable) UnlockLastImageArray();
	/*	if(plot_in_scope==1)      SelectPlot(1);
	else if(plot_in_scope==2) SelectPlot(2);*/

	if(number_of_exposures==last_plot_number){
		gui_acquisition_thread_running=0;
		StartStopDaqToggle(1);
		emit UpdatingPlotFinished();
	}else{
		plot_update_timer->start(500);
	}
}



void qDrawPlot::StopUpdatePlot(){
	plot_update_timer->stop();
}



/**----------------------------CLONES-------------------------*/


void qDrawPlot::ClonePlot(){
	int i=0;
	bool found = false;
	for(i=0;i<MAXCloneWindows;i++)
		if(!winClone[i]){
			found=true;
			break;
		}
	/** no space for more clone widget references*/
	if(!found){
		cout<<"Too many clones"<<endl;
		exit(-1);
	}
	/** save height to keep maintain same height of plot */
	int preheight = height();
	/** create clone */
	winClone[i] = new qCloneWidget(this,i,boxPlot->title(),(int)plot_in_scope,plot1D,plot2D,myDet->getFilePath());
	if(plot_in_scope==1){
		plot1D = new SlsQt1DPlot(boxPlot);
		plot1D->setFont(QFont("Sans Serif",9,QFont::Normal));
		plot1D->SetXTitle(histXAxisTitle.toAscii().constData());
		plot1D->SetYTitle(histYAxisTitle.toAscii().constData());
		plotLayout->addWidget(plot1D,1,1,1,1);
		winClone[i]->SetCloneHists((int)nHists,histNBins,histXAxis,histYAxis,histTitle);
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
	/** update the actual plot */
	UpdatePlot();
	connect(this, 		SIGNAL(InterpolateSignal(bool)),	plot2D, 	SIGNAL(InterpolateSignal(bool)));
	connect(this, 		SIGNAL(ContourSignal(bool)),		plot2D, 	SIGNAL(ContourSignal(bool)));
	connect(this, 		SIGNAL(LogzSignal(bool)),			plot2D, 	SLOT(SetZScaleToLog(bool)));
	winClone[i]->show();

	/** to remember which all clone widgets were closed*/
	connect(winClone[i], SIGNAL(CloneClosedSignal(int)),this, SLOT(CloneCloseEvent(int)));

}

void qDrawPlot::CloseClones(){
	for(int i=0;i<MAXCloneWindows;i++)
		if(winClone[i])
			winClone[i]->close();

}


void qDrawPlot::CloneCloseEvent(int id){
	winClone[id]=0;
#ifdef VERBOSE
	cout<<"Closing Clone Window id:"<<id<<endl;
#endif
}






/**----------------------------SAVE-------------------------*/
void qDrawPlot::SavePlot(QString FName){
	QImage img(size().width(),size().height(),QImage::Format_RGB32);
	QPainter painter(&img);
	render(&painter);
	img.save(FName);
}



void qDrawPlot::SetPersistency(int val){
	for(int i=0;i<=val;i++){
		if(!yvalues[i]) yvalues[i] = new double [nPixelsX];
		if(!histYAxis[i]) histYAxis[i] = new double [nPixelsX];
	}
	persistency = val;
}




void qDrawPlot::EnablePlot(bool enable){
#ifdef VERBOSE
	cout<<"Plotting set to:"<<enable<<endl;
#endif
	plotEnable = !enable;
	Clear1DPlot();

}




void qDrawPlot::DisableZoom(bool disable){
	if(plot_in_scope==1)
		plot1D->DisableZoom(disable);
	else
		plot2D->GetPlot()->DisableZoom(disable);
}
