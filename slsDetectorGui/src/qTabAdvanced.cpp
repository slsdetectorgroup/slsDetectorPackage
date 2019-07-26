#include "qTabAdvanced.h"
#include "qDefs.h"

#include <iostream>

qTabAdvanced::qTabAdvanced(QWidget *parent, multiSlsDetector *detector)
    : QWidget(parent), myDet(detector) {
    setupUi(this);
    SetupWidgetWindow();
    FILE_LOG(logDEBUG) << "Advanced ready";
}

qTabAdvanced::~qTabAdvanced() {
    for (size_t i = 0; i < lblFromX.size(); ++i) {
        delete lblFromX[i];
        delete lblFromY[i];
        delete lblToX[i];
        delete lblToY[i];
        delete spinFromX[i];
        delete spinFromY[i];
        delete spinToX[i];
        delete spinToY[i];
    }
}

void qTabAdvanced::SetupWidgetWindow() {
    // enabling according to det type
    switch (myDet->getDetectorTypeAsEnum()) {
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
            SLOT(SetDetector(int)));
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
        connect(btnAddRoi, SIGNAL(clicked()), this, SLOT(AddROISlot()));
        connect(btnSetRoi, SIGNAL(clicked()), this, SLOT(SetROI()));
        connect(btnGetRoi, SIGNAL(clicked()), this, SLOT(GetROI()));
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
}

void qTabAdvanced::PopulateDetectors() {
    FILE_LOG(logDEBUG) << "Populating detectors";
    disconnect(comboDetector, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetDetector(int)));

    comboDetector->clear();
    for (int i = 0; i < myDet->getNumberOfDetectors(); ++i)
        comboDetector->addItem(QString(myDet->getHostname(i).c_str()));
    comboDetector->setCurrentIndex(0);

    connect(comboDetector, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetDetector(int)));
}

void qTabAdvanced::GetControlPort() {
    FILE_LOG(logDEBUG) << "Getting control port ";
    disconnect(spinControlPort, SIGNAL(valueChanged(int)), this,
               SLOT(SetControlPort(int)));

    try {
        int retval = myDet->setControlPort(-1, comboDetector->currentIndex());
        spinControlPort->setValue(retval);
    } CATCH_DISPLAY ("Could not get detector control port.", "qTabAdvanced::GetControlPort")

    connect(spinControlPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetControlPort(int)));
}

void qTabAdvanced::GetStopPort() {
    FILE_LOG(logDEBUG) << "Getting stop port";
    disconnect(spinStopPort, SIGNAL(valueChanged(int)), this,
               SLOT(SetStopPort(int)));

    try {
        int retval = myDet->setStopPort(-1, comboDetector->currentIndex());
        spinStopPort->setValue(retval);
    } CATCH_DISPLAY ("Could not get detector stop port.", "qTabAdvanced::GetStopPort")

    connect(spinStopPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetStopPort(int)));
}

void qTabAdvanced::GetDetectorUDPIP() {
    FILE_LOG(logDEBUG) << "Getting Detector UDP IP";
    disconnect(dispDetectorUDPIP, SIGNAL(editingFinished()), this,
               SLOT(SetDetectorUDPIP()));

    try {
        auto retval = myDet->getDetectorIP(comboDetector->currentIndex());
        dispDetectorUDPIP->setText(QString(retval.c_str()));
    } CATCH_DISPLAY ("Could not get detector UDP IP.", "qTabAdvanced::GetDetectorUDPIP")

    connect(dispDetectorUDPIP, SIGNAL(editingFinished()), this,
            SLOT(SetDetectorUDPIP()));
}

void qTabAdvanced::GetDetectorUDPMAC() {
    FILE_LOG(logDEBUG) << "Getting Detector UDP MAC";
    disconnect(dispDetectorUDPMAC, SIGNAL(editingFinished()), this,
               SLOT(SetDetectorUDPMAC()));

    try {
        auto retval = myDet->getDetectorMAC(comboDetector->currentIndex());
        dispDetectorUDPMAC->setText(QString(retval.c_str()));
    } CATCH_DISPLAY ("Could not get detector UDP MAC.", "qTabAdvanced::GetDetectorUDPMAC")

    connect(dispDetectorUDPMAC, SIGNAL(editingFinished()), this,
            SLOT(SetDetectorUDPMAC()));
}

