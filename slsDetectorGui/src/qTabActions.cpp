/*
 * qTabActions.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
// Qt Project Class Headers
#include "qTabActions.h"
#include "qDefs.h"
#include "qActionsWidget.h"
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
// Qt Include Headers
#include <QButtonGroup>
// C++ Include Headers
#include<iostream>
using namespace std;


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabActions::qTabActions(QWidget *parent,multiSlsDetector*& detector):
		QWidget(parent),myDet(detector){
	SetupWidgetWindow();
	Initialization();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabActions::~qTabActions(){
	delete myDet;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabActions::SetupWidgetWindow(){
	// Window Settings
	setFixedSize(710,350);
	setContentsMargins(0,0,0,0);

	// Scroll Area Settings
	QScrollArea *scroll = new QScrollArea;
	scroll->setWidget(this);
	scroll->setWidgetResizable(true);

	// Layout Settings
	gridLayout = new QGridLayout(scroll);
	setLayout(gridLayout);
	gridLayout->setContentsMargins(10,5,0,0);
	gridLayout->setVerticalSpacing(2);

	// Buttongroup to know which +/- button was clicked
	group 			= new QButtonGroup(this);
	palette 		= new QPalette();


	// For each level of Actions
	for(int i=0;i<NUM_ACTION_WIDGETS;i++){
		// Add the extra widgets only for the 1st 2 levels
		if((i==ActionsWidget::Scan0)||(i==ActionsWidget::Scan1))
			actionWidget[i] = new ActionsWidget(this,myDet,1,i);
		else
			actionWidget[i] = new ActionsWidget(this,myDet,0,i);

		btnExpand[i] 	= new QPushButton("+");
			btnExpand[i]->setFixedSize(20,20);
		lblName[i]	 	= new QLabel("");
		group->addButton(btnExpand[i],i);
		gridLayout->addWidget(btnExpand[i],(i*2),0);
		gridLayout->addWidget(lblName[i],(i*2),1);
		gridLayout->addWidget(actionWidget[i],(i*2)+1,1,1,2);
/*		gridLayout->addWidget(btnExpand[i],(i*2),i);
		gridLayout->addWidget(lblName[i],(i*2),i+1);
		gridLayout->addWidget(actionWidget[i],(i*2)+1,i+1,1,2);*/

	}

	// Label Values
	lblName[ActionsWidget::Start]->setText("Action at Start");
	lblName[ActionsWidget::Scan0]->setText("Scan Level 0");
	lblName[ActionsWidget::Scan1]->setText("Scan Level 1");
	lblName[ActionsWidget::ActionBefore]->setText("Action before each Frame");
	lblName[ActionsWidget::NumPositions]->setText("Positions");
	lblName[ActionsWidget::HeaderBefore]->setText("Header before Frame");
	lblName[ActionsWidget::HeaderAfter]->setText("Header after Frame");
	lblName[ActionsWidget::ActionAfter]->setText("Action after each Frame");
	lblName[ActionsWidget::Stop]->setText("Action at Stop");

	// initially hide all the widgets
	for(int i=0;i<NUM_ACTION_WIDGETS;i++)
		actionWidget[i]->hide();

	//Number of positions is only for mythen or gotthard
	slsDetectorDefs::detectorType detType = myDet->getDetectorsType();
	if((detType == slsDetectorDefs::EIGER) || (detType == slsDetectorDefs::AGIPD)) {
		lblName[ActionsWidget::NumPositions]->setEnabled(false);
		btnExpand[ActionsWidget::NumPositions]->setEnabled(false);
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabActions::Initialization(){
	connect(group,				SIGNAL(buttonClicked(QAbstractButton*)),	this,SLOT(Expand(QAbstractButton*)));
	connect(actionWidget[ActionsWidget::Scan0],SIGNAL(EnableScanBox(bool,int)),			this,SIGNAL(EnableScanBox(bool,int)));
	connect(actionWidget[ActionsWidget::Scan1],SIGNAL(EnableScanBox(bool,int)),			this,SIGNAL(EnableScanBox(bool,int)));

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabActions::Expand(QAbstractButton *button ){
	int index = group->id(button);
	// Collapse
	if(!QString::compare(button->text(), "-")){
		palette->setColor(QPalette::WindowText,Qt::black);
		lblName[index]->setPalette(*palette);
		actionWidget[index]->hide();
		button->setText("+");
		if((index==ActionsWidget::Scan0)||(index==ActionsWidget::Scan1)){
			setFixedHeight(height()-130);
		}
		else if(index==ActionsWidget::NumPositions)
			setFixedHeight(height()-80);
		else
			setFixedHeight(height()-30);
	}else{
		// Expand
		palette->setColor(QPalette::WindowText,QColor(0,0,200,255));
		lblName[index]->setPalette(*palette);
		actionWidget[index]->show();
		button->setText("-");
		if((index==ActionsWidget::Scan0)||(index==ActionsWidget::Scan1)){
			setFixedHeight(height()+130);
		}
		else if(index==ActionsWidget::NumPositions)
			setFixedHeight(height()+80);
		else
			setFixedHeight(height()+30);
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabActions::Refresh(){
#ifdef VERBOSE
	cout << "Updating action widgets " << endl;
#endif
	for(int i=0;i<NUM_ACTION_WIDGETS;i++)
		actionWidget[i]->Refresh();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

