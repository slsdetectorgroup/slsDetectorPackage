/*
 * qDrawPlot.cpp
 *
 *  Created on: May 7, 2012
 *      Author:  Ian Johnson
 */
/** Qt Project Class Headers */
#include "qDrawPlot.h"
#include "qCloneWidget.h"
#include "SlsQt1DPlot.h"
#include "SlsQt2DPlotLayout.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** Qt Include Headers */
#include <QTimer>
/** C++ Include Headers */
#include <iostream>
#include <string>
using namespace std;


#define Detector_Index 0


qDrawPlot::qDrawPlot(QWidget *parent,slsDetectorUtils*& detector):QWidget(parent),myDet(detector){
	if(myDet)	{
		setupUi(this);
		Initialization();
		SetupWidgetWindow();
		StartStopDaqToggle(); //as default
	}
}


qDrawPlot::~qDrawPlot(){
	/** Clear plot*/
	Clear1DPlot();
	for(QVector<SlsQtH1D*>::iterator h = plot1D_hists.begin();h!=plot1D_hists.end();h++){
		delete *h;
	}
	plot1D_hists.clear();


	delete[] lastImageArray; lastImageArray=0;
	StartOrStopThread(0);

	/** delete detector object pointer*/
	delete myDet;
}




void qDrawPlot::Initialization(){
#ifdef VERBOSE
	 cout<<"Setting up plot variables"<<endl;
#endif
	  stop_signal = 0;
	  pthread_mutex_init(&last_image_complete_mutex,NULL);

	  plot_in_scope   = 0;
	  lastImageNumber = 0;
	  nPixelsX = 0;
	  nPixelsY = 0;
	  lastImageArray = 0;

	  nHists    = 0;
	  histNBins = 0;
	  histXAxis = 0;
	  for(int i=0;i<10;i++) histYAxis[i]=0;

	  for(int i=0;i<MAXCloneWindows;i++) winClone[i]=0;
}




void qDrawPlot::SetupWidgetWindow(){
	plot_update_timer = new QTimer(this);
	connect(plot_update_timer, SIGNAL(timeout()), this, SLOT(UpdatePlot()));

	plot1D = new SlsQt1DPlot(boxPlot);
	plot1D->SetXTitle("x axis");
	plot1D->SetYTitle("y axis");
	plot1D->hide();

	plot2D = new SlsQt2DPlotLayout(boxPlot);
	plot2D->SetXTitle("pixel");
	plot2D->SetYTitle("pixel");
	plot2D->SetZTitle("Intensity");
	boxPlot->setFlat(1);
	boxPlot->setTitle("Startup Image");

	plotLayout =  new QGridLayout(boxPlot);
	plotLayout->addWidget(plot1D,1,1,1,1);
	plotLayout->addWidget(plot2D,1,1,1,1);


	connect(this, 		SIGNAL(InterpolateSignal(bool)),	plot2D, 	SIGNAL(InterpolateSignal(bool)));
	connect(this, 		SIGNAL(ContourSignal(bool)),		plot2D, 	SIGNAL(ContourSignal(bool)));
	connect(this, 		SIGNAL(LogzSignal(bool)),			plot2D, 	SLOT(SetZScaleToLog(bool)));

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
	static bool             gui_acquisition_thread_running = 0;
	static pthread_t        gui_acquisition_thread;
	static pthread_mutex_t  gui_acquisition_start_stop_mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&gui_acquisition_start_stop_mutex);
	//stop part, before start or restart
	if(gui_acquisition_thread_running){
		cout<<"Stopping current acquisition thread ...."<<endl;
		stop_signal = 1;
		pthread_join(gui_acquisition_thread,NULL); //wait until he's finished, ie. exits
		gui_acquisition_thread_running = 0;
	}

	//start part
	if(start){
		stop_signal = 0;
		cout<<"Starting new acquisition thread ...."<<endl;
		gui_acquisition_thread_running = !pthread_create(&gui_acquisition_thread, NULL,qDrawPlot::DataAcquisionThread, (void*) this);
		cout<<"created"<<endl;
		//myDet->acquire(1);//acquiring
	}
	pthread_mutex_unlock(&gui_acquisition_start_stop_mutex);

	return gui_acquisition_thread_running;
}