void qTabAdvanced::GetCltZMQPort() {
    FILE_LOG(logDEBUG) << "Getting Client ZMQ port";
    disconnect(spinZMQPort, SIGNAL(valueChanged(int)), this,
               SLOT(SetCltZMQPort(int)));

    try {
        int retval =
            myDet->getClientStreamingPort(comboDetector->currentIndex());
        spinZMQPort->setValue(retval);
    } CATCH_DISPLAY ("Could not get client zmq port.", "qTabAdvanced::GetCltZMQPort")

    connect(spinZMQPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetCltZMQPort(int)));
}

void qTabAdvanced::GetCltZMQIP() {
    FILE_LOG(logDEBUG) << "Getting Client ZMQ IP";
    disconnect(dispZMQIP, SIGNAL(editingFinished()), this, SLOT(SetCltZMQIP()));

    try {
        auto retval =
            myDet->getClientStreamingIP(comboDetector->currentIndex());
        dispZMQIP->setText(QString(retval.c_str()));
    } CATCH_DISPLAY ("Could not get client zmq ip.", "qTabAdvanced::GetCltZMQIP")

    connect(dispZMQIP, SIGNAL(editingFinished()), this, SLOT(SetCltZMQIP()));
}

void qTabAdvanced::GetRxrHostname() {
    FILE_LOG(logDEBUG) << "Getting Receiver Hostname";
    disconnect(dispRxrHostname, SIGNAL(editingFinished()), this,
               SLOT(SetRxrHostname()));

    try {
        auto retval = myDet->getReceiverHostname(comboDetector->currentIndex());
        dispRxrHostname->setText(QString(retval.c_str()));
    } CATCH_DISPLAY ("Could not get receiver hostname.", "qTabAdvanced::GetRxrHostname")

    connect(dispRxrHostname, SIGNAL(editingFinished()), this,
            SLOT(SetRxrHostname()));
}

void qTabAdvanced::GetRxrTCPPort() {
    FILE_LOG(logDEBUG) << "Getting Receiver TCP port";
    disconnect(spinRxrTCPPort, SIGNAL(valueChanged(int)), this,
               SLOT(SetRxrTCPPort(int)));

    try {
        int retval = myDet->getReceiverPort(comboDetector->currentIndex());
        spinRxrTCPPort->setValue(retval);
    } CATCH_DISPLAY ("Could not get receiver tcp port.", "qTabAdvanced::GetRxrTCPPort")

    connect(spinRxrTCPPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetRxrTCPPort(int)));
}

void qTabAdvanced::GetRxrUDPPort() {
    FILE_LOG(logDEBUG) << "Getting Receiver UDP port";
    disconnect(spinRxrUDPPort, SIGNAL(valueChanged(int)), this,
               SLOT(SetRxrUDPPort(int)));

    try {
        int retval = myDet->getReceiverUDPPort(comboDetector->currentIndex());
        spinRxrUDPPort->setValue(retval);
    } CATCH_DISPLAY ("Could not get receiver udp port.", "qTabAdvanced::GetRxrUDPPort")

    connect(spinRxrUDPPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetRxrUDPPort(int)));
}

void qTabAdvanced::GetRxrUDPIP() {
    FILE_LOG(logDEBUG) << "Getting Receiver UDP IP";
    disconnect(dispRxrUDPIP, SIGNAL(editingFinished()), this,
               SLOT(SetRxrUDPIP()));

    try {
        auto retval = myDet->getReceiverUDPIP(comboDetector->currentIndex());
        dispRxrUDPIP->setText(QString(retval.c_str()));
    } CATCH_DISPLAY ("Could not get receiver udp ip.", "qTabAdvanced::GetRxrUDPIP")

    connect(dispRxrUDPIP, SIGNAL(editingFinished()), this, SLOT(SetRxrUDPIP()));
}

