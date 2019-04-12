#include "qTabDeveloper.h"
#include "qDetectorMain.h"

#include "multiSlsDetector.h"

#include <QDoubleValidator>
#include <QSpacerItem>
#include <QString>

#include <iostream>


qTabDeveloper::qTabDeveloper(QWidget *parent, multiSlsDetector *detector) : QWidget(parent),
		myDet(detector),
		detType(slsDetectorDefs::GENERIC),
		numDACWidgets(0),
		numADCWidgets(0),
		boxDacs(0),
		boxAdcs(0),
		lblHV(0),
		comboHV(0),
		dacLayout(0),
		comboDetector(0) {


	lblDacs.clear();
	lblAdcs.clear();
	spinDacs.clear();
	spinAdcs.clear();
	lblDacsmV.clear();

	SetupWidgetWindow();
	Initialization();
	FILE_LOG(logDEBUG) << "Developer ready";
}


qTabDeveloper::~qTabDeveloper() {
	if (myDet)
		delete myDet;
}

void qTabDeveloper::SetupWidgetWindow() {
	//Detector Type
	detType = myDet->getDetectorTypeAsEnum();

	//palette
	red = QPalette();
	red.setColor(QPalette::Active, QPalette::WindowText, Qt::red);

	//the number of dacs and adcs
	switch (detType) {
	case slsDetectorDefs::EIGER:
		numDACWidgets = 17;
		numADCWidgets = 6;

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

	case slsDetectorDefs::GOTTHARD:
		numDACWidgets = 8;
		numADCWidgets = 2;
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

	case slsDetectorDefs::JUNGFRAU:
		numDACWidgets = 8;
		numADCWidgets = 1;
		dacNames.push_back("v vb comp:");
		dacNames.push_back("v vdd prot:");
		dacNames.push_back("v vin com:");
		dacNames.push_back("v vref prech:");
		dacNames.push_back("v vb pixbuf:");
		dacNames.push_back("v vb ds:");
		dacNames.push_back("v vref ds:");
		dacNames.push_back("i vref comp:");

		adcNames.push_back("Temperature ADC/FPGA:");

		break;

	case slsDetectorDefs::MOENCH:

		numDACWidgets = 8;
		numADCWidgets = 0;
		dacNames.push_back("v Dac 0:");
		dacNames.push_back("v Dac 1:");
		dacNames.push_back("v Dac 2:");
		dacNames.push_back("v Dac 3:");
		dacNames.push_back("v Dac 4:");
		dacNames.push_back("v Dac 5:");
		dacNames.push_back("v Dac 6:");
		dacNames.push_back("i Dac 7:");

		break;

	default:
		FILE_LOG(logERROR) << "Unknown detector type: " + myDet->getDetectorTypeAsString();
		qDefs::Message(qDefs::CRITICAL, std::string("Unknown detector type:") + myDet->getDetectorTypeAsString(), "qTabDeveloper::SetupWidgetWindow");
		exit(-1);
		break;
	}

	//layout
	setFixedWidth(765);
	setFixedHeight(20 + 50 + (numDACWidgets / 2) * 35);

	QScrollArea* scroll = new QScrollArea;
	scroll->setWidget(this);
	scroll->setWidgetResizable(true);

	QGridLayout *layout = new QGridLayout(scroll);
	layout->setContentsMargins(20, 10, 10, 5);
	setLayout(layout);

	//readout
	comboDetector = new QComboBox(this);
	comboDetector->addItem("All");
	//add detectors
	for (int i = 1; i < myDet->getNumberOfDetectors() + 1; ++i)
		comboDetector->addItem(QString(myDet->getHostname(i - 1).c_str()));
	comboDetector->setCurrentIndex(0);

	//dacs
	CreateDACWidgets();

	// hv for gotthard, jungfrau, moench
	if ((detType == slsDetectorDefs::GOTTHARD) ||
			(detType == slsDetectorDefs::JUNGFRAU) ||
			(detType == slsDetectorDefs::MOENCH)) {
		CreateHVWidget();

	}
	layout->addWidget(comboDetector, 0, 0);
	layout->addWidget(boxDacs, 1, 0);

	//adcs
	if (numADCWidgets) {
		CreateADCWidgets();
		layout->addWidget(boxAdcs, 2, 0);
	}

	qDefs::checkErrorMessage(myDet, "qTabDeveloper::SetupWidgetWindow");
}


