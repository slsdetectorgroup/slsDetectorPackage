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
	cout<<"Setting up plot variables"<<endl;
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

	nPixelsX = 1280; nPixelsY = 100;

	lastImageArray = 0;
	image_data = 0;
	//1d
	nHists    = 0;
	histNBins = 0;
	histXAxis = 0;
	persistency = 0;
	currentPersistency = 0;
	progress = 0;
	plotEnable=true;
	plotDotted = false;
	XYRangeChanged = false;
	timerValue = PLOT_TIMER_MS;
	frameFactor=0;
	oldCopy = false;
	oldFrameNumber = 0;
	data_pause_over = true;//to get the first image
	isFrameEnabled = false;
	isTriggerEnabled = false;
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

	char temp_title[2000];
	for(int i=0;i<MAX_1DPLOTS;i++){
		sprintf(temp_title,"Frame -%d",i);
		histTitle[i] = temp_title;
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
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDrawPlot::Initialization(){
	connect(this, 		SIGNAL(InterpolateSignal(bool)),plot2D, 	SIGNAL(InterpolateSignal(bool)));
	connect(this, 		SIGNAL(ContourSignal(bool)),	plot2D, 	SIGNAL(ContourSignal(bool)));
	connect(this, 		SIGNAL(LogzSignal(bool)),		plot2D, 	SLOT(SetZScaleToLog(bool)));
	connect(this, 		SIGNAL(EnableZRangeSignal(bool)),plot2D, 	SLOT(EnableZRange(bool)));

	connect(this, 		SIGNAL(SetZRangeSignal(double,double)),	plot2D, 	SLOT(SetZRange(double,double)));



}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDrawPlot::StartStopDaqToggle(bool stop_if_running){
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

		number_of_exposures= numFrames * numTriggers;
		cout<<"\tNumber of Exposures:"<<number_of_exposures<<endl;
		// ExposureTime
		exposureTime= ((double)(myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME,-1))*1E-9);
		cout<<"\tExposure Time:"<<setprecision (10)<<exposureTime<<endl;
		// Acquisition Period
		acquisitionPeriod= ((double)(myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,-1))*1E-9);
		cout<<"\tAcquisition Period:"<<setprecision (10)<<acquisitionPeriod<<endl;

		StartDaq(true);
		running=!running;
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDrawPlot::StartDaq(bool start){
	if(start){
#ifdef VERBOSE
		cout<<"Start Daq(true) function"<<endl;
#endif
		ResetDaqForGui();
		StartDaqForGui();
		UpdatePlot();
	}else{
#ifdef VERBOSE
		cout<<"Start Daq(false) function"<<endl;
#endif
		StopDaqForGui();
		StopUpdatePlot();
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

int qDrawPlot::ResetDaqForGui(){
	if(!StopDaqForGui()) return 0;
	cout<<"Resetting image number"<<endl;
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
		cout<<"Stopping current acquisition thread ...."<<endl;
		stop_signal = 1;//sorta useless right now
		gui_acquisition_thread_running = 0;
	}

	//start part
	if(start){

		// Defaults
		progress = 0;
		currentFrame = 0;
		stop_signal = 0;
		oldFrameNumber = 0;
		histNBins = nPixelsX;
		if(!image_data) image_data = new double[nPixelsX*nPixelsY];
		if(!lastImageArray) lastImageArray = new double[nPixelsX*nPixelsY];
		if(!histXAxis)    histXAxis    = new double [nPixelsX];
		for(unsigned int px=0;px<nPixelsX;px++)	histXAxis[px]  = px+10;
		if(!yvalues[0]) yvalues[0] = new double [nPixelsX];
		if(!histYAxis[0]) histYAxis[0] = new double [nPixelsX];


		if(plot_in_scope==1) Clear1DPlot();
		cout<<"Starting new acquisition threadddd ...."<<endl;
		// Setting the callback function to get data from software client
		myDet->registerDataCallback(&(GetDataCallBack),this);
		// Start acquiring data from server
		if(!firstTime) pthread_join(gui_acquisition_thread,NULL);//wait until he's finished, ie. exits
		pthread_create(&gui_acquisition_thread, NULL,DataStartAcquireThread, (void*) this);
		firstTime = false;
		// This is set here and later reset to zero when all the plotting is done
		// This is manually done instead of keeping track of thread because
		// this thread returns immediately after executing the acquire command
		gui_acquisition_thread_running=1;
		cout<<"Started acquiring threaddd:"<<endl;
	}
	pthread_mutex_unlock(&gui_acquisition_start_stop_mutex);
	return gui_acquisition_thread_running;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void* qDrawPlot::DataStartAcquireThread(void *this_pointer){
	cout<<"before acquire ...."<<endl;
	((qDrawPlot*)this_pointer)->myDet->acquire(1);
	cout<<"after acquire ...."<<endl;
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
	cout<<"Entering GetDatafunction"<<endl;
#endif
	if(!stop_signal){
		//not frame factor
		if(!frameFactor){
			//if the time is not over, RETURN
			if(!data_pause_over){
				//lastImageNumber= currentFrame+1;
				currentFrame++;
				return 0;
			}
			data_pause_over=false;
			data_pause_timer->start((int)(PLOT_TIMER_MS/2));
		}//if frame factor or last frame of last measurement... for all other factors, RETURN
		else{
			if(((currentFrame+1==number_of_exposures)&&(currentMeasurement+1==number_of_measurements))
					||(!((currentFrame)%frameFactor)))
				oldCopy = false;//if this works, then we forget old data
			else{
				//lastImageNumber= currentFrame+1;
				currentFrame++;
				progress=(int)data->progressIndex;
				//if theres an old copy, try to get lock again
				if(oldCopy){
#ifdef VERBOSE
					cout<<"Copying old data: "<<oldFrameNumber<<endl;
#endif
					if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
						char temp_title[2000];
						// only if you got the lock, do u need to remember lastimagenumber to plot
						lastImageNumber= currentFrame;
						//1d
						if(plot_in_scope==1){
							// Titles
							sprintf(temp_title,"Frame %d",oldFrameNumber);    histTitle[0] = temp_title;
							// copy data//memcpy(histXAxis,    xvalues,nPixelsX*sizeof(double));
							for(int i=currentPersistency;i>0;i--)
								memcpy(histYAxis[i],yvalues[i-1],nPixelsX*sizeof(double));
							memcpy(histYAxis[0],yvalues[0],nPixelsX*sizeof(double));
						}//2d
						else{
							// Titles
							sprintf(temp_title,"Image Number %d",oldFrameNumber);
							imageTitle = temp_title;
							// copy data
							//memcpy(lastImageArray,image_data,nPixelsX*nPixelsY*sizeof(double));
						}
						pthread_mutex_unlock(&(last_image_complete_mutex));
					}
				}
				return 0;
			}
		}
		//if plot disabled, RETURN
		if(!plotEnable) {
			progress=(int)data->progressIndex;
			//lastImageNumber= currentFrame+1;
			currentFrame++;
			return 0;
		}
		//what comes here has plot enabled AND (frame factor OR data pause over )
		progress=(int)data->progressIndex;

		if((currentFrame)<(number_of_exposures)){
#ifdef VERYVERBOSE
			cout<<"Reading in image: "<<currentFrame<<endl;
#endif
			if(!pthread_mutex_trylock(&(last_image_complete_mutex))){
				char temp_title[2000];
				// only if you got the lock, do u need to remember lastimagenumber to plot
				lastImageNumber= currentFrame+1;

				//1d
				if(plot_in_scope==1){
					// Titles
					sprintf(temp_title,"Frame %d",currentFrame);    histTitle[0] = temp_title;
					// Persistency
					if(currentPersistency < persistency)currentPersistency++;
					else currentPersistency=persistency;
					nHists = currentPersistency+1;
					// copy data
					//memcpy(histXAxis,    xvalues,nPixelsX*sizeof(double));
					for(int i=currentPersistency;i>0;i--)
						memcpy(histYAxis[i],histYAxis[i-1],nPixelsX*sizeof(double));
					memcpy(histYAxis[0],data->values,nPixelsX*sizeof(double));
					//for(int i=0;i<(int)nPixelsX;i++)		*(yvalues[0]+i) = (double)*(data->values+i);
				}
				//2d
				else{
					// Titles
					sprintf(temp_title,"Image Number %d",currentFrame);
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
			}//copies old data only if its frame factor
			else if(frameFactor){
				oldCopy = true;
				oldFrameNumber = currentFrame;
				//1D
				if(plot_in_scope==1){
					// Persistency
					if(currentPersistency < persistency)currentPersistency++;
					else currentPersistency=persistency;
					nHists = currentPersistency+1;
					// copy old data
					for(int i=currentPersistency;i>0;i--)
						memcpy(yvalues[i],yvalues[i-1],nPixelsX*sizeof(double));
					nHists = currentPersistency+1;
					memcpy(yvalues[0],data->values,nPixelsX*sizeof(double));
				}//2D
				else{
					// copy old data
					//memcpy(image_data,data->values,nPixelsX*nPixelsY*sizeof(double));
				}
			}
			currentFrame++;
		}
	}
#ifdef VERYVERBOSE
	cout<<"Exiting GetData function"<<endl;
#endif
	return 0;
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
	cout<<"Entering UpdatePlot function"<<endl;
#endif

	plot_update_timer->stop();

	// only if no plot isnt enabled
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
						h->SetDotStyle(plotDotted);
					}
					h->setTitle(GetHistTitle(hist_num));
					h->Attach(plot1D);

				}
				// update range if required
				if(XYRangeChanged){
					if(!IsXYRange[qDefs::XMINIMUM])
						XYRangeValues[qDefs::XMINIMUM]= plot1D->GetXMinimum();
					if(!IsXYRange[qDefs::XMAXIMUM])
						XYRangeValues[qDefs::XMAXIMUM]= plot1D->GetXMaximum();
					if(!IsXYRange[qDefs::YMINIMUM])
						XYRangeValues[qDefs::YMINIMUM]= plot1D->GetYMinimum();
					if(!IsXYRange[qDefs::YMAXIMUM])
						XYRangeValues[qDefs::YMAXIMUM]= plot1D->GetYMaximum();

					plot1D->SetXMinMax(XYRangeValues[qDefs::XMINIMUM],XYRangeValues[qDefs::XMAXIMUM]);
					plot1D->SetYMinMax(XYRangeValues[qDefs::YMINIMUM],XYRangeValues[qDefs::YMAXIMUM]);

					XYRangeChanged	= false;
				}
			}
		}
		//2-d plot stuff
		if(lastImageArray){
			if(lastImageNumber&&last_plot_number!=(int)lastImageNumber && //there is a new plot
					nPixelsX>0&&nPixelsY>0){
				plot2D->GetPlot()->SetData(nPixelsX,-0.5,nPixelsX-0.5,nPixelsY,-0.5,nPixelsY-0.5,lastImageArray);
				plot2D->setTitle(GetImageTitle());
				plot2D->SetXTitle(imageXAxisTitle);
				plot2D->SetYTitle(imageYAxisTitle);
				plot2D->SetZTitle(imageZAxisTitle);
				plot2D->UpdateNKeepSetRangeIfSet(); //this will keep a "set" z range, and call Plot()->Update();
			}
			// update range if required
			if(XYRangeChanged){
				if(!IsXYRange[qDefs::XMINIMUM])
					XYRangeValues[qDefs::XMINIMUM]= plot2D->GetPlot()->GetXMinimum();
				if(!IsXYRange[qDefs::XMAXIMUM])
					XYRangeValues[qDefs::XMAXIMUM]= plot2D->GetPlot()->GetXMaximum();
				if(!IsXYRange[qDefs::YMINIMUM])
					XYRangeValues[qDefs::YMINIMUM]= plot2D->GetPlot()->GetYMinimum();
				if(!IsXYRange[qDefs::YMAXIMUM])
					XYRangeValues[qDefs::YMAXIMUM]= plot2D->GetPlot()->GetYMaximum();

				plot2D->GetPlot()->SetXMinMax(XYRangeValues[qDefs::XMINIMUM],XYRangeValues[qDefs::XMAXIMUM]);
				plot2D->GetPlot()->SetYMinMax(XYRangeValues[qDefs::YMINIMUM],XYRangeValues[qDefs::YMAXIMUM]);

				XYRangeChanged	= false;
			}
		}
	}
	last_plot_number=lastImageNumber;

	if(plotEnable) UnlockLastImageArray();
	// Measurement not over, continue
	if(number_of_exposures!=currentFrame){//las plot number?
		//if the interval is a timer and not nth frame *
		if(!frameFactor)
			plot_update_timer->start((int)timerValue);
		else
			plot_update_timer->start((int)PLOT_TIMER_MS);
	}
	// if a measurement is over
	else{
		currentMeasurement++;
		// if all the measurements are over
		if(currentMeasurement==number_of_measurements){
			StartStopDaqToggle(true);
			emit UpdatingPlotFinished();
		}// To start the next measurement
		else{
			emit SetCurrentMeasurementSignal(currentMeasurement);
			StopDaqForGui();
			StartDaq(true);
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDrawPlot::StopUpdatePlot(){
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
		cout<<"Too many clones"<<endl;
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
		if(running) winClone[i]->SetCloneHists((int)nHists,histNBins,histXAxis,histYAxis,histTitle);
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

void qDrawPlot::CloseClones(){
	for(int i=0;i<MAXCloneWindows;i++)
		if(winClone[i])
			winClone[i]->close();

}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDrawPlot::CloneCloseEvent(int id){
	winClone[id]=0;
#ifdef VERBOSE
	cout<<"Closing Clone Window id:"<<id<<endl;
#endif
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDrawPlot::SavePlot(){
	// render image
	QImage savedImage(size().width(),size().height(),QImage::Format_RGB32);
	QPainter painter(&savedImage);
	render(&painter);

	// save image
	QString fName = QString(myDet->getFilePath().c_str())+"/Image.png";
	fName = QFileDialog::getSaveFileName(0,tr("Save Image"),fName,tr("PNG Files (*.png);;XPM Files(*.xpm);;JPEG Files(*.jpg)"),0,QFileDialog::ShowDirsOnly);

	if (!fName.isEmpty())
		if(savedImage.save(fName))
			qDefs::InfoMessage("The Image has been successfully saved","Dock");
		else
			qDefs::WarningMessage("Attempt to save image failed.\n"
    				"Formats: .png, .jpg, .xpm.","Dock");
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
	cout<<"Plotting set to:"<<enable<<endl;
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
