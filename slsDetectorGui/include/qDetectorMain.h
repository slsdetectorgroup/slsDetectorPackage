#pragma once

#include "qDefs.h"
#include "qDrawPlot.h"
#include "qTabDataOutput.h"
#include "qTabMeasurement.h"
#include "ui_form_detectormain.h"
class qTabPlot;
class qTabAdvanced;
class qTabSettings;
class qTabDebugging;
class qTabDeveloper;
class qTabMessages;
class qServer;

class multiSlsDetector;

#include <QGridLayout>
#include <QResizeEvent>
#include <QScrollArea>
#include <QTabWidget>

#include <iostream>

/** To Over-ride the QTabWidget class to get the tabBar */
class MyTabWidget : public QTabWidget {
  public:
    MyTabWidget(QWidget *parent = 0) { setParent(parent); }
    /** Overridden method from QTabWidget */
    QTabBar *tabBar() { return QTabWidget::tabBar(); }
};

/**
 *@short Main window of the GUI.
 */
class qDetectorMain : public QMainWindow, private Ui::DetectorMainObject {
    Q_OBJECT

  public:
    /**
     * Main Window constructor.
     * This is mainly used to create detector object and all the tabs
     * @param argc number of command line arguments for server options
     * @param argv server options
     * @param app the qapplication3
     * @param parent makes the parent window 0 by default
     */
    qDetectorMain(int argc, char **argv, QApplication *app,
                  QWidget *parent = 0);

    /**
     * Destructor
     */
    ~qDetectorMain();

    /**
     * Starts or stops Acquisition From gui client
     * @param start 1 for start and 0 to stop
     * @returns success or fail
     */
    int StartStopAcquisitionFromClient(bool start);

    /**
     * Returns if plot is running
     */
    bool isPlotRunning();

    /**
     * Returns progress bar value
     */
    int GetProgress();

    /**
     * Verifies if output directories for all the receivers exist
     */
    int DoesOutputDirExist();

  private slots:
    /**
     * Enables modes as selected -Debug, Expert, Dockable: calls setdockablemode
     */
    void EnableModes(QAction *action);

    /**
     * Executes actions in the utilities menu as selected
     */
    void ExecuteUtilities(QAction *action);

    /**
     * Executes actions in the utilities menu as selected
     */
    void ExecuteHelp(QAction *action);

    /**
     * Refreshes the tab each time the tab is changed. Also displays the next
     * enabled tab
     */
    void Refresh(int index);

    /**
     * Resizes the main window if the plot is docked/undocked
     * @param b bool TRUE if undocked(outside main window), FALSE docked
     */
    void ResizeMainWindow(bool b);

    /**
     * Enables/disables tabs depending on if acquisition is currently in
     * progress
     */
    void EnableTabs();

    /**
     * Set the tool tip of mouse controlled zooming depening on if its
     * enabled/disabled
     */
    void SetZoomToolTip(bool disable);

    /**
     * Uncheck the Listen to gui client mode when the server has exited
     */
    void UncheckServer();

  protected:
    /**
     * Adjust the resizing to resize plot
     */
    void resizeEvent(QResizeEvent *event);

  private:
    /**
     * Sets up the layout of the widget
     */
    void SetUpWidgetWindow();

    /**
     * Sets up detector
     * @param fName file name of the config file at start up
     * @param multi detector ID
     */
    void SetUpDetector(const std::string fName, int multiID);

    /**
     * Sets up the signals and the slots
     */
    void Initialization();

    /**
     * Loads config file at start up
     */
    void LoadConfigFile(const std::string fName);

    /** enumeration of the tabs */
    enum {
        MEASUREMENT,
        SETTINGS,
        DATAOUTPUT,
        PLOT,
        ADVANCED,
        DEBUGGING,
        DEVELOPER,
        MESSAGES,
        NumberOfTabs
    };
    /** Detector Type */
    slsDetectorDefs::detectorType detType;
    /** The sls detector object */
    multiSlsDetector* myDet;
    /** The Plot widget	 */
    qDrawPlot* myPlot;
    /**Tab Widget */
    MyTabWidget* tabs;
    /**Measurement tab */
    qTabMeasurement* tabMeasurement;
    /**DataOutput tab */
    qTabDataOutput* tabDataOutput;
    /**Plot tab */
    qTabPlot* tabPlot;
    /**Settings tab */
    qTabSettings* tabSettings;
    /**Advanced tab */
    qTabAdvanced* tabAdvanced;
    /**Debugging tab */
    qTabDebugging* tabDebugging;
    /**Developer tab */
    qTabDeveloper* tabDeveloper;
    /**Messages tab */
    qTabMessages* tabMessages;
    /** server object*/
    qServer* myServer;
    /**if the developer tab should be enabled,known from command line */
    int isDeveloper;
    /** default height of Plot Window when docked */
    int heightPlotWindow;
    /** default height of central widgetwhen plot Window when docked */
    int heightCentralWidget;
    /** The default zooming tool tip */
    QString zoomToolTip;
    /** The default tab heading color */
    QColor defaultTabColor;
};
