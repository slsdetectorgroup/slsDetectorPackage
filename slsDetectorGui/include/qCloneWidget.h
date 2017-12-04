/*
 * qCloneWidget.h
 *
 *  Created on: May 18, 2012
 *      Author: l_maliakal_d
 */

#ifndef QCLONEWIDGET_H_
#define QCLONEWIDGET_H_

#include "qDefs.h"


/** Qt Project Class Headers */
class SlsQtH1D;
#include "SlsQt1DPlot.h"
#include "SlsQt2DPlotLayout.h"
/** Qt Include Headers */
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QGridLayout>
#include <QCloseEvent>
#include <QGroupBox>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QString>
#include <QSpacerItem>
#include <QWidget>
/** C++ Include Headers */
#include <string>
#include <iostream>
using namespace std;

/**
 *@short Sets up the clone plot widget
 */
class qCloneWidget:public QMainWindow{
	Q_OBJECT

public:
	/** \short The constructor
	 */
	qCloneWidget(QWidget *parent,int id,QString title,QString xTitle, QString yTitle, QString zTitle, int numDim,string FilePath,
		bool displayStats, QString min, QString max, QString sum);

	/** Destructor
	 */
	~qCloneWidget();

	/** Sets up the widget window
	 * @param title title of the image with frame number
	 * @param xTitle title of x axis
	 * @param yTitle title of y axis
	 * @param zTitle title of z axis
	 * @param numDim 1D or 2D
	 * */
	void SetupWidgetWindow(QString title, QString xTitle, QString yTitle, QString zTitle, int numDim);



	/** Get the 1D hist values to plot
	 * @param nHists Number of graphs in 1D
	 * @param histNBins Total Number of X axis values/channels in 1D
	 * @param histXAxis X Axis value in 1D
	 * @param histYAxis Y Axis value in 1D
	 * @param histTitle Title for all the graphs in 1D
	 * @param lines style of plot if lines or dots
	 * @param markers style of plot markers or not
	 * */
	void SetCloneHists(int nHists,int histNBins,double* histXAxis,double* histYAxis[],string histTitle[],bool lines,bool markers);

	/** Get the 1D hist values to plot for angle plotting
	 * @param nHists Number of graphs in 1D
	 * @param histNBins Total Number of X axis values/channels in 1D
	 * @param histXAxis X Axis value in 1D
	 * @param histYAxis Y Axis value in 1D
	 * @param histTitle Title for all the graphs in 1D
	 * @param lines style of plot if lines or dots
	 * @param markers style of plot markers or not
	 * */
	void SetCloneHists(int nHists,int histNBins,double* histXAxis,double* histYAxis,string histTitle[],bool lines,bool markers);

	/** Get the 1D hist values to plot for angle plotting
	 * @param nbinsx number of bins in x axis
	 * @param xmin minimum in x axis
	 * @param xmax maximum in x axis
	 * @param nbinsy number of bins in y axis
	 * @param ymin minimum in y axis
	 * @param ymax maximum in y axis
	 * @param d data
	 * */
	void SetCloneHists2D(int nbinsx,double xmin,double xmax,int nbinsy, double ymin, double ymax, double *d);

	/**Set the range of the 1d plot
	 * @param IsXYRange array of x,y,min,max if these values are set
	 * @param XYRangeValues array of set values of x,y, min, max
	 */
	void SetRange(bool IsXYRange[], double XYRangeValues[]);

	/** Returns the 1d plot
	 */
	SlsQt1DPlot* Get1dPlot(){return cloneplot1D;};

public slots:
/** Save Plots automatically by save all clones
 * returns -1 if fail*/
int SavePlotAutomatic();


private:
	/**	clone window id*/
	int id;
	/** Default Save file path */
	string filePath;

	/**	clone 2D Plot */
	SlsQt2DPlotLayout* 	cloneplot2D;
	/**	clone 1D Plot */
	SlsQt1DPlot* 		cloneplot1D;
	/**	vector of 1D hist values */
	QVector<SlsQtH1D*> 	cloneplot1D_hists;

	/** markers for the plot*/
	QwtSymbol *marker;
	QwtSymbol *nomarker;

	 QMenuBar 	*menubar;
	 QAction 	*actionSave;

	QGridLayout *mainLayout;
	QGroupBox 	*cloneBox;
	QGridLayout *gridClone;

    QLabel		*lblHistTitle;

    /** Gets the current time stamp for the window title*/
    char* GetCurrentTimeStamp();

    /** Display Statistics */
    void DisplayStats(bool enable, QString min, QString max, QString sum);


private slots:
/** Save Plot */
void SavePlot();

protected:
	void closeEvent(QCloseEvent* event);

signals:
void CloneClosedSignal(int);

};




#endif /* QCLONEWIDGET_H_ */
