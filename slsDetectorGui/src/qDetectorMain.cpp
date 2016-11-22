/**********************************************************************
 * TO DO
 * 1. settcpsocket is done with slsdetector.maybe do for all detectors connected: mythen
 * ********************************************************************/
// Qt Project Class Headers
#include "qDetectorMain.h"
#include "qTabDataOutput.h"
#include "qTabPlot.h"
#include "qTabActions.h"
#include "qTabAdvanced.h"
#include "qTabSettings.h"
#include "qTabDebugging.h"
#include "qTabDeveloper.h"
#include "qTabMessages.h"
#include "qServer.h"
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
#include "sls_detector_defs.h"
#include "gitInfoGui.h"
// Qt Include Headers
#include <QSizePolicy>
#include <QFileDialog>
#include <QPlastiqueStyle>
// C++ Include Headers
#include<iostream>
#include <string>
#include <getopt.h>
using namespace std;


//-------------------------------------------------------------------------------------------------------------------------------------------------

int main (int argc, char **argv) {
  QApplication *theApp = new QApplication(argc, argv);
  //	QApplication *theApp = new QApplication(argc, argv);
	theApp->setStyle(new QPlastiqueStyle);//not default when desktop is windows
	theApp->setWindowIcon(QIcon( ":/icons/images/mountain.png" ));
	qDetectorMain *det=new qDetectorMain(argc, argv, theApp,0);
	det->show();
	//theApp->connect( theApp, SIGNAL(lastWindowClosed()), theApp, SLOT(quit()));
	return theApp->exec();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qDetectorMain::qDetectorMain(int argc, char **argv, QApplication *app, QWidget *parent) :
				QMainWindow(parent), theApp(app),myDet(0),detID(0),myPlot(0),tabs(0),isDeveloper(0){
//	bool found;
	int c;
	string configFName = "";
	optind=1;
	// Getting all the command line arguments
	while(1) {
		static struct option long_options[] = {
			{ "developer", no_argument,       0, 'd' },
			{ "config",    required_argument, 0, 'f' },
			{ "id",        required_argument, 0, 'i' },
			{ "f",         required_argument, 0, 'f' },
			{ "help",      no_argument,       0, 'h' },
			{ 0,           0,                 0,  0  }
		};
		c = getopt_long (argc, argv, "hdf:i:", long_options, NULL);
		if (c == -1) break;

		switch (c) {
			case 'd' :
				isDeveloper=1;
				break;
			case 'f' :
				configFName=string(optarg);
				break;
			case 'i' :
				detID=atoi(optarg);
				break;
			case 'h' :
			default:
				cout << endl;
				cout << "\t" << argv[0] << " [ARGUMENT]..." << endl;
				cout << endl;
				cout << "Possible Arguments are:" << endl;
				cout << "\t-d, --developer \t\t : \t Enables the developer tab" << endl;
				cout << "\t-f, --f, --config fname\t\t : \t Loads config file fname" << endl;
				cout << "\t-i, --id NUMBER \t\t : \t Sets the multi detector id to NUMBER (the default is 0). "
					"Required only when more than one multi detector object is needed." << endl;
				exit(-1);
		}
	}
	if (optind < argc) {
		cout << "invalid option, try --help" << endl;
		exit(-1);
	}
/*
	for(int iarg=1; iarg<argc; iarg++){
		found = false;
		if(!strcasecmp(argv[iarg],"--developer"))					{isDeveloper=1;found = true;}
		if((!strcasecmp(argv[iarg],"--id")) && (iarg+1 < argc))		{detID=atoi(argv[iarg+1]);iarg++;found = true;}
		if((!strcasecmp(argv[iarg],"--config")) && (iarg+1 < argc))	{configFName=string(argv[iarg+1]);iarg++;found = true;}
		if((!strcasecmp(argv[iarg],"--f")) && (iarg+1 < argc))		{configFName=string(argv[iarg+1]);iarg++;found = true;}
		if(!found){
			cout << "Possible Arguments are:" << endl;
			cout << "--developer \t\t : \t Enables the developer tab" << endl;
			cout << "--f [fname]\t\t : \t Loads config file fname" << endl;
			cout << "--config [fname]\t : \t Loads config file fname" << endl;
			cout << "--id [i] \t\t : \t Sets the multi detector id to i (the default is 0). "
				"Required only when more than one multi detector object is needed." << endl;
			exit(-1);
		}
	}
*/
	setupUi(this);
	SetUpDetector(configFName);
	SetUpWidgetWindow();
	Initialization();

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qDetectorMain::~qDetectorMain(){
    if(myDet) delete myDet;
	if (menubar) delete menubar;
	if (centralwidget) delete centralwidget;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::SetUpWidgetWindow(){

// Layout
	layoutTabs= new QGridLayout;
	centralwidget->setLayout(layoutTabs);

//plot setup
	myPlot = new qDrawPlot(dockWidgetPlot,myDet);							cout<<"DockPlot ready"<<endl;
	dockWidgetPlot->setWidget(myPlot);

//tabs setup
	tabs = new MyTabWidget(this);
	layoutTabs->addWidget(tabs);


// creating all the other tab widgets
	tab_measurement 	=  new qTabMeasurement	(this,	myDet,myPlot);		cout<<"Measurement ready"<<endl;
	tab_dataoutput 		=  new qTabDataOutput	(this,	myDet);				cout<<"DataOutput ready"<<endl;
	tab_plot 			=  new qTabPlot			(this,	myDet,myPlot);		cout<<"Plot ready"<<endl;
	tab_actions			=  new qTabActions		(this,	myDet);				cout<<"Actions ready"<<endl;
	tab_settings 		=  new qTabSettings		(this,	myDet);				cout<<"Settings ready"<<endl;
	tab_advanced 		=  new qTabAdvanced		(this,	myDet,myPlot);		cout<<"Advanced ready"<<endl;
	tab_debugging 		=  new qTabDebugging	(this,	myDet);				cout<<"Debugging ready"<<endl;
	tab_developer 		=  new qTabDeveloper	(this,	myDet);				cout<<"Developer ready"<<endl;

	myServer = new qServer(this);											cout<<"Client Server ready"<<endl;

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
	//no scroll buttons this way
	tabs->insertTab(Messages,		tab_messages,		"Messages");

//swap tabs so that messages is last tab
	tabs->tabBar()->moveTab(tabs->indexOf(tab_measurement),	Measurement);
	tabs->tabBar()->moveTab(tabs->indexOf(tab_settings),	Settings);
	tabs->tabBar()->moveTab(tabs->indexOf(tab_dataoutput),	DataOutput);
	tabs->tabBar()->moveTab(tabs->indexOf(tab_plot),		Plot);
	tabs->tabBar()->moveTab(tabs->indexOf(tab_actions),		Actions);
	tabs->tabBar()->moveTab(tabs->indexOf(tab_advanced),	Advanced);
	tabs->tabBar()->moveTab(tabs->indexOf(tab_debugging),	Debugging);
	tabs->tabBar()->moveTab(tabs->indexOf(tab_developer),	Developer);
	tabs->tabBar()->moveTab(tabs->indexOf(tab_messages),	Messages);
	tabs->setCurrentIndex(Measurement);

//other tab properties
	// Default tab color
	defaultTabColor = tabs->tabBar()->tabTextColor(DataOutput);
	//Set the current tab(measurement) to blue as it is the current one
	tabs->tabBar()->setTabTextColor(0,QColor(0,0,200,255));
	// increase the width so it uses all the empty space for the tab titles
	tabs->tabBar()->setFixedWidth(width()+61);

	// mode setup - to set up the tabs initially as disabled, not in form so done here
#ifdef VERBOSE
	cout << "Setting Debug Mode to 0\nSetting Expert Mode to 0\nSetting Developer Mode to " << isDeveloper << "\nSetting Dockable Mode to false\n" << endl;
#endif
	tabs->setTabEnabled(Debugging,false);
	tabs->setTabEnabled(Advanced,false);
	tabs->setTabEnabled(Developer,isDeveloper);
	actionLoadTrimbits->setVisible(false);
	actionSaveTrimbits->setVisible(false);
	actionLoadCalibration->setVisible(false);
	actionSaveCalibration->setVisible(false);

	dockWidgetPlot->setFloating(false);
	dockWidgetPlot->setFeatures(QDockWidget::NoDockWidgetFeatures);

// Other setup
	//Height of plot and central widget
	heightPlotWindow = dockWidgetPlot->size().height();
	heightCentralWidget = centralwidget->size().height();
	// Default zoom Tool Tip
	zoomToolTip = dockWidgetPlot->toolTip();


}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::SetUpDetector(const string fName){


	//instantiate detector and set window title
	myDet = new multiSlsDetector(detID);

	//create messages tab to capture config file loading logs
	tab_messages		=  new qTabMessages		(this); 				cout<<"Messages ready"<<endl;

	//loads the config file at startup
	if(!fName.empty()) LoadConfigFile(fName);

	//gets the hostname if it worked
	string host = myDet->getHostname();
	qDefs::checkErrorMessage(myDet,"qDetectorMain::SetUpDetector");

	//if hostname doesnt exist even in shared memory
	if(!host.length()){
		cout << endl << "No Detector Connected." << endl;
		qDefs::Message(qDefs::CRITICAL,"No Detectors Connected. ","qDetectorMain::SetUpDetector");
		exit(-1);
	}

	//check if the detector is not even connected
	string offline = myDet->checkOnline();
	qDefs::checkErrorMessage(myDet,"qDetectorMain::SetUpDetector");

	if(!offline.empty()){
		qDefs::Message(qDefs::CRITICAL,string("<nobr>The detector(s)  <b>")+offline+string(" </b> is/are not connected.  Exiting GUI.</nobr>"),"qDetectorMain::SetUpDetector");
		cout << "The detector(s)  " << host << "  is/are not connected. Exiting GUI." << endl;
		exit(-1);
	}

	// Check if type valid. If not, exit
	slsDetectorDefs::detectorType detType = myDet->getDetectorsType();
	qDefs::checkErrorMessage(myDet,"qDetectorMain::SetUpDetector");

	switch(detType){
	case slsDetectorDefs::MYTHEN:	break;
	case slsDetectorDefs::EIGER:	break;
	case slsDetectorDefs::GOTTHARD:
	case slsDetectorDefs::AGIPD:
	case slsDetectorDefs::PROPIX:
	case slsDetectorDefs::MOENCH:
	case slsDetectorDefs::JUNGFRAU:
		actionLoadTrimbits->setText("Load Settings");  actionSaveTrimbits->setText("Save Settings");
		break;
	default:
		string detName = myDet->slsDetectorBase::getDetectorType(detType);
		qDefs::checkErrorMessage(myDet,"qDetectorMain::SetUpDetector");
		cout << "ERROR: " + host + " has unknown detector type \"" +  detName + "\". Exiting GUI." << endl;
		string errorMess = host+string(" has unknown detector type \"")+
				detName+string("\". Exiting GUI.");
		qDefs::Message(qDefs::CRITICAL,errorMess,"qDetectorMain::SetUpDetector");
		exit(-1);
	}
	setWindowTitle("SLS Detector GUI : "+
			QString(slsDetectorBase::getDetectorType(detType).c_str())+	" - "+QString(host.c_str()));
//#ifdef VERBOSE
	cout << endl << "Type : " << slsDetectorBase::getDetectorType(detType) << "\nDetector : " << host << endl;
//#endif
	myDet->setOnline(slsDetectorDefs::ONLINE_FLAG);
	qDefs::checkErrorMessage(myDet,"qDetectorMain::SetUpDetector");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::Initialization(){
// Dockable Plot
	connect(dockWidgetPlot,	SIGNAL(topLevelChanged(bool)),	this,SLOT(ResizeMainWindow(bool)));
// tabs
	connect(tabs,			SIGNAL(currentChanged(int)),	this, SLOT(Refresh(int)));//( QWidget*)));
		//	Measurement tab
		connect(tab_measurement,	SIGNAL(StartSignal()),				this,SLOT(EnableTabs()));
		connect(tab_measurement,	SIGNAL(StopSignal()),				myPlot,SLOT(StopAcquisition()));
		connect(tab_measurement,	SIGNAL(CheckPlotIntervalSignal()),	tab_plot,SLOT(SetFrequency()));
		// Data Output Tab
		connect(tab_dataoutput,	SIGNAL(AngularConversionSignal(bool)),	tab_actions,SLOT(EnablePositions(bool)));
		//enable scanbox( for angles)
		connect(tab_dataoutput,	SIGNAL(AngularConversionSignal(bool)),	tab_plot,SLOT(EnableScanBox()));
		// Plot tab
		connect(tab_plot,			SIGNAL(DisableZoomSignal(bool)),	this,SLOT(SetZoomToolTip(bool)));
		// Actions tab (only for scan)
		connect(tab_actions,		SIGNAL(EnableScanBox()),			tab_plot,SLOT(EnableScanBox()));
		//settings to advanced tab(int is always 0 to only refresh)
		connect(tab_settings,		SIGNAL(UpdateTrimbitSignal(int)),		tab_advanced,SLOT(UpdateTrimbitPlot(int)));
// Plotting
	// When the acquisition is finished, must update the meas tab
	connect(myPlot,	SIGNAL(UpdatingPlotFinished()),				this,				SLOT(EnableTabs()));
	connect(myPlot,	SIGNAL(UpdatingPlotFinished()),				tab_measurement,	SLOT(UpdateFinished()));
	//This should not be called as it will change file name to measurement when run finished
	//connect(myPlot,	SIGNAL(UpdatingPlotFinished()),				tab_plot,			SLOT(Refresh()));
	connect(myPlot,	SIGNAL(SetCurrentMeasurementSignal(int)),	tab_measurement,	SLOT(SetCurrentMeasurement(int)));


// menubar
	// Modes Menu
	connect(menuModes,		SIGNAL(triggered(QAction*)),	this,SLOT(EnableModes(QAction*)));
	// Utilities Menu
	connect(menuUtilities,	SIGNAL(triggered(QAction*)),	this,SLOT(ExecuteUtilities(QAction*)));
	// Help Menu
	connect(menuHelp,		SIGNAL(triggered(QAction*)),	this,SLOT(ExecuteHelp(QAction*)));


//server
	connect(myServer,		SIGNAL(ServerStoppedSignal()),		this,SLOT(UncheckServer()));
}



//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::LoadConfigFile(const string fName){
#ifdef VERBOSE
	cout << "Loading config file at start up:" << fName << endl;
#endif
	struct stat st_buf;
	QString file = QString(fName.c_str());

	//path doesnt exist
	if(stat(fName.c_str(),&st_buf))
		qDefs::Message(qDefs::WARNING,string("<nobr>Start up configuration failed to load. The following file does not exist:</nobr><br><nobr>")+fName,"qDetectorMain::LoadConfigFile");

	//not a file
	else if (!S_ISREG (st_buf.st_mode))
		qDefs::Message(qDefs::WARNING,string("<nobr>Start up configuration failed to load. The following file is not a recognized file format:</nobr><br><nobr>")+fName,"qDetectorMain::LoadConfigFile");

	else{
		//could not load config file
		if(myDet->readConfigurationFile(fName)==slsDetectorDefs::FAIL)
			qDefs::Message(qDefs::WARNING,string("<nobr>Could not load all the Configuration Parameters from file:<br>")+fName,"qDetectorMain::LoadConfigFile");
		//successful
		else
			qDefs::Message(qDefs::INFORMATION,"<nobr>The Configuration Parameters have been loaded successfully at start up.</nobr>","qDetectorMain::LoadConfigFile");

		qDefs::checkErrorMessage(myDet,"qDetectorMain::LoadConfigFile");
	}
}



//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::EnableModes(QAction *action){
	bool enable;

	//listen to gui client
	if(action==actionListenGuiClient){

		myServer->StartStopServer(actionListenGuiClient->isChecked());

		//disconnect(menuModes,		SIGNAL(triggered(QAction*)),	this,SLOT(EnableModes(QAction*)));
		//actionListenGuiClient->setChecked(myServer->StartStopServer(actionListenGuiClient->isChecked()));
		//connect(menuModes,		SIGNAL(triggered(QAction*)),	this,SLOT(EnableModes(QAction*)));
	}
	//Set DebugMode
	else if(action==actionDebug){
		enable = actionDebug->isChecked();
		tabs->setTabEnabled(Debugging,enable);
#ifdef VERBOSE
		cout << "Setting Debug Mode to " << enable << endl;
#endif
	}

	//Set ExpertMode(comes here only if its a digital detector)
	else if(action==actionExpert){
		enable = actionExpert->isChecked();

		tabs->setTabEnabled(Advanced,enable);
		actionLoadTrimbits->setVisible(enable);
		actionSaveTrimbits->setVisible(enable);
		actionLoadCalibration->setVisible(enable);
		actionSaveCalibration->setVisible(enable);
		tab_measurement->SetExpertMode(enable);
		tab_settings->SetExpertMode(enable);
		tab_dataoutput->SetExpertMode(enable);
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
	bool refreshTabs = false;
	if(action==actionOpenSetup){
#ifdef VERBOSE
		cout << "Loading Setup" << endl;
#endif
		QString fName = QString(myDet->getFilePath().c_str());
		qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
		fName = QFileDialog::getOpenFileName(this,
				tr("Load Detector Setup"),fName,
				tr("Detector Setup files (*.det);;All Files(*)"));
		// Gets called when cancelled as well
		if (!fName.isEmpty()){
			if(myDet->retrieveDetectorSetup(string(fName.toAscii().constData()))!=slsDetectorDefs::FAIL){
				qDefs::Message(qDefs::INFORMATION,"The Setup Parameters have been loaded successfully.","qDetectorMain::ExecuteUtilities");
				refreshTabs=true;
			}else qDefs::Message(qDefs::WARNING,string("Could not load the Setup Parameters from file:\n")+fName.toAscii().constData(),"qDetectorMain::ExecuteUtilities");
			qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
		}
	}
	else if(action==actionSaveSetup){
#ifdef VERBOSE
		cout << "Saving Setup" << endl;
#endif
		QString fName = QString(myDet->getFilePath().c_str());
		qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
		fName = QFileDialog::getSaveFileName(this,
				tr("Save Current Detector Setup"),fName,
				tr("Detector Setup files (*.det);;All Files(*) "));
		// Gets called when cancelled as well
		if (!fName.isEmpty()){
			if(myDet->dumpDetectorSetup(string(fName.toAscii().constData()))!=slsDetectorDefs::FAIL)
				qDefs::Message(qDefs::INFORMATION,"The Setup Parameters have been saved successfully.","qDetectorMain::ExecuteUtilities");
			else qDefs::Message(qDefs::WARNING,string("Could not save the Setup Parameters from file:\n")+fName.toAscii().constData(),"qDetectorMain::ExecuteUtilities");
			qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
		}
	}
	else if(action==actionOpenConfiguration){
#ifdef VERBOSE
		cout << "Loading Configuration" << endl;
#endif
		QString fName = QString(myDet->getFilePath().c_str());
		qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
		fName = QFileDialog::getOpenFileName(this,
				tr("Load Detector Configuration"),fName,
				tr("Configuration files (*.config);;All Files(*)"));
		// Gets called when cancelled as well
		if (!fName.isEmpty()){
			if(myDet->readConfigurationFile(string(fName.toAscii().constData()))!=slsDetectorDefs::FAIL){
				qDefs::Message(qDefs::INFORMATION,"The Configuration Parameters have been configured successfully.","qDetectorMain::ExecuteUtilities");
				refreshTabs=true;
			}else qDefs::Message(qDefs::WARNING,string("Could not load all the Configuration Parameters from file:\n")+fName.toAscii().constData(),"qDetectorMain::ExecuteUtilities");
			qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
		}
	}
	else if(action==actionSaveConfiguration){
#ifdef VERBOSE
		cout << "Saving Configuration" << endl;
#endif
		QString fName = QString(myDet->getFilePath().c_str());
		qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
		fName = QFileDialog::getSaveFileName(this,
				tr("Save Current Detector Configuration"),fName,
				tr("Configuration files (*.config) ;;All Files(*)"));
		// Gets called when cancelled as well
		if (!fName.isEmpty()){
			if(myDet->writeConfigurationFile(string(fName.toAscii().constData()))!=slsDetectorDefs::FAIL)
				qDefs::Message(qDefs::INFORMATION,"The Configuration Parameters have been saved successfully.","qDetectorMain::ExecuteUtilities");
			else qDefs::Message(qDefs::WARNING,string("Could not save the Configuration Parameters from file:\n")+fName.toAscii().constData(),"qDetectorMain::ExecuteUtilities");
			qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
		}
	}
	else if(action==actionLoadTrimbits){
		QString fName = QString(myDet->getSettingsDir());
		qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
		//gotthard
		if(actionLoadTrimbits->text().contains("Settings")){
#ifdef VERBOSE
			cout << "Loading Settings" << endl;
#endif
			fName = QFileDialog::getOpenFileName(this,
					tr("Load Detector Settings"),fName,
					tr("Settings files (*.settings settings.sn*);;All Files(*)"));
			// Gets called when cancelled as well
			if (!fName.isEmpty()){
				if(myDet->loadSettingsFile(string(fName.toAscii().constData()),-1)!=slsDetectorDefs::FAIL)
					qDefs::Message(qDefs::INFORMATION,"The Settings have been loaded successfully.","qDetectorMain::ExecuteUtilities");
				else qDefs::Message(qDefs::WARNING,string("Could not load the Settings from file:\n")+fName.toAscii().constData(),"qDetectorMain::ExecuteUtilities");
				qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
			}

		}//mythen and eiger
		else{
#ifdef VERBOSE
			cout << "Loading Trimbits" << endl;
#endif
			//so that even nonexisting files can be selected
			QFileDialog	*fileDialog = new QFileDialog(this,
					tr("Load Detector Trimbits"),fName,
					tr("Trimbit files (*.trim noise.sn*);;All Files(*)"));
			fileDialog->setFileMode(QFileDialog::AnyFile );
		    if ( fileDialog->exec() == QDialog::Accepted )
		    	fName = fileDialog->selectedFiles()[0];

			// Gets called when cancelled as well
			if (!fName.isEmpty()){
				if(myDet->loadSettingsFile(string(fName.toAscii().constData()),-1)!=slsDetectorDefs::FAIL)
					qDefs::Message(qDefs::INFORMATION,"The Trimbits have been loaded successfully.","qDetectorMain::ExecuteUtilities");
				else qDefs::Message(qDefs::WARNING,string("Could not load the Trimbits from file:\n")+fName.toAscii().constData(),"qDetectorMain::ExecuteUtilities");
				qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
			}
		}
	}
	else if(action==actionSaveTrimbits){
		//gotthard
		if(actionLoadTrimbits->text().contains("Settings")){
#ifdef VERBOSE
			cout << "Saving Settings" << endl;
#endif
			//different output directory so as not to overwrite
			QString fName = QString(myDet->getSettingsDir());
			qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
			fName = QFileDialog::getSaveFileName(this,
					tr("Save Current Detector Settings"),fName,
					tr("Settings files (*.settings settings.sn*);;All Files(*) "));
			// Gets called when cancelled as well
			if (!fName.isEmpty()){
				if(myDet->saveSettingsFile(string(fName.toAscii().constData()),-1)!=slsDetectorDefs::FAIL)
					qDefs::Message(qDefs::INFORMATION,"The Settings have been saved successfully.","qDetectorMain::ExecuteUtilities");
				else qDefs::Message(qDefs::WARNING,string("Could not save the Settings to file:\n")+fName.toAscii().constData(),"qDetectorMain::ExecuteUtilities");
				qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
			}
		}//mythen and eiger
		else{
#ifdef VERBOSE
			cout << "Saving Trimbits" << endl;
#endif//different output directory so as not to overwrite
			QString fName = QString(myDet->getSettingsDir());
			qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
			fName = QFileDialog::getSaveFileName(this,
					tr("Save Current Detector Trimbits"),fName,
					tr("Trimbit files (*.trim noise.sn*) ;;All Files(*)"));
			// Gets called when cancelled as well
			if (!fName.isEmpty()){
				if(myDet->saveSettingsFile(string(fName.toAscii().constData()),-1)!=slsDetectorDefs::FAIL)
					qDefs::Message(qDefs::INFORMATION,"The Trimbits have been saved successfully.","qDetectorMain::ExecuteUtilities");
				else qDefs::Message(qDefs::WARNING,string("Could not save the Trimbits to file:\n")+fName.toAscii().constData(),"qDetectorMain::ExecuteUtilities");
				qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
			}
		}
	}
	else if(action==actionLoadCalibration){
#ifdef VERBOSE
		cout << "Loading Calibration Data" << endl;
#endif
		QString fName = QString(myDet->getCalDir());
		qDefs::checkErrorMessage(myDet);

		//so that even nonexisting files can be selected
		QFileDialog	*fileDialog = new QFileDialog(this,
				tr("Load Detector Calibration Data"),fName,
				tr("Calibration files (*.cal calibration.sn*);;All Files(*)"));
		fileDialog->setFileMode(QFileDialog::AnyFile );
	    if ( fileDialog->exec() == QDialog::Accepted )
	    	fName = fileDialog->selectedFiles()[0];

		// Gets called when cancelled as well
		if (!fName.isEmpty()){
			if(myDet->loadCalibrationFile(string(fName.toAscii().constData()),-1)!=slsDetectorDefs::FAIL)
				qDefs::Message(qDefs::INFORMATION,"The Calibration Data have been loaded successfully.","qDetectorMain::ExecuteUtilities");
			else qDefs::Message(qDefs::WARNING,string("Could not load the Calibration data from file:\n")+ fName.toAscii().constData(),"qDetectorMain::ExecuteUtilities");
			qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
		}
	}
	else if(action==actionSaveCalibration){
#ifdef VERBOSE
		cout << "Saving Calibration Data" << endl;
#endif//different output directory so as not to overwrite
		QString fName = QString(myDet->getCalDir());
		qDefs::checkErrorMessage(myDet);
		fName = QFileDialog::getSaveFileName(this,
				tr("Save Current Detector Calibration Data"),fName,
				tr("Calibration files (*.cal calibration.sn*);;All Files(*) "));
		// Gets called when cancelled as well
		if (!fName.isEmpty()){
			if(myDet->saveCalibrationFile(string(fName.toAscii().constData()),-1)!=slsDetectorDefs::FAIL)
				qDefs::Message(qDefs::INFORMATION,"The Calibration Data have been saved successfully.","qDetectorMain::ExecuteUtilities");
			else qDefs::Message(qDefs::WARNING,string("Could not save the Calibration data to file:\n")+fName.toAscii().constData(),"qDetectorMain::ExecuteUtilities");
			qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteUtilities");
		}
	}

	Refresh(tabs->currentIndex());
	if(refreshTabs){
		tab_actions->Refresh();
		tab_measurement->Refresh();
		tab_settings->Refresh();
		tab_dataoutput->Refresh();
		if(tab_advanced->isEnabled())	tab_advanced->Refresh();
		if(tab_debugging->isEnabled())	tab_debugging->Refresh();
		if(tab_developer->isEnabled())	tab_developer->Refresh();

		tab_plot->Refresh();
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::ExecuteHelp(QAction *action){
	if(action==actionAbout){
#ifdef VERBOSE
		cout << "About: Common GUI for Mythen, Eiger, Gotthard, Jungfrau, Moench and Propix detectors" << endl;
#endif
		char version[200];
		int64_t retval= SVNREV;
		retval= (retval <<32) | SVNDATE;
		sprintf(version,"%llx",retval);
		string thisGUIVersion = string(version);

		sprintf(version,"%llx",myDet->getId(slsDetectorDefs::THIS_SOFTWARE_VERSION));
		qDefs::checkErrorMessage(myDet,"qDetectorMain::ExecuteHelp");
		string thisClientVersion = string(version);

		//<h1 style="font-family:verdana;">A heading</h1>
		qDefs::Message(qDefs::INFORMATION,"<p style=\"font-family:verdana;\">"
				"SLS Detector GUI version:&nbsp;&nbsp;&nbsp;" + thisGUIVersion+"<br>"
				"SLS Detector Client version:  "+thisClientVersion+"<br><br>"
				"Common GUI to control the SLS Detectors: "
				"Mythen, Eiger, Gotthard, Jungfrau, Moench and Propix.<br><br>"
				"It can be operated in parallel with the command line interface:<br>"
				"sls_detector_put,<br>sls_detector_get,<br>sls_detector_acquire and<br>sls_detector_help.<br><br>"
				"The GUI Software is still in progress. "
				"Please report bugs to dhanya.maliakal@psi.ch or anna.bergamaschi@psi.ch.<\\p>","qDetectorMain::ExecuteHelp");
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::Refresh(int index){
	myDet->setOnline(slsDetectorDefs::ONLINE_FLAG);
	myDet->setReceiverOnline(slsDetectorDefs::ONLINE_FLAG);
	qDefs::checkErrorMessage(myDet,"qDetectorMain::Refresh");
	if(!tabs->isTabEnabled(index))
		tabs->setCurrentIndex((index++)<(tabs->count()-1)?index:Measurement);
	else{
		switch(tabs->currentIndex()){
		case Measurement:	tab_measurement->Refresh();	break;
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

	//actions check
	actionOpenSetup->setEnabled(enable);
	actionSaveSetup->setEnabled(enable);
	actionOpenConfiguration->setEnabled(enable);
	actionSaveConfiguration->setEnabled(enable);
	actionMeasurementWizard->setEnabled(enable);
	actionDebug->setEnabled(enable);
	actionExpert->setEnabled(enable);


	// special tabs
	tabs->setTabEnabled(Debugging,enable && (actionDebug->isChecked()));
	tabs->setTabEnabled(Developer,enable && isDeveloper);
	//expert
	bool expertTab = enable && (actionExpert->isChecked());
	tabs->setTabEnabled(Advanced,expertTab);
	actionLoadTrimbits->setVisible(expertTab);
	actionSaveTrimbits->setVisible(expertTab);
	actionLoadCalibration->setVisible(expertTab);
	actionSaveCalibration->setVisible(expertTab);


	//moved to here, so that its all in order, instead of signals and different threads
	if(!enable) {
		myDet->setOnline(slsDetectorDefs::ONLINE_FLAG);
		myDet->setReceiverOnline(slsDetectorDefs::ONLINE_FLAG);
		qDefs::checkErrorMessage(myDet,"qDetectorMain::EnableTabs");
		//refresh all the required tabs
		tab_actions->Refresh();// angular, positions,

		//too slow to refresh
		/*tab_measurement->Refresh();*/

		tab_settings->Refresh();
		tab_dataoutput->Refresh();
		if(tab_advanced->isEnabled())	tab_advanced->Refresh();
		if(tab_debugging->isEnabled())	tab_debugging->Refresh();
		if(tab_developer->isEnabled())	tab_developer->Refresh();

		tab_plot->Refresh();

		//stop the adc timer in gotthard
		if(isDeveloper)
			tab_developer->StopADCTimer();
		//set the plot type first(acccss shared memory)
		tab_plot->SetScanArgument();
		//sets running to true
		myPlot->StartStopDaqToggle();
	}
	else{//to enable scan box
		tab_plot->Refresh();
		//to start adc timer
		if(tab_developer->isEnabled())
			tab_developer->Refresh();
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


int qDetectorMain::StartStopAcquisitionFromClient(bool start){
#ifdef VERBOSE
	cout << "Start/Stop Acquisition From Client:" << start << endl;
#endif

	if (tab_measurement->GetStartStatus() != start){
		if(start){
			if(!myPlot->isRunning()){
				//refresh all the required tabs - all these are done in button click anyway
			/*	tab_actions->Refresh();
				//too slow to refresh
				//tab_measurement->Refresh();
				tab_settings->Refresh();
				tab_dataoutput->Refresh();
				if(tab_advanced->isEnabled())	tab_advanced->Refresh();
				if(tab_debugging->isEnabled())	tab_debugging->Refresh();
				if(tab_developer->isEnabled())	tab_developer->Refresh();

				tab_plot->Refresh();*/
			}
		}
		//click start/stop
		tab_measurement->ClickStartStop();
		while(myPlot->GetClientInitiated());
	}

	return slsDetectorDefs::OK;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qDetectorMain::UncheckServer(){
#ifdef VERBOSE
	cout << "Unchecking Mode : Listen to Gui Client" << endl;
#endif
	disconnect(menuModes,		SIGNAL(triggered(QAction*)),	this,SLOT(EnableModes(QAction*)));
	actionListenGuiClient->setChecked(false);
	connect(menuModes,		SIGNAL(triggered(QAction*)),	this,SLOT(EnableModes(QAction*)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


bool qDetectorMain::isCurrentlyTabDeveloper(){
	return (tabs->currentIndex()==Developer);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------
