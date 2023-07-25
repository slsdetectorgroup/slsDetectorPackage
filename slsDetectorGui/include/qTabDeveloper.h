// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include "ui_form_tab_developer.h"
#include <vector>

namespace sls {

class qDacWidget;

class qTabDeveloper : public QWidget, private Ui::TabDeveloperObject {
    Q_OBJECT

  public:
    qTabDeveloper(QWidget *parent, Detector *detector);
    ~qTabDeveloper();

  public slots:
    void Refresh();

  private slots:
    void setDetectorIndex();

  private:
    void SetupWidgetWindow();
    void Initialization();
    void PopulateDetectors();

    Detector *det;
    std::vector<qDacWidget *> dacWidgets;
    std::vector<qDacWidget *> adcWidgets;

    const std::vector<std::string> eiger_dacs = {
        "vsvp",    "vsvn",    "vrpreamp", "vrshaper", "vtrim",     "vtgstv",
        "vcal",    "vcp",     "vcn",      "vishaper", "rxb_lb",    "rxb_rb",
        "vcmp_ll", "vcmp_lr", "vcmp_rl",  "vcmp_rr",  "vthreshold"};
    const std::vector<std::string> eiger_adcs = {
        "temp_fpga", "temp_fpgaext", "temp_10ge",   "temp_dcdc",
        "temp_sodl", "temp_sodr",    "temp_fpgafl", "temp_fpgafr"};
    const std::vector<std::string> gotthard_dacs = {
        "vref_ds",   "vcascn_pb", "vcascp_pb", "vout_cm",
        "vcasc_out", "vin_cm",    "vref_comp", "ib_test_c"};
    const std::vector<std::string> gotthard_adcs = {"temp_adc", "temp_fpga"};
    const std::vector<std::string> jungfrau_dacs = {
        "vb_comp",   "vdd_prot", "vin_com", "vref_prech",
        "vb_pixbuf", "vb_ds",    "vref_ds", "vref_comp"};
    const std::vector<std::string> jungfrau_adcs = {"temp_adc"};
    const std::vector<std::string> gotthard2_dacs = {
        "vref_h_adc",  "vb_comp_fe", "vb_comp_adc",  "vcom_cds",
        "vref_rstore", "vb_opa_1st", "vref_comp_fe", "vcom_adc1",
        "vref_prech",  "vref_l_adc", "vref_cds",     "vb_cs",
        "vb_opa_fd",   "vcom_adc2"};
    const std::vector<std::string> gotthard2_adcs = {"temp_fpga"};
    const std::vector<std::string> mythen3_dacs = {
        "vcassh",   "vth2",   "vrshaper", "vrshaper_n", "vipre_out", "vth3",
        "vth1",     "vicin",  "vcas",     "vrpreamp",   "vcal_p",    "vipre",
        "vishaper", "vcal_n", "vtrim",    "vdcsh",      "vthreshold"};
    const std::vector<std::string> mythen3_adcs = {"temp_fpga"};

    const std::vector<std::string> moench_dacs = {
        "vbp_colbuf", "vipre",   "vin_cm",    "vb_sda",
        "vcasc_sfp",  "vout_cm", "vipre_cds", "ibias_sfp"};
    const std::vector<std::string> moench_adcs = {"temp_fpga"};
};

} // namespace sls
