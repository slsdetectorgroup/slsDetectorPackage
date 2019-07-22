#include "qDetectorMain.h"
#include "qDefs.h"
#include "qDrawPlot.h"
#include "qTabMeasurement.h"
#include "qTabDataOutput.h"
#include "qTabPlot.h"
#include "qTabAdvanced.h"
#include "qTabSettings.h"
#include "qTabDebugging.h"
#include "qTabDeveloper.h"
#include "qTabMessages.h"

#include "versionAPI.h"

#include <QResizeEvent>
#include <QScrollArea>
#include <QFileDialog>
#include <QPlastiqueStyle>
#include <QSizePolicy>

#include <iostream>
#include <getopt.h>
#include <iostream>
#include <string>
#include <sys/stat.h>

int main(int argc, char **argv) {
    QApplication *theApp = new QApplication(argc, argv);
    theApp->setStyle(new QPlastiqueStyle);
    theApp->setWindowIcon(QIcon(":/icons/images/mountain.png"));
    try {
        qDetectorMain *det = new qDetectorMain(argc, argv, theApp, 0);
        det->show();
        theApp->exec();
    } catch (const std::exception &e) {
        qDefs::Message(qDefs::CRITICAL,
                       std::string(e.what()) + "\nExiting Gui :'( ", "main");
    }
    return 0;
}

qDetectorMain::qDetectorMain(int argc, char **argv, QApplication *app,
                             QWidget *parent)
    : QMainWindow(parent), detType(slsDetectorDefs::GENERIC), isDeveloper(0),
      heightPlotWindow(0), heightCentralWidget(0) {

    // options
    std::string fname = "";
    int64_t tempval = 0;
    int multiId = 0;

    // parse command line for config
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
            FILE_LOG(logDEBUG)
                << long_options[option_index].name << " " << optarg;
            break;

        case 'd':
            isDeveloper = 1;
            break;

        case 'i':
            multiId = atoi(optarg);
            break;

        case 'v':
            tempval = APIGUI;
            FILE_LOG(logINFO) << "SLS Detector GUI " << GITBRANCH << " (0x"
                              << std::hex << tempval << ")";
            return;

        case 'h':
        default:
            std::string help_message =
                "\n" + std::string(argv[0]) + "\n" +
                "Usage: " + std::string(argv[0]) + " [arguments]\n" +
                "Possible arguments are:\n" +
                "\t-d, --developer           : Enables the developer tab\n" +
                "\t-f, --config <fname>      : Loads config from file\n" +
                "\t-i, --id <i>              : Sets the multi detector id to "
                "i. Default: 0. Required \n" +
                "\t                            only when more than one multi "
                "detector object is needed.\n\n";
            FILE_LOG(logERROR) << help_message;
            exit(EXIT_FAILURE);
        }
    }

    setupUi(this);
    SetUpDetector(fname, multiId);
    SetUpWidgetWindow();
}

qDetectorMain::~qDetectorMain() {
    if (myDet)
        delete myDet;
    if (myPlot)
        delete myPlot;
    if (tabs)
        delete tabs;
    if (tabMeasurement)
        delete tabMeasurement;
    if (tabDataOutput)
        delete tabDataOutput;
    if (tabPlot)
        delete tabPlot;
    if (tabSettings)
        delete tabSettings;
    if (tabAdvanced)
        delete tabAdvanced;
    if (tabDebugging)
        delete tabDebugging;
    if (tabDeveloper)
        delete tabDeveloper;
    if (tabMessages)
        delete tabMessages;
}

