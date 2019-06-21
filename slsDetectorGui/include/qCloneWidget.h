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
    qCloneWidget(QWidget *parent, int id, QString title, QString xTitle, QString yTitle, QString zTitle, int numDim, std::string FilePath,
                 bool displayStats, QString min, QString max, QString sum);
    ~qCloneWidget();

    void SetupWidgetWindow(QString title, QString xTitle, QString yTitle, QString zTitle, int numDim);

    /**
     * Get the 1D hist values to plot
	   * @param nHists Number of graphs in 1D
	   * @param histNBins Total Number of X axis values/channels in 1D
	   * @param histXAxis X Axis value in 1D
	   * @param histYAxis Y Axis value in 1D
	   * @param histTitle Title for all the graphs in 1D
	   * @param lines style of plot if lines or dots
	   * @param markers style of plot markers or not
	   */
    void SetCloneHists(int nHists, int histNBins, double *histXAxis, std::vector<double*> histYAxis, std::vector<std::string> histTitle, bool lines, bool markers);

     /**
     * Get the 1D hist values to plot for angle plotting
	   * @param nbinsx number of bins in x axis
	   * @param xmin minimum in x axis
	   * @param xmax maximum in x axis
	   * @param nbinsy number of bins in y axis
	   * @param ymin minimum in y axis
	   * @param ymax maximum in y axis
	   * @param d data
	   */
    void SetCloneHists2D(int nbinsx, double xmin, double xmax, int nbinsy, double ymin, double ymax, double *d);

    /**
     * Set the range of the 1d plot
	   * @param IsXYRange array of x,y,min,max if these values are set
	   * @param XYRangeValues array of set values of x,y, min, max
	   */
    void SetRange(bool IsXYRange[], double XYRangeValues[]);

    SlsQt1DPlot *Get1dPlot();

  public slots:
    int SavePlotAutomatic();

  private slots:
    void SavePlot();
    
  protected:
    void closeEvent(QCloseEvent *event);

  private:
     char *GetCurrentTimeStamp();
    void DisplayStats(bool enable, QString min, QString max, QString sum);

  signals:
    void CloneClosedSignal(int);

  private:
    int id;
    std::string filePath;
    SlsQt1DPlot *cloneplot1D;
    SlsQt2DPlotLayout *cloneplot2D;
    QVector<SlsQtH1D *> cloneplot1D_hists;

    QwtSymbol *marker;
    QwtSymbol *nomarker;
    QGridLayout *mainLayout;
    QGroupBox *cloneBox;
    QLabel *lblHistTitle;
};

