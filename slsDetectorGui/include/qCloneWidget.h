/*
 * qCloneWidget.h
 *
 *  Created on: May 18, 2012
 *      Author: l_maliakal_d
 */

#ifndef QCLONEWIDGET_H_
#define QCLONEWIDGET_H_

/** Qt Project Class Headers */
class SlsQtH1D;
class SlsQt1DPlot;
class SlsQt2DPlotLayout;
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
/** C++ Include Headers */
#include <string>
using namespace std;

/**
 *@short Sets up the clone plot widget
 */
class qCloneWidget:public QMainWindow{
	Q_OBJECT

public:
	/** \short The constructor
	 */
	qCloneWidget(QWidget *parent,int id,QString title,int numDim,SlsQt1DPlot*& plot1D,SlsQt2DPlotLayout*& plot2D,string FilePath);

	/** Destructor
	 */
	~qCloneWidget();

	/** Sets up the widget window
	 * @param title title of the image with frame number
	 * @param numDim 1D or 2D
	 * @param plot1D plot1d object reference
	 * @param plot2D plot2d object reference
	 * */
	void SetupWidgetWindow(QString title,int numDim,SlsQt1DPlot*& plot1D,SlsQt2DPlotLayout*& plot2D);



	/** Get the 1D hist values to plot
	 * @param nHists Number of graphs in 1D
	 * @param histNBins Total Number of X axis values/channels in 1D
	 * @param histXAxis X Axis value in 1D
	 * @param histYAxis Y Axis value in 1D
	 * @param histTitle Title for all the graphs in 1D
	 * */
	void SetCloneHists(int nHists,int histNBins,double* histXAxis,double* histYAxis[],string histTitle[]);

	/** Get the 1D hist values to plot for angle plotting
	 * @param nHists Number of graphs in 1D
	 * @param histNBins Total Number of X axis values/channels in 1D
	 * @param histXAxis X Axis value in 1D
	 * @param histYAxis Y Axis value in 1D
	 * @param histTitle Title for all the graphs in 1D
	 * */
	void SetCloneHists(int nHists,int histNBins,double* histXAxis,double* histYAxis,string histTitle[]);

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


	 QMenuBar 	*menubar;
	// QMenu 		*menuFile;
	 QAction 	*actionSave;

	QGridLayout *mainLayout;
	QGroupBox 	*cloneBox;
	QGridLayout *gridClone;
    QGroupBox 	*boxSave;
    QHBoxLayout *layoutSave;
    QLabel 		*lblFName;
    QHBoxLayout *hLayoutSave;
    QLineEdit	*dispFName;
    QComboBox 	*comboFormat;
    QPushButton *btnSave;
    QCheckBox 	*chkAutoFName;
    QCheckBox 	*chkSaveAll;

    /** Gets the current time stamp for the window title*/
    char* GetCurrentTimeStamp();


private slots:
/** Save Plot */
void SavePlot();

protected:
	void closeEvent(QCloseEvent* event);

signals:
void CloneClosedSignal(int);

};




#endif /* QCLONEWIDGET_H_ */
