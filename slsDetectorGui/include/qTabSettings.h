#pragma once

#include "ui_form_tab_settings.h"

class multiSlsDetector;

class qTabSettings: public QWidget, private Ui::TabSettingsObject{
	Q_OBJECT

public:
	qTabSettings(QWidget *parent, multiSlsDetector* detector);
	~qTabSettings();
	void Refresh();

private slots:
	void SetSettings(int index);
	void SetDynamicRange(int index);
	void SetThresholdEnergy(int index);

private:
	void SetupWidgetWindow();
	void SetupDetectorSettings();
	void Initialization();

	void GetSettings();
	void GetDynamicRange();
	void GetThresholdEnergy();

	multiSlsDetector *myDet;
	enum {
		STANDARD, 
		FAST, 
		HIGHGAIN, 
		DYNAMICGAIN, 
		LOWGAIN, 
		MEDIUMGAIN, 
		VERYHIGHGAIN, 
		DYNAMICHG0, 
		FIXGAIN1, 
		FIXGAIN2, 
		FORCESWITCHG1, 
		FORCESWITCHG2, 
		VERLOWGAIN,
		UNDEFINED, 
		UNINITIALIZED, 
		NUMSETTINGS
	};
	enum {
		DYNAMICRANGE_32,
		DYNAMICRANGE_16,
		DYNAMICRANGE_8,
		DYNAMICRANGE_4
	};
};