void qTabDeveloper::Initialization() {
	connect(comboDetector, SIGNAL(currentIndexChanged(int)), this, SLOT(Refresh()));

	// dacs
	for (int i = 0; i < numDACWidgets; ++i)
		connect(spinDacs[i], SIGNAL(editingFinished(int)), this, SLOT(SetDacValues(int)));

	// hv
	if (comboHV) {
		connect(comboHV, SIGNAL(currentIndexChanged(int)), this, SLOT(SetHighVoltage()));
	} else {
		connect(spinHV, SIGNAL(valueChanged(int)), this, SLOT(SetHighVoltage()));
	}
}


void qTabDeveloper::CreateDACWidgets() {
	boxDacs = new QGroupBox("Dacs", this);
	boxDacs->setFixedHeight(25 + (numDACWidgets / 2) * 35);
	dacLayout = new QGridLayout(boxDacs);

	for (int i = 0; i < numDACWidgets; ++i) {
		lblDacs[i] = new QLabel(QString(dacNames[i].c_str()), boxDacs);
		spinDacs[i] = new MyDoubleSpinBox(i, boxDacs);
		spinDacs[i]->setMinimum(-1);
		spinDacs[i]->setMaximum(10000);
		lblDacsmV[i] = new QLabel("", boxDacs);

		dacLayout->addWidget(lblDacs[i], (int)(i / 2), ((i % 2) == 0) ? 1 : 5);
		dacLayout->addWidget(spinDacs[i], (int)(i / 2), ((i % 2) == 0) ? 2 : 6);
		dacLayout->addWidget(lblDacsmV[i], (int)(i / 2), ((i % 2) == 0) ? 3 : 7);
		if (!(i % 2)) {
			dacLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed), (int)(i / 2), 0);
			dacLayout->addItem(new QSpacerItem(60, 20, QSizePolicy::Fixed, QSizePolicy::Fixed), (int)(i / 2), 4);
			dacLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed), (int)(i / 2), 8);
		}
	}
}


void qTabDeveloper::CreateADCWidgets() {
	int rows = numADCWidgets / 2;
	if (numADCWidgets % 2)
		rows++;
	setFixedHeight(20 + (50 + (numDACWidgets / 2) * 35) + (50 + rows * 35));
	boxAdcs = new QGroupBox("ADCs", this);
	boxAdcs->setFixedHeight(25 + rows * 35);

	QGridLayout *adcLayout = new QGridLayout(boxAdcs);

	for (int i = 0; i < numADCWidgets; ++i) {
		lblAdcs[i] = new QLabel(QString(adcNames[i].c_str()), boxAdcs);
		spinAdcs[i] = new QLineEdit(boxAdcs);
		spinAdcs[i]->setReadOnly(true);

		adcLayout->addWidget(lblAdcs[i], (int)(i / 2), ((i % 2) == 0) ? 1 : 4);
		adcLayout->addWidget(spinAdcs[i], (int)(i / 2), ((i % 2) == 0) ? 2 : 5);
		if (!(i % 2)) {
			adcLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed), (int)(i / 2), 0);
			adcLayout->addItem(new QSpacerItem(60, 20, QSizePolicy::Fixed, QSizePolicy::Fixed), (int)(i / 2), 3);
			adcLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed), (int)(i / 2), 6);
		}
	}

	//to make the adcs at the bottom most
	if (detType != slsDetectorDefs::EIGER) {
		int diff = 340 - height();
		setFixedHeight(340);
		layout->setVerticalSpacing(diff / 2);
	}
}

