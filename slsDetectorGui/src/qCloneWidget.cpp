#include "qCloneWidget.h"
#include "qDefs.h"
#include "SlsQt1DPlot.h"
#include "SlsQt2DPlotLayout.h"

#include "qwt_symbol.h"
#include <QWidget>
#include <QCloseEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSpacerItem>
#include <QFileDialog>
#include <QImage>
#include <QPainter>

qCloneWidget::qCloneWidget(QWidget *parent, int id, QString title, QString xTitle, QString yTitle, QString zTitle,
                           int numDim, QString fPath, QString fName, int fIndex, bool displayStats, QString min, QString max, QString sum) : 
                           QMainWindow(parent), id(id), filePath(fPath), fileName(fName), fileIndex(fIndex), cloneplot1D(nullptr), cloneplot2D(nullptr),
                           marker(nullptr), nomarker(nullptr), mainLayout(nullptr), boxPlot(nullptr), lblHistTitle(nullptr) {
    // Window title
    char winTitle[300], currTime[50];
    strcpy(currTime, GetCurrentTimeStamp());
    sprintf(winTitle, "Snapshot:%d  -  %s", id, currTime);
    setWindowTitle(QString(winTitle));

    marker = new QwtSymbol();
    nomarker = new QwtSymbol();
    marker->setStyle(QwtSymbol::Cross);
    marker->setSize(5, 5);

    // Set up widget
    SetupWidgetWindow(title, xTitle, yTitle, zTitle, numDim);
    DisplayStats(displayStats, min, max, sum);
}

qCloneWidget::~qCloneWidget() {
    if (cloneplot1D)
        delete cloneplot1D;
    if (cloneplot2D)
        delete cloneplot2D;
    cloneplot1D_hists.clear();    
    if (marker)
        delete marker;
    if (nomarker)
        delete nomarker;  
    if (mainLayout)
        delete mainLayout; 
    if (boxPlot)
        delete boxPlot;        
    if (lblHistTitle)
        delete lblHistTitle;
}

SlsQt1DPlot* qCloneWidget::Get1dPlot() {
	return cloneplot1D;
}

void qCloneWidget::SetupWidgetWindow(QString title, QString xTitle, QString yTitle, QString zTitle, int numDim) {

	QMenuBar* menubar = new QMenuBar(this);
	QAction* actionSave = new QAction("&Save", this);
    menubar->addAction(actionSave);
    setMenuBar(menubar);

    //Main Window Layout
    QWidget *centralWidget = new QWidget(this);
    mainLayout = new QGridLayout(centralWidget);
    centralWidget->setLayout(mainLayout);

    //plot group box
    boxPlot = new QGroupBox(this);
    QGridLayout* plotLayout = new QGridLayout(boxPlot);
    boxPlot->setLayout(plotLayout);
    boxPlot->setAlignment(Qt::AlignHCenter);
    boxPlot->setFont(QFont("Sans Serif", 11, QFont::Normal));
    boxPlot->setTitle(title);
    boxPlot->setContentsMargins(0, 0, 0, 0);
    
    // According to dimensions, create appropriate 1D or 2Dplot
    if (numDim == 1) {
        cloneplot1D = new SlsQt1DPlot(boxPlot);

        cloneplot1D->setFont(QFont("Sans Serif", 9, QFont::Normal));
        cloneplot1D->SetXTitle(xTitle.toAscii().constData());
        cloneplot1D->SetYTitle(yTitle.toAscii().constData());

        boxPlot->setFlat(false);
        boxPlot->setContentsMargins(0, 30, 0, 0);
        plotLayout->addWidget(cloneplot1D, 0, 0);

        lblHistTitle = new QLabel("");
        mainLayout->addWidget(lblHistTitle, 0, 0);

    } else {
        cloneplot2D = new SlsQt2DPlotLayout(boxPlot);
        cloneplot2D->setFont(QFont("Sans Serif", 9, QFont::Normal));
        cloneplot2D->SetXTitle(xTitle);
        cloneplot2D->SetYTitle(yTitle);
        cloneplot2D->SetZTitle(zTitle);
        cloneplot2D->setAlignment(Qt::AlignLeft);

        boxPlot->setFlat(true);
        boxPlot->setContentsMargins(0, 20, 0, 0);
        plotLayout->addWidget(cloneplot2D, 0, 0);
    }

    // main window widgets
    mainLayout->addWidget(boxPlot, 1, 0);
    setCentralWidget(centralWidget);

    // Save
    connect(actionSave, SIGNAL(triggered()), this, SLOT(SavePlot()));

    setMinimumHeight(300);
    resize(500, 350);
}

