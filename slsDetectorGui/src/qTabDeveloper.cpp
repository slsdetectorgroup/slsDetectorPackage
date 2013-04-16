/*
 * qTabDeveloper.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
#include "qTabDeveloper.h"
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


qTabDeveloper::qTabDeveloper(QWidget *parent,multiSlsDetector*& detector):
						QWidget(parent),
						myDet(detector),
						boxDacs(0),
						boxAdcs(0),
						lblHV(0),
						comboHV(0),
						adcTimer(0),
						dacLayout(0){
	for(int i=0;i<20;i++){
		lblDacs[i]=0;
		lblAdcs[i]=0;
		spinDacs[i]=0;
		spinAdcs[i]=0;
	}
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

	//palette
	red = QPalette();
	red.setColor(QPalette::Active,QPalette::WindowText,Qt::red);


	//the number of dacs and adcs
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
		dacNames.push_back("v Reference:");
		dacNames.push_back("v Cascode n:");
		dacNames.push_back("v Cascode p:");
		dacNames.push_back("v Comp. Output:");
		dacNames.push_back("v Cascode out");
		dacNames.push_back("v Comp. Input:");
		dacNames.push_back("v Comp. Ref:");
		dacNames.push_back("i Base Test:");

		adcNames.push_back("Temperature ADC:");
		adcNames.push_back("Temperature FPGA:");

		break;
	case slsDetectorDefs::MOENCH:
		NUM_DAC_WIDGETS = 8;
		NUM_ADC_WIDGETS = 2;
		dacNames.push_back("v Reference:");
		dacNames.push_back("v Cascode n:");
		dacNames.push_back("v Cascode p:");
		dacNames.push_back("v Comp. Output:");
		dacNames.push_back("v Cascode out");
		dacNames.push_back("v Comp. Input:");
		dacNames.push_back("v Comp. Ref:");
		dacNames.push_back("i Base Test:");

		adcNames.push_back("Temperature ADC:");
		adcNames.push_back("Temperature FPGA:");

		break;
	default:
		qDefs::Message(qDefs::CRITICAL,string("Unknown detector type:")+myDet->slsDetectorBase::getDetectorType(detType),"Developer");
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
	CreateDACWidgets();

	//HV for gotthard
	if ((detType==slsDetectorDefs::GOTTHARD) || (detType==slsDetectorDefs::MOENCH)){
		boxDacs->setFixedHeight(boxDacs->height()+35);

		lblHV	= new QLabel("High Voltage",boxDacs);
		comboHV	= new QComboBox(boxDacs);
		comboHV->addItem("0");
		comboHV->addItem("90");
		comboHV->addItem("110");
		comboHV->addItem("120");
		comboHV->addItem("150");
		comboHV->addItem("180");
		comboHV->addItem("200");
		tipHV = "<nobr>Set high voltage to 0, 90, 110, 120, 150 or 200V.</nobr>";
		lblHV->setToolTip(tipHV);
		comboHV->setToolTip(tipHV);
		dacLayout->addWidget(lblHV,(int)(NUM_DAC_WIDGETS/2),1);
		dacLayout->addWidget(comboHV,(int)(NUM_DAC_WIDGETS/2),2);
		connect(comboHV,	SIGNAL(currentIndexChanged(int)),	this, SLOT(SetHighVoltage()));
	}
	layout->addWidget(boxDacs,0,0);


	//adcs
	if((detType==slsDetectorDefs::GOTTHARD) || (detType==slsDetectorDefs::MOENCH)){
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

	qDefs::checkErrorMessage(myDet);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::Initialization(){
	if(NUM_ADC_WIDGETS) connect(adcTimer, 	SIGNAL(timeout()), 		this, SLOT(RefreshAdcs()));

	for(int i=0;i<NUM_DAC_WIDGETS;i++)
		connect(spinDacs[i],	SIGNAL(editingFinished(int)),	this, SLOT(SetDacValues(int)));

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::CreateDACWidgets(){
	dacLayout = new QGridLayout(boxDacs);

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
		if((detType==slsDetectorDefs::GOTTHARD) || (detType==slsDetectorDefs::MOENCH))	spinAdcs[i]->setSuffix(0x00b0+QString("C"));

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
#ifdef VERBOSE
	cout << "Setting dac:" << dacNames[id] << " : " << spinDacs[id]->value() << endl;
#endif
	spinDacs[id]->setValue((double)myDet->setDAC((dacs_t)spinDacs[id]->value(),getSLSIndex(id)));

	qDefs::checkErrorMessage(myDet);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::SetHighVoltage(){
#ifdef VERBOSE
	cout << "Setting high voltage:" << comboHV->currentText().toAscii().constData() << endl;
#endif
	int highvoltage = comboHV->currentText().toInt();
	int ret = myDet->setDAC(highvoltage,slsDetectorDefs::HV_POT);
	qDefs::checkErrorMessage(myDet);
	//error
	if(ret != highvoltage){
		qDefs::Message(qDefs::CRITICAL,"High Voltage could not be set to this value.","Developer");
		lblHV->setPalette(red);
		lblHV->setText("High Voltage:*");
		QString errTip = tipHV+QString("<br><br><font color=\"red\"><nobr>High Voltage could not be set. The return value is ")+
				QString::number(ret)+ QString("</nobr></font>");
		lblHV->setToolTip(errTip);
		comboHV->setToolTip(errTip);
	}else{
		lblHV->setPalette(lblDacs[0]->palette());
		lblHV->setText("High Voltage:");
		lblHV->setToolTip(tipHV);
		comboHV->setToolTip(tipHV);
	}
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
			qDefs::Message(qDefs::CRITICAL,"Unknown DAC/ADC Index. Weird Error","Developer");
			Refresh();
			break;
		}
		break;
	case slsDetectorDefs::EIGER:
		return slsDetectorDefs::HUMIDITY;
		/**fill in here*/
		break;
	case slsDetectorDefs::MOENCH:
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
		case 9:return slsDetectorDefs::TEMPERATURE_FPGA;
		default:
			qDefs::Message(qDefs::CRITICAL,"Unknown DAC/ADC Index. Weird Error","Developer");
			Refresh();
			break;
		}
		break;
	default:
		qDefs::Message(qDefs::CRITICAL,string("Unknown detector type:")+myDet->slsDetectorBase::getDetectorType(detType),"Developer");
		qDefs::checkErrorMessage(myDet);
		exit(-1);
		break;
	}
	return slsDetectorDefs::HUMIDITY;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::RefreshAdcs(){
#ifdef VERYVERBOSE
	cout << "Updating ADCs" <<endl;
#endif
	adcTimer->stop();
	for(int i=0;i<NUM_ADC_WIDGETS;i++)
		spinAdcs[i]->setValue((double)myDet->getADC(getSLSIndex(i+NUM_DAC_WIDGETS)));
	adcTimer->start(ADC_TIMEOUT);
	qDefs::checkErrorMessage(myDet);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::Refresh(){
#ifdef VERBOSE
	cout  << endl << "**Updating Developer Tab" << endl;
#endif

#ifdef VERBOSE
	cout << "Gettings DACs"  << endl;
#endif
	//dacs
	for(int i=0;i<NUM_DAC_WIDGETS;i++)
		spinDacs[i]->setValue((double)myDet->setDAC(-1,getSLSIndex(i)));
	//adcs
	if(NUM_ADC_WIDGETS) RefreshAdcs();

	//gotthard -high voltage
	if((detType == slsDetectorDefs::GOTTHARD) || (detType == slsDetectorDefs::MOENCH)){
		disconnect(comboHV,	SIGNAL(currentIndexChanged(int)),	this, SLOT(SetHighVoltage()));

		//default should be correct
		lblHV->setPalette(lblDacs[0]->palette());
		lblHV->setText("High Voltage:");
		lblHV->setToolTip(tipHV);
		comboHV->setToolTip(tipHV);
		//getting hv value
		int ret = (int)myDet->setDAC(-1,slsDetectorDefs::HV_POT);
		switch(ret){
		case 0: 	comboHV->setCurrentIndex(0);break;
		case 90:	comboHV->setCurrentIndex(1);break;
		case 110:	comboHV->setCurrentIndex(2);break;
		case 120:	comboHV->setCurrentIndex(3);break;
		case 150:	comboHV->setCurrentIndex(4);break;
		case 180:	comboHV->setCurrentIndex(5);break;
		case 200:	comboHV->setCurrentIndex(6);break;
		default:	comboHV->setCurrentIndex(0);//error
					lblHV->setPalette(red);
					lblHV->setText("High Voltage:*");
					QString errTip = tipHV+QString("<br><br><font color=\"red\"><nobr>High Voltage could not be set. The return value is ")+
							QString::number(ret)+ QString("</nobr></font>");
					lblHV->setToolTip(errTip);
					comboHV->setToolTip(errTip);
					break;
		}

		connect(comboHV,	SIGNAL(currentIndexChanged(int)),	this, SLOT(SetHighVoltage()));
	}

#ifdef VERBOSE
		cout  << "**Updated Developer Tab" << endl << endl;
#endif

	qDefs::checkErrorMessage(myDet);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


