#include "qTabAdvanced.h"
#include "network_utils.h"
#include "qDefs.h"
#include "qDrawPlot.h"

#include <iostream>

qTabAdvanced::qTabAdvanced(QWidget *parent, sls::Detector *detector,
                           qDrawPlot *p)
    : QWidget(parent), det(detector), plot(p) {
    setupUi(this);
    SetupWidgetWindow();
    FILE_LOG(logDEBUG) << "Advanced ready";
}

qTabAdvanced::~qTabAdvanced() {}

void qTabAdvanced::SetupWidgetWindow() {
    // enabling according to det type
    switch (det->getDetectorType().squash()) {
    case slsDetectorDefs::EIGER:
        tab_trimming->setEnabled(true);
        lblSubExpTime->setEnabled(true);
        spinSubExpTime->setEnabled(true);
        comboSubExpTimeUnit->setEnabled(true);
        lblSubDeadTime->setEnabled(true);
        spinSubDeadTime->setEnabled(true);
        comboSubDeadTimeUnit->setEnabled(true);
        break;
    case slsDetectorDefs::GOTTHARD:
        tab_roi->setEnabled(true);
        break;
    case slsDetectorDefs::MYTHEN3:
    case slsDetectorDefs::GOTTHARD2:
        lblDiscardBits->setEnabled(true);
        spinDiscardBits->setEnabled(true);
    default:
        break;
    }

    // set initially to network tab
    tabAdvancedSettings->setCurrentWidget(tab_network);

    Initialization();

    PopulateDetectors();

    Refresh();
}

void qTabAdvanced::Initialization() {

    connect(tabAdvancedSettings, SIGNAL(currentChanged(int)), this,
            SLOT(Refresh()));

    // trimming
    if (tab_trimming->isEnabled()) {
        // editingFinished to not set trimbits for every character input
        connect(spinSetAllTrimbits, SIGNAL(editingFinished()), this,
                SLOT(SetAllTrimbits()));
    }

    // network
    connect(comboDetector, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetDetector()));
    connect(spinControlPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetControlPort(int)));
    connect(spinStopPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetStopPort(int)));
    connect(dispDetectorUDPIP, SIGNAL(editingFinished()), this,
            SLOT(SetDetectorUDPIP()));
    connect(dispDetectorUDPMAC, SIGNAL(editingFinished()), this,
            SLOT(SetDetectorUDPMAC()));
    connect(spinZMQPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetCltZMQPort(int)));
    connect(dispZMQIP, SIGNAL(editingFinished()), this, SLOT(SetCltZMQIP()));
    connect(dispRxrHostname, SIGNAL(editingFinished()), this,
            SLOT(SetRxrHostname()));
    connect(spinRxrTCPPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetRxrTCPPort(int)));
    connect(spinRxrUDPPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetRxrUDPPort(int)));
    connect(dispRxrUDPIP, SIGNAL(editingFinished()), this, SLOT(SetRxrUDPIP()));
    connect(dispRxrUDPMAC, SIGNAL(editingFinished()), this,
            SLOT(SetRxrUDPMAC()));
    connect(spinRxrZMQPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetRxrZMQPort(int)));
    connect(dispRxrZMQIP, SIGNAL(editingFinished()), this, SLOT(SetRxrZMQIP()));

    // roi
    if (tab_roi->isEnabled()) {
        connect(comboReadout, SIGNAL(currentIndexChanged(int)), this,
                SLOT(GetROI()));
        connect(btnSetRoi, SIGNAL(clicked()), this, SLOT(SetROI()));
        connect(btnClearRoi, SIGNAL(clicked()), this, SLOT(ClearROI()));
    }

    // storage cells
    if (lblNumStoragecells->isEnabled()) {
        connect(spinNumStoragecells, SIGNAL(valueChanged(int)), this,
                SLOT(SetNumStoragecells(int)));
    }

    // subexptime, subdeadtime
    if (lblSubExpTime->isEnabled()) {
        connect(spinSubExpTime, SIGNAL(valueChanged(double)), this,
                SLOT(SetSubExposureTime()));
        connect(comboSubExpTimeUnit, SIGNAL(currentIndexChanged(int)), this,
                SLOT(SetSubExposureTime()));
        connect(spinSubDeadTime, SIGNAL(valueChanged(double)), this,
                SLOT(SetSubDeadTime()));
        connect(comboSubDeadTimeUnit, SIGNAL(currentIndexChanged(int)), this,
                SLOT(SetSubDeadTime()));
    }

    // throw bits
    if (lblDiscardBits->isEnabled()) {
        connect(spinDiscardBits, SIGNAL(valueChanged(int)), plot,
                SLOT(SetNumDiscardBits(int)));
    }
}