void qTabDeveloper::CreateHVWidget() {
	boxDacs->setFixedHeight(boxDacs->height() + 35);
	lblHV = new QLabel("High Voltage", boxDacs);
	dacLayout->addWidget(lblHV, (int)(numDACWidgets / 2), 1);

	comboHV = nullptr;
	spinHV = nullptr;

	// drop down with specific values
	if (detType == slsDetectorDefs::GOTTHARD) {
		comboHV = new QComboBox(boxDacs);
		comboHV->addItem("0");
		comboHV->addItem("90");
		comboHV->addItem("110");
		comboHV->addItem("120");
		comboHV->addItem("150");
		comboHV->addItem("180");
		comboHV->addItem("200");
		tipHV = "<nobr>Set high voltage to 0, 90, 110, 120, 150 or 200V.</nobr>";
		comboHV->setToolTip(tipHV);
		dacLayout->addWidget(comboHV, (int)(numDACWidgets / 2), 2);
	}

	// jungfrau, moench (range of values)
	else  {
		spinHV = new QSpinBox(boxDacs);
		spinDacs[i]->setMinimum(-1);
		spinDacs[i]->setMaximum(200);
		tipHV = "<nobr>Set high voltage to 0 or 60 - 200V</nobr>";
		spinHV->setToolTip(tipHV);
		dacLayout->addWidget(spinHV, (int)(numDACWidgets / 2), 2);
	}
	lblHV->setToolTip(tipHV);
}


void qTabDeveloper::SetDacValues(int id) {
	FILE_LOG(logINFO) << "Setting dac:" << dacNames[id] << " : " << spinDacs[id]->value();

	int moduleId = comboDetector->currentIndex() - 1;

	myDet->setDAC(spinDacs[id]->value(),getSLSIndex(id), 0, moduleId);
	lblDacsmV[id]->setText(QString("%1mV").arg(myDet->setDAC(-1, getSLSIndex(id), 1, moduleId),-10));
	qDefs::checkErrorMessage(myDet, moduleId, "qTabDeveloper::SetDacValues");
}


void qTabDeveloper::SetHighVoltage() {
	int highvoltage = (comboHV ? comboHV->currentText().toInt() : spinHV->value());
	FILE_LOG(logINFO) << "Setting high voltage:" << highvoltage;

	auto moduleId = comboDetector->currentIndex() - 1;
	int ret = det->setDAC(highvoltage,slsDetectorDefs::HIGH_VOLTAGE, 0, moduleId);
	qDefs::checkErrorMessage(myDet, moduleId, "qTabDeveloper::SetHighVoltage");

	//error
	if (ret != highvoltage && highvoltage != -1) {
		qDefs::Message(qDefs::CRITICAL, "High Voltage could not be set to this value.", "qTabDeveloper::SetHighVoltage");
		FILE_LOG(logERROR) << "Could not set High voltage";
		lblHV->setPalette(red);
		lblHV->setText("High Voltage:*");
		QString errTip = tipHV + QString("<br><br><font color=\"red\"><nobr>High Voltage could not be set. The return value is ") +
				QString::number(ret) + QString("</nobr></font>");
		lblHV->setToolTip(errTip);
		if (comboHV)
			comboHV->setToolTip(errTip);
		else
			spinHV->setToolTip(errTip);
	} else {
		lblHV->setPalette(lblDacs[0]->palette());
		lblHV->setText("High Voltage:");
		lblHV->setToolTip(tipHV);
		if (comboHV)
			comboHV->setToolTip(tipHV);
		else
			spinHV->setToolTip(errTip);
	}
}


