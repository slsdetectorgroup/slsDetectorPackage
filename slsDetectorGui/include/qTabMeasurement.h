// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "sls/Detector.h"
#include "ui_form_tab_measurement.h"

#include <mutex>

class QStandardItemModel;

namespace sls {

class qDrawPlot;

class qTabMeasurement : public QWidget, private Ui::TabMeasurementObject {
    Q_OBJECT

  public:
    qTabMeasurement(QWidget *parent, Detector *detector, qDrawPlot *p);
    ~qTabMeasurement();

    void Refresh();

  public slots:
    void AcquireFinished();
    void AbortAcquire(QString exmsg);

  private slots:
    void SetTimingMode(int val);
    void SetBurstMode(int val);
    void SetNumMeasurements(int val);
    void SetNumFrames(int val);
    void SetNumTriggers(int val);
    void SetNumBursts(int val);
    void SetNumGates(int val);
    void SetExposureTime();
    void SetAcquisitionPeriod();
    void SetDelay();
    void SetBurstPeriod();
    void SetFileWrite(bool val);
    void SetFileName(bool force = false);
    void ForceSetFileName();
    void SetRunIndex(int val);
    void SetNextFrameNumber(int val);
    void UpdateProgress();
    void StartAcquisition();
    void StopAcquisition();

  private:
    void SetupWidgetWindow();
    void Initialization();
    /** default, show trigger and delay,
     * otherwise for gotthard2 in auto timing mode and burst mode,
     * show bursts and burst period
     */
    void ShowTriggerDelay();
    void ShowGates();
    void SetupTimingMode();
    void EnableWidgetsforTimingMode();

    void GetTimingMode();
    void GetBurstMode();
    void GetNumFrames();
    void GetNumTriggers();
    void GetNumBursts();
    void GetNumGates();
    void GetExposureTime();
    void GetAcquisitionPeriod();
    void CheckAcqPeriodGreaterThanExp();
    void GetDelay();
    void GetBurstPeriod();
    void GetFileWrite();
    void GetFileName();
    void GetRunIndex();
    void GetNextFrameNumber();

    void ResetProgress();

    void Enable(bool enable);
    int VerifyOutputDirectoryError();

  signals:
    void EnableTabsSignal(bool);
    void FileNameChangedSignal(QString);

  private:
    Detector *det;
    qDrawPlot *plot;
    // enum for the timing mode
    enum { AUTO, TRIGGER, GATED, BURST_TRIGGER, TRIGGER_GATED, NUMTIMINGMODES };
    QTimer *progressTimer;
    // tool tip variables
    QString acqPeriodTip;
    QString errPeriodTip;
    QPalette red;
    bool delayImplemented;
    bool gateImplemented;
    bool startingFnumImplemented;
    bool isAcquisitionStopped{false};
    int numMeasurements{1};
    int currentMeasurement{0};
    mutable std::mutex mProgress;
};

} // namespace sls
