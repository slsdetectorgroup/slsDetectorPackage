#pragma once

#include "ui_form_tab_messages.h"

class QProcess;
class QKeyEvent;

class qTabMessages:public QWidget, private Ui::TabMessagesObject {
	Q_OBJECT

public:
	qTabMessages(QWidget* parent);
	~qTabMessages();
	void Refresh();

protected:
	void keyPressEvent(QKeyEvent* event);

private slots:
	void ExecuteCommand();
	void SaveLog();
	void ClearLog();

private:
	void SetupWidgetWindow();
	void Initialization();
	void PrintNextLine();
	void GetLastCommand();
	void ClearCommand();
	void AppendOutput();
	void AppendError();

	QProcess* process;
	QStringList lastCommand;
};