void qTabAdvanced::GetRxrUDPMAC() {
    FILE_LOG(logDEBUG) << "Getting Receiver UDP MAC";
    disconnect(dispRxrUDPMAC, SIGNAL(editingFinished()), this,
               SLOT(SetRxrUDPMAC()));

    try {
        auto retval = myDet->getReceiverUDPMAC(comboDetector->currentIndex());
        dispRxrUDPMAC->setText(QString(retval.c_str()));
    } CATCH_DISPLAY ("Could not get receiver udp mac.", "qTabAdvanced::GetRxrUDPMAC")

    connect(dispRxrUDPMAC, SIGNAL(editingFinished()), this,
            SLOT(SetRxrUDPMAC()));
}

void qTabAdvanced::GetRxrZMQPort() {
    FILE_LOG(logDEBUG) << "Getting Receiver ZMQ port";
    disconnect(spinRxrZMQPort, SIGNAL(valueChanged(int)), this,
               SLOT(SetRxrZMQPort(int)));

    try {
        int retval =
            myDet->getReceiverStreamingPort(comboDetector->currentIndex());
        spinRxrZMQPort->setValue(retval);
    } CATCH_DISPLAY ("Could not get receiver zmq port.", "qTabAdvanced::GetRxrZMQPort")

    connect(spinRxrZMQPort, SIGNAL(valueChanged(int)), this,
            SLOT(SetRxrZMQPort(int)));
}

void qTabAdvanced::GetRxrZMQIP() {
    FILE_LOG(logDEBUG) << "Getting Receiver ZMQ IP";
    disconnect(dispRxrZMQIP, SIGNAL(editingFinished()), this,
               SLOT(SetRxrZMQIP()));

    try {
        auto retval =
            myDet->getReceiverStreamingIP(comboDetector->currentIndex());
        dispRxrZMQIP->setText(QString(retval.c_str()));
    } CATCH_DISPLAY ("Could not get receiver zmq ip.", "qTabAdvanced::GetRxrZMQIP")

    connect(dispRxrZMQIP, SIGNAL(editingFinished()), this, SLOT(SetRxrZMQIP()));
}

void qTabAdvanced::SetDetector(int index) {
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

    myDet->printReceiverConfiguration(logDEBUG);
}

void qTabAdvanced::SetControlPort(int port) {
    FILE_LOG(logINFO) << "Setting Control Port:" << port;
    try {
        myDet->setControlPort(port, comboDetector->currentIndex());
    } CATCH_HANDLE("Could not set control port.", "qTabAdvanced::SetControlPort",
                 this, &qTabAdvanced::GetControlPort)
}

void qTabAdvanced::SetStopPort(int port) {
    FILE_LOG(logINFO) << "Setting Stop Port:" << port;
    try {
        myDet->setStopPort(port, comboDetector->currentIndex());
    } CATCH_HANDLE ("Could not set stop port.", "qTabAdvanced::SetStopPort", this,
                                &qTabAdvanced::GetStopPort)
}

void qTabAdvanced::SetDetectorUDPIP() {
    std::string s = dispDetectorUDPIP->text().toAscii().constData();
    FILE_LOG(logINFO) << "Setting Detector UDP IP:" << s;
    try {
        myDet->setDetectorIP(s, comboDetector->currentIndex());
    } CATCH_HANDLE ("Could not set Detector UDP IP.",
                                "qTabAdvanced::SetDetectorUDPIP", this,
                                &qTabAdvanced::GetDetectorUDPIP)
}

void qTabAdvanced::SetDetectorUDPMAC() {
    std::string s = dispDetectorUDPMAC->text().toAscii().constData();
    FILE_LOG(logINFO) << "Setting Detector UDP MAC:" << s;
    try {
        myDet->setDetectorMAC(s, comboDetector->currentIndex());
    } CATCH_HANDLE ("Could not set Detector UDP MAC.",
                                "qTabAdvanced::SetDetectorUDPMAC", this,
                                &qTabAdvanced::GetDetectorUDPMAC)
}

void qTabAdvanced::SetCltZMQPort(int port) {
    FILE_LOG(logINFO) << "Setting Client ZMQ Port:" << port;
    try {
        myDet->setClientDataStreamingInPort(port,
                                            comboDetector->currentIndex());
    } CATCH_HANDLE ("Could not set Client ZMQ port.",
                                "qTabAdvanced::SetCltZMQPort", this,
                                &qTabAdvanced::GetCltZMQPort)
}

