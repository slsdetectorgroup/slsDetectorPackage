/** Qt Project Class Headers */
#include "qDetectorMain.h"
#include "qDrawPlot.h"
#include "qTabMeasurement.h"
#include "qTabDataOutput.h"
#include "qTabPlot.h"
#include "qTabActions.h"
#include "qTabAdvanced.h"
#include "qTabSettings.h"
#include "qTabDebugging.h"
#include "qTabDeveloper.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
#include "sls_detector_defs.h"
/** Qt Include Headers */
/** C++ Include Headers */
#include<iostream>
#include <string>
using namespace std;

#define Detector_Index 0

int main (int argc, char **argv) {

	QApplication *theApp = new QApplication(argc, argv);
	qDetectorMain *det=new qDetectorMain(argc, argv, theApp,0);
	det->show();
	theApp->connect( theApp, SIGNAL(lastWindowClosed()), theApp, SLOT(quit()));
	return theApp->exec();
}





qDetectorMain::qDetectorMain(int argc, char **argv, QApplication *app, QWidget *parent) : QMainWindow(parent), theApp(app),myPlot(NULL),tabs(NULL){
	myDet = 0;
	setupUi(this);
	SetUpWidgetWindow();
	Initialization();
	SetDeveloperMode();
	/**need to use argc and argv to determine which slsdet or multidet to use.*/
}


//Qt::ScrollBarAsNeeded


qDetectorMain::~qDetectorMain(){
	delete myDet;
	if (menubar) delete menubar;
	if (centralwidget) delete centralwidget;
}





void qDetectorMain::SetUpWidgetWindow(){


/*	scrollMain = new QScrollArea;
	setCentralWidget(scrollMain);
	scrollMain ->setWidget(centralwidget);
	scrollMain->setWidgetResizable(true);*/

	SetUpDetector();

/** plot setup*/
	myPlot = new qDrawPlot(framePlot,myDet);

/**tabs setup*/
	tabs = new QTabWidget(this);
	layoutTabs->addWidget(tabs);
	/** creating all the tab widgets */
	tab_measurement 	=  new qTabMeasurement	(this,	myDet,myPlot);
	tab_dataoutput 		=  new qTabDataOutput	(this,	myDet);
	tab_plot 			=  new qTabPlot			(this,	myDet,myPlot);
	tab_actions			=  new qTabActions		(this,	myDet);
	tab_advanced 		=  new qTabAdvanced		(this,	myDet);
	tab_Settings 		=  new qTabSettings		(this,	myDet);
	tab_debugging 		=  new qTabDebugging	(this,	myDet);
	tab_developer 		=  new qTabDeveloper	(this,	myDet);
	/**	creating the scroll area widgets for the tabs */
	for(int i=0;i<NUMBER_OF_TABS;i++){
		scroll[i] = new QScrollArea;
		scroll[i]->setFrameShape(QFrame::NoFrame);
	}
	/** setting the tab widgets to the scrollareas*/
	scroll[Measurement]	->setWidget(tab_measurement);
	scroll[DataOutput]	->setWidget(tab_dataoutput);
	scroll[Plot]		->setWidget(tab_plot);
	scroll[Actions]		->setWidget(tab_actions);
	scroll[Advanced]	->setWidget(tab_advanced);
	scroll[Settings]	->setWidget(tab_Settings);
	scroll[Debugging]	->setWidget(tab_debugging);
	scroll[Developer]	->setWidget(tab_developer);
	/** inserting all the tabs*/
	tabs->insertTab(Measurement,	scroll[Measurement],	"Measurement");
	tabs->insertTab(DataOutput,		scroll[DataOutput],		"Data Output");
	tabs->insertTab(Plot,			scroll[Plot],			"Plot");
	tabs->insertTab(Actions,		scroll[Actions],		"Actions");
	tabs->insertTab(Advanced,		scroll[Advanced],		"Advanced");
	tabs->insertTab(Settings,		scroll[Settings],		"Settings");
	tabs->insertTab(Debugging,		scroll[Debugging],		"Debugging");
	tabs->insertTab(Developer,		scroll[Developer],		"Developer");

/** mode setup - to set up the tabs initially as disabled, not in form so done here */
	SetDebugMode(false);
	SetBeamlineMode(false);
	SetExpertMode(false);
}





void qDetectorMain::SetUpDetector(){
	/**instantiate detector and set window title*/
	myDet = new multiSlsDetector(Detector_Index);
	if(!myDet->getHostname(Detector_Index).length()){
		setWindowTitle("SLS Detector GUI : No Detector Connected");
#ifdef VERBOSE
		cout<<endl<<"No Detector Connected"<<endl;
#endif
		myDet = 0;
	}
	else{
		setWindowTitle("SLS Detector GUI : "+QString(slsDetectorBase::getDetectorType(myDet->getDetectorsType()).c_str())+" - "+QString(myDet->getHostname(Detector_Index).c_str()));
#ifdef VERBOSE
		cout<<endl<<"Type : "<<slsDetectorBase::getDetectorType(myDet->getDetectorsType())<<"\t\t\tDetector : "<<myDet->getHostname(Detector_Index)<<endl;
#endif
		myDet->setOnline(slsDetectorDefs::ONLINE_FLAG);
	}
}



