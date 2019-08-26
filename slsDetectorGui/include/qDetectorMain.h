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

#include "Detector.h"

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
    void EnableTabs(bool enable);
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
    std::unique_ptr<sls::Detector> det;
    std::unique_ptr<qDrawPlot> plot;
    std::unique_ptr<MyTabWidget> tabs;
    std::unique_ptr<qTabMeasurement> tabMeasurement;
    std::unique_ptr<qTabDataOutput> tabDataOutput;
    std::unique_ptr<qTabPlot> tabPlot;
    std::unique_ptr<qTabSettings> tabSettings;
    std::unique_ptr<qTabAdvanced> tabAdvanced;
    std::unique_ptr<qTabDebugging> tabDebugging;
    std::unique_ptr<qTabDeveloper> tabDeveloper;
    std::unique_ptr<qTabMessages> tabMessages;
    int isDeveloper;
    int heightPlotWindow;
    int heightCentralWidget;
    QString zoomToolTip;
    QColor defaultTabColor;
};