void qDetectorMain::SetUpWidgetWindow() {
    setFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));

    // plot setup
    myPlot = new qDrawPlot(dockWidgetPlot, myDet);
    FILE_LOG(logDEBUG) << "DockPlot ready";
    dockWidgetPlot->setWidget(myPlot);

    // tabs setup
    tabs = new MyTabWidget(this);
    layoutTabs->addWidget(tabs);

    // creating all the other tab widgets
    tabMeasurement = new qTabMeasurement(this, myDet, myPlot);
    tabDataOutput = new qTabDataOutput(this, myDet);
    tabPlot = new qTabPlot(this, myDet, myPlot);
    tabSettings = new qTabSettings(this, myDet);
    tabAdvanced = new qTabAdvanced(this, myDet);
    tabDebugging = new qTabDebugging(this, myDet);
    tabDeveloper = new qTabDeveloper(this, myDet);

    //	creating the scroll area widgets for the tabs
    QScrollArea *scroll[NumberOfTabs];
    for (int i = 0; i < NumberOfTabs; ++i) {
        scroll[i] = new QScrollArea;
        scroll[i]->setFrameShape(QFrame::NoFrame);
    }
    // setting the tab widgets to the scrollareas
    scroll[MEASUREMENT]->setWidget(tabMeasurement);
    scroll[DATAOUTPUT]->setWidget(tabDataOutput);
    scroll[PLOT]->setWidget(tabPlot);
    scroll[SETTINGS]->setWidget(tabSettings);
    scroll[ADVANCED]->setWidget(tabAdvanced);
    scroll[DEBUGGING]->setWidget(tabDebugging);
    scroll[DEVELOPER]->setWidget(tabDeveloper);
    // inserting all the tabs
    tabs->insertTab(MEASUREMENT, scroll[MEASUREMENT], "Measurement");
    tabs->insertTab(DATAOUTPUT, scroll[DATAOUTPUT], "Data Output");
    tabs->insertTab(PLOT, scroll[PLOT], "Plot");
    tabs->insertTab(SETTINGS, scroll[SETTINGS], "Settings");
    tabs->insertTab(ADVANCED, scroll[ADVANCED], "Advanced");
    tabs->insertTab(DEBUGGING, scroll[DEBUGGING], "Debugging");
    tabs->insertTab(DEVELOPER, scroll[DEVELOPER], "Developer");
    // no scroll buttons this way
    tabs->insertTab(MESSAGES, tabMessages, "Messages");

    // swap tabs so that messages is last tab
    tabs->tabBar()->moveTab(tabs->indexOf(tabMeasurement), MEASUREMENT);
    tabs->tabBar()->moveTab(tabs->indexOf(tabSettings), SETTINGS);
    tabs->tabBar()->moveTab(tabs->indexOf(tabDataOutput), DATAOUTPUT);
    tabs->tabBar()->moveTab(tabs->indexOf(tabPlot), PLOT);
    tabs->tabBar()->moveTab(tabs->indexOf(tabAdvanced), ADVANCED);
    tabs->tabBar()->moveTab(tabs->indexOf(tabDebugging), DEBUGGING);
    tabs->tabBar()->moveTab(tabs->indexOf(tabDeveloper), DEVELOPER);
    tabs->tabBar()->moveTab(tabs->indexOf(tabMessages), MESSAGES);
    tabs->setCurrentIndex(MEASUREMENT);

    // other tab properties
    // Default tab color
    defaultTabColor = tabs->tabBar()->tabTextColor(DATAOUTPUT);
    // Set the current tab(measurement) to blue as it is the current one
    tabs->tabBar()->setTabTextColor(0, QColor(0, 0, 200, 255));
    // increase the width so it uses all the empty space for the tab titles
    tabs->tabBar()->setFixedWidth(width() + 61);

    // mode setup - to set up the tabs initially as disabled, not in form so
    // done here
    FILE_LOG(logINFO)
        << "Dockable Mode: 0, Debug Mode: 0, Expert Mode: 0, Developer Mode: "
        << isDeveloper;
    tabs->setTabEnabled(DEBUGGING, false);
    tabs->setTabEnabled(ADVANCED, false);
    tabs->setTabEnabled(DEVELOPER, isDeveloper);
    actionLoadTrimbits->setVisible(false);
    actionSaveTrimbits->setVisible(false);

    dockWidgetPlot->setFloating(false);
    dockWidgetPlot->setFeatures(QDockWidget::NoDockWidgetFeatures);

    // Other setup
    // Height of plot and central widget
    heightPlotWindow = dockWidgetPlot->size().height();
    heightCentralWidget = centralwidget->size().height();
    // Default zoom Tool Tip
    zoomToolTip = dockWidgetPlot->toolTip();

    Initialization();
}

