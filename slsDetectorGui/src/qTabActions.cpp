/*
 * qTabActions.cpp
 *
 *  Created on: May 10, 2012
 *      Author: l_maliakal_d
 */
// Qt Project Class Headers
#include "qTabActions.h"
// Project Class Headers
#include "slsDetector.h"
#include "multiSlsDetector.h"
// Qt Include Headers
#include <QButtonGroup>
// C++ Include Headers
#include<iostream>
using namespace std;


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabActions::qTabActions(QWidget *parent,multiSlsDetector*& detector):
				  QWidget(parent),myDet(detector),
				  positionWidget(0),
				  lblNumPos(0),
				  lblPosList(0),
				  spinNumPos(0),
				  comboPos(0),
				  btnDelete(0),
				  chkInvert(0),
				  chkSeparate(0),
				  chkReturn(0),
				  positions(0),
				  iconPlus(0),
				  iconMinus(0){
	for(int i=0;i<6;i++)
		actionWidget[i]=0;
	for(int i=0;i<2;i++)
		scanWidget[i]=0;
	for(int i=0;i<NumTotalActions;i++){
		btnExpand[i]=0;
		lblName[i]=0;
	}
	SetupWidgetWindow();
	Initialization();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


qTabActions::~qTabActions(){
	delete myDet;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabActions::SetupWidgetWindow(){
	// Window Settings
	setFixedHeight(350);
	//setFixedSize(710,350);
	setContentsMargins(0,0,0,0);

	// Scroll Area Settings
	QScrollArea *scroll = new QScrollArea;
	scroll->setWidget(this);
	scroll->setWidgetResizable(true);

	// Layout Settings
	gridLayout = new QGridLayout(scroll);
	setLayout(gridLayout);
	gridLayout->setContentsMargins(10,5,0,0);
	gridLayout->setVerticalSpacing(2);

	// Buttongroup to know which +/- button was clicked
	group 			= new QButtonGroup(this);
	group->setExclusive(false);
	palette 		= new QPalette();

	QPalette p;
	p.setColor(QPalette::Shadow,QColor(0,0,0,0));
	p.setColor(QPalette::Button,QColor(0,0,0,0));
/*get rid of this vector*/
	char names[NumTotalActions][200] = {
			"Action at Start",
			"Scan Level 0",
			"Scan Level 1",
			"Action before each Frame",
			"Positions",
			"Header before Frame",
			"Header after Frame",
			"Action after each Frame",
			"Action at Stop"
	};

	//creating the icons for the buttons
	iconPlus = new QIcon(":/icons/images/add.png");
	iconMinus = new QIcon(":/icons/images/remove.png");

	QString tip = "<nobr>Click on the \"+\" to Expand or \"-\" to Collapse.</nobr>";



	int hIndent=0, vIndent=0, colspan=6, posVal=0;
	QLabel *lblReal;
	// For each level of Actions
	for(int i=0;i<NumTotalActions;i++){
		//common widgets
		lblName[i]	 	= new QLabel(QString(names[i]));
		btnExpand[i] 	= new QPushButton();

		lblName[i]->setToolTip(tip);
		btnExpand[i]->setCheckable(true);
		btnExpand[i]->setChecked(false);
		btnExpand[i]->setFixedSize(16,16);
		btnExpand[i]->setToolTip(tip);
		btnExpand[i]->setIcon(*iconPlus);
		btnExpand[i]->setFocusPolicy(Qt::NoFocus);
		btnExpand[i]->setFlat(true);
		btnExpand[i]->setIconSize(QSize(16,16));
		btnExpand[i]->setPalette(p);
		group->addButton(btnExpand[i],i);


		//add label and button to expand or collapse
		gridLayout->addWidget(btnExpand[i],vIndent,hIndent,1,1);
		gridLayout->addWidget(lblName[i],vIndent,hIndent+1,1,colspan);

		//creating the action/scan/position widgets and adding them
		switch(i){
		case NumPositions:
			CreatePositionsWidget();
			gridLayout->addWidget(positionWidget,vIndent+1,hIndent+1,1,colspan);
			positionWidget->hide();
			break;
		case Scan0:
		case Scan1:
		        posVal = qScanWidget::NUM_SCAN_WIDGETS;
			scanWidget[posVal] = new qScanWidget(this,myDet);
			gridLayout->addWidget(scanWidget[posVal],vIndent+1,hIndent+1,1,colspan);
			scanWidget[posVal]->hide();
			break;
		default:
		        posVal = qActionsWidget::NUM_ACTION_WIDGETS;
			actionWidget[posVal] = new qActionsWidget(this,myDet);
			gridLayout->addWidget(actionWidget[posVal],vIndent+1,hIndent+1,1,colspan);
			actionWidget[posVal]->hide();
			break;
		}

		//incrementing the vertical and horizontal indent
		vIndent+=2;
		switch(i){
		case HeaderBefore:
			//real time acquisition
			palette->setColor(QPalette::Active,QPalette::WindowText,QColor(0,0,200,255));
			lblReal = new QLabel("      <b>Real Time Acquisition</b>");
			lblReal->setFixedHeight(25);
			//lblReal->setPalette(*palette);
			gridLayout->addWidget(lblReal,vIndent,hIndent+1,1,colspan);
			vIndent++;
			break;
		case HeaderAfter:
			hIndent-=2;
			colspan+=2;
			break;
		case ActionAfter:
			hIndent=0;
			colspan=6;
			break;
		default:
			hIndent++;
			colspan--;
			break;
		}

	}


	//Number of positions is only for mythen or gotthard
	detType = myDet->getDetectorsType();
	if((detType == slsDetectorDefs::EIGER) ||
			(detType == slsDetectorDefs::AGIPD) ||
			(detType == slsDetectorDefs::PROPIX) ||
			(detType == slsDetectorDefs::MOENCH)) {
		lblName[NumPositions]->setEnabled(false);
		btnExpand[NumPositions]->setEnabled(false);
	}else{
		//disable positions if angular conversion is enabled
		int ang;
		if(!myDet->getAngularConversion(ang)){
			lblName[NumPositions]->setEnabled(false);
			btnExpand[NumPositions]->setEnabled(false);
		}

	}

	//load positions
	if(lblName[NumPositions]->isEnabled()){
		//delete existing positions
		if (positions)  {delete [] positions; positions = NULL;}
		//get number of positions
		int numPos=myDet->getPositions();
		comboPos->setMaxCount(numPos);

		//set the number of positions in the gui
		spinNumPos->setValue(numPos);

		positions=new double[numPos];
		//load the positions
		myDet->getPositions(positions);

		//delete the combolist and reload it
		comboPos->setEnabled(numPos);
		lblPosList->setEnabled(numPos);
		btnDelete->setEnabled(numPos);
		lblPosList->setText("List of Positions: ");
		lblPosList->setPalette(normal);
		for(int i=0;i<comboPos->count();i++)		comboPos->removeItem(i);
		for(int i=0;i<numPos;i++)			comboPos->insertItem(i,QString("%1").arg(positions[i]));
	}


	qDefs::checkErrorMessage(myDet,"qTabActions::SetupWidgetWindow");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabActions::CreatePositionsWidget(){
	positionWidget = new QWidget;
	positionWidget->setFixedHeight(25);
	positionWidget->setFixedWidth(680);

	QGridLayout *layout = new QGridLayout(positionWidget);
	layout->setContentsMargins(0,0,0,0);
	layout->setHorizontalSpacing(0);
	layout->setVerticalSpacing(5);

	lblNumPos = new QLabel("Number of Positions: ");
	lblNumPos->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	layout->addWidget(lblNumPos,0,0);
	layout->addItem(new QSpacerItem(5,20,QSizePolicy::Fixed,QSizePolicy::Fixed),0,1);
	spinNumPos = new QSpinBox(this);
	layout->addWidget(spinNumPos,0,2);
	layout->addItem(new QSpacerItem(80,20,QSizePolicy::Fixed,QSizePolicy::Fixed),0,3);
	lblPosList = new QLabel("List of Positions: ");
	lblPosList->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	lblPosList->setFixedWidth(108);
	lblPosList->setEnabled(false);
	lblPosList->setToolTip("<nobr>Enter the positions at which the detector should be moved.</nobr><br>"
				"<nobr>Number of entries is restricted to <b>Number of Positions</b> field.</tnobr>");
	layout->addWidget(lblPosList,0,4);
	comboPos = new QComboBox(this);
	comboPos->setEditable(true);
	comboPos->setCompleter(false);
	normal = comboPos->palette();
	comboPos->setEnabled(false);
	QDoubleValidator *validate = new QDoubleValidator(comboPos);
	comboPos->setValidator(validate);
	layout->addWidget(comboPos,0,5);
	layout->addItem(new QSpacerItem(5,20,QSizePolicy::Fixed,QSizePolicy::Fixed),0,6);
	btnDelete = new QPushButton("Delete  ");
	btnDelete->setEnabled(false);
	btnDelete->setIcon(QIcon( ":/icons/images/close.png" ));
	btnDelete->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	layout->addWidget(btnDelete,0,7);

	//might be included at some point
/*	QGroupBox *w = new QGroupBox;
	layout->addWidget(w,1,0,1,9);
	QHBoxLayout *l1 = new QHBoxLayout(w);
	l1->setContentsMargins(0,0,0,0);
	l1->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	chkInvert = new QCheckBox("Invert Angles");
	l1->addWidget(chkInvert);
	chkSeparate = new QCheckBox("Separate Two Halves");
	l1->addWidget(chkSeparate);
	chkReturn = new QCheckBox("Return to Start Position");
	chkReturn->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
	l1->addWidget(chkReturn);
	l1->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
	w->setLayout(l1);*/

	positionWidget->setLayout(layout);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabActions::Initialization(){
	//expand
	connect(group,				SIGNAL(buttonClicked(QAbstractButton*)),	this,SLOT(Expand(QAbstractButton*)));
	//enable scan box in plot tab
	for(int i=0;i<qScanWidget::NUM_SCAN_WIDGETS;i++)
		connect(scanWidget[i],	SIGNAL(EnableScanBox()),			this,SIGNAL(EnableScanBox()));
	//positions
	connect(comboPos,		SIGNAL(currentIndexChanged(int)), 		this, SLOT(SetPosition()));
	connect(spinNumPos,		SIGNAL(valueChanged(int)), 				this, SLOT(SetPosition()));
	connect(btnDelete,		SIGNAL(clicked()),						this, SLOT(DeletePosition()));
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabActions::Expand(QAbstractButton *button ){
	int index = group->id(button);

	// Collapse
	if(!button->isChecked()){
		palette->setColor(QPalette::Active,QPalette::WindowText,Qt::black);


		lblName[index]->setPalette(*palette);
		button->setIcon(*iconPlus);

		if(index==NumPositions)	{
			positionWidget->hide();
			setFixedHeight(height()-30);//-80 if the checkboxes are included
			if(myDet->getPositions()) {
				palette->setColor(QPalette::Active,QPalette::WindowText,Qt::darkGreen);
				lblName[index]->setPalette(*palette);
			}
		}
		else if((index==Scan0)||(index==Scan1)) {
			scanWidget[GetActualIndex(index)]->hide();
			setFixedHeight(height()-130);
			if(myDet->getScanMode(GetActualIndex(index))){
				palette->setColor(QPalette::Active,QPalette::WindowText,Qt::darkGreen);
				lblName[index]->setPalette(*palette);
			}
		}
		else {
			actionWidget[GetActualIndex(index)]->hide();
			setFixedHeight(height()-30);
			if(myDet->getActionMode(GetActualIndex(index))){
				palette->setColor(QPalette::Active,QPalette::WindowText,Qt::darkGreen);
				lblName[index]->setPalette(*palette);
			}
		}
	}else{
		// Expand
		//always set title color to blue for expan\d

		palette->setColor(QPalette::Active,QPalette::WindowText,QColor(0,0,200,255));


		lblName[index]->setPalette(*palette);
		button->setIcon(*iconMinus);

		if(index==NumPositions){
			positionWidget->show();
			setFixedHeight(height()+30);//+80 if the checkboxes are included
		}
		else if((index==Scan0)||(index==Scan1)){
			scanWidget[GetActualIndex(index)]->show();
			setFixedHeight(height()+130);
		}
		else{
			actionWidget[GetActualIndex(index)]->show();
			setFixedHeight(height()+30);
		}
	}

	qDefs::checkErrorMessage(myDet,"qTabActions::Expand");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabActions::SetPosition(){
#ifdef VERBOSE
	cout << "Entering SetPosition\tnum Pos:" << spinNumPos->value() << "\tlist count:" << comboPos->count() << endl;
#endif
	//get number of positions
	int numPos = spinNumPos->value();
	comboPos->setMaxCount(numPos);
	comboPos->setEnabled(numPos);
	lblPosList->setEnabled(numPos);
	btnDelete->setEnabled(numPos);

	//deleting too many or not entering enough
	if(numPos>comboPos->count()){

		QPalette red = QPalette();
		red.setColor(QPalette::Active,QPalette::WindowText,Qt::red);
		lblPosList->setPalette(red);
		QString tip = QString("<nobr>Enter the positions at which the detector should be moved.</nobr><br>"
				"<nobr>Number of entries is restricted to <b>Number of Positions</b> field.</nobr><br><br>")+
				QString("<font color=\"red\"><nobr>Add ")+
				(QString("%1").arg(((numPos)-(comboPos->count()))))+
				QString(" more positions to the list to match <b>Number of Positions</b>.</nobr><br>"
						"<nobr><nobr>Or reduce <b>Number of Positions</b>.</nobr></font>");
		lblPosList->setToolTip(tip);
		lblPosList->setText("List of Positions:*");
	}else{
		lblPosList->setText("List of Positions: ");
		lblPosList->setPalette(normal);
		lblPosList->setToolTip("<nobr>Enter the positions at which the detector should be moved.</nobr><br>"
				"<nobr>Number of entries is restricted to <b>Number of Positions</b> field.</nobr>");
	}

	//delete existing positions
	if (positions)  {delete [] positions; positions = NULL;}
	positions=new double[comboPos->count()];
	//copying the list
	for(int i=0;i<comboPos->count();i++)
		positions[i] = comboPos->itemText(i).toDouble();
	//setting the list and catching error
	if(myDet->setPositions(comboPos->count(),positions)!=comboPos->count())
		qDefs::Message(qDefs::WARNING,"The positions list was not set for some reason.","qTabActions::SetPosition");


	qDefs::checkErrorMessage(myDet,"qTabActions::SetPosition");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabActions::DeletePosition(){
	QString pos = comboPos->currentText();
	bool found = false;
	//loops through to find the index and to make sure its in the list
	for(int i=0;i<comboPos->count();i++){
		if(!comboPos->itemText(i).compare(pos)){
			found = true;
			comboPos->removeItem(i);
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

void qTabActions::EnablePositions(bool enable){
#ifdef VERBOSE
	cout << "Enable Positions: " << enable << endl;
#endif
	if(enable){
		lblName[NumPositions]->setEnabled(true);
		btnExpand[NumPositions]->setEnabled(true);
	}else{
		//deletes all positions
		for(int i=0;i<comboPos->count();i++)
			comboPos->removeItem(i);
		cout<<"Number of Positions set to :"<<myDet->getPositions()<<endl;

		//to collapse if it was expanded
		if(btnExpand[NumPositions]->isChecked()){
			disconnect(group,				SIGNAL(buttonClicked(QAbstractButton*)),	this,SLOT(Expand(QAbstractButton*)));
			btnExpand[NumPositions]->setChecked(false);
			connect(group,				SIGNAL(buttonClicked(QAbstractButton*)),	this,SLOT(Expand(QAbstractButton*)));
			Expand(group->button(NumPositions));
		}
		lblName[NumPositions]->setEnabled(false);
		btnExpand[NumPositions]->setEnabled(false);
	}

	qDefs::checkErrorMessage(myDet,"qTabActions::EnablePositions");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabActions::Refresh(){
#ifdef VERBOSE
	cout << endl <<"**Updating all action widgets: " << endl;
#endif
	if((detType == slsDetectorDefs::MYTHEN) || (detType == slsDetectorDefs::GOTTHARD)){
		//positions is enabled only if angular conversion is enabled
		int ang; if(!myDet->getAngularConversion(ang))	EnablePositions(false);

		if(lblName[NumPositions]->isEnabled()){
			//delete existing positions
			if (positions)  {delete [] positions; positions = NULL;}
			//get number of positions
			int numPos=myDet->getPositions();
			comboPos->setMaxCount(numPos);

			//set the number of positions in the gui
			disconnect(spinNumPos,	SIGNAL(valueChanged(int)), 	this, SLOT(SetPosition()));
			spinNumPos->setValue(numPos);
			connect(spinNumPos,		SIGNAL(valueChanged(int)), 	this, SLOT(SetPosition()));

			positions=new double[numPos];
			//load the positions
			myDet->getPositions(positions);

			//delete the combolist and reload it
			disconnect(comboPos,SIGNAL(currentIndexChanged(int)), this, SLOT(SetPosition()));
			comboPos->setEnabled(numPos);
			lblPosList->setEnabled(numPos);
			btnDelete->setEnabled(numPos);
			lblPosList->setText("List of Positions: ");
			lblPosList->setPalette(normal);
			for(int i=0;i<comboPos->count();i++)
				comboPos->removeItem(i);
			for(int i=0;i<numPos;i++)
				comboPos->insertItem(i,QString("%1").arg(positions[i]));
			connect(comboPos,	SIGNAL(currentIndexChanged(int)), this, SLOT(SetPosition()));


#ifdef VERBOSE
			cout << "Updated position widget\tnum:" << numPos << "\t***" << endl;
#endif
		}
	}
	for(int i=0;i<qScanWidget::NUM_SCAN_WIDGETS;i++)
		scanWidget[i]->Refresh();
	for(int i=0;i<qActionsWidget::NUM_ACTION_WIDGETS;i++)
		actionWidget[i]->Refresh();
	UpdateCollapseColors();
#ifdef VERBOSE
	cout << "**Updated all action widgets: " << endl << endl;
#endif
	qDefs::checkErrorMessage(myDet,"qTabActions::Refresh");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qTabActions::GetActualIndex(int index){
	switch(index){
	case 0: 			return slsDetectorDefs::startScript;
	case Scan0:			return 0;
	case Scan1: 		return 1;
	case 3:				return slsDetectorDefs::scriptBefore;
	case 5:				return slsDetectorDefs::headerBefore;
	case 6:				return slsDetectorDefs::headerAfter;
	case 7:				return slsDetectorDefs::scriptAfter;
	case 8:				return slsDetectorDefs::stopScript;
	default:			return -1;
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qTabActions::UpdateCollapseColors(){
#ifdef VERYVERBOSE
	cout << "Updating Collapse Colors" << endl;
#endif
	for(int i=0;i<NumTotalActions;i++){
		//num positions
		if(i==NumPositions){
			if(myDet->getPositions()) 	palette->setColor(QPalette::Active,QPalette::WindowText,Qt::darkGreen);
			else						palette->setColor(QPalette::Active,QPalette::WindowText,Qt::black);
			lblName[i]->setPalette(*palette);
		}
		//scans
		else if((i==Scan0)||(i==Scan1)){
			if(myDet->getScanMode(GetActualIndex(i)))		palette->setColor(QPalette::Active,QPalette::WindowText,Qt::darkGreen);
			else											palette->setColor(QPalette::Active,QPalette::WindowText,Qt::black);
			lblName[i]->setPalette(*palette);
		}
		//actions
		else{
			if(myDet->getActionMode(GetActualIndex(i)))		palette->setColor(QPalette::Active,QPalette::WindowText,Qt::darkGreen);
			else											palette->setColor(QPalette::Active,QPalette::WindowText,Qt::black);
			lblName[i]->setPalette(*palette);
		}
	}

}
//-------------------------------------------------------------------------------------------------------------------------------------------------
