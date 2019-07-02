#pragma once

class SlsQtH1D;
class SlsQt1DPlot;
class SlsQt2DPlotLayout;

#include <QString>
#include <QMainWindow>
class QwtSymbol;
class QGridLayout;
class QGroupBox;
class QLabel;
class QCloseEvent;

#include <iostream>
#include <string>

class qCloneWidget : public QMainWindow {
    Q_OBJECT

  public:
    qCloneWidget(QWidget *parent, int id, QString title, QString xTitle, QString yTitle, QString zTitle, int numDim, 
                QString filePath, QString fileName, int imageIndex, 
                 bool displayStats, QString min, QString max, QString sum);
    ~qCloneWidget();

    void SetupWidgetWindow(QString title, QString xTitle, QString yTitle, QString zTitle, int numDim);
    void SetCloneHists(unsigned int nHists, int histNBins, double *histXAxis, std::vector<double*> histYAxis, QString histTitle, bool lines, bool markers);
    void SetCloneHists2D(int nbinsx, double xmin, double xmax, int nbinsy, double ymin, double ymax, double *d, QString frameIndexTitle);
    void SetRange(bool IsXYRange[], double XYRange[]);
    SlsQt1DPlot *Get1dPlot();

  public slots:
    int SavePlotAutomatic();

  private slots:
    void SavePlot();
    
  protected:
    void closeEvent(QCloseEvent *event);

  private:
    void DisplayStats(bool enable, QString min, QString max, QString sum);


  signals:
    void CloneClosedSignal(int);

  private:
    int id;
    QString filePath;
    QString fileName;
    int imageIndex;
    SlsQt1DPlot *cloneplot1D;
    SlsQt2DPlotLayout *cloneplot2D;
    QVector<SlsQtH1D *> cloneplot1D_hists;

    QwtSymbol *marker;
    QwtSymbol *nomarker;
    QGridLayout *mainLayout;
    QGroupBox *boxPlot;
    QLabel *lblHistTitle;
};

