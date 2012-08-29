/*
 * qScanWidget.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
// Qt Project Class Headers
#include "qScanWidget.h"
#include "qDefs.h"
// Qt Include Headers
#include <QFileDialog>
#include <QStackedLayout>

// C++ Include Headers
#include<iostream>
#include <fstream>
#include <sstream>
using namespace std;


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qScanWidget::NUM_SCAN_WIDGETS(0);
const string qScanWidget::modeNames[NumModes]={"","energy","threshold","trimbits","position","custom script"};
//-------------------------------------------------------------------------------------------------------------------------------------------------

qScanWidget::qScanWidget(QWidget *parent,multiSlsDetector*& detector):
		QWidget(parent),myDet(detector),actualNumSteps(0){
	setupUi(this);
	SetupWidgetWindow();
	Initialization();
	LoadPositions();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qScanWidget::~qScanWidget(){
	delete myDet;
	delete stackedLayout;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::SetupWidgetWindow(){
	id = NUM_SCAN_WIDGETS;
	NUM_SCAN_WIDGETS++;

	setFixedHeight(125);

	btnGroup = new QButtonGroup(this);
	btnGroup->addButton(radioRange,0);
	btnGroup->addButton(radioCustom,1);
	btnGroup->addButton(radioFile,2);


	normal = radioCustom->palette();
	red = QPalette();
	red.setColor(QPalette::Active,QPalette::WindowText,Qt::red);
	fileTip = radioFile->toolTip();
	customTip = radioCustom->toolTip();
	rangeTip = radioRange->toolTip();

	//layout for the size widgets
	stackedLayout = new QStackedLayout;
	stackedLayout->setSpacing(0);

	//  Range Size Layout
	QWidget *widgetRange = new QWidget;
	QHBoxLayout *layoutRange = new QHBoxLayout(widgetRange);
	layoutRange->setContentsMargins(0, 0, 0, 0);
	lblFrom	= new QLabel("from",widgetRange);
	spinFrom = new QDoubleSpinBox(widgetRange);
	lblTo = new QLabel("to",widgetRange);
	spinTo	= new QDoubleSpinBox(widgetRange);
	lblSize	= new QLabel("step size:",widgetRange);
	spinSize = new QDoubleSpinBox(widgetRange);
	lblFrom->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	lblFrom->setToolTip(rangeTip);
	spinFrom->setValue(0);
	spinFrom->setToolTip(rangeTip);
	spinFrom->setMaximum(1000000);
	spinFrom->setKeyboardTracking(false);
	lblTo->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	lblTo->setToolTip(rangeTip);
	spinTo->setValue(1);
	spinTo->setToolTip(rangeTip);
	spinTo->setMaximum(1000000);
	spinTo->setKeyboardTracking(false);
	lblSize->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	lblSize->setToolTip(rangeTip);
	lblSize->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	lblSize->setFixedWidth(67);
	spinSize->setValue(1);
	spinSize->setSingleStep(0.1);
	spinSize->setToolTip(rangeTip);
	spinSize->setKeyboardTracking(false);
	layoutRange->addItem(new QSpacerItem(40,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	layoutRange->addWidget(lblFrom);
	layoutRange->addItem(new QSpacerItem(5,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	layoutRange->addWidget(spinFrom);
	layoutRange->addItem(new QSpacerItem(5,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	layoutRange->addWidget(lblTo);
	layoutRange->addItem(new QSpacerItem(5,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	layoutRange->addWidget(spinTo);
	layoutRange->addItem(new QSpacerItem(30,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	layoutRange->addWidget(lblSize);
	layoutRange->addWidget(spinSize);
	layoutRange->addItem(new QSpacerItem(40,20,QSizePolicy::Fixed,QSizePolicy::Fixed));


	//  Custom Values Layout
	QWidget *widgetCustom = new QWidget;
	QHBoxLayout *layoutCustom = new QHBoxLayout(widgetCustom);
	layoutCustom->setContentsMargins(0, 0, 0, 0);
	comboCustom = new QComboBox(widgetCustom);
	btnCustom = new QPushButton("Delete",widgetCustom);
	comboCustom->setEditable(true);
	comboCustom->setCompleter(false);
	comboCustom->setValidator(new QDoubleValidator(comboCustom));
	comboCustom->setToolTip(customTip);
	btnCustom->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	btnCustom->setToolTip("<nobr>Deletes current position from list and reduces <b>Number of Positions</b> by 1.</nobr>");
	layoutCustom->addItem(new QSpacerItem(160,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	layoutCustom->addWidget(comboCustom);
	layoutCustom->addItem(new QSpacerItem(5,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	layoutCustom->addWidget(btnCustom);
	layoutCustom->addItem(new QSpacerItem(160,20,QSizePolicy::Fixed,QSizePolicy::Fixed));

	//  File values From a File Layout
	QWidget *widgetFile = new QWidget;
	QHBoxLayout *layoutFile = new QHBoxLayout(widgetFile);
	layoutFile->setContentsMargins(0, 0, 0, 0);
	layoutFile->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	dispFile = new QLineEdit(widgetFile);
	dispFile->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
	dispFile->setToolTip(fileTip);
	layoutFile->addWidget(dispFile);
	btnFile = new QPushButton("Browse",widgetFile);
	btnFile->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	btnFile->setToolTip(fileTip);
	layoutFile->addWidget(btnFile);
	layoutFile->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));


	stackedLayout->addWidget(widgetRange);
	stackedLayout->addWidget(widgetCustom);
	stackedLayout->addWidget(widgetFile);
	stackedWidget->setLayout(stackedLayout);

	radioCustom->setChecked(true);
	stackedLayout->setCurrentIndex(CustomValues);

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::Initialization(){
	//mode
	connect(comboScript,	SIGNAL(currentIndexChanged(int)),		this,SLOT(SetMode(int)));
	//file
	connect(dispScript,		SIGNAL(editingFinished()),				this, SLOT(SetScriptFile()));
	connect(btnBrowse,		SIGNAL(clicked()), 						this, SLOT(BrowsePath()));
	//parameter
	connect(dispParameter,	SIGNAL(editingFinished()),				this, SLOT(SetParameter()));
	//sizewidgets
	connect(btnGroup,		SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(EnableSizeWidgets()));
	//numsteps
	connect(spinSteps,		SIGNAL(valueChanged(int)), 				this, SLOT(SetNSteps(int)));
	//precision
	connect(spinPrecision,	SIGNAL(valueChanged(int)), 				this, SLOT(SetPrecision(int)));
	//range values
	connect(spinFrom,		SIGNAL(valueChanged(double)), 				this, SLOT(SetRangeSteps()));
	connect(spinTo,			SIGNAL(valueChanged(double)), 				this, SLOT(SetRangeSteps()));
	connect(spinSize,		SIGNAL(valueChanged(double)), 				this, SLOT(SetRangeSteps()));
	//custom values
	connect(comboCustom,	SIGNAL(currentIndexChanged(int)), 		this, SLOT(SetCustomSteps()));
	connect(btnCustom,		SIGNAL(clicked()),						this, SLOT(DeleteCustomSteps()));
	//file values
	connect(dispFile,		SIGNAL(editingFinished()),				this, SLOT(SetFileSteps()));
	connect(btnFile,		SIGNAL(clicked()), 						this, SLOT(BrowseFileStepsPath()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::EnableSizeWidgets(){
#ifdef VERYVERBOSE
	cout << "Entering EnableSizeWidgets()" << endl;
#endif
	//setting to defaults is not done here as this is called even if mode changes
	//so only if its to change the size widget type, its set to default for the others

	//scan is  none
	if(!comboScript->currentIndex()){
		radioCustom->setText("Specific Values");
		radioCustom->setPalette(normal);
		radioCustom->setToolTip(customTip);
		comboCustom->setToolTip(customTip);

		radioFile->setPalette(normal);
		radioFile->setText("Values from File:");
		radioFile->setToolTip(fileTip);
		dispFile->setToolTip(fileTip);
		btnFile->setToolTip(fileTip);
	}
	else{
		// Steps are enabled for all except Range step size
		/**lblSteps->setEnabled(!radioRange->isChecked());
		spinSteps->setEnabled(!radioRange->isChecked());*/
		//range values
		if(radioRange->isChecked()){
#ifdef VERBOSE
		cout << "Constant Range Values" << endl;
#endif
		radioCustom->setText("Specific Values");
		radioCustom->setPalette(normal);
		radioCustom->setToolTip(customTip);
		comboCustom->setToolTip(customTip);

		radioFile->setPalette(normal);
		radioFile->setText("Values from File:");
		radioFile->setToolTip(fileTip);
		dispFile->setToolTip(fileTip);
		btnFile->setToolTip(fileTip);

		stackedLayout->setCurrentIndex(RangeValues);
			SetRangeSteps();
		}
		//custom values
		else if(radioCustom->isChecked()){
#ifdef VERBOSE
			cout << "Custom Values" << endl;
#endif
			//defaults for other mode
			radioFile->setPalette(normal);
			radioFile->setText("Values from File:");
			radioFile->setToolTip(fileTip);
			dispFile->setToolTip(fileTip);
			btnFile->setToolTip(fileTip);

			//change it back as this list is what will be loaded.
			//also numstpes could have been changed in other modes too
			disconnect(spinSteps,	SIGNAL(valueChanged(int)), this, SLOT(SetNSteps(int)));
			spinSteps ->setValue(comboCustom->count());
			connect(spinSteps,		SIGNAL(valueChanged(int)), this, SLOT(SetNSteps(int)));

			stackedLayout->setCurrentIndex(CustomValues);
			//only for custom steps out here because many signals go through
			//custom steps and we want to give the info msg only when changig range types
			if(SetCustomSteps()==qDefs::OK){
				char cNum[200];sprintf(cNum,"%d",actualNumSteps);
				char cId[5];sprintf(cId,"%d",id);
				qDefs::InfoMessage(string("<nobr><font color=\"blue\">Scan Level ")+string(cId)+
								string(": Specific Values</font></nobr><br><br><nobr>Number of positions added: ")+
								string(cNum)+string("</nobr>"),"ScanWidget");
			}
		}

		//file values
		else{
#ifdef VERBOSE
		cout << "File Values" << endl;
#endif
			//defaults for other mode
			radioCustom->setText("Specific Values");
			radioCustom->setPalette(normal);
			radioCustom->setToolTip(customTip);
			comboCustom->setToolTip(customTip);

			stackedLayout->setCurrentIndex(FileValues);
			SetFileSteps();
		}
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::SetMode(int mode){
#ifdef VERYVERBOSE
	cout << "Entering SetMode()" << endl;
#endif
#ifdef VERBOSE
	cout << "Setting\tscan:" << id << "\tmode:" << mode << endl;
#endif
	// defaults
	dispScript->setEnabled(false);
	btnBrowse->setEnabled(false);
	lblParameter->setEnabled(false);
	dispParameter->setEnabled(false);
	group->setEnabled(false);
	lblSteps->setEnabled(false);
	spinSteps->setEnabled(false);
	lblPrecision->setEnabled(false);
	spinPrecision->setEnabled(false);

	// If anything other than None is selected
	if(mode){
		// Steps are enabled for all except Range step size
		/**lblSteps->setEnabled(!radioRange->isChecked());
		spinSteps->setEnabled(!radioRange->isChecked());*/
		lblSteps->setEnabled(true);
		spinSteps->setEnabled(true);
		// Custom Script only enables the first layout with addnl parameters etc
		if(mode==CustomScript){
			dispScript->setEnabled(true);
			btnBrowse->setEnabled(true);
			lblParameter->setEnabled(true);
			dispParameter->setEnabled(true);
		}
		group->setEnabled(true);
		lblPrecision->setEnabled(true);
		spinPrecision->setEnabled(true);

	}

	//set the group box widgets
	EnableSizeWidgets();

	//set the mode
	/*SetScan(mode);*/
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qScanWidget::SetScan(int mode){
#ifdef VERYVERBOSE
	cout << "Entering SetScan()" << endl;
#endif
	string parameter = string(dispParameter->text().toAscii().constData());
	string script = string(dispScript->text().toAscii().constData());
#ifdef VERBOSE
	cout << "SETTING scan:" << id << "\tmode:" << comboScript->currentIndex() <<
			"\tnumSteps:" << actualNumSteps <<
			"\tscript:" << script << "\tparameter:" << parameter << endl;
#endif
	double *values;
	if(actualNumSteps)		values = new double[actualNumSteps];
	else		values = NULL;
	for(int i=0;i<actualNumSteps;i++)	values[i] = positions[i];

	cout<<"modeNames[mode]:"<<modeNames[mode]<<endl;cout<<"actualNumSteps:"<<actualNumSteps<<endl;
	//setting the mode
	if(mode==CustomScript)
		myDet->setScan(id,script,actualNumSteps,values,parameter);
	else
		cout<<"return values:"<<myDet->setScan(id,modeNames[mode],actualNumSteps,values,parameter)<<endl;

	//custom script
	int actualMode = myDet->getScanMode(id);cout<<"actualmode:"<<actualMode<<endl;
	if((mode==CustomScript)&&((script=="")||(script=="none"))){
		return qDefs::OK;
	}else{//mode NOT set
		if((mode!=actualMode)&&(actualNumSteps)){
			qDefs::WarningMessage("The mode could not be changed for an unknown reason.","ScanWidget");
			comboScript->setCurrentIndex(actualMode);
			return qDefs::FAIL;
		}
	}

	emit EnableScanBox();

	return qDefs::OK;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qScanWidget::BrowsePath(){
#ifdef VERYVERBOSE
	cout << "Entering BrowsePath()" << endl;
#endif
#ifdef VERBOSE
	cout << "Browsing Script File Path\tscan:" << id << endl;
#endif
	QString fName = dispScript->text();
	QString dir = fName.section('/',0,-2,QString::SectionIncludeLeadingSep);
	if(dir.isEmpty()) dir = "/home/";
	//dialog
	fName = QFileDialog::getOpenFileName(this,
			tr("Load Script File"),dir,
			tr("Script Files(*.awk);;All Files(*)"));
	//if empty, set the file name and it calls setscriptfile, else ignore
	if (!fName.isEmpty()){
		dispScript->setText(fName);
		SetScriptFile();
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qScanWidget::SetScriptFile(){
#ifdef VERYVERBOSE
	cout << "Entering SetScriptFile()" << endl;
#endif
	QString fName = dispScript->text();bool set = false;
#ifdef VERBOSE
	cout << "Setting\tscan:" << id << "\tscript:" << fName.toAscii().constData() << endl;
#endif


	//blank
	if(fName.isEmpty())
		set = true;
	//none isnt in the modeNames list, so check separately
	else if(!fName.compare("none"))
			set = true;
	else{//if one of the other modes
		for(int i=1;i<NumModes;i++)
			if(!fName.compare(QString(modeNames[i].c_str()))){
				set = true;
				break;
			}
	}
	//not blank and custom script mode
	if(!set){
		QString file = dispScript->text().section('/',-1);
		//is a file
		if(file.contains('.')){
			//check if it exists and set the script file
			if(QFile::exists(fName))
				set = true;
			//if the file doesnt exist, set it to what it was before
			else{
				qDefs::WarningMessage("The script file entered does not exist","ScanWidget");
				dispScript->setText(QString(myDet->getScanScript(id).c_str()));
			}
		}//not a file, set it to what it was before
		else {
			qDefs::WarningMessage("The script file path entered is not a file","ScanWidget");
			dispScript->setText(QString(myDet->getScanScript(id).c_str()));
		}
	}

	//if blank or valid file
	if(set){
		myDet->setScanScript(id,fName.toAscii().constData());
		if(fName.compare(QString(myDet->getScanScript(id).c_str()))){
			//did not get set, write what is was before
			if(!fName.isEmpty())
				qDefs::WarningMessage("The script file could not be set. Reverting to previous file.","ScanWidget");
			dispScript->setText(QString(myDet->getScanScript(id).c_str()));
		}

	}

	//dont display if theres a none
	fName = dispScript->text();
	//none isnt in the modeNames list, so check separately
	if(!fName.compare("none"))
		dispScript->setText("");
	for(int i=1;i<NumModes;i++)
		if(!fName.compare(QString(modeNames[i].c_str())))
			  dispScript->setText("");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::SetParameter(){
#ifdef VERYVERBOSE
	cout << "Entering SetParameter()" << endl;
#endif
	QString parameter = dispParameter->text();
#ifdef VERBOSE
	cout << "Setting\tscan:" << id << "\tparameter:" << parameter.toAscii().constData() << endl;
#endif

	myDet->setScanParameter(id,parameter.toAscii().constData());
	//dont display if theres a none
	parameter = dispParameter->text();
	//none isnt in the modeNames list, so check separately
	if(!parameter.compare("none"))
		dispParameter->setText("");
	for(int i=1;i<NumModes;i++)
		if(!parameter.compare(QString(modeNames[i].c_str())))
			dispParameter->setText("");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::SetPrecision(int value){
#ifdef VERYVERBOSE
	cout << "Entering SetPrecision()" << endl;
#endif
#ifdef VERBOSE
	cout << "Setting\tscan:" << id << "\tprecision:" << value << endl;
#endif
	myDet->setScanPrecision(id,value);
	if(myDet->getScanPrecision(id)!=value)
		qDefs::WarningMessage("The precision was not set for an unknown reason.","ScanWidget");;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::SetNSteps(int num){
#ifdef VERYVERBOSE
	cout << "Entering SetNSteps()" << endl;
#endif
#ifdef VERBOSE
	cout << "Setting number of steps" << endl;
#endif

	int numSteps = spinSteps->value();
	comboCustom->setMaxCount(numSteps);

	//check if its ok
	if(radioRange->isChecked()){
		//calculate the step size and display it
		if(num==1){
			disconnect(spinTo,	SIGNAL(valueChanged(double)), this, SLOT(SetRangeSteps()));
			spinTo->setValue(spinFrom->value());
			connect(spinTo,		SIGNAL(valueChanged(double)), this, SLOT(SetRangeSteps()));
		}else if(num>1)
			num--;
		double stepSize = (spinTo->value()-spinFrom->value())/num;
		disconnect(spinSize,SIGNAL(valueChanged(double)), this, SLOT(SetRangeSteps()));
		spinSize->setValue(stepSize);
		connect(spinSize,	SIGNAL(valueChanged(double)), this, SLOT(SetRangeSteps()));
		//set these positions
		SetRangeSteps();
	}else if(radioCustom->isChecked()){
		SetCustomSteps();
	}else if(radioFile->isChecked()){
		SetFileSteps();
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::SetRangeSteps(){
#ifdef VERYVERBOSE
	cout << "Entering SetRangeSteps()" << endl;
#endif
#ifdef VERBOSE
	cout << "Setting\tscan:" << id << "\trange\t";
#endif
	double fromVal = spinFrom->value();
	double sizeVal = spinSize->value();
	//if step size is 0, min and max should be same
	if(!sizeVal){
		disconnect(spinTo,	SIGNAL(valueChanged(double)), this, SLOT(SetRangeSteps()));
		spinTo->setValue(fromVal);
		connect(spinTo,		SIGNAL(valueChanged(double)), this, SLOT(SetRangeSteps()));
		QString tip = rangeTip + QString("<br><br><font color=\"red\">"
				"<nobr>Note: Increase the <b>step size</b> from zero to be able to change the range.</nobr></font>");
		lblSize->setToolTip(tip);
		spinSize->setToolTip(tip);
		lblSize->setText("step size:*");
		lblSize->setPalette(red);
	}else{
		lblSize->setToolTip(rangeTip);
		spinSize->setToolTip(rangeTip);
		lblSize->setText("step size:");
		lblSize->setPalette(normal);
	}
	//if min and max is the same
	if(fromVal==spinTo->value()) actualNumSteps = 0;
	else actualNumSteps = (int)((spinTo->value()-fromVal)/sizeVal);

	//actualNumSteps will be negative if from<to and size wasnt negative.so change it to positive
	if(actualNumSteps<0){
		actualNumSteps*=-1;
		disconnect(spinSize,SIGNAL(valueChanged(double)), this, SLOT(SetRangeSteps()));
		spinSize->setValue(-1*sizeVal);
		connect(spinSize,	SIGNAL(valueChanged(double)), this, SLOT(SetRangeSteps()));
		sizeVal = spinSize->value();
	}
	//increment is required like vice versa in setNSteps
	actualNumSteps++;
#ifdef VERBOSE
	cout << "num pos:" << actualNumSteps << endl;
#endif

	//set the vector positions
	positions.resize(actualNumSteps);
	for(int i=0;i<actualNumSteps;i++){
		positions[i] = fromVal + i * sizeVal;
#ifdef VERBOSE
	cout << "positions[" << i << "]:\t" << positions[i] <<endl;
#endif
	}

	//correcting the number of steps
	disconnect(spinSteps,	SIGNAL(valueChanged(int)), this, SLOT(SetNSteps(int)));
	spinSteps->setValue(actualNumSteps);
	connect(spinSteps,		SIGNAL(valueChanged(int)), this, SLOT(SetNSteps(int)));


	//sets the scan
	if(SetScan(comboScript->currentIndex())==qDefs::OK){
		char cId[5];sprintf(cId,"%d",id);
		QString script = dispScript->text();

		disconnect(spinFrom,		SIGNAL(valueChanged(double)), 				this, SLOT(SetRangeSteps()));
		disconnect(spinTo,			SIGNAL(valueChanged(double)), 				this, SLOT(SetRangeSteps()));
		disconnect(spinSize,		SIGNAL(valueChanged(double)), 				this, SLOT(SetRangeSteps()));

		//positions wont be loaded if its custom script
		if((comboScript->currentIndex()==CustomScript)&&((script=="")||(script=="none"))){
			qDefs::InfoMessage(string("<nobr><font color=\"blue\">Scan Level ")+string(cId)+
					string(": Constant Step Size</font></nobr><br><br>"
							"<nobr>Positions could not be loaded as the script file path is empty.</nobr>"),"ScanWidget");
		}else{
			//error loading positions
			if(myDet->getScanSteps(id)!=actualNumSteps){
				qDefs::WarningMessage(string("<nobr><font color=\"blue\">Scan Level ")+string(cId)+
						string(": Values From File</font></nobr><br><br>"
						"<nobr>The positions list was not set for an unknown reason.</nobr>"),"ScanWidget");
				/*LoadPositions();
				comboScript->setCurrentIndex(myDet->getScanMode(id))*/
			}else{//SUCCESS
				char cNum[200];sprintf(cNum,"%d",actualNumSteps);
				qDefs::InfoMessage(string("<nobr><font color=\"blue\">Scan Level ")+string(cId)+
						string(": Constant Step Size</font></nobr><br><br><nobr>Number of positions added: ")+
						string(cNum)+string("</nobr>"),"ScanWidget");
			}
		}

		connect(spinFrom,		SIGNAL(valueChanged(double)), 				this, SLOT(SetRangeSteps()));
		connect(spinTo,			SIGNAL(valueChanged(double)), 				this, SLOT(SetRangeSteps()));
		connect(spinSize,		SIGNAL(valueChanged(double)), 				this, SLOT(SetRangeSteps()));

	}

}

//-------------------------------------------------------------------------------------------------------------------------------------------------


int qScanWidget::SetCustomSteps(){
#ifdef VERYVERBOSE
	cout << "Entering SetCustomSteps()" << endl;
#endif
#ifdef VERBOSE
	cout << "Setting\tscan:" << id << "\tcustom\tnum pos:" << comboCustom->count() << endl;
#endif
	disconnect(comboCustom,	SIGNAL(currentIndexChanged(int)), 	this, SLOT(SetCustomSteps()));

	//get number of positions
	int numSteps = spinSteps->value();
	comboCustom->setMaxCount(numSteps);
	comboCustom->setEnabled(numSteps);
	btnCustom->setEnabled(numSteps);

	//deleting too many or not entering enough
	if(numSteps>comboCustom->count()){
#ifdef VERBOSE
	cout << "Too few entries" << endl;
#endif
		int diff = numSteps - (comboCustom->count());
		radioCustom->setPalette(red);
		radioCustom->setText("Specific Values*");
		QString tip = customTip + QString("<br><br><font color=\"red\"><nobr>Add ")+
				(QString("%1").arg(diff))+
				QString(" more positions to the list to match <b>Number of Steps</b>.<nobr><br>"
						"<nobr><nobr>Or reduce <b>Number of Steps</b>.</nobr></font>");
		radioCustom->setToolTip(tip);
		comboCustom->setToolTip(tip);
	}else{
		radioCustom->setText("Specific Values");
		radioCustom->setPalette(normal);
		radioCustom->setToolTip(customTip);
		comboCustom->setToolTip(customTip);
	}

	actualNumSteps = comboCustom->count();
	//delete existing positions
	positions.resize(actualNumSteps);
	//copying the list
	for(int i=0;i<actualNumSteps;i++){
		positions[i] = comboCustom->itemText(i).toDouble();
#ifdef VERBOSE
		cout<<"positions["<<i<<"]:"<<positions[i]<<endl;
#endif
	}

	//sets the scan
	if(SetScan(comboScript->currentIndex())==qDefs::OK){
		char cId[5];sprintf(cId,"%d",id);
		QString script = dispScript->text();
		//positions wont be loaded if its custom script
		if((comboScript->currentIndex()==CustomScript)&&((script=="")||(script=="none"))){
			qDefs::InfoMessage(string("<nobr><font color=\"blue\">Scan Level ")+string(cId)+
				string(": Values From File</font></nobr><br><br>"
				"<nobr>Positions could not be loaded as the script file path is empty.</nobr>"),"ScanWidget");
			return qDefs::FAIL;
		}else{
			if(myDet->getScanSteps(id)!=actualNumSteps){
				qDefs::WarningMessage("The positions list was not set for an unknown reason.","ScanWidget");
				LoadPositions();
				comboScript->setCurrentIndex(myDet->getScanMode(id));
				return qDefs::FAIL;
			}
			//else success is checked in enabledsizewidgets , else it does this for every add, delete etc
		}
	}

	//if num of steps = 0
	if(!spinSteps->value()){
		comboCustom->setEnabled(false);
		btnCustom->setEnabled(false);
		radioCustom->setPalette(red);
		radioCustom->setText("Specific Values*");
		QString tip = customTip + QString("<br><br><nobr><font color=\"red\">First, increase <b>Number of Steps</b>. "
				"Then, enter values here.</font></nobr>");
		radioCustom->setToolTip(tip);
		comboCustom->setToolTip(tip);
	}
	connect(comboCustom,	SIGNAL(currentIndexChanged(int)), 	this, SLOT(SetCustomSteps()));

	return qDefs::OK;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::DeleteCustomSteps(){
#ifdef VERYVERBOSE
	cout << "Entering DeleteCustomSteps()" << endl;
#endif
	QString pos = comboCustom->currentText();
	bool found = false;
	//loops through to find the index and to make sure its in the list
	for(int i=0;i<comboCustom->count();i++){
		if(!comboCustom->itemText(i).compare(pos)){
			found = true;
			comboCustom->removeItem(i);
			break;
		}
	}
	if(found){
#ifdef VERBOSE
	cout << "Deleting Position " << endl;
#endif
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::SetFileSteps(){
#ifdef VERYVERBOSE
	cout << "Entering SetFileSteps()" << endl;
#endif
	QString fName = dispFile->text();
#ifdef VERBOSE
	cout << "Setting\tscan:" << id << "\tfile\t:" << fName.toAscii().constData() << endl;
#endif
	bool set = false;

	if(fName.isEmpty()){	//blank
#ifdef VERBOSE
		cout << "Empty file" << endl;
#endif
		radioFile->setPalette(red);
		radioFile->setText("Values from File:*");
		QString errTip = fileTip + QString("<br><br><nobr><font color=\"red\">The file path is empty.</font></nobr>");;
		radioFile->setToolTip(errTip);dispFile->setToolTip(errTip);btnFile->setToolTip(errTip);
	}else{
		QString file = dispFile->text().section('/',-1);
		if(file.contains('.')){	//is a file
			//check if it exists and set the script file
			if(QFile::exists(fName)) set = true;
			else{//if the file doesnt exist, set it to what it was before
#ifdef VERBOSE
				cout << "The file entered does not exist." << endl;
#endif
				radioFile->setPalette(red);
				radioFile->setText("Values from File:*");
				QString errTip = fileTip + QString("<br><br><nobr><font color=\"red\">The file entered does not exist.</font></nobr>");
				radioFile->setToolTip(errTip);dispFile->setToolTip(errTip);btnFile->setToolTip(errTip);
			}
		} else {//not a file, set it to what it was before
#ifdef VERBOSE
			cout << "The file path entered is not a file." << endl;
#endif
			radioFile->setPalette(red);
			radioFile->setText("Values from File:*");
			QString errTip = fileTip + QString("<br><br><nobr><font color=\"red\">The file path entered is not a file.</font></nobr>");
			radioFile->setToolTip(errTip);	dispFile->setToolTip(errTip);btnFile->setToolTip(errTip);
		}
	}

	//if valid file
	if(set){
		ifstream inFile;string sLine;char sArg[200]="";
		//open file
		inFile.open(fName.toAscii().constData(), ifstream::in);
		if(inFile.is_open()){
			//delete existing positions
			positions.resize(0);
			actualNumSteps = 0;
#ifdef VERBOSE
			cout<< "Opening file "<< fName.toAscii().constData() << endl;
#endif
			while(inFile.good()) {
				getline(inFile,sLine);
				if(sLine.find('#')!=string::npos) continue;//commented out
				else if(sLine.length()<2)    continue;// empty line
				else {
					istringstream sstr(sLine);
					if(sstr.good()){
						actualNumSteps++;
						positions.resize(actualNumSteps);
						sstr>>sArg;
						if(!sscanf(sArg,"%lf",&positions[actualNumSteps-1])){
							actualNumSteps--;
							positions.resize(actualNumSteps);
						}
						else cout<<"value["<<actualNumSteps-1<<"]:"<<positions[actualNumSteps-1]<<endl;
					}
				}
			}
			disconnect(spinSteps,	SIGNAL(valueChanged(int)), 	this, SLOT(SetNSteps(int)));
			spinSteps->setValue(actualNumSteps);
			connect(spinSteps,		SIGNAL(valueChanged(int)), 	this, SLOT(SetNSteps(int)));
			inFile.close();
		}else {//could not open file
#ifdef VERBOSE
			cout << "Could not open file" << endl;
#endif
			set = false;
			radioFile->setPalette(red);
			radioFile->setText("Values from File:*");
			QString errTip = fileTip +	QString("<br><br><nobr><font color=\"red\">Could not read file.</font></nobr>");
			radioFile->setToolTip(errTip);dispFile->setToolTip(errTip);	btnFile->setToolTip(errTip);
		}
	}
	if(set){//no error while reading file
		//sets the scan and positions
		if(SetScan(comboScript->currentIndex())==qDefs::OK){
			char cId[5];sprintf(cId,"%d",id);
			QString script = dispScript->text();
			radioFile->setPalette(normal);
			radioFile->setText("Values from File:");
			radioFile->setToolTip(fileTip);dispFile->setToolTip(fileTip);btnFile->setToolTip(fileTip);
			//positions wont be loaded if its custom script
			if((comboScript->currentIndex()==CustomScript)&&((script=="")||(script=="none"))){
				qDefs::InfoMessage(string("<nobr><font color=\"blue\">Scan Level ")+string(cId)+
					string(": Values From File</font></nobr><br><br>"
					"<nobr>Positions could not be loaded as the script file path is empty.</nobr>"),"ScanWidget");
			}else{
				//error loading positions
				if(myDet->getScanSteps(id)!=actualNumSteps){
					qDefs::WarningMessage(string("<nobr><font color=\"blue\">Scan Level ")+string(cId)+
					string(": Values From File</font></nobr><br><br>"
					"<nobr>The positions list was not set for an unknown reason.</nobr>"),"ScanWidget");
				}else{//SUCCESS
					char cNum[200];sprintf(cNum,"%d",actualNumSteps);
					qDefs::InfoMessage(string("<nobr><font color=\"blue\">Scan Level ")+string(cId)+
							string(": Values From File</font></nobr><br><br><nobr>Number of positions added: ")+
							string(cNum)+string("</nobr>"),"ScanWidget");
				}
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::BrowseFileStepsPath(){
#ifdef VERYVERBOSE
	cout << "Entering BrowseFileStepsPath()" << endl;
#endif
#ifdef VERBOSE
	cout << "Browsing Steps File Path\tscan:" << id << endl;
#endif
	QString fName = dispFile->text();
	QString dir = fName.section('/',0,-2,QString::SectionIncludeLeadingSep);
	if(dir.isEmpty()) dir = "/home/";
	//dialog
	fName = QFileDialog::getOpenFileName(this,
			tr("Load Scan Steps Script File"),dir,
			tr("Scan Steps Script Files(*.awk);;All Files(*)"));
	//if empty, set the file name and it calls SetFileSteps, else ignore
	if (!fName.isEmpty()){
		dispFile->setText(fName);
		SetFileSteps();
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::LoadPositions(){
#ifdef VERYVERBOSE
	cout << "Entering LoadPositions()" << endl;
#endif
	disconnect(comboCustom,		SIGNAL(currentIndexChanged(int)), 		this, SLOT(SetCustomSteps()));
	disconnect(spinSteps,		SIGNAL(valueChanged(int)), 				this, SLOT(SetNSteps(int)));
	disconnect(btnGroup,		SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(EnableSizeWidgets()));


	int mode = myDet->getScanMode(id);
	radioCustom->setChecked(true);

	int numSteps = myDet->getScanSteps(id);
	actualNumSteps = numSteps;
	comboCustom->setMaxCount(numSteps);
	positions.resize(actualNumSteps);

	//set the number of steps in the gui
	spinSteps->setValue(numSteps);

	//load the positions
	double *values = NULL;
	if(actualNumSteps){
		values = new double[actualNumSteps];
		myDet->getScanSteps(id,values);
	}
	for(int i=0;i<actualNumSteps;i++)
		positions[i] = values[i];

	//if there are positions
	if(numSteps){
		radioCustom->setText("Specific Values");
		radioCustom->setPalette(normal);
		radioCustom->setToolTip(customTip);
		comboCustom->setToolTip(customTip);
	}//no positions and has a mode
	else if(mode){
		radioCustom->setPalette(red);
		radioCustom->setText("Specific Values*");
		QString tip = customTip + QString("<br><br><nobr><font color=\"red\">First, increase <b>Number of Steps</b>. "
				"Then, enter values here.</font></nobr>");
		radioCustom->setToolTip(tip);
		comboCustom->setToolTip(tip);
	}
	for(int i=0;i<comboCustom->count();i++)
		comboCustom->removeItem(i);
	for(int i=0;i<numSteps;i++)
		comboCustom->insertItem(i,QString("%1").arg(positions[i]));

	//delete the combolist and reload it
	comboCustom->setEnabled(numSteps&&mode);
	btnCustom->setEnabled(numSteps&&mode);

	connect(comboCustom,	SIGNAL(currentIndexChanged(int)), 	this, SLOT(SetCustomSteps()));
	connect(spinSteps,		SIGNAL(valueChanged(int)), 			this, SLOT(SetNSteps(int)));
	connect(btnGroup,		SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(EnableSizeWidgets()));


	//do not set the range variables because if the stepsize is by any chance 0..
	//then the number of steps should change to 1. so only set custom steps
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::Refresh(){
#ifdef VERYVERBOSE
	cout << "Entering Refresh()" << endl;
#endif
	int mode = (myDet->getScanMode(id));
	string script = myDet->getScanScript(id);
	string parameter = myDet->getScanParameter(id);
	int precision = myDet->getScanPrecision(id);


	//settings values and checking for none
	dispScript->setText(QString(script.c_str()));
	SetScriptFile();
	dispParameter->setText(QString(parameter.c_str()));
	SetParameter();
	spinPrecision->setValue(precision);

	//set mode which also checks number of steps
	//and enable size widgets and set the positions from combolist to server
	comboScript->setCurrentIndex(mode);


#ifdef VERBOSE
	cout << "Updated\tscan:" << id << "\t"
			"mode:"<<mode<<"\t"
			"script:" << script << "\t"
			"numSteps:" << actualNumSteps << "\t"
			//"values:" << arrSteps << "\t"
			"parameter:" << parameter << "\t"
			"precision:" << precision << endl << endl;
#endif
}


//-------------------------------------------------------------------------------------------------------------------------------------------------