void qTabAdvanced::PopulateDetectors() {
    FILE_LOG(logDEBUG) << "Populating detectors";
    disconnect(comboDetector, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetDetector()));
    disconnect(comboReadout, SIGNAL(currentIndexChanged(int)), this,
               SLOT(GetROI()));

    comboDetector->clear();
    comboReadout->clear();
    auto res = det->getHostname();
    for (auto &it : res) {
        comboDetector->addItem(QString(it.c_str()));
        comboReadout->addItem(QString(it.c_str()));
    }
    comboDetector->setCurrentIndex(0);
    comboReadout->setCurrentIndex(0);

    connect(comboDetector, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetDetector()));
    connect(comboReadout, SIGNAL(currentIndexChanged(int)), this,
            SLOT(GetROI()));
}

void qTabAdvanced::GetControlPort() {
    FILE_LOG(logDEBUG) << "Getting control port ";
    disconnect(spinControlPort, SIGNAL(valueChanged(int)), this,
               SLOT(SetControlPort(int)));

    try {
        int retval = det->getControlPort({comboDetector->currentIndex()})[0];
        spinControlPort->setValue(retval);
    }
    CATCH_DISPLAY("Could not get detector control port.",
                  "qTabAdvanced::GetControlPort")

    connect(spinControlPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetControlPort(int)));
}

void qTabAdvanced::GetStopPort() {
    FILE_LOG(logDEBUG) << "Getting stop port";
    disconnect(spinStopPort, SIGNAL(valueChanged(int)), this,
               SLOT(SetStopPort(int)));

    try {
        int retval = det->getStopPort({comboDetector->currentIndex()})[0];
        spinStopPort->setValue(retval);
    }
    CATCH_DISPLAY("Could not get detector stop port.",
                  "qTabAdvanced::GetStopPort")

    connect(spinStopPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetStopPort(int)));
}

void qTabAdvanced::GetDetectorUDPIP() {
    FILE_LOG(logDEBUG) << "Getting Detector UDP IP";
    disconnect(dispDetectorUDPIP, SIGNAL(editingFinished()), this,
               SLOT(SetDetectorUDPIP()));

    try {
        auto retval =
            det->getSourceUDPIP({comboDetector->currentIndex()})[0].str();
        dispDetectorUDPIP->setText(QString(retval.c_str()));
    }
    CATCH_DISPLAY("Could not get detector UDP IP.",
                  "qTabAdvanced::GetDetectorUDPIP")

    connect(dispDetectorUDPIP, SIGNAL(editingFinished()), this,
            SLOT(SetDetectorUDPIP()));
}

void qTabAdvanced::GetDetectorUDPMAC() {
    FILE_LOG(logDEBUG) << "Getting Detector UDP MAC";
    disconnect(dispDetectorUDPMAC, SIGNAL(editingFinished()), this,
               SLOT(SetDetectorUDPMAC()));

    try {
        auto retval =
            det->getSourceUDPMAC({comboDetector->currentIndex()})[0].str();
        dispDetectorUDPMAC->setText(QString(retval.c_str()));
    }
    CATCH_DISPLAY("Could not get detector UDP MAC.",
                  "qTabAdvanced::GetDetectorUDPMAC")

    connect(dispDetectorUDPMAC, SIGNAL(editingFinished()), this,
            SLOT(SetDetectorUDPMAC()));
}

