/**********************************************************************
 * TO DO
 * 1. settcpsocket is done with slsdetector.maybe do for all detectors connected: mythen
 * ********************************************************************/
#include "qDetectorMain.h"
#include "qServer.h"
#include "qTabAdvanced.h"
#include "qTabDataOutput.h"
#include "qTabDebugging.h"
#include "qTabDeveloper.h"
#include "qTabMessages.h"
#include "qTabPlot.h"
#include "qTabSettings.h"

#include "gitInfoGui.h"
#include "multiSlsDetector.h"
#include "slsDetector.h"
#include "sls_detector_defs.h"

#include <QFileDialog>
#include <QPlastiqueStyle>
#include <QSizePolicy>

#include <getopt.h>
#include <iostream>
#include <string>
#include <sys/stat.h>

//-------------------------------------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv) {
    QApplication *theApp = new QApplication(argc, argv);
    theApp->setStyle(new QPlastiqueStyle);
    theApp->setWindowIcon(QIcon(":/icons/images/mountain.png"));
    try{
    	qDetectorMain *det = new qDetectorMain(argc, argv, theApp, 0);
        det->show();
    } catch(...) {
    	;
    }
    return theApp->exec();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

qDetectorMain::qDetectorMain(int argc, char **argv, QApplication *app, QWidget *parent) : QMainWindow(parent), myDet(0), myPlot(0), tabs(0), isDeveloper(0) {

    // options
    std::string fname = "";
    int64_t tempval = 0;
    int multiId = 0;

    //parse command line for config
    static struct option long_options[] = {
        // These options set a flag.
        //{"verbose", no_argument,       &verbose_flag, 1},
        // These options don’t set a flag. We distinguish them by their indices.
        {"developer", no_argument, 0, 'd'},
        {"config", required_argument, 0, 'f'},
        {"id", required_argument, 0, 'i'},
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}};

    // getopt_long stores the option index here
	optind = 1;
    int option_index = 0;
    int c = 0;

    while (c != -1) {
        c = getopt_long(argc, argv, "hvdf:i:", long_options, &option_index);
        // Detect the end of the options
        if (c == -1)
            break;
        switch (c) {
			
        case 'f':
            fname = optarg;
            FILE_LOG(logDEBUG) << long_options[option_index].name << " " << optarg;
            break;

        case 'd':
            isDeveloper = 1;
            break;

        case 'i':
        	multiId = atoi(optarg);
            break;

        case 'v':
            tempval = GITDATE;
            FILE_LOG(logINFO) << "SLS Detector GUI " << GITBRANCH << " (0x" << std::hex << tempval << ")";
            return;

        case 'h':
        default:
            std::string help_message = "\n"
            		+ std::string(argv[0]) + "\n"
					+ "Usage: " + std::string(argv[0]) + " [arguments]\n"
					+ "Possible arguments are:\n"
					+ "\t-d, --developer           : Enables the developer tab\n"
					+ "\t-f, --config <fname>      : Loads config from file\n"
					+ "\t-i, --id <i>              : Sets the multi detector id to i. Default: 0. Required \n"
					+ "\t                            only when more than one multi detector object is needed.\n\n";
            FILE_LOG(logERROR) << help_message << '\n';
            throw std::exception();
        }
    }

    setupUi(this);
    SetUpDetector(fname, multiId);
    SetUpWidgetWindow();
    Initialization();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

