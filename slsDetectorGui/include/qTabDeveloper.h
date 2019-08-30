#pragma once

#include "ui_form_tab_developer.h"
class qDacWidget;

#include "Detector.h"
#include "sls_detector_defs.h"

#include <string>
#include <vector>

class qTabDeveloper:public QWidget, private Ui::TabDeveloperObject {
	Q_OBJECT

public:
	qTabDeveloper(QWidget *parent, sls::Detector* detector);
	~qTabDeveloper();

public slots:
	void Refresh();

private slots:
	void SetHighVoltage();

private:
	void SetupWidgetWindow();
	void Initialization();
	void PopulateDetectors();
	void GetHighVoltage();
	slsDetectorDefs::dacIndex getSLSIndex(slsDetectorDefs::detectorType detType, int index);

	sls::Detector *det;
	std::vector<qDacWidget*> dacWidgets;
	std::vector<qDacWidget*> adcWidgets;
	
	enum hvVals {
		HV_0,
		HV_90,
		HV_110,
		HV_120,
		HV_150,
		HV_180,
		HV_200
	};

	static const int HV_MIN = 60;
	static const int HV_MAX = 200;
};

