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
#include <QSizePolicy>
using namespace std;





qCloneWidget::qCloneWidget(QWidget *parent,int id,QSize fSize,QString title,int numDim,SlsQt1DPlot*& plot1D,SlsQt2DPlotLayout*& plot2D):QMainWindow(parent),id(id){


/*	mainLayout = new QGridLayout(this);
	setLayout(mainLayout);*/




	QGroupBox *cloneBox = new QGroupBox(this);
	QGridLayout *gridClone = new QGridLayout(cloneBox);
	cloneBox->setLayout(gridClone);
		cloneBox->resize(fSize);
		cloneBox->setTitle(title);
		cloneBox->setAlignment(Qt::AlignHCenter);
		cloneBox->setFont(QFont("Sans Serif",11,QFont::Bold));
	if(numDim==1){
		cloneplot1D = plot1D;
		gridClone->addWidget(cloneplot1D,0,0);
		cloneBox->setFlat(false);

	}else{
		cloneplot2D = plot2D;
		gridClone->addWidget(cloneplot2D,0,0);
		cloneBox->setFlat(true);
	}

	cloneBox->setCentralWidget();

/*
			boxSave = new QGroupBox(this);
	        layoutSave = new QHBoxLayout;
	        boxSave->setLayout(layoutSave);

	        lblFName = new QLabel(layoutSave);
	        lblFName->setText("File Name:");
	        layoutSave->addWidget(lblFName);



	        hLayoutSave = new QHBoxLayout();
	        hLayoutSave->setSpacing(0);

	        dispFName = new QLineEdit(layoutSave);
	        hLayoutSave->addWidget(dispFName);

	        comboFormat = new QComboBox(layoutSave);
	        comboFormat->setFrame(true);
	        comboFormat->insertItem(".gif");
	        comboFormat->insertItem(".pdf");
	        comboFormat->insertItem(".png");
	        comboFormat->insertItem(".gif+");
	        comboFormat->insertItem(".jpg");
	        comboFormat->insertItem(".ps");
	        comboFormat->insertItem(".eps");
	        comboFormat->insertItem(".xpm");
	        comboFormat->insertItem(".C");
	        hLayoutSave->addWidget(comboFormat);

	        layoutSave->addLayout(hLayoutSave);


	        btnSave = new QPushButton(layoutSave);
	        bnSave->setText("Save");
	    	QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
	        sizePolicy2.setHeightForWidth(btnSave->sizePolicy().hasHeightForWidth());
	        btnSave->setSizePolicy(sizePolicy2);
	        btnSave->setFocusPolicy(Qt::NoFocus);
	        layoutSave->addWidget(btnSave);

	        chkAutoFName = new QCheckBox(layoutSave);
	        chkAutoFName->setText("Automatic File Name");
	        layoutSave->addWidget(chkAutoFName);

	        chkSaveAll = new QCheckBox(layoutSave);
	        chkSaveAll->setText("Save All")
	        layoutSave->addWidget(chkSaveAll);



	        gridClone->addWidget(boxSave,0,0);
	        mainLayout->addWidget(boxSave,1,1);*/
/*
	        mainLayout->addWidget(cloneBox,0,0);*/

}


qCloneWidget::~qCloneWidget(){
	delete cloneplot1D;
	delete cloneplot2D;
}


void qCloneWidget::closeEvent(QCloseEvent* event){
	emit CloneClosedSignal(id);
	event->accept();
}
