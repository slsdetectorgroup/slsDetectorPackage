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

qCloneWidget::qCloneWidget(QWidget *parent,int id,QString title,QString xTitle, QString yTitle, QString zTitle,
		int numDim,string FilePath,bool displayStats, QString min, QString max, QString sum):
	QMainWindow(parent),id(id),cloneplot2D(0),cloneplot1D(0),filePath(FilePath)
	{
	// Window title
	char winTitle[300],currTime[50];
	strcpy(currTime,GetCurrentTimeStamp());
	sprintf(winTitle,"Snapshot:%d  -  %s",id,currTime);
	setWindowTitle(QString(winTitle));

	marker = new QwtSymbol();
	nomarker = new QwtSymbol();
	marker->setStyle(QwtSymbol::Cross);
	marker->setSize(5,5);

	// Set up widget
	SetupWidgetWindow(title,xTitle, yTitle, zTitle, numDim);
    DisplayStats(displayStats,min,max,sum);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

qCloneWidget::~qCloneWidget(){
	delete cloneplot1D;
	delete cloneplot2D;
	delete cloneBox;
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qCloneWidget::SetupWidgetWindow(QString title, QString xTitle, QString yTitle, QString zTitle, int numDim){

	 menubar = new QMenuBar(this);
	 actionSave = new QAction("&Save",this);
	 menubar->addAction(actionSave);
	 setMenuBar(menubar);


	//Main Window Layout
	 QWidget *centralWidget = new QWidget(this);
	 mainLayout = new QGridLayout(centralWidget);
	 centralWidget->setLayout(mainLayout);

	//plot group box
	cloneBox = new QGroupBox(this);
	gridClone = new QGridLayout(cloneBox);
	cloneBox->setLayout(gridClone);
	cloneBox->setContentsMargins(0,0,0,0);
		cloneBox->setAlignment(Qt::AlignHCenter);
		cloneBox->setFont(QFont("Sans Serif",11,QFont::Normal));
		cloneBox->setTitle(title);
	// According to dimensions, create appropriate 1D or 2Dplot
	if(numDim==1){
		cloneplot1D = new SlsQt1DPlot(cloneBox);

		cloneplot1D->setFont(QFont("Sans Serif",9,QFont::Normal));
		cloneplot1D->SetXTitle(xTitle.toAscii().constData());
		cloneplot1D->SetYTitle(yTitle.toAscii().constData());


		cloneBox->setFlat(false);
		cloneBox->setContentsMargins(0,30,0,0);
		gridClone->addWidget(cloneplot1D,0,0);

		lblHistTitle = new QLabel("");
		mainLayout->addWidget(lblHistTitle,0,0);

	}else{
		cloneplot2D = new SlsQt2DPlotLayout(cloneBox);
		cloneplot2D->setFont(QFont("Sans Serif",9,QFont::Normal));
		cloneplot2D->SetXTitle(xTitle);
		cloneplot2D->SetYTitle(yTitle);
		cloneplot2D->SetZTitle(zTitle);
		cloneplot2D->setAlignment(Qt::AlignLeft);

		cloneBox->setFlat(true);
		cloneBox->setContentsMargins(0,20,0,0);
		gridClone->addWidget(cloneplot2D,0,0);
	}

	// main window widgets
	mainLayout->addWidget(cloneBox,1,0);
	setCentralWidget(centralWidget);

	// Save
	connect(actionSave,SIGNAL(triggered()),this,SLOT(SavePlot()));

	setMinimumHeight(300);
	resize(500,350);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------

void qCloneWidget::SetCloneHists(int nHists,int histNBins,double* histXAxis,double* histYAxis[],string histTitle[],bool lines,bool markers){
	//for each plot,  create hists
	for(int hist_num=0;hist_num<nHists;hist_num++){
		SlsQtH1D*  k;
		if(hist_num+1>cloneplot1D_hists.size()){
			cloneplot1D_hists.append(k=new SlsQtH1D("1d plot",histNBins,histXAxis,histYAxis[hist_num]));
			k->SetLineColor(0);
		}else{
			k=cloneplot1D_hists.at(hist_num);
			k->SetData(histNBins,histXAxis,histYAxis[hist_num]);
		}


		//style of plot
		if(lines) 		k->setStyle(QwtPlotCurve::Lines);
		else 			k->setStyle(QwtPlotCurve::Dots);
#if QWT_VERSION<0x060000
		if(markers) 	k->setSymbol(*marker);
		else 			k->setSymbol(*nomarker);
#else
		if(markers) 	k->setSymbol(marker);
		else 			k->setSymbol(nomarker);
#endif

		//set title and attach plot
		lblHistTitle->setText(QString(histTitle[0].c_str()));

		k->Attach(cloneplot1D);
	}

}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qCloneWidget::SetCloneHists(int nHists,int histNBins,double* histXAxis,double* histYAxis,string histTitle[],bool lines,bool markers){
	// for each plot create hists
	for(int hist_num=0;hist_num<nHists;hist_num++){
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
		k->Attach(cloneplot1D);
	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


void qCloneWidget::SetCloneHists2D(int nbinsx,double xmin,double xmax,int nbinsy, double ymin, double ymax, double *d){
	cloneplot2D->GetPlot()->SetData(nbinsx,xmin,xmax,nbinsy,ymin,ymax,d);
	cloneplot2D->UpdateNKeepSetRangeIfSet();
}


//-------------------------------------------------------------------------------------------------------------------------------------------------



void qCloneWidget::SetRange(bool IsXYRange[],double XYRangeValues[]){
	double XYCloneRangeValues[4];

	if(!IsXYRange[qDefs::XMINIMUM]){
		if(cloneplot1D)					XYCloneRangeValues[qDefs::XMINIMUM]= cloneplot1D->GetXMinimum();
		else							XYCloneRangeValues[qDefs::XMINIMUM]= cloneplot2D->GetPlot()->GetXMinimum();
	}else 								XYCloneRangeValues[qDefs::XMINIMUM]= XYRangeValues[qDefs::XMINIMUM];

	if(!IsXYRange[qDefs::XMAXIMUM]){
		if(cloneplot1D)					XYCloneRangeValues[qDefs::XMAXIMUM]= cloneplot1D->GetXMaximum();
		else							XYCloneRangeValues[qDefs::XMINIMUM]= cloneplot2D->GetPlot()->GetXMaximum();
	}else 								XYCloneRangeValues[qDefs::XMAXIMUM]= XYRangeValues[qDefs::XMAXIMUM];

	if(!IsXYRange[qDefs::YMINIMUM]){
		if(cloneplot1D)					XYCloneRangeValues[qDefs::YMINIMUM]= cloneplot1D->GetYMinimum();
		else							XYCloneRangeValues[qDefs::XMINIMUM]= cloneplot2D->GetPlot()->GetYMinimum();
	}else 								XYCloneRangeValues[qDefs::YMINIMUM]= XYRangeValues[qDefs::YMINIMUM];

	if(!IsXYRange[qDefs::YMAXIMUM]){
		if(cloneplot1D)	 				XYCloneRangeValues[qDefs::YMAXIMUM]= cloneplot1D->GetYMaximum();
		else							XYCloneRangeValues[qDefs::XMINIMUM]= cloneplot2D->GetPlot()->GetYMaximum();
	}else 								XYCloneRangeValues[qDefs::YMAXIMUM]= XYRangeValues[qDefs::YMAXIMUM];

	if(cloneplot1D){
		cloneplot1D->SetXMinMax(XYCloneRangeValues[qDefs::XMINIMUM],XYCloneRangeValues[qDefs::XMAXIMUM]);
		cloneplot1D->SetYMinMax(XYCloneRangeValues[qDefs::YMINIMUM],XYCloneRangeValues[qDefs::YMAXIMUM]);
	}else{
		cloneplot2D->GetPlot()->SetXMinMax(XYRangeValues[qDefs::XMINIMUM],XYRangeValues[qDefs::XMAXIMUM]);
		cloneplot2D->GetPlot()->SetYMinMax(XYRangeValues[qDefs::YMINIMUM],XYRangeValues[qDefs::YMAXIMUM]);
		cloneplot2D->GetPlot()->Update();
	}
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


void qCloneWidget::DisplayStats(bool enable, QString min, QString max, QString sum){
	if(enable){
		QWidget *widgetStatistics = new QWidget(this);
		widgetStatistics->setFixedHeight(15);
		QHBoxLayout *hl1 = new QHBoxLayout;
		hl1->setSpacing(0);
		hl1->setContentsMargins(0, 0, 0, 0);
		QLabel *lblMin = new QLabel("Min:  ");
		lblMin->setFixedWidth(40);
		lblMin->setAlignment(Qt::AlignRight);
		QLabel *lblMax = new QLabel("Max:  ");
		lblMax->setFixedWidth(40);
		lblMax->setAlignment(Qt::AlignRight);
		QLabel *lblSum = new QLabel("Sum:  ");
		lblSum->setFixedWidth(40);
		lblSum->setAlignment(Qt::AlignRight);
		QLabel *lblMinDisp = new QLabel(min);
		lblMinDisp->setAlignment(Qt::AlignLeft);
		QLabel *lblMaxDisp = new QLabel(max);
		lblMaxDisp->setAlignment(Qt::AlignLeft);
		QLabel *lblSumDisp = new QLabel(sum);
		lblSumDisp->setAlignment(Qt::AlignLeft);
		hl1->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
		hl1->addWidget(lblMin);
		hl1->addWidget(lblMinDisp);
		hl1->addItem(new QSpacerItem(20,20,QSizePolicy::Expanding,QSizePolicy::Fixed));
		hl1->addWidget(lblMax);
		hl1->addWidget(lblMaxDisp);
		hl1->addItem(new QSpacerItem(20,20,QSizePolicy::Expanding,QSizePolicy::Fixed));
		hl1->addWidget(lblSum);
		hl1->addWidget(lblSumDisp);
		hl1->addItem(new QSpacerItem(20,20,QSizePolicy::Fixed,QSizePolicy::Fixed));
		widgetStatistics->setLayout(hl1);
		mainLayout->addWidget(widgetStatistics,2,0);
		widgetStatistics->show();

	}
}


//-------------------------------------------------------------------------------------------------------------------------------------------------