void qDetectorMain::SetUpDetector(const std::string fName, int multiID) {

    // instantiate detector and set window title
    myDet = new multiSlsDetector(multiID);

    // create messages tab to capture config file loading logs
    tabMessages = new qTabMessages(this);

    // loads the config file at startup
    if (!fName.empty())
        LoadConfigFile(fName);

    // validate detector type (for GUI) and update menu
    detType = myDet->getDetectorTypeAsEnum();
    switch (detType) {
    case slsDetectorDefs::EIGER:
        break;
    case slsDetectorDefs::GOTTHARD:
    case slsDetectorDefs::JUNGFRAU:
        actionLoadTrimbits->setText("Load Settings");
        actionSaveTrimbits->setText("Save Settings");
        break;
    case slsDetectorDefs::MOENCH:
        actionLoadTrimbits->setEnabled(false);
        actionSaveTrimbits->setEnabled(false);
        break;
    default:
        std::string errorMess =
            myDet->getHostname() + std::string(" has ") +
            myDet->getDetectorTypeAsString() + std::string(" detector type (") +
            std::to_string(detType) + std::string("). Exiting GUI.");
        throw sls::RuntimeError(errorMess.c_str());
    }

    std::string title =
        "SLS Detector GUI : " + myDet->getDetectorTypeAsString() + " - " +
        myDet->getHostname();
    FILE_LOG(logINFO) << title;
    setWindowTitle(QString(title.c_str()));
    //FIXME: not needed anymore due to client creating socket each time
    //myDet->setOnline(slsDetectorDefs::ONLINE_FLAG);
    //myDet->setReceiverOnline(slsDetectorDefs::ONLINE_FLAG);
}

void qDetectorMain::Initialization() {
    // Dockable Plot
	connect(dockWidgetPlot, SIGNAL(topLevelChanged(bool)), this, SLOT(ResizeMainWindow(bool)));
    // tabs
	connect(tabs,SIGNAL(currentChanged(int)), this, SLOT(Refresh(int)));//( QWidget*)));
    //	Measurement tab
    connect(tabMeasurement, SIGNAL(EnableTabsSignal(bool)), this, SLOT(EnableTabs(bool)));
    connect(tabMeasurement, SIGNAL(FileNameChangedSignal(QString)), myPlot, SLOT(SetSaveFileName(QString)));
    // Plot tab
    connect(tabPlot, SIGNAL(DisableZoomSignal(bool)), this, SLOT(SetZoomToolTip(bool)));

    // Plotting
    connect(myPlot, SIGNAL(AcquireFinishedSignal()), tabMeasurement, SLOT(AcquireFinished()));
    connect(myPlot, SIGNAL(AbortSignal()), tabMeasurement, SLOT(AbortAcquire()));

    // menubar
    // Modes Menu
    connect(menuModes, SIGNAL(triggered(QAction *)), this, SLOT(EnableModes(QAction *)));
    // Utilities Menu
    connect(menuUtilities, SIGNAL(triggered(QAction *)), this, SLOT(ExecuteUtilities(QAction *)));
    // Help Menu
    connect(menuHelp, SIGNAL(triggered(QAction *)), this, SLOT(ExecuteHelp(QAction *)));

}