void qTabAdvanced::GetCltZMQPort() {
    FILE_LOG(logDEBUG) << "Getting Client ZMQ port";
    disconnect(spinZMQPort, SIGNAL(valueChanged(int)), this,
               SLOT(SetCltZMQPort(int)));

    try {
        int retval = det->getClientZmqPort({comboDetector->currentIndex()})[0];
        spinZMQPort->setValue(retval);
    }
    CATCH_DISPLAY("Could not get client zmq port.",
                  "qTabAdvanced::GetCltZMQPort")

    connect(spinZMQPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetCltZMQPort(int)));
}

void qTabAdvanced::GetCltZMQIP() {
    FILE_LOG(logDEBUG) << "Getting Client ZMQ IP";
    disconnect(dispZMQIP, SIGNAL(editingFinished()), this, SLOT(SetCltZMQIP()));

    try {
        auto retval =
            det->getClientZmqIp({comboDetector->currentIndex()})[0].str();
        dispZMQIP->setText(QString(retval.c_str()));
    }
    CATCH_DISPLAY("Could not get client zmq ip.", "qTabAdvanced::GetCltZMQIP")

    connect(dispZMQIP, SIGNAL(editingFinished()), this, SLOT(SetCltZMQIP()));
}

void qTabAdvanced::GetRxrHostname() {
    FILE_LOG(logDEBUG) << "Getting Receiver Hostname";
    disconnect(dispRxrHostname, SIGNAL(editingFinished()), this,
               SLOT(SetRxrHostname()));

    try {
        auto retval = det->getRxHostname({comboDetector->currentIndex()})[0];
        dispRxrHostname->setText(QString(retval.c_str()));
    }
    CATCH_DISPLAY("Could not get receiver hostname.",
                  "qTabAdvanced::GetRxrHostname")

    connect(dispRxrHostname, SIGNAL(editingFinished()), this,
            SLOT(SetRxrHostname()));
}

void qTabAdvanced::GetRxrTCPPort() {
    FILE_LOG(logDEBUG) << "Getting Receiver TCP port";
    disconnect(spinRxrTCPPort, SIGNAL(valueChanged(int)), this,
               SLOT(SetRxrTCPPort(int)));

    try {
        int retval = det->getRxPort({comboDetector->currentIndex()})[0];
        spinRxrTCPPort->setValue(retval);
    }
    CATCH_DISPLAY("Could not get receiver tcp port.",
                  "qTabAdvanced::GetRxrTCPPort")

    connect(spinRxrTCPPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetRxrTCPPort(int)));
}

void qTabAdvanced::GetRxrUDPPort() {
    FILE_LOG(logDEBUG) << "Getting Receiver UDP port";
    disconnect(spinRxrUDPPort, SIGNAL(valueChanged(int)), this,
               SLOT(SetRxrUDPPort(int)));

    try {
        int retval =
            det->getDestinationUDPPort({comboDetector->currentIndex()})[0];
        spinRxrUDPPort->setValue(retval);
    }
    CATCH_DISPLAY("Could not get receiver udp port.",
                  "qTabAdvanced::GetRxrUDPPort")

    connect(spinRxrUDPPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetRxrUDPPort(int)));
}

void qTabAdvanced::GetRxrUDPIP() {
    FILE_LOG(logDEBUG) << "Getting Receiver UDP IP";
    disconnect(dispRxrUDPIP, SIGNAL(editingFinished()), this,
               SLOT(SetRxrUDPIP()));

    try {
        auto retval =
            det->getDestinationUDPIP({comboDetector->currentIndex()})[0].str();
        dispRxrUDPIP->setText(QString(retval.c_str()));
    }
    CATCH_DISPLAY("Could not get receiver udp ip.", "qTabAdvanced::GetRxrUDPIP")

    connect(dispRxrUDPIP, SIGNAL(editingFinished()), this, SLOT(SetRxrUDPIP()));
}

