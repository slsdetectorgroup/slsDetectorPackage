#include "qTabDebugging.h"
#include "qDefs.h"

#include <QDesktopWidget>
#include <QGridLayout>
#include <QTreeWidget>

#include <iostream>

qTabDebugging::qTabDebugging(QWidget *parent, multiSlsDetector *detector) : 
    QWidget(parent), myDet(detector), treeDet(nullptr), lblDetectorHostname(nullptr), lblDetectorFirmware(nullptr), lblDetectorSoftware(nullptr) {
    setupUi(this);
    SetupWidgetWindow();
    FILE_LOG(logDEBUG) << "Debugging ready";
}

qTabDebugging::~qTabDebugging() {
    if (treeDet)
        delete treeDet;
    if (lblDetectorHostname)
        delete lblDetectorHostname;
    if (lblDetectorFirmware)
        delete lblDetectorFirmware;
    if (lblDetectorSoftware)
        delete lblDetectorSoftware;
}


void qTabDebugging::SetupWidgetWindow() {
	// enabling according to det type
    if (myDet->getDetectorTypeAsEnum() == slsDetectorDefs::EIGER) {
        lblDetector->setText("Half Module:");
        chkDetectorFirmware->setEnabled(false);
        chkDetectorBus->setEnabled(false);
        btnTest->setEnabled(false);
    }

    PopulateDetectors();

    Initialization();

    Refresh();
}


void qTabDebugging::Initialization() {
    connect(comboDetector, SIGNAL(currentIndexChanged(int)), this, SLOT(GetDetectorStatus()));
    connect(btnGetInfo, SIGNAL(clicked()), this, SLOT(GetInfo()));
    if (btnTest ->isEnabled()) {
    	connect(btnTest, SIGNAL(clicked()), this, SLOT(TestDetector()));
    }
}

void qTabDebugging::PopulateDetectors() {
	FILE_LOG(logDEBUG) << "Populating detectors";

	comboDetector->clear();
    for (int i = 0; i < myDet->size(); ++i) {
        comboDetector->addItem(QString(myDet->getHostname(i).c_str()));
    }
}

void qTabDebugging::GetDetectorStatus() {
    FILE_LOG(logDEBUG) << "Getting Status";

	try {
        std::string status = slsDetectorDefs::runStatusType(myDet->getRunStatus(comboDetector->currentIndex()));
        lblStatus->setText(QString(status.c_str()).toUpper());
    } CATCH_DISPLAY ("Could not get detector status.", "qTabDebugging::GetDetectorStatus")
}


void qTabDebugging::GetInfo() {
    FILE_LOG(logDEBUG) << "Getting Readout Info";

    // open info in a new popup
    QFrame *popup1 = new QFrame(this, Qt::Popup | Qt::SubWindow);
    QList<QTreeWidgetItem *> items;
    QGridLayout *layout = new QGridLayout(popup1);
    treeDet = new QTreeWidget(popup1);
    layout->addWidget(treeDet, 0, 0);
    QFrame *dispFrame = new QFrame(popup1);
    QGridLayout *formLayout = new QGridLayout(dispFrame);
    lblDetectorHostname = new QLabel("");
    lblDetectorFirmware = new QLabel("");
    lblDetectorSoftware = new QLabel("");
    //to make sure the size is constant
    lblDetectorFirmware->setFixedWidth(100);
    layout->addWidget(dispFrame, 0, 1);
    QString detName = QString(myDet->getDetectorTypeAsString().c_str());

    switch (myDet->getDetectorTypeAsEnum()) {

    case slsDetectorDefs::EIGER:
        formLayout->addWidget(new QLabel("Half Module:"), 0, 0);
        formLayout->addItem(new QSpacerItem(15, 20, QSizePolicy::Fixed, QSizePolicy::Fixed), 0, 1);
        formLayout->addWidget(lblDetectorHostname, 0, 2);
        formLayout->addWidget(new QLabel("Half Module Firmware Version:"), 1, 0);
        formLayout->addWidget(lblDetectorFirmware, 1, 2);
        formLayout->addWidget(new QLabel("Half Module Software Version:"), 2, 0);
        formLayout->addWidget(lblDetectorSoftware, 2, 2);
        treeDet->setHeaderLabel("Eiger Detector");
        //get num modules
        for (int i = 0; i < comboDetector->count() / 2; ++i)
            items.append(new QTreeWidgetItem((QTreeWidget *)0, QStringList(QString("Module %1").arg(i))));
        treeDet->insertTopLevelItems(0, items);
        //gets det names
        for (int i = 0; i < comboDetector->count(); ++i) {
            QList<QTreeWidgetItem *> childItems;
            childItems.append(new QTreeWidgetItem((QTreeWidget *)0, QStringList(QString("Half Module (%1)").arg(comboDetector->itemText(i)))));
            treeDet->topLevelItem(i * 2)->insertChildren(0, childItems);
        }
        break;

    default:
        formLayout->addWidget(new QLabel("Module:"), 0, 0);
        formLayout->addItem(new QSpacerItem(15, 20, QSizePolicy::Fixed, QSizePolicy::Fixed), 0, 1);
        formLayout->addWidget(lblDetectorHostname, 0, 2);
        formLayout->addWidget(new QLabel("Module Firmware Version:"), 1, 0);
        formLayout->addWidget(lblDetectorFirmware, 1, 2);
        formLayout->addWidget(new QLabel("Module Software Version:"), 2, 0);
        formLayout->addWidget(lblDetectorSoftware, 2, 2);
        treeDet->setHeaderLabel(QString(detName + " Detector"));
        //gets det names
        for (int i = 0; i < comboDetector->count(); ++i)
            items.append(new QTreeWidgetItem((QTreeWidget *)0, QStringList(QString("Module (%1)").arg(comboDetector->itemText(i)))));
        treeDet->insertTopLevelItems(0, items);
        break;
    }

    //show and center widget
    int x = ((parentWidget()->width()) - (popup1->frameGeometry().width())) / 2;
    int y = ((parentWidget()->height()) - (popup1->frameGeometry().height())) / 2;
    QDesktopWidget *desktop = QApplication::desktop();
    int screen = desktop->screenNumber(this);
    popup1->setWindowModality(Qt::WindowModal);
    popup1->move((desktop->screenGeometry(screen).x()) + x, (desktop->screenGeometry(screen).y()) + y);
    popup1->show();

    //put the first parameters
    SetParameters(treeDet->topLevelItem(0));

    // connect to slots
    connect(treeDet, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(SetParameters(QTreeWidgetItem *)));
}