void qTabAdvanced::SetCltZMQIP() {
    std::string s = dispZMQIP->text().toAscii().constData();
    FILE_LOG(logINFO) << "Setting Client ZMQ IP:" << s;
    try {
        myDet->setClientDataStreamingInIP(s, comboDetector->currentIndex());
    } CATCH_HANDLE ("Could not set Client ZMQ IP.",
                                "qTabAdvanced::SetCltZMQIP", this,
                                &qTabAdvanced::GetCltZMQIP)
}

void qTabAdvanced::SetRxrHostname() {
    std::string s = dispZMQIP->text().toAscii().constData();
    FILE_LOG(logINFO) << "Setting Receiver Hostname:" << s;
    try {
        myDet->setReceiverHostname(s, comboDetector->currentIndex());
    } CATCH_HANDLE ("Could not set Client ZMQ IP.",
                                "qTabAdvanced::SetRxrHostname", this,
                                &qTabAdvanced::GetRxrHostname)

    // update all network widgets (receiver mainly)
    SetDetector(comboDetector->currentIndex());
}

void qTabAdvanced::SetRxrTCPPort(int port) {
    FILE_LOG(logINFO) << "Setting Receiver TCP Port:" << port;
    try {
        myDet->setReceiverPort(port, comboDetector->currentIndex());
    } CATCH_HANDLE ("Could not set Receiver TCP port.",
                                "qTabAdvanced::SetRxrTCPPort", this,
                                &qTabAdvanced::GetRxrTCPPort)
}

void qTabAdvanced::SetRxrUDPPort(int port) {
    FILE_LOG(logINFO) << "Setting Receiver UDP Port:" << port;
    try {
        myDet->setReceiverUDPPort(port, comboDetector->currentIndex());
    } CATCH_HANDLE ("Could not set Receiver UDP port.",
                                "qTabAdvanced::SetRxrUDPPort", this,
                                &qTabAdvanced::GetRxrUDPPort)
}

void qTabAdvanced::SetRxrUDPIP() {
    std::string s = dispRxrUDPIP->text().toAscii().constData();
    FILE_LOG(logINFO) << "Setting Receiver UDP IP:" << s;
    try {
        myDet->setReceiverUDPIP(s, comboDetector->currentIndex());
    } CATCH_HANDLE ("Could not set Receiver UDP IP.",
                                "qTabAdvanced::SetRxrUDPIP", this,
                                &qTabAdvanced::GetRxrUDPIP)
}

void qTabAdvanced::SetRxrUDPMAC() {
    std::string s = dispRxrUDPMAC->text().toAscii().constData();
    FILE_LOG(logINFO) << "Setting Receiver UDP MAC:" << s;
    try {
        myDet->setReceiverUDPMAC(s, comboDetector->currentIndex());
    } CATCH_HANDLE ("Could not set Receiver UDP MAC.",
                                "qTabAdvanced::SetRxrUDPMAC", this,
                                &qTabAdvanced::GetRxrUDPMAC)
}

void qTabAdvanced::SetRxrZMQPort(int port) {
    FILE_LOG(logINFO) << "Setting Receiver ZMQ Port:" << port;
    try {
        myDet->setReceiverDataStreamingOutPort(port,
                                               comboDetector->currentIndex());
    } CATCH_HANDLE ("Could not set Receiver ZMQ port.",
                                "qTabAdvanced::SetRxrZMQPort", this,
                                &qTabAdvanced::GetRxrZMQPort)
}

void qTabAdvanced::SetRxrZMQIP() {
    std::string s = dispRxrZMQIP->text().toAscii().constData();
    FILE_LOG(logINFO) << "Setting Receiver ZMQ IP:" << s;
    try {
        myDet->setReceiverDataStreamingOutIP(s, comboDetector->currentIndex());
    } CATCH_HANDLE ("Could not set Receiver ZMQ IP.",
                                "qTabAdvanced::SetRxrZMQIP", this,
                                &qTabAdvanced::GetRxrZMQIP)
}

