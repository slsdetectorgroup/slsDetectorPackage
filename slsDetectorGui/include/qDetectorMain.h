/*
 * qDetectorMain.h
 * Main Window of the GUI
 *  Created on: Apr 30, 2012
 *      Author: l_maliakal_d
 */
#ifndef QDETECTORMAIN_H
#define QDETECTORMAIN_H

/** Form Header */
#include "ui_form_detectormain.h"
/** Qt Project Class Headers */
class qDrawPlot;
class qTabMeasurement;
class qTabDataOutput;
class qTabPlot;
class qTabActions;
class qTabAdvanced;
class qTabSettings;
class qTabDebugging;
class qTabDeveloper;
class qTabMessages;
/** Project Class Headers */
class slsDetectorUtils;
#include "sls_detector_defs.h"
/** Qt Include Headers */
#include <QScrollArea>
#include <QGridLayout>
#include <QResizeEvent>


/** To Over-ride the QTabWidget class to get the tabBar */
class MyTabWidget:public QTabWidget{
public:
    MyTabWidget(QWidget* parent = 0) {setParent(parent);}
    /** Overridden method from QTabWidget */
    QTabBar* tabBar(){return QTabWidget::tabBar();}
};


/**
 *@short Main window of the GUI.
 */
class qDetectorMain:public QMainWindow, private Ui::DetectorMainObject{
	Q_OBJECT


public:
	/** \short Main Window constructor.
	 *  This is mainly used to create detector object and all the tabs
	 *   @param argc number of command line arguments for server options
	 *   @param argv server options
	 *   @param app the qapplication
	 *   @param parent makes the parent window 0 by default
	 */
	qDetectorMain(int argc, char **argv, QApplication *app, QWidget *parent = 0);

	/**Destructor */
	~qDetectorMain();

private:
	/** The Qt Application */
	QApplication *theApp;
	/** The sls detector object */
	slsDetectorUtils *myDet;
	/** sls detector id */
	int detID;
	/** The Plot widget	 */
	qDrawPlot *myPlot;
	/**Tab Widget */
	MyTabWidget *tabs;
	/**Layout of the central Widget */
	QGridLayout *layoutTabs;
	/** default height of Plot Window when docked */
	int heightPlotWindow;
	/** default height of central widgetwhen plot Window when docked */
	int heightCentralWidget;
	/** The default zooming tool tip */
	QString zoomToolTip;

	/** The default tab heading color */
	QColor defaultTabColor;
	/** enumeration of the tabs */
	enum {Measurement, Settings, DataOutput, Plot, Actions, Advanced, Debugging, Developer, Messages, NumberOfTabs };
	/* Scroll Area for the tabs**/
	QScrollArea *scroll[NumberOfTabs];
	/**Measurement tab */
	qTabMeasurement 	*tab_measurement;
	/**DataOutput tab */
	qTabDataOutput 		*tab_dataoutput;
	/**Plot tab */
	qTabPlot		 	*tab_plot;
	/**Actions tab */
	qTabActions 		*tab_actions;
	/**Settings tab */
	qTabSettings 		*tab_settings;
	/**Advanced tab */
	qTabAdvanced 		*tab_advanced;
	/**Debugging tab */
	qTabDebugging 		*tab_debugging;
	/**Developer tab */
	qTabDeveloper 		*tab_developer;
	/**Messages tab */
	qTabMessages 		*tab_messages;
	/**if the developer tab should be enabled,known from command line */

	int isDeveloper;
	/**Sets up the layout of the widget
	 */
	void SetUpWidgetWindow();

	/**Sets up detector
	 */
	void SetUpDetector();

	/**Sets up the signals and the slots
	 */
	void Initialization();

	/** Sets/unsets the developer mode (developer tab)
	 * @param b bool TRUE sets, FALSE unsets\
	 */
	void SetDeveloperMode(bool b);

private slots:
/** Sets/unsets the debug mode i.e. enables/disables the debug tab
 *	@param b bool TRUE sets, FALSE unsets
 */
void SetDebugMode(bool b);

/** Sets/unsets the beamline mode (at the moment it doesn't do anything)
 * @param b bool TRUE sets, FALSE unsets
 */
void SetBeamlineMode(bool b);

/** Sets/unsets the expert mode i.e. enables/disables the advanced and Settings tabs
 * @param b bool TRUE sets, FALSE unsets
 */
void SetExpertMode(bool b);

/** Sets/unsets the dockable plot mode
 * @param b bool TRUE sets, FALSE unsets
 */
void SetDockableMode(bool b);

/** Refreshes the tab each time the tab is changed. Also displays the next enabled tab */
void refresh(int index);

/** Opens Setup */
void OpenSetup();

/** Saves Setup */
void SaveSetup();

/** Measurement Wizard */
void MeasurementWizard();

/** Open Configuration*/
void OpenConfiguration();

/** Save Configuration */
void SaveConfiguration();

/** Executing Energy Calibration */
void EnergyCalibration();

/** Executing Angular Calibration */
void AngularCalibration();

/** Executing Version */
void Version();

/** Executing About */
void About();

/** Resizes the main window if the plot is docked/undocked
 *	@param b bool TRUE if undocked(outside main window), FALSE docked
 */
void ResizeMainWindow(bool b);

/** Enables/disables tabs depending on if acquisition is currently in progress */
void EnableTabs();

/** Set the tool tip of mouse controlled zooming depening on if its enabled/disabled*/
void SetZoomToolTip(bool disable);

protected:
void resizeEvent(QResizeEvent* event);

signals:

};

#endif /* QDETECTORMAIN_H */
