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
		lblDacsmV[i]=0;
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

		dacNames.push_back("v SvP:");
		dacNames.push_back("v SvN");
		dacNames.push_back("v Vrf:");
		dacNames.push_back("v Vrs:");
		dacNames.push_back("v Vtr:");
		dacNames.push_back("v Vtgstv:");
		dacNames.push_back("v cal:");
		dacNames.push_back("v Vcp");
		dacNames.push_back("v Vcn:");
		dacNames.push_back("v Vis:");
		dacNames.push_back("v rxb_lb:");
		dacNames.push_back("v rxb_rb:");
		dacNames.push_back("v Vcmp_ll:");
		dacNames.push_back("v Vcmp_lr:");
		dacNames.push_back("v Vcmp_rl:");
		dacNames.push_back("v Vcmp_rr:");



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
		dacNames.push_back("v Dac 0:");
		dacNames.push_back("v Dac 1:");
		dacNames.push_back("v Dac 2:");
		dacNames.push_back("v Dac 3:");
		dacNames.push_back("v Dac 4:");
		dacNames.push_back("v Dac 5:");
		dacNames.push_back("v Dac 6:");
		dacNames.push_back("i Dac 7:");

		adcNames.push_back("Temperature ADC:");
		adcNames.push_back("Temperature FPGA:");

		break;
	default:
		cout << "ERROR: Unknown detector type: " + myDet->slsDetectorBase::getDetectorType(detType) << endl;
		qDefs::Message(qDefs::CRITICAL,string("Unknown detector type:")+myDet->slsDetectorBase::getDetectorType(detType),"qTabDeveloper::SetupWidgetWindow");
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

	qDefs::checkErrorMessage(myDet,"qTabDeveloper::SetupWidgetWindow");
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
		spinDacs[i]->setMinimum(-1);
		spinDacs[i]->setMaximum(10000);
		lblDacsmV[i]= new QLabel("",boxDacs);


		dacLayout->addWidget(lblDacs[i],(int)(i/2),((i%2)==0)?1:5);
		dacLayout->addWidget(spinDacs[i],(int)(i/2),((i%2)==0)?2:6);
		dacLayout->addWidget(lblDacsmV[i],(int)(i/2),((i%2)==0)?3:7);
		if(!(i%2)){
			dacLayout->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed),(int)(i/2),0);
			dacLayout->addItem(new QSpacerItem(60,20,QSizePolicy::Fixed,QSizePolicy::Fixed),(int)(i/2),4);
			dacLayout->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed),(int)(i/2),8);
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
	//spinDacs[id]->setValue((double)myDet->setDAC((dacs_t)spinDacs[id]->value(),getSLSIndex(id)));
	myDet->setDAC((dacs_t)spinDacs[id]->value(),getSLSIndex(id),0);
	lblDacsmV[id]->setText(QString("%1mV").arg(myDet->setDAC(-1,getSLSIndex(id),1),-10));

	qDefs::checkErrorMessage(myDet,"qTabDeveloper::SetDacValues");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::SetHighVoltage(){
#ifdef VERBOSE
	cout << "Setting high voltage:" << comboHV->currentText().toAscii().constData() << endl;
#endif
	int highvoltage = comboHV->currentText().toInt();
	int ret = myDet->setDAC(highvoltage,slsDetectorDefs::HV_POT,0);
	qDefs::checkErrorMessage(myDet,"qTabDeveloper::SetHighVoltage");
	//error
	if(ret != highvoltage){
		qDefs::Message(qDefs::CRITICAL,"High Voltage could not be set to this value.","qTabDeveloper::SetHighVoltage");
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
			qDefs::Message(qDefs::CRITICAL,"Unknown DAC/ADC Index. Weird Error Index:"+ index,"qTabDeveloper::getSLSIndex");
			Refresh();
			break;
		}
		break;
	case slsDetectorDefs::EIGER:
		switch(index){
		case 0:	return slsDetectorDefs::E_SvP;
		case 1:	return slsDetectorDefs::E_SvN;
		case 2:	return slsDetectorDefs::E_Vrf;
		case 3:	return slsDetectorDefs::E_Vrs;
		case 4:	return slsDetectorDefs::E_Vtr;
		case 5:	return slsDetectorDefs::E_Vtgstv;
		case 6:	return slsDetectorDefs::E_cal;
		case 7:	return slsDetectorDefs::E_Vcp;
		case 8:	return slsDetectorDefs::E_Vcn;
		case 9:	return slsDetectorDefs::E_Vis;
		case 10:return slsDetectorDefs::E_rxb_lb;
		case 11:return slsDetectorDefs::E_rxb_rb;
		case 12:return slsDetectorDefs::E_Vcmp_ll;
		case 13:return slsDetectorDefs::E_Vcmp_lr;
		case 14:return slsDetectorDefs::E_Vcmp_rl;
		case 15:return slsDetectorDefs::E_Vcmp_rr;



		default:
			qDefs::Message(qDefs::CRITICAL,"Unknown DAC/ADC Index. Weird Error Index:"+ index,"qTabDeveloper::getSLSIndex");
			Refresh();
			break;
		}
		break;
	case slsDetectorDefs::MOENCH:
		switch(index){
		case 0:	return slsDetectorDefs::V_DAC0;
		case 1:	return slsDetectorDefs::V_DAC1;
		case 2:	return slsDetectorDefs::V_DAC2;
		case 3:	return slsDetectorDefs::V_DAC3;
		case 4:	return slsDetectorDefs::V_DAC4;
		case 5:	return slsDetectorDefs::V_DAC5;
		case 6:	return slsDetectorDefs::V_DAC6;
		case 7:	return slsDetectorDefs::V_DAC7;
		case 8:	return slsDetectorDefs::TEMPERATURE_ADC;
		case 9:return slsDetectorDefs::TEMPERATURE_FPGA;

		default:
			qDefs::Message(qDefs::CRITICAL,"Unknown DAC/ADC Index. Weird Error. Index:"+ index,"qTabDeveloper::getSLSIndex");
			Refresh();
			break;
		}
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
		case 9:return slsDetectorDefs::TEMPERATURE_FPGA;
		default:
			qDefs::Message(qDefs::CRITICAL,"Unknown DAC/ADC Index. Weird Error Index:"+ index,"qTabDeveloper::getSLSIndex");
			Refresh();
			break;
		}
		break;
	default:
		cout << "Unknown detector type:" + myDet->slsDetectorBase::getDetectorType(detType) << endl;
		qDefs::Message(qDefs::CRITICAL,string("Unknown detector type:")+myDet->slsDetectorBase::getDetectorType(detType),"qTabDeveloper::getSLSIndex");
		qDefs::checkErrorMessage(myDet,"qTabDeveloper::getSLSIndex");
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
	qDefs::checkErrorMessage(myDet,"qTabDeveloper::RefreshAdcs");
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

	for(int i=0;i<NUM_DAC_WIDGETS;i++){
		spinDacs[i]->setValue((double)myDet->setDAC(-1,getSLSIndex(i),0));
		lblDacsmV[i]->setText(QString("%1mV").arg(myDet->setDAC(-1,getSLSIndex(i),1),-10));
	}
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
		int ret = (int)myDet->setDAC(-1,slsDetectorDefs::HV_POT,0);
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

	qDefs::checkErrorMessage(myDet,"qTabDeveloper::Refresh");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


