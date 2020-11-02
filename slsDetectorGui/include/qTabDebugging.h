#pragma once
#include "sls/Detector.h"
#include "ui_form_tab_debugging.h"

class QTreeWidget;
class QTreeWidgetItem;

class qTabDebugging : public QWidget, private Ui::TabDebuggingObject {
    Q_OBJECT

  public:
    qTabDebugging(QWidget *parent, sls::Detector *detector);
    ~qTabDebugging();
    void Refresh();

  private slots:
    void GetDetectorStatus();
    void GetInfo();
    void SetParameters(QTreeWidgetItem *item);
    void TestDetector();

  private:
    void SetupWidgetWindow();
    void Initialization();
    void PopulateDetectors();

    sls::Detector *det;
    /** Tree Widget displaying the detectors, modules */
    QTreeWidget *treeDet;
    QLabel *lblDetectorHostname;
    QLabel *lblDetectorFirmware;
    QLabel *lblDetectorSoftware;
};
