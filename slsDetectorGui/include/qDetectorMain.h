#pragma once

#include "ui_form_detectormain.h"

#include "qDefs.h"
class qDrawPlot;
class qTabMeasurement;
class qTabDataOutput;
class qTabPlot;
class qTabAdvanced;
class qTabSettings;
class qTabDebugging;
class qTabDeveloper;
class qTabMessages;

class multiSlsDetector;

#include <QTabWidget>
class QResizeEvent;

/** To Over-ride the QTabWidget class to get the tabBar */
class MyTabWidget : public QTabWidget {
  public:
    MyTabWidget(QWidget *parent = 0) { setParent(parent); }
    /** Overridden method from QTabWidget */
    QTabBar *tabBar() { return QTabWidget::tabBar(); }
};

class qDetectorMain : public QMainWindow, private Ui::DetectorMainObject {
    Q_OBJECT

  public:
    qDetectorMain(int argc, char **argv, QApplication *app,
                  QWidget *parent = 0);
    ~qDetectorMain();

  private slots:
    void EnableModes(QAction *action);
    void ExecuteUtilities(QAction *action);
    void ExecuteHelp(QAction *action);
    void Refresh(int index);

    /**
     * Resizes the main window if the plot is docked/undocked
     * @param b bool TRUE if undocked(outside main window), FALSE docked
     */
    void ResizeMainWindow(bool b);
    void EnableTabs();
    void SetZoomToolTip(bool disable);

  protected:
    void resizeEvent(QResizeEvent *event);

  private:
    void SetUpWidgetWindow();
    void SetUpDetector(const std::string fName, int multiID);
    void Initialization();
    void LoadConfigFile(const std::string fName);

    /** enumeration of the tabs */
    enum {
        MEASUREMENT,
        SETTINGS,
        DATAOUTPUT,
        PLOT,
        ADVANCED,
        DEBUGGING,
        DEVELOPER,
        MESSAGES,
        NumberOfTabs
    };
    slsDetectorDefs::detectorType detType;
    multiSlsDetector* myDet;
    qDrawPlot* myPlot;
    MyTabWidget* tabs;
    qTabMeasurement* tabMeasurement;
    qTabDataOutput* tabDataOutput;
    qTabPlot* tabPlot;
    qTabSettings* tabSettings;
    qTabAdvanced* tabAdvanced;
    qTabDebugging* tabDebugging;
    qTabDeveloper* tabDeveloper;
    qTabMessages* tabMessages;
    int isDeveloper;
    int heightPlotWindow;
    int heightCentralWidget;
    QString zoomToolTip;
    QColor defaultTabColor;
};
