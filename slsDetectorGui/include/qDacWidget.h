#pragma once

#include "ui_form_dac.h"

class multiSlsDetector;
#include "sls_detector_defs.h"

#include <string>

class qDacWidget:public QWidget, private Ui::WidgetDacObject {
	Q_OBJECT

public:
	qDacWidget(QWidget *parent, multiSlsDetector* detector, bool d, std::string n, slsDetectorDefs::dacIndex i, bool t);
	~qDacWidget();
	void SetDetectorIndex(int id);

private slots:
	void SetDac();

private:
	void SetupWidgetWindow(std::string name);
	void Initialization();
	void GetDac();
	void GetAdc();
	void Refresh();

	multiSlsDetector *myDet;
	bool isDac{true};
	slsDetectorDefs::dacIndex index;
	bool isMillideg{false};
	int detectorIndex{-1};
};