void qTabAdvanced::AddROISlot() {
    FILE_LOG(logDEBUG) << "Add ROI Slot";

    QLabel *lFromX = new QLabel("x min:");
    QLabel *lFromY = new QLabel("y min:");
    QLabel *lToX = new QLabel("x max:");
    QLabel *lToY = new QLabel("y max:");
    QSpinBox *sFromX = new QSpinBox();
    QSpinBox *sFromY = new QSpinBox();
    QSpinBox *sToX = new QSpinBox();
    QSpinBox *sToY = new QSpinBox();
    lFromX->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lFromY->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lToX->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lToY->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sFromX->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sFromY->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sToX->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sToY->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lFromX->setFixedWidth(50);
    lFromY->setFixedWidth(50);
    lToX->setFixedWidth(50);
    lToY->setFixedWidth(50);
    sFromX->setFixedWidth(80);
    sFromY->setFixedWidth(80);
    sToX->setFixedWidth(80);
    sToY->setFixedWidth(80);
    sFromX->setFixedHeight(19);
    sFromY->setFixedHeight(19);
    sToX->setFixedHeight(19);
    sToY->setFixedHeight(19);
    sFromX->setMaximum(myDet->getTotalNumberOfChannels(slsDetectorDefs::X) - 1);
    sToX->setMaximum(myDet->getTotalNumberOfChannels(slsDetectorDefs::X) - 1);
    sFromY->setMaximum(myDet->getTotalNumberOfChannels(slsDetectorDefs::Y) - 1);
    sToY->setMaximum(myDet->getTotalNumberOfChannels(slsDetectorDefs::Y) - 1);
    sFromX->setMinimum(-1);
    sToX->setMinimum(-1);
    sFromY->setMinimum(-1);
    sToY->setMinimum(-1);
    sFromX->setValue(-1);
    sFromY->setValue(-1);
    sToX->setValue(-1);
    sToY->setValue(-1);

    lblFromX.push_back(lFromX);
    lblFromY.push_back(lFromY);
    lblToX.push_back(lToX);
    lblToY.push_back(lToY);
    spinFromX.push_back(sFromX);
    spinFromY.push_back(sFromY);
    spinToX.push_back(sToX);
    spinToY.push_back(sToY);

    int nroi = (int)lblFromX.size();
    gridRoi->addWidget(lblFromX[nroi], nroi, 0, Qt::AlignTop);
    gridRoi->addWidget(spinFromX[nroi], nroi, 1, Qt::AlignTop);
    // FIXME: gridRoi->addItem(new
    // QSpacerItem(40,20,QSizePolicy::Expanding,QSizePolicy::Fixed),
    // nroi,2,Qt::AlignTop);
    gridRoi->addWidget(lblToX[nroi], nroi, 3, Qt::AlignTop);
    gridRoi->addWidget(spinToX[nroi], nroi, 4, Qt::AlignTop);
    // FIXME: gridRoi->addItem(new
    // QSpacerItem(40,20,QSizePolicy::Expanding,QSizePolicy::Fixed),
    // nroi,5,Qt::AlignTop);
    gridRoi->addWidget(lblFromY[nroi], nroi, 6, Qt::AlignTop);
    gridRoi->addWidget(spinFromY[nroi], nroi, 7, Qt::AlignTop);
    // FIXME: gridRoi->addItem(new
    // QSpacerItem(40,20,QSizePolicy::Expanding,QSizePolicy::Fixed),
    // nroi,8,Qt::AlignTop);
    gridRoi->addWidget(lblToY[nroi], nroi, 9, Qt::AlignTop);
    gridRoi->addWidget(spinToY[nroi], nroi, 10, Qt::AlignTop);

    lblFromX[nroi]->show();
    spinFromX[nroi]->show();
    lblToX[nroi]->show();
    spinToX[nroi]->show();
    lblFromY[nroi]->show();
    spinFromY[nroi]->show();
    lblToY[nroi]->show();
    spinToY[nroi]->show();

    FILE_LOG(logDEBUG) << "ROI Inputs added";
}

