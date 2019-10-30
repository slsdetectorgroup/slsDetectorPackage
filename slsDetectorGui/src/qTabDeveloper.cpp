#include "qTabDeveloper.h"
#include "qDefs.h"
#include "qDacWidget.h"

#include <iostream>

qTabDeveloper::qTabDeveloper(QWidget *parent, sls::Detector *detector) : QWidget(parent), det(detector) {
	setupUi(this);
	SetupWidgetWindow();
	FILE_LOG(logDEBUG) << "Developer ready";
}

qTabDeveloper::~qTabDeveloper() {}

void qTabDeveloper::SetupWidgetWindow() {
	int tempid = 0;

	comboHV->hide();
	lblComboHV->hide();
	lblSpinHV->hide();
	spinHV->hide();

	try{
		slsDetectorDefs::detectorType detType = det->getDetectorType().squash();
		switch (detType) {
		case slsDetectorDefs::EIGER:
			dacWidgets.push_back(new qDacWidget(this, det, true, "v SvP: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v SvN ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Vrf: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Vrs: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Vtr: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Vtgstv: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v cal: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Vcp ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Vcn: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Vis: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v rxb_lb: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v rxb_rb: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Vcmp_ll: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Vcmp_lr: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Vcmp_rl: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Vcmp_rr: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v threshold: ", getSLSIndex(detType, tempid++), false));
			adcWidgets.push_back(new qDacWidget(this, det, false, "Temperature FPGA Ext: ", getSLSIndex(detType, tempid++), true));
			adcWidgets.push_back(new qDacWidget(this, det, false, "Temperature 10GE: ", getSLSIndex(detType, tempid++), true));
			adcWidgets.push_back(new qDacWidget(this, det, false, "Temperature DCDC: ", getSLSIndex(detType, tempid++), true));
			adcWidgets.push_back(new qDacWidget(this, det, false, "Temperature SODL: ", getSLSIndex(detType, tempid++), true));
			adcWidgets.push_back(new qDacWidget(this, det, false, "Temperature SODR: ", getSLSIndex(detType, tempid++), true));
			adcWidgets.push_back(new qDacWidget(this, det, false, "Temperature FPGA: ", getSLSIndex(detType, tempid++), true));
			break;

		case slsDetectorDefs::GOTTHARD:
			comboHV->show();
			lblComboHV->show();
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Reference: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Cascode n: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Cascode p: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Comp. Output: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Cascode out ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Comp. Input: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Comp. Ref: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "i Base Test: ", getSLSIndex(detType, tempid++), false));
			adcWidgets.push_back(new qDacWidget(this, det, false, "Temperature ADC: ", getSLSIndex(detType, tempid++), false));
			adcWidgets.push_back(new qDacWidget(this, det, false, "Temperature FPGA: ", getSLSIndex(detType, tempid++), false));
			break;

		case slsDetectorDefs::JUNGFRAU:
			lblSpinHV->show();
			spinHV->show();
			dacWidgets.push_back(new qDacWidget(this, det, true, "v vb comp: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v vdd prot: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v vin com: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v vref prech: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v vb pixbuf: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v vb ds: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v vref ds: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "i vref comp: ", getSLSIndex(detType, tempid++), false));
			adcWidgets.push_back(new qDacWidget(this, det, false, "Temperature ADC: ", getSLSIndex(detType, tempid++), true));
			break;

		case slsDetectorDefs::MOENCH:
			lblSpinHV->show();
			spinHV->show();
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Dac 0: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Dac 1: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Dac 2: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Dac 3: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Dac 4: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Dac 5: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "v Dac 6: ", getSLSIndex(detType, tempid++), false));
			dacWidgets.push_back(new qDacWidget(this, det, true, "i Dac 7: ", getSLSIndex(detType, tempid++), false));
			break;

		default:
			break;
		}
	} CATCH_DISPLAY (std::string("Could not get dac/ adc index ") + std::to_string(tempid), "qTabDeveloper::SetupWidgetWindow")

	for (size_t i = 0; i < dacWidgets.size(); ++i) {
			gridlayoutDac->addWidget(dacWidgets[i], i / 2, i % 2);
	}
		gridlayoutDac->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Expanding),dacWidgets.size(), 0);
	for (size_t i = 0; i < adcWidgets.size(); ++i) {
	gridlayoutAdc->addWidget(adcWidgets[i], i / 2, i % 2);
	}
	gridlayoutAdc->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Expanding),adcWidgets.size(), 0);

	tabWidget->setCurrentIndex(0);
	
	PopulateDetectors();
	Initialization();
	Refresh();
}


void qTabDeveloper::Initialization() {
	connect(comboDetector, SIGNAL(currentIndexChanged(int)), this, SLOT(Refresh()));
	connect(comboHV, SIGNAL(currentIndexChanged(int)), this, SLOT(SetHighVoltage()));
	connect(spinHV, SIGNAL(editingFinished()), this, SLOT(SetHighVoltage()));
}

void qTabDeveloper::PopulateDetectors() {
	FILE_LOG(logDEBUG) << "Populating detectors";

	comboDetector->clear();
	comboDetector->addItem("All");
	auto res = det->getHostname();
	if (det->size() > 1) {
		for (auto &it : res) {
			comboDetector->addItem(QString(it.c_str()));
		}
	}
	comboDetector->setCurrentIndex(0);
}

