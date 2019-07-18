#include "qCloneWidget.h"
#include "qDefs.h"
#include "SlsQt1DPlot.h"
#include "SlsQt2DPlot.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QImage>
#include <QPainter>
#include <qwt_text.h>

int qCloneWidget::NumClones{0};

qCloneWidget::qCloneWidget(QWidget *parent, SlsQt1DPlot* p1, SlsQt2DPlot* p2, SlsQt2DPlot* gp, 
                QString title, QString fPath, QString fName, int64_t aIndex, 
                 bool displayStats, QString min, QString max, QString sum):
                 QMainWindow(parent), plot1d(p1), plot2d(p2), gainplot2d(gp), filePath(fPath), fileName(fName), acqIndex(aIndex) {
    setupUi(this);  
    id = qCloneWidget::NumClones++;   
    SetupWidgetWindow(title);
    DisplayStats(displayStats, min, max, sum);
}

qCloneWidget::~qCloneWidget() {
    if (plot1d)
        delete plot1d;
    if (plot2d)
        delete plot2d;
    if (gainplot2d)
        delete gainplot2d;      
}

void qCloneWidget::SetupWidgetWindow(QString title) {

    std::string winTitle = std::string("Snapshot:") + std::to_string(id) + std::string("  -  ") + NowTime();
    setWindowTitle(QString(winTitle.c_str()));
    
    boxPlot->setFont(QFont("Sans Serif", qDefs::Q_FONT_SIZE, QFont::Normal));
    boxPlot->setTitle(title);
    
    // 1d
    if (plot1d != nullptr) {
        plotLayout->addWidget(plot1d);
    } 
    // 2d
    else {
        if (gainplot2d == nullptr) {
            plotLayout->addWidget(plot2d);
        } else {
            gainplot2d->setFixedWidth(plot2d->width() / qDefs::DATA_GAIN_PLOT_RATIO);
            gainplot2d->setFixedHeight(plot2d->height() / qDefs::DATA_GAIN_PLOT_RATIO);
            int ratio = qDefs::DATA_GAIN_PLOT_RATIO - 1;
            plotLayout->addWidget(plot2d, 0, 0, ratio, ratio);
            plotLayout->addWidget(gainplot2d, 0, ratio, 1, 1, Qt::AlignRight | Qt::AlignTop);
        }
    }
    connect(actionSaveClone, SIGNAL(triggered()), this, SLOT(SavePlot()));
}

void qCloneWidget::DisplayStats(bool enable, QString min, QString max, QString sum) {
    if (enable) {
        lblMinDisp->setText(QString("%1").arg(min));
        lblMaxDisp->setText(QString("%1").arg(max));
        lblSumDisp->setText(QString("%1").arg(sum));
        widgetStatistics->show();
    } else {
        widgetStatistics->hide();
    }
}

void qCloneWidget::SavePlot() {
    char cID[10];
    sprintf(cID, "%d", id);
    //title
    QString fName = filePath + QString('/') + fileName + QString("_clone") + QString("%1").arg(id) + QString("_acq") + QString("%1").arg(acqIndex) + QString(".png");
    FILE_LOG(logINFO) << "Saving Clone:" << fName.toAscii().constData();
    //save
    QImage img(centralwidget->size().width(), centralwidget->size().height(), QImage::Format_RGB32);
    QPainter painter(&img);
    centralwidget->render(&painter);

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

void qCloneWidget::resizeEvent(QResizeEvent *event) {
    if (gainplot2d != nullptr) {
        gainplot2d->setFixedWidth(plot2d->width() / qDefs::DATA_GAIN_PLOT_RATIO);
        gainplot2d->setFixedHeight(plot2d->height() / qDefs::DATA_GAIN_PLOT_RATIO);
    }
    event->accept();
}