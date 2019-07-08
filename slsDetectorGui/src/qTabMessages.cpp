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

qTabMessages::~qTabMessages() {
    process->close();
    if (process)
        delete process;
}

void qTabMessages::SetupWidgetWindow() {
    process = new QProcess;
    process->setWorkingDirectory(QDir::cleanPath(QDir::currentPath()));
    PrintNextLine();

    qDebugStream(std::cout, this);
    qDebugStream(std::cerr, this);

    Initialization();
}

void qTabMessages::Initialization() {
    connect(btnSave, SIGNAL(clicked()), this, SLOT(SaveLog()));
    connect(btnClear, SIGNAL(clicked()), this, SLOT(ClearLog()));
    connect(dispCommand, SIGNAL(returnPressed()), this, SLOT(ExecuteCommand()));
}

void qTabMessages::ExecuteCommand() {
    QStringList param = dispCommand->text().split(" ");
    dispCommand->clear();
    // appending command to log without newline
    dispLog->moveCursor (QTextCursor::End);
    dispLog->insertHtml(QString("<font color = \"DarkBlue\">") + param.join(" ") + QString("</font>"));
 
    QString command = param.at(0);
    param.removeFirst();
    FILE_LOG(logINFO) << "Executing Command:[" << command.toAscii().constData() << "] with Arguments:[" << param.join(" ").toAscii().constData() << "]";

    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start(command, param);
    if(!process->waitForFinished()) {
        AppendError();
    } else {
        AppendOutput();
    }
}

void qTabMessages::PrintNextLine() {
    dispLog->append(QString("<font color = \"DarkGrey\">") + QDir::current().dirName() + QString("$ ") + QString("</font>"));
}

void qTabMessages::AppendOutput() {
    QByteArray result = process->readAll();
    result.replace("\n", "<br>");
    dispLog->append(QString("<font color = \"DarkBlue\">") + result + QString("</font>"));
    FILE_LOG(logDEBUG) << "Command executed successfully";
    PrintNextLine();
}

void qTabMessages::AppendError() {
    dispLog->append(QString("<font color = \"Red\">") + process->errorString() + QString("</font>"));
    FILE_LOG(logERROR) << "Error executing command";
    PrintNextLine();
}


void qTabMessages::customEvent(QEvent *e) {
    if (e->type() == (STREAMEVENT)) {
        QString temp = ((qStreamEvent *)e)->getString();
        dispLog->append(temp);
    }
}

void qTabMessages::SaveLog() {
    QString fName = QDir::cleanPath(QDir::currentPath()) + "/LogFile.txt";
    fName = QFileDialog::getSaveFileName(this, tr("Save Snapshot "),
                                         fName, tr("Text files (*.txt);;All Files(*)"));
    if (!fName.isEmpty()) {
        QFile outfile;
        outfile.setFileName(fName);
        if (outfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&outfile);
            out << dispLog->toPlainText() << endl;
            std::string mess  = std::string("The Log has been successfully saved to ") + fName.toAscii().constData();
            qDefs::Message(qDefs::INFORMATION, mess, "TabMessages::SaveLog");
            FILE_LOG(logINFO) << mess;
        } else {
        	FILE_LOG(logWARNING) << "Attempt to save log file failed: " << fName.toAscii().constData();
        	qDefs::Message(qDefs::WARNING, "Attempt to save log file failed.", "qTabMessages::SaveLog");
        }
    }
    dispCommand->setFocus();
}

void qTabMessages::ClearLog() {
    dispLog->clear();
    FILE_LOG(logINFO) << "Log Cleared";
    PrintNextLine();
    dispCommand->setFocus();
}

void qTabMessages::Refresh() {
    dispCommand->clear();
    dispCommand->setFocus();
}

