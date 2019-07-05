#pragma once

#include "ui_form_tab_messages.h"
#include "qDebugStream.h"

class qTabMessages:public QWidget, private Ui::TabMessagesObject {
	Q_OBJECT

public:
	qTabMessages(QWidget* parent);
	~qTabMessages();
	void Refresh();

private slots:
	void ExecuteCommand();
	void customEvent(QEvent *e);
	void SaveLog();
	void ClearLog();

private:
	void SetupWidgetWindow();
	void Initialization();
	void PrintNextLine();
};

