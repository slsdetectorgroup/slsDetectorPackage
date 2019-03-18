#include "qTabDebugging.h"

#include "multiSlsDetector.h"

#include <QDesktopWidget>
#include <QGridLayout>

#include <iostream>

//-------------------------------------------------------------------------------------------------------------------------------------------------

qTabDebugging::qTabDebugging(QWidget *parent, multiSlsDetector *&detector) : QWidget(parent),
                                                                             myDet(detector),
                                                                             det(0),
																			 detType(0),
                                                                             treeDet(0),
                                                                             lblDetectorId(0),
                                                                             lblDetectorSerial(0),
                                                                             lblDetectorFirmware(0),
                                                                             lblDetectorSoftware(0) {
    setupUi(this);
    SetupWidgetWindow();
    Initialization();
    FILE_LOG(logDEBUG) << "Debugging ready";
}

qTabDebugging::~qTabDebugging() {
    delete myDet;
    if (det)
        delete det;
}


void qTabDebugging::SetupWidgetWindow() {

    blue = new QPalette();
    blue->setColor(QPalette::Active, QPalette::WindowText, Qt::darkBlue);

    // Detector Type
    detType = myDet->getDetectorTypeAsEnum();

    // rename label and disable tests
    if (detType == slsDetectorDefs::EIGER) {
        lblDetector->setText("Half Module:");
        chkDetectorFirmware->setEnabled(false);
        chkDetectorBus->setEnabled(false);
        btnTest->setEnabled(false);
    }

    //add detectors
    for (int i = 0; i < myDet->getNumberOfDetectors(); ++i) {
        comboDetector->addItem(QString(myDet->getHostname(i).c_str()));
    }

    UpdateStatus();

    qDefs::checkErrorMessage(myDet, "qTabDebugging::SetupWidgetWindow");
}


void qTabDebugging::Initialization() {

    connect(comboDetector, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateStatus()));
    connect(btnGetInfo, SIGNAL(clicked()), this, SLOT(GetInfo()));
    if (btnTest ->isEnabled())
    	connect(btnTest, SIGNAL(clicked()), this, SLOT(TestDetector()));
}


void qTabDebugging::UpdateStatus() {
    FILE_LOG(logDEBUG) << "Getting Status";

    auto moduleId = comboDetector->currentIndex();
    int detStatus = (int)myDet->getRunStatus(moduleId);
    std::string status = slsDetectorDefs::runStatusType(slsDetectorDefs::runStatus(detStatus));
    lblStatus->setText(QString(status.c_str()).toUpper());

    qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabDebugging::UpdateStatus");
}


