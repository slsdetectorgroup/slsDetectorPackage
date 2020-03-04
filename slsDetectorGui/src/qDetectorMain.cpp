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
#include "ToString.h"

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

    // options
    std::string fname = "";
    bool isDeveloper = false;
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
            isDeveloper = true;
            break;

        case 'i':
            multiId = atoi(optarg);
            break;

        case 'v':
            tempval = APIGUI;
            FILE_LOG(logINFO) << "SLS Detector GUI " << GITBRANCH << " (0x"
                              << std::hex << tempval << ")";
            return 0;

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
            return -1;
        }
    }


    QApplication app(argc, argv);
    app.setStyle(new QPlastiqueStyle);
    //app.setWindowIcon(QIcon(":/icons/images/mountain.png"));
    try {
        qDetectorMain det(multiId, fname, isDeveloper);
        det.show();
        app.exec();
    } catch (const std::exception &e) {
        qDefs::Message(qDefs::CRITICAL,
                       std::string(e.what()) + "\nExiting Gui :'( ", "main");
    }
    return 0;
}

qDetectorMain::qDetectorMain(int multiId, std::string fname, bool isDevel)
    : QMainWindow(0), detType(slsDetectorDefs::GENERIC), isDeveloper(isDevel),
      heightPlotWindow(0), heightCentralWidget(0) {

    setupUi(this);
    SetUpDetector(fname, multiId);
    SetUpWidgetWindow();
}

qDetectorMain::~qDetectorMain() {}

void qDetectorMain::SetUpWidgetWindow() {
    setFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));

    // plot setup
    plot = sls::make_unique<qDrawPlot>(dockWidgetPlot, det.get());
    FILE_LOG(logDEBUG) << "DockPlot ready";
    dockWidgetPlot->setWidget(plot.get());

    // tabs setup
    tabs = sls::make_unique<MyTabWidget>(this);
    layoutTabs->addWidget(tabs.get());

    // creating all the other tab widgets
    tabMeasurement = sls::make_unique<qTabMeasurement>(this, det.get(), plot.get());
    tabDataOutput = sls::make_unique<qTabDataOutput>(this, det.get());
    tabPlot = sls::make_unique<qTabPlot>(this, det.get(), plot.get());
    tabSettings = sls::make_unique<qTabSettings>(this, det.get());
    tabAdvanced = sls::make_unique<qTabAdvanced>(this, det.get(), plot.get());
    tabDebugging = sls::make_unique<qTabDebugging>(this, det.get());
    tabDeveloper = sls::make_unique<qTabDeveloper>(this, det.get());

    //	creating the scroll area widgets for the tabs
    QScrollArea *scroll[NumberOfTabs];
    for (int i = 0; i < NumberOfTabs; ++i) {
        scroll[i] = new QScrollArea;
        scroll[i]->setFrameShape(QFrame::NoFrame);
    }
    // setting the tab widgets to the scrollareas
    scroll[MEASUREMENT]->setWidget(tabMeasurement.get());
    scroll[DATAOUTPUT]->setWidget(tabDataOutput.get());
    scroll[PLOT]->setWidget(tabPlot.get());
    scroll[SETTINGS]->setWidget(tabSettings.get());
    scroll[ADVANCED]->setWidget(tabAdvanced.get());
    scroll[DEBUGGING]->setWidget(tabDebugging.get());
    scroll[DEVELOPER]->setWidget(tabDeveloper.get());
    // inserting all the tabs
    tabs->insertTab(MEASUREMENT, scroll[MEASUREMENT], "Measurement");
    tabs->insertTab(DATAOUTPUT, scroll[DATAOUTPUT], "Data Output");
    tabs->insertTab(PLOT, scroll[PLOT], "Plot");
    tabs->insertTab(SETTINGS, scroll[SETTINGS], "Settings");
    tabs->insertTab(ADVANCED, scroll[ADVANCED], "Advanced");
    tabs->insertTab(DEBUGGING, scroll[DEBUGGING], "Debugging");
    tabs->insertTab(DEVELOPER, scroll[DEVELOPER], "Developer");
    // no scroll buttons this way
    tabs->insertTab(MESSAGES, tabMessages.get(), "Terminal");

    // swap tabs so that messages is last tab
    tabs->tabBar()->moveTab(tabs->indexOf(tabMeasurement.get()), MEASUREMENT);
    tabs->tabBar()->moveTab(tabs->indexOf(tabSettings.get()), SETTINGS);
    tabs->tabBar()->moveTab(tabs->indexOf(tabDataOutput.get()), DATAOUTPUT);
    tabs->tabBar()->moveTab(tabs->indexOf(tabPlot.get()), PLOT);
    tabs->tabBar()->moveTab(tabs->indexOf(tabAdvanced.get()), ADVANCED);
    tabs->tabBar()->moveTab(tabs->indexOf(tabDebugging.get()), DEBUGGING);
    tabs->tabBar()->moveTab(tabs->indexOf(tabDeveloper.get()), DEVELOPER);
    tabs->tabBar()->moveTab(tabs->indexOf(tabMessages.get()), MESSAGES);
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
    det = sls::make_unique<sls::Detector>(multiID);

    // create messages tab to capture config file loading logs
    tabMessages = sls::make_unique<qTabMessages>(this);

    // loads the config file at startup
    if (!fName.empty())
        LoadConfigFile(fName);

    // validate detector type (for GUI) and update menu
    detType = det->getDetectorType().tsquash("Different detector type for all modules.");
    actionLoadTrimbits->setEnabled(false);
    switch (detType) {
    case slsDetectorDefs::EIGER:
        actionLoadTrimbits->setEnabled(true);
        break;
    case slsDetectorDefs::GOTTHARD:
    case slsDetectorDefs::JUNGFRAU:
    case slsDetectorDefs::MOENCH:
    case slsDetectorDefs::MYTHEN3:
    case slsDetectorDefs::GOTTHARD2:
         break;
    default:
        std::ostringstream os;
        os << det->getHostname() << " has " <<
            sls::ToString(det->getDetectorType().squash()) << " detector type (" <<
            std::to_string(detType) << "). Exiting GUI.";
        std::string errorMess = os.str();
        throw sls::RuntimeError(errorMess.c_str());
    }
    std::ostringstream os;
    os << "SLS Detector GUI : " << sls::ToString(det->getDetectorType().squash()) 
        << " - " << det->getHostname();
    std::string title = os.str();
    FILE_LOG(logINFO) << title;
    setWindowTitle(QString(title.c_str()));
}

