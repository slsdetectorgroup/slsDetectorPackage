#pragma once
#include "Detector.h"
#include "ui_form_tab_dataoutput.h"

class qTabDataOutput : public QWidget, private Ui::TabDataOutputObject {
    Q_OBJECT

  public:
    qTabDataOutput(QWidget *parent, sls::Detector *detector);
    ~qTabDataOutput();
    void Refresh();

  private slots:
    void GetOutputDir();
    void BrowseOutputDir();
    void SetOutputDir();
    void SetFileFormat(int format);
    void SetOverwriteEnable(bool enable);
    void SetTenGigaEnable(bool enable);
    void EnableRateCorrection();
    void SetRateCorrection();
    void SetSpeed(int speed);
    void SetFlags();

  private:
    void SetupWidgetWindow();
    void Initialization();
    void PopulateDetectors();
    void EnableBrowse();
    void GetFileWrite();
    void GetFileName();
    void GetFileFormat();
    void GetFileOverwrite();
    void GetTenGigaEnable();
    void GetRateCorrection();
    void GetSpeed();
    void GetFlags();

    sls::Detector *det;
    // Button group for radiobuttons for rate
    QButtonGroup *btnGroupRate;
    // enum for the Eiger Parallel flag
    enum { PARALLEL, NONPARALLEL };
};