void qTabDeveloper::GetHighVoltage() {
	// not enabled for eiger
	if (!comboHV->isVisible() && !spinHV->isVisible())
		return;
	FILE_LOG(logDEBUG) << "Getting High Voltage";
	disconnect(spinHV, SIGNAL(editingFinished()), this, SLOT(SetHighVoltage()));	
	disconnect(comboHV, SIGNAL(currentIndexChanged(int)), this, SLOT(SetHighVoltage()));
	try {
		// dac units
		auto retval = det->getHighVoltage({comboDetector->currentIndex() - 1}).tsquash("Inconsistent values for high voltage.");
		//spinHV
		if (spinHV->isVisible()) {
			if (retval != 0 && retval < HV_MIN &&  retval > HV_MAX) {
				throw sls::RuntimeError(std::string("Unknown High Voltage: ") + std::to_string(retval));
			} 
			spinHV->setValue(retval);	
		} 
		// combo HV
		else {
			switch (retval) {
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
				throw sls::RuntimeError(std::string("Unknown High Voltage: ") + std::to_string(retval));
			}
		}
    } CATCH_DISPLAY ("Could not get high voltage.", "qTabDeveloper::GetHighVoltage")
	connect(spinHV, SIGNAL(editingFinished()), this, SLOT(SetHighVoltage()));	
	connect(comboHV, SIGNAL(currentIndexChanged(int)), this, SLOT(SetHighVoltage()));
}

void qTabDeveloper::SetHighVoltage() {
	int val = (comboHV->isVisible() ? comboHV->currentText().toInt() : spinHV->value());
	FILE_LOG(logINFO) << "Setting high voltage:" << val;
	
	try {
        det->setHighVoltage({comboDetector->currentIndex() - 1});
    } CATCH_HANDLE ("Could not set high voltage.", "qTabDeveloper::SetHighVoltage", 
					this, &qTabDeveloper::GetHighVoltage)
}

slsDetectorDefs::dacIndex qTabDeveloper::getSLSIndex(slsDetectorDefs::detectorType detType, int index) {
	switch (detType) {

	case slsDetectorDefs::EIGER:
		switch (index) {
		case 0:
			return slsDetectorDefs::SVP;
		case 1:
			return slsDetectorDefs::SVN;
		case 2:
			return slsDetectorDefs::VRF;
		case 3:
			return slsDetectorDefs::VRS;
		case 4:
			return slsDetectorDefs::VTR;
		case 5:
			return slsDetectorDefs::VTGSTV;
		case 6:
			return slsDetectorDefs::CAL;
		case 7:
			return slsDetectorDefs::VCP;
		case 8:
			return slsDetectorDefs::VCN;
		case 9:
			return slsDetectorDefs::VIS;
		case 10:
			return slsDetectorDefs::RXB_LB;
		case 11:
			return slsDetectorDefs::RXB_RB;
		case 12:
			return slsDetectorDefs::VCMP_LL;
		case 13:
			return slsDetectorDefs::VCMP_LR;
		case 14:
			return slsDetectorDefs::VCMP_RL;
		case 15:
			return slsDetectorDefs::VCMP_RR;
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
			return slsDetectorDefs::VREF_DS;
		case 1:
			return slsDetectorDefs::VCASCN_PB;
		case 2:
			return slsDetectorDefs::VCASCP_PB;
		case 3:
			return slsDetectorDefs::VOUT_CM;
		case 4:
			return slsDetectorDefs::VCASC_OUT;
		case 5:
			return slsDetectorDefs::VIN_CM;
		case 6:
			return slsDetectorDefs::VREF_COMP;
		case 7:
			return slsDetectorDefs::IB_TESTC;
		case 8:
			return slsDetectorDefs::TEMPERATURE_ADC;
		case 9:
			return slsDetectorDefs::TEMPERATURE_FPGA;
		default:
			throw sls::RuntimeError(std::string("Unknown dac/adc index") + std::to_string(index));
		}
		break;

	case slsDetectorDefs::JUNGFRAU:
		switch (index) {
		case 0:
			return slsDetectorDefs::VB_COMP;
		case 1:
			return slsDetectorDefs::VDD_PROT;
		case 2:
			return slsDetectorDefs::VIN_COM;
		case 3:
			return slsDetectorDefs::VREF_PRECH;
		case 4:
			return slsDetectorDefs::VB_PIXBUF;
		case 5:
			return slsDetectorDefs::VB_DS;
		case 6:
			return slsDetectorDefs::VREF_DS;
		case 7:
			return slsDetectorDefs::VREF_COMP;
		case 8:
			return slsDetectorDefs::TEMPERATURE_ADC;
		default:
			throw sls::RuntimeError(std::string("Unknown dac/adc index") + std::to_string(index));
		}
		break;

	case slsDetectorDefs::MOENCH:
		if (index >= 0 && index < (int)dacWidgets.size()) {
			return (slsDetectorDefs::dacIndex)index;
		} 
		throw sls::RuntimeError(std::string("Unknown dac/adc index") + std::to_string(index));

	default:
		throw sls::RuntimeError(std::string("Unknown detector type"));	
	}
}

void qTabDeveloper::Refresh() {
	FILE_LOG(logDEBUG) << "**Updating Developer Tab\n";
	for (const auto &it : dacWidgets) {
			it->SetDetectorIndex(comboDetector->currentIndex() - 1);
	}
	for (const auto &it : adcWidgets) {
			it->SetDetectorIndex(comboDetector->currentIndex() - 1);
	}
	GetHighVoltage();
	FILE_LOG(logDEBUG) << "**Updated Developer Tab";
}

