#pragma once

#include "ui_form_tab_messages.h"

class QProcess;

class qTabMessages:public QWidget, private Ui::TabMessagesObject {
	Q_OBJECT

public:
	qTabMessages(QWidget* parent);
	~qTabMessages();
	void Refresh();

private slots:
	void ExecuteCommand();
	void SaveLog();
	void ClearLog();

private:
	void SetupWidgetWindow();
	void Initialization();
	void PrintNextLine();
	void AppendOutput();
	void AppendError();

	QProcess* process;
};

