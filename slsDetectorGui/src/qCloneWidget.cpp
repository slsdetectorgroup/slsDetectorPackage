/*
 * qCloneWidget.cpp
 *
 *  Created on: May 18, 2012
 *      Author: l_maliakal_d
 */

/** Qt Project Class Headers */
#include "qCloneWidget.h"
#include "SlsQt1DPlot.h"
#include "SlsQt2DPlotLayout.h"
/** Qt Include Headers */
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>

/** C++ Include Headers */
#include <iostream>
#include <string>
using namespace std;





qCloneWidget::qCloneWidget(QWidget *parent,int id,QSize fSize,int numDim,SlsQt1DPlot*& plot1D,SlsQt2DPlotLayout*& plot2D):QFrame(parent,Qt::Popup|Qt::SubWindow),id(id){

	QGroupBox *cloneBox = new QGroupBox(this);
	QGridLayout *gridClone = new QGridLayout(cloneBox);
	cloneBox->setLayout(gridClone);
		cloneBox->setFlat(1);
		cloneBox->setTitle("Startup Image");
		cloneBox->resize(fSize);

	if(numDim==1){
		cloneplot1D = plot1D;
		gridClone->addWidget(cloneplot1D,0,0);
	}else{
		cloneplot2D = plot2D;
		gridClone->addWidget(cloneplot2D,0,0);
	}


}


qCloneWidget::~qCloneWidget(){
	delete cloneplot1D;
	delete cloneplot2D;
}


void qCloneWidget::closeEvent(QCloseEvent* event){
	emit CloneClosedSignal(id);
	event->accept();
}
