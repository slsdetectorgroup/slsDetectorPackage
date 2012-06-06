/*
 * qTabActions.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

/** Qt Project Class Headers */
#include "qActionsWidget.h"
/** Qt Include Headers */
#include <QGridLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QLayout>
#include <QSpacerItem>
#include <QSpinBox>
#include <QGroupBox>
#include <QRadioButton>
/** C++ Include Headers */
#include<iostream>
using namespace std;


#define Detector_Index 0


ActionsWidget::ActionsWidget(QWidget *parent, int scanType): QFrame(parent){
	SetupWidgetWindow(scanType);
	Initialization();
}





ActionsWidget::~ActionsWidget(){
	delete layout;
}




void ActionsWidget::SetupWidgetWindow(int scanType){
	/** Widget Settings */
	//setFrameStyle(QFrame::Box);
	//setFrameShadow(QFrame::Raised);
	setFixedHeight(25);
	if(scanType)	setFixedHeight(125);


	/** Main Layout Settings */
	layout = new QGridLayout(this);
	setLayout(layout);
	layout->setContentsMargins(0,0,0,0);
	if(scanType)	layout->setVerticalSpacing(5);


	/** Main Widgets*/
	comboScript = new QComboBox(this);
	if(!scanType){
		comboScript->addItem("None");
		comboScript->addItem("Custom Script");
	}else{
		comboScript->addItem("None");
		comboScript->addItem("Energy Scan");
		comboScript->addItem("Threshold Scan");
		comboScript->addItem("Trimbits Scan");
		comboScript->addItem("Custom Script");
	}
		layout->addWidget(comboScript,0,0);
	layout->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed),0,1);
	dispScript = new QLineEdit("None");
		dispScript->setEnabled(false);
		layout->addWidget(dispScript,0,2);
	btnBrowse = new QPushButton("Browse");
		btnBrowse->setEnabled(false);
		layout->addWidget(btnBrowse,0,3);
	layout->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed),0,4);
	lblParameter = new QLabel("Additional Parameter:");
		lblParameter->setEnabled(false);
		layout->addWidget(lblParameter,0,5);
	dispParameter = new QLineEdit("None");
		dispParameter->setEnabled(false);
		dispParameter->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
		layout->addWidget(dispParameter,0,6);



	/**	Scan Levels Widgets*/
	if(scanType){
		lblSteps = new QLabel("Number of Steps:");
			lblSteps->setEnabled(false);
			layout->addWidget(lblSteps,1,2);
		spinSteps = new QSpinBox(this);
			spinSteps->setEnabled(false);
			layout->addWidget(spinSteps,1,3);
		lblPrecision = new QLabel("Precision:");
			lblPrecision->setEnabled(false);
			layout->addWidget(lblPrecision,1,5);
		spinPrecision = new QSpinBox(this);
			spinPrecision->setEnabled(false);
			layout->addWidget(spinPrecision,1,6);
		group = new QGroupBox(this);
			group->setEnabled(false);
			/** Fix the size of the groupbox*/
			group->setFixedSize(513,66);
			layout->addWidget(group,2,2,1,5);


		/** Group Box for step size */
		/**  Radio Buttons Layout */
		QWidget *h1Widget = new QWidget(group);
		h1Widget->setGeometry(QRect(10, 5, group->width()-20, 23));
		QHBoxLayout *h1 = new QHBoxLayout(h1Widget);
		h1->setContentsMargins(0, 0, 0, 0);
		radioConstant = new QRadioButton("Constant Step Size",h1Widget);
			radioConstant->setChecked(true);
			h1->addWidget(radioConstant);
		radioSpecific = new QRadioButton("Specific Values",h1Widget);
			h1->addWidget(radioSpecific);
		radioValue 	= new QRadioButton("Values from File",h1Widget);
			radioValue->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
			h1->addWidget(radioValue);

		/**  Constant Size Layout */
		QWidget *h2ConstantWidget = new QWidget(group);
		h2ConstantWidget->setGeometry(QRect(10, 30, group->width()-20, 31));
		QHBoxLayout *h2Constant = new QHBoxLayout(h2ConstantWidget);
		h2Constant->setContentsMargins(0, 0, 0, 0);

		h2Constant->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
		lblFrom	= new QLabel("from",h2ConstantWidget);
			lblFrom->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
			h2Constant->addWidget(lblFrom);
		spinFrom = new QSpinBox(h2ConstantWidget);
			h2Constant->addWidget(spinFrom);
		lblTo = new QLabel("to",h2ConstantWidget);
			lblTo->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
			h2Constant->addWidget(lblTo);
		spinTo			= new QSpinBox(h2ConstantWidget);
			h2Constant->addWidget(spinTo);
		h2Constant->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
		lblSize			= new QLabel("Size",h2ConstantWidget);
			lblSize->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
			h2Constant->addWidget(lblSize);
		spinSize		= new QSpinBox(h2ConstantWidget);
			h2Constant->addWidget(spinSize);
		h2Constant->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));

		/**  Specific Values Layout */
		QWidget *h2SpecificWidget = new QWidget(group);
		h2SpecificWidget->setGeometry(QRect(10, 30, group->width()-20, 31));
		QHBoxLayout *h2Specific = new QHBoxLayout(h2SpecificWidget);
		h2Specific->setContentsMargins(0, 0, 0, 0);
		h2Specific->addItem(new QSpacerItem(200,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
		comboSpecific = new QComboBox(h2SpecificWidget);
			h2Specific->addWidget(comboSpecific);
			comboSpecific->hide();
		h2Specific->addItem(new QSpacerItem(200,20,QSizePolicy::Fixed,QSizePolicy::Fixed));

		/**  Values From a File Layout */
		QWidget *h2ValuesWidget = new QWidget(group);
		h2ValuesWidget->setGeometry(QRect(10, 30, group->width()-20, 31));
		QHBoxLayout *h2Values = new QHBoxLayout(h2ValuesWidget);
		h2Values->setContentsMargins(0, 0, 0, 0);
		h2Values->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
		dispValues = new QLineEdit("steps.txt",h2ValuesWidget);
			dispValues->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
			h2Values->addWidget(dispValues);
			dispValues->hide();
		btnValues = new QPushButton("Browse",h2ValuesWidget);
			btnValues->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
			h2Values->addWidget(btnValues);
			btnValues->hide();
		h2Values->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));

	}

}



