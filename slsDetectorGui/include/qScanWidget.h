/*
 * qScanWidget.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QSCANWIDGET_H_
#define QSCANWIDGET_H_

/** Form Header */
#include "ui_form_scan.h"
/** Project Class Headers */
class multiSlsDetector;
/** Qt Include Headers */
#include <QStackedLayout>
/** C++ Include Headers */
#include <string>
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

	QStackedLayout 	*stackedLayout;
	QLabel			*lblFrom;
	QSpinBox		*spinFrom;
	QLabel			*lblTo;
	QSpinBox		*spinTo;
	QLabel			*lblSize;
	QSpinBox		*spinSize;
	QComboBox 		*comboSpecific;
	QLineEdit		*dispValues;
	QPushButton		*btnValues;


	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals */
	void Initialization();



private slots:
/** Sets the scan or script. Accordingly enables, disables other widgets
 * 	@param index value chosen*/
void SetScript(int index);

/** Enables widgets depending on which size is clicked.
 * 	Options: constant size,specific values,values from file */
void EnableSizeWidgets();

/** Browse for the script
 * */
void BrowsePath();

/** Sets the script file
 * */
void SetScriptFile();

/** Set Parameter
 * @param parameter is the parameter to be set to
 * */
void SetParameter(const QString& parameter);



signals:
void EnableScanBox(bool,int);
};




#endif /* QSCANWIDGET_H_ */

