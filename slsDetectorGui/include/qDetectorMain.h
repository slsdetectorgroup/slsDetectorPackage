// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "qDefs.h"
#include "sls/Detector.h"
#include "ui_form_detectormain.h"
class QResizeEvent;

namespace sls {

class qDrawPlot;
class qTabMeasurement;
class qTabDataOutput;
class qTabPlot;
class qTabAdvanced;
class qTabSettings;
class qTabDebugging;
class qTabDeveloper;
class qTabMessages;

class qDetectorMain : public QMainWindow, private Ui::DetectorMainObject {
    Q_OBJECT

  public:
    qDetectorMain(int multiId, const std::string &fname, bool isDevel);
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
    void SetUpDetector(const std::string &config_file, int multiID);
    void Initialization();
    void LoadConfigFile(const std::string &config_file);

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
    std::unique_ptr<Detector> det;
    qDrawPlot *plot;
    qTabMeasurement *tabMeasurement;
    qTabDataOutput *tabDataOutput;
    qTabPlot *tabPlot;
    qTabSettings *tabSettings;
    qTabAdvanced *tabAdvanced;
    qTabDebugging *tabDebugging;
    qTabDeveloper *tabDeveloper;
    qTabMessages *tabMessages;
    int isDeveloper{false};
    QString zoomToolTip;
    QColor defaultTabColor;
};

} // namespace sls
