/*
 * qCloneWidget.h
 *
 *  Created on: May 18, 2012
 *      Author: l_maliakal_d
 */

#ifndef QCLONEWIDGET_H_
#define QCLONEWIDGET_H_

/** Qt Project Class Headers */
class SlsQt1DPlot;
class SlsQt2DPlotLayout;
/** Qt Include Headers */
#include <QFrame>
#include <QCloseEvent>
#include <QGroupBox>
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>

/**
 *@short Sets up the clone plot widget
 */
class qCloneWidget:public QFrame{
	Q_OBJECT

public:
	/** \short The constructor
	 */
	qCloneWidget(QWidget *parent,int id,QSize fSize,QString title,int numDim,SlsQt1DPlot*& plot1D,SlsQt2DPlotLayout*& plot2D);

	/** Destructor
	 */
	~qCloneWidget();


	/** Get the 1D plot reference
	 */
	SlsQt1DPlot* Get1Dplot(){ return cloneplot1D;};

public slots:



private:
	/**	clone window id*/
	int id;
	/**	clone 1D Plot */
	SlsQt1DPlot* 		cloneplot1D;
	/**	clone 2D Plot */
	SlsQt2DPlotLayout* 	cloneplot2D;

	QGridLayout *mainLayout;
    QGroupBox *boxSave;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *layoutSave;
    QLabel *lblFName;
    QHBoxLayout *hLayoutSave;
    QLineEdit *dispFName;
    QComboBox *comboFormat;
    QPushButton *btnSave;
    QCheckBox *chkAutoFName;
    QCheckBox *chkSaveAll;
protected:
	void closeEvent(QCloseEvent* event);

signals:
void CloneClosedSignal(int);

};




#endif /* QCLONEWIDGET_H_ */