slsDetectorDefs::dacIndex qTabDeveloper::getSLSIndex(int index) {
	switch (detType) {

	case slsDetectorDefs::EIGER:
		switch (index) {
		case 0:
			return slsDetectorDefs::E_SvP;
		case 1:
			return slsDetectorDefs::E_SvN;
		case 2:
			return slsDetectorDefs::E_Vrf;
		case 3:
			return slsDetectorDefs::E_Vrs;
		case 4:
			return slsDetectorDefs::E_Vtr;
		case 5:
			return slsDetectorDefs::E_Vtgstv;
		case 6:
			return slsDetectorDefs::E_cal;
		case 7:
			return slsDetectorDefs::E_Vcp;
		case 8:
			return slsDetectorDefs::E_Vcn;
		case 9:
			return slsDetectorDefs::E_Vis;
		case 10:
			return slsDetectorDefs::E_rxb_lb;
		case 11:
			return slsDetectorDefs::E_rxb_rb;
		case 12:
			return slsDetectorDefs::E_Vcmp_ll;
		case 13:
			return slsDetectorDefs::E_Vcmp_lr;
		case 14:
			return slsDetectorDefs::E_Vcmp_rl;
		case 15:
			return slsDetectorDefs::E_Vcmp_rr;
		case 16:
			return slsDetectorDefs::THRESHOLD;
		case 17:
			return slsDetectorDefs::TEMPERATURE_FPGAEXT;
		case 18:
			return slsDetectorDefs::TEMPERATURE_10GE;
		case 19:
			return slsDetectorDefs::TEMPERATURE_DCDC;
		case 20:
			return slsDetectorDefs::TEMPERATURE_SODL;
		case 21:
			return slsDetectorDefs::TEMPERATURE_SODR;
		case 22:
			return slsDetectorDefs::TEMPERATURE_FPGA;
		default:
			qDefs::Message(qDefs::CRITICAL, "Unknown DAC/ADC Index. Weird Error Index:" + index, "qTabDeveloper::getSLSIndex");
			Refresh();
			break;
		}
		break;
	case slsDetectorDefs::GOTTHARD:
		switch (index) {
		case 0:
			return slsDetectorDefs::G_VREF_DS;
		case 1:
			return slsDetectorDefs::G_VCASCN_PB;
		case 2:
			return slsDetectorDefs::G_VCASCP_PB;
		case 3:
			return slsDetectorDefs::G_VOUT_CM;
		case 4:
			return slsDetectorDefs::G_VCASC_OUT;
		case 5:
			return slsDetectorDefs::G_VIN_CM;
		case 6:
			return slsDetectorDefs::G_VREF_COMP;
		case 7:
			return slsDetectorDefs::G_IB_TESTC;
		case 8:
			return slsDetectorDefs::TEMPERATURE_ADC;
		case 9:
			return slsDetectorDefs::TEMPERATURE_FPGA;
		default:
			qDefs::Message(qDefs::CRITICAL, "Unknown DAC/ADC Index. Weird Error Index:" + index, "qTabDeveloper::getSLSIndex");
			Refresh();
			break;
		}
		break;

	case slsDetectorDefs::JUNGFRAU:
		if (index >= 0 && index <= 7) {
			return (slsDetectorDefs::dacIndex)index;
		}
		if (index == 8) {
			return slsDetectorDefs::TEMPERATURE_ADC;
		} else {
			qDefs::Message(qDefs::CRITICAL, "Unknown DAC/ADC Index. Weird Error Index:" + index, "qTabDeveloper::getSLSIndex");
			Refresh();
		}
		break;
	case slsDetectorDefs::MOENCH:
		if (index >= 0 && index <= 7) {
			return (slsDetectorDefs::dacIndex)index;
		} else {
			qDefs::Message(qDefs::CRITICAL, "Unknown DAC/ADC Index. Weird Error Index:" + index, "qTabDeveloper::getSLSIndex");
			Refresh();
		}
		break;

	default:
		FILE_LOG(logERROR) << "Unknown detector type:" + myDet->getDetectorTypeAsString();
		qDefs::Message(qDefs::CRITICAL, std::string("Unknown detector type:") + myDet->getDetectorTypeAsString(), "qTabDeveloper::getSLSIndex");
		qDefs::checkErrorMessage(myDet, "qTabDeveloper::getSLSIndex");
		exit(-1);
		break;
	}
	return (slsDetectorDefs::dacIndex)0;
}