void qCloneWidget::SetCloneHists(unsigned int nHists, int histNBins, double *histXAxis, std::vector<double*> histYAxis, QString histTitle, bool lines, bool markers) {
    //for each plot,  create hists
    for (unsigned int hist_num = 0; hist_num < nHists; ++hist_num) {
        SlsQtH1D *h = new SlsQtH1D("1d plot", histNBins, histXAxis, histYAxis[hist_num]);
        h->SetLineColor(0);
        h->setStyle(lines ? QwtPlotCurve::Lines : QwtPlotCurve::Dots);
#if QWT_VERSION < 0x060000
        h->setSymbol(markers ? *marker : *nomarker);
#else
        h->setSymbol(markers ? marker : nomarker);
#endif
        cloneplot1D_hists.append(h);
        h->Attach(cloneplot1D);

        lblHistTitle->setText(histTitle);
    }
}

void qCloneWidget::SetCloneHists2D(int nbinsx, double xmin, double xmax, int nbinsy, double ymin, double ymax, double *d, QwtText frameIndexTitle) {
    cloneplot2D->GetPlot()->SetData(nbinsx, xmin, xmax, nbinsy, ymin, ymax, d);
    cloneplot2D->KeepZRangeIfSet();
    cloneplot2D->setTitle(frameIndexTitle);
}

void qCloneWidget::SetRange(bool IsXYRange[], double XYRange[]) {
    double XYRange[4] {0, 0, 0, 0};
    void* plot = cloneplot1D;
    if (cloneplot2D) {
        plot = cloneplot2D->GetPlot();
    }

    plot->SetXMinMax(XYRange[qDefs::XMIN], XYRange[qDefs::XMAX]);
    plot->SetYMinMax(XYRange[qDefs::YMIN], XYRange[qDefs::YMAX]);
    plot->Update();
}

void qCloneWidget::SavePlot() {
    char cID[10];
    sprintf(cID, "%d", id);
    //title
    QString fName = filePath + Qstring('/') + fileName + Qstring('_') + imageIndex +  Qstring('_') + QString(NowTime().c_str()) + QString(".png");
    FILE_LOG(logDEBUG) << "fname:" << fName.toAscii().constData();
    //save
    QImage img(boxPlot->size().width(), boxPlot->size().height(), QImage::Format_RGB32);
    QPainter painter(&img);
    boxPlot->render(&painter);

    fName = QFileDialog::getSaveFileName(this, tr("Save Snapshot "), fName, tr("PNG Files (*.png);;XPM Files(*.xpm);;JPEG Files(*.jpg)"), 0, QFileDialog::ShowDirsOnly);
    if (!fName.isEmpty()) {
        if ((img.save(fName))) {
            qDefs::Message(qDefs::INFORMATION, "The SnapShot has been successfully saved", "qCloneWidget::SavePlot");
            FILE_LOG(logINFO) << "The SnapShot has been successfully saved";
        } else {
            qDefs::Message(qDefs::WARNING, "Attempt to save snapshot failed.\n Formats: .png, .jpg, .xpm.", "qCloneWidget::SavePlot");
            FILE_LOG(logWARNING) << "Attempt to save snapshot failed";
        }
    }
}

int qCloneWidget::SavePlotAutomatic() {
    char cID[10];
    sprintf(cID, "%d", id);
    //title
    QString fName = filePath + Qstring('/') + fileName + Qstring('_') + imageIndex +  Qstring('_') + QString(NowTime().c_str()) + QString(".png");
    FILE_LOG(logDEBUG) << "fname:" << fName.toAscii().constData();
    //save
    QImage img(boxPlot->size().width(), boxPlot->size().height(), QImage::Format_RGB32);
    QPainter painter(&img);
    boxPlot->render(&painter);
    if (img.save(fName))
        return 0;
    else
        return -1;
}

void qCloneWidget::closeEvent(QCloseEvent *event) {
    emit CloneClosedSignal(id);
    event->accept();
}

char *qCloneWidget::GetCurrentTimeStamp() {
    char output[30];
    char *result;

    //using sys cmds to get output or str
    FILE *sysFile = popen("date", "r");
    fgets(output, sizeof(output), sysFile);
    pclose(sysFile);

    result = output + 0;
    return result;
}

void qCloneWidget::DisplayStats(bool enable, QString min, QString max, QString sum) {
    if (enable) {
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
        hl1->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed));
        hl1->addWidget(lblMin);
        hl1->addWidget(lblMinDisp);
        hl1->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed));
        hl1->addWidget(lblMax);
        hl1->addWidget(lblMaxDisp);
        hl1->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed));
        hl1->addWidget(lblSum);
        hl1->addWidget(lblSumDisp);
        hl1->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed));
        widgetStatistics->setLayout(hl1);
        mainLayout->addWidget(widgetStatistics, 2, 0);
        widgetStatistics->show();
    }
}

