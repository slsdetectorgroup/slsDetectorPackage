/*
 * qScanWidget.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QSCANWIDGET_H_
#define QSCANWIDGET_H_

#include "qDefs.h"

/** Form Header */
#include "ui_form_scan.h"
/** Project Class Headers */
class multiSlsDetector;
/** Qt Include Headers */
#include <QStackedLayout>
/** C++ Include Headers */
#include <string>
#include <vector>
using namespace std;


class qScanWidget : public QWidget,private Ui::ScanObject{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	qScanWidget(QWidget *parent, multiSlsDetector*& detector);

	~qScanWidget();

	/**to update the widgets*/
	void Refresh();


	/**number of scan widgets*/
	static int NUM_SCAN_WIDGETS;

private:
	/** The sls detector object */
	multiSlsDetector *myDet;
	/**id of the scan widget*/
	int id;
	/**type of steps*/
	enum sizeIndex{RangeValues, CustomValues, FileValues};
	enum modes{None,EnergyScan,ThresholdScan,TrimbitsScan,PositionScan,CustomScript,NumModes};
	static const string modeNames[NumModes];

	/**values*/
	int		actualNumSteps;
	vector <double> positions;

	/**non error font*/
	QPalette normal;
	QPalette red;
	QString customTip;
	QString fileTip;
	QString rangeTip;

	/**widgets needed for diff size types*/
	QButtonGroup 		*btnGroup;
	QStackedLayout 		*stackedLayout;
	QLabel				*lblFrom;
	QDoubleSpinBox		*spinFrom;
	QLabel				*lblTo;
	QDoubleSpinBox		*spinTo;
	QLabel				*lblSize;
	QDoubleSpinBox		*spinSize;
	QComboBox 			*comboCustom;
	QPushButton			*btnCustom;
	QLineEdit			*dispFile;
	QPushButton			*btnFile;


	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals */
	void Initialization();

	/** Sets up all the parameters from server/client */
	void LoadPositions();

	/** Sets up the scan parameters
	 * returns if it was set
	 */
	int SetScan(int mode);

private slots:
/** Enables widgets depending on which size is clicked.
 * Options: constant size,specific values,values from file
 * */
void EnableSizeWidgets();

/** Sets the scan or script. Accordingly enables, disables other widgets
 * 	@param mode value chosen*/
void SetMode(int mode);

/** Browse for the script
 * */
void BrowsePath();

/** Sets the script file
 * */
void SetScriptFile();

/** Set Parameter
 * */
void SetParameter();

/** Set precision
 * @param value value of precision to be set
 * */
void SetPrecision(int value);

/** Set number of steps
 * */
void SetNSteps();

/**Checks if size is zero and return FAIL if it does
 * */
int RangeCheckSizeZero();

/**returns ok and number of steps if valid
 * @param number of steps
 * */
int RangeCheckNumValid(int &num);

/**returns fail if to is not exact
 * */
int RangeCheckToValid();

/** Sets number of steps if from changed
 * */
void RangeFromChanged();

/** Sets number of steps if to changed
 * */
void RangeToChanged();

/** Sets s of steps if size changed
 * */
void RangeSizeChanged();

/** Set range for scan
 * */
void SetRangeSteps();


/** Range type: If size is changed,
 * the Number of steps is calculated
 * */
//void RangeCalculateNumSteps();

/** Range type: If size is changed, the To is calculated
 * */
//void RangeCalculateTo();


/** Set custom steps
 * returns OK if set properly
 * */
int SetCustomSteps();

/** Delete custom steps
 * */
void DeleteCustomSteps();

/** Reads the file to get the steps
 * */
void SetFileSteps();

/** Browses for the file path for steps
 * */
void BrowseFileStepsPath();



signals:
void EnableScanBox();
};




#endif /* QSCANWIDGET_H_ */

