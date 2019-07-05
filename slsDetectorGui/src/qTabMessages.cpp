#include "qTabMessages.h"
#include "qDefs.h"

#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QDir>
#include <QProcess>

#include <iostream>
#include <string>

qTabMessages::qTabMessages(QWidget *parent) : QWidget(parent) {
    setupUi(this);
    SetupWidgetWindow();
    FILE_LOG(logDEBUG) << "Messages ready";
}

qTabMessages::~qTabMessages() {}

void qTabMessages::Refresh() {
    dispCommand->clear();
}

void qTabMessages::SetupWidgetWindow() {
    PrintNextLine();
    qDebugStream(std::cout, this);
    qDebugStream(std::cerr, this);

    Initialization();
}

void qTabMessages::Initialization() {
    connect(btnSave, SIGNAL(clicked()), this, SLOT(SaveLog()));
    connect(btnClear, SIGNAL(clicked()), this, SLOT(ClearLog()));
    connect(dispCommand, SIGNAL(editingFinished()), this, SLOT(ExecuteCommand()));
}

void qTabMessages::ExecuteCommand() {
    QString command = dispCommand->text();
    dispLog->append(command);
    dispCommand->clear();

    // take 1st string as program
    QStringList arguments;

    QProcess *myProcess = new QProcess(this);
    myProcess->start(command, arguments);

    // print readall
    QByteArray result = myProcess.readAll();

    PrintNextLine();
}

void qTabMessages::PrintNextLine() {
    QString path = QDir::cleanPath(QDir::currentPath());
    dispLog->append(QString("\n") + path + QString("$"));
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