void qTabDebugging::SetParameters(QTreeWidgetItem *item) {
    // eiger: if half module clicked, others: true always
    bool ignoreOrHalfModuleClicked = true;
    if (myDet->getDetectorTypeAsEnum() == slsDetectorDefs::EIGER) {
        if (!(item->text(0).contains("Half Module"))) {
            ignoreOrHalfModuleClicked = false;
        }
    }

    if (ignoreOrHalfModuleClicked) {
        // find index
        for (int i = 0; i < comboDetector->count(); ++i) {
            if (item == treeDet->topLevelItem(i))
                break;
        }
        try {
            auto retval = std::string("0x") + std::to_string((unsigned long)myDet->getId(slsDetectorDefs::DETECTOR_FIRMWARE_VERSION, comboDetector->currentIndex()));
            lblDetectorFirmware->setText(QString(retval.c_str()));
            retval = std::string("0x") + std::to_string((unsigned long)myDet->getId(slsDetectorDefs::DETECTOR_SOFTWARE_VERSION, comboDetector->currentIndex()));
            lblDetectorSoftware->setText(QString(retval.c_str()));
        } CATCH_DISPLAY ("Could not get versions.", "qTabDebugging::SetParameters")
    }
}

void qTabDebugging::TestDetector() {
    FILE_LOG(logINFO) << "Testing Readout";

    try {
        QString moduleName = "Module";
        if (myDet->getDetectorTypeAsEnum() == slsDetectorDefs::EIGER) {
            moduleName = "Half Module";
        }

        // construct message
        QString message = QString("<nobr>Test Results for %1:</nobr><br><br>").arg(comboDetector->currentText());

        //detector firmware
        if (chkDetectorFirmware->isChecked()) {
            auto retval = myDet->digitalTest(slsDetectorDefs::DETECTOR_FIRMWARE_TEST, comboDetector->currentIndex());
            if (retval == slsDetectorDefs::FAIL) {
                message.append(QString("<nobr>%1 Firmware: FAIL</nobr><br>").arg(moduleName));
                FILE_LOG(logERROR) << "Firmware fail";
            }
            else
                message.append(QString("<nobr>%1 Firmware: %2</nobr><br>").arg(moduleName, QString::number(retval)));
            FILE_LOG(logINFO) << "Detector Firmware Test: " << retval;
        }

        //detector CPU-FPGA bus
        if (chkDetectorBus->isChecked()) {
            auto retval = myDet->digitalTest(slsDetectorDefs::DETECTOR_BUS_TEST, comboDetector->currentIndex());
            if (retval == slsDetectorDefs::FAIL) {
                message.append(QString("<nobr>%1 Bus: &nbsp;&nbsp;&nbsp;&nbsp;FAIL</nobr><br>").arg(moduleName));
                FILE_LOG(logERROR) << "Bus Test fail";
            } else
                message.append(QString("<nobr>%1 Bus: &nbsp;&nbsp;&nbsp;&nbsp;%2</nobr><br>").arg(moduleName, QString::number(retval)));
            FILE_LOG(logINFO) << "Detector Bus Test: " << retval;
        }

        //display message
        qDefs::Message(qDefs::INFORMATION, message.toAscii().constData(), "qTabDebugging::TestDetector");
    } CATCH_DISPLAY ("Could not execute digital test.", "qTabDebugging::TestDetector")
}


void qTabDebugging::Refresh() {
    FILE_LOG(logDEBUG) << "**Updating Debugging Tab";
    GetDetectorStatus();
    FILE_LOG(logDEBUG) << "**Updated Debugging Tab";
}

