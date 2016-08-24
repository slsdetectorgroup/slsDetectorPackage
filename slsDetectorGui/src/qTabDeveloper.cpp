/*
 * qTabDeveloper.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
#include "qTabDeveloper.h"
#include "qDetectorMain.h"
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


qTabDeveloper::qTabDeveloper(qDetectorMain *parent,multiSlsDetector*& detector):
						thisParent(parent),
						myDet(detector),
						det(0),
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
	if(det) delete det;
	if(thisParent) delete thisParent;
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
		NUM_DAC_WIDGETS = 17;
		NUM_ADC_WIDGETS = 6;

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
		dacNames.push_back("v threshold:");

		adcNames.push_back("Temperature FPGA Ext:");
		adcNames.push_back("Temperature 10GE:");
		adcNames.push_back("Temperature DCDC:");
		adcNames.push_back("Temperature SODL:");
		adcNames.push_back("Temperature SODR:");
		adcNames.push_back("Temperature FPGA:");

		break;
	case slsDetectorDefs::PROPIX:
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


	case slsDetectorDefs::JUNGFRAU:
		NUM_DAC_WIDGETS = 16;
		NUM_ADC_WIDGETS = 0;
		dacNames.push_back("v Dac 0:");
		dacNames.push_back("v Dac 1:");
		dacNames.push_back("v Dac 2:");
		dacNames.push_back("v Dac 3:");
		dacNames.push_back("v Dac 4:");
		dacNames.push_back("v Dac 5:");
		dacNames.push_back("v Dac 6:");
		dacNames.push_back("i Dac 7:");
		dacNames.push_back("v Dac 8:");
		dacNames.push_back("v Dac 9:");
		dacNames.push_back("v Dac 10:");
		dacNames.push_back("v Dac 11:");
		dacNames.push_back("v Dac 12:");
		dacNames.push_back("v Dac 13:");
		dacNames.push_back("v Dac 14:");
		dacNames.push_back("i Dac 15:");


		break;

	default:
		cout << "ERROR: Unknown detector type: " + myDet->slsDetectorBase::getDetectorType(detType) << endl;
		qDefs::Message(qDefs::CRITICAL,string("Unknown detector type:")+myDet->slsDetectorBase::getDetectorType(detType),"qTabDeveloper::SetupWidgetWindow");
		exit(-1);
		break;
	}


	//layout
	setFixedWidth(765);
	setFixedHeight(20+50+(NUM_DAC_WIDGETS/2)*35);
	//setHeight(340);

	scroll  = new QScrollArea;
	//scroll->setFrameShape(QFrame::NoFrame);
	scroll->setWidget(this);
	scroll->setWidgetResizable(true);

	layout = new QGridLayout(scroll);
	layout->setContentsMargins(20,10,10,5);
	setLayout(layout);

	//readout
	comboDetector = new QComboBox(this);
	//comboDetector->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	comboDetector->addItem("All");
	//add detectors
	for(int i=1;i<myDet->getNumberOfDetectors()+1;i++)
		comboDetector->addItem(QString(myDet->getHostname(i-1).c_str()));
	comboDetector->setCurrentIndex(0);


	//dacs
	boxDacs = new QGroupBox("Dacs",this);
	boxDacs->setFixedHeight(25+(NUM_DAC_WIDGETS/2)*35);
	CreateDACWidgets();

	//HV for gotthard
	if ((detType==slsDetectorDefs::GOTTHARD) ||
			(detType==slsDetectorDefs::PROPIX) ||
			(detType==slsDetectorDefs::MOENCH)){
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
	layout->addWidget(comboDetector,0,0);
	layout->addWidget(boxDacs,1,0);


	//adcs
	if((detType==slsDetectorDefs::GOTTHARD) ||
			(detType==slsDetectorDefs::PROPIX) ||
			(detType==slsDetectorDefs::MOENCH)||
			(detType==slsDetectorDefs::EIGER))		{
	    setFixedHeight(20+(50+(NUM_DAC_WIDGETS/2)*35)+(50+(NUM_ADC_WIDGETS/2)*35));
		boxAdcs = new QGroupBox("ADCs",this);
		boxAdcs->setFixedHeight(25+(NUM_ADC_WIDGETS/2)*35);
		layout->addWidget(boxAdcs,2,0);
		CreateADCWidgets();
		//to make the adcs at the bottom most
		if(detType!=slsDetectorDefs::EIGER) {
			int diff = 340-height();
			setFixedHeight(340);
			layout->setVerticalSpacing(diff/2);
		}
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

	connect(comboDetector,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(Refresh()));

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
		spinAdcs[i]->setMinimum(-1);
		if((detType==slsDetectorDefs::GOTTHARD) ||
				(detType==slsDetectorDefs::PROPIX) ||
				(detType==slsDetectorDefs::MOENCH)||
				(detType==slsDetectorDefs::EIGER))
			spinAdcs[i]->setSuffix(0x00b0+QString("C"));

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

	int detid = comboDetector->currentIndex();
	if(detid)
		det = myDet->getSlsDetector(detid-1);

	//all detectors
	if(!detid){
		myDet->setDAC((dacs_t)spinDacs[id]->value(),getSLSIndex(id),0);
		lblDacsmV[id]->setText(QString("%1mV").arg(myDet->setDAC(-1,getSLSIndex(id),1),-10));
		qDefs::checkErrorMessage(myDet,"qTabDeveloper::SetDacValues");
	}
	//specific detector
	else{
		det->setDAC((dacs_t)spinDacs[id]->value(),getSLSIndex(id),0);
		lblDacsmV[id]->setText(QString("%1mV").arg(det->setDAC(-1,getSLSIndex(id),1),-10));
		qDefs::checkErrorMessage(det,"qTabDeveloper::SetDacValues");
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::SetHighVoltage(){
#ifdef VERBOSE
	cout << "Setting high voltage:" << comboHV->currentText().toAscii().constData() << endl;
#endif

	int detid = comboDetector->currentIndex();
	if(detid)
		det = myDet->getSlsDetector(detid-1);

	int highvoltage = comboHV->currentText().toInt();
	int ret;

	//all detectors
	if(!detid){
		ret = myDet->setDAC(highvoltage,slsDetectorDefs::HV_POT,0);
		qDefs::checkErrorMessage(myDet,"qTabDeveloper::SetHighVoltage");
	}
	//specific detector
	else{
		ret = det->setDAC(highvoltage,slsDetectorDefs::HV_POT,0);
		qDefs::checkErrorMessage(det,"qTabDeveloper::SetHighVoltage");
	}


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
			case 16:return slsDetectorDefs::THRESHOLD;
			case 17:return slsDetectorDefs::TEMPERATURE_FPGAEXT;
			case 18:return slsDetectorDefs::TEMPERATURE_10GE;
			case 19:return slsDetectorDefs::TEMPERATURE_DCDC;
			case 20:return slsDetectorDefs::TEMPERATURE_SODL;
			case 21:return slsDetectorDefs::TEMPERATURE_SODR;
			case 22:return slsDetectorDefs::TEMPERATURE_FPGA;
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
				case slsDetectorDefs::PROPIX:
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
					case slsDetectorDefs::JUNGFRAU:
						switch(index){
						case 0:
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 6:
						case 7:
						case 8:
						case 9:
						case 10:
						case 11:
						case 12:
						case 13:
						case 14:
						case 15:
							return (slsDetectorDefs::dacIndex)index;
							break;
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
	if(!thisParent->isCurrentlyTabDeveloper())
		return;

#ifdef VERYVERBOSE
	cout << "Updating ADCs" <<endl;
#endif
	adcTimer->stop();

	int detid = comboDetector->currentIndex();
	if(detid)
		det = myDet->getSlsDetector(detid-1);

	for(int i=0;i<NUM_ADC_WIDGETS;i++){
		//all detectors
		if(!detid){
			if(detType == slsDetectorDefs::EIGER){
				double value = (double)myDet->getADC(getSLSIndex(i+NUM_DAC_WIDGETS));
				if(value == -1)
					spinAdcs[i]->setValue(value);
				else
					spinAdcs[i]->setValue(value/1000.00);

			}
			else
				spinAdcs[i]->setValue((double)myDet->getADC(getSLSIndex(i+NUM_DAC_WIDGETS)));
		}
		//specific detector
		else{
			if(detType == slsDetectorDefs::EIGER)
				spinAdcs[i]->setValue((double)det->getADC(getSLSIndex(i+NUM_DAC_WIDGETS))/1000.00);
			else
				spinAdcs[i]->setValue((double)det->getADC(getSLSIndex(i+NUM_DAC_WIDGETS)));
		}
	}


	adcTimer->start(ADC_TIMEOUT);
	qDefs::checkErrorMessage(myDet,"qTabDeveloper::RefreshAdcs");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabDeveloper::Refresh(){
#ifdef VERBOSE
	cout  << endl << "**Updating Developer Tab" << endl;
#endif


	int detid = comboDetector->currentIndex();
	if(detid)
		det = myDet->getSlsDetector(detid-1);


	//dacs
#ifdef VERBOSE
	cout << "Gettings DACs"  << endl;
#endif
	for(int i=0;i<NUM_DAC_WIDGETS;i++){
		//all detectors
		if(!detid){
			spinDacs[i]->setValue((double)myDet->setDAC(-1,getSLSIndex(i),0));
			lblDacsmV[i]->setText(QString("%1mV").arg(myDet->setDAC(-1,getSLSIndex(i),1),-10));
		}
		//specific detector
		else{
			spinDacs[i]->setValue((double)det->setDAC(-1,getSLSIndex(i),0));
			lblDacsmV[i]->setText(QString("%1mV").arg(det->setDAC(-1,getSLSIndex(i),1),-10));
		}
	}


	//adcs
	if(NUM_ADC_WIDGETS) RefreshAdcs();

	//gotthard -high voltage
	if((detType == slsDetectorDefs::GOTTHARD) ||
			(detType == slsDetectorDefs::PROPIX) ||
			(detType == slsDetectorDefs::MOENCH)){
		disconnect(comboHV,	SIGNAL(currentIndexChanged(int)),	this, SLOT(SetHighVoltage()));

		//default should be correct
		lblHV->setPalette(lblDacs[0]->palette());
		lblHV->setText("High Voltage:");
		lblHV->setToolTip(tipHV);
		comboHV->setToolTip(tipHV);
		//getting hv value
		int ret;
		if(!detid) 	ret = (int)myDet->setDAC(-1,slsDetectorDefs::HV_POT,0);
		else 		ret = (int)det->setDAC(-1,slsDetectorDefs::HV_POT,0);

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