void qDetectorMain::LoadConfigFile(const std::string fName) {

    FILE_LOG(logINFO) << "Loading config file at start up:" << fName;

    struct stat st_buf;
    QString file = QString(fName.c_str());

    // path doesnt exist
    if (stat(fName.c_str(), &st_buf)) {
        qDefs::Message(
            qDefs::WARNING,
            std::string("<nobr>Start up configuration failed to load. The "
                        "following file does not exist:</nobr><br><nobr>") +
                fName,
            "qDetectorMain::LoadConfigFile");
        FILE_LOG(logWARNING) << "Config file does not exist";
    }
    // not a file
    else if (!S_ISREG(st_buf.st_mode)) {
        qDefs::Message(
            qDefs::WARNING,
            std::string(
                "<nobr>Start up configuration failed to load. The following "
                "file is not a recognized file format:</nobr><br><nobr>") +
                fName,
            "qDetectorMain::LoadConfigFile");
        FILE_LOG(logWARNING) << "File not recognized";
    } else {
        try {
            myDet->readConfigurationFile(fName);
        } CATCH_DISPLAY ("Could not load config file.", "qDetectorMain::LoadConfigFile")
    }
}

void qDetectorMain::EnableModes(QAction *action) {
    bool enable;

    // Set DebugMode
    if (action == actionDebug) {
        enable = actionDebug->isChecked();
        tabs->setTabEnabled(DEBUGGING, enable);
        FILE_LOG(logINFO) << "Debug Mode: "
                          << slsDetectorDefs::stringEnable(enable);

    }

    // Set ExpertMode(comes here only if its a digital detector)
    else if (action == actionExpert) {
        enable = actionExpert->isChecked();

        tabs->setTabEnabled(ADVANCED, enable);
        actionLoadTrimbits->setVisible(enable && detType != slsDetectorDefs::MOENCH);
        actionSaveTrimbits->setVisible(enable && detType != slsDetectorDefs::MOENCH);
        FILE_LOG(logINFO) << "Expert Mode: "
                          << slsDetectorDefs::stringEnable(enable);
    }

    // Set DockableMode
    else {
        enable = actionDockable->isChecked();
        if (enable) {
            dockWidgetPlot->setFeatures(QDockWidget::DockWidgetFloatable);
        } else {
            dockWidgetPlot->setFloating(false);
            dockWidgetPlot->setFeatures(QDockWidget::NoDockWidgetFeatures);
        }
        FILE_LOG(logINFO) << "Dockable Mode: "
                          << slsDetectorDefs::stringEnable(enable);
    }
}

