// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "qTabDeveloper.h"
#include "qDacWidget.h"
#include "qDefs.h"

namespace sls {

qTabDeveloper::qTabDeveloper(QWidget *parent, Detector *detector)
    : QWidget(parent), det(detector) {
    setupUi(this);
    SetupWidgetWindow();
    LOG(logDEBUG) << "Developer ready";
}

qTabDeveloper::~qTabDeveloper() {}

void qTabDeveloper::SetupWidgetWindow() {
    int tempid = 0;

    try {
        slsDetectorDefs::detectorType detType = det->getDetectorType().squash();
        switch (detType) {
        case slsDetectorDefs::EIGER:
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vsvp: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vsvn ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vrpreamp: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vrshaper: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vtrim: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vtgstv: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vcal: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vcp ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vcn: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vishaper: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "rxb_lb: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "rxb_rb: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vcmp_ll: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vcmp_lr: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vcmp_rl: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vcmp_rr: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "vthreshold: ", getSLSIndex(detType, tempid++)));
            adcWidgets.push_back(new qDacWidget(
                this, det, false,
                "Temperature FPGA Ext: ", getSLSIndex(detType, tempid++)));
            adcWidgets.push_back(new qDacWidget(
                this, det, false,
                "Temperature 10GE: ", getSLSIndex(detType, tempid++)));
            adcWidgets.push_back(new qDacWidget(
                this, det, false,
                "Temperature DCDC: ", getSLSIndex(detType, tempid++)));
            adcWidgets.push_back(new qDacWidget(
                this, det, false,
                "Temperature SODL: ", getSLSIndex(detType, tempid++)));
            adcWidgets.push_back(new qDacWidget(
                this, det, false,
                "Temperature SODR: ", getSLSIndex(detType, tempid++)));
            adcWidgets.push_back(new qDacWidget(
                this, det, false,
                "Temperature FPGA: ", getSLSIndex(detType, tempid++)));
            break;

        case slsDetectorDefs::GOTTHARD:
            dacWidgets.push_back(new qDacWidget(
                this, det, true,
                "v Reference: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true,
                "v Cascode n: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true,
                "v Cascode p: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true,
                "v Comp. Output: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true, "v Cascode out ",
                               getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true,
                "v Comp. Input: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true,
                "v Comp. Ref: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true,
                "i Base Test: ", getSLSIndex(detType, tempid++)));
            adcWidgets.push_back(new qDacWidget(
                this, det, false,
                "Temperature ADC: ", getSLSIndex(detType, tempid++)));
            adcWidgets.push_back(new qDacWidget(
                this, det, false,
                "Temperature FPGA: ", getSLSIndex(detType, tempid++)));
            break;

        case slsDetectorDefs::JUNGFRAU:
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "v vb comp: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "v vdd prot: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "v vin com: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true,
                "v vref prech: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true,
                "v vb pixbuf: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "v vb ds: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "v vref ds: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true,
                "i vref comp: ", getSLSIndex(detType, tempid++)));
            adcWidgets.push_back(new qDacWidget(
                this, det, false,
                "Temperature ADC: ", getSLSIndex(detType, tempid++)));
            break;

        case slsDetectorDefs::MOENCH:
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "vbp_colbuf: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vipre: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vin_cm: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vb_sda: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "vcasc_sfp: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vout_cm: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "vipre_cds: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "ibias_sfp: ", getSLSIndex(detType, tempid++)));
            break;

        case slsDetectorDefs::MYTHEN3:
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vcassh: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vth2: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vrshaper: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "vrshaper_n: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "vipre_out: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vth3: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vth1: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vicin: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vcas: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vrpreamp: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vcal_p: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vipre: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vishaper: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vcal_n: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vtrim: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vdcsh: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "vthreshold: ", getSLSIndex(detType, tempid++)));
            break;

        case slsDetectorDefs::GOTTHARD2:
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "vref_h_adc: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "vb_comp_fe: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true,
                "vb_comp_adc: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vcom_cds: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true,
                "vref_rstore: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "vb_opa_1st: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true,
                "vref_comp_fe: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "vcom_adc1: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "vref_prech: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "vref_l_adc: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vref_cds: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(new qDacWidget(
                this, det, true, "vb_cs: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "vb_opa_fd: ", getSLSIndex(detType, tempid++)));
            dacWidgets.push_back(
                new qDacWidget(this, det, true,
                               "vcom_adc2: ", getSLSIndex(detType, tempid++)));
            break;
        default:
            break;
        }
    }
    CATCH_DISPLAY(std::string("Could not get dac/ adc index ") +
                      std::to_string(tempid),
                  "qTabDeveloper::SetupWidgetWindow")

    for (size_t i = 0; i < dacWidgets.size(); ++i) {
        gridlayoutDac->addWidget(dacWidgets[i], i / 2, i % 2);
    }
    gridlayoutDac->addItem(
        new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Expanding),
        dacWidgets.size(), 0);
    for (size_t i = 0; i < adcWidgets.size(); ++i) {
        gridlayoutAdc->addWidget(adcWidgets[i], i / 2, i % 2);
    }
    gridlayoutAdc->addItem(
        new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Expanding),
        adcWidgets.size(), 0);

    tabWidget->setCurrentIndex(0);

    PopulateDetectors();
    Initialization();
    Refresh();
}