void qTabAdvanced::GetRxrUDPMAC() {
    FILE_LOG(logDEBUG) << "Getting Receiver UDP MAC";
    disconnect(dispRxrUDPMAC, SIGNAL(editingFinished()), this,
               SLOT(SetRxrUDPMAC()));

    try {
        auto retval =
            det->getDestinationUDPMAC({comboDetector->currentIndex()})[0].str();
        dispRxrUDPMAC->setText(QString(retval.c_str()));
    }
    CATCH_DISPLAY("Could not get receiver udp mac.",
                  "qTabAdvanced::GetRxrUDPMAC")

    connect(dispRxrUDPMAC, SIGNAL(editingFinished()), this,
            SLOT(SetRxrUDPMAC()));
}

void qTabAdvanced::GetRxrZMQPort() {
    FILE_LOG(logDEBUG) << "Getting Receiver ZMQ port";
    disconnect(spinRxrZMQPort, SIGNAL(valueChanged(int)), this,
               SLOT(SetRxrZMQPort(int)));

    try {
        int retval = det->getRxZmqPort({comboDetector->currentIndex()})[0];
        spinRxrZMQPort->setValue(retval);
    }
    CATCH_DISPLAY("Could not get receiver zmq port.",
                  "qTabAdvanced::GetRxrZMQPort")

    connect(spinRxrZMQPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetRxrZMQPort(int)));
}

void qTabAdvanced::GetRxrZMQIP() {
    FILE_LOG(logDEBUG) << "Getting Receiver ZMQ IP";
    disconnect(dispRxrZMQIP, SIGNAL(editingFinished()), this,
               SLOT(SetRxrZMQIP()));

    try {
        auto retval = det->getRxZmqIP({comboDetector->currentIndex()})[0].str();
        dispRxrZMQIP->setText(QString(retval.c_str()));
    }
    CATCH_DISPLAY("Could not get receiver zmq ip.", "qTabAdvanced::GetRxrZMQIP")

    connect(dispRxrZMQIP, SIGNAL(editingFinished()), this, SLOT(SetRxrZMQIP()));
}

void qTabAdvanced::SetDetector() {
    FILE_LOG(logDEBUG) << "Set Detector: "
                       << comboDetector->currentText().toAscii().data();

    GetControlPort();
    GetStopPort();
    GetDetectorUDPIP();
    GetDetectorUDPMAC();
    GetCltZMQPort();
    GetCltZMQIP();
    GetRxrHostname();
    GetRxrTCPPort();
    GetRxrUDPPort();
    GetRxrUDPIP();
    GetRxrUDPMAC();
    GetRxrZMQPort();
    GetRxrZMQIP();

    FILE_LOG(logDEBUG) << det->printRxConfiguration();
}

void qTabAdvanced::SetControlPort(int port) {
    FILE_LOG(logINFO) << "Setting Control Port:" << port;
    try {
        det->setControlPort(port, {comboDetector->currentIndex()});
    }
    CATCH_HANDLE("Could not set control port.", "qTabAdvanced::SetControlPort",
                 this, &qTabAdvanced::GetControlPort)
}

void qTabAdvanced::SetStopPort(int port) {
    FILE_LOG(logINFO) << "Setting Stop Port:" << port;
    try {
        det->setStopPort(port, {comboDetector->currentIndex()});
    }
    CATCH_HANDLE("Could not set stop port.", "qTabAdvanced::SetStopPort", this,
                 &qTabAdvanced::GetStopPort)
}

void qTabAdvanced::SetDetectorUDPIP() {
    std::string s = dispDetectorUDPIP->text().toAscii().constData();
    FILE_LOG(logINFO) << "Setting Detector UDP IP:" << s;
    try {
        det->setSourceUDPIP(sls::IpAddr{s}, {comboDetector->currentIndex()});
    }
    CATCH_HANDLE("Could not set Detector UDP IP.",
                 "qTabAdvanced::SetDetectorUDPIP", this,
                 &qTabAdvanced::GetDetectorUDPIP)
}

void qTabAdvanced::SetDetectorUDPMAC() {
    std::string s = dispDetectorUDPMAC->text().toAscii().constData();
    FILE_LOG(logINFO) << "Setting Detector UDP MAC:" << s;
    try {
        det->setSourceUDPMAC(sls::MacAddr{s}, {comboDetector->currentIndex()});
    }
    CATCH_HANDLE("Could not set Detector UDP MAC.",
                 "qTabAdvanced::SetDetectorUDPMAC", this,
                 &qTabAdvanced::GetDetectorUDPMAC)
}