qDetectorMain::~qDetectorMain() {
    if (myDet)
        delete myDet;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

bool qDetectorMain::isPlotRunning() {
	return myPlot->isRunning();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

int qDetectorMain::GetProgress() {
	return tab_measurement->GetProgress();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

QString qDetectorMain::GetFilePath() {
    QString s = QString(myDet->getFilePath().c_str());
    qDefs::checkErrorMessage(myDet);
    return s;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

int qDetectorMain::DoesOutputDirExist() {
	return tab_dataoutput->VerifyOutputDirectory();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

bool qDetectorMain::isCurrentlyTabDeveloper() {
    return (tabs->currentIndex() == Developer);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDetectorMain::SetUpWidgetWindow() {

    // Layout
	QGridLayout *layoutTabs = new QGridLayout;
    centralwidget->setLayout(layoutTabs);

    //plot setup
    myPlot = new qDrawPlot(dockWidgetPlot, myDet);
    FILE_LOG(logDEBUG) << "DockPlot ready";
    dockWidgetPlot->setWidget(myPlot);

    //tabs setup
    tabs = new MyTabWidget(this);
    layoutTabs->addWidget(tabs);

    // creating all the other tab widgets
    tab_measurement = new qTabMeasurement(this, myDet, myPlot);
    FILE_LOG(logDEBUG) << "Measurement ready";
    tab_dataoutput = new qTabDataOutput(this, myDet);
    FILE_LOG(logDEBUG) << "DataOutput ready";
    tab_plot = new qTabPlot(this, myDet, myPlot);
    FILE_LOG(logDEBUG) << "Plot ready";
    tab_settings = new qTabSettings(this, myDet);
    FILE_LOG(logDEBUG) << "Settings ready";
    tab_advanced = new qTabAdvanced(this, myDet, myPlot);
    FILE_LOG(logDEBUG) << "Advanced ready";
    tab_debugging = new qTabDebugging(this, myDet);
    FILE_LOG(logDEBUG) << "Debugging ready";
    tab_developer = new qTabDeveloper(this, myDet);
    FILE_LOG(logDEBUG) << "Developer ready";
    myServer = new qServer(this);
    FILE_LOG(logDEBUG) << "Client Server ready";

    //	creating the scroll area widgets for the tabs
    QScrollArea *scroll[NumberOfTabs];
    for (int i = 0; i < NumberOfTabs; i++) {
        scroll[i] = new QScrollArea;
        scroll[i]->setFrameShape(QFrame::NoFrame);
    }
    // setting the tab widgets to the scrollareas
    scroll[Measurement]->setWidget(tab_measurement);
    scroll[DataOutput]->setWidget(tab_dataoutput);
    scroll[Plot]->setWidget(tab_plot);
    scroll[Settings]->setWidget(tab_settings);
    scroll[Advanced]->setWidget(tab_advanced);
    scroll[Debugging]->setWidget(tab_debugging);
    scroll[Developer]->setWidget(tab_developer);
    // inserting all the tabs
    tabs->insertTab(Measurement, scroll[Measurement], "Measurement");
    tabs->insertTab(DataOutput, scroll[DataOutput], "Data Output");
    tabs->insertTab(Plot, scroll[Plot], "Plot");
    tabs->insertTab(Settings, scroll[Settings], "Settings");
    tabs->insertTab(Advanced, scroll[Advanced], "Advanced");
    tabs->insertTab(Debugging, scroll[Debugging], "Debugging");
    tabs->insertTab(Developer, scroll[Developer], "Developer");
    //no scroll buttons this way
    tabs->insertTab(Messages, tab_messages, "Messages");

    //swap tabs so that messages is last tab
    tabs->tabBar()->moveTab(tabs->indexOf(tab_measurement), Measurement);
    tabs->tabBar()->moveTab(tabs->indexOf(tab_settings), Settings);
    tabs->tabBar()->moveTab(tabs->indexOf(tab_dataoutput), DataOutput);
    tabs->tabBar()->moveTab(tabs->indexOf(tab_plot), Plot);
    tabs->tabBar()->moveTab(tabs->indexOf(tab_advanced), Advanced);
    tabs->tabBar()->moveTab(tabs->indexOf(tab_debugging), Debugging);
    tabs->tabBar()->moveTab(tabs->indexOf(tab_developer), Developer);
    tabs->tabBar()->moveTab(tabs->indexOf(tab_messages), Messages);
    tabs->setCurrentIndex(Measurement);

    //other tab properties
    // Default tab color
    defaultTabColor = tabs->tabBar()->tabTextColor(DataOutput);
    //Set the current tab(measurement) to blue as it is the current one
    tabs->tabBar()->setTabTextColor(0, QColor(0, 0, 200, 255));
    // increase the width so it uses all the empty space for the tab titles
    tabs->tabBar()->setFixedWidth(width() + 61);

    // mode setup - to set up the tabs initially as disabled, not in form so done here
    FILE_LOG(logINFO) << "Dockable Mode: 0, Debug Mode: 0, Expert Mode: 0, Developer Mode: " << isDeveloper;
    tabs->setTabEnabled(Debugging, false);
    tabs->setTabEnabled(Advanced, false);
    tabs->setTabEnabled(Developer, isDeveloper);
    actionLoadTrimbits->setVisible(false);
    actionSaveTrimbits->setVisible(false);

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

void qDetectorMain::SetUpDetector(const std::string fName, int multiID) {

    //instantiate detector and set window title
    myDet = new multiSlsDetector(multiID);

    //create messages tab to capture config file loading logs
    tab_messages = new qTabMessages(this);
    FILE_LOG(logDEBUG) << "Messages ready";

    //loads the config file at startup
    if (!fName.empty())
        LoadConfigFile(fName);

    //gets the hostname if it worked
    std::string host = myDet->getHostname();
    qDefs::checkErrorMessage(myDet, "qDetectorMain::SetUpDetector");

    //if hostname doesnt exist even in shared memory
    if (!host.length()) {
    	FILE_LOG(logERROR)  << "No Detector Connected.";
        qDefs::Message(qDefs::CRITICAL, "No Detectors Connected. ", "qDetectorMain::SetUpDetector");
        throw std::exception();
    }

    //check if the detector is not even connected
    std::string offline = myDet->checkOnline();
    qDefs::checkErrorMessage(myDet, "qDetectorMain::SetUpDetector");

    if (!offline.empty()) {
        qDefs::Message(qDefs::CRITICAL, std::string("<nobr>The detector(s)  <b>") + offline + std::string(" </b> is/are not connected.  Exiting GUI.</nobr>"), "qDetectorMain::SetUpDetector");
        FILE_LOG(logERROR) << "The detector(s)  " << host << "  is/are not connected. Exiting GUI.";
        throw std::exception();
    }

    // Check if type valid. If not, exit
    slsDetectorDefs::detectorType detType = myDet->getDetectorTypeAsEnum();
    qDefs::checkErrorMessage(myDet, "qDetectorMain::SetUpDetector");

    switch (detType) {
    case slsDetectorDefs::EIGER:
        break;
    case slsDetectorDefs::GOTTHARD:
    case slsDetectorDefs::MOENCH:
    case slsDetectorDefs::JUNGFRAU:
    case slsDetectorDefs::CHIPTESTBOARD:
        actionLoadTrimbits->setText("Load Settings");
        actionSaveTrimbits->setText("Save Settings");
        break;
    default:
        std::string detName = myDet->getDetectorTypeAsString();
        qDefs::checkErrorMessage(myDet, "qDetectorMain::SetUpDetector");
        std::string errorMess = host + std::string(" has unknown detector type \"") +
                           detName + std::string("\". Exiting GUI.");
        FILE_LOG(logERROR) << errorMess;
        qDefs::Message(qDefs::CRITICAL, errorMess, "qDetectorMain::SetUpDetector");
        throw std::exception();
    }

    setWindowTitle("SLS Detector GUI : " +
                   QString(myDet->getDetectorTypeAsString().c_str()) + " - " + QString(host.c_str()));
    FILE_LOG(logINFO) << "Type : " << myDet->getDetectorTypeAsString() << "\nDetector : " << host;

    myDet->setOnline(slsDetectorDefs::ONLINE_FLAG);
    myDet->setReceiverOnline(slsDetectorDefs::ONLINE_FLAG);
    qDefs::checkErrorMessage(myDet, "qDetectorMain::SetUpDetector");
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDetectorMain::Initialization() {
    // Dockable Plot
    connect(dockWidgetPlot, SIGNAL(topLevelChanged(bool)), this, SLOT(ResizeMainWindow(bool)));
    // tabs
    connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(Refresh(int))); //( QWidget*)));
    //	Measurement tab
    connect(tab_measurement, SIGNAL(StartSignal()), this, SLOT(EnableTabs()));
    connect(tab_measurement, SIGNAL(StopSignal()), myPlot, SLOT(StopAcquisition()));
    connect(tab_measurement, SIGNAL(CheckPlotIntervalSignal()), tab_plot, SLOT(SetFrequency()));
     // Plot tab
    connect(tab_plot, SIGNAL(DisableZoomSignal(bool)), this, SLOT(SetZoomToolTip(bool)));
    //settings to advanced tab(int is always 0 to only refresh)
    connect(tab_settings, SIGNAL(UpdateTrimbitSignal(int)), tab_advanced, SLOT(UpdateTrimbitPlot(int)));

    // Plotting
    // When the acquisition is finished, must update the meas tab
    connect(myPlot, SIGNAL(UpdatingPlotFinished()), this, SLOT(EnableTabs()));
    connect(myPlot, SIGNAL(UpdatingPlotFinished()), tab_measurement, SLOT(UpdateFinished()));
    connect(myPlot, SIGNAL(SetCurrentMeasurementSignal(int)), tab_measurement, SLOT(SetCurrentMeasurement(int)));

    // menubar
    // Modes Menu
    connect(menuModes, SIGNAL(triggered(QAction *)), this, SLOT(EnableModes(QAction *)));
    // Utilities Menu
    connect(menuUtilities, SIGNAL(triggered(QAction *)), this, SLOT(ExecuteUtilities(QAction *)));
    // Help Menu
    connect(menuHelp, SIGNAL(triggered(QAction *)), this, SLOT(ExecuteHelp(QAction *)));

    //server
    connect(myServer, SIGNAL(ServerStoppedSignal()), this, SLOT(UncheckServer()));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDetectorMain::LoadConfigFile(const std::string fName) {

	FILE_LOG(logINFO) << "Loading config file at start up:" << fName;

    struct stat st_buf;
    QString file = QString(fName.c_str());

    //path doesnt exist
    if (stat(fName.c_str(), &st_buf)) {
        qDefs::Message(qDefs::WARNING, std::string("<nobr>Start up configuration failed to load. The following file does not exist:</nobr><br><nobr>") + fName, "qDetectorMain::LoadConfigFile");
        FILE_LOG(logWARNING) << "Config file does not exist";
    }
    //not a file
    else if (!S_ISREG(st_buf.st_mode)) {
        qDefs::Message(qDefs::WARNING, std::string("<nobr>Start up configuration failed to load. The following file is not a recognized file format:</nobr><br><nobr>") + fName, "qDetectorMain::LoadConfigFile");
        FILE_LOG(logWARNING) << "File not recognized";
    }
    else {
        //could not load config file
        if (myDet->readConfigurationFile(fName) == slsDetectorDefs::FAIL) {
            qDefs::Message(qDefs::WARNING, std::string("<nobr>Could not load all the Configuration Parameters from file:<br>") + fName, "qDetectorMain::LoadConfigFile");
            FILE_LOG(logWARNING) << "Could not load all configuration parameters";
        }
        //successful
        else {
            qDefs::Message(qDefs::INFORMATION, "<nobr>The Configuration Parameters have been loaded successfully at start up.</nobr>", "qDetectorMain::LoadConfigFile");
            FILE_LOG(logINFO) << "Config file loaded successfully";
        }
        qDefs::checkErrorMessage(myDet, "qDetectorMain::LoadConfigFile");
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDetectorMain::EnableModes(QAction *action) {
    bool enable;

    //listen to gui client
    if (action == actionListenGuiClient) {
        myServer->StartStopServer(actionListenGuiClient->isChecked());
     }
    //Set DebugMode
    else if (action == actionDebug) {
        enable = actionDebug->isChecked();
        tabs->setTabEnabled(Debugging, enable);
        FILE_LOG(logINFO) << "Debug Mode: " << slsDetectorDefs::stringEnable(enable);

    }

    //Set ExpertMode(comes here only if its a digital detector)
    else if (action == actionExpert) {
        enable = actionExpert->isChecked();

        tabs->setTabEnabled(Advanced, enable);
        actionLoadTrimbits->setVisible(enable);
        actionSaveTrimbits->setVisible(enable);
        tab_measurement->SetExpertMode(enable);
        tab_settings->SetExpertMode(enable);
        FILE_LOG(logINFO) << "Expert Mode: " << slsDetectorDefs::stringEnable(enable);
    }

    //Set DockableMode
    else {
        enable = actionDockable->isChecked();
        if (enable) {
            dockWidgetPlot->setFeatures(QDockWidget::DockWidgetFloatable);
        } else {
            dockWidgetPlot->setFloating(false);
            dockWidgetPlot->setFeatures(QDockWidget::NoDockWidgetFeatures);
        }
        FILE_LOG(logINFO) << "Dockable Mode: " << slsDetectorDefs::stringEnable(enable);
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDetectorMain::ExecuteUtilities(QAction *action) {
    bool refreshTabs = false;

    if (action == actionOpenSetup) {
    	FILE_LOG(logDEBUG) << "Loading Setup";
    	QString fName = QString(myDet->getFilePath().c_str());
    	qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteUtilities");
    	fName = QFileDialog::getOpenFileName(this,
    			tr("Load Detector Setup"), fName,
				tr("Detector Setup files (*.det);;All Files(*)"));
    	// Gets called when cancelled as well
    	if (!fName.isEmpty()) {
    		if (myDet->retrieveDetectorSetup(std::string(fName.toAscii().constData())) != slsDetectorDefs::FAIL) {
    			qDefs::Message(qDefs::INFORMATION, "The Setup Parameters have been loaded successfully.", "qDetectorMain::ExecuteUtilities");
    			refreshTabs = true;
    	    	FILE_LOG(logINFO) << "Setup Parameters loaded successfully";
    		} else {
    			qDefs::Message(qDefs::WARNING, std::string("Could not load the Setup Parameters from file:\n") + fName.toAscii().constData(), "qDetectorMain::ExecuteUtilities");
    	    	FILE_LOG(logWARNING) << "Could not load Setup Parameters";
    		}
    		qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteUtilities");
    	}
    }

    else if (action == actionSaveSetup) {
    	FILE_LOG(logDEBUG) << "Saving Setup";
        QString fName = QString(myDet->getFilePath().c_str());
        qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteUtilities");
        fName = QFileDialog::getSaveFileName(this,
                                             tr("Save Current Detector Setup"), fName,
                                             tr("Detector Setup files (*.det);;All Files(*) "));
        // Gets called when cancelled as well
        if (!fName.isEmpty()) {
            if (myDet->dumpDetectorSetup(std::string(fName.toAscii().constData())) != slsDetectorDefs::FAIL) {
                qDefs::Message(qDefs::INFORMATION, "The Setup Parameters have been saved successfully.", "qDetectorMain::ExecuteUtilities");
                FILE_LOG(logINFO) << "Setup Parameters saved successfully";
            } else {
                qDefs::Message(qDefs::WARNING, std::string("Could not save the Setup Parameters from file:\n") + fName.toAscii().constData(), "qDetectorMain::ExecuteUtilities");
                FILE_LOG(logWARNING) << "Could not save Setup Parameters";
            }
            qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteUtilities");
        }
    }

    else if (action == actionOpenConfiguration) {
    	FILE_LOG(logDEBUG) << "Loading Configuration";
        QString fName = QString(myDet->getFilePath().c_str());
        qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteUtilities");
        fName = QFileDialog::getOpenFileName(this,
                                             tr("Load Detector Configuration"), fName,
                                             tr("Configuration files (*.config);;All Files(*)"));
        // Gets called when cancelled as well
        if (!fName.isEmpty()) {
            if (myDet->readConfigurationFile(std::string(fName.toAscii().constData())) != slsDetectorDefs::FAIL) {
                qDefs::Message(qDefs::INFORMATION, "The Configuration Parameters have been configured successfully.", "qDetectorMain::ExecuteUtilities");
                refreshTabs = true;
                FILE_LOG(logINFO) << "Configuration Parameters loaded successfully";
            } else {
                qDefs::Message(qDefs::WARNING, std::string("Could not load all the Configuration Parameters from file:\n") + fName.toAscii().constData(), "qDetectorMain::ExecuteUtilities");
            	FILE_LOG(logWARNING) << "Could not load all Configuration Parameters";
            }
            qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteUtilities");
        }
    }

    else if (action == actionSaveConfiguration) {
    	FILE_LOG(logDEBUG) << "Saving Configuration";
        QString fName = QString(myDet->getFilePath().c_str());
        qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteUtilities");
        fName = QFileDialog::getSaveFileName(this,
                                             tr("Save Current Detector Configuration"), fName,
                                             tr("Configuration files (*.config) ;;All Files(*)"));
        // Gets called when cancelled as well
        if (!fName.isEmpty()) {
            if (myDet->writeConfigurationFile(std::string(fName.toAscii().constData())) != slsDetectorDefs::FAIL) {
                qDefs::Message(qDefs::INFORMATION, "The Configuration Parameters have been saved successfully.", "qDetectorMain::ExecuteUtilities");
                FILE_LOG(logINFO) << "Configuration Parameters saved successfully";
            } else {
                qDefs::Message(qDefs::WARNING, std::string("Could not save the Configuration Parameters from file:\n") + fName.toAscii().constData(), "qDetectorMain::ExecuteUtilities");
                FILE_LOG(logWARNING) << "Could not save Configuration Parameters";
            }
            qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteUtilities");
        }
    }

    else if (action == actionLoadTrimbits) {
        QString fName = QString((myDet->getSettingsDir()).c_str());
        qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteUtilities");
        //gotthard
        if (actionLoadTrimbits->text().contains("Settings")) {
        	FILE_LOG(logDEBUG) << "Loading Settings";
            fName = QFileDialog::getOpenFileName(this,
                                                 tr("Load Detector Settings"), fName,
                                                 tr("Settings files (*.settings settings.sn*);;All Files(*)"));
            // Gets called when cancelled as well
            if (!fName.isEmpty()) {
                if (myDet->loadSettingsFile(std::string(fName.toAscii().constData()), -1) != slsDetectorDefs::FAIL) {
                    qDefs::Message(qDefs::INFORMATION, "The Settings have been loaded successfully.", "qDetectorMain::ExecuteUtilities");
                } else {
                    qDefs::Message(qDefs::WARNING, std::string("Could not load the Settings from file:\n") + fName.toAscii().constData(), "qDetectorMain::ExecuteUtilities");
                }
                qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteUtilities");
            }

        } //mythen and eiger
        else {
        	FILE_LOG(logDEBUG) << "Loading Trimbits";
            //so that even nonexisting files can be selected
            QFileDialog *fileDialog = new QFileDialog(this,
                                                      tr("Load Detector Trimbits"), fName,
                                                      tr("Trimbit files (*.trim noise.sn*);;All Files(*)"));
            fileDialog->setFileMode(QFileDialog::AnyFile);
            if (fileDialog->exec() == QDialog::Accepted)
                fName = fileDialog->selectedFiles()[0];

            // Gets called when cancelled as well
            if (!fName.isEmpty()) {
                if (myDet->loadSettingsFile(std::string(fName.toAscii().constData()), -1) != slsDetectorDefs::FAIL) {
                    qDefs::Message(qDefs::INFORMATION, "The Trimbits have been loaded successfully.", "qDetectorMain::ExecuteUtilities");
                } else {
                    qDefs::Message(qDefs::WARNING, std::string("Could not load the Trimbits from file:\n") + fName.toAscii().constData(), "qDetectorMain::ExecuteUtilities");
                }
                qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteUtilities");
            }
        }
    }

    else if (action == actionSaveTrimbits) {
        //gotthard
        if (actionLoadTrimbits->text().contains("Settings")) {
        	FILE_LOG(logDEBUG) << "Saving Settings";
            //different output directory so as not to overwrite
            QString fName = QString((myDet->getSettingsDir()).c_str());
            qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteUtilities");
            fName = QFileDialog::getSaveFileName(this,
                                                 tr("Save Current Detector Settings"), fName,
                                                 tr("Settings files (*.settings settings.sn*);;All Files(*) "));
            // Gets called when cancelled as well
            if (!fName.isEmpty()) {
                if (myDet->saveSettingsFile(std::string(fName.toAscii().constData()), -1) != slsDetectorDefs::FAIL)
                    qDefs::Message(qDefs::INFORMATION, "The Settings have been saved successfully.", "qDetectorMain::ExecuteUtilities");
                else
                    qDefs::Message(qDefs::WARNING, std::string("Could not save the Settings to file:\n") + fName.toAscii().constData(), "qDetectorMain::ExecuteUtilities");
                qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteUtilities");
            }
        } //mythen and eiger
        else {
        	FILE_LOG(logDEBUG) << "Saving Trimbits";
        	//different output directory so as not to overwrite
            QString fName = QString((myDet->getSettingsDir()).c_str());
            qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteUtilities");
            fName = QFileDialog::getSaveFileName(this,
                                                 tr("Save Current Detector Trimbits"), fName,
                                                 tr("Trimbit files (*.trim noise.sn*) ;;All Files(*)"));
            // Gets called when cancelled as well
            if (!fName.isEmpty()) {
                if (myDet->saveSettingsFile(std::string(fName.toAscii().constData()), -1) != slsDetectorDefs::FAIL)
                    qDefs::Message(qDefs::INFORMATION, "The Trimbits have been saved successfully.", "qDetectorMain::ExecuteUtilities");
                else
                    qDefs::Message(qDefs::WARNING, std::string("Could not save the Trimbits to file:\n") + fName.toAscii().constData(), "qDetectorMain::ExecuteUtilities");
                qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteUtilities");
            }
        }
    }

    Refresh(tabs->currentIndex());
    if (refreshTabs) {
        tab_measurement->Refresh();
        tab_settings->Refresh();
        tab_dataoutput->Refresh();
        if (tab_advanced->isEnabled())
            tab_advanced->Refresh();
        if (tab_debugging->isEnabled())
            tab_debugging->Refresh();
        if (tab_developer->isEnabled())
            tab_developer->Refresh();

        tab_plot->Refresh();
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDetectorMain::ExecuteHelp(QAction *action) {
    if (action == actionAbout) {
#ifdef VERBOSE
        std::cout << "About: Common GUI for Eiger, Gotthard, Jungfrau, Moench and Propix detectors" << '\n';
#endif
        char version[200];
        long long unsigned int retval = GITDATE;
        sprintf(version, "%llx", retval);
        std::string thisGUIVersion{version};

        sprintf(version, "%llx", (long long unsigned int)myDet->getId(slsDetectorDefs::THIS_SOFTWARE_VERSION));
        qDefs::checkErrorMessage(myDet, "qDetectorMain::ExecuteHelp");
        std::string thisClientVersion{version};

        qDefs::Message(qDefs::INFORMATION, "<p style=\"font-family:verdana;\">"
                                           "SLS Detector GUI version:&nbsp;&nbsp;&nbsp;" +
                                               thisGUIVersion + "<br>"
                                                                "SLS Detector Client version:  " +
                                               thisClientVersion + "<br><br>"
                                                                   "Common GUI to control the SLS Detectors: "
                                                                   "Mythen, Eiger, Gotthard, Jungfrau, Moench and Propix.<br><br>"
                                                                   "It can be operated in parallel with the command line interface:<br>"
                                                                   "sls_detector_put,<br>sls_detector_get,<br>sls_detector_acquire and<br>sls_detector_help.<br><br>"
                                                                   "The GUI Software is still in progress. "
                                                                   "Please report bugs to dhanya.maliakal@psi.ch or anna.bergamaschi@psi.ch.<\\p>",
                       "qDetectorMain::ExecuteHelp");
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDetectorMain::Refresh(int index) {
    myDet->setOnline(slsDetectorDefs::ONLINE_FLAG);
    myDet->setReceiverOnline(slsDetectorDefs::ONLINE_FLAG);
    qDefs::checkErrorMessage(myDet, "qDetectorMain::Refresh");
    if (!tabs->isTabEnabled(index))
        tabs->setCurrentIndex((index++) < (tabs->count() - 1) ? index : Measurement);
    else {
        switch (tabs->currentIndex()) {
        case Measurement:
            tab_measurement->Refresh();
            break;
        case Settings:
            tab_settings->Refresh();
            break;
        case DataOutput:
            tab_dataoutput->Refresh();
            break;
        case Plot:
            tab_plot->Refresh();
            break;
        case Advanced:
            tab_advanced->Refresh();
            break;
        case Debugging:
            tab_debugging->Refresh();
            break;
        case Developer:
            tab_developer->Refresh();
            break;
        case Messages:
            break;
        }
    }
    for (int i = 0; i < NumberOfTabs; i++)
        tabs->tabBar()->setTabTextColor(i, defaultTabColor);
    tabs->tabBar()->setTabTextColor(index, QColor(0, 0, 200, 255));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDetectorMain::ResizeMainWindow(bool b) {
#ifdef VERBOSE
    std::cout << "Resizing Main Window: height:" << height() << '\n';
#endif
    // undocked from the main window
    if (b) {
        // sets the main window height to a smaller maximum to get rid of space
        setMaximumHeight(height() - heightPlotWindow - 9);
        dockWidgetPlot->setMinimumHeight(0);
        std::cout << "undocking it from main window" << '\n';
    } else {
        setMaximumHeight(QWIDGETSIZE_MAX);
        // the minimum for plot will be set when the widget gets resized automatically
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDetectorMain::resizeEvent(QResizeEvent *event) {
    if (!dockWidgetPlot->isFloating()) {
            dockWidgetPlot->setMinimumHeight(height() - centralwidget->height() - 50);
            centralwidget->setMaximumHeight(heightCentralWidget);
    }

    //adjusting tab width
    if (width() >= 800) {
        tabs->tabBar()->setFixedWidth(width() + 61);
    } else {
        tabs->tabBar()->setMinimumWidth(0);
        tabs->tabBar()->setExpanding(true);
        tabs->tabBar()->setUsesScrollButtons(true);
    }

    event->accept();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDetectorMain::EnableTabs() {
#ifdef VERBOSE
    std::cout << "Entering EnableTabs function" << '\n';
#endif

    bool enable;
    enable = !(tabs->isTabEnabled(DataOutput));

    // or use the Enable/Disable button
    // normal tabs
    tabs->setTabEnabled(DataOutput, enable);
    tabs->setTabEnabled(Settings, enable);
    tabs->setTabEnabled(Messages, enable);

    //actions check
    actionOpenSetup->setEnabled(enable);
    actionSaveSetup->setEnabled(enable);
    actionOpenConfiguration->setEnabled(enable);
    actionSaveConfiguration->setEnabled(enable);
    actionMeasurementWizard->setEnabled(enable);
    actionDebug->setEnabled(enable);
    actionExpert->setEnabled(enable);

    // special tabs
    tabs->setTabEnabled(Debugging, enable && (actionDebug->isChecked()));
	std::cout << "Developer: " << enable << " isdev: " << isDeveloper << '\n';
    tabs->setTabEnabled(Developer, enable && isDeveloper);
    //expert
    bool expertTab = enable && (actionExpert->isChecked());
    tabs->setTabEnabled(Advanced, expertTab);
    actionLoadTrimbits->setVisible(expertTab);
    actionSaveTrimbits->setVisible(expertTab);

    //moved to here, so that its all in order, instead of signals and different threads
    if (!enable) {
        myDet->setOnline(slsDetectorDefs::ONLINE_FLAG);
        myDet->setReceiverOnline(slsDetectorDefs::ONLINE_FLAG);
        qDefs::checkErrorMessage(myDet, "qDetectorMain::EnableTabs");

        tab_settings->Refresh();
        tab_dataoutput->Refresh();
        if (tab_advanced->isEnabled())
            tab_advanced->Refresh();
        if (tab_debugging->isEnabled())
            tab_debugging->Refresh();
        if (tab_developer->isEnabled())
            tab_developer->Refresh();

        tab_plot->Refresh();

        //stop the adc timer in gotthard
        if (isDeveloper)
            tab_developer->StopADCTimer();
        //set the plot type first(acccss shared memory)
        // tab_plot->SetScanArgument();
        //sets running to true
        myPlot->StartStopDaqToggle();
    } else { //to enable scan box
        tab_plot->Refresh();
        //to start adc timer
        if (tab_developer->isEnabled())
            tab_developer->Refresh();
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDetectorMain::SetZoomToolTip(bool disable) {
    if (disable)
        dockWidgetPlot->setToolTip("<span style=\" color:#00007f;\">To Enable mouse-controlled zooming capabilities,\ndisable min and max for all axes.<span> ");
    else
        dockWidgetPlot->setToolTip(zoomToolTip);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

int qDetectorMain::StartStopAcquisitionFromClient(bool start) {
#ifdef VERBOSE
    std::cout << "Start/Stop Acquisition From Client:" << start << '\n';
#endif

    if (tab_measurement->GetStartStatus() != start) {
        tab_measurement->ClickStartStop();
    }

    return slsDetectorDefs::OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qDetectorMain::UncheckServer() {
#ifdef VERBOSE
    std::cout << "Unchecking Mode : Listen to Gui Client\n";
#endif
    disconnect(menuModes, SIGNAL(triggered(QAction *)), this, SLOT(EnableModes(QAction *)));
    actionListenGuiClient->setChecked(false);
    connect(menuModes, SIGNAL(triggered(QAction *)), this, SLOT(EnableModes(QAction *)));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------
