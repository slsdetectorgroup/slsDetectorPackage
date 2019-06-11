#pragma once

class multiSlsDetector;

class QGroupBox;
class QLabel;
class QDoubleSpinBox;
class MyDoubleSpinBox;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QGridLayout;
class QString;
class QPalette;

#include <string>
#include <vector>


/**To override the spin box class to have an id and emit it*/
class MyDoubleSpinBox: public QDoubleSpinBox{
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


class qTabDeveloper:public QWidget {
	Q_OBJECT

public:
	qTabDeveloper(QWidget *parent, multiSlsDetector* detector);
	~qTabDeveloper();

public slots:
	void Refresh();

private slots:
	void GetAdcs();
	void SetDac(int id);
	void SetHighVoltage();

private:
	void SetupWidgetWindow();
	void Initialization();
	void PopulateDetectors();
	void CreateDACWidgets();
	void CreateADCWidgets();
	void CreateHVWidget();
	void GetDac(int id);
	void GetDacs();
	void GetHighVoltage();
	slsDetectorDefs::dacIndex getSLSIndex(int index);

	multiSlsDetector *myDet;
	slsDetectorDefs::detectorType detType;
	int numDACWidgets;
	int numADCWidgets;
	std::vector<std::string>dacNames;
	std::vector<std::string>adcNames;

	enum hvVals {
		HV_0,
		HV_90,
		HV_110,
		HV_120,
		HV_150,
		HV_180,
		HV_200
	};

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
	QComboBox *comboDetector;

	static const int HV_MIN = 60;
	static const int HV_MAX = 200;
};

