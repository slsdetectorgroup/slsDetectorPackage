// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "ui_form_cloneplot.h"
#include <QMainWindow>
#include <QString>

namespace sls {

class SlsQtH1D;
class SlsQt1DPlot;
class SlsQt2DPlot;

class qCloneWidget : public QMainWindow, private Ui::ClonePlotObject {
    Q_OBJECT

  public:
    qCloneWidget(QWidget *parent, SlsQt1DPlot *p1, SlsQt2DPlot *p2,
                 SlsQt1DPlot *gp1, SlsQt2DPlot *gp, QString title,
                 QString filePath, QString fileName, int64_t aIndex,
                 bool displayStats, QString min, QString max, QString sum,
                 bool completeImage);

    ~qCloneWidget();

  private slots:
    void SavePlot();

  protected:
    void resizeEvent(QResizeEvent *event);

  private:
    void SetupWidgetWindow(QString title);
    void DisplayStats(bool enable, QString min, QString max, QString sum);

  private:
    int id;
    SlsQt1DPlot *plot1d{nullptr};
    SlsQt2DPlot *plot2d{nullptr};
    SlsQt1DPlot *gainplot1d{nullptr};
    SlsQt2DPlot *gainplot2d{nullptr};
    QString filePath{"/"};
    QString fileName{"run"};
    int64_t acqIndex{0};

    static int NumClones;
};

} // namespace sls