void ActionsWidget::Initialization(){
	connect(comboScript,SIGNAL(currentIndexChanged(int)),this,SLOT(SetScript(int)));
	if(comboScript->count()>2){
		connect(radioConstant,SIGNAL(toggled(bool)),this,SLOT(EnableSizeWidgets()));
		connect(radioSpecific,SIGNAL(toggled(bool)),this,SLOT(EnableSizeWidgets()));
		connect(radioValue,SIGNAL(toggled(bool)),this,SLOT(EnableSizeWidgets()));
	}
}



void ActionsWidget::SetScript(int index){
	/** defaults */
	dispScript->setEnabled(false);
	btnBrowse->setEnabled(false);
	lblParameter->setEnabled(false);
	dispParameter->setEnabled(false);
	if(comboScript->count()>2){
		group->setEnabled(false);
		lblSteps->setEnabled(false);
		spinSteps->setEnabled(false);
		lblPrecision->setEnabled(false);
		spinPrecision->setEnabled(false);
	}
	/** If anything other than None is selected*/
	if(index){
		/** Custom Script only enables the first layout with addnl parameters etc */
		if(!comboScript->currentText().compare("Custom Script")){
			dispScript->setEnabled(true);
			btnBrowse->setEnabled(true);
			lblParameter->setEnabled(true);
			dispParameter->setEnabled(true);
		}
		/** If this group includes Energy scan , threhold scan etc */
		if(comboScript->count()>2){
			group->setEnabled(true);
			lblPrecision->setEnabled(true);
			spinPrecision->setEnabled(true);
			/** Steps are enabled only if constant step size is not checked*/
			lblSteps->setEnabled(!radioConstant->isChecked());
			spinSteps->setEnabled(!radioConstant->isChecked());
		}
	}
}




void ActionsWidget::EnableSizeWidgets(){
	/** defaults */
	lblFrom->hide();
	spinFrom->hide();
	lblTo->hide();
	spinTo->hide();
	lblSize->hide();
	spinSize->hide();
	comboSpecific->hide();
	dispValues->hide();
	btnValues->hide();
	lblSteps->setEnabled(true);
	spinSteps->setEnabled(true);
	/** Constant Step Size */
	if(radioConstant->isChecked()){
		lblFrom->show();
		spinFrom->show();
		lblTo->show();
		spinTo->show();
		lblSize->show();
		spinSize->show();
		lblSteps->setEnabled(false);
		spinSteps->setEnabled(false);
	}/** Specific Values */
	else if(radioSpecific->isChecked())
		comboSpecific->show();
	/** Values from a File */
	else{
		dispValues->show();
		btnValues->show();
	}
}