void qDetectorMain::ExecuteUtilities(QAction *action) {
    bool refreshTabs = false;
    try {
        if (action == actionOpenSetup) {
            FILE_LOG(logDEBUG) << "Loading Setup";
            QString fName = QString(myDet->getFilePath().c_str());
            fName = QFileDialog::getOpenFileName(
                this, tr("Load Detector Setup"), fName,
                tr("Detector Setup files (*.det);;All Files(*)"));
            // Gets called when cancelled as well
            if (!fName.isEmpty()) {
                refreshTabs = true;
                myDet->retrieveDetectorSetup(
                    std::string(fName.toAscii().constData()));
                qDefs::Message(
                    qDefs::INFORMATION,
                    "The Setup Parameters have been loaded successfully.",
                    "qDetectorMain::ExecuteUtilities");
                FILE_LOG(logINFO) << "Setup Parameters loaded successfully";
            }
        }

        else if (action == actionSaveSetup) {
            FILE_LOG(logDEBUG) << "Saving Setup";
            QString fName = QString(myDet->getFilePath().c_str());
            fName = QFileDialog::getSaveFileName(
                this, tr("Save Current Detector Setup"), fName,
                tr("Detector Setup files (*.det);;All Files(*) "));
            // Gets called when cancelled as well
            if (!fName.isEmpty()) {
                myDet->dumpDetectorSetup(
                    std::string(fName.toAscii().constData()));
                qDefs::Message(
                    qDefs::INFORMATION,
                    "The Setup Parameters have been saved successfully.",
                    "qDetectorMain::ExecuteUtilities");
                FILE_LOG(logINFO) << "Setup Parameters saved successfully";
            }
        }

        else if (action == actionOpenConfiguration) {
            FILE_LOG(logDEBUG) << "Loading Configuration";
            QString fName = QString(myDet->getFilePath().c_str());
            fName = QFileDialog::getOpenFileName(
                this, tr("Load Detector Configuration"), fName,
                tr("Configuration files (*.config);;All Files(*)"));
            // Gets called when cancelled as well
            if (!fName.isEmpty()) {
                refreshTabs = true;
                myDet->readConfigurationFile(
                    std::string(fName.toAscii().constData()));
                qDefs::Message(qDefs::INFORMATION,
                               "The Configuration Parameters have been "
                               "configured successfully.",
                               "qDetectorMain::ExecuteUtilities");
                FILE_LOG(logINFO)
                    << "Configuration Parameters loaded successfully";
            }
        }

        else if (action == actionSaveConfiguration) {
            FILE_LOG(logDEBUG) << "Saving Configuration";
            QString fName = QString(myDet->getFilePath().c_str());
            fName = QFileDialog::getSaveFileName(
                this, tr("Save Current Detector Configuration"), fName,
                tr("Configuration files (*.config) ;;All Files(*)"));
            // Gets called when cancelled as well
            if (!fName.isEmpty()) {
                myDet->writeConfigurationFile(
                    std::string(fName.toAscii().constData()));
                qDefs::Message(qDefs::INFORMATION,
                               "The Configuration Parameters have been saved "
                               "successfully.",
                               "qDetectorMain::ExecuteUtilities");
                FILE_LOG(logINFO)
                    << "Configuration Parameters saved successfully";
            }
        }

        else if (action == actionLoadTrimbits) {
            QString fName = QString((myDet->getSettingsDir()).c_str());
            // gotthard
            if (actionLoadTrimbits->text().contains("Settings")) {
                FILE_LOG(logDEBUG) << "Loading Settings";
                fName = QFileDialog::getOpenFileName(
                    this, tr("Load Detector Settings"), fName,
                    tr("Settings files (*.settings settings.sn*);;All "
                       "Files(*)"));
                // Gets called when cancelled as well
                if (!fName.isEmpty()) {
                    myDet->loadSettingsFile(
                        std::string(fName.toAscii().constData()), -1);
                    qDefs::Message(
                        qDefs::INFORMATION,
                        "The Settings have been loaded successfully.",
                        "qDetectorMain::ExecuteUtilities");
                    FILE_LOG(logINFO) << "Settings loaded successfully";
                }

            } // mythen and eiger
            else {
                FILE_LOG(logDEBUG) << "Loading Trimbits";
                // so that even nonexisting files can be selected
                QFileDialog *fileDialog = new QFileDialog(
                    this, tr("Load Detector Trimbits"), fName,
                    tr("Trimbit files (*.trim noise.sn*);;All Files(*)"));
                fileDialog->setFileMode(QFileDialog::AnyFile);
                if (fileDialog->exec() == QDialog::Accepted)
                    fName = fileDialog->selectedFiles()[0];

                // Gets called when cancelled as well
                if (!fName.isEmpty()) {
                    myDet->loadSettingsFile(
                        std::string(fName.toAscii().constData()), -1);
                    qDefs::Message(
                        qDefs::INFORMATION,
                        "The Trimbits have been loaded successfully.",
                        "qDetectorMain::ExecuteUtilities");
                    FILE_LOG(logINFO) << "Trimbits loaded successfully";
                }
            }
        }

        else if (action == actionSaveTrimbits) {
            // gotthard
            if (actionLoadTrimbits->text().contains("Settings")) {
                FILE_LOG(logDEBUG) << "Saving Settings";
                // different output directory so as not to overwrite
                QString fName = QString((myDet->getSettingsDir()).c_str());
                fName = QFileDialog::getSaveFileName(
                    this, tr("Save Current Detector Settings"), fName,
                    tr("Settings files (*.settings settings.sn*);;All "
                       "Files(*) "));
                // Gets called when cancelled as well
                if (!fName.isEmpty()) {
                    myDet->saveSettingsFile(
                        std::string(fName.toAscii().constData()), -1);
                    qDefs::Message(qDefs::INFORMATION,
                                   "The Settings have been saved successfully.",
                                   "qDetectorMain::ExecuteUtilities");
                    FILE_LOG(logINFO) << "Settings saved successfully";
                }
            } // mythen and eiger
            else {
                FILE_LOG(logDEBUG) << "Saving Trimbits";
                // different output directory so as not to overwrite
                QString fName = QString((myDet->getSettingsDir()).c_str());
                fName = QFileDialog::getSaveFileName(
                    this, tr("Save Current Detector Trimbits"), fName,
                    tr("Trimbit files (*.trim noise.sn*) ;;All Files(*)"));
                // Gets called when cancelled as well
                if (!fName.isEmpty()) {
                    myDet->saveSettingsFile(
                        std::string(fName.toAscii().constData()), -1);
                    qDefs::Message(qDefs::INFORMATION,
                                   "The Trimbits have been saved successfully.",
                                   "qDetectorMain::ExecuteUtilities");
                    FILE_LOG(logINFO) << "Trimbits saved successfully";
                }
            }
        }
    } CATCH_DISPLAY ("Could not execute utilities.", "qDetectorMain::ExecuteUtilities")

    Refresh(tabs->currentIndex());
    if (refreshTabs) {
        tabMeasurement->Refresh();
        tabSettings->Refresh();
        tabDataOutput->Refresh();
        if (tabAdvanced->isEnabled())
            tabAdvanced->Refresh();
        if (tabDebugging->isEnabled())
            tabDebugging->Refresh();
        if (tabDeveloper->isEnabled())
            tabDeveloper->Refresh();
        tabPlot->Refresh();
    }
}

