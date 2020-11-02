#pragma once
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include "ui_form_dac.h"
#include <string>

class qDacWidget : public QWidget, private Ui::WidgetDacObject {
    Q_OBJECT

  public:
    qDacWidget(QWidget *parent, sls::Detector *detector, bool d, std::string n,
               slsDetectorDefs::dacIndex i);
    ~qDacWidget();
    void SetDetectorIndex(int id);

  private slots:
    void SetDac();

  private:
    void SetupWidgetWindow(std::string name);
    void Initialization();
    void GetDac();
    void GetAdc();
    void Refresh();

    sls::Detector *det;
    bool isDac{true};
    slsDetectorDefs::dacIndex index;
    int detectorIndex{-1};
};