void qTabAdvanced::SetCltZMQPort(int port) {
    FILE_LOG(logINFO) << "Setting Client ZMQ Port:" << port;
    try {
        det->setClientZmqPort(port, {comboDetector->currentIndex()});
    }
    CATCH_HANDLE("Could not set Client ZMQ port.",
                 "qTabAdvanced::SetCltZMQPort", this,
                 &qTabAdvanced::GetCltZMQPort)
}

void qTabAdvanced::SetCltZMQIP() {
    std::string s = dispZMQIP->text().toAscii().constData();
    FILE_LOG(logINFO) << "Setting Client ZMQ IP:" << s;
    try {
        det->setClientZmqIp(sls::IpAddr{s}, {comboDetector->currentIndex()});
    }
    CATCH_HANDLE("Could not set Client ZMQ IP.", "qTabAdvanced::SetCltZMQIP",
                 this, &qTabAdvanced::GetCltZMQIP)
}

void qTabAdvanced::SetRxrHostname() {
    std::string s = dispZMQIP->text().toAscii().constData();
    FILE_LOG(logINFO) << "Setting Receiver Hostname:" << s;
    try {
        det->setRxHostname(s, {comboDetector->currentIndex()});
    }
    CATCH_HANDLE("Could not set Client ZMQ IP.", "qTabAdvanced::SetRxrHostname",
                 this, &qTabAdvanced::GetRxrHostname)

    // update all network widgets (receiver mainly)
    SetDetector();
}

void qTabAdvanced::SetRxrTCPPort(int port) {
    FILE_LOG(logINFO) << "Setting Receiver TCP Port:" << port;
    try {
        det->setRxPort(port, {comboDetector->currentIndex()});
    }
    CATCH_HANDLE("Could not set Receiver TCP port.",
                 "qTabAdvanced::SetRxrTCPPort", this,
                 &qTabAdvanced::GetRxrTCPPort)
}

void qTabAdvanced::SetRxrUDPPort(int port) {
    FILE_LOG(logINFO) << "Setting Receiver UDP Port:" << port;
    try {
        det->setRxPort(port, {comboDetector->currentIndex()});
    }
    CATCH_HANDLE("Could not set Receiver UDP port.",
                 "qTabAdvanced::SetRxrUDPPort", this,
                 &qTabAdvanced::GetRxrUDPPort)
}

void qTabAdvanced::SetRxrUDPIP() {
    std::string s = dispRxrUDPIP->text().toAscii().constData();
    FILE_LOG(logINFO) << "Setting Receiver UDP IP:" << s;
    try {
        det->setDestinationUDPIP(sls::IpAddr{s},
                                 {comboDetector->currentIndex()});
    }
    CATCH_HANDLE("Could not set Receiver UDP IP.", "qTabAdvanced::SetRxrUDPIP",
                 this, &qTabAdvanced::GetRxrUDPIP)
}

void qTabAdvanced::SetRxrUDPMAC() {
    std::string s = dispRxrUDPMAC->text().toAscii().constData();
    FILE_LOG(logINFO) << "Setting Receiver UDP MAC:" << s;
    try {
        det->setDestinationUDPMAC(sls::MacAddr{s},
                                  {comboDetector->currentIndex()});
    }
    CATCH_HANDLE("Could not set Receiver UDP MAC.",
                 "qTabAdvanced::SetRxrUDPMAC", this,
                 &qTabAdvanced::GetRxrUDPMAC)
}

void qTabAdvanced::SetRxrZMQPort(int port) {
    FILE_LOG(logINFO) << "Setting Receiver ZMQ Port:" << port;
    try {
        det->setRxZmqPort(port, {comboDetector->currentIndex()});
    }
    CATCH_HANDLE("Could not set Receiver ZMQ port.",
                 "qTabAdvanced::SetRxrZMQPort", this,
                 &qTabAdvanced::GetRxrZMQPort)
}

