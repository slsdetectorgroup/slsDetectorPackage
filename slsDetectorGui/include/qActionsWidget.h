/*
 * qTabActions.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QACTIONSWIDGET_H_
#define QACTIONSWIDGET_H_
#include <QFrame>
class QGridLayout;
class QComboBox;
class QLineEdit;
class QPushButton;
class QLabel;
class QSpinBox;
class QGroupBox;
class QRadioButton;


class ActionsWidget : public QFrame{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	ActionsWidget(QWidget *parent, int scanType);

	~ActionsWidget();

private:
	QGridLayout *layout;
	QComboBox 	*comboScript;
	QLineEdit 	*dispScript;
	QPushButton	*btnBrowse;
	QLabel 		*lblParameter;
	QLineEdit 	*dispParameter;
	QLabel 		*lblSteps;
	QSpinBox 	*spinSteps;
	QLabel 		*lblPrecision;
	QSpinBox 	*spinPrecision;
	QGroupBox	*group;
	QRadioButton *radioConstant;
	QRadioButton *radioSpecific;
	QRadioButton *radioValue;
	QLabel		*lblFrom;
	QSpinBox	*spinFrom;
	QLabel		*lblTo;
	QSpinBox	*spinTo;
	QLabel		*lblSize;
	QSpinBox	*spinSize;
	QComboBox 	*comboSpecific;
	QLineEdit	*dispValues;
	QPushButton	*btnValues;

	/** Sets up the widget
	 * @param scanType 1 if it includes Threshold Scan,Energy Scan and Trimbits Scan, else 0*/
	void SetupWidgetWindow(int scanType);

	/** Sets up all the slots and signals */
	void Initialization();


private slots:
/** Sets the scan or script. Accordingly enables, disables other widgets
 * 	@param index value chosen*/
void SetScript(int index);

/** Enables widgets depending on which size is clicked.
 * 	Options: constant size,specific values,values from file */
void EnableSizeWidgets();



protected:
};




#endif /* QACTIONSWIDGET_H_ */

