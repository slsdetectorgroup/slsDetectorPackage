#include "qTabMessages.h"
#include "qDetectorMain.h"

#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QTextStream>

#include <iostream>
#include <string>


qTabMessages::qTabMessages(QWidget *parent) : QWidget(parent) {
    SetupWidgetWindow();
    Initialization();
    FILE_LOG(logDEBUG) << "Messages ready";
}


qTabMessages::~qTabMessages() {
    delete dispLog;
}


void qTabMessages::SetupWidgetWindow() {
    /** Layout */
    QGridLayout *gridLayout = new QGridLayout(this);

    dispLog = new QTextEdit(this);
    dispLog->setReadOnly(true);
    dispLog->setFocusPolicy(Qt::NoFocus);
    dispLog->setTextColor(Qt::darkBlue);

    btnSave = new QPushButton("Save Log  ", this);
    btnSave->setFocusPolicy(Qt::NoFocus);
    btnSave->setFixedWidth(100);
    btnSave->setIcon(QIcon(":/icons/images/save.png"));

    btnClear = new QPushButton("Clear  ", this);
    btnClear->setFocusPolicy(Qt::NoFocus);
    btnClear->setFixedWidth(100);
    btnClear->setIcon(QIcon(":/icons/images/erase.png"));

    gridLayout->addItem(new QSpacerItem(15, 10, QSizePolicy::Fixed, QSizePolicy::Fixed), 0, 0);
    gridLayout->addWidget(btnSave, 1, 0, 1, 1);
    gridLayout->addWidget(btnClear, 1, 4, 1, 1);
    gridLayout->addItem(new QSpacerItem(15, 10, QSizePolicy::Fixed, QSizePolicy::Fixed), 2, 0);
    gridLayout->addWidget(dispLog, 3, 0, 1, 5);

    qDebugStream *qout = new qDebugStream(std::cout, this);
    qDebugStream *qerr = new qDebugStream(std::cerr, this);
}


void qTabMessages::Initialization() {
    connect(btnSave, SIGNAL(clicked()), this, SLOT(SaveLog()));
    connect(btnClear, SIGNAL(clicked()), this, SLOT(ClearLog()));
}


void qTabMessages::customEvent(QEvent *e) {
    if (e->type() == (STREAMEVENT)) {
        QString temp = ((qStreamEvent *)e)->getString();
        dispLog->append(temp);
    }
}


void qTabMessages::SaveLog() {
    QString fName = QString(""); //FIXME:current directory?
    fName = fName + "/LogFile.txt";
    fName = QFileDialog::getSaveFileName(this, tr("Save Snapshot "),
                                         fName, tr("Text files (*.txt);;All Files(*)"));
    if (!fName.isEmpty()) {
        QFile outfile;
        outfile.setFileName(fName);
        if (outfile.open(QIODevice::WriteOnly | QIODevice::Text)) { //Append
            QTextStream out(&outfile);
            out << dispLog->toPlainText() << endl;
            qDefs::Message(qDefs::INFORMATION, std::string("The Log has been successfully saved to "
                                                      "") +
                                                   fName.toAscii().constData(),
                           "qTabMessages::SaveLog");
        } else {
        	FILE_LOG(logWARNING) << "Attempt to save log file failed.";
        	qDefs::Message(qDefs::WARNING, "Attempt to save log file failed.", "qTabMessages::SaveLog");
        }
    }
}


void qTabMessages::ClearLog() {
    dispLog->clear();
    FILE_LOG(logINFO) << "Log Cleared";
}
