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
#include <QImage>
#include <QPainter>
/** C++ Include Headers */
#include <iostream>
using namespace std;





qCloneWidget::qCloneWidget(QWidget *parent,int id,QString title,int numDim,SlsQt1DPlot*& plot1D,SlsQt2DPlotLayout*& plot2D,string FilePath):QFrame(parent,Qt::Popup|Qt::SubWindow),id(id),filePath(FilePath){
	/** Window title*/
	char winTitle[100];
	sprintf(winTitle,"SLS Detector GUI Clone %d",id);
	setWindowTitle(QString(winTitle));

	/** Set up widget*/
	SetupWidgetWindow(title,numDim,plot1D,plot2D);

}


qCloneWidget::~qCloneWidget(){
	delete cloneplot1D;
	delete cloneplot2D;
	delete cloneBox;
	delete boxSave;
}



void qCloneWidget::SetupWidgetWindow(QString title,int numDim,SlsQt1DPlot*& plot1D,SlsQt2DPlotLayout*& plot2D){

	/** Main Window Layout */
	mainLayout = new QGridLayout(this);
	setLayout(mainLayout);

	/** plot group box*/
	cloneBox = new QGroupBox(this);
	gridClone = new QGridLayout(cloneBox);
	cloneBox->setLayout(gridClone);
	cloneBox->setContentsMargins(0,0,0,0);
		//cloneBox->resize(400,200);
		cloneBox->setTitle(title);
		cloneBox->setAlignment(Qt::AlignHCenter);
		cloneBox->setFont(QFont("Sans Serif",11,QFont::Bold));
	/** According to dimensions, create appropriate 1D or 2Dplot */
	if(numDim==1){
		cloneplot1D = plot1D;
		gridClone->addWidget(cloneplot1D,0,0);
		cloneBox->setFlat(false);
		cloneBox->setContentsMargins(0,30,0,0);

	}else{
		cloneplot2D = plot2D;
		//cloneplot2D->setContentsMargins(0,0,0,0);
		gridClone->addWidget(cloneplot2D,0,0);
		cloneBox->setFlat(true);
		cloneBox->setContentsMargins(0,5,0,0);
	}

	/** Save group box */
	boxSave = new QGroupBox("Save",this);
	boxSave->setFixedHeight(45);
	boxSave->setContentsMargins(0,8,0,0);
    layoutSave = new QHBoxLayout;
    boxSave->setLayout(layoutSave);
    	/** Label file name*/
        lblFName = new QLabel("File Name:",this);
        layoutSave->addWidget(lblFName);
        /** To get 0 spacing between the next 2 widgets file name and file format */
        hLayoutSave = new QHBoxLayout();
        layoutSave->addLayout(hLayoutSave);
        	hLayoutSave->setSpacing(0);
        	/** file name */
        	dispFName = new QLineEdit(this);
        	hLayoutSave->addWidget(dispFName);
        	/** file format */
        	comboFormat = new QComboBox(this);
        	comboFormat->setFrame(true);
        	comboFormat->addItem(".gif");
        	comboFormat->addItem(".pdf");
	    	comboFormat->addItem(".png");
	    	comboFormat->addItem(".gif+");
	    	comboFormat->addItem(".jpg");
	    	comboFormat->addItem(".ps");
	    	comboFormat->addItem(".eps");
	    	comboFormat->addItem(".xpm");
	    	comboFormat->addItem(".C");
	    	hLayoutSave->addWidget(comboFormat);
	    /** save button */
	    btnSave = new QPushButton("Save",this);
	    btnSave->setFocusPolicy(Qt::NoFocus);
	    layoutSave->addWidget(btnSave);
	    /** automatic file name check box */
	    chkAutoFName = new QCheckBox("Automatic File Name",this);
	    layoutSave->addWidget(chkAutoFName);
	    /** automatic save all check box */
	    chkSaveAll = new QCheckBox("Save All",this);
	    layoutSave->addWidget(chkSaveAll);

	/** main window widgets */
	mainLayout->addWidget(boxSave,0,0);
	mainLayout->addWidget(cloneBox,1,0);

	/** Save */
		connect(btnSave, SIGNAL(clicked()),	this, SLOT(SavePlot()));


}




void qCloneWidget::SetCloneHists(int nHists,int histNBins,double* histXAxis,double* histYAxis[],string histTitle[]){
/** for each plot*/
for(int hist_num=0;hist_num<nHists;hist_num++){
	/** create hists */
	SlsQtH1D*  k;
	if(hist_num+1>cloneplot1D_hists.size()){
		cloneplot1D_hists.append(k=new SlsQtH1D("1d plot",histNBins,histXAxis,histYAxis[hist_num]));
		k->SetLineColor(hist_num+1);
	}else{
		k=cloneplot1D_hists.at(hist_num);
		k->SetData(histNBins,histXAxis,histYAxis[hist_num]);
	}
	k->setTitle(histTitle[hist_num].c_str());
	k->Attach(cloneplot1D);
}
cloneplot1D->UnZoom();


}




void qCloneWidget::SavePlot(){
	QString fName = QString(filePath.c_str())+'/'+dispFName->text()+comboFormat->currentText();
	QImage img(cloneBox->size().width(),cloneBox->size().height(),QImage::Format_RGB32);
	QPainter painter(&img);
	cloneBox->render(&painter);
	img.save(fName);
}




void qCloneWidget::closeEvent(QCloseEvent* event){
	emit CloneClosedSignal(id);
	event->accept();
}
