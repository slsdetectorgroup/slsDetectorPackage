// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "qTabMessages.h"
#include "qDefs.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>
#include <QProcess>
#include <QTextStream>
#include <string>

namespace sls {

qTabMessages::qTabMessages(QWidget *parent) : QWidget(parent) {
    setupUi(this);
    SetupWidgetWindow();
    LOG(logDEBUG) << "Messages ready";
}

qTabMessages::~qTabMessages() {
    process->close();
    delete process;
}

void qTabMessages::SetupWidgetWindow() {
    process = new QProcess;
    process->setWorkingDirectory(QDir::cleanPath(QDir::currentPath()));
    PrintNextLine();
    lastCommand.clear();

    Initialization();
}

void qTabMessages::Initialization() {
    connect(btnSave, SIGNAL(clicked()), this, SLOT(SaveLog()));
    connect(btnClear, SIGNAL(clicked()), this, SLOT(ClearLog()));
    connect(dispCommand, SIGNAL(returnPressed()), this, SLOT(ExecuteCommand()));
}

void qTabMessages::keyPressEvent(QKeyEvent *event) {
    // cout<<"inside KeyPressEvent()\n";
    if (event->key() == Qt::Key_Up) {
        GetLastCommand();
    } else if (event->key() == Qt::Key_Down) {
        ClearCommand();
    }
    /* else if((event->key() == Qt::Key_Return) ||(event->key() ==
   Qt::Key_Enter)) { ExecuteCommand();
   }*/
    else {
        event->ignore();
    }
}

void qTabMessages::PrintNextLine() {
    dispLog->append(QString("<font color = \"DarkGrey\">") +
                    QDir::current().dirName() + QString("$ ") +
                    QString("</font>"));
}

void qTabMessages::GetLastCommand() {
    dispCommand->setText(lastCommand.join(" "));
}

void qTabMessages::ClearCommand() { dispCommand->setText(""); }

void qTabMessages::ExecuteCommand() {
    QStringList param = dispCommand->text().split(" ");
    lastCommand.clear();
    lastCommand += param;
    dispCommand->clear();
    // appending command to log without newline
    dispLog->moveCursor(QTextCursor::End);
    dispLog->insertHtml(QString("<font color = \"DarkBlue\">") +
                        param.join(" ") + QString("</font>"));

    QString command = param.at(0);
    param.removeFirst();
    LOG(logINFO) << "Executing Command:[" << command.toLatin1().constData()
                 << "] with Arguments:["
                 << param.join(" ").toLatin1().constData() << "]";

    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start(command, param);
    if (!process->waitForFinished()) {
        AppendError();
    } else {
        AppendOutput();
    }
}

void qTabMessages::AppendOutput() {
    QByteArray result = process->readAll();
    result.replace("\n", "<br>");
    dispLog->append(QString("<font color = \"DarkBlue\">") + result +
                    QString("</font>"));
    LOG(logDEBUG) << "Command executed successfully";
    PrintNextLine();
}

void qTabMessages::AppendError() {
    dispLog->append(QString("<font color = \"Red\">") + process->errorString() +
                    QString("</font>"));
    LOG(logERROR) << "Error executing command";
    PrintNextLine();
}

void qTabMessages::SaveLog() {
    QString fName = QDir::cleanPath(QDir::currentPath()) + "/LogFile.txt";
    fName =
        QFileDialog::getSaveFileName(this, tr("Save Snapshot "), fName,
                                     tr("Text files (*.txt);;All Files(*)"));
    if (!fName.isEmpty()) {
        QFile outfile;
        outfile.setFileName(fName);
        if (outfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&outfile);
            out << dispLog->toPlainText() << '\n';
            std::string mess =
                std::string("The Log has been successfully saved to ") +
                fName.toLatin1().constData();
            qDefs::Message(qDefs::INFORMATION, mess, "TabMessages::SaveLog");
            LOG(logINFO) << mess;
        } else {
            LOG(logWARNING) << "Attempt to save log file failed: "
                            << fName.toLatin1().constData();
            qDefs::Message(qDefs::WARNING, "Attempt to save log file failed.",
                           "qTabMessages::SaveLog");
        }
    }
    dispCommand->setFocus();
}

void qTabMessages::ClearLog() {
    dispLog->clear();
    LOG(logINFO) << "Log Cleared";
    PrintNextLine();
    dispCommand->setFocus();
}

void qTabMessages::Refresh() {
    dispCommand->clear();
    dispCommand->setFocus();
}

} // namespace sls
