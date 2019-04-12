#pragma once

#include "qDefs.h"

class multiSlsDetector;

#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QTimer>
#include <QString>
#include <QPalette>
class qDetectorMain;

#include <string>
#include <vector>


/**To override the spin box class to have an id and emit it*/
class MyDoubleSpinBox:public QDoubleSpinBox{
Q_OBJECT
private:
	int myId;
	private slots:
	void valueChangedWithID() {emit editingFinished(myId);};
	public:
	/** Overridden constructor from QDoubleSpinBox */
	MyDoubleSpinBox(int id,QWidget* parent = 0)	:QDoubleSpinBox(parent), myId(id){
		connect(this, SIGNAL(editingFinished()), this, SLOT(valueChangedWithID()));
	}
	signals:
	void editingFinished(int myId);
};


/**
 *@short sets up the Developer parameters
 */
class qTabDeveloper:public QWidget {
	Q_OBJECT

public:
	/**
	 * The constructor
	 * @param parent is the parent tab widget
	 * @param detector is the detector returned from the detector tab
	 */
	qTabDeveloper(QWidget *parent, multiSlsDetector* detector);

	/**
	 * Destructor
	 */
	~qTabDeveloper();

public slots:

	/**
	 * Refresh and update widgets
	 */
	void Refresh();

private slots:
	/**
	 * Refreshes the adcs
	 */
	void RefreshAdcs();

	/**
	 * Set Dac values
	 * @param id id of dac
	 */
	void SetDacValues(int id);

	/**
	 * Set High Voltage
	 */
	void SetHighVoltage();

private:

	/**
	 * Sets up the widget
	 */
	void SetupWidgetWindow();

	/**
	 * Sets up all the slots and signals
	 */
	void Initialization();

	/**
	 * Sets up the DAC Widgets
	 */
	void CreateDACWidgets();

	/**
	 * Sets up the ADC Widgets
	 */
	void CreateADCWidgets();

	/**
	 * Sets up HV widget
	 */
	void CreateHVWidget();

	/**
	 * Gets the sls index to set/get dac/adc
	 * @param index is the gui dac/adc index
	 * @returns the sls index
	 */
	slsDetectorDefs::dacIndex getSLSIndex(int index);

	/** The sls detector object */
	multiSlsDetector *myDet;
	/** detector type */
	slsDetectorDefs::detectorType detType;
	/**number of dac widgets*/
	int numDACWidgets;
	/**number of adc widgets*/
	int numADCWidgets;

	/** list of dac and adc names */
	std::vector<std::string>dacNames;
	std::vector<std::string>adcNames;


	/**widgets needed*/
	QGroupBox *boxDacs;
	QGroupBox *boxAdcs;
	std::vector<QLabel*>lblDacs;
	std::vector<QLabel*>lblAdcs;
	std::vector<MyDoubleSpinBox*>spinDacs;
	std::vector<QLabel*>lblDacsmV;
	std::vector<QLineEdit*>spinAdcs;
	QLabel *lblHV;
	QComboBox *comboHV;
	QSpinBox *spinHV;
	QGridLayout *dacLayout;
	QString tipHV;
	QPalette red;
	QComboBox *comboDetector;
};

