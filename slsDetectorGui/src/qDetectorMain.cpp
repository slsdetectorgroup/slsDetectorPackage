// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "qDetectorMain.h"
#include "qDefs.h"
#include "qDrawPlot.h"
#include "qTabAdvanced.h"
#include "qTabDataOutput.h"
#include "qTabDebugging.h"
#include "qTabDeveloper.h"
#include "qTabMeasurement.h"
#include "qTabMessages.h"
#include "qTabPlot.h"
#include "qTabSettings.h"

#include "sls/ToString.h"
#include "sls/versionAPI.h"

#include <QFileDialog>
#include <QResizeEvent>
#include <QSizePolicy>

#include "sls/Version.h"
#include <getopt.h>
#include <string>
#include <sys/stat.h>

std::string getClientVersion() {
    try {
        sls::Version v(APILIB);
        return v.concise();
    } catch (...) {
        return std::string("unknown");
    }
}

int main(int argc, char **argv) {

    // options
    std::string fname;
    bool isDeveloper = false;
    int multiId = 0;

    // parse command line for config
    static struct option long_options[] = {
        // These options set a flag.
        //{"verbose", no_argument,       &verbose_flag, 1},
        // These options don’t set a flag. We distinguish them by their indices.
        {"developer", no_argument, nullptr, 'd'},
        {"config", required_argument, nullptr, 'f'},
        {"id", required_argument, nullptr, 'i'},
        {"version", no_argument, nullptr, 'v'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}};

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
            LOG(sls::logDEBUG)
                << long_options[option_index].name << " " << optarg;
            break;

        case 'd':
            isDeveloper = true;
            break;

        case 'i':
            multiId = atoi(optarg);
            break;

        case 'v':
            LOG(sls::logINFO) << "SLS Detector GUI " << getClientVersion();
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
            LOG(sls::logERROR) << help_message;
            return -1;
        }
    }

    QApplication app(argc, argv);
    try {
        sls::qDetectorMain det(multiId, fname, isDeveloper);
        det.show();
        app.exec();
    } catch (const std::exception &e) {
        sls::qDefs::Message(sls::qDefs::CRITICAL,
                            std::string(e.what()) + "\nExiting Gui :'( ",
                            "main");
    }
    return 0;
}