void qTabAdvanced::SetRxrZMQIP() {
    std::string s = dispRxrZMQIP->text().toAscii().constData();
    FILE_LOG(logINFO) << "Setting Receiver ZMQ IP:" << s;
    try {
        det->setRxZmqIP(sls::IpAddr{s}, {comboDetector->currentIndex()});
    }
    CATCH_HANDLE("Could not set Receiver ZMQ IP.", "qTabAdvanced::SetRxrZMQIP",
                 this, &qTabAdvanced::GetRxrZMQIP)
}

void qTabAdvanced::GetROI() {
    FILE_LOG(logDEBUG) << "Getting ROI";
    try {
        slsDetectorDefs::ROI roi =
            det->getROI({comboReadout->currentIndex()})[0];
        spinXmin->setValue(roi.xmin);
        spinXmax->setValue(roi.xmax);
    }
    CATCH_DISPLAY("Could not get ROI.", "qTabAdvanced::GetROI")
}

void qTabAdvanced::ClearROI() {
    FILE_LOG(logINFO) << "Clearing ROI";
    spinXmin->setValue(-1);
    spinXmax->setValue(-1);
    SetROI();
    FILE_LOG(logDEBUG) << "ROIs cleared";
}

void qTabAdvanced::SetROI() {

    slsDetectorDefs::ROI roi;
    roi.xmin = spinXmin->value();
    roi.xmax = spinXmax->value();

    // set roi
    FILE_LOG(logINFO) << "Setting ROI: [" << roi.xmin << ", " << roi.xmax
                      << "]";
    try {
        det->setROI(roi, {comboReadout->currentIndex()});
    }
    CATCH_DISPLAY("Could not set these ROIs.", "qTabAdvanced::SetROI")

    // update corrected list
    GetROI();
}

void qTabAdvanced::GetAllTrimbits() {
    FILE_LOG(logDEBUG) << "Getting all trimbits value";
    disconnect(spinSetAllTrimbits, SIGNAL(editingFinished()), this,
               SLOT(SetAllTrimbits()));

    try {
        int retval = det->getAllTrimbits().squash(-1);
        spinSetAllTrimbits->setValue(retval);
    }
    CATCH_DISPLAY("Could not get all trimbits.", "qTabAdvanced::GetAllTrimbits")

    connect(spinSetAllTrimbits, SIGNAL(editingFinished()), this,
            SLOT(SetAllTrimbits()));
}

void qTabAdvanced::SetAllTrimbits() {
    int value = spinSetAllTrimbits->value();
    FILE_LOG(logINFO) << "Setting all trimbits:" << value;

    try {
        det->setAllTrimbits(value);
    }
    CATCH_HANDLE("Could not set all trimbits.", "qTabAdvanced::SetAllTrimbits",
                 this, &qTabAdvanced::GetAllTrimbits)
}

void qTabAdvanced::GetNumStoragecells() {
    FILE_LOG(logDEBUG) << "Getting number of additional storage cells";
    disconnect(spinNumStoragecells, SIGNAL(valueChanged(int)), this,
               SLOT(SetNumStoragecells(int)));

    try {
        auto retval = det->getNumberOfAdditionalStorageCells().tsquash(
            "Inconsistent values for number of addditional storage cells.");
        spinNumStoragecells->setValue(retval);
    }
    CATCH_DISPLAY("Could not get number of additional storage cells.",
                  "qTabAdvanced::GetNumStoragecells")

    connect(spinNumStoragecells, SIGNAL(valueChanged(int)), this,
            SLOT(SetNumStoragecells(int)));
}

void qTabAdvanced::SetNumStoragecells(int value) {
    FILE_LOG(logINFO) << "Setting number of additional stoarge cells: "
                      << value;
    try {
        det->setNumberOfAdditionalStorageCells(value);
    }
    CATCH_HANDLE("Could not set number of additional storage cells.",
                 "qTabAdvanced::SetNumStoragecells", this,
                 &qTabAdvanced::GetNumStoragecells)
}

