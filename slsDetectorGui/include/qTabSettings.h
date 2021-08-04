#pragma once
#include "sls/Detector.h"
#include "ui_form_tab_settings.h"
#include <QCheckBox>

class qTabSettings : public QWidget, private Ui::TabSettingsObject {
    Q_OBJECT

  public:
    qTabSettings(QWidget *parent, sls::Detector *detector);
    ~qTabSettings();
    void Refresh();

  private slots:
    void SetSettings(int index);
    void SetDynamicRange(int index);
    void SetThresholdEnergy(int index);
    void SetThresholdEnergies();
    void SetCounterMask();

  private:
    void SetupWidgetWindow();
    void SetupDetectorSettings();
    void Initialization();

    void GetSettings();
    void GetDynamicRange();
    void GetThresholdEnergy();
    void GetThresholdEnergies();
    void GetCounterMask();

    sls::Detector *det;
    std::vector<QCheckBox *> counters;

    enum { DYNAMICRANGE_32, DYNAMICRANGE_16, DYNAMICRANGE_8, DYNAMICRANGE_4 };
};
