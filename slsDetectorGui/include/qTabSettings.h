#pragma once

#include "ui_form_tab_settings.h"

#include "Detector.h"

class qTabSettings: public QWidget, private Ui::TabSettingsObject{
	Q_OBJECT

public:
	qTabSettings(QWidget *parent, sls::Detector* detector);
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

	sls::Detector *det;
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
        G1_HIGHGAIN,
        G1_LOWGAIN,
        G2_HIGHCAP_HIGHGAIN,
        G2_HIGHCAP_LOWGAIN,
        G2_LOWCAP_HIGHGAIN,
        G2_LOWCAP_LOWGAIN,
        G4_HIGHGAIN,
        G4_LOWGAIN,		
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