void qTabAdvanced::GetSubExposureTime() {
    FILE_LOG(logDEBUG) << "Getting sub exposure time";
    disconnect(spinSubExpTime, SIGNAL(valueChanged(double)), this,
               SLOT(SetSubExposureTime()));
    disconnect(comboSubExpTimeUnit, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetSubExposureTime()));
    try {
        auto retval = det->getSubExptime().tsquash(
            "Subexptime is inconsistent for all detectors.");
        auto time = qDefs::getUserFriendlyTime(retval);
        spinSubExpTime->setValue(time.first);
        comboSubExpTimeUnit->setCurrentIndex(static_cast<int>(time.second));
    }
    CATCH_DISPLAY("Could not get sub exposure time.",
                  "qTabSettings::GetSubExposureTime")
    connect(spinSubExpTime, SIGNAL(valueChanged(double)), this,
            SLOT(SetSubExposureTime()));
    connect(comboSubExpTimeUnit, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetSubExposureTime()));
}

void qTabAdvanced::SetSubExposureTime() {
    auto timeNS = qDefs::getNSTime(std::make_pair(
        spinSubExpTime->value(),
        static_cast<qDefs::timeUnit>(comboSubExpTimeUnit->currentIndex())));
    FILE_LOG(logINFO)
        << "Setting sub frame acquisition time to " << timeNS.count() << " ns"
        << "/" << spinSubExpTime->value()
        << qDefs::getUnitString(
               (qDefs::timeUnit)comboSubExpTimeUnit->currentIndex());
    try {
        det->setSubExptime(timeNS);
    }
    CATCH_DISPLAY("Could not set sub exposure time.",
                  "qTabAdvanced::SetSubExposureTime")

    GetSubExposureTime();
}

void qTabAdvanced::GetSubDeadTime() {
    FILE_LOG(logDEBUG) << "Getting sub dead time";
    disconnect(spinSubDeadTime, SIGNAL(valueChanged(double)), this,
               SLOT(SetSubDeadTime()));
    disconnect(comboSubDeadTimeUnit, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetSubDeadTime()));
    try {
        auto retval = det->getSubDeadTime().tsquash(
            "Sub dead time is inconsistent for all detectors.");
        auto time = qDefs::getUserFriendlyTime(retval);
        spinSubDeadTime->setValue(time.first);
        comboSubDeadTimeUnit->setCurrentIndex(static_cast<int>(time.second));
    }
    CATCH_DISPLAY("Could not get sub dead time.",
                  "qTabSettings::GetSubDeadTime")
    connect(spinSubDeadTime, SIGNAL(valueChanged(double)), this,
            SLOT(SetSubDeadTime()));
    connect(comboSubDeadTimeUnit, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetSubDeadTime()));
}

void qTabAdvanced::SetSubDeadTime() {
    auto timeNS = qDefs::getNSTime(std::make_pair(
        spinSubDeadTime->value(),
        static_cast<qDefs::timeUnit>(comboSubDeadTimeUnit->currentIndex())));

    FILE_LOG(logINFO)
        << "Setting sub frame dead time to " << timeNS.count() << " ns"
        << "/" << spinSubDeadTime->value()
        << qDefs::getUnitString(
               (qDefs::timeUnit)comboSubDeadTimeUnit->currentIndex());
    try {
        det->setSubDeadTime(timeNS);
    }
    CATCH_DISPLAY("Could not set sub dead time.",
                  "qTabAdvanced::SetSubDeadTime")
    GetSubDeadTime();
}

void qTabAdvanced::Refresh() {
    FILE_LOG(logDEBUG) << "**Updating Advanced Tab";

    // trimming
    if (tab_trimming->isEnabled()) {
        GetAllTrimbits();
    }

    // update all network widgets
    SetDetector();

    // roi
    if (tab_roi->isEnabled()) {
        GetROI();
    }

    // storage cells
    if (lblNumStoragecells->isEnabled()) {
        GetNumStoragecells();
    }

    // subexptime, subdeadtime
    if (lblSubExpTime->isEnabled()) {
        GetSubExposureTime();
        GetSubDeadTime();
    }

    FILE_LOG(logDEBUG) << "**Updated Advanced Tab";
}
