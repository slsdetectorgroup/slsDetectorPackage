/*
 * qActionsWidget.h
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */

#ifndef QACTIONSWIDGET_H_
#define QACTIONSWIDGET_H_

#include "qDefs.h"

/** Form Header */
#include "ui_form_action.h"
/** Project Class Headers */
class multiSlsDetector;
/** Qt Include Headers */

/** C++ Include Headers */
#include <string>
using namespace std;



class qActionsWidget : public QWidget,private Ui::ActionsObject{
	Q_OBJECT

public:
	/** \short The constructor
	 *    @param parent is the parent tab widget
	 *    @param detector is the detector returned from the detector tab
	 */
	qActionsWidget(QWidget *parent, multiSlsDetector*& detector);

	~qActionsWidget();

	/**to update the widgets*/
	void Refresh();


	/**number of action widgets*/
	static int NUM_ACTION_WIDGETS;



private:
	/** The sls detector object */
	multiSlsDetector *myDet;
	/**id of the action widget*/
	int id;

	/** Sets up the widget
	 */
	void SetupWidgetWindow();

	/** Sets up all the slots and signals
	 * */
	void Initialization();


private slots:
/** Sets the scan or script. Accordingly enables, disables other widgets
 * 	@param mode value chosen
 * 	*/
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

};




#endif /* QACTIONSWIDGET_H_ */