namespace sls {

qDetectorMain::qDetectorMain(int multiId, const std::string &fname,
                             bool isDevel)
    : QMainWindow(nullptr), detType(slsDetectorDefs::GENERIC),
      isDeveloper(isDevel) {

    setupUi(this);
    SetUpDetector(fname, multiId);
    SetUpWidgetWindow();
}

qDetectorMain::~qDetectorMain() {
    disconnect(tabs, SIGNAL(currentChanged(int)), this, SLOT(Refresh(int)));
}

void qDetectorMain::SetUpWidgetWindow() {
    // plot setup
    plot = new qDrawPlot(dockWidgetPlot, det.get());
    LOG(logDEBUG) << "DockPlot ready";
    dockWidgetPlot->setWidget(plot);
    dockWidgetPlot->setFloating(false);
    zoomToolTip = dockWidgetPlot->toolTip();

    // creating all the other tab widgets
    tabMeasurement = new qTabMeasurement(tMeasurement, det.get(), plot);
    tabDataOutput = new qTabDataOutput(tDataOutput, det.get());
    tabPlot = new qTabPlot(tPlot, det.get(), plot);
    tabSettings = new qTabSettings(tSettings, det.get());
    tabAdvanced = new qTabAdvanced(tAdvanced, det.get(), plot);
    tabDebugging = new qTabDebugging(tDebugging, det.get());
    tabDeveloper = new qTabDeveloper(tDeveloper, det.get());

    scrollMeasurement->setWidget(tabMeasurement);
    scrollDataOutput->setWidget(tabDataOutput);
    scrollPlot->setWidget(tabPlot);
    scrollSettings->setWidget(tabSettings);
    scrollAdvanced->setWidget(tabAdvanced);
    scrollDebugging->setWidget(tabDebugging);
    scrollDeveloper->setWidget(tabDeveloper);
    scrollTerminal->setWidget(tabMessages);

    tabs->setCurrentIndex(MEASUREMENT);
    defaultTabColor = tabs->tabBar()->tabTextColor(DATAOUTPUT);
    // set current tab to blue
    tabs->tabBar()->setTabTextColor(0, QColor(0, 0, 200, 255));
    tabs->setTabEnabled(DEBUGGING, false);
    tabs->setTabEnabled(ADVANCED, false);
    tabs->setTabEnabled(DEVELOPER, isDeveloper);
    actionLoadTrimbits->setVisible(false);
    actionSaveTrimbits->setVisible(false);
    LOG(logINFO) << "Debug Mode: 0, Expert Mode: 0, Developer Mode: "
                 << isDeveloper;

    Initialization();
}

void qDetectorMain::SetUpDetector(const std::string &config_file, int multiID) {

    // instantiate detector and set window title
    det = make_unique<Detector>(multiID);

    // create messages tab to capture config file loading logs
    tabMessages = new qTabMessages(tTerminal);

    // loads the config file at startup
    if (!config_file.empty())
        LoadConfigFile(config_file);

    // validate detector type (for GUI) and update menu
    detType = det->getDetectorType().tsquash(
        "Different detector type for all modules.");
    actionLoadTrimbits->setEnabled(false);
    actionSaveTrimbits->setEnabled(false);
    switch (detType) {
    case slsDetectorDefs::EIGER:
    case slsDetectorDefs::MYTHEN3:
        actionLoadTrimbits->setEnabled(true);
        actionSaveTrimbits->setEnabled(true);
        break;
    case slsDetectorDefs::GOTTHARD:
    case slsDetectorDefs::JUNGFRAU:
    case slsDetectorDefs::MOENCH:
    case slsDetectorDefs::GOTTHARD2:
        break;
    default:
        std::ostringstream os;
        os << det->getHostname() << " has "
           << ToString(det->getDetectorType().squash()) << " detector type ("
           << std::to_string(detType) << "). Exiting GUI.";
        std::string errorMess = os.str();
        throw RuntimeError(errorMess.c_str());
    }
    std::ostringstream os;
    os << "SLS Detector GUI : " << ToString(det->getDetectorType().squash())
       << " - " << det->getHostname();
    std::string title = os.str();
    LOG(logINFO) << title;
    setWindowTitle(QString(title.c_str()));
}

void qDetectorMain::Initialization() {
    // Dockable Plot
    connect(dockWidgetPlot, SIGNAL(topLevelChanged(bool)), this,
            SLOT(ResizeMainWindow(bool)));
    // tabs
    connect(tabs, SIGNAL(currentChanged(int)), this,
            SLOT(Refresh(int))); //( QWidget*)));
    //	Measurement tab
    connect(tabMeasurement, SIGNAL(EnableTabsSignal(bool)), this,
            SLOT(EnableTabs(bool)));
    connect(tabMeasurement, SIGNAL(FileNameChangedSignal(QString)), plot,
            SLOT(SetSaveFileName(QString)));
    // Plot tab
    connect(tabPlot, SIGNAL(DisableZoomSignal(bool)), this,
            SLOT(SetZoomToolTip(bool)));

    // Plotting
    connect(plot, SIGNAL(AcquireFinishedSignal()), tabMeasurement,
            SLOT(AcquireFinished()));
    connect(plot, SIGNAL(AbortSignal(QString)), tabMeasurement,
            SLOT(AbortAcquire(QString)));

    // menubar
    // Modes Menu
    connect(menuModes, SIGNAL(triggered(QAction *)), this,
            SLOT(EnableModes(QAction *)));
    // Utilities Menu
    connect(menuUtilities, SIGNAL(triggered(QAction *)), this,
            SLOT(ExecuteUtilities(QAction *)));
    // Help Menu
    connect(menuHelp, SIGNAL(triggered(QAction *)), this,
            SLOT(ExecuteHelp(QAction *)));
}

void qDetectorMain::LoadConfigFile(const std::string &config_file) {

    LOG(logINFO) << "Loading config file at start up:" << config_file;

    struct stat st_buf;
    QString file = QString(config_file.c_str());

    // path doesnt exist
    if (stat(config_file.c_str(), &st_buf)) {
        qDefs::Message(
            qDefs::WARNING,
            std::string("<nobr>Start up configuration failed to load. The "
                        "following file does not exist:</nobr><br><nobr>") +
                config_file,
            "qDetectorMain::LoadConfigFile");
        LOG(logWARNING) << "Config file does not exist";
    }
    // not a file
    else if (!S_ISREG(st_buf.st_mode)) {
        qDefs::Message(
            qDefs::WARNING,
            std::string(
                "<nobr>Start up configuration failed to load. The following "
                "file is not a recognized file format:</nobr><br><nobr>") +
                config_file,
            "qDetectorMain::LoadConfigFile");
        LOG(logWARNING) << "File not recognized";
    } else {
        try {
            det->loadConfig(config_file);
        }
        CATCH_DISPLAY("Could not load config file.",
                      "qDetectorMain::LoadConfigFile")
    }
}

void qDetectorMain::EnableModes(QAction *action) {
    bool enable;

    // Set DebugMode
    if (action == actionDebug) {
        enable = actionDebug->isChecked();
        tabs->setTabEnabled(DEBUGGING, enable);
        LOG(logINFO) << "Debug Mode: " << qDefs::stringEnable(enable);

    }

    // Set ExpertMode(comes here only if its a digital detector)
    else if (action == actionExpert) {
        enable = actionExpert->isChecked();

        tabs->setTabEnabled(ADVANCED, enable);
        bool visible = enable && (detType == slsDetectorDefs::EIGER ||
                                  detType == slsDetectorDefs::MYTHEN3);
        actionLoadTrimbits->setVisible(visible);
        actionSaveTrimbits->setVisible(visible);
        tabSettings->SetExportMode(enable);
        LOG(logINFO) << "Expert Mode: " << qDefs::stringEnable(enable);
    } else {
        LOG(logERROR) << "Unknown action";
    }
}

void qDetectorMain::ExecuteUtilities(QAction *action) {
    bool refreshTabs = false;
    try {

        if (action == actionLoadConfiguration) {
            LOG(logDEBUG) << "Loading Configuration";
            QString fName = QString(det->getFilePath().squash("/tmp/").c_str());
            fName = QFileDialog::getOpenFileName(
                this, tr("Load Detector Configuration"), fName,
                tr("Configuration files (*.config);;All Files(*)"));
            // Gets called when cancelled as well
            if (!fName.isEmpty()) {
                refreshTabs = true;
                det->loadConfig(std::string(fName.toLatin1().constData()));
                qDefs::Message(qDefs::INFORMATION,
                               "The Configuration Parameters have been "
                               "configured successfully.",
                               "qDetectorMain::ExecuteUtilities");
                LOG(logINFO) << "Configuration Parameters loaded successfully";
            }
        }

        else if (action == actionLoadParameters) {
            LOG(logDEBUG) << "Loading Parameters";
            QString fName = QString(det->getFilePath().squash("/tmp/").c_str());
            fName = QFileDialog::getOpenFileName(
                this, tr("Load Measurement Setup"), fName,
                tr("Parameter files (*.det);;All Files(*)"));
            // Gets called when cancelled as well
            if (!fName.isEmpty()) {
                refreshTabs = true;
                det->loadParameters(std::string(fName.toLatin1().constData()));
                qDefs::Message(qDefs::INFORMATION,
                               "The Detector Parameters have been "
                               "configured successfully.",
                               "qDetectorMain::ExecuteUtilities");
                LOG(logINFO) << "Parameters loaded successfully";
            }
        }

        else if (action == actionLoadTrimbits) {
            QString fName =
                QString((det->getSettingsPath().squash("/tmp/")).c_str());
            LOG(logDEBUG) << "Loading Trimbits";
            // so that even nonexisting files can be selected
            QFileDialog *fileDialog = new QFileDialog(
                this, tr("Load Detector Trimbits"), fName,
                tr("Trimbit files (*.trim noise.sn*);;All Files(*)"));
            fileDialog->setFileMode(QFileDialog::AnyFile);
            if (fileDialog->exec() == QDialog::Accepted) {
                fName = fileDialog->selectedFiles()[0];
                det->loadTrimbits(std::string(fName.toLatin1().constData()));
                qDefs::Message(qDefs::INFORMATION,
                               "The Trimbits have been loaded successfully.",
                               "qDetectorMain::ExecuteUtilities");
                LOG(logINFO) << "Trimbits loaded successfully";
            }
        }

        else if (action == actionSaveTrimbits) {
            QString fPath =
                QString((det->getSettingsPath().squash("/tmp/")).c_str());
            LOG(logDEBUG) << "Saving Trimbits";
            QString fName = QFileDialog::getSaveFileName(
                this, tr("Save Detector Trimbits"), fPath,
                tr("Trimbit files (*.trim noise.sn*);;All Files(*)"));
            if (!fName.isEmpty()) {
                det->saveTrimbits(std::string(fName.toLatin1().constData()));
                qDefs::Message(qDefs::INFORMATION,
                               "The Trimbits have been saved successfully.",
                               "qDetectorMain::ExecuteUtilities");
                LOG(logINFO) << "Trimbits saved successfully";
            }
        }
    }
    CATCH_DISPLAY("Could not execute utilities.",
                  "qDetectorMain::ExecuteUtilities")

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
        LOG(logINFO) << "About Common GUI for Jungfrau, Eiger, Mythen3, "
                        "Gotthard, Gotthard2 and Moench detectors";

        std::string clientVersion = "unknown";
        try {
            clientVersion = det->getClientVersion();
        }
        CATCH_DISPLAY("Could not get client version.",
                      "qDetectorMain::ExecuteHelp")

        qDefs::Message(
            qDefs::INFORMATION,
            "<p style=\"font-family:verdana;\">"

            "<b>SLS Detector Client version:  " +
                clientVersion +
                "</b><br><br>"

                "Common GUI to control the SLS Detectors: "
                "Jungfrau, Eiger, Mythen3, Gotthard, Gotthard2 and "
                "Moench.<br><br>"

                "It can be operated in parallel with the command "
                "line interface: sls_detector_put, sls_detector_get, "
                "sls_detector_acquire and sls_detector_help.<br><br>"

                "Support:<br>"
                "Dhanya.Thattil@psi.ch <br>"
                "Erik.Froejdh@psi.ch.<br><br><br>"

                "<br>slsDetectorGui Copyright (C) 2021 Contributors to SLS "
                "Detector Package<br><br>"

                "See COPYING in root folder for Licensing details:<br>"
                "LGPL-3.0-or-other"

                "<\\p>",
            "qDetectorMain::ExecuteHelp");
    }
}

void qDetectorMain::Refresh(int index) {
    LOG(logDEBUG) << "Refresh Main Tab";

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
    LOG(logDEBUG1) << "Resizing Main Window: height:" << height();
    if (b) {
        setMaximumHeight(centralwidget->height() + menu->height());
        LOG(logINFO) << "Plot undocked from main window";
    } else {
        setMaximumHeight(QWIDGETSIZE_MAX);
        LOG(logINFO) << "Plot docked back to main window";
    }
}

void qDetectorMain::resizeEvent(QResizeEvent *event) {
    tabs->tabBar()->setFixedWidth(width());
    event->accept();
}

void qDetectorMain::EnableTabs(bool enable) {
    LOG(logDEBUG) << "qDetectorMain::EnableTabs";

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
    actionLoadTrimbits->setVisible(expertTab &&
                                   detType == slsDetectorDefs::EIGER);
    actionSaveTrimbits->setVisible(expertTab &&
                                   detType == slsDetectorDefs::EIGER);

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

} // namespace sls