void qDetectorMain::ExecuteHelp(QAction *action) {
    if (action == actionAbout) {
        FILE_LOG(logINFO) << "About Common GUI for Eiger, Gotthard, Jungfrau "
                             "and Moench detectors";

        std::string guiVersion = std::to_string(APIGUI);
        std::string clientVersion = "unknown";
        try {
            clientVersion = std::to_string(myDet->getId(slsDetectorDefs::THIS_SOFTWARE_VERSION));
        } CATCH_DISPLAY ("Could not get client version.", "qDetectorMain::ExecuteHelp")

        qDefs::Message(qDefs::INFORMATION,
                       "<p style=\"font-family:verdana;\">"
                       "SLS Detector GUI version:&nbsp;&nbsp;&nbsp;" +
                           guiVersion +
                           "<br>"
                           "SLS Detector Client version:  " +
                           clientVersion +
                           "<br><br>"
                           "Common GUI to control the SLS Detectors: "
                           "Eiger, Gotthard, Jungfrau and Moench.<br><br>"
                           "It can be operated in parallel with the command "
                           "line interface:<br>"
                           "sls_detector_put,<br>sls_detector_get,<br>sls_"
                           "detector_acquire and<br>sls_detector_help.<br><br>"
                           "Please report bugs to: <br>"
                           "Dhanya.Thattil@psi.ch, <br>"
                           "Erik.Froejdh@psi.ch or  <br>"
                           "Anna.Bergamaschi@psi.ch.<\\p>",
                       "qDetectorMain::ExecuteHelp");
    }
}

void qDetectorMain::Refresh(int index) {
    FILE_LOG(logDEBUG) << "Refresh Main Tab";

    if (!tabs->isTabEnabled(index))
        tabs->setCurrentIndex((index++) < (tabs->count() - 1) ? index
                                                              : MEASUREMENT);
    else {
        switch (tabs->currentIndex()) {
        case MEASUREMENT:
            tabMeasurement->Refresh();
            break;
        case SETTINGS:
            tabSettings->Refresh();
            break;
        case DATAOUTPUT:
            tabDataOutput->Refresh();
            break;
        case PLOT:
            tabPlot->Refresh();
            break;
        case ADVANCED:
            tabAdvanced->Refresh();
            break;
        case DEBUGGING:
            tabDebugging->Refresh();
            break;
        case DEVELOPER:
            tabDeveloper->Refresh();
            break;
        case MESSAGES:
            tabMessages->Refresh();
            break;
        }
    }
    for (int i = 0; i < NumberOfTabs; ++i)
        tabs->tabBar()->setTabTextColor(i, defaultTabColor);
    tabs->tabBar()->setTabTextColor(index, QColor(0, 0, 200, 255));
}