void qDetectorMain::Initialization(){


/** tabs */
	connect(tabs,SIGNAL(currentChanged(int)),this, SLOT(refresh(int)));//( QWidget*)));
		/**	Measurement tab*/
		/** Plot tab */


/** Plotting */
		/** When the acquisition is finished, must update the meas tab */
		connect(myPlot,				SIGNAL(UpdatingPlotFinished()),	tab_measurement,SLOT(UpdateFinished()));


/** menubar */
		/** Modes Menu */
		connect(actionDebug,		SIGNAL(toggled(bool)),this,SLOT(SetDebugMode(bool)));
		connect(actionBeamline,		SIGNAL(toggled(bool)),this,SLOT(SetBeamlineMode(bool)));
		connect(actionExpert,		SIGNAL(toggled(bool)),this,SLOT(SetExpertMode(bool)));
		connect(actionDockable,		SIGNAL(toggled(bool)),this,SLOT(SetDockableMode(bool)));


		/** Utilities Menu */
		connect(actionOpenSetup,SIGNAL(triggered()),this,SLOT(OpenSetup()));
		connect(actionSaveSetup,SIGNAL(triggered()),this,SLOT(SaveSetup()));
		connect(actionMeasurementWizard,SIGNAL(triggered()),this,SLOT(MeasurementWizard()));
		connect(actionOpenConfiguration,SIGNAL(triggered()),this,SLOT(OpenConfiguration()));
		connect(actionSaveConfiguration,SIGNAL(triggered()),this,SLOT(SaveConfiguration()));
		connect(actionEnergyCalibration,SIGNAL(triggered()),this,SLOT(EnergyCalibration()));
		connect(actionAngularCalibration,SIGNAL(triggered()),this,SLOT(AngularCalibration()));
		connect(actionAbout,SIGNAL(triggered()),this,SLOT(About()));
		connect(actionVersion,SIGNAL(triggered()),this,SLOT(Version()));

}


void qDetectorMain::SetDeveloperMode(){
#ifdef VERBOSE
	cout<<"Enabling Developer Mode "<<endl;
#endif
	tabs->setTabEnabled(Developer,true);
}


void qDetectorMain::SetDebugMode(bool b){
#ifdef VERBOSE
	cout<<"Setting Debug Mode to "<<b<<endl;
#endif
	tabs->setTabEnabled(Debugging,b);
}

void qDetectorMain::SetBeamlineMode(bool b){
#ifdef VERBOSE
	cout<<"Setting Beamline Mode to "<<b<<endl;
#endif
}

void qDetectorMain::SetExpertMode(bool b){
#ifdef VERBOSE
	cout<<"Setting Expert Mode to "<<b<<endl;
#endif
	//threshold part in measu is enabled
	tabs->setTabEnabled(Advanced,b);
	tabs->setTabEnabled(Settings,b);
	tab_advanced->setEnabled(b);
}



void qDetectorMain::refresh(int index){
	if(!tabs->isTabEnabled(index))
		tabs->setCurrentIndex((index++)<(tabs->count()-1)?index:Measurement);
	else{
;
	}
}


void qDetectorMain::SetDockableMode(bool b){
#ifdef VERBOSE
	cout<<"Setting Dockable Plot Mode to "<<b<<endl;
#endif
	if(b)
		dockWidgetPlot->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	else
		dockWidgetPlot->setFeatures(QDockWidget::NoDockWidgetFeatures);

}


void qDetectorMain::OpenSetup(){
#ifdef VERBOSE
	cout<<"Opening Setup"<<endl;
#endif
}


void qDetectorMain::SaveSetup(){
#ifdef VERBOSE
	cout<<"Saving Setup"<<endl;
#endif
}


void qDetectorMain::MeasurementWizard(){
#ifdef VERBOSE
	cout<<"Measurement Wizard"<<endl;
#endif
}


void qDetectorMain::OpenConfiguration(){
#ifdef VERBOSE
	cout<<"Opening Configuration"<<endl;
#endif
}


void qDetectorMain::SaveConfiguration(){
#ifdef VERBOSE
	cout<<"Saving Configuration"<<endl;
#endif
}


void qDetectorMain::EnergyCalibration(){
#ifdef VERBOSE
	cout<<"Executing Energy Calibration"<<endl;
#endif
}


void qDetectorMain::AngularCalibration(){
#ifdef VERBOSE
	cout<<"Executing Angular Calibration"<<endl;
#endif
}


void qDetectorMain::Version(){
#ifdef VERBOSE
	cout<<"Executing Version"<<endl;
#endif
}


void qDetectorMain::About(){
#ifdef VERBOSE
	cout<<"Executing About"<<endl;
#endif
}

