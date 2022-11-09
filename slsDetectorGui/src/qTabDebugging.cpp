// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "qTabDebugging.h"
#include "qDefs.h"
#include "sls/ToString.h"
#include <QDesktopWidget>
#include <QGridLayout>
#include <QTreeWidget>

namespace sls {

qTabDebugging::qTabDebugging(QWidget *parent, Detector *detector)
    : QWidget(parent), det(detector), treeDet(nullptr),
      lblDetectorHostname(nullptr), lblDetectorFirmware(nullptr),
      lblDetectorSoftware(nullptr) {
    setupUi(this);
    SetupWidgetWindow();
    LOG(logDEBUG) << "Debugging ready";
}

qTabDebugging::~qTabDebugging() {
    delete treeDet;
    delete lblDetectorHostname;
    delete lblDetectorFirmware;
    delete lblDetectorSoftware;
}

void qTabDebugging::SetupWidgetWindow() {
    // enabling according to det type
    if (det->getDetectorType().squash() == slsDetectorDefs::EIGER) {
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
    connect(comboDetector, SIGNAL(currentIndexChanged(int)), this,
            SLOT(GetDetectorStatus()));
    connect(btnGetInfo, SIGNAL(clicked()), this, SLOT(GetInfo()));
    if (btnTest->isEnabled()) {
        connect(btnTest, SIGNAL(clicked()), this, SLOT(TestDetector()));
    }
}

void qTabDebugging::PopulateDetectors() {
    LOG(logDEBUG) << "Populating detectors";

    comboDetector->clear();
    auto res = det->getHostname();
    for (auto &it : res) {
        comboDetector->addItem(QString(it.c_str()));
    }
}

void qTabDebugging::GetDetectorStatus() {
    LOG(logDEBUG) << "Getting Status";

    try {
        std::string status = ToString(
            det->getDetectorStatus({comboDetector->currentIndex()})[0]);
        lblStatus->setText(QString(status.c_str()).toUpper());
    }
    CATCH_DISPLAY("Could not get detector status.",
                  "qTabDebugging::GetDetectorStatus")
}

void qTabDebugging::GetInfo() {
    LOG(logDEBUG) << "Getting Readout Info";

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
    // to make sure the size is constant
    lblDetectorFirmware->setFixedWidth(100);
    layout->addWidget(dispFrame, 0, 1);
    QString detName =
        QString(ToString(det->getDetectorType().squash()).c_str());

    switch (det->getDetectorType().squash()) {

    case slsDetectorDefs::EIGER:
        formLayout->addWidget(new QLabel("Half Module:"), 0, 0);
        formLayout->addItem(
            new QSpacerItem(15, 20, QSizePolicy::Fixed, QSizePolicy::Fixed), 0,
            1);
        formLayout->addWidget(lblDetectorHostname, 0, 2);
        formLayout->addWidget(new QLabel("Half Module Firmware Version:"), 1,
                              0);
        formLayout->addWidget(lblDetectorFirmware, 1, 2);
        formLayout->addWidget(new QLabel("Half Module Software Version:"), 2,
                              0);
        formLayout->addWidget(lblDetectorSoftware, 2, 2);
        treeDet->setHeaderLabel("Eiger Detector");
        // get num modules
        for (int i = 0; i < comboDetector->count() / 2; ++i)
            items.append(
                new QTreeWidgetItem((QTreeWidget *)nullptr,
                                    QStringList(QString("Module %1").arg(i))));
        treeDet->insertTopLevelItems(0, items);
        // gets det names
        for (int i = 0; i < comboDetector->count(); ++i) {
            QList<QTreeWidgetItem *> childItems;
            childItems.append(new QTreeWidgetItem(
                (QTreeWidget *)nullptr,
                QStringList(QString("Half Module (%1)")
                                .arg(comboDetector->itemText(i)))));
            treeDet->topLevelItem(i * 2)->insertChildren(0, childItems);
        }
        break;

    default:
        formLayout->addWidget(new QLabel("Module:"), 0, 0);
        formLayout->addItem(
            new QSpacerItem(15, 20, QSizePolicy::Fixed, QSizePolicy::Fixed), 0,
            1);
        formLayout->addWidget(lblDetectorHostname, 0, 2);
        formLayout->addWidget(new QLabel("Module Firmware Version:"), 1, 0);
        formLayout->addWidget(lblDetectorFirmware, 1, 2);
        formLayout->addWidget(new QLabel("Module Software Version:"), 2, 0);
        formLayout->addWidget(lblDetectorSoftware, 2, 2);
        treeDet->setHeaderLabel(QString(detName + " Detector"));
        // gets det names
        for (int i = 0; i < comboDetector->count(); ++i)
            items.append(new QTreeWidgetItem(
                (QTreeWidget *)nullptr,
                QStringList(
                    QString("Module (%1)").arg(comboDetector->itemText(i)))));
        treeDet->insertTopLevelItems(0, items);
        break;
    }

    // show and center widget
    int x = ((parentWidget()->width()) - (popup1->frameGeometry().width())) / 2;
    int y =
        ((parentWidget()->height()) - (popup1->frameGeometry().height())) / 2;
    QDesktopWidget *desktop = QApplication::desktop();
    int screen = desktop->screenNumber(this);
    popup1->setWindowModality(Qt::WindowModal);
    popup1->move((desktop->screenGeometry(screen).x()) + x,
                 (desktop->screenGeometry(screen).y()) + y);
    popup1->show();

    // put the first parameters
    SetParameters(treeDet->topLevelItem(0));

    // connect to slots
    connect(treeDet, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this,
            SLOT(SetParameters(QTreeWidgetItem *)));
}

void qTabDebugging::SetParameters(QTreeWidgetItem *item) {
    // eiger: if half module clicked, others: true always
    bool ignoreOrHalfModuleClicked = true;
    if (det->getDetectorType().squash() == slsDetectorDefs::EIGER) {
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
            auto retval = std::string("0x") +
                          std::to_string((unsigned long)det->getFirmwareVersion(
                              {comboDetector->currentIndex()})[0]);
            lblDetectorFirmware->setText(QString(retval.c_str()));
            retval = det->getDetectorServerVersion(
                {comboDetector->currentIndex()})[0];
            lblDetectorSoftware->setText(QString(retval.c_str()));
        }
        CATCH_DISPLAY("Could not get versions.", "qTabDebugging::SetParameters")
    }
}

void qTabDebugging::TestDetector() {
    LOG(logINFO) << "Testing Readout";

    try {
        QString moduleName = "Module";
        if (det->getDetectorType().squash() == slsDetectorDefs::EIGER) {
            moduleName = "Half Module";
        }

        // construct message
        QString message = QString("<nobr>Test Results for %1:</nobr><br><br>")
                              .arg(comboDetector->currentText());

        // detector firmware
        if (chkDetectorFirmware->isChecked()) {
            try {
                det->executeFirmwareTest({comboDetector->currentIndex()});
                message.append(QString("<nobr>%1 Firmware: PASS</nobr><br>")
                                   .arg(moduleName));
                LOG(logINFO) << "Detector Firmware Test: Pass";
            }
            CATCH_DISPLAY("Firmware test failed.",
                          "qTabDebugging::TestDetector")
        }

        // detector CPU-FPGA bus
        if (chkDetectorBus->isChecked()) {
            try {
                det->executeBusTest({comboDetector->currentIndex()});
                message.append(
                    QString("<nobr>%1 Bus: PASS</nobr><br>").arg(moduleName));
                LOG(logINFO) << "Detector Bus Test: Pass";
            }
            CATCH_DISPLAY("Bus test failed.", "qTabDebugging::TestDetector")
        }

        // display message
        qDefs::Message(qDefs::INFORMATION, message.toAscii().constData(),
                       "qTabDebugging::TestDetector");
    }
    CATCH_DISPLAY("Could not execute digital test.",
                  "qTabDebugging::TestDetector")
}

void qTabDebugging::Refresh() {
    LOG(logDEBUG) << "**Updating Debugging Tab";
    GetDetectorStatus();
    LOG(logDEBUG) << "**Updated Debugging Tab";
}

} // namespace sls