void* qDrawPlot::DataAcquisionThread(void *this_pointer){
	((qDrawPlot*)this_pointer)->AcquireImages();
	return this_pointer;
}




void* qDrawPlot::AcquireImages(){
	//send data to detector

	static unsigned int nx=1280,ny=100;
	static double* image_data = new double[nx*ny];
	if(!lastImageArray) lastImageArray = new double[nx*ny];
	static double* xvalues  = new double [nx];
	static double* yvalues0 = new double [nx];
	static double* yvalues1 = new double [nx];
	if(!histXAxis)    histXAxis    = new double [nx];
	if(!histYAxis[0]) histYAxis[0] = new double [nx];
	if(!histYAxis[1]) histYAxis[1] = new double [nx];



	char cIndex[200];

	//string filePath = myDet->getFilePath()+'/'+myDet->getFileName()+'_';
	//cout<<"filePath:"<<filePath<<endl;
	//string fileName;

	for(int i=0;i<number_of_exposures;i++){

		/////
		//fileName.assign(filePath);
		//sprintf(cIndex,"%d",i);
		//fileName.append(cIndex);
		//fileName.append(".raw");
		//cout<<"filename:"<<fileName<<endl;
		//short int arg[1280];

		//while(myDet->readDataFile(fileName,arg)==-1);


		/////////


		//readout detector
		//fill and write data here
		for(unsigned int px=0;px<nx;px++)
			for(unsigned int py=0;py<ny;py++)
				image_data[py*nx+px] = sqrt(pow(i+1,2)*pow(double(px)-nx/2,2)/pow(nx/2,2)/pow(number_of_exposures+1,2) + pow(double(py)-ny/2,2)/pow(ny/2,2))/sqrt(2);

		for(unsigned int px=0;px<nx;px++){
/*
			xvalues[px]  = px;//+10;
			yvalues0[px] = (double)arg[px];//i + pow(1 - 2*fabs(double(px)-nx/2)/nx,2);
			yvalues1[px] = 0;//i + pow(1 - 2*fabs(double(px)-nx/2)/nx,4);
*/
			xvalues[px]  = px+10;
			yvalues0[px] = i + pow(1 - 2*fabs(double(px)-nx/2)/nx,2);
			yvalues1[px] = i + pow(1 - 2*fabs(double(px)-nx/2)/nx,4);

		}

/*		if(framePeriod<acquisitionTime) usleep((int)acquisitionTime*1e6);
		else                         	usleep((int)framePeriod*1e6);*/
		cout<<"Reading in image: "<<i<<endl;
//usleep(1000000);
		if(stop_signal) break; //stop_signal should also go to readout function
		if(!pthread_mutex_trylock(&last_image_complete_mutex)){

			//plot_in_scope = 1;//i%2 + 1;
			plot_in_scope = 2;
			cout<<"value:"<<image_data[6]<<endl;

			lastImageNumber = i+1;
			char temp_title[2000];
			//1d image stuff
			nHists = 2;
			sprintf(temp_title,"curve one %d",i);    histTitle[0] = temp_title;
			sprintf(temp_title,"curve two %d",i);    histTitle[1] = temp_title;
			sprintf(temp_title,"Channel Number");  histXAxisTitle=temp_title;
			sprintf(temp_title,"Intensity");  histYAxisTitle=temp_title;
			histNBins = nx;
			memcpy(histXAxis,    xvalues,nx*sizeof(double));
			memcpy(histYAxis[0],yvalues0,nx*sizeof(double));
			memcpy(histYAxis[1],yvalues1,nx*sizeof(double));

			//2d image stuff
			sprintf(temp_title,"Image number %d",i); imageTitle=temp_title;
			sprintf(temp_title,"The x-axis title");  imageXAxisTitle=temp_title;
			sprintf(temp_title,"The y-axis title");  imageYAxisTitle=temp_title;
			sprintf(temp_title,"The z-axis title");  imageZAxisTitle=temp_title;
			nPixelsX = nx;
			nPixelsY = ny;
			memcpy(lastImageArray,image_data,nx*ny*sizeof(double));

			pthread_mutex_unlock(&last_image_complete_mutex);
		}
	}

	return this;
}








