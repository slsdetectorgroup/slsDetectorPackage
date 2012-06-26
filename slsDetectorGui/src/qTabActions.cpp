/*
 * qTabActions.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
/** Qt Project Class Headers */
#include "qTabActions.h"
#include "qDefs.h"
#include "qActionsWidget.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** Qt Include Headers */
#include <QButtonGroup>
/** C++ Include Headers */
#include<iostream>
using namespace std;




qTabActions::qTabActions(QWidget *parent,slsDetectorUtils*& detector):QWidget(parent),myDet(detector){
	SetupWidgetWindow();
	Initialization();
}




qTabActions::~qTabActions(){
	delete myDet;
}




void qTabActions::SetupWidgetWindow(){
	/** Window Settings*/
	setFixedSize(705,350);
	setContentsMargins(0,0,0,0);

	/** Scroll Area Settings*/
	QScrollArea *scroll = new QScrollArea;
	scroll->setWidget(this);
	scroll->setWidgetResizable(true);

	/** Layout Settings*/
	gridLayout = new QGridLayout(scroll);
	setLayout(gridLayout);
	gridLayout->setContentsMargins(10,5,0,0);
	gridLayout->setVerticalSpacing(2);

	/** Buttongroup to know which +/- button was clicked*/
	group 			= new QButtonGroup(this);
	palette 		= new QPalette();

	/** For each level of Actions */
	for(int i=0;i<NUM_ACTION_WIDGETS;i++){
		/** Add the extra widgets only for the 1st 2 levels*/
		if((i==1)||(i==2))
			actionWidget[i] = new ActionsWidget(this,1);
		else
			actionWidget[i] = new ActionsWidget(this,0);

		btnExpand[i] 	= new QPushButton("+");
			btnExpand[i]->setFixedSize(20,20);
		lblName[i]	 	= new QLabel("");
		group->addButton(btnExpand[i],i);
		gridLayout->addWidget(btnExpand[i],(i*2),0);
		gridLayout->addWidget(lblName[i],(i*2),1);
		gridLayout->addWidget(actionWidget[i],(i*2)+1,1,1,2);

	}

	/** Label Values */
	lblName[0]->setText("Action at Start");
	lblName[1]->setText("Scan Level 0");
	lblName[2]->setText("Scan Level 1");
	lblName[3]->setText("Action before each Frame");
	lblName[4]->setText("Number of Positions");
	lblName[5]->setText("Header before Frame");
	lblName[6]->setText("Header after Frame");
	lblName[7]->setText("Action after each Frame");
	lblName[8]->setText("Action at Stop");

	/** initially hide all the widgets*/
	for(int i=0;i<NUM_ACTION_WIDGETS;i++)	actionWidget[i]->hide();

}



void qTabActions::Initialization(){
	connect(group,SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(Expand(QAbstractButton*)));
}



void qTabActions::Expand(QAbstractButton *button ){
	int index = group->id(button);
	/** Collapse */
	if(!QString::compare(button->text(), "-")){
		palette->setColor(QPalette::WindowText,Qt::black);
		lblName[index]->setPalette(*palette);
		actionWidget[index]->hide();
		button->setText("+");
		if((index==1)||(index==2))
			setFixedHeight(height()-130);
		else
			setFixedHeight(height()-30);
	}else{
		/** Expand */
		palette->setColor(QPalette::WindowText,QColor(0,0,200,255));
		lblName[index]->setPalette(*palette);
		actionWidget[index]->show();
		button->setText("-");
		if((index==1)||(index==2))
			setFixedHeight(height()+130);
		else
			setFixedHeight(height()+30);
	}
}


























/*
void qTabActions::Initialization(){
	connect(radio0Constant,	SIGNAL(toggled(bool)),this,SLOT(ChangeStepSize(bool)));
	connect(radio0Specific,	SIGNAL(toggled(bool)),this,SLOT(ChangeStepSize(bool)));
	connect(radio0Value,	SIGNAL(toggled(bool)),this,SLOT(ChangeStepSize(bool)));
	connect(radio1Constant,	SIGNAL(toggled(bool)),this,SLOT(ChangeStepSize(bool)));
	connect(radio1Specific,	SIGNAL(toggled(bool)),this,SLOT(ChangeStepSize(bool)));
	connect(radio1Value,	SIGNAL(toggled(bool)),this,SLOT(ChangeStepSize(bool)));

	connect(btntry,	SIGNAL(clicked()),this,SLOT(Trial()));

}



void qTabActions::Enable(bool enable){


}


void qTabActions::ChangeStepSize(bool b){
	* defaults
	lbl0From->hide();
	lbl0Size->hide();
	lbl0To->hide();
	spin0From->hide();
	spin0Size->hide();
	spin0To->hide();
	combo0Specific->hide();
	btn0Browse->hide();
	disp0File->hide();
	lbl1From->hide();
	lbl1Size->hide();
	lbl1To->hide();
	spin1From->hide();
	spin1Size->hide();
	spin1To->hide();
	combo1Specific->hide();
	btn1Browse->hide();
	disp1File->hide();
	*Scan 0
	* constant step size
	if(radio0Constant->isChecked()){
		lbl0From->show();
		lbl0Size->show();
		lbl0To->show();
		spin0From->show();
		spin0Size->show();
		spin0To->show();
	}
	* specific values
	else if(radio0Specific->isChecked())
		combo0Specific->show();
	* values from a file
	else{
		btn0Browse->show();
		disp0File->show();
	}
	*Scan 1
	* constant step size
	if(radio1Constant->isChecked()){
		lbl1From->show();
		lbl1Size->show();
		lbl1To->show();
		spin1From->show();
		spin1Size->show();
		spin1To->show();
	}
	* specific values
	else if(radio1Specific->isChecked())
		combo1Specific->show();
	* values from a file
	else{
		btn1Browse->show();
		disp1File->show();
	}


	//groupBox->hide();
}


	//if(!QString::compare(btntry->text(),"+")){
*/

