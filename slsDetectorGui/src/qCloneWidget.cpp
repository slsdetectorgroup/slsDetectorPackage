/*
 * qCloneWidget.cpp
 *
 *  Created on: May 18, 2012
 *      Author: l_maliakal_d
 */

/** Qt Project Class Headers */
#include "qCloneWidget.h"
/** Qt Include Headers */
#include <QImage>
#include <QPainter>
#include <QFileDialog>
#include "qwt_symbol.h"
/** C++ Include Headers */



//-------------------------------------------------------------------------------------------------------------------------------------------------

qCloneWidget::qCloneWidget(QWidget *parent,int id,QString title,int numDim,SlsQt1DPlot*& plot1D,SlsQt2DPlotLayout*& plot2D,string FilePath):
	QMainWindow(parent),id(id),cloneplot2D(0),cloneplot1D(0),filePath(FilePath){
	/** Window title*/
	char winTitle[300],currTime[50];
	strcpy(currTime,GetCurrentTimeStamp());
	sprintf(winTitle,"Snapshot:%d  -  %s",id,currTime);
	setWindowTitle(QString(winTitle));

	/** Set up widget*/
	SetupWidgetWindow(title,numDim,plot1D,plot2D);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

qCloneWidget::~qCloneWidget(){
	delete cloneplot1D;
	delete cloneplot2D;
	delete cloneBox;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qCloneWidget::SetupWidgetWindow(QString title,int numDim,SlsQt1DPlot*& plot1D,SlsQt2DPlotLayout*& plot2D){

	 menubar = new QMenuBar(this);
	 //menuFile = new QMenu("&File",menubar);
	 actionSave = new QAction("&Save",this);
	// menubar->addAction(menuFile->menuAction());
	 menubar->addAction(actionSave);
	 setMenuBar(menubar);


	/** Main Window Layout */
	 QWidget *centralWidget = new QWidget(this);
	 mainLayout = new QGridLayout(centralWidget);
	 centralWidget->setLayout(mainLayout);

	/** plot group box*/
	cloneBox = new QGroupBox(this);
	gridClone = new QGridLayout(cloneBox);
	cloneBox->setLayout(gridClone);
	cloneBox->setContentsMargins(0,0,0,0);
		cloneBox->setTitle(title);
		cloneBox->setAlignment(Qt::AlignHCenter);
		cloneBox->setFont(QFont("Sans Serif",11,QFont::Normal));
	/** According to dimensions, create appropriate 1D or 2Dplot */
	if(numDim==1){
		cloneplot1D = plot1D;
		gridClone->addWidget(cloneplot1D,0,0);
		cloneBox->setFlat(false);
		cloneBox->setContentsMargins(0,30,0,0);
		lblHistTitle = new QLabel("");
		mainLayout->addWidget(lblHistTitle,0,0);

	}else{
		cloneplot2D = plot2D;
		//cloneplot2D->setContentsMargins(0,0,0,0);
		gridClone->addWidget(cloneplot2D,0,0);
		cloneBox->setFlat(true);
		cloneBox->setContentsMargins(0,20,0,0);
	}

	/** main window widgets */
	mainLayout->addWidget(cloneBox,1,0);
	setCentralWidget(centralWidget);

	/** Save */
	connect(actionSave,SIGNAL(triggered()),this,SLOT(SavePlot()));

	setMinimumHeight(300);
	resize(500,350);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qCloneWidget::SetCloneHists(int nHists,int histNBins,double* histXAxis,double* histYAxis[],string histTitle[],bool lines,bool markers){
	/** for each plot*/
	for(int hist_num=0;hist_num<nHists;hist_num++){
		/** create hists */
		SlsQtH1D*  k;
		if(hist_num+1>cloneplot1D_hists.size()){
			cloneplot1D_hists.append(k=new SlsQtH1D("1d plot",histNBins,histXAxis,histYAxis[hist_num]));
			k->SetLineColor(hist_num+1);
		}else{
			k=cloneplot1D_hists.at(hist_num);
			k->SetData(histNBins,histXAxis,histYAxis[hist_num]);
		}
		//style of plot
		if(lines) 	k->setStyle(QwtPlotCurve::Lines);
		else 		k->setStyle(QwtPlotCurve::Dots);
		if(markers) {
			QwtSymbol *marker = new QwtSymbol();
			marker->setStyle(QwtSymbol::Cross);
			marker->setSize(5,5);
#if QWT_VERSION<0x060000
			k->setSymbol(*marker);
#else
			k->setSymbol(marker);
#endif
		}else {
			QwtSymbol *noMarker = new QwtSymbol();
#if QWT_VERSION<0x060000
			k->setSymbol(*noMarker);
#else
			k->setSymbol(noMarker);
#endif
		}
		//set title and attach plot
		lblHistTitle->setText(QString(histTitle[0].c_str()));
		//k->setTitle(histTitle[hist_num].c_str());
		k->Attach(cloneplot1D);
	}
	//cloneplot1D->UnZoom();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qCloneWidget::SetCloneHists(int nHists,int histNBins,double* histXAxis,double* histYAxis,string histTitle[],bool lines,bool markers){
	/** for each plot*/
	for(int hist_num=0;hist_num<nHists;hist_num++){
		/** create hists */
		SlsQtH1D*  k;
		if(hist_num+1>cloneplot1D_hists.size()){
			cloneplot1D_hists.append(k=new SlsQtH1D("1d plot",histNBins,histXAxis,histYAxis));
			k->SetLineColor(hist_num+1);
		}else{
			k=cloneplot1D_hists.at(hist_num);
			k->SetData(histNBins,histXAxis,histYAxis);
		}
		//style of plot
		if(lines) 	k->setStyle(QwtPlotCurve::Lines);
		else 		k->setStyle(QwtPlotCurve::Dots);
		if(markers) {
			QwtSymbol *marker = new QwtSymbol();
			marker->setStyle(QwtSymbol::Cross);
			marker->setSize(5,5);
#if QWT_VERSION<0x060000
			k->setSymbol(*marker);
#else
			k->setSymbol(marker);
#endif
		}else {
			QwtSymbol *noMarker = new QwtSymbol();
#if QWT_VERSION<0x060000
			k->setSymbol(*noMarker);
#else
			k->setSymbol(noMarker);
#endif
		}
		//set title and attach plot
		lblHistTitle->setText(QString(histTitle[0].c_str()));
		//k->setTitle(histTitle[hist_num].c_str());
		k->Attach(cloneplot1D);
	}
	//cloneplot1D->UnZoom();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------


void qCloneWidget::SetRange(bool IsXYRange[],double XYRangeValues[]){
	double XYCloneRangeValues[4];
	if(!IsXYRange[qDefs::XMINIMUM])		XYCloneRangeValues[qDefs::XMINIMUM]= cloneplot1D->GetXMinimum();
	else 								XYCloneRangeValues[qDefs::XMINIMUM]= XYRangeValues[qDefs::XMINIMUM];
	if(!IsXYRange[qDefs::XMAXIMUM])		XYCloneRangeValues[qDefs::XMAXIMUM]= cloneplot1D->GetXMaximum();
	else 								XYCloneRangeValues[qDefs::XMAXIMUM]= XYRangeValues[qDefs::XMAXIMUM];
	if(!IsXYRange[qDefs::YMINIMUM])		XYCloneRangeValues[qDefs::YMINIMUM]= cloneplot1D->GetYMinimum();
	else 								XYCloneRangeValues[qDefs::YMINIMUM]= XYRangeValues[qDefs::YMINIMUM];
	if(!IsXYRange[qDefs::YMAXIMUM])		XYCloneRangeValues[qDefs::YMAXIMUM]= cloneplot1D->GetYMaximum();
	else 								XYCloneRangeValues[qDefs::YMAXIMUM]= XYRangeValues[qDefs::YMAXIMUM];
	cloneplot1D->SetXMinMax(XYCloneRangeValues[qDefs::XMINIMUM],XYCloneRangeValues[qDefs::XMAXIMUM]);
	cloneplot1D->SetYMinMax(XYCloneRangeValues[qDefs::YMINIMUM],XYCloneRangeValues[qDefs::YMAXIMUM]);
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


char* qCloneWidget::GetCurrentTimeStamp(){
	char output[30];
	char *result;

	//using sys cmds to get output or str
	FILE* sysFile = popen("date", "r");
	fgets(output, sizeof(output), sysFile);
	pclose(sysFile);

	result = output + 0;
	return result;
}


//-------------------------------------------------------------------------------------------------------------------------------------------------

void qCloneWidget::SavePlot(){
	char cID[10];
	sprintf(cID,"%d",id);
	//title
	QString fName = QString(filePath.c_str());
	if(cloneBox->title().contains('.')){
		fName.append(QString('/')+cloneBox->title());
		fName.replace(".dat",".png");
		fName.replace(".raw",".png");
	}else  fName.append(QString("/Snapshot_unknown_title.png"));
	//save
	QImage img(cloneBox->size().width(),cloneBox->size().height(),QImage::Format_RGB32);
	QPainter painter(&img);
	cloneBox->render(&painter);

    fName = QFileDialog::getSaveFileName(this,tr("Save Snapshot "),fName,tr("PNG Files (*.png);;XPM Files(*.xpm);;JPEG Files(*.jpg)"),0,QFileDialog::ShowDirsOnly);
    if (!fName.isEmpty())
    	if((img.save(fName)))
    		qDefs::Message(qDefs::INFORMATION,"The SnapShot has been successfully saved","qCloneWidget::SavePlot");
    	else
    		qDefs::Message(qDefs::WARNING,"Attempt to save snapshot failed.\n"
    				"Formats: .png, .jpg, .xpm.","qCloneWidget::SavePlot");
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


int qCloneWidget::SavePlotAutomatic(){
	char cID[10];
	sprintf(cID,"%d",id);
	//title
	QString fName = QString(filePath.c_str());
	if(cloneBox->title().contains('.')){
		fName.append(QString('/')+cloneBox->title());
		fName.replace(".dat",".png");
		fName.replace(".raw",".png");
	}else  fName.append(QString("/Snapshot_unknown_title.png"));
	cout<<"fname:"<<fName.toAscii().constData()<<endl;
	//save
	QImage img(cloneBox->size().width(),cloneBox->size().height(),QImage::Format_RGB32);
	QPainter painter(&img);
	cloneBox->render(&painter);
	if(img.save(fName))
		return 0;
	else return -1;

}
//-------------------------------------------------------------------------------------------------------------------------------------------------

void qCloneWidget::closeEvent(QCloseEvent* event){
	emit CloneClosedSignal(id);
	event->accept();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------