void qDrawPlot::SelectPlot(int i){ //1 for 1D otherwise 2D
	if(i==1){
		plot1D->show();
		plot2D->hide();
	}else{
		plot1D->hide();
		plot2D->show();
	}
}




void qDrawPlot::Clear1DPlot(){
	for(QVector<SlsQtH1D*>::iterator h = plot1D_hists.begin();
			h!=plot1D_hists.end();h++) (*h)->Detach(plot1D); //clear plot
}




void qDrawPlot::UpdatePlot(){
	//emit UpdatingPlot();

	plot_update_timer->stop();

	LockLastImageArray();
	//1-d plot stuff
	if(GetHistNBins()){
		plot1D->SetXTitle(GetHistXAxisTitle());
		plot1D->SetYTitle(GetHistYAxisTitle());
		for(int hist_num=0;hist_num<GetNHists();hist_num++){
			SlsQtH1D*  h;
			if(hist_num+1>plot1D_hists.size()){
				plot1D_hists.append(h=new SlsQtH1D("1d plot",GetHistNBins(),GetHistXAxis(),GetHistYAxis(hist_num)));
				h->SetLineColor(hist_num+1);
			}else{
				h=plot1D_hists.at(hist_num);
				h->SetData(GetHistNBins(),GetHistXAxis(),GetHistYAxis(hist_num));
			}
			h->setTitle(GetHistTitle(hist_num));
			h->Attach(plot1D);
		}
		plot1D->UnZoom();
	}

	//2-d plot stuff
	static int last_plot_number = 0;
	if(GetLastImageArray()){
		if(GetLastImageNumber()&&last_plot_number!=GetLastImageNumber() && //there is a new plot
				GetNPixelsX()>0&&GetNPixelsY()>0){
			plot2D->GetPlot()->SetData(GetNPixelsX(),-0.5,GetNPixelsX()-0.5,GetNPixelsY(),-0.5,GetNPixelsY()-0.5,GetLastImageArray());
			//as it inherits a widget
			boxPlot->setTitle(GetImageTitle());
			plot2D->SetXTitle(GetImageXAxisTitle());
			plot2D->SetYTitle(GetImageYAxisTitle());
			plot2D->SetZTitle(GetImageZAxisTitle());
			plot2D->UpdateNKeepSetRangeIfSet(); //this will keep a "set" z range, and call Plot()->Update();
		}
	}
	last_plot_number=GetLastImageNumber();
	UnlockLastImageArray();

	if(PlotInScope()==1)      SelectPlot(1);
	else if(PlotInScope()==2) SelectPlot(2);

	if(number_of_exposures==last_plot_number){
		StartStopDaqToggle(1);
		emit UpdatingPlotFinished();
	}else{
		plot_update_timer->start(500);
	}
}



void qDrawPlot::StopUpdatePlot(){
	plot_update_timer->stop();
}




void qDrawPlot::ClonePlot(){
	int i=0;
	bool found = false;
	for(i=0;i<MAXCloneWindows;i++)
		if(!winClone[i]){
			found=true;
			break;
		}
	if(!found){
		cout<<"Too many clones"<<endl;
		exit(-1);
	}

	winClone[i] = new qCloneWidget(this,i,boxPlot->size(),(int)PlotInScope(),plot1D,plot2D);
	if(PlotInScope()==1)
		plot1D = new SlsQt1DPlot(boxPlot);
	else
		plot2D = new SlsQt2DPlotLayout(boxPlot);
	plotLayout->addWidget(plot1D,1,1,1,1);
	plotLayout->addWidget(plot2D,1,1,1,1);
	UpdatePlot();
	connect(this, 		SIGNAL(InterpolateSignal(bool)),	plot2D, 	SIGNAL(InterpolateSignal(bool)));
	connect(this, 		SIGNAL(ContourSignal(bool)),		plot2D, 	SIGNAL(ContourSignal(bool)));
	connect(this, 		SIGNAL(LogzSignal(bool)),			plot2D, 	SLOT(SetZScaleToLog(bool)));
	winClone[i]->show();
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

