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
const string qScanWidget::modeNames[NumModes]={"","energy","threshold","trimbits","custom script"};
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
	lblSize	= new QLabel("Size",widgetRange);
	spinSize = new QDoubleSpinBox(widgetRange);
	lblFrom->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	spinFrom->setValue(0);
	lblTo->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	spinTo->setValue(1);
	lblSize->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	spinSize->setValue(1);
	layoutRange->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	layoutRange->addWidget(lblFrom);
	layoutRange->addWidget(spinFrom);
	layoutRange->addWidget(lblTo);
	layoutRange->addWidget(spinTo);
	layoutRange->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	layoutRange->addWidget(lblSize);
	layoutRange->addWidget(spinSize);
	layoutRange->addItem(new QSpacerItem(50,20,QSizePolicy::Fixed,QSizePolicy::Fixed));


	//  Custom Values Layout
	QWidget *widgetCustom = new QWidget;
	QHBoxLayout *layoutCustom = new QHBoxLayout(widgetCustom);
	layoutCustom->setContentsMargins(0, 0, 0, 0);
	comboCustom = new QComboBox(widgetCustom);
	btnCustom = new QPushButton("Delete",widgetCustom);
	comboCustom->setEditable(true);
	comboCustom->setCompleter(false);
	QDoubleValidator *validate = new QDoubleValidator(comboCustom);
	comboCustom->setValidator(validate);
	radioCustom->setToolTip("<nobr>Measures only at specific values listed by the user.</nobr><br>"
			"<nobr>Number of entries is restricted to <b>Number of Steps</b> field.</nobr>");
	comboCustom->setToolTip("<nobr>Measures only at specific values listed by the user.</nobr><br>"
			"<nobr>Number of entries is restricted to <b>Number of Steps</b> field.</nobr>");
	btnCustom->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
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
	layoutFile->addWidget(dispFile);
	btnFile = new QPushButton("Browse",widgetFile);
	btnFile->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
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
//	connect(radioRange,		SIGNAL(toggled(bool)),this,SLOT(EnableSizeWidgets()));
	//numsteps
	connect(spinSteps,		SIGNAL(valueChanged(int)), 				this, SLOT(SetNSteps()));
	//precision
	connect(spinPrecision,	SIGNAL(valueChanged(int)), 				this, SLOT(SetPrecision(int)));
	//range values
	//custom values
	connect(comboCustom,	SIGNAL(currentIndexChanged(int)), 		this, SLOT(SetCustomSteps()));
	connect(btnCustom,		SIGNAL(clicked()),						this, SLOT(DeleteCustomSteps()));
	//file values
	connect(dispFile,		SIGNAL(editingFinished()),				this, SLOT(SetFileSteps()));
	connect(btnFile,		SIGNAL(clicked()), 						this, SLOT(BrowseFileStepsPath()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::EnableSizeWidgets(){
#ifdef VERBOSE
	cout << "Entering enable size widgets" << endl;
#endif
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
		lblSteps->setEnabled(!radioRange->isChecked());
		spinSteps->setEnabled(!radioRange->isChecked());
		//range values
		if(radioRange->isChecked()){
#ifdef VERBOSE
		cout << "Constant Range Values" << endl;
#endif
			stackedLayout->setCurrentIndex(RangeValues);
			/**refresh this part*/
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
			disconnect(spinSteps,	SIGNAL(valueChanged(int)), this, SLOT(SetNSteps()));
			spinSteps ->setValue(comboCustom->count());
			connect(spinSteps,		SIGNAL(valueChanged(int)), this, SLOT(SetNSteps()));

			stackedLayout->setCurrentIndex(CustomValues);
			//only for custom steps out here because many signals go through
			//custom steps and we want to give the info msg only when changig range types
			if(SetCustomSteps()==qDefs::OK){
				char cNum[200];sprintf(cNum,"%d",actualNumSteps);
				char cId[5];sprintf(cId,"%d",id);
				qDefs::InfoMessage(string("Scan Level ")+string(cId)+(" - ")+string(" Number of positions added from list : ")+string(cNum),"ScanWidget");
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
		lblSteps->setEnabled(!radioRange->isChecked());
		spinSteps->setEnabled(!radioRange->isChecked());
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

	//emit signal to enable scanbox in plot tab
	emit EnableScanBox(mode,id);

	//set the mode
	SetScan(mode);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qScanWidget::SetScan(int mode){
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

	//setting the mode
	switch(mode){
	case None:
		myDet->setScan(id,modeNames[mode],actualNumSteps,values,parameter);
		break;
	case EnergyScan:
		myDet->setScan(id,modeNames[mode],actualNumSteps,values,parameter);
		break;
	case ThresholdScan:
		myDet->setScan(id,modeNames[mode],actualNumSteps,values,parameter);
		break;
	case TrimbitsScan:
		myDet->setScan(id,modeNames[mode],actualNumSteps,values,parameter);
		break;
	case CustomScript:
		myDet->setScan(id,script,actualNumSteps,values,parameter);
		break;
	}

	if(mode!=CustomScript){
		if((mode!=myDet->getScanMode(id))&&(actualNumSteps)){
			qDefs::WarningMessage("The mode could not be changed for an unknown reason.","ScanWidget");
			comboScript->setCurrentIndex(myDet->getScanMode(id));
			return qDefs::FAIL;
		}
	}

	return qDefs::OK;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qScanWidget::BrowsePath(){
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
	QString fName = dispScript->text();bool set = false;
#ifdef VERBOSE
	cout << "Setting\tscan:" << id << "\tscript:" << fName.toAscii().constData() << endl;
#endif


	//blank
	if(fName.isEmpty())
		set = true;
	else if(  	(!fName.compare("none"))||
				(!fName.compare("energy"))||
				(!fName.compare("threshold"))||
				(!fName.compare("trimbits"))  )
		set = true;
	//not blank
	else{
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
	if(  	(!fName.compare("none"))||
			(!fName.compare("energy"))||
			(!fName.compare("threshold"))||
			(!fName.compare("trimbits"))  )
		  dispScript->setText("");

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::SetParameter(){
	QString parameter = dispParameter->text();
#ifdef VERBOSE
	cout << "Setting\tscan:" << id << "\tparameter:" << parameter.toAscii().constData() << endl;
#endif

	myDet->setScanParameter(id,parameter.toAscii().constData());
	//dont display if theres a none
	parameter = dispParameter->text();
	if(  	(!parameter.compare("none"))||
			(!parameter.compare("energy"))||
			(!parameter.compare("threshold"))||
			(!parameter.compare("trimbits"))  )
		dispParameter->setText("");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::SetPrecision(int value){
#ifdef VERBOSE
	cout << "Setting\tscan:" << id << "\tprecision:" << value << endl;
#endif
	myDet->setScanPrecision(id,value);
	if(myDet->getScanPrecision(id)!=value)
		qDefs::WarningMessage("The precision was not set for an unknown reason.","ScanWidget");;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::SetNSteps(){

#ifdef VERBOSE
	cout << "Setting number of steps" << endl;
#endif

	int numSteps = spinSteps->value();
	comboCustom->setMaxCount(numSteps);

	//check if its ok
	if(radioCustom->isChecked()){
		SetCustomSteps();
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qScanWidget::SetCustomSteps(){

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
		cout<<"positions["<<i<<"]:"<<positions[i]<<endl;
	}
	//setting the list
	//cout<<"output:"<<myDet->setScanSteps(id,actualNumSteps,positions)<<endl;

	//sets the scan
	if(SetScan(comboScript->currentIndex())==qDefs::OK){
		if(myDet->getScanSteps(id)!=actualNumSteps){
			qDefs::WarningMessage("The positions list was not set for an unknown reason.","ScanWidget");
			LoadPositions();
			comboScript->setCurrentIndex(myDet->getScanMode(id));
			return qDefs::FAIL;
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
			disconnect(spinSteps,	SIGNAL(valueChanged(int)), 	this, SLOT(SetNSteps()));
			spinSteps->setValue(actualNumSteps);
			connect(spinSteps,		SIGNAL(valueChanged(int)), 	this, SLOT(SetNSteps()));
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
			radioFile->setPalette(normal);
			radioFile->setText("Values from File:");
			radioFile->setToolTip(fileTip);dispFile->setToolTip(fileTip);btnFile->setToolTip(fileTip);
			//error loading positions
			if(myDet->getScanSteps(id)!=actualNumSteps){
				qDefs::WarningMessage("The positions list was not set for an unknown reason.","ScanWidget");
			}else{//SUCCESS
				char cNum[200];sprintf(cNum,"%d",actualNumSteps);
				char cId[5];sprintf(cId,"%d",id);
				qDefs::InfoMessage(string("Scan Level ")+string(cId)+(" - ")+string(" Number of positions added from file : ")+string(cNum),"ScanWidget");
			}
		}
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::BrowseFileStepsPath(){
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
#ifdef VERBOSE
	cout << "Loading positions" << endl;
#endif
	disconnect(comboCustom,		SIGNAL(currentIndexChanged(int)), 		this, SLOT(SetCustomSteps()));
	disconnect(spinSteps,		SIGNAL(valueChanged(int)), 				this, SLOT(SetNSteps()));
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



	//if there are no step sizes
	if(numSteps){
		radioCustom->setText("Specific Values");
		radioCustom->setPalette(normal);
		radioCustom->setToolTip(customTip);
		comboCustom->setToolTip(customTip);
	}else{
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
	connect(spinSteps,		SIGNAL(valueChanged(int)), 			this, SLOT(SetNSteps()));
	connect(btnGroup,		SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(EnableSizeWidgets()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qScanWidget::Refresh(){
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