void qTabAdvanced::GetROI() {
    FILE_LOG(logDEBUG) << "Getting ROI";
    ClearROIWidgets();

    try {
        int nroi = 0;
        const slsDetectorDefs::ROI *roi = myDet->getROI(nroi);
        if (roi != nullptr) {
            for (int i = 0; i < nroi; ++i) {
                AddROISlot();
                spinFromX[i]->setValue(roi[i].xmin);
                spinFromY[i]->setValue(roi[i].ymin);
                spinToX[i]->setValue(roi[i].xmax);
                spinToY[i]->setValue(roi[i].ymax);
            }
            FILE_LOG(logDEBUG) << "ROIs populated: " << nroi;
        }

    } CATCH_DISPLAY ("Could not get ROI.", "qTabAdvanced::GetROI")
}

void qTabAdvanced::ClearROIWidgets() {
    FILE_LOG(logDEBUG) << "Clear ROI Widgets";

    // hide widgets
    QLayoutItem *item;
    while ((item = gridRoi->takeAt(0))) {
        if (item->widget()) {
            item->widget()->hide();
            gridRoi->removeWidget(item->widget());
        }
    }

    // delete widgets
    for (size_t i = 0; i < lblFromX.size(); ++i) {
        delete lblFromX[i];
        delete spinFromX[i];
        delete lblToX[i];
        delete spinToY[i];
        delete lblFromY[i];
        delete spinFromY[i];
        delete lblToY[i];
        delete spinToY[i];
    }
    lblFromX.clear();
    spinFromX.clear();
    lblToX.clear();
    spinToY.clear();
    lblFromY.clear();
    spinFromY.clear();
    lblToY.clear();
    spinToY.clear();
}

void qTabAdvanced::ClearROI() {
    FILE_LOG(logINFO) << "Clearing ROI";
    if (QMessageBox::warning(
            this, "Clear ROI",
            "Are you sure you want to clear all the ROI in detector?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No) == QMessageBox::Yes) {

        ClearROIWidgets();
        SetROI();
        FILE_LOG(logDEBUG) << "ROIs cleared";
    }
}

void qTabAdvanced::SetROI() {
    // get roi from widgets
    int nroi = (int)lblFromX.size();
    slsDetectorDefs::ROI roi[nroi];
    for (int i = 0; i < nroi; ++i) {
        roi[i].xmin = spinFromX[i]->value();
        roi[i].ymin = spinFromY[i]->value();
        roi[i].xmax = spinToX[i]->value();
        roi[i].ymax = spinToY[i]->value();
    }

    // set roi
    FILE_LOG(logINFO) << "Setting ROI:" << nroi;
    try {
        myDet->setROI(nroi, roi, -1);
    } CATCH_DISPLAY ("Could not set these ROIs.",
                                "qTabAdvanced::SetROI")

    // update corrected list
    GetROI();
}

void qTabAdvanced::GetAllTrimbits() {
    FILE_LOG(logDEBUG) << "Getting all trimbits value";
    disconnect(spinSetAllTrimbits, SIGNAL(editingFinished()), this,
               SLOT(SetAllTrimbits()));

    try {
        int retval = myDet->setAllTrimbits(-1);
        spinSetAllTrimbits->setValue(retval);
    } CATCH_DISPLAY ("Could not get all trimbits.", "qTabAdvanced::GetAllTrimbits")

    connect(spinSetAllTrimbits, SIGNAL(editingFinished()), this,
            SLOT(SetAllTrimbits()));
}

void qTabAdvanced::SetAllTrimbits() {
    int value = spinSetAllTrimbits->value();
    FILE_LOG(logINFO) << "Setting all trimbits:" << value;

    try {
        myDet->setAllTrimbits(value);
    } CATCH_HANDLE("Could not set all trimbits.", "qTabAdvanced::SetAllTrimbits",
                 this, &qTabAdvanced::GetAllTrimbits)
}

void qTabAdvanced::GetNumStoragecells() {
    FILE_LOG(logDEBUG) << "Getting number of additional storage cells";
    disconnect(spinNumStoragecells, SIGNAL(valueChanged(int)), this,
               SLOT(SetNumStoragecells(int)));

    try {
        auto retval = myDet->setTimer(slsDetectorDefs::STORAGE_CELL_NUMBER);
        spinNumStoragecells->setValue(retval);
    } CATCH_DISPLAY (
            "Could not get number of additional storage cells.",
            "qTabAdvanced::GetNumStoragecells")

    connect(spinNumStoragecells, SIGNAL(valueChanged(int)), this,
            SLOT(SetNumStoragecells(int)));
}

void qTabAdvanced::SetNumStoragecells(int value) {
    FILE_LOG(logINFO) << "Setting number of additional stoarge cells: "
                      << value;
    try {
        myDet->setTimer(slsDetectorDefs::STORAGE_CELL_NUMBER, value, -1);
    } CATCH_HANDLE (
            "Could not set number of additional storage cells.",
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
        int64_t retval =
            myDet->setTimer(slsDetectorDefs::SUBFRAME_ACQUISITION_TIME);
        if (retval == -1) {
            qDefs::Message(qDefs::WARNING,
                           "Subexptime is inconsistent for all detectors.",
                           "qTabAdvanced::GetSubExposureTime");
            spinSubExpTime->setValue(-1);
        } else {
            double value = (double)(retval * (1E-9));
            auto time = qDefs::getCorrectTime(value);
            spinSubExpTime->setValue(time.first);
            comboSubExpTimeUnit->setCurrentIndex(static_cast<int>(time.second));
        }
    } CATCH_DISPLAY ("Could not get sub exposure time.",
                                "qTabSettings::GetSubExposureTime")

    connect(spinSubExpTime, SIGNAL(valueChanged(double)), this,
            SLOT(SetSubExposureTime()));
    connect(comboSubExpTimeUnit, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetSubExposureTime()));
}

