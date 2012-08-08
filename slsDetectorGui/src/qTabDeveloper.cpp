/*
 * qTabDeveloper.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
#include "qTabDeveloper.h"
#include "qDefs.h"
//Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
//Qt Include Headers
#include <QSpacerItem>
#include <QString>
#include <QDoubleValidator>
//C++ Include Headers
#include<iostream>
using namespace std;

//-------------------------------------------------------------------------------------------------------------------------------------------------

int qTabDeveloper::NUM_DAC_WIDGETS(0);
int qTabDeveloper::NUM_ADC_WIDGETS(0);

//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabDeveloper::qTabDeveloper(QWidget *parent,multiSlsDetector*& detector):QWidget(parent),myDet(detector){
	SetupWidgetWindow();
	Initialization();

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabDeveloper::~qTabDeveloper(){
	delete myDet;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::SetupWidgetWindow(){
	//Detector Type
	detType=myDet->getDetectorsType();

	//the nu
	switch(detType){
	case slsDetectorDefs::MYTHEN:
		NUM_DAC_WIDGETS = 6;
		NUM_ADC_WIDGETS = 0;
		dacNames.push_back("v Trimbit:");
		dacNames.push_back("v Threshold:");
		dacNames.push_back("v Shaper1:");
		dacNames.push_back("v Shaper2:");
		dacNames.push_back("v Calibration:");
		dacNames.push_back("v Preamp:");
		break;
	case slsDetectorDefs::EIGER:
		NUM_DAC_WIDGETS = 16;
		NUM_ADC_WIDGETS = 0;
		break;
	case slsDetectorDefs::GOTTHARD:
		NUM_DAC_WIDGETS = 8;
		NUM_ADC_WIDGETS = 2;
		dacNames.push_back("Reference Voltage:");
		dacNames.push_back("Cascade n Voltage:");
		dacNames.push_back("Cascade p Voltage:");
		dacNames.push_back("Comp. Output Voltage:");
		dacNames.push_back("Cascade out Voltage:");
		dacNames.push_back("Comp. Input Voltage:");
		dacNames.push_back("Comp. Ref Voltage:");
		dacNames.push_back("Base Test Current:");

		adcNames.push_back("Temperature ADC:");
		adcNames.push_back("Temperature FPGA:");

		break;
	default:
		qDefs::ErrorMessage(string("Unknown detector type:")+myDet->slsDetectorBase::getDetectorType(detType),"Developer");
		exit(-1);
		break;
	}


	//layout
	setFixedWidth(765);
	setFixedHeight(50+(NUM_DAC_WIDGETS/2)*35);
	//setHeight(340);
	scroll  = new QScrollArea;
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setWidget(this);
	scroll->setWidgetResizable(true);

	layout = new QGridLayout(scroll);
	layout->setContentsMargins(20,10,10,5);
	setLayout(layout);

	//dacs
	boxDacs = new QGroupBox("Dacs",this);
	boxDacs->setFixedHeight(25+(NUM_DAC_WIDGETS/2)*35);
	layout->addWidget(boxDacs,0,0);
	CreateDACWidgets();
	//adcs
	if((detType==slsDetectorDefs::GOTTHARD)||(detType==slsDetectorDefs::AGIPD)){
	    setFixedHeight((50+(NUM_DAC_WIDGETS/2)*35)+(50+(NUM_ADC_WIDGETS/2)*35));
		boxAdcs = new QGroupBox("ADCs",this);
		boxAdcs->setFixedHeight(25+(NUM_ADC_WIDGETS/2)*35);
		layout->addWidget(boxAdcs,1,0);
		CreateADCWidgets();
		//to make the adcs at the bottom most
		int diff = 340-height();
		setFixedHeight(340);
		layout->setVerticalSpacing(diff);
		//timer to check adcs
		adcTimer = new QTimer(this);
	}


}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::Initialization(){
	connect(adcTimer, 	SIGNAL(timeout()), 		this, SLOT(RefreshAdcs()));

	for(int i=0;i<NUM_DAC_WIDGETS;i++)
		connect(spinDacs[i],	SIGNAL(editingFinished(int)),	this, SLOT(SetDacValues(int)));

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::CreateDACWidgets(){
	QGridLayout *dacLayout = new QGridLayout(boxDacs);

	for(int i=0;i<NUM_DAC_WIDGETS;i++){
		lblDacs[i] 	= new QLabel(QString(dacNames[i].c_str()),boxDacs);
		spinDacs[i]	= new MyDoubleSpinBox(i,boxDacs);
		spinDacs[i]->setMaximum(10000);

		dacLayout->addWidget(lblDacs[i],(int)(i/2),((i%2)==0)?1:4);
		dacLayout->addWidget(spinDacs[i],(int)(i/2),((i%2)==0)?2:5);
		if(!(i%2)){
			dacLayout->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed),(int)(i/2),0);
			dacLayout->addItem(new QSpacerItem(60,20,QSizePolicy::Fixed,QSizePolicy::Fixed),(int)(i/2),3);
			dacLayout->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed),(int)(i/2),6);
		}
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::CreateADCWidgets(){
	QGridLayout *adcLayout = new QGridLayout(boxAdcs);

	for(int i=0;i<NUM_ADC_WIDGETS;i++){
		lblAdcs[i] 	= new QLabel(QString(adcNames[i].c_str()),boxAdcs);
		spinAdcs[i]	= new QDoubleSpinBox(boxAdcs);
		spinAdcs[i]->setMaximum(10000);

		adcLayout->addWidget(lblAdcs[i],(int)(i/2),((i%2)==0)?1:4);
		adcLayout->addWidget(spinAdcs[i],(int)(i/2),((i%2)==0)?2:5);
		if(!(i%2)){
			adcLayout->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed),(int)(i/2),0);
			adcLayout->addItem(new QSpacerItem(60,20,QSizePolicy::Fixed,QSizePolicy::Fixed),(int)(i/2),3);
			adcLayout->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed),(int)(i/2),6);
		}
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::SetDacValues(int id){
#ifdef VERYVERBOSE
	cout << "Setting dac:" <<dacNames[id] << " : " << spinDacs[id]->value() << endl;
#endif

	spinDacs[id]->setValue(myDet->setDAC(spinDacs[id]->value(),getSLSIndex(id)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


slsDetectorDefs::dacIndex qTabDeveloper::getSLSIndex(int index){
	switch(detType){
	case slsDetectorDefs::MYTHEN:
		switch(index){
		case 0:	return slsDetectorDefs::TRIMBIT_SIZE;
		case 1:	return slsDetectorDefs::THRESHOLD;
		case 2:	return slsDetectorDefs::SHAPER1;
		case 3:	return slsDetectorDefs::SHAPER2;
		case 4:	return slsDetectorDefs::CALIBRATION_PULSE;
		case 5:	return slsDetectorDefs::PREAMP;
		default:
			qDefs::ErrorMessage("Unknown DAC/ADC Index. Weird Error","Developer");
			Refresh();
			break;
		}
	case slsDetectorDefs::EIGER:
		return slsDetectorDefs::HUMIDITY;
		/**fill in here*/
		break;
	case slsDetectorDefs::GOTTHARD:
		switch(index){
		case 0:	return slsDetectorDefs::G_VREF_DS;
		case 1:	return slsDetectorDefs::G_VCASCN_PB;
		case 2:	return slsDetectorDefs::G_VCASCP_PB;
		case 3:	return slsDetectorDefs::G_VOUT_CM;
		case 4:	return slsDetectorDefs::G_VCASC_OUT;
		case 5:	return slsDetectorDefs::G_VIN_CM;
		case 6:	return slsDetectorDefs::G_VREF_COMP;
		case 7:	return slsDetectorDefs::G_IB_TESTC;
		case 8:	return slsDetectorDefs::TEMPERATURE_ADC;
		case 9:	return slsDetectorDefs::TEMPERATURE_FPGA;
		default:
			qDefs::ErrorMessage("Unknown DAC/ADC Index. Weird Error","Developer");
			Refresh();
			break;
		}
	default:
		qDefs::ErrorMessage(string("Unknown detector type:")+myDet->slsDetectorBase::getDetectorType(detType),"Developer");
		exit(-1);
		break;
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::RefreshAdcs(){
#ifdef VERYVERBOSE
	cout << "Updating ADCs" <<endl;
#endif
	adcTimer->stop();
	for(int i=0;i<NUM_ADC_WIDGETS;i++)
		spinAdcs[i]->setValue(myDet->getADC(getSLSIndex(i+NUM_DAC_WIDGETS)));
	adcTimer->start(ADC_TIMEOUT);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::Refresh(){
#ifdef VERBOSE
	cout << "Updating Dacs and Adcs" <<endl;
#endif
	//dacs
	for(int i=0;i<NUM_DAC_WIDGETS;i++)
		spinDacs[i]->setValue(myDet->setDAC(-1,getSLSIndex(i)));

	//adcs
	RefreshAdcs();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


