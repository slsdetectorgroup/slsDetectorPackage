/*
 * qTabAdvanced.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
#include "qTabAdvanced.h"
#include "qDrawPlot.h"
/** Project Class Headers */
#include "slsDetector.h"
#include "multiSlsDetector.h"
/** Qt Include Headers */
#include <QFileDialog>
/** C++ Include Headers */
#include<iostream>


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabAdvanced::qTabAdvanced(QWidget *parent,multiSlsDetector*& detector, qDrawPlot*& plot):
				  QWidget(parent),myDet(detector),myPlot(plot),btnGroup(NULL),isEnergy(false),
				  lblFromX(0),
				  spinFromX(0),
				  lblFromY(0),
				  spinFromY(0),
				  lblToX(0),
				  spinToX(0),
				  lblToY(0),
				  spinToY(0),
				  numRois(0){
	setupUi(this);
	SetupWidgetWindow();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------



qTabAdvanced::~qTabAdvanced(){
	delete myDet;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetupWidgetWindow(){

//executed even for non digital, so make sure its necessary

	//Network
	lblIP->setEnabled(false);
	lblMAC->setEnabled(false);
	dispIP->setEnabled(false);
	dispMAC->setEnabled(false);
	boxRxr->setEnabled(false);
	boxSetAllTrimbits->setEnabled(false);


	red = QPalette();
	red.setColor(QPalette::Active,QPalette::WindowText,Qt::red);
	detOnlineTip = comboOnline->toolTip();
	rxrOnlineTip = comboRxrOnline->toolTip();
	errOnlineTip = QString("<nobr><br><br><font color=\"red\"><nobr>It is offline!</nobr></font>");

	acqSubPeriodTip = spinSubPeriod->toolTip();
	errSubPeriodTip = acqSubPeriodTip +
					QString("<nobr><br><br><font color=\"red\"><b>Sub Frame Period</b> "
							"should be greater than or equal to "
							"<b>Sub Frame Exposure Time</b>.</font></nobr>");



	detType = myDet->getDetectorTypeAsEnum();
	switch(detType){
	case slsDetectorDefs::EIGER:
		isEnergy = true;
		lblIP->setEnabled(true);
		lblMAC->setEnabled(true);
		dispIP->setEnabled(true);
		dispMAC->setEnabled(true);
		boxRxr->setEnabled(true);
		boxSetAllTrimbits->setEnabled(true);
		lblSubExpTime->setEnabled(true);
		spinSubExpTime->setEnabled(true);
		comboSubExpTimeUnit->setEnabled(true);
		lblSubPeriod->setEnabled(true);
		spinSubPeriod->setEnabled(true);
		comboSubPeriodUnit->setEnabled(true);
		break;
	case slsDetectorDefs::MOENCH:
		isEnergy = false;
		lblIP->setEnabled(true);
		lblMAC->setEnabled(true);
		dispIP->setEnabled(true);
		dispMAC->setEnabled(true);
		boxRxr->setEnabled(true);
		break;
	case slsDetectorDefs::GOTTHARD:
		isEnergy = false;
		lblIP->setEnabled(true);
		lblMAC->setEnabled(true);
		dispIP->setEnabled(true);
		dispMAC->setEnabled(true);
		boxRxr->setEnabled(true);
		break;
	case slsDetectorDefs::JUNGFRAU:
	case slsDetectorDefs::CHIPTESTBOARD:
		isEnergy = false;
		lblIP->setEnabled(true);
		lblMAC->setEnabled(true);
		dispIP->setEnabled(true);
		dispMAC->setEnabled(true);
		boxRxr->setEnabled(true);
		break;
	default: break;
	}


	// logs and trimming
	if(!isEnergy){
		boxPlot->setEnabled(false);
	}else{
		btnGroup = new QButtonGroup(this);
		btnGroup->addButton(btnRefresh,0);
		btnGroup->addButton(btnGetTrimbits,1);
	}


	//network

	//add detectors
	for(int i=0;i<myDet->getNumberOfDetectors();i++)
		comboDetector->addItem(QString(myDet->getHostname(i).c_str()));

	comboDetector->setCurrentIndex(0);
	int module_id = comboDetector->currentIndex();

	qDefs::checkErrorMessage(myDet,"qTabAdvanced::SetupWidgetWindow");
	cout << "Getting ports" << endl;
	spinControlPort->setValue(myDet->setControlPort(-1, module_id));
	spinStopPort->setValue(myDet->setStopPort(-1, module_id));
	spinTCPPort->setValue(myDet->setReceiverPort(-1, module_id));
	spinUDPPort->setValue(myDet->getReceiverUDPPort(module_id));
	spinZmqPort->setValue(myDet->getClientStreamingPort(module_id));
	spinZmqPort2->setValue(myDet->getReceiverStreamingPort(module_id));

	cout << "Getting network information" << endl;
	dispIP->setText(myDet->getDetectorIP(module_id).c_str());
	dispMAC->setText(myDet->getDetectorMAC(module_id).c_str());
	dispRxrHostname->setText(myDet->getReceiver(module_id).c_str());
	dispUDPIP->setText(myDet->getReceiverUDPIP(module_id).c_str());
	dispUDPMAC->setText(myDet->getReceiverUDPMAC(module_id).c_str());
	dispZMQIP->setText(myDet->getClientStreamingIP(module_id).c_str());
	dispZMQIP2->setText(myDet->getReceiverStreamingIP(module_id).c_str());

	//check if its online and set it to red if offline
#ifdef VERYVERBOSE
	cout << "online" << endl;
#endif
	if(myDet->setOnline(module_id)==slsDetectorDefs::ONLINE_FLAG)
		myDet->checkOnline(module_id);
	if(myDet->setReceiverOnline(module_id)==slsDetectorDefs::ONLINE_FLAG)
		myDet->checkReceiverOnline(module_id);
	comboOnline->setCurrentIndex(myDet->setOnline(module_id));
	comboRxrOnline->setCurrentIndex(myDet->setReceiverOnline(module_id));
	if(!comboOnline->currentIndex()){
		comboOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setPalette(red);
		lblOnline->setText("Online:*");
	}
	if((comboRxrOnline->isEnabled())&&(!comboRxrOnline->currentIndex())){
		comboRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
		lblRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
		lblRxrOnline->setPalette(red);
		lblRxrOnline->setText("Online:*");
	}


	//updates roi
	cout << "Getting ROI" << endl;
	if (detType == slsDetectorDefs::GOTTHARD)
		updateROIList();
#ifdef VERYVERBOSE
	//  print receiver configurations
	// if(detType != slsDetectorDefs::MYTHEN){
		cout << endl;
		myDet->printReceiverConfiguration();
	// }
#endif

	// jungfrau
	if (detType == slsDetectorDefs::JUNGFRAU) {
	    lblNumStoragecells->setEnabled(true);
	    spinNumStoragecells->setEnabled(true);
	    spinNumStoragecells->setValue((int)myDet->setTimer(slsDetectorDefs::STORAGE_CELL_NUMBER,-1));
	} else if (detType == slsDetectorDefs::EIGER) {
		//subexptime
		qDefs::timeUnit unit;
		double time = qDefs::getCorrectTime(unit,((double)(myDet->setTimer(slsDetectorDefs::SUBFRAME_ACQUISITION_TIME,-1)*(1E-9))));
		spinSubExpTime->setValue(time);
		comboSubExpTimeUnit->setCurrentIndex((int)unit);
		//period
		time = qDefs::getCorrectTime(unit,((double)(myDet->setTimer(slsDetectorDefs::SUBFRAME_DEADTIME,-1)*(1E-9))));
		spinSubPeriod->setValue(time);
		comboSubPeriodUnit->setCurrentIndex((int)unit);

		CheckAcqPeriodGreaterThanExp();
	}

	Initialization();

	qDefs::checkErrorMessage(myDet, module_id,"qTabAdvanced::SetupWidgetWindow");

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::Initialization(){

	connect(tabAdvancedSettings,SIGNAL(currentChanged(int)),	this, SLOT(Refresh()));

	if(isEnergy){
		//setalltrimbits
		if(boxSetAllTrimbits->isEnabled())
			connect(spinSetAllTrimbits,	SIGNAL(editingFinished()),	this,	SLOT(SetAllTrimbits()));

		//refresh
		connect(btnGroup,		SIGNAL(buttonClicked(int)),	this, SLOT(UpdateTrimbitPlot(int)));
	}

	//network
	connect(comboDetector,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetDetector(int)));
	connect(spinControlPort,	SIGNAL(valueChanged(int)),	this,	SLOT(SetControlPort(int)));
	connect(spinStopPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetStopPort(int)));
	connect(comboOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetOnline(int)));

	//network
	connect(spinTCPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrTCPPort(int)));
	connect(spinUDPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrUDPPort(int)));
	connect(spinZmqPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetCltZmqPort(int)));
	connect(spinZmqPort2,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrZmqPort(int)));
	connect(comboRxrOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetReceiverOnline(int)));

	connect(dispIP,				SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	connect(dispMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	connect(dispUDPIP,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	connect(dispUDPMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));

	connect(dispZMQIP,			SIGNAL(editingFinished()),	this, SLOT(SetClientZMQIP()));
	connect(dispZMQIP2,			SIGNAL(editingFinished()),	this, SLOT(SetReceiverZMQIP()));

	connect(btnRxr,				SIGNAL(clicked()),			this, SLOT(SetReceiver()));



	//roi

	if (detType == slsDetectorDefs::GOTTHARD) {
		connect(btnClearRoi,		SIGNAL(clicked()),			this, SLOT(clearROIinDetector()));
		connect(btnGetRoi,			SIGNAL(clicked()),			this, SLOT(updateROIList()));
		connect(btnSetRoi,			SIGNAL(clicked()),			this, SLOT(setROI()));
	}

	if(detType == slsDetectorDefs::JUNGFRAU) {
	    connect(spinNumStoragecells, SIGNAL(valueChanged(int)),  this,  SLOT(SetNumStoragecells(int)));
	} else if (detType == slsDetectorDefs::EIGER) {
		//Exposure Time
		connect(spinSubExpTime,SIGNAL(valueChanged(double)),			this,	SLOT(SetSubExposureTime()));
		connect(comboSubExpTimeUnit,SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetSubExposureTime()));
		//Frame Period between exposures
		connect(spinSubPeriod,SIGNAL(valueChanged(double)),			this,	SLOT(SetSubPeriod()));
		connect(comboSubPeriodUnit,SIGNAL(currentIndexChanged(int)),this,	SLOT(SetSubPeriod()));

	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


 void qTabAdvanced::UpdateTrimbitPlot(int id){
 	if(boxPlot->isChecked()){
 		//refresh
 		if(!id)	myPlot->UpdateTrimbitPlot(false,radioHistogram->isChecked());
 		//from detector
 		else	myPlot->UpdateTrimbitPlot(true,radioHistogram->isChecked());
 	}
 }


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetControlPort(int port){
#ifdef VERBOSE
	cout << "Setting Control Port:" << port << endl;
#endif
	disconnect(spinControlPort,	SIGNAL(valueChanged(int)),	this,	SLOT(SetControlPort(int)));
	spinControlPort->setValue(myDet->setControlPort(port, comboDetector->currentIndex()));
	qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabAdvanced::SetControlPort");
	connect(spinControlPort,	SIGNAL(valueChanged(int)),	this,	SLOT(SetControlPort(int)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetStopPort(int port){
#ifdef VERBOSE
	cout << "Setting Stop Port:" << port << endl;
#endif
	disconnect(spinStopPort,	SIGNAL(valueChanged(int)),	this,	SLOT(SetStopPort(int)));
	spinStopPort->setValue(myDet->setStopPort(port, comboDetector->currentIndex()));
	qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabAdvanced::SetStopPort");
	connect(spinStopPort,	SIGNAL(valueChanged(int)),	this,	SLOT(SetStopPort(int)));

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetRxrTCPPort(int port){
#ifdef VERBOSE
	cout << "Setting Receiver TCP Port:" << port << endl;
#endif
	disconnect(spinTCPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrTCPPort(int)));
	spinTCPPort->setValue(myDet->setReceiverPort(port, comboDetector->currentIndex()));
	qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabAdvanced::SetRxrTCPPort");
	connect(spinTCPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrTCPPort(int)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetRxrUDPPort(int port){
#ifdef VERBOSE
	std::cout << "Setting Receiver UDP Port:" << port << endl;
#endif

	disconnect(spinUDPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrUDPPort(int)));
	spinUDPPort->setValue(myDet->setReceiverUDPPort(port, comboDetector->currentIndex()));
	qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabAdvanced::SetRxrUDPPort");
	connect(spinUDPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrUDPPort(int)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetCltZmqPort(int port){
#ifdef VERBOSE
	std::cout << "Setting Client UDP Port:" << port << endl;
#endif
	 std::ostringstream ss; ss << port; std::string sport = ss.str();

	disconnect(spinZmqPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetCltZmqPort(int)));
	myDet->setClientDataStreamingInPort(port, comboDetector->currentIndex());
	spinZmqPort->setValue(myDet->getClientStreamingPort(comboDetector->currentIndex()));
	qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabAdvanced::SetCltZmqPort");
	myDet->enableDataStreamingToClient(false);
	myDet->enableDataStreamingToClient(true);
	qDefs::checkErrorMessage(myDet,"qTabAdvanced::SetCltZmqPort");
	connect(spinZmqPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetCltZmqPort(int)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetRxrZmqPort(int port){
#ifdef VERBOSE
	std::cout << "Setting Receiver UDP Port:" << port << endl;
#endif
	 std::ostringstream ss; ss << port; std::string sport = ss.str();

	disconnect(spinZmqPort2,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrZmqPort(int)));
	myDet->setReceiverDataStreamingOutPort(port, comboDetector->currentIndex());
	spinZmqPort2->setValue(myDet->getReceiverStreamingPort(comboDetector->currentIndex()));
	qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabAdvanced::SetRxrZmqPort");
	myDet->enableDataStreamingFromReceiver(false);
	myDet->enableDataStreamingFromReceiver(true);
	qDefs::checkErrorMessage(myDet,"qTabAdvanced::SetRxrZmqPort");
	connect(spinZmqPort2,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrZmqPort(int)));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetReceiverOnline(int index){
#ifdef VERBOSE
	cout << "Setting Reciever Online to :" << index << endl;
#endif
	disconnect(comboRxrOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetReceiverOnline(int)));
	if(index){
		SetReceiver();
	}else{
		comboRxrOnline->setCurrentIndex(myDet->setReceiverOnline(index, comboDetector->currentIndex()));
	}
	qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabAdvanced::SetReceiverOnline");
	connect(comboRxrOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetReceiverOnline(int)));
	//highlight in red if offline
	if(!comboRxrOnline->currentIndex()){
		comboRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
		lblRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
		lblRxrOnline->setPalette(red);
		lblRxrOnline->setText("Online:*");
	}else{
		comboRxrOnline->setToolTip(rxrOnlineTip);
		lblRxrOnline->setToolTip(rxrOnlineTip);
		lblRxrOnline->setPalette(lblHostname->palette());
		lblRxrOnline->setText("Online:");
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetOnline(int index){
#ifdef VERBOSE
	cout << "Setting Detector Online to " << index << endl;
#endif
	disconnect(comboOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetOnline(int)));
	comboOnline->setCurrentIndex(myDet->setOnline(index, comboDetector->currentIndex()));
	qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabAdvanced::SetOnline");
	connect(comboOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetOnline(int)));
	//highlight in red if offline
	if(!comboOnline->currentIndex()){
		comboOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setPalette(red);
		lblOnline->setText("Online:*");
	}else{
		comboOnline->setToolTip(detOnlineTip);
		lblOnline->setToolTip(detOnlineTip);
		lblOnline->setPalette(lblHostname->palette());
		lblOnline->setText("Online:");
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetNetworkParameters(){
#ifdef VERBOSE
	cout << "Setting Network Parametrs" << endl;
#endif
	disconnect(dispIP,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	disconnect(dispMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	disconnect(dispUDPIP,		SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	disconnect(dispUDPMAC,		SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));

	auto module_id = comboDetector->currentIndex();
	dispIP->setText(QString(myDet->setDetectorIP(dispIP->text().toAscii().constData(), module_id).c_str()));
	dispMAC->setText(QString(myDet->setDetectorMAC(dispMAC->text().toAscii().constData(), module_id).c_str()));
	dispUDPIP->setText(QString(myDet->setReceiverUDPIP(dispUDPIP->text().toAscii().constData(), module_id).c_str()));
	dispUDPMAC->setText(QString(myDet->setReceiverUDPMAC(dispUDPMAC->text().toAscii().constData(), module_id).c_str()));
	qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabAdvanced::SetNetworkParameters");

	connect(dispIP,				SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	connect(dispMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	connect(dispUDPIP,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	connect(dispUDPMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetClientZMQIP(){
#ifdef VERBOSE
	cout << "Setting Client ZMQ IP" << endl;
#endif
	disconnect(dispZMQIP,			SIGNAL(editingFinished()),	this, SLOT(SetClientZMQIP()));

	auto module_id = comboDetector->currentIndex();
	myDet->setClientDataStreamingInIP(dispZMQIP->text().toAscii().constData(), module_id);
	dispZMQIP->setText(QString(myDet->getClientStreamingIP(module_id).c_str()));
	myDet->enableDataStreamingToClient(false);
	myDet->enableDataStreamingToClient(true);
	qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabAdvanced::SetClientZMQIP");

	connect(dispZMQIP,			SIGNAL(editingFinished()),	this, SLOT(SetClientZMQIP()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetReceiverZMQIP(){
#ifdef VERBOSE
	cout << "Setting Receiver ZMQ IP" << endl;
#endif
	disconnect(dispZMQIP2,			SIGNAL(editingFinished()),	this, SLOT(SetReceiverZMQIP()));

	auto module_id = comboDetector->currentIndex();
	myDet->setReceiverDataStreamingOutIP(dispZMQIP2->text().toAscii().constData(), module_id);
	dispZMQIP2->setText(QString(myDet->getReceiverStreamingIP(module_id).c_str()));
	myDet->enableDataStreamingFromReceiver(false);
	myDet->enableDataStreamingFromReceiver(true);
	qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(),"qTabAdvanced::SetReceiverZMQIP");

	connect(dispZMQIP2,			SIGNAL(editingFinished()),	this, SLOT(SetReceiverZMQIP()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetReceiver(){
#ifdef VERBOSE
	cout << "Setting Receiver" << endl;
#endif
	auto outdir = myDet->getFilePath();
	auto module_id = comboDetector->currentIndex();
	dispRxrHostname->setText(QString(myDet->setReceiver(dispRxrHostname->text().toAscii().constData(), module_id).c_str()));
	qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabAdvanced::SetReceiver");
	myDet->setFilePath(outdir, module_id);
	qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabAdvanced::SetReceiver");
	Refresh();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::updateROIList(){
#ifdef VERYVERBOSE
	cout<<"in updateROIList() " << endl;
#endif
	clearROI();

	int n,i;
	slsDetectorDefs::ROI* temp = myDet->getROI(n);
	qDefs::checkErrorMessage(myDet,"qTabAdvanced::updateROIList");

	if((temp!=NULL)&&(n>0)){
		//assign into array, else it loses values cuz of memory
		slsDetectorDefs::ROI allroi[n];
		for(i=0;i<n;i++)
			allroi[i] =  temp[i];

		//add roi inputs
		AddROIInput(n);
		//populating roi list
		for (i=0;i<n;i++){
			spinFromX[i]->setValue(allroi[i].xmin);
			spinFromY[i]->setValue(allroi[i].ymin);
			spinToX[i]->setValue(allroi[i].xmax);
			spinToY[i]->setValue(allroi[i].ymax);
		}
		cout << "ROIs populated: " << n << endl;
	}


}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::AddROIInput(int num){
#ifdef VERVERBOSE
	cout<<"in AddROIInput() " << num << endl;
#endif
	if((int)lblFromX.size()){
		disconnect(spinFromX[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
		disconnect(spinFromY[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
		disconnect(spinToX[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
		disconnect(spinToY[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
	}

	int exists = numRois+1;
	int total = exists+num;
	//if cleared, addding just one
	if ((num==0) && (numRois==0)){
		exists = 0;
		total = 1;
	}/*else{
		gridRoi->removeWidget
	}*/

	for (int i=exists;i<total;i++){

		if(i >= ((int)lblFromX.size())){
			lblFromX.resize(i+1);	spinFromX.resize(i+1);
			lblFromY.resize(i+1);	spinFromY.resize(i+1);
			lblToX.resize(i+1);	spinToX.resize(i+1);
			lblToY.resize(i+1);	spinToY.resize(i+1);

			lblFromX[i] 	= new QLabel("x min:");
			lblFromY[i] 	= new QLabel("y min:");
			lblToX[i] 		= new QLabel("x max:");
			lblToY[i] 		= new QLabel("y max:");
			spinFromX[i] 	= new QSpinBox();
			spinFromY[i] 	= new QSpinBox();
			spinToX[i] 		= new QSpinBox();
			spinToY[i] 		= new QSpinBox();


			lblFromX[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			lblFromX[i]->setFixedWidth(50);
			lblFromY[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			lblFromY[i]->setFixedWidth(50);
			lblToX[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			lblToX[i]->setFixedWidth(50);
			lblToY[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			lblToY[i]->setFixedWidth(50);
			spinFromX[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			spinFromX[i]->setFixedWidth(80);
			spinFromY[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			spinFromY[i]->setFixedWidth(80);
			spinToX[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			spinToX[i]->setFixedWidth(80);
			spinToY[i]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);			spinToY[i]->setFixedWidth(80);
			spinFromX[i]->setFixedHeight(19);
			spinFromY[i]->setFixedHeight(19);
			spinToX[i]->setFixedHeight(19);
			spinToY[i]->setFixedHeight(19);

			spinFromX[i]->setMaximum(myDet->getTotalNumberOfChannels(slsDetectorDefs::X)-1);
			spinToX[i]->setMaximum(myDet->getTotalNumberOfChannels(slsDetectorDefs::X)-1);
			spinFromY[i]->setMaximum(myDet->getTotalNumberOfChannels(slsDetectorDefs::Y)-1);
			spinToY[i]->setMaximum(myDet->getTotalNumberOfChannels(slsDetectorDefs::Y)-1);
			spinFromX[i]->setMinimum(-1);
			spinToX[i]->setMinimum(-1);
			spinFromY[i]->setMinimum(-1);
			spinToY[i]->setMinimum(-1);
			spinFromX[i]->setValue(-1);
			spinFromY[i]->setValue(-1);
			spinToX[i]->setValue(-1);
			spinToY[i]->setValue(-1);
		}

		gridRoi->addWidget(lblFromX[i],	i,0,Qt::AlignTop);
		gridRoi->addWidget(spinFromX[i],i,1,Qt::AlignTop);
		gridRoi->addItem(new QSpacerItem(40,20,QSizePolicy::Expanding,QSizePolicy::Fixed),		i,2,Qt::AlignTop);
		gridRoi->addWidget(lblToX[i],	i,3,Qt::AlignTop);
		gridRoi->addWidget(spinToX[i],	i,4,Qt::AlignTop);
		gridRoi->addItem(new QSpacerItem(40,20,QSizePolicy::Expanding,QSizePolicy::Fixed),		i,5,Qt::AlignTop);
		gridRoi->addWidget(lblFromY[i],	i,6,Qt::AlignTop);
		gridRoi->addWidget(spinFromY[i],i,7,Qt::AlignTop);
		gridRoi->addItem(new QSpacerItem(40,20,QSizePolicy::Expanding,QSizePolicy::Fixed),		i,8,Qt::AlignTop);
		gridRoi->addWidget(lblToY[i],	i,9,Qt::AlignTop);
		gridRoi->addWidget(spinToY[i],	i,10,Qt::AlignTop);

		lblFromX[i]->show();
		spinFromX[i]->show();
		lblToX[i]->show();
		spinToX[i]->show();
		lblFromY[i]->show();
		spinFromY[i]->show();
		lblToY[i]->show();
		spinToY[i]->show();
	}

	numRois += num;

	connect(spinFromX[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
	connect(spinFromY[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
	connect(spinToX[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
	connect(spinToY[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));

#ifdef VERYVERBOSE
	cout<<"ROI Inputs added " << num << endl;
#endif

	qDefs::checkErrorMessage(myDet,"qTabAdvanced::AddROIInput");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::clearROI(){
#ifdef VERYVERBOSE
	cout<<"in clearROI() " << endl;
#endif
	if((int)lblFromX.size()){
		disconnect(spinFromX[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
		disconnect(spinFromY[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
		disconnect(spinToX[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));
		disconnect(spinToY[numRois],	SIGNAL(valueChanged(int)),		this,	SLOT(AddROIInputSlot()));

	}


	for (int i=0;i<numRois;i++){
		spinFromX[i]->setValue(-1);
		spinFromY[i]->setValue(-1);
		spinToX[i]->setValue(-1);
		spinToY[i]->setValue(-1);
	}


	//hide widget because they are still visible even when removed and layout deleted
	QLayoutItem *item;
	while((item = gridRoi->takeAt(0))) {
		if (item->widget()){
			item->widget()->hide();
			gridRoi->removeWidget(item->widget());
		}
		//if (item->spacerItem())
	}

	numRois = 0;
	AddROIInput(0);

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::setROI(){
#ifdef VERYVERBOSE
	cout<<"in setROI() " << endl;
#endif

	slsDetectorDefs::ROI allroi[MAX_ROIS];

	for (int i=0;i<numRois;i++){
		allroi[i].xmin = spinFromX[i]->value();
		allroi[i].ymin = spinFromY[i]->value();
		allroi[i].xmax = spinToX[i]->value();
		allroi[i].ymax = spinToY[i]->value();
	}

	myDet->setROI(numRois,allroi);
	//qDefs::checkErrorMessage(myDet);
	cout<<"ROIs set" << endl;
	//get the correct list back
	updateROIList();
	//configuremac
	myDet->configureMAC();

	qDefs::checkErrorMessage(myDet,"qTabAdvanced::setROI");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::clearROIinDetector(){
#ifdef VERYVERBOSE
	cout<<"in clearROIinDetector() " << endl;
#endif

	if (QMessageBox::warning(this, "Clear ROI",
			"Are you sure you want to clear all the ROI in detector?",
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes){

		clearROI();
		setROI();
#ifdef VERBOSE
		cout << "ROIs cleared" << endl;
#endif
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetDetector(int index){
#ifdef VERYVERBOSE
	cout<<"in SetDetector: " << index << endl;
#endif
	// det = myDet->getSlsDetector(comboDetector->currentIndex());
	auto module_id = comboDetector->currentIndex();

	spinControlPort->setValue(myDet->setControlPort(-1, module_id));
	spinStopPort->setValue(myDet->setStopPort(-1, module_id));
	spinTCPPort->setValue(myDet->setReceiverPort(-1, module_id));
	spinUDPPort->setValue(myDet->getReceiverUDPPort(module_id));
	spinZmqPort->setValue(myDet->getClientStreamingPort(module_id));
	spinZmqPort2->setValue(myDet->getReceiverStreamingPort(module_id));

	dispIP->setText(myDet->getDetectorIP(module_id).c_str());
	dispMAC->setText(myDet->getDetectorMAC(module_id).c_str());
	dispRxrHostname->setText(myDet->getReceiver(module_id).c_str());
	dispUDPIP->setText(myDet->getReceiverUDPIP(module_id).c_str());
	dispUDPMAC->setText(myDet->getReceiverUDPMAC(module_id).c_str());
	dispZMQIP->setText(myDet->getClientStreamingIP(module_id).c_str());
	dispZMQIP2->setText(myDet->getReceiverStreamingIP(module_id).c_str());


	//check if its online and set it to red if offline
	if(myDet->getOnline(module_id) == slsDetectorDefs::ONLINE_FLAG)
		myDet->checkOnline(module_id);
	if(myDet->getReceiverOnline(module_id)==slsDetectorDefs::ONLINE_FLAG)
		myDet->checkReceiverOnline(module_id);
	comboOnline->setCurrentIndex(myDet->getOnline(module_id));
	comboRxrOnline->setCurrentIndex(myDet->getReceiverOnline(module_id));
	//highlight in red if detector or receiver is offline
	if(!comboOnline->currentIndex()){
		comboOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setPalette(red);
		lblOnline->setText("Online:*");
	}else{
		comboOnline->setToolTip(detOnlineTip);
		lblOnline->setToolTip(detOnlineTip);
		lblOnline->setPalette(lblHostname->palette());
		lblOnline->setText("Online:");
	}
	if(comboRxrOnline->isEnabled()){
		if(!comboRxrOnline->currentIndex()){
			comboRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
			lblRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
			lblRxrOnline->setPalette(red);
			lblRxrOnline->setText("Online:*");
		}else{
			comboRxrOnline->setToolTip(rxrOnlineTip);
			lblRxrOnline->setToolTip(rxrOnlineTip);
			lblRxrOnline->setPalette(lblHostname->palette());
			lblRxrOnline->setText("Online:");
		}
	}

	qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabAdvanced::SetDetector");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetAllTrimbits(){
#ifdef VERBOSE
	cout<<"Set all trimbits to " << spinSetAllTrimbits->value() << endl;
#endif
	myDet->setAllTrimbits(spinSetAllTrimbits->value());
	 qDefs::checkErrorMessage(myDet,"qTabAdvanced::SetAllTrimbits");
	 updateAllTrimbitsFromServer();

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::updateAllTrimbitsFromServer(){
#ifdef VERBOSE
	cout<<"Getting all trimbits value" << endl;
#endif
	disconnect(spinSetAllTrimbits,	SIGNAL(editingFinished()),	this,	SLOT(SetAllTrimbits()));

	int ret = myDet->setAllTrimbits(-1);
	qDefs::checkErrorMessage(myDet,"qTabAdvanced::updateAllTrimbitsFromServer");
	spinSetAllTrimbits->setValue(ret);

	connect(spinSetAllTrimbits,	SIGNAL(editingFinished()),	this,	SLOT(SetAllTrimbits()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetNumStoragecells(int value) {
#ifdef VERBOSE
    cout << "Setting number of stoarge cells to " << value << endl;
#endif
    myDet->setTimer(slsDetectorDefs::STORAGE_CELL_NUMBER,value);

    disconnect(spinNumStoragecells,SIGNAL(valueChanged(int)),this,   SLOT(SetNumStoragecells(int)));
    spinNumStoragecells->setValue((int)myDet->setTimer(slsDetectorDefs::STORAGE_CELL_NUMBER,-1));
    connect(spinNumStoragecells,SIGNAL(valueChanged(int)),   this,   SLOT(SetNumStoragecells(int)));

    qDefs::checkErrorMessage(myDet,"qTabAdvanced::SetNumStoragecells");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetSubExposureTime() {
	disconnect(spinSubExpTime,SIGNAL(valueChanged(double)),			this,	SLOT(SetSubExposureTime()));
	disconnect(comboSubExpTimeUnit,SIGNAL(currentIndexChanged(int)),this,	SLOT(SetSubExposureTime()));

	//Get the value of timer in ns
	double timeNS = qDefs::getNSTime(
			(qDefs::timeUnit)comboSubExpTimeUnit->currentIndex(),
			spinSubExpTime->value());

	// set value
#ifdef VERBOSE
	cout << "Setting sub frame acquisition time to " << timeNS << " clocks" <<
			"/" << spinSubExpTime->value() <<
			qDefs::getUnitString((qDefs::timeUnit)comboSubExpTimeUnit->currentIndex()) << endl;
#endif
	myDet->setTimer(slsDetectorDefs::SUBFRAME_ACQUISITION_TIME,(int64_t)timeNS);
	qDefs::checkErrorMessage(myDet,"qTabAdvanced::SetSubExposureTime");

	// update value in gui
	qDefs::timeUnit unit;
	double time = qDefs::getCorrectTime(unit,((double)(
			myDet->setTimer(slsDetectorDefs::SUBFRAME_ACQUISITION_TIME,-1)*(1E-9))));
	spinSubExpTime->setValue(time);
	comboSubExpTimeUnit->setCurrentIndex((int)unit);


	// highlight if period < exptime
	CheckAcqPeriodGreaterThanExp();

	connect(spinSubExpTime,SIGNAL(valueChanged(double)),			this,	SLOT(SetSubExposureTime()));
	connect(comboSubExpTimeUnit,SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetSubExposureTime()));


	qDefs::checkErrorMessage(myDet,"qTabAdvanced::SetSubExposureTime");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::SetSubPeriod() {
	disconnect(spinSubPeriod,SIGNAL(valueChanged(double)),		   this,	SLOT(SetSubPeriod()));
	disconnect(comboSubPeriodUnit,SIGNAL(currentIndexChanged(int)),this,	SLOT(SetSubPeriod()));

	//Get the value of timer in ns
	double timeNS = qDefs::getNSTime(
			(qDefs::timeUnit)comboSubPeriodUnit->currentIndex(),
			spinSubPeriod->value());

	// set value
#ifdef VERBOSE
	cout << "Setting sub frame period to " << timeNS << " clocks" <<
			"/" << spinSubPeriod->value() <<
			qDefs::getUnitString((qDefs::timeUnit)comboSubPeriodUnit->currentIndex()) << endl;
#endif
	myDet->setTimer(slsDetectorDefs::SUBFRAME_DEADTIME,(int64_t)timeNS);
	qDefs::checkErrorMessage(myDet,"qTabAdvanced::SetSubPeriod");

	// update value in gui
	qDefs::timeUnit unit;
	double time = qDefs::getCorrectTime(unit,((double)(
			myDet->setTimer(slsDetectorDefs::SUBFRAME_DEADTIME,-1)*(1E-9))));
	spinSubPeriod->setValue(time);
	comboSubPeriodUnit->setCurrentIndex((int)unit);

	// highlight if period < exptime
	CheckAcqPeriodGreaterThanExp();

	connect(spinSubPeriod,SIGNAL(valueChanged(double)),			this,	SLOT(SetSubPeriod()));
	connect(comboSubPeriodUnit,SIGNAL(currentIndexChanged(int)),this,	SLOT(SetSubPeriod()));


	qDefs::checkErrorMessage(myDet,"qTabAdvanced::SetSubPeriod");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::CheckAcqPeriodGreaterThanExp(){
	double exptimeNS = qDefs::getNSTime(
			(qDefs::timeUnit)comboSubExpTimeUnit->currentIndex(),
			spinSubExpTime->value());
	double acqtimeNS = qDefs::getNSTime(
			(qDefs::timeUnit)comboSubPeriodUnit->currentIndex(),
			spinSubPeriod->value());
	if(exptimeNS>acqtimeNS && acqtimeNS > 0) {
		spinSubPeriod->setToolTip(errSubPeriodTip);
		lblSubPeriod->setToolTip(errSubPeriodTip);
		lblSubPeriod->setPalette(red);
		lblSubPeriod->setText("Sub Frame Period:*");
	}
	else {
		spinSubPeriod->setToolTip(acqSubPeriodTip);
		lblSubPeriod->setToolTip(acqSubPeriodTip);
		lblSubPeriod->setPalette(lblSetAllTrimbits->palette());
		lblSubPeriod->setText("Sub Frame Period:");
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabAdvanced::Refresh(){


#ifdef VERBOSE
		cout  << endl << "**Updating Advanced Tab" << endl;
#endif

	//network
	auto module_id = comboDetector->currentIndex();
	qDefs::checkErrorMessage(myDet,"qTabAdvanced::Refresh");



#ifdef VERBOSE
		cout << "Getting Detector Ports" << endl;
#endif
	//disconnect
	disconnect(spinControlPort,	SIGNAL(valueChanged(int)),			this,	SLOT(SetControlPort(int)));
	disconnect(spinStopPort,	SIGNAL(valueChanged(int)),			this,	SLOT(SetStopPort(int)));
	disconnect(comboOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetOnline(int)));

	//so that updated status
	if(myDet->getOnline(module_id)==slsDetectorDefs::ONLINE_FLAG)
		myDet->checkOnline(module_id);
	comboOnline->setCurrentIndex(myDet->getOnline(module_id));
	spinControlPort->setValue(myDet->setControlPort(-1, module_id));
	spinStopPort->setValue(myDet->setStopPort(-1, module_id));

	//connect
	connect(spinControlPort,	SIGNAL(valueChanged(int)),			this,	SLOT(SetControlPort(int)));
	connect(spinStopPort,		SIGNAL(valueChanged(int)),			this,	SLOT(SetStopPort(int)));
	connect(comboOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetOnline(int)));


#ifdef VERBOSE
	cout << "Getting Receiver Network Information" << endl;
#endif
	//disconnect
	disconnect(spinTCPPort,			SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrTCPPort(int)));
	disconnect(spinUDPPort,			SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrUDPPort(int)));
	disconnect(spinZmqPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetCltZmqPort(int)));
	disconnect(spinZmqPort2,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrZmqPort(int)));
	disconnect(comboRxrOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetReceiverOnline(int)));
	disconnect(dispIP,				SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	disconnect(dispMAC,				SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	disconnect(dispUDPIP,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	disconnect(dispUDPMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	disconnect(btnRxr,				SIGNAL(clicked()),			this, SLOT(SetReceiver()));

	dispIP->setText(myDet->getDetectorIP(module_id).c_str());
	dispMAC->setText(myDet->getDetectorMAC(module_id).c_str());

	//so that updated status
	if(myDet->setReceiverOnline(slsDetectorDefs::GET_ONLINE_FLAG, module_id)==slsDetectorDefs::ONLINE_FLAG)
		myDet->checkReceiverOnline(module_id);
	comboRxrOnline->setCurrentIndex(myDet->setReceiverOnline(slsDetectorDefs::GET_ONLINE_FLAG, module_id));

	dispRxrHostname->setText(myDet->getReceiver(module_id).c_str());
	spinTCPPort->setValue(myDet->setReceiverPort(-1, module_id));
	spinUDPPort->setValue(myDet->getReceiverUDPPort(module_id));
	spinZmqPort->setValue(myDet->getClientStreamingPort(module_id));
	spinZmqPort2->setValue(myDet->getReceiverStreamingPort(module_id));

	dispUDPIP->setText(myDet->getReceiverUDPIP(module_id).c_str());
	dispUDPMAC->setText(myDet->getReceiverUDPMAC(module_id).c_str());

	//connect
	connect(spinTCPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrTCPPort(int)));
	connect(spinUDPPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrUDPPort(int)));
	connect(spinZmqPort,		SIGNAL(valueChanged(int)),	this,	SLOT(SetCltZmqPort(int)));
	connect(spinZmqPort2,		SIGNAL(valueChanged(int)),	this,	SLOT(SetRxrZmqPort(int)));
	connect(comboRxrOnline,		SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetReceiverOnline(int)));
	connect(dispIP,				SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	connect(dispMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	connect(dispUDPIP,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	connect(dispUDPMAC,			SIGNAL(editingFinished()),	this, SLOT(SetNetworkParameters()));
	connect(btnRxr,				SIGNAL(clicked()),			this, SLOT(SetReceiver()));

	// zmq parameters
	disconnect(dispZMQIP,			SIGNAL(editingFinished()),	this, SLOT(SetClientZMQIP()));
	dispZMQIP->setText(myDet->getClientStreamingIP(module_id).c_str());
	connect(dispZMQIP,			SIGNAL(editingFinished()),	this, SLOT(SetClientZMQIP()));

	disconnect(dispZMQIP2,			SIGNAL(editingFinished()),	this, SLOT(SetReceiverZMQIP()));
	dispZMQIP2->setText(myDet->getReceiverStreamingIP(module_id).c_str());
	connect(dispZMQIP2,			SIGNAL(editingFinished()),	this, SLOT(SetReceiverZMQIP()));

	//highlight in red if detector or receiver is offline
	if(!comboOnline->currentIndex()){
		comboOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setToolTip(detOnlineTip + errOnlineTip);
		lblOnline->setPalette(red);
		lblOnline->setText("Online:*");
	}else{
		comboOnline->setToolTip(detOnlineTip);
		lblOnline->setToolTip(detOnlineTip);
		lblOnline->setPalette(lblHostname->palette());
		lblOnline->setText("Online:");
	}
	if(comboRxrOnline->isEnabled()){
		if(!comboRxrOnline->currentIndex()){
			comboRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
			lblRxrOnline->setToolTip(rxrOnlineTip + errOnlineTip);
			lblRxrOnline->setPalette(red);
			lblRxrOnline->setText("Online:*");
		}else{
			comboRxrOnline->setToolTip(rxrOnlineTip);
			lblRxrOnline->setToolTip(rxrOnlineTip);
			lblRxrOnline->setPalette(lblHostname->palette());
			lblRxrOnline->setText("Online:");
		}
	}

	//roi
#ifdef VERBOSE
		cout << "Getting ROI" << endl;
#endif
		if (detType == slsDetectorDefs::GOTTHARD)
			updateROIList();

	//update alltirmbits from server
	if(boxSetAllTrimbits->isEnabled())
		updateAllTrimbitsFromServer();

	// storage cells
	if (detType == slsDetectorDefs::JUNGFRAU) {
	    disconnect(spinNumStoragecells,SIGNAL(valueChanged(int)),this,   SLOT(SetNumStoragecells(int)));
	    spinNumStoragecells->setValue((int)myDet->setTimer(slsDetectorDefs::STORAGE_CELL_NUMBER,-1));
	    connect(spinNumStoragecells,SIGNAL(valueChanged(int)),   this,   SLOT(SetNumStoragecells(int)));
	}

	// sub exptime and sub period
	else if (detType == slsDetectorDefs::EIGER) {
		disconnect(spinSubExpTime,SIGNAL(valueChanged(double)),			this,	SLOT(SetSubExposureTime()));
		disconnect(comboSubExpTimeUnit,SIGNAL(currentIndexChanged(int)),this,	SLOT(SetSubExposureTime()));
		disconnect(spinSubPeriod,SIGNAL(valueChanged(double)),		   this,	SLOT(SetSubPeriod()));
		disconnect(comboSubPeriodUnit,SIGNAL(currentIndexChanged(int)),this,	SLOT(SetSubPeriod()));

#ifdef VERBOSE
		cout  << "Getting Sub Exposure time and Sub Period" << endl;
#endif
		// subexptime
		qDefs::timeUnit unit;
		double time = qDefs::getCorrectTime(unit,((double)(
				myDet->setTimer(slsDetectorDefs::SUBFRAME_ACQUISITION_TIME,-1)*(1E-9))));
		spinSubExpTime->setValue(time);
		comboSubExpTimeUnit->setCurrentIndex((int)unit);

		// subperiod
		time = qDefs::getCorrectTime(unit,((double)(myDet->setTimer(slsDetectorDefs::SUBFRAME_DEADTIME,-1)*(1E-9))));
		spinSubPeriod->setValue(time);
		comboSubPeriodUnit->setCurrentIndex((int)unit);


		// highlight if period < exptime
		CheckAcqPeriodGreaterThanExp();

		connect(spinSubExpTime,SIGNAL(valueChanged(double)),			this,	SLOT(SetSubExposureTime()));
		connect(comboSubExpTimeUnit,SIGNAL(currentIndexChanged(int)),	this,	SLOT(SetSubExposureTime()));
		connect(spinSubPeriod,SIGNAL(valueChanged(double)),			this,	SLOT(SetSubPeriod()));
		connect(comboSubPeriodUnit,SIGNAL(currentIndexChanged(int)),this,	SLOT(SetSubPeriod()));
	}

#ifdef VERBOSE
		cout  << "**Updated Advanced Tab" << endl << endl;
#endif

	qDefs::checkErrorMessage(myDet, module_id, "qTabAdvanced::Refresh");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

