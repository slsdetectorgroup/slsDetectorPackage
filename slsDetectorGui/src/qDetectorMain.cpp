/**********************************************************************
 * TO DO
 * 1. settcpsocket is done with slsdetector.maybe do for all detectors connected: mythen
 * ********************************************************************/
// Qt Project Class Headers
#include "qDetectorMain.h"
#include "qDefs.h"
#include "qDrawPlot.h"
#include "qTabMeasurement.h"
#include "qTabDataOutput.h"
#include "qTabPlot.h"
#include "qTabActions.h"
#include "qTabAdvanced.h"
#include "qTabSettings.h"
#include "qTabDebugging.h"
#include "qTabDeveloper.h"
#include "qTabMessages.h"
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
#include "sls_detector_defs.h"
// Qt Include Headers
#include <QSizePolicy>
#include <QFileDialog>
// C++ Include Headers
#include<iostream>
#include <string>
using namespace std;


//-------------------------------------------------------------------------------------------------------------------------------------------------


int main (int argc, char **argv) {

	QApplication *theApp = new QApplication(argc, argv);
	theApp->setWindowIcon(QIcon( ":/icons/images/mountain.png" ));
	qDetectorMain *det=new qDetectorMain(argc, argv, theApp,0);
	det->show();
	//theApp->connect( theApp, SIGNAL(lastWindowClosed()), theApp, SLOT(quit()));
	return theApp->exec();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qDetectorMain::qDetectorMain(int argc, char **argv, QApplication *app, QWidget *parent) :
		QMainWindow(parent), theApp(app),myDet(0),detID(0),myPlot(NULL),tabs(NULL),isDeveloper(0){

	// Getting all the command line arguments
	for(int iarg=1; iarg<argc; iarg++){
		if(!strcasecmp(argv[iarg],"-developer"))	{isDeveloper=1;}
		if(!strcasecmp(argv[iarg],"-id"))			{detID=atoi(argv[iarg+1]);}

		if(!strcasecmp(argv[iarg],"-help")){
			cout << "Possible Arguments are:" << endl;
			cout << "-help \t\t : \t This help" << endl;
			cout << "-developer \t : \t Enables the developer tab" << endl;
			cout << "-id i \t : \t Sets the detector to id i (the default is 0). "
				"Required only when more than one detector is connected in parallel." << endl;
		}
	}

	setupUi(this);
	SetUpDetector();
	SetUpWidgetWindow();
	Initialization();

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qDetectorMain::~qDetectorMain(){
	delete myDet;
	if (menubar) delete menubar;
	if (centralwidget) delete centralwidget;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::SetUpWidgetWindow(){

// Layout
	layoutTabs= new QGridLayout;
	centralwidget->setLayout(layoutTabs);

// plot setup
	myPlot = new qDrawPlot(dockWidgetPlot,myDet);
	dockWidgetPlot->setWidget(myPlot);

//tabs setup
	tabs = new MyTabWidget(this);
	layoutTabs->addWidget(tabs);
	// creating all the tab widgets
	tab_messages		=  new qTabMessages		(this,	myDet);
	tab_measurement 	=  new qTabMeasurement	(this,	myDet,myPlot);
	tab_dataoutput 		=  new qTabDataOutput	(this,	myDet, detID);
	tab_plot 			=  new qTabPlot			(this,	myDet,myPlot);
	tab_actions			=  new qTabActions		(this,	myDet);
	tab_settings 		=  new qTabSettings		(this,	myDet, detID);
	tab_advanced 		=  new qTabAdvanced		(this,	myDet);
	tab_debugging 		=  new qTabDebugging	(this,	myDet);
	tab_developer 		=  new qTabDeveloper	(this,	myDet);
	//	creating the scroll area widgets for the tabs
	for(int i=0;i<NumberOfTabs;i++){
		scroll[i] = new QScrollArea;
		scroll[i]->setFrameShape(QFrame::NoFrame);
	}
	// setting the tab widgets to the scrollareas
	scroll[Measurement]	->setWidget(tab_measurement);
	scroll[DataOutput]	->setWidget(tab_dataoutput);
	scroll[Plot]		->setWidget(tab_plot);
	scroll[Actions]		->setWidget(tab_actions);
	scroll[Settings]	->setWidget(tab_settings);
	scroll[Advanced]	->setWidget(tab_advanced);
	scroll[Debugging]	->setWidget(tab_debugging);
	scroll[Developer]	->setWidget(tab_developer);
	// inserting all the tabs
	tabs->insertTab(Measurement,	scroll[Measurement],	"Measurement");
	tabs->insertTab(DataOutput,		scroll[DataOutput],		"Data Output");
	tabs->insertTab(Plot,			scroll[Plot],			"Plot");
	tabs->insertTab(Actions,		scroll[Actions],		"Actions");
	tabs->insertTab(Settings,		scroll[Settings],		"Settings");
	tabs->insertTab(Advanced,		scroll[Advanced],		"Advanced");
	tabs->insertTab(Debugging,		scroll[Debugging],		"Debugging");
	tabs->insertTab(Developer,		scroll[Developer],		"Developer");
	// Prefer this to expand and not have scroll buttons
	tabs->insertTab(Messages,		tab_messages,		"Messages");
	// Default tab color
	defaultTabColor = tabs->tabBar()->tabTextColor(DataOutput);
	//Set the current tab(measurement) to blue as it is the current one
	tabs->tabBar()->setTabTextColor(0,QColor(0,0,200,255));
	// increase the width so it uses all the empty space for the tab titles
	tabs->tabBar()->setFixedWidth(width()+61);

	// mode setup - to set up the tabs initially as disabled, not in form so done here
#ifdef VERBOSE
	cout << "Setting Debug Mode to 0\nSetting Beamline Mode to 0\n"
			"Setting Expert Mode to 0\nSetting Dockable Mode to false\n"
			"Setting Developer Mode to " << isDeveloper << endl;
#endif
	tabs->setTabEnabled(Debugging,false);
	//beamline mode to false
	tabs->setTabEnabled(Advanced,false);
	dockWidgetPlot->setFloating(false);
	dockWidgetPlot->setFeatures(QDockWidget::NoDockWidgetFeatures);
	tabs->setTabEnabled(Developer,isDeveloper);

// Other setup
	//Height of plot and central widget
	heightPlotWindow = dockWidgetPlot->size().height();
	heightCentralWidget = centralwidget->size().height();
	// Default zoom Tool Tip
	zoomToolTip = dockWidgetPlot->toolTip();

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::SetUpDetector(){


	//instantiate detector and set window title
	myDet = new multiSlsDetector(detID);
	string host = myDet->getHostname(detID);
	slsDetector *s = myDet->getSlsDetector(detID);
	//if hostname doesnt exist even in shared memory
	if(!host.length()){
#ifdef VERBOSE
		cout << endl << "No Detector Connected at id:" << detID << endl;
#endif
		char cIndex[10];
		sprintf(cIndex,"%d",detID);
		qDefs::ErrorMessage(string("No Detector Connected at id : ")+string(cIndex),"Main");
		exit(-1);
	}//if the detector is not even connected
	else if(s->setTCPSocket()==slsDetectorDefs::FAIL){
		qDefs::ErrorMessage(string("The detector ")+host+string(" is not connected. Exiting GUI."),"Main");
		cout << "The detector " << host << "is not connected. Exiting GUI." << endl;
		exit(-1);
	}
	else{
		slsDetectorDefs::detectorType detType = myDet->getDetectorsType();
		// Check if type valid. If not, exit
		switch(detType){
				case slsDetectorDefs::MYTHEN:	break;
				case slsDetectorDefs::EIGER:	break;
				case slsDetectorDefs::GOTTHARD:	break;
				default:
					string detName = myDet->slsDetectorBase::getDetectorType(detType);
					string errorMess = host+string(" has unknown detector type \"")+
							detName+string("\". Exiting GUI.");
					qDefs::ErrorMessage(errorMess,"Main");
					exit(-1);
				}
		setWindowTitle("SLS Detector GUI : "+
				QString(slsDetectorBase::getDetectorType(detType).c_str())+	" - "+QString(host.c_str()));
#ifdef VERBOSE
		cout << endl << "Type : " << slsDetectorBase::getDetectorType(detType) << "\nDetector : " << host << endl;
#endif
		myDet->setOnline(slsDetectorDefs::ONLINE_FLAG);
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::Initialization(){
// Dockable Plot
	connect(dockWidgetPlot,	SIGNAL(topLevelChanged(bool)),	this,SLOT(ResizeMainWindow(bool)));
// tabs
	connect(tabs,			SIGNAL(currentChanged(int)),	this, SLOT(Refresh(int)));//( QWidget*)));
		//	Measurement tab
		connect(tab_measurement,	SIGNAL(StartSignal()),				this,SLOT(EnableTabs()));
		connect(tab_measurement,	SIGNAL(StopSignal()),				this,SLOT(EnableTabs()));
		connect(tab_measurement,	SIGNAL(CheckPlotIntervalSignal()),	tab_plot,SLOT(SetFrequency()));
		// Plot tab
		connect(tab_plot,			SIGNAL(DisableZoomSignal(bool)),	this,SLOT(SetZoomToolTip(bool)));
		// Actions tab
		connect(tab_actions,		SIGNAL(EnableScanBox(int,int)),		tab_plot,SLOT(EnableScanBox(int,int)));
// Plotting
	// When the acquisition is finished, must update the meas tab
	connect(myPlot,	SIGNAL(UpdatingPlotFinished()),				this,				SLOT(EnableTabs()));
	connect(myPlot,	SIGNAL(UpdatingPlotFinished()),				tab_measurement,	SLOT(UpdateFinished()));
	connect(myPlot,	SIGNAL(SetCurrentMeasurementSignal(int)),	tab_measurement,	SLOT(SetCurrentMeasurement(int)));
// menubar
	// Modes Menu
	connect(menuModes,		SIGNAL(triggered(QAction*)),	this,SLOT(EnableModes(QAction*)));
	// Utilities Menu
	connect(menuUtilities,	SIGNAL(triggered(QAction*)),	this,SLOT(ExecuteUtilities(QAction*)));
	// Help Menu
	connect(menuHelp,		SIGNAL(triggered(QAction*)),	this,SLOT(ExecuteHelp(QAction*)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::EnableModes(QAction *action){
	bool enable;

	//Set DebugMode
	if(action==actionDebug){
		enable = actionDebug->isChecked();
		tabs->setTabEnabled(Debugging,enable);
#ifdef VERBOSE
		cout << "Setting Debug Mode to " << enable << endl;
#endif
	}

	//Set BeamlineMode
	else if(action==actionBeamline){
		enable = actionBeamline->isChecked();
#ifdef VERBOSE
		cout << "Setting Beamline Mode to " << enable << endl;
#endif
	}

	//Set ExpertMode
	else if(action==actionExpert){
		enable = actionExpert->isChecked();
		tabs->setTabEnabled(Advanced,enable);
		tab_settings->EnableExpertMode(enable);
#ifdef VERBOSE
		cout << "Setting Expert Mode to " << enable << endl;
#endif
	}

	//Set DockableMode
	else{
		enable = actionDockable->isChecked();
		if(enable)
			dockWidgetPlot->setFeatures(QDockWidget::DockWidgetFloatable);
		else{
			dockWidgetPlot->setFloating(false);
			dockWidgetPlot->setFeatures(QDockWidget::NoDockWidgetFeatures);
		}
#ifdef VERBOSE
		cout << "Setting Dockable Mode to " << enable << endl;
#endif
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::ExecuteUtilities(QAction *action){

	if(action==actionOpenSetup){
#ifdef VERBOSE
		cout << "Opening Setup" << endl;
#endif
		QString fName = QString(myDet->getFilePath().c_str());
		fName = QFileDialog::getOpenFileName(this,
				tr("Load Detector Setup"),fName,
				tr("Detector Setup files (*.det)"));
		// Gets called when cancelled as well
		if (!fName.isEmpty()){
			myDet->retrieveDetectorSetup(string(fName.toAscii().constData()));
			qDefs::InfoMessage("The parameters have been successfully setup.","Main");
		}
	}
	else if(action==actionSaveSetup){
#ifdef VERBOSE
		cout << "Saving Setup" << endl;
#endif
		QString fName = QString(myDet->getFilePath().c_str());
		fName = QFileDialog::getSaveFileName(this,
				tr("Save Current Detector Setup"),fName,
				tr("Detector Setup files (*.det) "));
		// Gets called when cancelled as well
		if (!fName.isEmpty()){
			myDet->dumpDetectorSetup(string(fName.toAscii().constData()));
			qDefs::InfoMessage("The setup parameters have been successfully saved.","Main");
		}
	}
	else if(action==actionMeasurementWizard){
#ifdef VERBOSE
		cout << "Measurement Wizard" << endl;
#endif
	}
	else if(action==actionOpenConfiguration){
#ifdef VERBOSE
		cout << "Opening Configuration" << endl;
#endif
		QString fName = QString(myDet->getFilePath().c_str());
		fName = QFileDialog::getOpenFileName(this,
				tr("Load Detector Configuration"),fName,
				tr("Configuration files (*.config)"));
		// Gets called when cancelled as well
		if (!fName.isEmpty()){
			myDet->readConfigurationFile(string(fName.toAscii().constData()));
			qDefs::InfoMessage("The parameters have been successfully configured.","Main");
		}
	}
	else if(action==actionSaveConfiguration){
#ifdef VERBOSE
		cout << "Saving Configuration" << endl;
#endif
		QString fName = QString(myDet->getFilePath().c_str());
		fName = QFileDialog::getSaveFileName(this,
				tr("Save Current Detector Configuration"),fName,
				tr("Configuration files (*.config) "));
		// Gets called when cancelled as well
		if (!fName.isEmpty()){
			myDet->writeConfigurationFile(string(fName.toAscii().constData()));
			qDefs::InfoMessage("The configuration parameters have been successfully saved.","Main");
		}
	}

	Refresh(tabs->currentIndex());
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::ExecuteHelp(QAction *action){
	if(action==actionAbout){
#ifdef VERBOSE
		cout << "About: Common GUI for Mythen, Eiger, Gotthard and Agipd detectors" << endl;
#endif
		//<h1 style="font-family:verdana;">A heading</h1>
		qDefs::InfoMessage("<p style=\"font-family:verdana;\">SLS Detector GUI version:   1.0<br><br>"
				"Common GUI to control the SLS Detectors: "
				"Mythen, Eiger, Gotthard and Agipd.<br><br>"
				"It can be operated in parallel with the command line interface:<br>"
				"sls_detector_put,<br>sls_detector_get,<br>sls_detector_acquire and<br>sls_detector_help.<br><br>"
				"The software is still in progress. "
				"Please report bugs to dhanya.maliakal@psi.ch.<\\p>","About SLS Detector GUI");
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::Refresh(int index){
	if(!tabs->isTabEnabled(index))
		tabs->setCurrentIndex((index++)<(tabs->count()-1)?index:Measurement);
	else{
		switch(tabs->currentIndex()){
		case Measurement:	if(!myPlot->isRunning()) tab_measurement->Refresh();	break;
		case Settings:		tab_settings->Refresh();	break;
		case DataOutput:	tab_dataoutput->Refresh();	break;
		case Plot:			tab_plot->Refresh();		break;
		case Actions:		tab_actions->Refresh();		break;
		case Advanced:		tab_advanced->Refresh();	break;
		case Debugging:		tab_debugging->Refresh();	break;
		case Developer:		tab_developer->Refresh();	break;
		case Messages:		break;
		}
	}
	for(int i=0;i<NumberOfTabs;i++)
		tabs->tabBar()->setTabTextColor(i,defaultTabColor);
	tabs->tabBar()->setTabTextColor(index,QColor(0,0,200,255));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::ResizeMainWindow(bool b){
#ifdef VERBOSE
	cout << "Resizing Main Window: height:" << height() << endl;
#endif
	// undocked from the main window
	if(b){
		// sets the main window height to a smaller maximum to get rid of space
		setMaximumHeight(height()-heightPlotWindow-9);
		dockWidgetPlot->setMinimumHeight(0);
		cout << "undocking it from main window" << endl;
	}
	else{
		setMaximumHeight(QWIDGETSIZE_MAX);
		// the minimum for plot will be set when the widget gets resized automatically
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::resizeEvent(QResizeEvent* event){
	if(!dockWidgetPlot->isFloating()){
		if(tabs->currentIndex()== Actions){
			dockWidgetPlot->setMinimumHeight(heightPlotWindow-100);
			centralwidget->setMaximumHeight(QWIDGETSIZE_MAX);

		}
		else{
			dockWidgetPlot->setMinimumHeight(height()-centralwidget->height()-50);
			centralwidget->setMaximumHeight(heightCentralWidget);
		}
	}

	//adjusting tab width
	if(width()>=800){ tabs->tabBar()->setFixedWidth(width()+61);}
	else { tabs->tabBar()->setMinimumWidth(0);
		tabs->tabBar()->setExpanding(true);
		tabs->tabBar()->setUsesScrollButtons(true);
	}

	event->accept();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::EnableTabs(){
#ifdef VERBOSE
	cout << "Entering EnableTabs function" << endl;
#endif
	bool enable;
	enable=!(tabs->isTabEnabled(DataOutput));

	// or use the Enable/Disable button
	// normal tabs
	tabs->setTabEnabled(DataOutput,enable);
	tabs->setTabEnabled(Actions,enable);
	tabs->setTabEnabled(Settings,enable);
	tabs->setTabEnabled(Messages,enable);

	// special tabs
	if(enable==false){
		tabs->setTabEnabled(Debugging,enable);
		tabs->setTabEnabled(Advanced,enable);
		tabs->setTabEnabled(Developer,enable);
	}
	else{
	// enable these tabs only if they were enabled earlier
		if(actionDebug->isChecked())
			tabs->setTabEnabled(Debugging,enable);
		if(actionExpert->isChecked())
			tabs->setTabEnabled(Advanced,enable);
		if(isDeveloper)
			tabs->setTabEnabled(Developer,enable);
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::SetZoomToolTip(bool disable){
	if(disable)
		dockWidgetPlot->setToolTip("<span style=\" color:#00007f;\">To Enable mouse-controlled zooming capabilities,\ndisable min and max for all axes.<span> ");
	else
		dockWidgetPlot->setToolTip(zoomToolTip);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------