void qTabDeveloper::Initialization() {
    connect(comboDetector, SIGNAL(currentIndexChanged(int)), this,
            SLOT(Refresh()));
}

void qTabDeveloper::PopulateDetectors() {
    LOG(logDEBUG) << "Populating detectors";

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

slsDetectorDefs::dacIndex
qTabDeveloper::getSLSIndex(slsDetectorDefs::detectorType detType, int index) {
    switch (detType) {

    case slsDetectorDefs::EIGER:
        switch (index) {
        case 0:
            return slsDetectorDefs::VSVP;
        case 1:
            return slsDetectorDefs::VSVN;
        case 2:
            return slsDetectorDefs::VRPREAMP;
        case 3:
            return slsDetectorDefs::VRSHAPER;
        case 4:
            return slsDetectorDefs::VTRIM;
        case 5:
            return slsDetectorDefs::VTGSTV;
        case 6:
            return slsDetectorDefs::VCAL;
        case 7:
            return slsDetectorDefs::VCP;
        case 8:
            return slsDetectorDefs::VCN;
        case 9:
            return slsDetectorDefs::VISHAPER;
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
            return slsDetectorDefs::VTHRESHOLD;
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
            throw RuntimeError(std::string("Unknown dac/adc index") +
                               std::to_string(index));
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
            throw RuntimeError(std::string("Unknown dac/adc index") +
                               std::to_string(index));
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
            throw RuntimeError(std::string("Unknown dac/adc index") +
                               std::to_string(index));
        }
        break;

    case slsDetectorDefs::MOENCH:
        switch (index) {
        case 0:
            return slsDetectorDefs::VBP_COLBUF;
        case 1:
            return slsDetectorDefs::VIPRE;
        case 2:
            return slsDetectorDefs::VIN_CM;
        case 3:
            return slsDetectorDefs::VB_SDA;
        case 4:
            return slsDetectorDefs::VCASC_SFP;
        case 5:
            return slsDetectorDefs::VOUT_CM;
        case 6:
            return slsDetectorDefs::VIPRE_CDS;
        case 7:
            return slsDetectorDefs::IBIAS_SFP;
        default:
            throw RuntimeError(std::string("Unknown dac/adc index") +
                               std::to_string(index));
        }
        break;

    case slsDetectorDefs::MYTHEN3:
        switch (index) {
        case 0:
            return slsDetectorDefs::VCASSH;
        case 1:
            return slsDetectorDefs::VTH2;
        case 2:
            return slsDetectorDefs::VRSHAPER;
        case 3:
            return slsDetectorDefs::VRSHAPER_N;
        case 4:
            return slsDetectorDefs::VIPRE_OUT;
        case 5:
            return slsDetectorDefs::VTH3;
        case 6:
            return slsDetectorDefs::VTH1;
        case 7:
            return slsDetectorDefs::VICIN;
        case 8:
            return slsDetectorDefs::VCAS;
        case 9:
            return slsDetectorDefs::VRPREAMP;
        case 10:
            return slsDetectorDefs::VCAL_P;
        case 11:
            return slsDetectorDefs::VIPRE;
        case 12:
            return slsDetectorDefs::VISHAPER;
        case 13:
            return slsDetectorDefs::VCAL_N;
        case 14:
            return slsDetectorDefs::VTRIM;
        case 15:
            return slsDetectorDefs::VDCSH;
        case 16:
            return slsDetectorDefs::VTHRESHOLD;
        default:
            throw RuntimeError(std::string("Unknown dac/adc index") +
                               std::to_string(index));
        }
        break;

    case slsDetectorDefs::GOTTHARD2:
        switch (index) {
        case 0:
            return slsDetectorDefs::VREF_H_ADC;
        case 1:
            return slsDetectorDefs::VB_COMP_FE;
        case 2:
            return slsDetectorDefs::VB_COMP_ADC;
        case 3:
            return slsDetectorDefs::VCOM_CDS;
        case 4:
            return slsDetectorDefs::VREF_RSTORE;
        case 5:
            return slsDetectorDefs::VB_OPA_1ST;
        case 6:
            return slsDetectorDefs::VREF_COMP_FE;
        case 7:
            return slsDetectorDefs::VCOM_ADC1;
        case 8:
            return slsDetectorDefs::VREF_PRECH;
        case 9:
            return slsDetectorDefs::VREF_L_ADC;
        case 10:
            return slsDetectorDefs::VREF_CDS;
        case 11:
            return slsDetectorDefs::VB_CS;
        case 12:
            return slsDetectorDefs::VB_OPA_FD;
        case 13:
            return slsDetectorDefs::VCOM_ADC2;
        default:
            throw RuntimeError(std::string("Unknown dac/adc index") +
                               std::to_string(index));
        }
        break;

    default:
        throw RuntimeError(std::string("Unknown detector type"));
    }
}

void qTabDeveloper::Refresh() {
    LOG(logDEBUG) << "**Updating Developer Tab\n";
    for (const auto &it : dacWidgets) {
        it->SetDetectorIndex(comboDetector->currentIndex() - 1);
    }
    for (const auto &it : adcWidgets) {
        it->SetDetectorIndex(comboDetector->currentIndex() - 1);
    }
    LOG(logDEBUG) << "**Updated Developer Tab";
}

} // namespace sls