void qTabDeveloper::RefreshAdcs() {
	FILE_LOG(logDEBUG) << "Updating ADCs";

	auto moduleId = comboDetector->currentIndex() - 1;

	for (int i = 0; i < numADCWidgets; ++i) {

		double value = (double)myDet->getADC(getSLSIndex(i + numDACWidgets), moduleId);
		if (value == -1 && moduleId == -1) {
			spinAdcs[i]->setText(QString("Different values"));
		} else {
			if (detType == slsDetectorDefs::EIGER || detType == slsDetectorDefs::JUNGFRAU)
				value /= 1000.00;
			spinAdcs[i]->setText(QString::number(value, 'f', 2) + 0x00b0 + QString("C"));
		}
	}

	qDefs::checkErrorMessage(myDet, "qTabDeveloper::RefreshAdcs");
}


void qTabDeveloper::Refresh() {
	FILE_LOG(logDEBUG) << "**Updating Developer Tab\n";

	auto moduleId = comboDetector->currentIndex() - 1;

	// dacs
	FILE_LOG(logDEBUG) << "Getting DACs";
	for (int i = 0; i < numDACWidgets; ++i) {
		spinDacs[i]->setValue(myDet->setDAC(-1, getSLSIndex(i), 0, moduleId));
		lblDacsmV[i]->setText(QString("%1mV").arg(myDet->setDAC(-1, getSLSIndex(i), 1, moduleId), -10));
	}

	//adcs
	if (numADCWidgets)
		RefreshAdcs();

	//gotthard -high voltage
	if ((detType == slsDetectorDefs::GOTTHARD) ||
			(detType == slsDetectorDefs::JUNGFRAU) ||
			(detType == slsDetectorDefs::MOENCH)) {

		if (comboHV)
			disconnect(comboHV, SIGNAL(currentIndexChanged(int)), this, SLOT(SetHighVoltage()));
		else
			disconnect(spinHV, SIGNAL(valueChanged(int)), this, SLOT(SetHighVoltage()));

		//default should be correct
		lblHV->setPalette(lblDacs[0]->palette());
		lblHV->setText("High Voltage:");
		lblHV->setToolTip(tipHV);
		if (comboHV)
			comboHV->setToolTip(tipHV);
		else
			spinHV->setToolTip(tipHV);

		//getting hv value
		int ret = myDet->setDAC(-1, slsDetectorDefs::HIGH_VOLTAGE, 0, moduleId);

		bool error = false;
		if (spinHV) {
			if (ret != 0 && ret < 60 &&  ret > 200)
				error = true;
			else
				spinHV->setValue(ret);
		} else  {
			switch (ret) {
			case 0:
				comboHV->setCurrentIndex(0);
				break;
			case 90:
				comboHV->setCurrentIndex(1);
				break;
			case 110:
				comboHV->setCurrentIndex(2);
				break;
			case 120:
				comboHV->setCurrentIndex(3);
				break;
			case 150:
				comboHV->setCurrentIndex(4);
				break;
			case 180:
				comboHV->setCurrentIndex(5);
				break;
			case 200:
				comboHV->setCurrentIndex(6);
				break;
			default:
				comboHV->setCurrentIndex(0); //error
				error = true;
				break;
			}
		}

		if (error) {
			lblHV->setPalette(red);
			lblHV->setText("High Voltage:*");
			QString errTip = tipHV + QString("<br><br><font color=\"red\"><nobr>High Voltage could not be set. The return value is ") +
					QString::number(ret) + QString("</nobr></font>");
			lblHV->setToolTip(errTip);
			if (comboHV)
				comboHV->setToolTip(errTip);
			else
				spinHV->setToolTip(errTip);
		} else {
			lblHV->setPalette(lblDacs[0]->palette());
			lblHV->setText("High Voltage:");
			lblHV->setToolTip(tipHV);
			if (comboHV)
				comboHV->setToolTip(tipHV);
			else
				spinHV->setToolTip(errTip);
		}
		if (comboHV)
			connect(comboHV, SIGNAL(currentIndexChanged(int)), this, SLOT(SetHighVoltage()));
		else
			connect(spinHV, SIGNAL(valueChanged(int)), this, SLOT(SetHighVoltage()));
	}

	FILE_LOG(logDEBUG) << "**Updated Developer Tab";

	qDefs::checkErrorMessage(myDet, "qTabDeveloper::Refresh");
}

