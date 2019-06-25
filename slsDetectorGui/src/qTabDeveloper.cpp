#include "qTabDeveloper.h"
#include "qDefs.h"

#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QTimer>
#include <QString>
#include <QPalette>
#include <QDoubleValidator>
#include <QSpacerItem>
#include <QString>

#include <iostream>


qTabDeveloper::qTabDeveloper(QWidget *parent, multiSlsDetector *detector) : 
	QWidget(parent), myDet(detector), detType(slsDetectorDefs::GENERIC), numDACWidgets(0), numADCWidgets(0),
	boxDacs(nullptr), boxAdcs(nullptr), lblHV(nullptr), comboHV(nullptr), spinHV(nullptr), dacLayout(nullptr), comboDetector(nullptr), layout(nullptr) {
	SetupWidgetWindow();
	Initialization();
	FILE_LOG(logDEBUG) << "Developer ready";
}

qTabDeveloper::~qTabDeveloper() {
	for (size_t i = 0; i < lblDacs.size(); ++i) {
		delete lblDacs[i];
		delete lblDacsmV[i];
		delete spinDacs[i];
	}
	for (size_t i = 0; i < lblAdcs.size(); ++i) {
		delete lblAdcs[i];
		delete spinAdcs[i];
	}
	if (boxDacs)
		delete boxDacs;
	if (boxAdcs)
		delete boxAdcs;
	if (lblHV)
		delete lblHV;
	if (comboHV)
		delete comboHV;	
	if (spinHV)
		delete spinHV;
	if (dacLayout)
		delete dacLayout;
	if (comboDetector)
		delete comboDetector;
	if (layout)
		delete layout;
}

void qTabDeveloper::SetupWidgetWindow() {
	detType = myDet->getDetectorTypeAsEnum();
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
		break;
	}

	setFixedWidth(765);
	setFixedHeight(20 + 50 + (numDACWidgets / 2) * 35);
	QScrollArea* scroll = new QScrollArea;
	scroll->setWidget(this);
	scroll->setWidgetResizable(true);
	layout = new QGridLayout(scroll);
	layout->setContentsMargins(20, 10, 10, 5);
	setLayout(layout);

	comboDetector = new QComboBox(this);
	PopulateDetectors();

	CreateDACWidgets();
	CreateHVWidget();
	layout->addWidget(comboDetector, 0, 0);
	layout->addWidget(boxDacs, 1, 0);
	CreateADCWidgets();

	Refresh();
}


void qTabDeveloper::Initialization() {
	connect(comboDetector, SIGNAL(currentIndexChanged(int)), this, SLOT(Refresh()));
	for (int i = 0; i < numDACWidgets; ++i)
		connect(spinDacs[i], SIGNAL(editingFinished(int)), this, SLOT(SetDac(int)));
	if (comboHV != nullptr) {
		connect(comboHV, SIGNAL(currentIndexChanged(int)), this, SLOT(SetHighVoltage()));
	} else {
		connect(spinHV, SIGNAL(valueChanged(int)), this, SLOT(SetHighVoltage()));
	}
}

void qTabDeveloper::PopulateDetectors() {
	FILE_LOG(logDEBUG) << "Populating detectors";

	comboDetector->clear();
	comboDetector->addItem("All");
	if (myDet->getNumberOfDetectors() > 1) {
		for (int i = 0; i < myDet->getNumberOfDetectors(); ++i)
			comboDetector->addItem(QString(myDet->getHostname(i).c_str()));
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
	if (!numADCWidgets)
		return;

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

	layout->addWidget(boxAdcs, 2, 0);
}

void qTabDeveloper::CreateHVWidget() {

	switch(detType) {
		case slsDetectorDefs::GOTTHARD:
		case slsDetectorDefs::JUNGFRAU:
		case slsDetectorDefs::MOENCH:
			break;
		default:
			return;
	}

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
		comboHV->setToolTip("<nobr>Set high voltage to 0, 90, 110, 120, 150 or 200V.</nobr>");
		lblHV->setToolTip(comboHV->toolTip());
		dacLayout->addWidget(comboHV, (int)(numDACWidgets / 2), 2);
	}

	// jungfrau, moench (range of values)
	else  {
		spinHV = new QSpinBox(boxDacs);
		spinDacs[numDACWidgets]->setMinimum(-1);
		spinDacs[numDACWidgets]->setMaximum(HV_MAX);
		spinHV->setToolTip("<nobr>Set high voltage to 0 or 60 - 200V</nobr>");
		lblHV->setToolTip(spinHV->toolTip());
		dacLayout->addWidget(spinHV, (int)(numDACWidgets / 2), 2);
	}
}

void qTabDeveloper::GetDac(int id) {
	FILE_LOG(logDEBUG) << "Getting Dac " << id;

	disconnect(spinDacs[id], SIGNAL(editingFinished(int)), this, SLOT(SetDac(int)));
	try {
		// dac units
		auto retval = myDet->setDAC(-1, getSLSIndex(id), 0, comboDetector->currentIndex() - 1);
		spinDacs[id]->setValue(retval);
		// mv
		retval = myDet->setDAC(-1, getSLSIndex(id), 1, comboDetector->currentIndex() - 1);
		lblDacsmV[id]->setText(QString("%1mV").arg(retval -10));
    } CATCH_DISPLAY("Could not get dac.", "qTabDeveloper::GetDac")

	connect(spinDacs[id], SIGNAL(editingFinished(int)), this, SLOT(SetDac(int)));
}

void qTabDeveloper::GetDacs() {
	FILE_LOG(logDEBUG) << "Getting All Dacs";

	for (int i = 0; i < numDACWidgets; ++i) {
		GetDac(i);
	}
}

