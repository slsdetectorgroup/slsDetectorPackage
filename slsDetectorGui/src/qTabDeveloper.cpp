/*
 * qTabDeveloper.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
#include "qTabDeveloper.h"
#include "qDetectorMain.h"
//Project Class Headers
#include "multiSlsDetector.h"
#include "slsDetector.h"
//Qt Include Headers
#include <QDoubleValidator>
#include <QSpacerItem>
#include <QString>
//C++ Include Headers
#include <iostream>


//-------------------------------------------------------------------------------------------------------------------------------------------------

int qTabDeveloper::NUM_DAC_WIDGETS(0);
int qTabDeveloper::NUM_ADC_WIDGETS(0);

//-------------------------------------------------------------------------------------------------------------------------------------------------

qTabDeveloper::qTabDeveloper(qDetectorMain *parent, multiSlsDetector *&detector) : thisParent(parent),
                                                                                   myDet(detector),
                                                                                   boxDacs(0),
                                                                                   boxAdcs(0),
                                                                                   lblHV(0),
                                                                                   comboHV(0),
                                                                                   adcTimer(0),
                                                                                   dacLayout(0) {
    for (int i = 0; i < 20; i++) {
        lblDacs[i] = 0;
        lblAdcs[i] = 0;
        spinDacs[i] = 0;
        spinAdcs[i] = 0;
        lblDacsmV[i] = 0;
    }
    SetupWidgetWindow();
    Initialization();
    FILE_LOG(logDEBUG) << "Developer ready";
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

qTabDeveloper::~qTabDeveloper() {
    delete myDet;
    // if(det) delete det;
    if (thisParent)
        delete thisParent;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabDeveloper::SetupWidgetWindow() {
    //Detector Type
    detType = myDet->getDetectorTypeAsEnum();

    //palette
    red = QPalette();
    red.setColor(QPalette::Active, QPalette::WindowText, Qt::red);

    //the number of dacs and adcs
    switch (detType) {
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
    case slsDetectorDefs::CHIPTESTBOARD:
        NUM_DAC_WIDGETS = 8;
        NUM_ADC_WIDGETS = 1;
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

    default:
        cout << "ERROR: Unknown detector type: " + myDet->getDetectorTypeAsString() << endl;
        qDefs::Message(qDefs::CRITICAL, std::string("Unknown detector type:") + myDet->getDetectorTypeAsString(), "qTabDeveloper::SetupWidgetWindow");
        exit(-1);
        break;
    }

    //layout
    setFixedWidth(765);
    setFixedHeight(20 + 50 + (NUM_DAC_WIDGETS / 2) * 35);
    //setHeight(340);

    scroll = new QScrollArea;
    //scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(this);
    scroll->setWidgetResizable(true);

    layout = new QGridLayout(scroll);
    layout->setContentsMargins(20, 10, 10, 5);
    setLayout(layout);

    //readout
    comboDetector = new QComboBox(this);
    //comboDetector->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    comboDetector->addItem("All");
    //add detectors
    for (int i = 1; i < myDet->getNumberOfDetectors() + 1; i++)
        comboDetector->addItem(QString(myDet->getHostname(i - 1).c_str()));
    comboDetector->setCurrentIndex(0);

    //dacs
    boxDacs = new QGroupBox("Dacs", this);
    boxDacs->setFixedHeight(25 + (NUM_DAC_WIDGETS / 2) * 35);
    CreateDACWidgets();

    if ((detType == slsDetectorDefs::GOTTHARD) ||
        (detType == slsDetectorDefs::MOENCH)) {
        boxDacs->setFixedHeight(boxDacs->height() + 35);

        lblHV = new QLabel("High Voltage", boxDacs);
        comboHV = new QComboBox(boxDacs);
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
        dacLayout->addWidget(lblHV, (int)(NUM_DAC_WIDGETS / 2), 1);
        dacLayout->addWidget(comboHV, (int)(NUM_DAC_WIDGETS / 2), 2);
        connect(comboHV, SIGNAL(currentIndexChanged(int)), this, SLOT(SetHighVoltage()));
    }
    layout->addWidget(comboDetector, 0, 0);
    layout->addWidget(boxDacs, 1, 0);

    //adcs
    if (NUM_ADC_WIDGETS) {
        int rows = NUM_ADC_WIDGETS / 2;
        if (NUM_ADC_WIDGETS % 2)
            rows++;
        setFixedHeight(20 + (50 + (NUM_DAC_WIDGETS / 2) * 35) + (50 + rows * 35));
        boxAdcs = new QGroupBox("ADCs", this);
        boxAdcs->setFixedHeight(25 + rows * 35);
        layout->addWidget(boxAdcs, 2, 0);
        CreateADCWidgets();
        //to make the adcs at the bottom most
        if (detType != slsDetectorDefs::EIGER) {
            int diff = 340 - height();
            setFixedHeight(340);
            layout->setVerticalSpacing(diff / 2);
        }
        //timer to check adcs
        /*adcTimer = new QTimer(this); adc timer disabled, display adcs only when refreshing developer tab */
    }

    qDefs::checkErrorMessage(myDet, "qTabDeveloper::SetupWidgetWindow");
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabDeveloper::Initialization() {
    /*if(NUM_ADC_WIDGETS) connect(adcTimer, 	SIGNAL(timeout()), 		this, SLOT(RefreshAdcs()));*/

    for (int i = 0; i < NUM_DAC_WIDGETS; i++)
        connect(spinDacs[i], SIGNAL(editingFinished(int)), this, SLOT(SetDacValues(int)));

    connect(comboDetector, SIGNAL(currentIndexChanged(int)), this, SLOT(Refresh()));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabDeveloper::CreateDACWidgets() {
    dacLayout = new QGridLayout(boxDacs);

    for (int i = 0; i < NUM_DAC_WIDGETS; i++) {
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

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabDeveloper::CreateADCWidgets() {
    QGridLayout *adcLayout = new QGridLayout(boxAdcs);

    for (int i = 0; i < NUM_ADC_WIDGETS; i++) {
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
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabDeveloper::SetDacValues(int id) {
#ifdef VERBOSE
    cout << "Setting dac:" << dacNames[id] << " : " << spinDacs[id]->value() << endl;
#endif

    int module_id = comboDetector->currentIndex() - 1;
    myDet->setDAC(spinDacs[id]->value(), getSLSIndex(id), 0, module_id);
    lblDacsmV[id]->setText(QString("%1mV").arg(myDet->setDAC(-1, getSLSIndex(id), 1, module_id), -10));
    qDefs::checkErrorMessage(myDet, "qTabDeveloper::SetDacValues");
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabDeveloper::SetHighVoltage() {
#ifdef VERBOSE
    cout << "Setting high voltage:" << comboHV->currentText().toAscii().constData() << endl;
#endif

    auto module_id = comboDetector->currentIndex();
    if (module_id)
        module_id--;

    int highvoltage = comboHV->currentText().toInt();
    int ret;

    //all detectors
    // if(!detid){
    ret = myDet->setDAC(highvoltage, slsDetectorDefs::HIGH_VOLTAGE, 0, module_id);
    qDefs::checkErrorMessage(myDet, "qTabDeveloper::SetHighVoltage");
    // }
    // //specific detector
    // else{
    // 	ret = det->setDAC(highvoltage,slsDetectorDefs::HV_POT,0);
    // 	qDefs::checkErrorMessage(det,"qTabDeveloper::SetHighVoltage");
    // }

    //error
    if (ret != highvoltage) {
        qDefs::Message(qDefs::CRITICAL, "High Voltage could not be set to this value.", "qTabDeveloper::SetHighVoltage");
        lblHV->setPalette(red);
        lblHV->setText("High Voltage:*");
        QString errTip = tipHV + QString("<br><br><font color=\"red\"><nobr>High Voltage could not be set. The return value is ") +
                         QString::number(ret) + QString("</nobr></font>");
        lblHV->setToolTip(errTip);
        comboHV->setToolTip(errTip);
    } else {
        lblHV->setPalette(lblDacs[0]->palette());
        lblHV->setText("High Voltage:");
        lblHV->setToolTip(tipHV);
        comboHV->setToolTip(tipHV);
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

slsDetectorDefs::dacIndex qTabDeveloper::getSLSIndex(int index) {
    switch (detType) {
    // case slsDetectorDefs::MYTHEN:
    // 	switch(index){
    // 	case 0:	return slsDetectorDefs::TRIMBIT_SIZE;
    // 	case 1:	return slsDetectorDefs::THRESHOLD;
    // 	case 2:	return slsDetectorDefs::SHAPER1;
    // 	case 3:	return slsDetectorDefs::SHAPER2;
    // 	case 4:	return slsDetectorDefs::CALIBRATION_PULSE;
    // 	case 5:	return slsDetectorDefs::PREAMP;
    // 	default:
    // 		qDefs::Message(qDefs::CRITICAL,"Unknown DAC/ADC Index. Weird Error Index:"+ index,"qTabDeveloper::getSLSIndex");
    // 		Refresh();
    // 		break;
    // 	}
    // 	break;
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
    case slsDetectorDefs::MOENCH:
        switch (index) {
            //TODO! Moench DACS
            // case 0:	return slsDetectorDefs::V_DAC0;
            // case 1:	return slsDetectorDefs::V_DAC1;
            // case 2:	return slsDetectorDefs::V_DAC2;
            // case 3:	return slsDetectorDefs::V_DAC3;
            // case 4:	return slsDetectorDefs::V_DAC4;
            // case 5:	return slsDetectorDefs::V_DAC5;
            // case 6:	return slsDetectorDefs::V_DAC6;
            // case 7:	return slsDetectorDefs::V_DAC7;
            // case 8:	return slsDetectorDefs::TEMPERATURE_ADC;
            // case 9:return slsDetectorDefs::TEMPERATURE_FPGA;

        default:
            qDefs::Message(qDefs::CRITICAL, "Unknown DAC/ADC Index. Weird Error. Index:" + index, "qTabDeveloper::getSLSIndex");
            Refresh();
            break;
        }
        break;
    // case slsDetectorDefs::PROPIX:
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
    case slsDetectorDefs::CHIPTESTBOARD:

        switch (index) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            return (slsDetectorDefs::dacIndex)index;
            break;
        case 8:
            return slsDetectorDefs::TEMPERATURE_ADC;
        default:
            qDefs::Message(qDefs::CRITICAL, "Unknown DAC/ADC Index. Weird Error Index:" + index, "qTabDeveloper::getSLSIndex");
            Refresh();
            break;
        }
        break;
    default:
        cout << "Unknown detector type:" + myDet->getDetectorTypeAsString() << endl;
        qDefs::Message(qDefs::CRITICAL, std::string("Unknown detector type:") + myDet->getDetectorTypeAsString(), "qTabDeveloper::getSLSIndex");
        qDefs::checkErrorMessage(myDet, "qTabDeveloper::getSLSIndex");
        exit(-1);
        break;
    }
    return slsDetectorDefs::HUMIDITY;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabDeveloper::RefreshAdcs() {
    if (!thisParent->isCurrentlyTabDeveloper())
        return;

#ifdef VERYVERBOSE
    cout << "Updating ADCs" << endl;
#endif
    /*adcTimer->stop();*/

    auto module_id = comboDetector->currentIndex();
    if (module_id)
        module_id--;

    for (int i = 0; i < NUM_ADC_WIDGETS; i++) {
        //all detectors
        if (module_id == -1) {
            double value = (double)myDet->getADC(getSLSIndex(i + NUM_DAC_WIDGETS), -1);

            if (value == -1)
                spinAdcs[i]->setText(QString("Different values"));
            else {
                if (detType == slsDetectorDefs::EIGER || detType == slsDetectorDefs::JUNGFRAU || detType == slsDetectorDefs::CHIPTESTBOARD)
                    value /= 1000.00;
                spinAdcs[i]->setText(QString::number(value, 'f', 2) + 0x00b0 + QString("C"));
            }
        }
        //specific detector
        else {
            double value = (double)myDet->getADC(getSLSIndex(i + NUM_DAC_WIDGETS), module_id);

            if (detType == slsDetectorDefs::EIGER || detType == slsDetectorDefs::JUNGFRAU || detType == slsDetectorDefs::CHIPTESTBOARD)
                value /= 1000.00;
            spinAdcs[i]->setText(QString::number(value, 'f', 2) + 0x00b0 + QString("C"));
        }
    }

    /*adcTimer->start(ADC_TIMEOUT);*/
    qDefs::checkErrorMessage(myDet, "qTabDeveloper::RefreshAdcs");
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qTabDeveloper::Refresh() {
#ifdef VERBOSE
    std::cout << "**Updating Developer Tab\n";
    std::cout << "Getting DACs" << NUM_DAC_WIDGETS << '\n';
#endif

    
    auto module_id = comboDetector->currentIndex() - 1;
    for (int i = 0; i < NUM_DAC_WIDGETS; i++) {
        spinDacs[i]->setValue(myDet->setDAC(-1, getSLSIndex(i), 0, module_id));
        lblDacsmV[i]->setText(QString("%1mV").arg(myDet->setDAC(-1, getSLSIndex(i), 1, module_id), -10));
    }

    //adcs
    if (NUM_ADC_WIDGETS)
        RefreshAdcs();

    //gotthard -high voltage
    if ((detType == slsDetectorDefs::GOTTHARD) ||
        // (detType == slsDetectorDefs::PROPIX) ||
        (detType == slsDetectorDefs::MOENCH)) {
        disconnect(comboHV, SIGNAL(currentIndexChanged(int)), this, SLOT(SetHighVoltage()));

        //default should be correct
        lblHV->setPalette(lblDacs[0]->palette());
        lblHV->setText("High Voltage:");
        lblHV->setToolTip(tipHV);
        comboHV->setToolTip(tipHV);
        //getting hv value
        int ret;
        // if(!detid) 	ret = (int)myDet->setDAC(-1,slsDetectorDefs::HV_POT,0);
        ret = myDet->setDAC(-1, slsDetectorDefs::HIGH_VOLTAGE, 0, module_id);

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
            lblHV->setPalette(red);
            lblHV->setText("High Voltage:*");
            QString errTip = tipHV + QString("<br><br><font color=\"red\"><nobr>High Voltage could not be set. The return value is ") +
                             QString::number(ret) + QString("</nobr></font>");
            lblHV->setToolTip(errTip);
            comboHV->setToolTip(errTip);
            break;
        }

        connect(comboHV, SIGNAL(currentIndexChanged(int)), this, SLOT(SetHighVoltage()));
    }

#ifdef VERBOSE
    cout << "**Updated Developer Tab" << endl
         << endl;
#endif

    qDefs::checkErrorMessage(myDet, "qTabDeveloper::Refresh");
}

//-------------------------------------------------------------------------------------------------------------------------------------------------