void qTabAdvanced::SetSubExposureTime() {
    double timeNS =
        qDefs::getNSTime((qDefs::timeUnit)comboSubExpTimeUnit->currentIndex(),
                         spinSubExpTime->value());
    FILE_LOG(logINFO)
        << "Setting sub frame acquisition time to " << timeNS << " ns"
        << "/" << spinSubExpTime->value()
        << qDefs::getUnitString(
               (qDefs::timeUnit)comboSubExpTimeUnit->currentIndex());
    try {
        myDet->setTimer(slsDetectorDefs::SUBFRAME_ACQUISITION_TIME,
                        (int64_t)timeNS, -1);
    } CATCH_DISPLAY ("Could not set sub exposure time.",
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
        int64_t retval = myDet->setTimer(slsDetectorDefs::SUBFRAME_DEADTIME);
        if (retval == -1) {
            qDefs::Message(qDefs::WARNING,
                           "Sub dead time is inconsistent for all detectors.",
                           "qTabAdvanced::GetSubDeadTime");
            spinSubDeadTime->setValue(-1);
        } else {
            double value = (double)(retval * (1E-9));
            auto time = qDefs::getCorrectTime(value);
            spinSubDeadTime->setValue(time.first);
            comboSubDeadTimeUnit->setCurrentIndex(
                static_cast<int>(time.second));
        }
    } CATCH_DISPLAY ("Could not get sub dead time.",
                                "qTabSettings::GetSubDeadTime")

    connect(spinSubDeadTime, SIGNAL(valueChanged(double)), this,
            SLOT(SetSubDeadTime()));
    connect(comboSubDeadTimeUnit, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetSubDeadTime()));
}

void qTabAdvanced::SetSubDeadTime() {
    double timeNS =
        qDefs::getNSTime((qDefs::timeUnit)comboSubDeadTimeUnit->currentIndex(),
                         spinSubDeadTime->value());
    FILE_LOG(logINFO)
        << "Setting sub frame dead time to " << timeNS << " ns"
        << "/" << spinSubDeadTime->value()
        << qDefs::getUnitString(
               (qDefs::timeUnit)comboSubDeadTimeUnit->currentIndex());
    try {
        myDet->setTimer(slsDetectorDefs::SUBFRAME_DEADTIME, (int64_t)timeNS,
                        -1);
    } CATCH_DISPLAY ("Could not set sub dead time.",
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
    SetDetector(comboDetector->currentIndex());

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
