/********************************************************************************
** Form generated from reading UI file 'form_tab_advanced.ui'
**
** Created: Tue Jul 25 12:31:25 2017
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORM_TAB_ADVANCED_H
#define UI_FORM_TAB_ADVANCED_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QScrollArea>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QTabWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TabAdvancedObject
{
public:
    QTabWidget *tabAdvancedSettings;
    QWidget *tab_4;
    QGroupBox *boxLogs;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *chkEnergyLog;
    QCheckBox *chkAngularLog;
    QWidget *tab_3;
    QGroupBox *boxPlot;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *btnRefresh;
    QPushButton *btnGetTrimbits;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QRadioButton *radioDataGraph;
    QRadioButton *radioHistogram;
    QGroupBox *boxTrimming;
    QWidget *gridLayoutWidget_3;
    QGridLayout *gridLayout_3;
    QLabel *label_5;
    QSpacerItem *horizontalSpacer_2;
    QLabel *lblExpTime;
    QDoubleSpinBox *spinExpTime;
    QComboBox *comboExpUnit;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *horizontalSpacer_4;
    QLabel *lblFile;
    QLineEdit *dispFile;
    QPushButton *btnFile;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *btnStart;
    QDoubleSpinBox *spinThreshold;
    QLabel *label_15;
    QCheckBox *chkOptimize;
    QLabel *lblResolution;
    QSpinBox *spinResolution;
    QComboBox *comboMethod;
    QLabel *lblCounts;
    QSpinBox *spinCounts;
    QGroupBox *boxSetAllTrimbits;
    QLabel *label;
    QSpinBox *spinSetAllTrimbits;
    QWidget *tab_5;
    QScrollArea *scrollArea;
    QWidget *roiWidget;
    QGridLayout *gridRoi;
    QWidget *horizontalLayoutWidget_4;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *btnGetRoi;
    QSpacerItem *horizontalSpacer_6;
    QPushButton *btnSetRoi;
    QSpacerItem *horizontalSpacer_5;
    QPushButton *btnClearRoi;
    QWidget *tab;
    QGroupBox *boxRxr;
    QPushButton *btnRxr;
    QWidget *gridLayoutWidget_9;
    QGridLayout *gridLayout_8;
    QLabel *label_8;
    QLineEdit *dispUDPIP;
    QSpacerItem *horizontalSpacer_7;
    QSpinBox *spinUDPPort;
    QLabel *label_14;
    QLabel *label_13;
    QComboBox *comboRxrOnline;
    QLineEdit *dispUDPMAC;
    QSpinBox *spinTCPPort;
    QLabel *lblRxrOnline;
    QLabel *label_10;
    QLineEdit *dispRxrHostname;
    QLabel *label_18;
    QWidget *gridLayoutWidget_7;
    QGridLayout *gridLayout_6;
    QComboBox *comboOnline;
    QLabel *label_9;
    QLabel *label_7;
    QLineEdit *dispIP;
    QLineEdit *dispMAC;
    QLabel *lblMAC;
    QLabel *lblOnline;
    QLabel *lblHostname;
    QLabel *lblIP;
    QComboBox *comboDetector;
    QSpacerItem *horizontalSpacer;
    QSpinBox *spinControlPort;
    QSpinBox *spinStopPort;

    void setupUi(QWidget *TabAdvancedObject)
    {
        if (TabAdvancedObject->objectName().isEmpty())
            TabAdvancedObject->setObjectName(QString::fromUtf8("TabAdvancedObject"));
        TabAdvancedObject->resize(775, 351);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(TabAdvancedObject->sizePolicy().hasHeightForWidth());
        TabAdvancedObject->setSizePolicy(sizePolicy);
        TabAdvancedObject->setMinimumSize(QSize(0, 0));
        TabAdvancedObject->setMaximumSize(QSize(1000, 1000));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/images/refresh.png"), QSize(), QIcon::Normal, QIcon::Off);
        TabAdvancedObject->setWindowIcon(icon);
        tabAdvancedSettings = new QTabWidget(TabAdvancedObject);
        tabAdvancedSettings->setObjectName(QString::fromUtf8("tabAdvancedSettings"));
        tabAdvancedSettings->setGeometry(QRect(5, 3, 761, 343));
        tabAdvancedSettings->setTabPosition(QTabWidget::North);
        tabAdvancedSettings->setElideMode(Qt::ElideLeft);
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        boxLogs = new QGroupBox(tab_4);
        boxLogs->setObjectName(QString::fromUtf8("boxLogs"));
        boxLogs->setGeometry(QRect(5, 10, 746, 66));
        horizontalLayoutWidget_2 = new QWidget(boxLogs);
        horizontalLayoutWidget_2->setObjectName(QString::fromUtf8("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(25, 20, 320, 31));
        horizontalLayout_2 = new QHBoxLayout(horizontalLayoutWidget_2);
        horizontalLayout_2->setSpacing(42);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        chkEnergyLog = new QCheckBox(horizontalLayoutWidget_2);
        chkEnergyLog->setObjectName(QString::fromUtf8("chkEnergyLog"));
        sizePolicy.setHeightForWidth(chkEnergyLog->sizePolicy().hasHeightForWidth());
        chkEnergyLog->setSizePolicy(sizePolicy);

        horizontalLayout_2->addWidget(chkEnergyLog);

        chkAngularLog = new QCheckBox(horizontalLayoutWidget_2);
        chkAngularLog->setObjectName(QString::fromUtf8("chkAngularLog"));
        sizePolicy.setHeightForWidth(chkAngularLog->sizePolicy().hasHeightForWidth());
        chkAngularLog->setSizePolicy(sizePolicy);

        horizontalLayout_2->addWidget(chkAngularLog);

        tabAdvancedSettings->addTab(tab_4, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        boxPlot = new QGroupBox(tab_3);
        boxPlot->setObjectName(QString::fromUtf8("boxPlot"));
        boxPlot->setGeometry(QRect(5, 10, 467, 66));
        boxPlot->setCheckable(true);
        boxPlot->setChecked(false);
        horizontalLayoutWidget = new QWidget(boxPlot);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(230, 15, 228, 44));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setSpacing(17);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        btnRefresh = new QPushButton(horizontalLayoutWidget);
        btnRefresh->setObjectName(QString::fromUtf8("btnRefresh"));
        btnRefresh->setAutoFillBackground(false);
        btnRefresh->setIcon(icon);
        btnRefresh->setIconSize(QSize(24, 16));

        horizontalLayout->addWidget(btnRefresh);

        btnGetTrimbits = new QPushButton(horizontalLayoutWidget);
        btnGetTrimbits->setObjectName(QString::fromUtf8("btnGetTrimbits"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/images/download.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnGetTrimbits->setIcon(icon1);

        horizontalLayout->addWidget(btnGetTrimbits);

        gridLayoutWidget = new QWidget(boxPlot);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(25, 20, 187, 31));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        radioDataGraph = new QRadioButton(gridLayoutWidget);
        radioDataGraph->setObjectName(QString::fromUtf8("radioDataGraph"));
        radioDataGraph->setChecked(true);

        gridLayout->addWidget(radioDataGraph, 0, 0, 1, 1);

        radioHistogram = new QRadioButton(gridLayoutWidget);
        radioHistogram->setObjectName(QString::fromUtf8("radioHistogram"));

        gridLayout->addWidget(radioHistogram, 0, 1, 1, 1);

        boxTrimming = new QGroupBox(tab_3);
        boxTrimming->setObjectName(QString::fromUtf8("boxTrimming"));
        boxTrimming->setEnabled(true);
        boxTrimming->setGeometry(QRect(5, 85, 746, 226));
        boxTrimming->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        boxTrimming->setFlat(false);
        boxTrimming->setCheckable(true);
        boxTrimming->setChecked(true);
        gridLayoutWidget_3 = new QWidget(boxTrimming);
        gridLayoutWidget_3->setObjectName(QString::fromUtf8("gridLayoutWidget_3"));
        gridLayoutWidget_3->setGeometry(QRect(25, 25, 696, 195));
        gridLayout_3 = new QGridLayout(gridLayoutWidget_3);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout_3->setVerticalSpacing(12);
        gridLayout_3->setContentsMargins(0, 0, 0, 0);
        label_5 = new QLabel(gridLayoutWidget_3);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy1);

        gridLayout_3->addWidget(label_5, 0, 0, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_2, 0, 1, 1, 1);

        lblExpTime = new QLabel(gridLayoutWidget_3);
        lblExpTime->setObjectName(QString::fromUtf8("lblExpTime"));
        sizePolicy1.setHeightForWidth(lblExpTime->sizePolicy().hasHeightForWidth());
        lblExpTime->setSizePolicy(sizePolicy1);

        gridLayout_3->addWidget(lblExpTime, 3, 0, 1, 1);

        spinExpTime = new QDoubleSpinBox(gridLayoutWidget_3);
        spinExpTime->setObjectName(QString::fromUtf8("spinExpTime"));
        spinExpTime->setEnabled(true);
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(spinExpTime->sizePolicy().hasHeightForWidth());
        spinExpTime->setSizePolicy(sizePolicy2);
        spinExpTime->setMinimumSize(QSize(0, 0));
        spinExpTime->setMaximumSize(QSize(16777215, 16777215));
        spinExpTime->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinExpTime->setKeyboardTracking(false);
        spinExpTime->setDecimals(5);
        spinExpTime->setMinimum(0);
        spinExpTime->setMaximum(2e+09);
        spinExpTime->setValue(1);

        gridLayout_3->addWidget(spinExpTime, 3, 2, 1, 1);

        comboExpUnit = new QComboBox(gridLayoutWidget_3);
        comboExpUnit->setObjectName(QString::fromUtf8("comboExpUnit"));
        comboExpUnit->setEnabled(true);
        sizePolicy2.setHeightForWidth(comboExpUnit->sizePolicy().hasHeightForWidth());
        comboExpUnit->setSizePolicy(sizePolicy2);
        comboExpUnit->setMinimumSize(QSize(0, 0));
        comboExpUnit->setMaximumSize(QSize(16777215, 16777215));
        comboExpUnit->setLayoutDirection(Qt::LeftToRight);

        gridLayout_3->addWidget(comboExpUnit, 3, 3, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_3, 3, 6, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(50, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_4, 3, 4, 1, 1);

        lblFile = new QLabel(gridLayoutWidget_3);
        lblFile->setObjectName(QString::fromUtf8("lblFile"));
        sizePolicy.setHeightForWidth(lblFile->sizePolicy().hasHeightForWidth());
        lblFile->setSizePolicy(sizePolicy);

        gridLayout_3->addWidget(lblFile, 4, 0, 1, 1);

        dispFile = new QLineEdit(gridLayoutWidget_3);
        dispFile->setObjectName(QString::fromUtf8("dispFile"));

        gridLayout_3->addWidget(dispFile, 4, 2, 1, 6);

        btnFile = new QPushButton(gridLayoutWidget_3);
        btnFile->setObjectName(QString::fromUtf8("btnFile"));
        sizePolicy.setHeightForWidth(btnFile->sizePolicy().hasHeightForWidth());
        btnFile->setSizePolicy(sizePolicy);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/images/browse.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnFile->setIcon(icon2);

        gridLayout_3->addWidget(btnFile, 4, 8, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(0);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));

        gridLayout_3->addLayout(horizontalLayout_4, 5, 4, 1, 1);

        btnStart = new QPushButton(gridLayoutWidget_3);
        btnStart->setObjectName(QString::fromUtf8("btnStart"));
        sizePolicy.setHeightForWidth(btnStart->sizePolicy().hasHeightForWidth());
        btnStart->setSizePolicy(sizePolicy);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/images/start.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnStart->setIcon(icon3);

        gridLayout_3->addWidget(btnStart, 5, 2, 1, 1);

        spinThreshold = new QDoubleSpinBox(gridLayoutWidget_3);
        spinThreshold->setObjectName(QString::fromUtf8("spinThreshold"));
        spinThreshold->setEnabled(true);
        sizePolicy2.setHeightForWidth(spinThreshold->sizePolicy().hasHeightForWidth());
        spinThreshold->setSizePolicy(sizePolicy2);
        spinThreshold->setMinimumSize(QSize(0, 0));
        spinThreshold->setMaximumSize(QSize(16777215, 16777215));
        spinThreshold->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinThreshold->setKeyboardTracking(false);
        spinThreshold->setDecimals(3);
        spinThreshold->setMinimum(0);
        spinThreshold->setMaximum(10000);
        spinThreshold->setValue(560);

        gridLayout_3->addWidget(spinThreshold, 3, 7, 1, 2);

        label_15 = new QLabel(gridLayoutWidget_3);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        sizePolicy1.setHeightForWidth(label_15->sizePolicy().hasHeightForWidth());
        label_15->setSizePolicy(sizePolicy1);

        gridLayout_3->addWidget(label_15, 3, 5, 1, 1);

        chkOptimize = new QCheckBox(gridLayoutWidget_3);
        chkOptimize->setObjectName(QString::fromUtf8("chkOptimize"));
        chkOptimize->setEnabled(true);
        sizePolicy.setHeightForWidth(chkOptimize->sizePolicy().hasHeightForWidth());
        chkOptimize->setSizePolicy(sizePolicy);
        chkOptimize->setMinimumSize(QSize(0, 0));

        gridLayout_3->addWidget(chkOptimize, 0, 5, 1, 1);

        lblResolution = new QLabel(gridLayoutWidget_3);
        lblResolution->setObjectName(QString::fromUtf8("lblResolution"));
        sizePolicy1.setHeightForWidth(lblResolution->sizePolicy().hasHeightForWidth());
        lblResolution->setSizePolicy(sizePolicy1);

        gridLayout_3->addWidget(lblResolution, 2, 0, 1, 1);

        spinResolution = new QSpinBox(gridLayoutWidget_3);
        spinResolution->setObjectName(QString::fromUtf8("spinResolution"));
        sizePolicy2.setHeightForWidth(spinResolution->sizePolicy().hasHeightForWidth());
        spinResolution->setSizePolicy(sizePolicy2);
        spinResolution->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinResolution->setKeyboardTracking(false);
        spinResolution->setMinimum(1);
        spinResolution->setMaximum(9);
        spinResolution->setValue(4);

        gridLayout_3->addWidget(spinResolution, 2, 2, 1, 2);

        comboMethod = new QComboBox(gridLayoutWidget_3);
        comboMethod->setObjectName(QString::fromUtf8("comboMethod"));
        sizePolicy.setHeightForWidth(comboMethod->sizePolicy().hasHeightForWidth());
        comboMethod->setSizePolicy(sizePolicy);
        comboMethod->setMinimumSize(QSize(0, 0));

        gridLayout_3->addWidget(comboMethod, 0, 2, 1, 2);

        lblCounts = new QLabel(gridLayoutWidget_3);
        lblCounts->setObjectName(QString::fromUtf8("lblCounts"));
        sizePolicy1.setHeightForWidth(lblCounts->sizePolicy().hasHeightForWidth());
        lblCounts->setSizePolicy(sizePolicy1);

        gridLayout_3->addWidget(lblCounts, 2, 5, 1, 1);

        spinCounts = new QSpinBox(gridLayoutWidget_3);
        spinCounts->setObjectName(QString::fromUtf8("spinCounts"));
        sizePolicy2.setHeightForWidth(spinCounts->sizePolicy().hasHeightForWidth());
        spinCounts->setSizePolicy(sizePolicy2);
        spinCounts->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinCounts->setKeyboardTracking(false);
        spinCounts->setMinimum(0);
        spinCounts->setMaximum(16000000);
        spinCounts->setValue(500);

        gridLayout_3->addWidget(spinCounts, 2, 7, 1, 2);

        boxSetAllTrimbits = new QGroupBox(tab_3);
        boxSetAllTrimbits->setObjectName(QString::fromUtf8("boxSetAllTrimbits"));
        boxSetAllTrimbits->setEnabled(false);
        boxSetAllTrimbits->setGeometry(QRect(518, 10, 233, 66));
        boxSetAllTrimbits->setCheckable(false);
        boxSetAllTrimbits->setChecked(false);
        label = new QLabel(boxSetAllTrimbits);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(12, 28, 94, 16));
        spinSetAllTrimbits = new QSpinBox(boxSetAllTrimbits);
        spinSetAllTrimbits->setObjectName(QString::fromUtf8("spinSetAllTrimbits"));
        spinSetAllTrimbits->setGeometry(QRect(121, 24, 86, 25));
        spinSetAllTrimbits->setMaximum(63);
        tabAdvancedSettings->addTab(tab_3, QString());
        tab_5 = new QWidget();
        tab_5->setObjectName(QString::fromUtf8("tab_5"));
        scrollArea = new QScrollArea(tab_5);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setGeometry(QRect(10, 50, 736, 246));
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setWidgetResizable(true);
        roiWidget = new QWidget();
        roiWidget->setObjectName(QString::fromUtf8("roiWidget"));
        roiWidget->setGeometry(QRect(0, 0, 736, 246));
        gridRoi = new QGridLayout(roiWidget);
        gridRoi->setObjectName(QString::fromUtf8("gridRoi"));
        gridRoi->setHorizontalSpacing(1);
        gridRoi->setVerticalSpacing(15);
        scrollArea->setWidget(roiWidget);
        horizontalLayoutWidget_4 = new QWidget(tab_5);
        horizontalLayoutWidget_4->setObjectName(QString::fromUtf8("horizontalLayoutWidget_4"));
        horizontalLayoutWidget_4->setGeometry(QRect(10, 5, 736, 41));
        horizontalLayout_5 = new QHBoxLayout(horizontalLayoutWidget_4);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        horizontalLayout_5->setContentsMargins(0, 0, 0, 0);
        btnGetRoi = new QPushButton(horizontalLayoutWidget_4);
        btnGetRoi->setObjectName(QString::fromUtf8("btnGetRoi"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(btnGetRoi->sizePolicy().hasHeightForWidth());
        btnGetRoi->setSizePolicy(sizePolicy3);
        btnGetRoi->setMinimumSize(QSize(150, 0));
        btnGetRoi->setIcon(icon1);

        horizontalLayout_5->addWidget(btnGetRoi);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_6);

        btnSetRoi = new QPushButton(horizontalLayoutWidget_4);
        btnSetRoi->setObjectName(QString::fromUtf8("btnSetRoi"));
        sizePolicy3.setHeightForWidth(btnSetRoi->sizePolicy().hasHeightForWidth());
        btnSetRoi->setSizePolicy(sizePolicy3);
        btnSetRoi->setMinimumSize(QSize(150, 0));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/images/upload.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnSetRoi->setIcon(icon4);

        horizontalLayout_5->addWidget(btnSetRoi);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_5);

        btnClearRoi = new QPushButton(horizontalLayoutWidget_4);
        btnClearRoi->setObjectName(QString::fromUtf8("btnClearRoi"));
        sizePolicy3.setHeightForWidth(btnClearRoi->sizePolicy().hasHeightForWidth());
        btnClearRoi->setSizePolicy(sizePolicy3);
        btnClearRoi->setMinimumSize(QSize(150, 0));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/images/erase.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnClearRoi->setIcon(icon5);

        horizontalLayout_5->addWidget(btnClearRoi);

        tabAdvancedSettings->addTab(tab_5, QString());
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        boxRxr = new QGroupBox(tab);
        boxRxr->setObjectName(QString::fromUtf8("boxRxr"));
        boxRxr->setGeometry(QRect(10, 135, 736, 171));
        btnRxr = new QPushButton(boxRxr);
        btnRxr->setObjectName(QString::fromUtf8("btnRxr"));
        btnRxr->setGeometry(QRect(25, 135, 291, 25));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/icons/images/setup.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnRxr->setIcon(icon6);
        gridLayoutWidget_9 = new QWidget(boxRxr);
        gridLayoutWidget_9->setObjectName(QString::fromUtf8("gridLayoutWidget_9"));
        gridLayoutWidget_9->setGeometry(QRect(25, 25, 686, 96));
        gridLayout_8 = new QGridLayout(gridLayoutWidget_9);
        gridLayout_8->setObjectName(QString::fromUtf8("gridLayout_8"));
        gridLayout_8->setVerticalSpacing(6);
        gridLayout_8->setContentsMargins(0, 0, 0, 0);
        label_8 = new QLabel(gridLayoutWidget_9);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy4);

        gridLayout_8->addWidget(label_8, 1, 0, 1, 1);

        dispUDPIP = new QLineEdit(gridLayoutWidget_9);
        dispUDPIP->setObjectName(QString::fromUtf8("dispUDPIP"));
        sizePolicy3.setHeightForWidth(dispUDPIP->sizePolicy().hasHeightForWidth());
        dispUDPIP->setSizePolicy(sizePolicy3);
        dispUDPIP->setMinimumSize(QSize(180, 0));

        gridLayout_8->addWidget(dispUDPIP, 1, 4, 1, 1);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_8->addItem(horizontalSpacer_7, 0, 2, 1, 1);

        spinUDPPort = new QSpinBox(gridLayoutWidget_9);
        spinUDPPort->setObjectName(QString::fromUtf8("spinUDPPort"));
        sizePolicy3.setHeightForWidth(spinUDPPort->sizePolicy().hasHeightForWidth());
        spinUDPPort->setSizePolicy(sizePolicy3);
        spinUDPPort->setMinimumSize(QSize(127, 0));
        spinUDPPort->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinUDPPort->setKeyboardTracking(false);
        spinUDPPort->setMinimum(0);
        spinUDPPort->setMaximum(2000000000);
        spinUDPPort->setValue(0);

        gridLayout_8->addWidget(spinUDPPort, 2, 1, 1, 1);

        label_14 = new QLabel(gridLayoutWidget_9);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        sizePolicy4.setHeightForWidth(label_14->sizePolicy().hasHeightForWidth());
        label_14->setSizePolicy(sizePolicy4);

        gridLayout_8->addWidget(label_14, 1, 3, 1, 1);

        label_13 = new QLabel(gridLayoutWidget_9);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        sizePolicy4.setHeightForWidth(label_13->sizePolicy().hasHeightForWidth());
        label_13->setSizePolicy(sizePolicy4);

        gridLayout_8->addWidget(label_13, 0, 0, 1, 1);

        comboRxrOnline = new QComboBox(gridLayoutWidget_9);
        comboRxrOnline->setObjectName(QString::fromUtf8("comboRxrOnline"));
        sizePolicy2.setHeightForWidth(comboRxrOnline->sizePolicy().hasHeightForWidth());
        comboRxrOnline->setSizePolicy(sizePolicy2);

        gridLayout_8->addWidget(comboRxrOnline, 0, 4, 1, 1);

        dispUDPMAC = new QLineEdit(gridLayoutWidget_9);
        dispUDPMAC->setObjectName(QString::fromUtf8("dispUDPMAC"));
        sizePolicy3.setHeightForWidth(dispUDPMAC->sizePolicy().hasHeightForWidth());
        dispUDPMAC->setSizePolicy(sizePolicy3);
        dispUDPMAC->setMinimumSize(QSize(180, 0));

        gridLayout_8->addWidget(dispUDPMAC, 2, 4, 1, 1);

        spinTCPPort = new QSpinBox(gridLayoutWidget_9);
        spinTCPPort->setObjectName(QString::fromUtf8("spinTCPPort"));
        sizePolicy3.setHeightForWidth(spinTCPPort->sizePolicy().hasHeightForWidth());
        spinTCPPort->setSizePolicy(sizePolicy3);
        spinTCPPort->setMinimumSize(QSize(127, 0));
#ifndef QT_NO_TOOLTIP
        spinTCPPort->setToolTip(QString::fromUtf8("Sets Receiver TCP Port\n"
"#rx_tcpport#"));
#endif // QT_NO_TOOLTIP
        spinTCPPort->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinTCPPort->setKeyboardTracking(false);
        spinTCPPort->setMinimum(0);
        spinTCPPort->setMaximum(2000000000);
        spinTCPPort->setValue(0);

        gridLayout_8->addWidget(spinTCPPort, 1, 1, 1, 1);

        lblRxrOnline = new QLabel(gridLayoutWidget_9);
        lblRxrOnline->setObjectName(QString::fromUtf8("lblRxrOnline"));
        sizePolicy4.setHeightForWidth(lblRxrOnline->sizePolicy().hasHeightForWidth());
        lblRxrOnline->setSizePolicy(sizePolicy4);

        gridLayout_8->addWidget(lblRxrOnline, 0, 3, 1, 1);

        label_10 = new QLabel(gridLayoutWidget_9);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        sizePolicy4.setHeightForWidth(label_10->sizePolicy().hasHeightForWidth());
        label_10->setSizePolicy(sizePolicy4);

        gridLayout_8->addWidget(label_10, 2, 0, 1, 1);

        dispRxrHostname = new QLineEdit(gridLayoutWidget_9);
        dispRxrHostname->setObjectName(QString::fromUtf8("dispRxrHostname"));
        sizePolicy3.setHeightForWidth(dispRxrHostname->sizePolicy().hasHeightForWidth());
        dispRxrHostname->setSizePolicy(sizePolicy3);
        dispRxrHostname->setMinimumSize(QSize(180, 0));

        gridLayout_8->addWidget(dispRxrHostname, 0, 1, 1, 1);

        label_18 = new QLabel(gridLayoutWidget_9);
        label_18->setObjectName(QString::fromUtf8("label_18"));
        sizePolicy4.setHeightForWidth(label_18->sizePolicy().hasHeightForWidth());
        label_18->setSizePolicy(sizePolicy4);

        gridLayout_8->addWidget(label_18, 2, 3, 1, 1);

        gridLayoutWidget_7 = new QWidget(tab);
        gridLayoutWidget_7->setObjectName(QString::fromUtf8("gridLayoutWidget_7"));
        gridLayoutWidget_7->setGeometry(QRect(35, 15, 686, 101));
        gridLayout_6 = new QGridLayout(gridLayoutWidget_7);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        gridLayout_6->setVerticalSpacing(6);
        gridLayout_6->setContentsMargins(0, 0, 0, 0);
        comboOnline = new QComboBox(gridLayoutWidget_7);
        comboOnline->setObjectName(QString::fromUtf8("comboOnline"));
        sizePolicy3.setHeightForWidth(comboOnline->sizePolicy().hasHeightForWidth());
        comboOnline->setSizePolicy(sizePolicy3);
        comboOnline->setMinimumSize(QSize(180, 0));

        gridLayout_6->addWidget(comboOnline, 0, 4, 1, 1);

        label_9 = new QLabel(gridLayoutWidget_7);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        sizePolicy4.setHeightForWidth(label_9->sizePolicy().hasHeightForWidth());
        label_9->setSizePolicy(sizePolicy4);

        gridLayout_6->addWidget(label_9, 2, 0, 1, 1);

        label_7 = new QLabel(gridLayoutWidget_7);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        sizePolicy4.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy4);

        gridLayout_6->addWidget(label_7, 1, 0, 1, 1);

        dispIP = new QLineEdit(gridLayoutWidget_7);
        dispIP->setObjectName(QString::fromUtf8("dispIP"));
        sizePolicy3.setHeightForWidth(dispIP->sizePolicy().hasHeightForWidth());
        dispIP->setSizePolicy(sizePolicy3);
        dispIP->setMinimumSize(QSize(180, 0));

        gridLayout_6->addWidget(dispIP, 1, 4, 1, 1);

        dispMAC = new QLineEdit(gridLayoutWidget_7);
        dispMAC->setObjectName(QString::fromUtf8("dispMAC"));
        sizePolicy3.setHeightForWidth(dispMAC->sizePolicy().hasHeightForWidth());
        dispMAC->setSizePolicy(sizePolicy3);
        dispMAC->setMinimumSize(QSize(180, 0));

        gridLayout_6->addWidget(dispMAC, 2, 4, 1, 1);

        lblMAC = new QLabel(gridLayoutWidget_7);
        lblMAC->setObjectName(QString::fromUtf8("lblMAC"));
        sizePolicy4.setHeightForWidth(lblMAC->sizePolicy().hasHeightForWidth());
        lblMAC->setSizePolicy(sizePolicy4);

        gridLayout_6->addWidget(lblMAC, 2, 3, 1, 1);

        lblOnline = new QLabel(gridLayoutWidget_7);
        lblOnline->setObjectName(QString::fromUtf8("lblOnline"));
        QSizePolicy sizePolicy5(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(lblOnline->sizePolicy().hasHeightForWidth());
        lblOnline->setSizePolicy(sizePolicy5);
        lblOnline->setMinimumSize(QSize(108, 0));

        gridLayout_6->addWidget(lblOnline, 0, 3, 1, 1);

        lblHostname = new QLabel(gridLayoutWidget_7);
        lblHostname->setObjectName(QString::fromUtf8("lblHostname"));
        sizePolicy5.setHeightForWidth(lblHostname->sizePolicy().hasHeightForWidth());
        lblHostname->setSizePolicy(sizePolicy5);
        lblHostname->setMinimumSize(QSize(108, 0));

        gridLayout_6->addWidget(lblHostname, 0, 0, 1, 1);

        lblIP = new QLabel(gridLayoutWidget_7);
        lblIP->setObjectName(QString::fromUtf8("lblIP"));
        sizePolicy4.setHeightForWidth(lblIP->sizePolicy().hasHeightForWidth());
        lblIP->setSizePolicy(sizePolicy4);

        gridLayout_6->addWidget(lblIP, 1, 3, 1, 1);

        comboDetector = new QComboBox(gridLayoutWidget_7);
        comboDetector->setObjectName(QString::fromUtf8("comboDetector"));
        sizePolicy3.setHeightForWidth(comboDetector->sizePolicy().hasHeightForWidth());
        comboDetector->setSizePolicy(sizePolicy3);
        comboDetector->setMinimumSize(QSize(180, 0));

        gridLayout_6->addWidget(comboDetector, 0, 1, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_6->addItem(horizontalSpacer, 0, 2, 1, 1);

        spinControlPort = new QSpinBox(gridLayoutWidget_7);
        spinControlPort->setObjectName(QString::fromUtf8("spinControlPort"));
        sizePolicy3.setHeightForWidth(spinControlPort->sizePolicy().hasHeightForWidth());
        spinControlPort->setSizePolicy(sizePolicy3);
        spinControlPort->setMinimumSize(QSize(180, 0));
        spinControlPort->setMaximumSize(QSize(16777215, 16777215));
        spinControlPort->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinControlPort->setKeyboardTracking(false);
        spinControlPort->setMinimum(0);
        spinControlPort->setMaximum(2000000000);
        spinControlPort->setValue(0);

        gridLayout_6->addWidget(spinControlPort, 1, 1, 1, 1);

        spinStopPort = new QSpinBox(gridLayoutWidget_7);
        spinStopPort->setObjectName(QString::fromUtf8("spinStopPort"));
        sizePolicy3.setHeightForWidth(spinStopPort->sizePolicy().hasHeightForWidth());
        spinStopPort->setSizePolicy(sizePolicy3);
        spinStopPort->setMinimumSize(QSize(180, 0));
        spinStopPort->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinStopPort->setKeyboardTracking(false);
        spinStopPort->setMinimum(0);
        spinStopPort->setMaximum(2000000000);
        spinStopPort->setValue(0);

        gridLayout_6->addWidget(spinStopPort, 2, 1, 1, 1);

        tabAdvancedSettings->addTab(tab, QString());
        QWidget::setTabOrder(tabAdvancedSettings, chkEnergyLog);
        QWidget::setTabOrder(chkEnergyLog, chkAngularLog);
        QWidget::setTabOrder(chkAngularLog, boxPlot);
        QWidget::setTabOrder(boxPlot, radioDataGraph);
        QWidget::setTabOrder(radioDataGraph, radioHistogram);
        QWidget::setTabOrder(radioHistogram, btnRefresh);
        QWidget::setTabOrder(btnRefresh, btnGetTrimbits);
        QWidget::setTabOrder(btnGetTrimbits, boxTrimming);
        QWidget::setTabOrder(boxTrimming, comboMethod);
        QWidget::setTabOrder(comboMethod, chkOptimize);
        QWidget::setTabOrder(chkOptimize, spinResolution);
        QWidget::setTabOrder(spinResolution, spinCounts);
        QWidget::setTabOrder(spinCounts, spinExpTime);
        QWidget::setTabOrder(spinExpTime, comboExpUnit);
        QWidget::setTabOrder(comboExpUnit, spinThreshold);
        QWidget::setTabOrder(spinThreshold, dispFile);
        QWidget::setTabOrder(dispFile, btnFile);
        QWidget::setTabOrder(btnFile, btnStart);
        QWidget::setTabOrder(btnStart, btnGetRoi);
        QWidget::setTabOrder(btnGetRoi, btnSetRoi);
        QWidget::setTabOrder(btnSetRoi, btnClearRoi);
        QWidget::setTabOrder(btnClearRoi, scrollArea);
        QWidget::setTabOrder(scrollArea, comboDetector);
        QWidget::setTabOrder(comboDetector, comboOnline);
        QWidget::setTabOrder(comboOnline, spinControlPort);
        QWidget::setTabOrder(spinControlPort, dispIP);
        QWidget::setTabOrder(dispIP, spinStopPort);
        QWidget::setTabOrder(spinStopPort, dispMAC);
        QWidget::setTabOrder(dispMAC, dispRxrHostname);
        QWidget::setTabOrder(dispRxrHostname, comboRxrOnline);
        QWidget::setTabOrder(comboRxrOnline, spinTCPPort);
        QWidget::setTabOrder(spinTCPPort, dispUDPIP);
        QWidget::setTabOrder(dispUDPIP, spinUDPPort);
        QWidget::setTabOrder(spinUDPPort, dispUDPMAC);
        QWidget::setTabOrder(dispUDPMAC, btnRxr);

        retranslateUi(TabAdvancedObject);

        tabAdvancedSettings->setCurrentIndex(1);
        comboExpUnit->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(TabAdvancedObject);
    } // setupUi

    void retranslateUi(QWidget *TabAdvancedObject)
    {
        TabAdvancedObject->setWindowTitle(QApplication::translate("TabAdvancedObject", "Form", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        tabAdvancedSettings->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        boxLogs->setTitle(QApplication::translate("TabAdvancedObject", "Calibration Logs", 0, QApplication::UnicodeUTF8));
        chkEnergyLog->setText(QApplication::translate("TabAdvancedObject", "Energy Calibration", 0, QApplication::UnicodeUTF8));
        chkAngularLog->setText(QApplication::translate("TabAdvancedObject", "Angular Calibration", 0, QApplication::UnicodeUTF8));
        tabAdvancedSettings->setTabText(tabAdvancedSettings->indexOf(tab_4), QApplication::translate("TabAdvancedObject", "Logs", 0, QApplication::UnicodeUTF8));
        boxPlot->setTitle(QApplication::translate("TabAdvancedObject", "Trimbits Plot", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        btnRefresh->setToolTip(QApplication::translate("TabAdvancedObject", "<nobr>\n"
"Updates plot with Trimbits from Shared Memory, not from Detector.\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnRefresh->setText(QApplication::translate("TabAdvancedObject", "Refresh  ", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        btnGetTrimbits->setToolTip(QApplication::translate("TabAdvancedObject", "<nobr>\n"
"Plots Trimbits from Detector. This will take time.\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnGetTrimbits->setText(QApplication::translate("TabAdvancedObject", "Get Trimbits  ", 0, QApplication::UnicodeUTF8));
        radioDataGraph->setText(QApplication::translate("TabAdvancedObject", "Data Graph", 0, QApplication::UnicodeUTF8));
        radioHistogram->setText(QApplication::translate("TabAdvancedObject", "Histogram", 0, QApplication::UnicodeUTF8));
        boxTrimming->setTitle(QApplication::translate("TabAdvancedObject", "Trimming", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("TabAdvancedObject", "Trimming Method:", 0, QApplication::UnicodeUTF8));
        lblExpTime->setText(QApplication::translate("TabAdvancedObject", "Exposure Time:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinExpTime->setToolTip(QApplication::translate("TabAdvancedObject", "Exposure time of each frame. \n"
" #exptime#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        comboExpUnit->clear();
        comboExpUnit->insertItems(0, QStringList()
         << QApplication::translate("TabAdvancedObject", "hr", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabAdvancedObject", "min", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabAdvancedObject", "s", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabAdvancedObject", "ms", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabAdvancedObject", "us", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabAdvancedObject", "ns", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        lblFile->setToolTip(QApplication::translate("TabAdvancedObject", "<nobr>\n"
"Trimfile to which the resulting trimbits will be written. \n"
"</nobr><br><nobr>\n"
"An extension given by the modules serial number will be attached.\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        lblFile->setText(QApplication::translate("TabAdvancedObject", "Output Trim File: ", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        dispFile->setToolTip(QApplication::translate("TabAdvancedObject", "<nobr>\n"
"Trimfile to which the resulting trimbits will be written. \n"
"</nobr><br><nobr>\n"
"An extension given by the modules serial number will be attached.\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        btnFile->setToolTip(QApplication::translate("TabAdvancedObject", "<nobr>\n"
"Trimfile to which the resulting trimbits will be written. \n"
"</nobr><br><nobr>\n"
"An extension given by the modules serial number will be attached.\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnFile->setText(QApplication::translate("TabAdvancedObject", "Browse", 0, QApplication::UnicodeUTF8));
        btnStart->setText(QApplication::translate("TabAdvancedObject", "Start Trimming  ", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinThreshold->setToolTip(QApplication::translate("TabAdvancedObject", "Exposure time of each frame. \n"
" #exptime#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        label_15->setToolTip(QApplication::translate("TabAdvancedObject", "Sets the Threshold DAC", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_15->setText(QApplication::translate("TabAdvancedObject", "Threshold (DACu):", 0, QApplication::UnicodeUTF8));
        chkOptimize->setText(QApplication::translate("TabAdvancedObject", "Optimize Settings", 0, QApplication::UnicodeUTF8));
        lblResolution->setText(QApplication::translate("TabAdvancedObject", "Resolution (a.u.):", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinResolution->setToolTip(QApplication::translate("TabAdvancedObject", "Number of measurements (not in real time) that will be acquired. \n"
" #frames#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        spinResolution->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        spinResolution->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        spinResolution->setSuffix(QString());
        comboMethod->clear();
        comboMethod->insertItems(0, QStringList()
         << QApplication::translate("TabAdvancedObject", "Adjust to Fix Count Level     ", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabAdvancedObject", "Equalize to Median", 0, QApplication::UnicodeUTF8)
        );
        lblCounts->setText(QApplication::translate("TabAdvancedObject", "Counts/ Channel:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinCounts->setToolTip(QApplication::translate("TabAdvancedObject", "Number of measurements (not in real time) that will be acquired. \n"
" #frames#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        spinCounts->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        spinCounts->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        spinCounts->setSuffix(QString());
        boxSetAllTrimbits->setTitle(QApplication::translate("TabAdvancedObject", "Developer Option", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("TabAdvancedObject", "Set All trimbits:", 0, QApplication::UnicodeUTF8));
        tabAdvancedSettings->setTabText(tabAdvancedSettings->indexOf(tab_3), QApplication::translate("TabAdvancedObject", "Trimming", 0, QApplication::UnicodeUTF8));
        btnGetRoi->setText(QApplication::translate("TabAdvancedObject", "  Get ROI  ", 0, QApplication::UnicodeUTF8));
        btnSetRoi->setText(QApplication::translate("TabAdvancedObject", "  Set ROI  ", 0, QApplication::UnicodeUTF8));
        btnClearRoi->setText(QApplication::translate("TabAdvancedObject", "  Clear ROI  ", 0, QApplication::UnicodeUTF8));
        tabAdvancedSettings->setTabText(tabAdvancedSettings->indexOf(tab_5), QApplication::translate("TabAdvancedObject", "Region of Interest", 0, QApplication::UnicodeUTF8));
        boxRxr->setTitle(QApplication::translate("TabAdvancedObject", "Receiver Parameters", 0, QApplication::UnicodeUTF8));
        btnRxr->setText(QApplication::translate("TabAdvancedObject", " Setup Receiver", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("TabAdvancedObject", "TCP Port:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        dispUDPIP->setToolTip(QApplication::translate("TabAdvancedObject", "Sets the Receiver UDP IP\n"
"#rx_udpip#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        dispUDPIP->setText(QApplication::translate("TabAdvancedObject", "none", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinUDPPort->setToolTip(QApplication::translate("TabAdvancedObject", "Sets Receiver UDP Port\n"
"#rx_udpport#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        spinUDPPort->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        spinUDPPort->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        spinUDPPort->setSuffix(QString());
        label_14->setText(QApplication::translate("TabAdvancedObject", "UDP IP:", 0, QApplication::UnicodeUTF8));
        label_13->setText(QApplication::translate("TabAdvancedObject", "Hostname / IP:", 0, QApplication::UnicodeUTF8));
        comboRxrOnline->clear();
        comboRxrOnline->insertItems(0, QStringList()
         << QApplication::translate("TabAdvancedObject", "Offline", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabAdvancedObject", "Online", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        comboRxrOnline->setToolTip(QApplication::translate("TabAdvancedObject", "<nobr>If the receiver is online<br>#r_online#</nobr><", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        dispUDPMAC->setToolTip(QApplication::translate("TabAdvancedObject", "Sets the Receiver UDP MAC\n"
"#rx_udpmac#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        dispUDPMAC->setText(QApplication::translate("TabAdvancedObject", "none", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_STATUSTIP
        spinTCPPort->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        spinTCPPort->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        spinTCPPort->setSuffix(QString());
        lblRxrOnline->setText(QApplication::translate("TabAdvancedObject", "Online:", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("TabAdvancedObject", "UDP Port:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        dispRxrHostname->setToolTip(QApplication::translate("TabAdvancedObject", "Sets the Receiver Hostname, connects to it and gets the receiver mac address and eth. Sets some receiver parameters like file name, file dir, file index in receiver. \n"
"#rx_hostname#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        dispRxrHostname->setText(QApplication::translate("TabAdvancedObject", "none", 0, QApplication::UnicodeUTF8));
        label_18->setText(QApplication::translate("TabAdvancedObject", "UDP MAC:", 0, QApplication::UnicodeUTF8));
        comboOnline->clear();
        comboOnline->insertItems(0, QStringList()
         << QApplication::translate("TabAdvancedObject", "Offline", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabAdvancedObject", "Online", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        comboOnline->setToolTip(QApplication::translate("TabAdvancedObject", "<nobr>If the detector is online<br>#online#</nobr><", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_9->setText(QApplication::translate("TabAdvancedObject", "Stop Port:", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("TabAdvancedObject", "Control Port:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        dispIP->setToolTip(QApplication::translate("TabAdvancedObject", "Sets the detector IP to send packets to receiver\n"
"#detectorip#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        dispIP->setText(QString());
#ifndef QT_NO_TOOLTIP
        dispMAC->setToolTip(QApplication::translate("TabAdvancedObject", "Sets the detector MAC to send packets to receiver\n"
"#detectormac#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        dispMAC->setText(QString());
        lblMAC->setText(QApplication::translate("TabAdvancedObject", "MAC:", 0, QApplication::UnicodeUTF8));
        lblOnline->setText(QApplication::translate("TabAdvancedObject", "Online:", 0, QApplication::UnicodeUTF8));
        lblHostname->setText(QApplication::translate("TabAdvancedObject", "Detector:", 0, QApplication::UnicodeUTF8));
        lblIP->setText(QApplication::translate("TabAdvancedObject", "IP:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinControlPort->setToolTip(QApplication::translate("TabAdvancedObject", "Sets Control Port\n"
"#port#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        spinControlPort->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        spinControlPort->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        spinControlPort->setSuffix(QString());
#ifndef QT_NO_TOOLTIP
        spinStopPort->setToolTip(QApplication::translate("TabAdvancedObject", "Sets Stop Port \n"
"#stopport#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        spinStopPort->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        spinStopPort->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        spinStopPort->setSuffix(QString());
        tabAdvancedSettings->setTabText(tabAdvancedSettings->indexOf(tab), QApplication::translate("TabAdvancedObject", "Network", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TabAdvancedObject: public Ui_TabAdvancedObject {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORM_TAB_ADVANCED_H
