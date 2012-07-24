/*
 * qTabActions.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QACTIONSWIDGET_H_
#define QACTIONSWIDGET_H_


/** Project Class Headers */
class multiSlsDetector;
/** Qt Include Headers */
#include <QFrame>
class QGridLayout;
class QComboBox;
class QLineEdit;
class QPushButton;
class QLabel;
class QSpinBox;
class QGroupBox;
class QRadioButton;
class QCheckBox;
/** C++ Include Headers */
#include <string>
using namespace std;



class ActionsWidget : public QFrame{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 *    @param scanType is if its an energy/threshold scan type
	 *    @param id	is the id of the widget. to know which one was emitting it
	 */
	ActionsWidget(QWidget *parent, multiSlsDetector*& detector, int scanType, int id);

	~ActionsWidget();

	/**set variable expand
	 */
	void SetExpand(bool expanded){expand = expanded;};

	/**get variable expand
	 */
	bool isExpanded(){return expand;};

	/**to update the widgets*/
	void Refresh();


	enum actions{Start,Scan0,Scan1,ActionBefore,NumPositions,
		HeaderBefore,HeaderAfter,ActionAfter,Stop};

private:
	/** The sls detector object */
	multiSlsDetector *myDet;
	/**if its a scan type*/
	int scanType;
	/**id of the action widget*/
	int id;
	/**if this widget has been expanded*/
	bool expand;

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
	QSpinBox 	*spinNumPos;
	QComboBox	*comboPos;
	QPushButton *btnDelete;
	QCheckBox 	*chkInvert;
	QCheckBox 	*chkSeparate;
	QCheckBox 	*chkReturn;

	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals */
	void Initialization();

	/**Gets the sls class action index using the gui index
	 * @param index gui index
	 */
	int GetActionIndex(int gIndex);



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

/** Sets the number of positions
 * */
void SetNumPositions(int index);

/** Deletes current position
 * */
void DeletePosition();


signals:
void EnableScanBox(bool,int);
};




#endif /* QACTIONSWIDGET_H_ */