void qDetectorMain::Initialization() {
    // Dockable Plot
	connect(dockWidgetPlot, SIGNAL(topLevelChanged(bool)), this, SLOT(ResizeMainWindow(bool)));
    // tabs
	connect(tabs.get(),SIGNAL(currentChanged(int)), this, SLOT(Refresh(int)));//( QWidget*)));
    //	Measurement tab
    connect(tabMeasurement.get(), SIGNAL(EnableTabsSignal(bool)), this, SLOT(EnableTabs(bool)));
    connect(tabMeasurement.get(), SIGNAL(FileNameChangedSignal(QString)), plot.get(), SLOT(SetSaveFileName(QString)));
    // Plot tab
    connect(tabPlot.get(), SIGNAL(DisableZoomSignal(bool)), this, SLOT(SetZoomToolTip(bool)));

    // Plotting
    connect(plot.get(), SIGNAL(AcquireFinishedSignal()), tabMeasurement.get(), SLOT(AcquireFinished()));
    connect(plot.get(), SIGNAL(AbortSignal()), tabMeasurement.get(), SLOT(AbortAcquire()));

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
            det->loadConfig(fName);
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
                          << qDefs::stringEnable(enable);

    }

    // Set ExpertMode(comes here only if its a digital detector)
    else if (action == actionExpert) {
        enable = actionExpert->isChecked();

        tabs->setTabEnabled(ADVANCED, enable);
        actionLoadTrimbits->setVisible(enable && detType == slsDetectorDefs::EIGER);
        FILE_LOG(logINFO) << "Expert Mode: "
                          << qDefs::stringEnable(enable);
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
                          << qDefs::stringEnable(enable);
    }
}

void qDetectorMain::ExecuteUtilities(QAction *action) {
    bool refreshTabs = false;
    try {

        if (action == actionLoadConfiguration) {
            FILE_LOG(logDEBUG) << "Loading Configuration";
            QString fName = QString(det->getFilePath().squash("/tmp/").c_str());
            fName = QFileDialog::getOpenFileName(
                this, tr("Load Detector Configuration"), fName,
                tr("Configuration files (*.config);;All Files(*)"));
            // Gets called when cancelled as well
            if (!fName.isEmpty()) {
                refreshTabs = true;
                det->loadConfig(
                    std::string(fName.toAscii().constData()));
                qDefs::Message(qDefs::INFORMATION,
                               "The Configuration Parameters have been "
                               "configured successfully.",
                               "qDetectorMain::ExecuteUtilities");
                FILE_LOG(logINFO)
                    << "Configuration Parameters loaded successfully";
            }
        }

        else if (action == actionLoadParameters) {
            FILE_LOG(logDEBUG) << "Loading Parameters";
            QString fName = QString(det->getFilePath().squash("/tmp/").c_str());
            fName = QFileDialog::getOpenFileName(
                this, tr("Load Measurement Setup"), fName,
                tr("Parameter files (*.det);;All Files(*)"));
            // Gets called when cancelled as well
            if (!fName.isEmpty()) {
                refreshTabs = true;
                det->loadParameters(
                    std::string(fName.toAscii().constData()));
                qDefs::Message(qDefs::INFORMATION,
                               "The Detector Parameters have been "
                               "configured successfully.",
                               "qDetectorMain::ExecuteUtilities");
                FILE_LOG(logINFO)
                    << "Parameters loaded successfully";
            }
        }

        else if (action == actionLoadTrimbits) {
            QString fName = QString((det->getSettingsPath().squash("/tmp/")).c_str());
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
                det->loadTrimbits(
                    std::string(fName.toAscii().constData()));
                qDefs::Message(
                    qDefs::INFORMATION,
                    "The Trimbits have been loaded successfully.",
                    "qDetectorMain::ExecuteUtilities");
                FILE_LOG(logINFO) << "Trimbits loaded successfully";
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
        FILE_LOG(logINFO) << "About Common GUI for Jungfrau, Eiger, Mythen3, Gotthard, Gotthard2 and Moench detectors";

        std::string guiVersion = std::to_string(APIGUI);
        std::string clientVersion = "unknown";
        try {
            clientVersion = std::to_string(det->getClientVersion());
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
                           "Jungfrau, Eiger, Mythen3, Gotthard, Gotthard2 and Moench.<br><br>"
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
    actionLoadConfiguration->setEnabled(enable);
    actionLoadParameters->setEnabled(enable);
    actionDebug->setEnabled(enable);
    actionExpert->setEnabled(enable);

    // special tabs
    tabs->setTabEnabled(DEBUGGING, enable && (actionDebug->isChecked()));
    tabs->setTabEnabled(DEVELOPER, enable && isDeveloper);
    // expert
    bool expertTab = enable && (actionExpert->isChecked());
    tabs->setTabEnabled(ADVANCED, expertTab);
    actionLoadTrimbits->setVisible(expertTab && detType == slsDetectorDefs::EIGER);

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
        plot->StartAcquisition();
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