void qTabDebugging::GetInfo() {
    FILE_LOG(logDEBUG) << "Getting Readout Info";

    //window
    QFrame *popup1 = new QFrame(this, Qt::Popup | Qt::SubWindow);
    QList<QTreeWidgetItem *> items;

    //layout
    QGridLayout *layout = new QGridLayout(popup1);
    //treewidget
    treeDet = new QTreeWidget(popup1);
    layout->addWidget(treeDet, 0, 0);
    //display the details
    QFrame *dispFrame = new QFrame(popup1);
    QGridLayout *formLayout = new QGridLayout(dispFrame);
    // hostname
    lblDetectorId = new QLabel("");
    lblDetectorId->setPalette(*blue);
    // firmware version
    lblDetectorFirmware = new QLabel("");
    lblDetectorFirmware->setPalette(*blue);
    // software version
    lblDetectorSoftware = new QLabel("");
    lblDetectorSoftware->setPalette(*blue);
    //to make sure the size is constant
    lblDetectorFirmware->setFixedWidth(100);
    layout->addWidget(dispFrame, 0, 1);

    QString detName = myDet->getDetectorTypeAsString();

    switch (detType) {


    case slsDetectorDefs::EIGER:
        //display widget
        formLayout->addWidget(new QLabel("Half Module:"), 0, 0);
        formLayout->addItem(new QSpacerItem(15, 20, QSizePolicy::Fixed, QSizePolicy::Fixed), 0, 1);
        formLayout->addWidget(lblDetectorId, 0, 2);
        formLayout->addWidget(new QLabel("Half Module Firmware Version:"), 1, 0);
        formLayout->addWidget(lblDetectorFirmware, 1, 2);
        formLayout->addWidget(new QLabel("Half Module Software Version:"), 2, 0);
        formLayout->addWidget(lblDetectorSoftware, 2, 2);

        //tree widget
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

        //display widget
        formLayout->addWidget(new QLabel("Module:"), 0, 0);
        formLayout->addItem(new QSpacerItem(15, 20, QSizePolicy::Fixed, QSizePolicy::Fixed), 0, 1);
        formLayout->addWidget(lblDetectorId, 0, 2);
        formLayout->addWidget(new QLabel("Module Firmware Version:"), 1, 0);
        formLayout->addWidget(lblDetectorFirmware, 1, 2);
        formLayout->addWidget(new QLabel("Module Software Version:"), 2, 0);
        formLayout->addWidget(lblDetectorSoftware, 2, 2);
        //tree widget
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

    //initializations
    connect(treeDet, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(SetParameters(QTreeWidgetItem *)));
}


void qTabDebugging::SetParameters(QTreeWidgetItem *item) {
    char value[200];
    int i;

    auto moduleId = comboDetector->currentIndex();
    switch (detType) {


    case slsDetectorDefs::EIGER:
        //only if half module clicked
        if (item->text(0).contains("Half Module")) {
            //find index
            for (i = 0; i < comboDetector->count(); ++i)
                if (item == treeDet->topLevelItem(i))
                    break;

            sprintf(value, "%lx", (long long unsigned int)myDet->getId(slsDetectorDefs::DETECTOR_FIRMWARE_VERSION, moduleId));
            lblDetectorFirmware->setText(QString(value));
            sprintf(value, "%lx", (long long unsigned int)myDet->getId(slsDetectorDefs::DETECTOR_SOFTWARE_VERSION, moduleId));
            lblDetectorSoftware->setText(QString(value));

            qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabDebugging::SetParameters");
        }
        break;

    default:
        //find index
        for (i = 0; i < comboDetector->count(); ++i)
            if (item == treeDet->topLevelItem(i))
                break;

        sprintf(value, "%lx", (long long unsigned int)myDet->getId(slsDetectorDefs::DETECTOR_FIRMWARE_VERSION, moduleId));
        lblDetectorFirmware->setText(QString(value));
        sprintf(value, "%lx", (long long unsigned int)myDet->getId(slsDetectorDefs::DETECTOR_SOFTWARE_VERSION, moduleId));
        lblDetectorSoftware->setText(QString(value));

        qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabDebugging::SetParameters");
        break;

    default:
        break;
    }
}


void qTabDebugging::TestDetector() {
    FILE_LOG(logINFO) << "Testing Readout";


    int retval = slsDetectorDefs::FAIL;
    QString message;
    QString Detector = "Detector";

    //main messagebox title
    switch (detType) {
    case slsDetectorDefs::EIGER:
        Detector = "Half Module";
        break;
    default:
        Detector = "Module";
        break;
    default:
        break;
    }

    // construct message
    message = QString("<nobr>Test Results for %1:</nobr><br><br>").arg(comboDetector->currentText());

    auto moduleId = comboDetector->currentIndex();

    //detector firmware
    if (chkDetectorFirmware->isChecked()) {
        retval = myDet->digitalTest(slsDetectorDefs::DETECTOR_FIRMWARE_TEST, moduleId);
        if (retval == slsDetectorDefs::FAIL) {
            message.append(QString("<nobr>%1 Firmware: FAIL</nobr><br>").arg(Detector));
            FILE_LOG(logERROR) << "Firmware fail";
        }
        else
            message.append(QString("<nobr>%1 Firmware: %2</nobr><br>").arg(Detector, QString::number(retval)));
        FILE_LOG(logINFO) << "Detector Firmware Test: " << retval;
    }

    //detector CPU-FPGA bus
    if (chkDetectorBus->isChecked()) {
        retval = myDet->digitalTest(slsDetectorDefs::DETECTOR_BUS_TEST, moduleId);
        if (retval == slsDetectorDefs::FAIL) {
            message.append(QString("<nobr>%1 Bus: &nbsp;&nbsp;&nbsp;&nbsp;FAIL</nobr><br>").arg(Detector));
            FILE_LOG(logERROR) << "Bus Test fail";
        } else
            message.append(QString("<nobr>%1 Bus: &nbsp;&nbsp;&nbsp;&nbsp;%2</nobr><br>").arg(Detector, QString::number(retval)));
        FILE_LOG(logINFO) << "Detector Bus Test: " << retval;
    }

    //display message
    qDefs::Message(qDefs::INFORMATION, message.toAscii().constData(), "qTabDebugging::TestDetector");

    qDefs::checkErrorMessage(myDet, comboDetector->currentIndex(), "qTabDebugging::TestDetector");
}


void qTabDebugging::Refresh() {
    FILE_LOG(logDEBUG) << "\n**Updating Debugging Tab";
    UpdateStatus();
    FILE_LOG(logDEBUG) << "**Updated Debugging Tab";
}

