/********************************************************************************
** Form generated from reading UI file 'form_detectormain.ui'
**
** Created: Thu Jul 13 14:40:29 2017
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORM_DETECTORMAIN_H
#define UI_FORM_DETECTORMAIN_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDockWidget>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DetectorMainObject
{
public:
    QAction *actionOpenSetup;
    QAction *actionSaveSetup;
    QAction *actionMeasurementWizard;
    QAction *actionOpenConfiguration;
    QAction *actionSaveConfiguration;
    QAction *actionEnergyCalibration;
    QAction *actionAngularCalibration;
    QAction *actionDebug;
    QAction *actionBeamline;
    QAction *actionExpert;
    QAction *actionConfiguration;
    QAction *actionVersion;
    QAction *actionAbout;
    QAction *actionDockable;
    QAction *actionLoadTrimbits;
    QAction *actionSaveTrimbits;
    QAction *actionLoadCalibration;
    QAction *actionSaveCalibration;
    QAction *actionListenGuiClient;
    QWidget *centralwidget;
    QMenuBar *menubar;
    QMenu *menuUtilities;
    QMenu *menuModes;
    QMenu *menuHelp;
    QDockWidget *dockWidgetPlot;
    QWidget *dockWidgetContentsPlot;

    void setupUi(QMainWindow *DetectorMainObject)
    {
        if (DetectorMainObject->objectName().isEmpty())
            DetectorMainObject->setObjectName(QString::fromUtf8("DetectorMainObject"));
        DetectorMainObject->setEnabled(true);
        DetectorMainObject->resize(800, 848);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(DetectorMainObject->sizePolicy().hasHeightForWidth());
        DetectorMainObject->setSizePolicy(sizePolicy);
        QPalette palette;
        QBrush brush(QColor(0, 0, 0, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Active, QPalette::Text, brush);
        palette.setBrush(QPalette::Active, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Active, QPalette::Shadow, brush);
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Shadow, brush);
        QBrush brush1(QColor(185, 185, 185, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Text, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush1);
        QBrush brush2(QColor(89, 89, 89, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::Shadow, brush2);
        DetectorMainObject->setPalette(palette);
        QFont font;
        font.setPointSize(9);
        DetectorMainObject->setFont(font);
        DetectorMainObject->setInputMethodHints(Qt::ImhNone);
        DetectorMainObject->setDocumentMode(false);
        DetectorMainObject->setTabShape(QTabWidget::Rounded);
        DetectorMainObject->setDockNestingEnabled(false);
        actionOpenSetup = new QAction(DetectorMainObject);
        actionOpenSetup->setObjectName(QString::fromUtf8("actionOpenSetup"));
        QFont font1;
        actionOpenSetup->setFont(font1);
        actionSaveSetup = new QAction(DetectorMainObject);
        actionSaveSetup->setObjectName(QString::fromUtf8("actionSaveSetup"));
        actionSaveSetup->setFont(font1);
        actionMeasurementWizard = new QAction(DetectorMainObject);
        actionMeasurementWizard->setObjectName(QString::fromUtf8("actionMeasurementWizard"));
        actionOpenConfiguration = new QAction(DetectorMainObject);
        actionOpenConfiguration->setObjectName(QString::fromUtf8("actionOpenConfiguration"));
        actionSaveConfiguration = new QAction(DetectorMainObject);
        actionSaveConfiguration->setObjectName(QString::fromUtf8("actionSaveConfiguration"));
        actionEnergyCalibration = new QAction(DetectorMainObject);
        actionEnergyCalibration->setObjectName(QString::fromUtf8("actionEnergyCalibration"));
        actionAngularCalibration = new QAction(DetectorMainObject);
        actionAngularCalibration->setObjectName(QString::fromUtf8("actionAngularCalibration"));
        actionDebug = new QAction(DetectorMainObject);
        actionDebug->setObjectName(QString::fromUtf8("actionDebug"));
        actionDebug->setCheckable(true);
        actionDebug->setChecked(false);
        actionBeamline = new QAction(DetectorMainObject);
        actionBeamline->setObjectName(QString::fromUtf8("actionBeamline"));
        actionBeamline->setCheckable(true);
        actionBeamline->setChecked(false);
        actionExpert = new QAction(DetectorMainObject);
        actionExpert->setObjectName(QString::fromUtf8("actionExpert"));
        actionExpert->setCheckable(true);
        actionExpert->setChecked(false);
        actionConfiguration = new QAction(DetectorMainObject);
        actionConfiguration->setObjectName(QString::fromUtf8("actionConfiguration"));
        actionConfiguration->setCheckable(true);
        actionConfiguration->setChecked(false);
        actionVersion = new QAction(DetectorMainObject);
        actionVersion->setObjectName(QString::fromUtf8("actionVersion"));
        actionAbout = new QAction(DetectorMainObject);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        actionDockable = new QAction(DetectorMainObject);
        actionDockable->setObjectName(QString::fromUtf8("actionDockable"));
        actionDockable->setCheckable(true);
        actionLoadTrimbits = new QAction(DetectorMainObject);
        actionLoadTrimbits->setObjectName(QString::fromUtf8("actionLoadTrimbits"));
        actionSaveTrimbits = new QAction(DetectorMainObject);
        actionSaveTrimbits->setObjectName(QString::fromUtf8("actionSaveTrimbits"));
        actionLoadCalibration = new QAction(DetectorMainObject);
        actionLoadCalibration->setObjectName(QString::fromUtf8("actionLoadCalibration"));
        actionSaveCalibration = new QAction(DetectorMainObject);
        actionSaveCalibration->setObjectName(QString::fromUtf8("actionSaveCalibration"));
        actionListenGuiClient = new QAction(DetectorMainObject);
        actionListenGuiClient->setObjectName(QString::fromUtf8("actionListenGuiClient"));
        actionListenGuiClient->setCheckable(true);
        centralwidget = new QWidget(DetectorMainObject);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(centralwidget->sizePolicy().hasHeightForWidth());
        centralwidget->setSizePolicy(sizePolicy1);
        centralwidget->setMinimumSize(QSize(0, 395));
        centralwidget->setMaximumSize(QSize(524287, 395));
        DetectorMainObject->setCentralWidget(centralwidget);
        menubar = new QMenuBar(DetectorMainObject);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 25));
        QSizePolicy sizePolicy2(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(menubar->sizePolicy().hasHeightForWidth());
        menubar->setSizePolicy(sizePolicy2);
        menubar->setMinimumSize(QSize(0, 25));
        menubar->setMaximumSize(QSize(16777215, 25));
        menubar->setFont(font);
        menubar->setFocusPolicy(Qt::StrongFocus);
        menubar->setDefaultUp(false);
        menubar->setNativeMenuBar(true);
        menuUtilities = new QMenu(menubar);
        menuUtilities->setObjectName(QString::fromUtf8("menuUtilities"));
        menuUtilities->setFont(font);
        menuModes = new QMenu(menubar);
        menuModes->setObjectName(QString::fromUtf8("menuModes"));
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        DetectorMainObject->setMenuBar(menubar);
        dockWidgetPlot = new QDockWidget(DetectorMainObject);
        dockWidgetPlot->setObjectName(QString::fromUtf8("dockWidgetPlot"));
        dockWidgetPlot->setEnabled(true);
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(dockWidgetPlot->sizePolicy().hasHeightForWidth());
        dockWidgetPlot->setSizePolicy(sizePolicy3);
        dockWidgetPlot->setMinimumSize(QSize(36, 422));
        dockWidgetPlot->setMaximumSize(QSize(524287, 524287));
        QFont font2;
        font2.setFamily(QString::fromUtf8("Sans Serif"));
        font2.setPointSize(11);
        font2.setBold(false);
        font2.setWeight(50);
        dockWidgetPlot->setFont(font2);
        dockWidgetPlot->setLayoutDirection(Qt::LeftToRight);
        dockWidgetPlot->setFeatures(QDockWidget::NoDockWidgetFeatures);
        dockWidgetPlot->setAllowedAreas(Qt::BottomDockWidgetArea);
        dockWidgetContentsPlot = new QWidget();
        dockWidgetContentsPlot->setObjectName(QString::fromUtf8("dockWidgetContentsPlot"));
        sizePolicy1.setHeightForWidth(dockWidgetContentsPlot->sizePolicy().hasHeightForWidth());
        dockWidgetContentsPlot->setSizePolicy(sizePolicy1);
        dockWidgetContentsPlot->setMinimumSize(QSize(0, 400));
        dockWidgetContentsPlot->setMaximumSize(QSize(16777215, 16777215));
        QPalette palette1;
        palette1.setBrush(QPalette::Active, QPalette::WindowText, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        QBrush brush3(QColor(119, 119, 119, 255));
        brush3.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Disabled, QPalette::WindowText, brush3);
        dockWidgetContentsPlot->setPalette(palette1);
        QFont font3;
        font3.setPointSize(9);
        font3.setBold(false);
        font3.setWeight(50);
        dockWidgetContentsPlot->setFont(font3);
        dockWidgetPlot->setWidget(dockWidgetContentsPlot);
        DetectorMainObject->addDockWidget(static_cast<Qt::DockWidgetArea>(8), dockWidgetPlot);

        menubar->addAction(menuUtilities->menuAction());
        menubar->addAction(menuModes->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menuUtilities->addAction(actionOpenSetup);
        menuUtilities->addAction(actionSaveSetup);
        menuUtilities->addSeparator();
        menuUtilities->addAction(actionOpenConfiguration);
        menuUtilities->addAction(actionSaveConfiguration);
        menuUtilities->addSeparator();
        menuUtilities->addAction(actionLoadTrimbits);
        menuUtilities->addAction(actionSaveTrimbits);
        menuUtilities->addSeparator();
        menuUtilities->addAction(actionLoadCalibration);
        menuUtilities->addAction(actionSaveCalibration);
        menuModes->addAction(actionDebug);
        menuModes->addAction(actionExpert);
        menuModes->addAction(actionDockable);
        menuModes->addAction(actionListenGuiClient);
        menuHelp->addAction(actionAbout);

        retranslateUi(DetectorMainObject);

        QMetaObject::connectSlotsByName(DetectorMainObject);
    } // setupUi

    void retranslateUi(QMainWindow *DetectorMainObject)
    {
        DetectorMainObject->setWindowTitle(QApplication::translate("DetectorMainObject", "SLS Detector GUI", 0, QApplication::UnicodeUTF8));
        actionOpenSetup->setText(QApplication::translate("DetectorMainObject", "&Load &Setup", 0, QApplication::UnicodeUTF8));
        actionSaveSetup->setText(QApplication::translate("DetectorMainObject", "&Save &Setup", 0, QApplication::UnicodeUTF8));
        actionMeasurementWizard->setText(QApplication::translate("DetectorMainObject", "&Measurement Wizard", 0, QApplication::UnicodeUTF8));
        actionOpenConfiguration->setText(QApplication::translate("DetectorMainObject", "&Load &Configuration", 0, QApplication::UnicodeUTF8));
        actionSaveConfiguration->setText(QApplication::translate("DetectorMainObject", "&Save &Configuration", 0, QApplication::UnicodeUTF8));
        actionEnergyCalibration->setText(QApplication::translate("DetectorMainObject", "&Energy Calibration", 0, QApplication::UnicodeUTF8));
        actionAngularCalibration->setText(QApplication::translate("DetectorMainObject", "&Angular Calibration", 0, QApplication::UnicodeUTF8));
        actionDebug->setText(QApplication::translate("DetectorMainObject", "&Debug", 0, QApplication::UnicodeUTF8));
        actionBeamline->setText(QApplication::translate("DetectorMainObject", "&Beamline", 0, QApplication::UnicodeUTF8));
        actionExpert->setText(QApplication::translate("DetectorMainObject", "&Expert", 0, QApplication::UnicodeUTF8));
        actionConfiguration->setText(QApplication::translate("DetectorMainObject", "&Configuration", 0, QApplication::UnicodeUTF8));
        actionVersion->setText(QApplication::translate("DetectorMainObject", "&Version", 0, QApplication::UnicodeUTF8));
        actionAbout->setText(QApplication::translate("DetectorMainObject", "&About", 0, QApplication::UnicodeUTF8));
        actionDockable->setText(QApplication::translate("DetectorMainObject", "D&ockable Windows", 0, QApplication::UnicodeUTF8));
        actionLoadTrimbits->setText(QApplication::translate("DetectorMainObject", "&Load &Trimbits", 0, QApplication::UnicodeUTF8));
        actionSaveTrimbits->setText(QApplication::translate("DetectorMainObject", "&Save &Trimbits", 0, QApplication::UnicodeUTF8));
        actionLoadCalibration->setText(QApplication::translate("DetectorMainObject", "&Load C&alibration", 0, QApplication::UnicodeUTF8));
        actionSaveCalibration->setText(QApplication::translate("DetectorMainObject", "&Save C&alibration", 0, QApplication::UnicodeUTF8));
        actionListenGuiClient->setText(QApplication::translate("DetectorMainObject", "&Listen to Gui Client", 0, QApplication::UnicodeUTF8));
        menuUtilities->setTitle(QApplication::translate("DetectorMainObject", "&Utilities", 0, QApplication::UnicodeUTF8));
        menuModes->setTitle(QApplication::translate("DetectorMainObject", "&Modes", 0, QApplication::UnicodeUTF8));
        menuHelp->setTitle(QApplication::translate("DetectorMainObject", "&Help", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        dockWidgetPlot->setToolTip(QApplication::translate("DetectorMainObject", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<table border=\"0\" style=\"-qt-table-type: root; margin-top:4px; margin-bottom:4px; margin-left:4px; margin-right:4px;\">\n"
"<tr>\n"
"<td style=\"border: none;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" color:#00007f;\">Left Click :zoom in</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" color:#00007f;\">Righ</span><span style=\" color:#00007f;\">t Click </span><span style=\" color:#00007f;\">: zoom out by 1</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-lef"
                        "t:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" color:#00007f;\">Middle Click : panning</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" color:#00007f;\">Ctrl+Right Click : zoom out to full size</span></p></td></tr></table></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        dockWidgetPlot->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
        dockWidgetPlot->setWindowTitle(QApplication::translate("DetectorMainObject", "SLS Detector Plot", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DetectorMainObject: public Ui_DetectorMainObject {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORM_DETECTORMAIN_H
