// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "qCloneWidget.h"
#include "SlsQt1DPlot.h"
#include "SlsQt2DPlot.h"
#include "qDefs.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QImage>
#include <QPainter>
#include <qwt_text.h>

namespace sls {

int qCloneWidget::NumClones{0};

qCloneWidget::qCloneWidget(QWidget *parent, SlsQt1DPlot *p1, SlsQt2DPlot *p2,
                           SlsQt1DPlot *gp1, SlsQt2DPlot *gp, QString title,
                           QString fPath, QString fName, int64_t aIndex,
                           bool displayStats, QString min, QString max,
                           QString sum, bool completeImage)
    : QMainWindow(parent), plot1d(p1), plot2d(p2), gainplot1d(gp1),
      gainplot2d(gp), filePath(fPath), fileName(fName), acqIndex(aIndex) {
    setupUi(this);
    id = qCloneWidget::NumClones++;
    SetupWidgetWindow(title);
    DisplayStats(displayStats, min, max, sum);
    lblCompleteImage->hide();
    lblInCompleteImage->hide();
    if (completeImage) {
        lblCompleteImage->show();
    } else {
        lblInCompleteImage->show();
    }
}

qCloneWidget::~qCloneWidget() {

    delete plot1d;
    delete plot2d;
    delete gainplot1d;
    delete gainplot2d;
}

void qCloneWidget::SetupWidgetWindow(QString title) {

    std::string winTitle = std::string("Snapshot:") + std::to_string(id) +
                           std::string("  -  ") + Logger::Timestamp();
    setWindowTitle(QString(winTitle.c_str()));

    boxPlot->setTitle(title);

    // 1d
    if (plot1d != nullptr) {
        if (gainplot1d == nullptr) {
            plotLayout->addWidget(plot1d);
        } else {
            int ratio = qDefs::DATA_GAIN_PLOT_RATIO - 1;
            plotLayout->addWidget(plot1d, 0, 0, ratio, ratio);
            plotLayout->addWidget(gainplot1d, ratio, 0, 1, ratio, Qt::AlignTop);
        }
    }
    // 2d
    else {
        if (gainplot2d == nullptr) {
            plotLayout->addWidget(plot2d);
        } else {
            int ratio = qDefs::DATA_GAIN_PLOT_RATIO - 1;
            plotLayout->addWidget(plot2d, 0, 0, ratio, ratio);
            plotLayout->addWidget(gainplot2d, 0, ratio, 1, 1,
                                  Qt::AlignRight | Qt::AlignTop);
        }
    }
    connect(actionSaveClone, SIGNAL(triggered()), this, SLOT(SavePlot()));
    this->show();
    if (gainplot1d != nullptr) {
        gainplot1d->setMinimumHeight(qDefs::MIN_HEIGHT_GAIN_PLOT_1D);
        gainplot1d->setFixedWidth(plot1d->width());
        // gainplot1d->setFixedHeight(plot1d->height() /
        // qDefs::DATA_GAIN_PLOT_RATIO - 1);
    }
    if (gainplot2d != nullptr) {
        gainplot2d->setFixedWidth(plot2d->width() /
                                  qDefs::DATA_GAIN_PLOT_RATIO);
        gainplot2d->setFixedHeight(plot2d->height() /
                                   qDefs::DATA_GAIN_PLOT_RATIO);
    }
}

void qCloneWidget::DisplayStats(bool enable, QString min, QString max,
                                QString sum) {
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
    // title
    QString fName = filePath + QString('/') + fileName + QString("_clone") +
                    QString("%1").arg(id) + QString("_acq") +
                    QString("%1").arg(acqIndex) + QString(".png");
    LOG(logINFO) << "Saving Clone:" << fName.toLatin1().constData();
    // save
    QImage img(centralwidget->size().width(), centralwidget->size().height(),
               QImage::Format_RGB32);
    QPainter painter(&img);
    centralwidget->render(&painter);

    fName = QFileDialog::getSaveFileName(
        this, tr("Save Snapshot "), fName,
        tr("PNG Files (*.png);;XPM Files(*.xpm);;JPEG Files(*.jpg)"), nullptr,
        QFileDialog::ShowDirsOnly);
    if (!fName.isEmpty()) {
        if ((img.save(fName))) {
            qDefs::Message(qDefs::INFORMATION,
                           "The SnapShot has been successfully saved",
                           "qCloneWidget::SavePlot");
            LOG(logINFO) << "The SnapShot has been successfully saved";
        } else {
            qDefs::Message(
                qDefs::WARNING,
                "Attempt to save snapshot failed.\n Formats: .png, .jpg, .xpm.",
                "qCloneWidget::SavePlot");
            LOG(logWARNING) << "Attempt to save snapshot failed";
        }
    }
}

void qCloneWidget::resizeEvent(QResizeEvent *event) {
    if (gainplot1d != nullptr) {
        gainplot1d->setFixedWidth(plot1d->width());
        gainplot1d->setFixedHeight(plot1d->height() /
                                   qDefs::DATA_GAIN_PLOT_RATIO);
    }
    if (gainplot2d != nullptr) {
        gainplot2d->setFixedWidth(plot2d->width() /
                                  qDefs::DATA_GAIN_PLOT_RATIO);
        gainplot2d->setFixedHeight(plot2d->height() /
                                   qDefs::DATA_GAIN_PLOT_RATIO);
    }
    event->accept();
}

} // namespace sls
