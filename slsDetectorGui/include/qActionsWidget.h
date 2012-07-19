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
	 *    @param scanType is if its an energy/threshold scan type
	 *    @param id	is the id of the widget. to know which one was emitting it
	 */
	ActionsWidget(QWidget *parent, int scanType, int id);

	~ActionsWidget();

private:
	/**if its a scan type*/
	int scanType;
	/**id of the action widget*/
	int id;

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

signals:
void EnableScanBox(bool,int);
void SetScriptSignal(QString&,int);

};




#endif /* QACTIONSWIDGET_H_ */

