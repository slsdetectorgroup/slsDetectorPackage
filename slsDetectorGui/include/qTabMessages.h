#pragma once

#include "qDebugStream.h"

class QTextEdit;
class QPushButton;

class qTabMessages:public QWidget{
	Q_OBJECT

public:
	qTabMessages(QWidget* parent);
	~qTabMessages();

private slots:
	void customEvent(QEvent *e);
	void SaveLog();
	void ClearLog();

private:
	void SetupWidgetWindow();
	void Initialization();

	QTextEdit *dispLog;
	QPushButton *btnSave;
	QPushButton *btnClear;
};