void qTabDeveloper::SetDac(int id) {
	int val = spinDacs[id]->value();
	FILE_LOG(logINFO) << "Setting dac:" << dacNames[id] << " : " << val;

	try {
		myDet->setDAC(val, getSLSIndex(id), 0, comboDetector->currentIndex() - 1);
    } CATCH_DISPLAY ("Could not set dac.", "qTabDeveloper::SetDac")
	
	// update mV anyway
    GetDac(id);
}

void qTabDeveloper::GetAdcs() {
	FILE_LOG(logDEBUG) << "Getting ADCs";

	auto moduleId = comboDetector->currentIndex() - 1;
	for (int i = 0; i < numADCWidgets; ++i) {
		try {
			auto retval = myDet->getADC(getSLSIndex(i + numDACWidgets), moduleId);
			if (retval == -1 && moduleId == -1) {
				spinAdcs[i]->setText(QString("Different values"));
			} else {
				if (detType == slsDetectorDefs::EIGER || detType == slsDetectorDefs::JUNGFRAU) {
					retval /= 1000.00;
				}
				spinAdcs[i]->setText(QString::number(retval, 'f', 2) + 0x00b0 + QString("C"));
			}
		} CATCH_DISPLAY ("Could not get adcs.", "qTabDeveloper::GetAdcs")
	}
}

void qTabDeveloper::GetHighVoltage() {
	// not enabled for this detector type
	if (comboHV == nullptr && spinHV == nullptr)
		return;

	FILE_LOG(logDEBUG) << "Getting High Voltage";
	if (comboHV == nullptr) {
		disconnect(spinHV, SIGNAL(valueChanged(int)), this, SLOT(SetHighVoltage()));	
	} else {
		disconnect(comboHV, SIGNAL(currentIndexChanged(int)), this, SLOT(SetHighVoltage()));
	}

	try {
		// dac units
		auto retval = myDet->setDAC(-1, slsDetectorDefs::HIGH_VOLTAGE, 0, comboDetector->currentIndex() - 1);
		if (spinHV != nullptr) {
			if (retval != 0 && retval != -1 && retval < HV_MIN &&  retval > HV_MAX) {
				qDefs::Message(qDefs::WARNING, std::string("Unknown High Voltage: ") + std::to_string(retval), "qTabDeveloper::GetHighVoltage");
			} else{
				spinHV->setValue(retval);	
			}
		} else {
			switch (retval) {
			case -1:
				qDefs::Message(qDefs::WARNING, "Different values for high voltage.", "qTabDeveloper::GetHighVoltage");
				break;
			case 0:
				comboHV->setCurrentIndex(HV_0);
				break;
			case 90:
				comboHV->setCurrentIndex(HV_90);
				break;
			case 110:
				comboHV->setCurrentIndex(HV_110);
				break;
			case 120:
				comboHV->setCurrentIndex(HV_120);
				break;
			case 150:
				comboHV->setCurrentIndex(HV_150);
				break;
			case 180:
				comboHV->setCurrentIndex(HV_180);
				break;
			case 200:
				comboHV->setCurrentIndex(HV_200);
				break;
			default:
				qDefs::Message(qDefs::WARNING, std::string("Unknown High Voltage: ") + std::to_string(retval), "qTabDeveloper::GetHighVoltage");
				break;
			}
		}

    } CATCH_DISPLAY ("Could not get high voltage.", "qTabDeveloper::GetHighVoltage")

	if (comboHV == nullptr) {
		connect(spinHV, SIGNAL(valueChanged(int)), this, SLOT(SetHighVoltage()));	
	} else {
		connect(comboHV, SIGNAL(currentIndexChanged(int)), this, SLOT(SetHighVoltage()));
	}
}

void qTabDeveloper::SetHighVoltage() {
	int val = (comboHV ? comboHV->currentText().toInt() : spinHV->value());
	FILE_LOG(logINFO) << "Setting high voltage:" << val;
	
	try {
        myDet->setDAC(val, slsDetectorDefs::HIGH_VOLTAGE, 0, comboDetector->currentIndex() - 1);
    } CATCH_HANDLE ("Could not set high voltage.", "qTabDeveloper::SetHighVoltage", 
					this, &qTabDeveloper::GetHighVoltage)
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
			throw sls::RuntimeError(std::string("Unknown dac/adc index") + std::to_string(index));
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
			throw sls::RuntimeError(std::string("Unknown dac/adc index") + std::to_string(index));
		}
		break;

	case slsDetectorDefs::JUNGFRAU:
		if (index >= 0 && index < numDACWidgets) {
			return (slsDetectorDefs::dacIndex)index;
		}
		if (index == numDACWidgets) {
			return slsDetectorDefs::TEMPERATURE_ADC;
		} else {
			throw sls::RuntimeError(std::string("Unknown dac/adc index") + std::to_string(index));
		}
		break;

	case slsDetectorDefs::MOENCH:
		if (index >= 0 && index < numDACWidgets) {
			return (slsDetectorDefs::dacIndex)index;
		} else {
			throw sls::RuntimeError(std::string("Unknown dac/adc index") + std::to_string(index));
		}
		break;

	default:
		break;
	}

	return (slsDetectorDefs::dacIndex)0;
}

void qTabDeveloper::Refresh() {
	FILE_LOG(logDEBUG) << "**Updating Developer Tab\n";
	GetDacs();
	GetAdcs();
	GetHighVoltage();
	FILE_LOG(logDEBUG) << "**Updated Developer Tab";
}