void qDetectorMain::ResizeMainWindow(bool b) {
    FILE_LOG(logDEBUG1) << "Resizing Main Window: height:" << height();

    // undocked from the main window
    if (b) {
        // sets the main window height to a smaller maximum to get rid of space
        setMaximumHeight(height() - heightPlotWindow - 9);
        dockWidgetPlot->setMinimumHeight(0);
        FILE_LOG(logINFO) << "Undocking from main window";
    } else {
        setMaximumHeight(QWIDGETSIZE_MAX);
        // the minimum for plot will be set when the widget gets resized
        // automatically
    }
}

void qDetectorMain::resizeEvent(QResizeEvent *event) {
    if (!dockWidgetPlot->isFloating()) {
        dockWidgetPlot->setMinimumHeight(height() - centralwidget->height() -
                                         50);
        centralwidget->setMaximumHeight(heightCentralWidget);
    }

    // adjusting tab width
    if (width() >= 800) {
        tabs->tabBar()->setFixedWidth(width() + 61);
    } else {
        tabs->tabBar()->setMinimumWidth(0);
        tabs->tabBar()->setExpanding(true);
        tabs->tabBar()->setUsesScrollButtons(true);
    }

    event->accept();
}

void qDetectorMain::EnableTabs(bool enable) {
    FILE_LOG(logDEBUG) << "qDetectorMain::EnableTabs";

    // normal tabs
    tabs->setTabEnabled(DATAOUTPUT, enable);
    tabs->setTabEnabled(SETTINGS, enable);
    tabs->setTabEnabled(MESSAGES, enable);

    // actions check
    actionOpenSetup->setEnabled(enable);
    actionSaveSetup->setEnabled(enable);
    actionOpenConfiguration->setEnabled(enable);
    actionSaveConfiguration->setEnabled(enable);
    actionDebug->setEnabled(enable);
    actionExpert->setEnabled(enable);

    // special tabs
    tabs->setTabEnabled(DEBUGGING, enable && (actionDebug->isChecked()));
    tabs->setTabEnabled(DEVELOPER, enable && isDeveloper);
    // expert
    bool expertTab = enable && (actionExpert->isChecked());
    tabs->setTabEnabled(ADVANCED, expertTab);
    actionLoadTrimbits->setVisible(expertTab && detType != slsDetectorDefs::MOENCH);
    actionSaveTrimbits->setVisible(expertTab && detType != slsDetectorDefs::MOENCH);

    // moved to here, so that its all in order, instead of signals and different
    // threads
    if (!enable) {
        // tabMeasurement->Refresh(); too slow to refresh
        tabSettings->Refresh();
        tabDataOutput->Refresh();
        if (tabAdvanced->isEnabled())
            tabAdvanced->Refresh();
        if (tabDebugging->isEnabled())
            tabDebugging->Refresh();
        if (tabDeveloper->isEnabled())
            tabDeveloper->Refresh();

        tabPlot->Refresh();
        myPlot->StartAcquisition();
    } else { // to enable scan box
        tabPlot->Refresh();
        // to start adc timer
        if (tabDeveloper->isEnabled())
            tabDeveloper->Refresh();
    }
}

void qDetectorMain::SetZoomToolTip(bool disable) {
    if (disable)
        dockWidgetPlot->setToolTip(
            "<span style=\" color:#00007f;\">To Enable mouse-controlled "
            "zooming capabilities,\ndisable min and max for all axes.<span> ");
    else
        dockWidgetPlot->setToolTip(zoomToolTip);
}
