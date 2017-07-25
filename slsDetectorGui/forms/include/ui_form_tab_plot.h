/********************************************************************************
** Form generated from reading UI file 'form_tab_plot.ui'
**
** Created: Tue Jul 25 12:31:25 2017
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORM_TAB_PLOT_H
#define UI_FORM_TAB_PLOT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QStackedWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TabPlotObject
{
public:
    QGroupBox *box2D;
    QStackedWidget *stackedWidget_2;
    QWidget *page_6;
    QWidget *gridLayoutWidget_5;
    QGridLayout *gridLayout_5;
    QCheckBox *chkInterpolate;
    QCheckBox *chkContour;
    QCheckBox *chkLogz;
    QSpacerItem *horizontalSpacer_15;
    QSpacerItem *horizontalSpacer_16;
    QWidget *pageAccumulate_2;
    QWidget *horizontalLayoutWidget_6;
    QHBoxLayout *layoutSave_4;
    QCheckBox *chkAccumulate_2;
    QPushButton *btnResetAccumulate_2;
    QWidget *pagePedestal_2;
    QWidget *horizontalLayoutWidget_5;
    QHBoxLayout *layoutSave_3;
    QCheckBox *chkPedestal_2;
    QPushButton *btnRecalPedestal_2;
    QWidget *page_9;
    QWidget *horizontalLayoutWidget_10;
    QHBoxLayout *layoutThreshold_2;
    QCheckBox *chkBinary_2;
    QSpacerItem *horizontalSpacer_21;
    QLabel *lblFrom_2;
    QSpinBox *spinFrom_2;
    QLabel *lblTo_2;
    QSpinBox *spinTo_2;
    QWidget *page_11;
    QWidget *horizontalLayoutWidget_12;
    QHBoxLayout *layoutSave_8;
    QCheckBox *chkStatistics_2;
    QWidget *pageHistogram_2;
    QWidget *horizontalLayoutWidget_14;
    QHBoxLayout *layoutThreshold_4;
    QLabel *lblHistFrom_2;
    QSpinBox *spinHistFrom_2;
    QLabel *lblHistTo_2;
    QSpinBox *spinHistTo_2;
    QLabel *lblHistSize_2;
    QSpinBox *spinHistSize_2;
    QGroupBox *box1D;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QWidget *gridLayoutWidget_2;
    QGridLayout *gridLayout_2;
    QCheckBox *chkSuperimpose;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *horizontalLayout;
    QLabel *lblPersistency;
    QSpinBox *spinPersistency;
    QWidget *page_2;
    QWidget *gridLayoutWidget_8;
    QGridLayout *gridLayout_10;
    QCheckBox *chk1DLog;
    QSpacerItem *horizontalSpacer_20;
    QCheckBox *chkPoints;
    QCheckBox *chkLines;
    QWidget *pageAccumulate;
    QPushButton *btnResetAccumulate;
    QWidget *horizontalLayoutWidget_7;
    QHBoxLayout *layoutSave_5;
    QCheckBox *chkAccumulate;
    QWidget *pagePedestal;
    QWidget *horizontalLayoutWidget_8;
    QHBoxLayout *layoutSave_6;
    QCheckBox *chkPedestal;
    QPushButton *btnRecalPedestal;
    QWidget *page_8;
    QWidget *horizontalLayoutWidget_11;
    QHBoxLayout *layoutThreshold;
    QCheckBox *chkBinary;
    QSpacerItem *horizontalSpacer_29;
    QLabel *lblFrom;
    QSpinBox *spinFrom;
    QLabel *lblTo;
    QSpinBox *spinTo;
    QWidget *page_10;
    QWidget *horizontalLayoutWidget_9;
    QHBoxLayout *layoutSave_7;
    QCheckBox *chkStatistics;
    QWidget *pageHistogram;
    QWidget *horizontalLayoutWidget_13;
    QHBoxLayout *layoutThreshold_3;
    QLabel *lblHistFrom;
    QSpinBox *spinHistFrom;
    QLabel *lblHistTo;
    QSpinBox *spinHistTo;
    QLabel *lblHistSize;
    QSpinBox *spinHistSize;
    QGroupBox *boxSave;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *layoutSave;
    QCheckBox *chkSaveAll;
    QPushButton *btnSave;
    QGroupBox *boxScan;
    QWidget *horizontalLayoutWidget_3;
    QHBoxLayout *horizontalLayout_2;
    QRadioButton *radioLevel0;
    QSpacerItem *horizontalSpacer_6;
    QRadioButton *radioLevel1;
    QSpacerItem *horizontalSpacer_7;
    QRadioButton *radioFileIndex;
    QSpacerItem *horizontalSpacer_8;
    QRadioButton *radioAllFrames;
    QGroupBox *boxPlotAxis;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QCheckBox *chkTitle;
    QLineEdit *dispTitle;
    QLineEdit *dispZMax;
    QCheckBox *chkZMax;
    QLineEdit *dispYMax;
    QCheckBox *chkYMax;
    QCheckBox *chkXMax;
    QLineEdit *dispXMax;
    QSpacerItem *horizontalSpacer_2;
    QCheckBox *chkZMin;
    QLineEdit *dispZMin;
    QCheckBox *chkXMin;
    QLineEdit *dispXMin;
    QCheckBox *chkYMin;
    QLineEdit *dispYMin;
    QSpacerItem *horizontalSpacer_9;
    QCheckBox *chkXAxis;
    QLineEdit *dispXAxis;
    QCheckBox *chkYAxis;
    QLineEdit *dispYAxis;
    QCheckBox *chkZAxis;
    QLineEdit *dispZAxis;
    QGroupBox *groupBox_3;
    QWidget *gridLayoutWidget_6;
    QGridLayout *gridLayout_6;
    QRadioButton *radioNoPlot;
    QRadioButton *radioDataGraph;
    QRadioButton *radioHistogram;
    QSpacerItem *horizontalSpacer_17;
    QSpacerItem *horizontalSpacer_18;
    QGroupBox *boxFrequency;
    QWidget *horizontalLayoutWidget_4;
    QHBoxLayout *layoutSave_2;
    QComboBox *comboFrequency;
    QSpacerItem *horizontalSpacer_14;
    QWidget *stackWidget;
    QGroupBox *boxSnapshot;
    QWidget *gridLayoutWidget_4;
    QGridLayout *gridLayout_4;
    QPushButton *btnClone;
    QPushButton *btnCloseClones;
    QSpacerItem *horizontalSpacer_12;
    QSpacerItem *horizontalSpacer_13;
    QPushButton *btnSaveClones;
    QPushButton *btnRight;
    QPushButton *btnLeft;
    QGroupBox *boxHistogram;
    QWidget *horizontalLayoutWidget_15;
    QHBoxLayout *horizontalLayout_3;
    QRadioButton *radioHistIntensity;
    QSpacerItem *horizontalSpacer_10;
    QRadioButton *radioHistLevel0;
    QSpacerItem *horizontalSpacer_11;
    QRadioButton *radioHistLevel1;

    void setupUi(QWidget *TabPlotObject)
    {
        if (TabPlotObject->objectName().isEmpty())
            TabPlotObject->setObjectName(QString::fromUtf8("TabPlotObject"));
        TabPlotObject->resize(775, 345);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(TabPlotObject->sizePolicy().hasHeightForWidth());
        TabPlotObject->setSizePolicy(sizePolicy);
        TabPlotObject->setMinimumSize(QSize(0, 0));
        TabPlotObject->setMaximumSize(QSize(1000, 1000));
        QPalette palette;
        QBrush brush(QColor(0, 0, 30, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Shadow, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Shadow, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Shadow, brush);
        TabPlotObject->setPalette(palette);
        box2D = new QGroupBox(TabPlotObject);
        box2D->setObjectName(QString::fromUtf8("box2D"));
        box2D->setEnabled(true);
        box2D->setGeometry(QRect(15, 70, 371, 51));
        box2D->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        box2D->setFlat(false);
        stackedWidget_2 = new QStackedWidget(box2D);
        stackedWidget_2->setObjectName(QString::fromUtf8("stackedWidget_2"));
        stackedWidget_2->setGeometry(QRect(5, 15, 361, 31));
        page_6 = new QWidget();
        page_6->setObjectName(QString::fromUtf8("page_6"));
        gridLayoutWidget_5 = new QWidget(page_6);
        gridLayoutWidget_5->setObjectName(QString::fromUtf8("gridLayoutWidget_5"));
        gridLayoutWidget_5->setGeometry(QRect(20, 5, 327, 26));
        gridLayout_5 = new QGridLayout(gridLayoutWidget_5);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        gridLayout_5->setContentsMargins(0, 0, 0, 0);
        chkInterpolate = new QCheckBox(gridLayoutWidget_5);
        chkInterpolate->setObjectName(QString::fromUtf8("chkInterpolate"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(chkInterpolate->sizePolicy().hasHeightForWidth());
        chkInterpolate->setSizePolicy(sizePolicy1);

        gridLayout_5->addWidget(chkInterpolate, 0, 0, 1, 1);

        chkContour = new QCheckBox(gridLayoutWidget_5);
        chkContour->setObjectName(QString::fromUtf8("chkContour"));
        sizePolicy1.setHeightForWidth(chkContour->sizePolicy().hasHeightForWidth());
        chkContour->setSizePolicy(sizePolicy1);

        gridLayout_5->addWidget(chkContour, 0, 2, 1, 1);

        chkLogz = new QCheckBox(gridLayoutWidget_5);
        chkLogz->setObjectName(QString::fromUtf8("chkLogz"));
        sizePolicy1.setHeightForWidth(chkLogz->sizePolicy().hasHeightForWidth());
        chkLogz->setSizePolicy(sizePolicy1);

        gridLayout_5->addWidget(chkLogz, 0, 4, 1, 1);

        horizontalSpacer_15 = new QSpacerItem(15, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_15, 0, 1, 1, 1);

        horizontalSpacer_16 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_16, 0, 3, 1, 1);

        stackedWidget_2->addWidget(page_6);
        pageAccumulate_2 = new QWidget();
        pageAccumulate_2->setObjectName(QString::fromUtf8("pageAccumulate_2"));
        horizontalLayoutWidget_6 = new QWidget(pageAccumulate_2);
        horizontalLayoutWidget_6->setObjectName(QString::fromUtf8("horizontalLayoutWidget_6"));
        horizontalLayoutWidget_6->setGeometry(QRect(25, 5, 95, 26));
        layoutSave_4 = new QHBoxLayout(horizontalLayoutWidget_6);
        layoutSave_4->setSpacing(0);
        layoutSave_4->setObjectName(QString::fromUtf8("layoutSave_4"));
        layoutSave_4->setContentsMargins(0, 0, 0, 0);
        chkAccumulate_2 = new QCheckBox(horizontalLayoutWidget_6);
        chkAccumulate_2->setObjectName(QString::fromUtf8("chkAccumulate_2"));
        chkAccumulate_2->setEnabled(true);
        sizePolicy1.setHeightForWidth(chkAccumulate_2->sizePolicy().hasHeightForWidth());
        chkAccumulate_2->setSizePolicy(sizePolicy1);

        layoutSave_4->addWidget(chkAccumulate_2);

        btnResetAccumulate_2 = new QPushButton(pageAccumulate_2);
        btnResetAccumulate_2->setObjectName(QString::fromUtf8("btnResetAccumulate_2"));
        btnResetAccumulate_2->setGeometry(QRect(175, 3, 156, 26));
        sizePolicy1.setHeightForWidth(btnResetAccumulate_2->sizePolicy().hasHeightForWidth());
        btnResetAccumulate_2->setSizePolicy(sizePolicy1);
        btnResetAccumulate_2->setMaximumSize(QSize(16777215, 16777215));
        QPalette palette1;
        QBrush brush1(QColor(20, 20, 20, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Shadow, brush1);
        palette1.setBrush(QPalette::Inactive, QPalette::Shadow, brush1);
        palette1.setBrush(QPalette::Disabled, QPalette::Shadow, brush1);
        btnResetAccumulate_2->setPalette(palette1);
        btnResetAccumulate_2->setFocusPolicy(Qt::NoFocus);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/images/calculate.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnResetAccumulate_2->setIcon(icon);
        btnResetAccumulate_2->setIconSize(QSize(16, 16));
        stackedWidget_2->addWidget(pageAccumulate_2);
        pagePedestal_2 = new QWidget();
        pagePedestal_2->setObjectName(QString::fromUtf8("pagePedestal_2"));
        horizontalLayoutWidget_5 = new QWidget(pagePedestal_2);
        horizontalLayoutWidget_5->setObjectName(QString::fromUtf8("horizontalLayoutWidget_5"));
        horizontalLayoutWidget_5->setGeometry(QRect(20, 5, 77, 26));
        layoutSave_3 = new QHBoxLayout(horizontalLayoutWidget_5);
        layoutSave_3->setSpacing(0);
        layoutSave_3->setObjectName(QString::fromUtf8("layoutSave_3"));
        layoutSave_3->setContentsMargins(0, 0, 0, 0);
        chkPedestal_2 = new QCheckBox(horizontalLayoutWidget_5);
        chkPedestal_2->setObjectName(QString::fromUtf8("chkPedestal_2"));
        chkPedestal_2->setEnabled(true);
        sizePolicy1.setHeightForWidth(chkPedestal_2->sizePolicy().hasHeightForWidth());
        chkPedestal_2->setSizePolicy(sizePolicy1);

        layoutSave_3->addWidget(chkPedestal_2);

        btnRecalPedestal_2 = new QPushButton(pagePedestal_2);
        btnRecalPedestal_2->setObjectName(QString::fromUtf8("btnRecalPedestal_2"));
        btnRecalPedestal_2->setGeometry(QRect(160, 3, 171, 26));
        sizePolicy1.setHeightForWidth(btnRecalPedestal_2->sizePolicy().hasHeightForWidth());
        btnRecalPedestal_2->setSizePolicy(sizePolicy1);
        btnRecalPedestal_2->setMaximumSize(QSize(16777215, 16777215));
        QPalette palette2;
        palette2.setBrush(QPalette::Active, QPalette::Shadow, brush1);
        palette2.setBrush(QPalette::Inactive, QPalette::Shadow, brush1);
        palette2.setBrush(QPalette::Disabled, QPalette::Shadow, brush1);
        btnRecalPedestal_2->setPalette(palette2);
        btnRecalPedestal_2->setFocusPolicy(Qt::NoFocus);
        btnRecalPedestal_2->setIcon(icon);
        btnRecalPedestal_2->setIconSize(QSize(16, 16));
        stackedWidget_2->addWidget(pagePedestal_2);
        page_9 = new QWidget();
        page_9->setObjectName(QString::fromUtf8("page_9"));
        horizontalLayoutWidget_10 = new QWidget(page_9);
        horizontalLayoutWidget_10->setObjectName(QString::fromUtf8("horizontalLayoutWidget_10"));
        horizontalLayoutWidget_10->setGeometry(QRect(25, 5, 311, 26));
        layoutThreshold_2 = new QHBoxLayout(horizontalLayoutWidget_10);
        layoutThreshold_2->setSpacing(1);
        layoutThreshold_2->setObjectName(QString::fromUtf8("layoutThreshold_2"));
        layoutThreshold_2->setContentsMargins(0, 0, 0, 0);
        chkBinary_2 = new QCheckBox(horizontalLayoutWidget_10);
        chkBinary_2->setObjectName(QString::fromUtf8("chkBinary_2"));
        sizePolicy1.setHeightForWidth(chkBinary_2->sizePolicy().hasHeightForWidth());
        chkBinary_2->setSizePolicy(sizePolicy1);

        layoutThreshold_2->addWidget(chkBinary_2);

        horizontalSpacer_21 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        layoutThreshold_2->addItem(horizontalSpacer_21);

        lblFrom_2 = new QLabel(horizontalLayoutWidget_10);
        lblFrom_2->setObjectName(QString::fromUtf8("lblFrom_2"));
        lblFrom_2->setEnabled(false);
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(lblFrom_2->sizePolicy().hasHeightForWidth());
        lblFrom_2->setSizePolicy(sizePolicy2);

        layoutThreshold_2->addWidget(lblFrom_2);

        spinFrom_2 = new QSpinBox(horizontalLayoutWidget_10);
        spinFrom_2->setObjectName(QString::fromUtf8("spinFrom_2"));
        spinFrom_2->setEnabled(false);
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(spinFrom_2->sizePolicy().hasHeightForWidth());
        spinFrom_2->setSizePolicy(sizePolicy3);
        spinFrom_2->setMaximumSize(QSize(16777215, 16777215));
        spinFrom_2->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinFrom_2->setMinimum(-16777215);
        spinFrom_2->setMaximum(16777215);
        spinFrom_2->setValue(0);

        layoutThreshold_2->addWidget(spinFrom_2);

        lblTo_2 = new QLabel(horizontalLayoutWidget_10);
        lblTo_2->setObjectName(QString::fromUtf8("lblTo_2"));
        lblTo_2->setEnabled(false);
        sizePolicy2.setHeightForWidth(lblTo_2->sizePolicy().hasHeightForWidth());
        lblTo_2->setSizePolicy(sizePolicy2);

        layoutThreshold_2->addWidget(lblTo_2);

        spinTo_2 = new QSpinBox(horizontalLayoutWidget_10);
        spinTo_2->setObjectName(QString::fromUtf8("spinTo_2"));
        spinTo_2->setEnabled(false);
        sizePolicy3.setHeightForWidth(spinTo_2->sizePolicy().hasHeightForWidth());
        spinTo_2->setSizePolicy(sizePolicy3);
        spinTo_2->setMaximumSize(QSize(16777215, 16777215));
        spinTo_2->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinTo_2->setMinimum(-16777215);
        spinTo_2->setMaximum(16777215);
        spinTo_2->setValue(0);

        layoutThreshold_2->addWidget(spinTo_2);

        stackedWidget_2->addWidget(page_9);
        page_11 = new QWidget();
        page_11->setObjectName(QString::fromUtf8("page_11"));
        horizontalLayoutWidget_12 = new QWidget(page_11);
        horizontalLayoutWidget_12->setObjectName(QString::fromUtf8("horizontalLayoutWidget_12"));
        horizontalLayoutWidget_12->setGeometry(QRect(25, 5, 128, 26));
        layoutSave_8 = new QHBoxLayout(horizontalLayoutWidget_12);
        layoutSave_8->setSpacing(0);
        layoutSave_8->setObjectName(QString::fromUtf8("layoutSave_8"));
        layoutSave_8->setContentsMargins(0, 0, 0, 0);
        chkStatistics_2 = new QCheckBox(horizontalLayoutWidget_12);
        chkStatistics_2->setObjectName(QString::fromUtf8("chkStatistics_2"));
        chkStatistics_2->setEnabled(true);
        sizePolicy1.setHeightForWidth(chkStatistics_2->sizePolicy().hasHeightForWidth());
        chkStatistics_2->setSizePolicy(sizePolicy1);

        layoutSave_8->addWidget(chkStatistics_2);

        stackedWidget_2->addWidget(page_11);
        pageHistogram_2 = new QWidget();
        pageHistogram_2->setObjectName(QString::fromUtf8("pageHistogram_2"));
        pageHistogram_2->setEnabled(true);
        horizontalLayoutWidget_14 = new QWidget(pageHistogram_2);
        horizontalLayoutWidget_14->setObjectName(QString::fromUtf8("horizontalLayoutWidget_14"));
        horizontalLayoutWidget_14->setGeometry(QRect(25, 5, 311, 26));
        layoutThreshold_4 = new QHBoxLayout(horizontalLayoutWidget_14);
        layoutThreshold_4->setSpacing(1);
        layoutThreshold_4->setObjectName(QString::fromUtf8("layoutThreshold_4"));
        layoutThreshold_4->setContentsMargins(0, 0, 0, 0);
        lblHistFrom_2 = new QLabel(horizontalLayoutWidget_14);
        lblHistFrom_2->setObjectName(QString::fromUtf8("lblHistFrom_2"));
        lblHistFrom_2->setEnabled(true);
        sizePolicy2.setHeightForWidth(lblHistFrom_2->sizePolicy().hasHeightForWidth());
        lblHistFrom_2->setSizePolicy(sizePolicy2);

        layoutThreshold_4->addWidget(lblHistFrom_2);

        spinHistFrom_2 = new QSpinBox(horizontalLayoutWidget_14);
        spinHistFrom_2->setObjectName(QString::fromUtf8("spinHistFrom_2"));
        spinHistFrom_2->setEnabled(true);
        sizePolicy3.setHeightForWidth(spinHistFrom_2->sizePolicy().hasHeightForWidth());
        spinHistFrom_2->setSizePolicy(sizePolicy3);
        spinHistFrom_2->setMaximumSize(QSize(16777215, 16777215));
        spinHistFrom_2->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinHistFrom_2->setMinimum(-16777215);
        spinHistFrom_2->setMaximum(16777215);
        spinHistFrom_2->setValue(0);

        layoutThreshold_4->addWidget(spinHistFrom_2);

        lblHistTo_2 = new QLabel(horizontalLayoutWidget_14);
        lblHistTo_2->setObjectName(QString::fromUtf8("lblHistTo_2"));
        lblHistTo_2->setEnabled(true);
        sizePolicy2.setHeightForWidth(lblHistTo_2->sizePolicy().hasHeightForWidth());
        lblHistTo_2->setSizePolicy(sizePolicy2);

        layoutThreshold_4->addWidget(lblHistTo_2);

        spinHistTo_2 = new QSpinBox(horizontalLayoutWidget_14);
        spinHistTo_2->setObjectName(QString::fromUtf8("spinHistTo_2"));
        spinHistTo_2->setEnabled(true);
        sizePolicy3.setHeightForWidth(spinHistTo_2->sizePolicy().hasHeightForWidth());
        spinHistTo_2->setSizePolicy(sizePolicy3);
        spinHistTo_2->setMaximumSize(QSize(16777215, 16777215));
        spinHistTo_2->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinHistTo_2->setMinimum(-16777215);
        spinHistTo_2->setMaximum(16777215);
        spinHistTo_2->setValue(10000);

        layoutThreshold_4->addWidget(spinHistTo_2);

        lblHistSize_2 = new QLabel(horizontalLayoutWidget_14);
        lblHistSize_2->setObjectName(QString::fromUtf8("lblHistSize_2"));
        lblHistSize_2->setEnabled(true);
        sizePolicy2.setHeightForWidth(lblHistSize_2->sizePolicy().hasHeightForWidth());
        lblHistSize_2->setSizePolicy(sizePolicy2);

        layoutThreshold_4->addWidget(lblHistSize_2);

        spinHistSize_2 = new QSpinBox(horizontalLayoutWidget_14);
        spinHistSize_2->setObjectName(QString::fromUtf8("spinHistSize_2"));
        spinHistSize_2->setEnabled(true);
        sizePolicy3.setHeightForWidth(spinHistSize_2->sizePolicy().hasHeightForWidth());
        spinHistSize_2->setSizePolicy(sizePolicy3);
        spinHistSize_2->setMaximumSize(QSize(16777215, 16777215));
        spinHistSize_2->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinHistSize_2->setMinimum(-16777215);
        spinHistSize_2->setMaximum(16777215);
        spinHistSize_2->setValue(1000);

        layoutThreshold_4->addWidget(spinHistSize_2);

        stackedWidget_2->addWidget(pageHistogram_2);
        box1D = new QGroupBox(TabPlotObject);
        box1D->setObjectName(QString::fromUtf8("box1D"));
        box1D->setGeometry(QRect(15, 70, 371, 51));
        box1D->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        box1D->setFlat(false);
        stackedWidget = new QStackedWidget(box1D);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        stackedWidget->setGeometry(QRect(4, 15, 366, 31));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        gridLayoutWidget_2 = new QWidget(page);
        gridLayoutWidget_2->setObjectName(QString::fromUtf8("gridLayoutWidget_2"));
        gridLayoutWidget_2->setGeometry(QRect(25, 5, 107, 26));
        gridLayout_2 = new QGridLayout(gridLayoutWidget_2);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        chkSuperimpose = new QCheckBox(gridLayoutWidget_2);
        chkSuperimpose->setObjectName(QString::fromUtf8("chkSuperimpose"));
        sizePolicy1.setHeightForWidth(chkSuperimpose->sizePolicy().hasHeightForWidth());
        chkSuperimpose->setSizePolicy(sizePolicy1);
        chkSuperimpose->setTristate(false);

        gridLayout_2->addWidget(chkSuperimpose, 0, 0, 1, 1);

        horizontalLayoutWidget_2 = new QWidget(page);
        horizontalLayoutWidget_2->setObjectName(QString::fromUtf8("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(210, 5, 128, 26));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget_2);
        horizontalLayout->setSpacing(1);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        lblPersistency = new QLabel(horizontalLayoutWidget_2);
        lblPersistency->setObjectName(QString::fromUtf8("lblPersistency"));
        lblPersistency->setEnabled(false);
        sizePolicy2.setHeightForWidth(lblPersistency->sizePolicy().hasHeightForWidth());
        lblPersistency->setSizePolicy(sizePolicy2);

        horizontalLayout->addWidget(lblPersistency);

        spinPersistency = new QSpinBox(horizontalLayoutWidget_2);
        spinPersistency->setObjectName(QString::fromUtf8("spinPersistency"));
        spinPersistency->setEnabled(false);
        QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Maximum);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(spinPersistency->sizePolicy().hasHeightForWidth());
        spinPersistency->setSizePolicy(sizePolicy4);
        spinPersistency->setMaximumSize(QSize(40, 16777215));
        spinPersistency->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinPersistency->setMinimum(1);
        spinPersistency->setMaximum(10);
        spinPersistency->setValue(1);

        horizontalLayout->addWidget(spinPersistency);

        stackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        gridLayoutWidget_8 = new QWidget(page_2);
        gridLayoutWidget_8->setObjectName(QString::fromUtf8("gridLayoutWidget_8"));
        gridLayoutWidget_8->setGeometry(QRect(25, 5, 321, 26));
        gridLayout_10 = new QGridLayout(gridLayoutWidget_8);
        gridLayout_10->setObjectName(QString::fromUtf8("gridLayout_10"));
        gridLayout_10->setContentsMargins(0, 0, 0, 0);
        chk1DLog = new QCheckBox(gridLayoutWidget_8);
        chk1DLog->setObjectName(QString::fromUtf8("chk1DLog"));
        sizePolicy1.setHeightForWidth(chk1DLog->sizePolicy().hasHeightForWidth());
        chk1DLog->setSizePolicy(sizePolicy1);

        gridLayout_10->addWidget(chk1DLog, 0, 0, 1, 1);

        horizontalSpacer_20 = new QSpacerItem(95, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_10->addItem(horizontalSpacer_20, 0, 1, 1, 1);

        chkPoints = new QCheckBox(gridLayoutWidget_8);
        chkPoints->setObjectName(QString::fromUtf8("chkPoints"));
        chkPoints->setEnabled(true);
        sizePolicy1.setHeightForWidth(chkPoints->sizePolicy().hasHeightForWidth());
        chkPoints->setSizePolicy(sizePolicy1);

        gridLayout_10->addWidget(chkPoints, 0, 2, 1, 1);

        chkLines = new QCheckBox(gridLayoutWidget_8);
        chkLines->setObjectName(QString::fromUtf8("chkLines"));
        chkLines->setEnabled(true);
        sizePolicy1.setHeightForWidth(chkLines->sizePolicy().hasHeightForWidth());
        chkLines->setSizePolicy(sizePolicy1);
        chkLines->setChecked(true);

        gridLayout_10->addWidget(chkLines, 0, 3, 1, 1);

        stackedWidget->addWidget(page_2);
        pageAccumulate = new QWidget();
        pageAccumulate->setObjectName(QString::fromUtf8("pageAccumulate"));
        btnResetAccumulate = new QPushButton(pageAccumulate);
        btnResetAccumulate->setObjectName(QString::fromUtf8("btnResetAccumulate"));
        btnResetAccumulate->setGeometry(QRect(175, 3, 161, 26));
        sizePolicy1.setHeightForWidth(btnResetAccumulate->sizePolicy().hasHeightForWidth());
        btnResetAccumulate->setSizePolicy(sizePolicy1);
        btnResetAccumulate->setMaximumSize(QSize(16777215, 16777215));
        QPalette palette3;
        palette3.setBrush(QPalette::Active, QPalette::Shadow, brush1);
        palette3.setBrush(QPalette::Inactive, QPalette::Shadow, brush1);
        palette3.setBrush(QPalette::Disabled, QPalette::Shadow, brush1);
        btnResetAccumulate->setPalette(palette3);
        btnResetAccumulate->setFocusPolicy(Qt::NoFocus);
        btnResetAccumulate->setIcon(icon);
        btnResetAccumulate->setIconSize(QSize(16, 16));
        horizontalLayoutWidget_7 = new QWidget(pageAccumulate);
        horizontalLayoutWidget_7->setObjectName(QString::fromUtf8("horizontalLayoutWidget_7"));
        horizontalLayoutWidget_7->setGeometry(QRect(25, 5, 95, 26));
        layoutSave_5 = new QHBoxLayout(horizontalLayoutWidget_7);
        layoutSave_5->setSpacing(0);
        layoutSave_5->setObjectName(QString::fromUtf8("layoutSave_5"));
        layoutSave_5->setContentsMargins(0, 0, 0, 0);
        chkAccumulate = new QCheckBox(horizontalLayoutWidget_7);
        chkAccumulate->setObjectName(QString::fromUtf8("chkAccumulate"));
        chkAccumulate->setEnabled(true);
        sizePolicy1.setHeightForWidth(chkAccumulate->sizePolicy().hasHeightForWidth());
        chkAccumulate->setSizePolicy(sizePolicy1);

        layoutSave_5->addWidget(chkAccumulate);

        stackedWidget->addWidget(pageAccumulate);
        pagePedestal = new QWidget();
        pagePedestal->setObjectName(QString::fromUtf8("pagePedestal"));
        horizontalLayoutWidget_8 = new QWidget(pagePedestal);
        horizontalLayoutWidget_8->setObjectName(QString::fromUtf8("horizontalLayoutWidget_8"));
        horizontalLayoutWidget_8->setGeometry(QRect(25, 5, 77, 26));
        layoutSave_6 = new QHBoxLayout(horizontalLayoutWidget_8);
        layoutSave_6->setSpacing(0);
        layoutSave_6->setObjectName(QString::fromUtf8("layoutSave_6"));
        layoutSave_6->setContentsMargins(0, 0, 0, 0);
        chkPedestal = new QCheckBox(horizontalLayoutWidget_8);
        chkPedestal->setObjectName(QString::fromUtf8("chkPedestal"));
        chkPedestal->setEnabled(true);
        sizePolicy1.setHeightForWidth(chkPedestal->sizePolicy().hasHeightForWidth());
        chkPedestal->setSizePolicy(sizePolicy1);

        layoutSave_6->addWidget(chkPedestal);

        btnRecalPedestal = new QPushButton(pagePedestal);
        btnRecalPedestal->setObjectName(QString::fromUtf8("btnRecalPedestal"));
        btnRecalPedestal->setGeometry(QRect(165, 3, 171, 26));
        sizePolicy1.setHeightForWidth(btnRecalPedestal->sizePolicy().hasHeightForWidth());
        btnRecalPedestal->setSizePolicy(sizePolicy1);
        btnRecalPedestal->setMaximumSize(QSize(16777215, 16777215));
        QPalette palette4;
        palette4.setBrush(QPalette::Active, QPalette::Shadow, brush1);
        palette4.setBrush(QPalette::Inactive, QPalette::Shadow, brush1);
        palette4.setBrush(QPalette::Disabled, QPalette::Shadow, brush1);
        btnRecalPedestal->setPalette(palette4);
        btnRecalPedestal->setFocusPolicy(Qt::NoFocus);
        btnRecalPedestal->setIcon(icon);
        btnRecalPedestal->setIconSize(QSize(16, 16));
        stackedWidget->addWidget(pagePedestal);
        page_8 = new QWidget();
        page_8->setObjectName(QString::fromUtf8("page_8"));
        horizontalLayoutWidget_11 = new QWidget(page_8);
        horizontalLayoutWidget_11->setObjectName(QString::fromUtf8("horizontalLayoutWidget_11"));
        horizontalLayoutWidget_11->setGeometry(QRect(25, 5, 311, 26));
        layoutThreshold = new QHBoxLayout(horizontalLayoutWidget_11);
        layoutThreshold->setSpacing(1);
        layoutThreshold->setObjectName(QString::fromUtf8("layoutThreshold"));
        layoutThreshold->setContentsMargins(0, 0, 0, 0);
        chkBinary = new QCheckBox(horizontalLayoutWidget_11);
        chkBinary->setObjectName(QString::fromUtf8("chkBinary"));
        sizePolicy1.setHeightForWidth(chkBinary->sizePolicy().hasHeightForWidth());
        chkBinary->setSizePolicy(sizePolicy1);

        layoutThreshold->addWidget(chkBinary);

        horizontalSpacer_29 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        layoutThreshold->addItem(horizontalSpacer_29);

        lblFrom = new QLabel(horizontalLayoutWidget_11);
        lblFrom->setObjectName(QString::fromUtf8("lblFrom"));
        lblFrom->setEnabled(false);
        sizePolicy2.setHeightForWidth(lblFrom->sizePolicy().hasHeightForWidth());
        lblFrom->setSizePolicy(sizePolicy2);

        layoutThreshold->addWidget(lblFrom);

        spinFrom = new QSpinBox(horizontalLayoutWidget_11);
        spinFrom->setObjectName(QString::fromUtf8("spinFrom"));
        spinFrom->setEnabled(false);
        sizePolicy3.setHeightForWidth(spinFrom->sizePolicy().hasHeightForWidth());
        spinFrom->setSizePolicy(sizePolicy3);
        spinFrom->setMaximumSize(QSize(16777215, 16777215));
        spinFrom->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinFrom->setMinimum(-16777215);
        spinFrom->setMaximum(16777215);
        spinFrom->setValue(0);

        layoutThreshold->addWidget(spinFrom);

        lblTo = new QLabel(horizontalLayoutWidget_11);
        lblTo->setObjectName(QString::fromUtf8("lblTo"));
        lblTo->setEnabled(false);
        sizePolicy2.setHeightForWidth(lblTo->sizePolicy().hasHeightForWidth());
        lblTo->setSizePolicy(sizePolicy2);

        layoutThreshold->addWidget(lblTo);

        spinTo = new QSpinBox(horizontalLayoutWidget_11);
        spinTo->setObjectName(QString::fromUtf8("spinTo"));
        spinTo->setEnabled(false);
        sizePolicy3.setHeightForWidth(spinTo->sizePolicy().hasHeightForWidth());
        spinTo->setSizePolicy(sizePolicy3);
        spinTo->setMaximumSize(QSize(16777215, 16777215));
        spinTo->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinTo->setMinimum(-16777215);
        spinTo->setMaximum(16777215);
        spinTo->setValue(0);

        layoutThreshold->addWidget(spinTo);

        stackedWidget->addWidget(page_8);
        page_10 = new QWidget();
        page_10->setObjectName(QString::fromUtf8("page_10"));
        horizontalLayoutWidget_9 = new QWidget(page_10);
        horizontalLayoutWidget_9->setObjectName(QString::fromUtf8("horizontalLayoutWidget_9"));
        horizontalLayoutWidget_9->setGeometry(QRect(25, 5, 128, 26));
        layoutSave_7 = new QHBoxLayout(horizontalLayoutWidget_9);
        layoutSave_7->setSpacing(0);
        layoutSave_7->setObjectName(QString::fromUtf8("layoutSave_7"));
        layoutSave_7->setContentsMargins(0, 0, 0, 0);
        chkStatistics = new QCheckBox(horizontalLayoutWidget_9);
        chkStatistics->setObjectName(QString::fromUtf8("chkStatistics"));
        chkStatistics->setEnabled(true);
        sizePolicy1.setHeightForWidth(chkStatistics->sizePolicy().hasHeightForWidth());
        chkStatistics->setSizePolicy(sizePolicy1);

        layoutSave_7->addWidget(chkStatistics);

        stackedWidget->addWidget(page_10);
        pageHistogram = new QWidget();
        pageHistogram->setObjectName(QString::fromUtf8("pageHistogram"));
        pageHistogram->setEnabled(true);
        horizontalLayoutWidget_13 = new QWidget(pageHistogram);
        horizontalLayoutWidget_13->setObjectName(QString::fromUtf8("horizontalLayoutWidget_13"));
        horizontalLayoutWidget_13->setGeometry(QRect(25, 5, 311, 26));
        layoutThreshold_3 = new QHBoxLayout(horizontalLayoutWidget_13);
        layoutThreshold_3->setSpacing(1);
        layoutThreshold_3->setObjectName(QString::fromUtf8("layoutThreshold_3"));
        layoutThreshold_3->setContentsMargins(0, 0, 0, 0);
        lblHistFrom = new QLabel(horizontalLayoutWidget_13);
        lblHistFrom->setObjectName(QString::fromUtf8("lblHistFrom"));
        lblHistFrom->setEnabled(true);
        sizePolicy2.setHeightForWidth(lblHistFrom->sizePolicy().hasHeightForWidth());
        lblHistFrom->setSizePolicy(sizePolicy2);

        layoutThreshold_3->addWidget(lblHistFrom);

        spinHistFrom = new QSpinBox(horizontalLayoutWidget_13);
        spinHistFrom->setObjectName(QString::fromUtf8("spinHistFrom"));
        spinHistFrom->setEnabled(true);
        sizePolicy3.setHeightForWidth(spinHistFrom->sizePolicy().hasHeightForWidth());
        spinHistFrom->setSizePolicy(sizePolicy3);
        spinHistFrom->setMaximumSize(QSize(16777215, 16777215));
        spinHistFrom->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinHistFrom->setMinimum(0);
        spinHistFrom->setMaximum(16777215);
        spinHistFrom->setValue(0);

        layoutThreshold_3->addWidget(spinHistFrom);

        lblHistTo = new QLabel(horizontalLayoutWidget_13);
        lblHistTo->setObjectName(QString::fromUtf8("lblHistTo"));
        lblHistTo->setEnabled(true);
        sizePolicy2.setHeightForWidth(lblHistTo->sizePolicy().hasHeightForWidth());
        lblHistTo->setSizePolicy(sizePolicy2);

        layoutThreshold_3->addWidget(lblHistTo);

        spinHistTo = new QSpinBox(horizontalLayoutWidget_13);
        spinHistTo->setObjectName(QString::fromUtf8("spinHistTo"));
        spinHistTo->setEnabled(true);
        sizePolicy3.setHeightForWidth(spinHistTo->sizePolicy().hasHeightForWidth());
        spinHistTo->setSizePolicy(sizePolicy3);
        spinHistTo->setMaximumSize(QSize(16777215, 16777215));
        spinHistTo->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinHistTo->setMinimum(-16777215);
        spinHistTo->setMaximum(16777215);
        spinHistTo->setValue(10000);

        layoutThreshold_3->addWidget(spinHistTo);

        lblHistSize = new QLabel(horizontalLayoutWidget_13);
        lblHistSize->setObjectName(QString::fromUtf8("lblHistSize"));
        lblHistSize->setEnabled(true);
        sizePolicy2.setHeightForWidth(lblHistSize->sizePolicy().hasHeightForWidth());
        lblHistSize->setSizePolicy(sizePolicy2);

        layoutThreshold_3->addWidget(lblHistSize);

        spinHistSize = new QSpinBox(horizontalLayoutWidget_13);
        spinHistSize->setObjectName(QString::fromUtf8("spinHistSize"));
        spinHistSize->setEnabled(true);
        sizePolicy3.setHeightForWidth(spinHistSize->sizePolicy().hasHeightForWidth());
        spinHistSize->setSizePolicy(sizePolicy3);
        spinHistSize->setMaximumSize(QSize(16777215, 16777215));
        spinHistSize->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinHistSize->setMinimum(-16777215);
        spinHistSize->setMaximum(16777215);
        spinHistSize->setValue(1000);

        layoutThreshold_3->addWidget(spinHistSize);

        stackedWidget->addWidget(pageHistogram);
        boxSave = new QGroupBox(TabPlotObject);
        boxSave->setObjectName(QString::fromUtf8("boxSave"));
        boxSave->setGeometry(QRect(410, 70, 351, 51));
        boxSave->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        boxSave->setFlat(false);
        horizontalLayoutWidget = new QWidget(boxSave);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(10, 20, 227, 26));
        layoutSave = new QHBoxLayout(horizontalLayoutWidget);
        layoutSave->setSpacing(0);
        layoutSave->setObjectName(QString::fromUtf8("layoutSave"));
        layoutSave->setContentsMargins(0, 0, 0, 0);
        chkSaveAll = new QCheckBox(horizontalLayoutWidget);
        chkSaveAll->setObjectName(QString::fromUtf8("chkSaveAll"));
        chkSaveAll->setEnabled(true);
        sizePolicy1.setHeightForWidth(chkSaveAll->sizePolicy().hasHeightForWidth());
        chkSaveAll->setSizePolicy(sizePolicy1);

        layoutSave->addWidget(chkSaveAll);

        btnSave = new QPushButton(boxSave);
        btnSave->setObjectName(QString::fromUtf8("btnSave"));
        btnSave->setGeometry(QRect(264, 19, 76, 25));
        sizePolicy1.setHeightForWidth(btnSave->sizePolicy().hasHeightForWidth());
        btnSave->setSizePolicy(sizePolicy1);
        btnSave->setMaximumSize(QSize(16777215, 16777215));
        QPalette palette5;
        palette5.setBrush(QPalette::Active, QPalette::Shadow, brush1);
        palette5.setBrush(QPalette::Inactive, QPalette::Shadow, brush1);
        palette5.setBrush(QPalette::Disabled, QPalette::Shadow, brush1);
        btnSave->setPalette(palette5);
        btnSave->setFocusPolicy(Qt::NoFocus);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/images/save.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnSave->setIcon(icon1);
        btnSave->setIconSize(QSize(16, 16));
        boxScan = new QGroupBox(TabPlotObject);
        boxScan->setObjectName(QString::fromUtf8("boxScan"));
        boxScan->setGeometry(QRect(410, 5, 351, 51));
        boxScan->setCheckable(true);
        boxScan->setChecked(false);
        horizontalLayoutWidget_3 = new QWidget(boxScan);
        horizontalLayoutWidget_3->setObjectName(QString::fromUtf8("horizontalLayoutWidget_3"));
        horizontalLayoutWidget_3->setGeometry(QRect(10, 20, 342, 26));
        horizontalLayout_2 = new QHBoxLayout(horizontalLayoutWidget_3);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        radioLevel0 = new QRadioButton(horizontalLayoutWidget_3);
        radioLevel0->setObjectName(QString::fromUtf8("radioLevel0"));
        sizePolicy1.setHeightForWidth(radioLevel0->sizePolicy().hasHeightForWidth());
        radioLevel0->setSizePolicy(sizePolicy1);
        radioLevel0->setChecked(true);

        horizontalLayout_2->addWidget(radioLevel0);

        horizontalSpacer_6 = new QSpacerItem(7, 10, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_6);

        radioLevel1 = new QRadioButton(horizontalLayoutWidget_3);
        radioLevel1->setObjectName(QString::fromUtf8("radioLevel1"));
        sizePolicy1.setHeightForWidth(radioLevel1->sizePolicy().hasHeightForWidth());
        radioLevel1->setSizePolicy(sizePolicy1);

        horizontalLayout_2->addWidget(radioLevel1);

        horizontalSpacer_7 = new QSpacerItem(7, 10, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_7);

        radioFileIndex = new QRadioButton(horizontalLayoutWidget_3);
        radioFileIndex->setObjectName(QString::fromUtf8("radioFileIndex"));
        sizePolicy1.setHeightForWidth(radioFileIndex->sizePolicy().hasHeightForWidth());
        radioFileIndex->setSizePolicy(sizePolicy1);

        horizontalLayout_2->addWidget(radioFileIndex);

        horizontalSpacer_8 = new QSpacerItem(7, 10, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_8);

        radioAllFrames = new QRadioButton(horizontalLayoutWidget_3);
        radioAllFrames->setObjectName(QString::fromUtf8("radioAllFrames"));
        sizePolicy1.setHeightForWidth(radioAllFrames->sizePolicy().hasHeightForWidth());
        radioAllFrames->setSizePolicy(sizePolicy1);
        radioAllFrames->setChecked(false);

        horizontalLayout_2->addWidget(radioAllFrames);

        boxPlotAxis = new QGroupBox(TabPlotObject);
        boxPlotAxis->setObjectName(QString::fromUtf8("boxPlotAxis"));
        boxPlotAxis->setGeometry(QRect(15, 200, 746, 141));
        boxPlotAxis->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        boxPlotAxis->setFlat(false);
        gridLayoutWidget = new QWidget(boxPlotAxis);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(10, 15, 726, 121));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setSizeConstraint(QLayout::SetNoConstraint);
        gridLayout->setHorizontalSpacing(0);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        chkTitle = new QCheckBox(gridLayoutWidget);
        chkTitle->setObjectName(QString::fromUtf8("chkTitle"));

        gridLayout->addWidget(chkTitle, 0, 0, 1, 1);

        dispTitle = new QLineEdit(gridLayoutWidget);
        dispTitle->setObjectName(QString::fromUtf8("dispTitle"));
        sizePolicy3.setHeightForWidth(dispTitle->sizePolicy().hasHeightForWidth());
        dispTitle->setSizePolicy(sizePolicy3);
        dispTitle->setMinimumSize(QSize(250, 0));
        dispTitle->setEchoMode(QLineEdit::Normal);
        dispTitle->setReadOnly(false);

        gridLayout->addWidget(dispTitle, 0, 1, 1, 7);

        dispZMax = new QLineEdit(gridLayoutWidget);
        dispZMax->setObjectName(QString::fromUtf8("dispZMax"));
        dispZMax->setEnabled(true);
        QSizePolicy sizePolicy5(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(dispZMax->sizePolicy().hasHeightForWidth());
        dispZMax->setSizePolicy(sizePolicy5);
        dispZMax->setMinimumSize(QSize(50, 0));

        gridLayout->addWidget(dispZMax, 3, 7, 1, 1);

        chkZMax = new QCheckBox(gridLayoutWidget);
        chkZMax->setObjectName(QString::fromUtf8("chkZMax"));
        sizePolicy1.setHeightForWidth(chkZMax->sizePolicy().hasHeightForWidth());
        chkZMax->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(chkZMax, 3, 6, 1, 1);

        dispYMax = new QLineEdit(gridLayoutWidget);
        dispYMax->setObjectName(QString::fromUtf8("dispYMax"));
        sizePolicy5.setHeightForWidth(dispYMax->sizePolicy().hasHeightForWidth());
        dispYMax->setSizePolicy(sizePolicy5);
        dispYMax->setMinimumSize(QSize(50, 0));

        gridLayout->addWidget(dispYMax, 2, 7, 1, 1);

        chkYMax = new QCheckBox(gridLayoutWidget);
        chkYMax->setObjectName(QString::fromUtf8("chkYMax"));
        sizePolicy1.setHeightForWidth(chkYMax->sizePolicy().hasHeightForWidth());
        chkYMax->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(chkYMax, 2, 6, 1, 1);

        chkXMax = new QCheckBox(gridLayoutWidget);
        chkXMax->setObjectName(QString::fromUtf8("chkXMax"));
        sizePolicy1.setHeightForWidth(chkXMax->sizePolicy().hasHeightForWidth());
        chkXMax->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(chkXMax, 1, 6, 1, 1);

        dispXMax = new QLineEdit(gridLayoutWidget);
        dispXMax->setObjectName(QString::fromUtf8("dispXMax"));
        sizePolicy5.setHeightForWidth(dispXMax->sizePolicy().hasHeightForWidth());
        dispXMax->setSizePolicy(sizePolicy5);
        dispXMax->setMinimumSize(QSize(50, 0));

        gridLayout->addWidget(dispXMax, 1, 7, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_2, 1, 5, 1, 1);

        chkZMin = new QCheckBox(gridLayoutWidget);
        chkZMin->setObjectName(QString::fromUtf8("chkZMin"));
        sizePolicy1.setHeightForWidth(chkZMin->sizePolicy().hasHeightForWidth());
        chkZMin->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(chkZMin, 3, 3, 1, 1);

        dispZMin = new QLineEdit(gridLayoutWidget);
        dispZMin->setObjectName(QString::fromUtf8("dispZMin"));
        sizePolicy5.setHeightForWidth(dispZMin->sizePolicy().hasHeightForWidth());
        dispZMin->setSizePolicy(sizePolicy5);
        dispZMin->setMinimumSize(QSize(50, 0));

        gridLayout->addWidget(dispZMin, 3, 4, 1, 1);

        chkXMin = new QCheckBox(gridLayoutWidget);
        chkXMin->setObjectName(QString::fromUtf8("chkXMin"));
        sizePolicy1.setHeightForWidth(chkXMin->sizePolicy().hasHeightForWidth());
        chkXMin->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(chkXMin, 1, 3, 1, 1);

        dispXMin = new QLineEdit(gridLayoutWidget);
        dispXMin->setObjectName(QString::fromUtf8("dispXMin"));
        sizePolicy5.setHeightForWidth(dispXMin->sizePolicy().hasHeightForWidth());
        dispXMin->setSizePolicy(sizePolicy5);
        dispXMin->setMinimumSize(QSize(50, 0));
        dispXMin->setInputMethodHints(Qt::ImhDigitsOnly);

        gridLayout->addWidget(dispXMin, 1, 4, 1, 1);

        chkYMin = new QCheckBox(gridLayoutWidget);
        chkYMin->setObjectName(QString::fromUtf8("chkYMin"));
        sizePolicy1.setHeightForWidth(chkYMin->sizePolicy().hasHeightForWidth());
        chkYMin->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(chkYMin, 2, 3, 1, 1);

        dispYMin = new QLineEdit(gridLayoutWidget);
        dispYMin->setObjectName(QString::fromUtf8("dispYMin"));
        sizePolicy5.setHeightForWidth(dispYMin->sizePolicy().hasHeightForWidth());
        dispYMin->setSizePolicy(sizePolicy5);
        dispYMin->setMinimumSize(QSize(50, 0));

        gridLayout->addWidget(dispYMin, 2, 4, 1, 1);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_9, 1, 2, 1, 1);

        chkXAxis = new QCheckBox(gridLayoutWidget);
        chkXAxis->setObjectName(QString::fromUtf8("chkXAxis"));
        sizePolicy1.setHeightForWidth(chkXAxis->sizePolicy().hasHeightForWidth());
        chkXAxis->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(chkXAxis, 1, 0, 1, 1);

        dispXAxis = new QLineEdit(gridLayoutWidget);
        dispXAxis->setObjectName(QString::fromUtf8("dispXAxis"));
        sizePolicy3.setHeightForWidth(dispXAxis->sizePolicy().hasHeightForWidth());
        dispXAxis->setSizePolicy(sizePolicy3);
        dispXAxis->setMinimumSize(QSize(250, 0));
        dispXAxis->setInputMethodHints(Qt::ImhDigitsOnly);

        gridLayout->addWidget(dispXAxis, 1, 1, 1, 1);

        chkYAxis = new QCheckBox(gridLayoutWidget);
        chkYAxis->setObjectName(QString::fromUtf8("chkYAxis"));
        sizePolicy1.setHeightForWidth(chkYAxis->sizePolicy().hasHeightForWidth());
        chkYAxis->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(chkYAxis, 2, 0, 1, 1);

        dispYAxis = new QLineEdit(gridLayoutWidget);
        dispYAxis->setObjectName(QString::fromUtf8("dispYAxis"));
        sizePolicy3.setHeightForWidth(dispYAxis->sizePolicy().hasHeightForWidth());
        dispYAxis->setSizePolicy(sizePolicy3);
        dispYAxis->setMinimumSize(QSize(20, 0));
        dispYAxis->setInputMethodHints(Qt::ImhDigitsOnly);

        gridLayout->addWidget(dispYAxis, 2, 1, 1, 1);

        chkZAxis = new QCheckBox(gridLayoutWidget);
        chkZAxis->setObjectName(QString::fromUtf8("chkZAxis"));
        sizePolicy1.setHeightForWidth(chkZAxis->sizePolicy().hasHeightForWidth());
        chkZAxis->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(chkZAxis, 3, 0, 1, 1);

        dispZAxis = new QLineEdit(gridLayoutWidget);
        dispZAxis->setObjectName(QString::fromUtf8("dispZAxis"));
        sizePolicy3.setHeightForWidth(dispZAxis->sizePolicy().hasHeightForWidth());
        dispZAxis->setSizePolicy(sizePolicy3);
        dispZAxis->setMinimumSize(QSize(20, 0));
        dispZAxis->setInputMethodHints(Qt::ImhDigitsOnly);

        gridLayout->addWidget(dispZAxis, 3, 1, 1, 1);

        groupBox_3 = new QGroupBox(TabPlotObject);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setEnabled(true);
        groupBox_3->setGeometry(QRect(15, 5, 371, 51));
        groupBox_3->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        groupBox_3->setFlat(false);
        gridLayoutWidget_6 = new QWidget(groupBox_3);
        gridLayoutWidget_6->setObjectName(QString::fromUtf8("gridLayoutWidget_6"));
        gridLayoutWidget_6->setGeometry(QRect(10, 20, 358, 26));
        gridLayout_6 = new QGridLayout(gridLayoutWidget_6);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        gridLayout_6->setContentsMargins(0, 0, 0, 0);
        radioNoPlot = new QRadioButton(gridLayoutWidget_6);
        radioNoPlot->setObjectName(QString::fromUtf8("radioNoPlot"));
        sizePolicy1.setHeightForWidth(radioNoPlot->sizePolicy().hasHeightForWidth());
        radioNoPlot->setSizePolicy(sizePolicy1);

        gridLayout_6->addWidget(radioNoPlot, 0, 0, 1, 1);

        radioDataGraph = new QRadioButton(gridLayoutWidget_6);
        radioDataGraph->setObjectName(QString::fromUtf8("radioDataGraph"));
        sizePolicy1.setHeightForWidth(radioDataGraph->sizePolicy().hasHeightForWidth());
        radioDataGraph->setSizePolicy(sizePolicy1);
        radioDataGraph->setChecked(true);

        gridLayout_6->addWidget(radioDataGraph, 0, 4, 1, 1);

        radioHistogram = new QRadioButton(gridLayoutWidget_6);
        radioHistogram->setObjectName(QString::fromUtf8("radioHistogram"));
        radioHistogram->setEnabled(true);
        sizePolicy1.setHeightForWidth(radioHistogram->sizePolicy().hasHeightForWidth());
        radioHistogram->setSizePolicy(sizePolicy1);

        gridLayout_6->addWidget(radioHistogram, 0, 2, 1, 1);

        horizontalSpacer_17 = new QSpacerItem(45, 10, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_6->addItem(horizontalSpacer_17, 0, 1, 1, 1);

        horizontalSpacer_18 = new QSpacerItem(45, 10, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_6->addItem(horizontalSpacer_18, 0, 3, 1, 1);

        boxFrequency = new QGroupBox(TabPlotObject);
        boxFrequency->setObjectName(QString::fromUtf8("boxFrequency"));
        boxFrequency->setGeometry(QRect(15, 135, 371, 49));
        boxFrequency->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        boxFrequency->setFlat(false);
        horizontalLayoutWidget_4 = new QWidget(boxFrequency);
        horizontalLayoutWidget_4->setObjectName(QString::fromUtf8("horizontalLayoutWidget_4"));
        horizontalLayoutWidget_4->setGeometry(QRect(10, 16, 351, 26));
        layoutSave_2 = new QHBoxLayout(horizontalLayoutWidget_4);
        layoutSave_2->setSpacing(0);
        layoutSave_2->setObjectName(QString::fromUtf8("layoutSave_2"));
        layoutSave_2->setContentsMargins(0, 0, 0, 0);
        comboFrequency = new QComboBox(horizontalLayoutWidget_4);
        comboFrequency->setObjectName(QString::fromUtf8("comboFrequency"));
        QSizePolicy sizePolicy6(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(comboFrequency->sizePolicy().hasHeightForWidth());
        comboFrequency->setSizePolicy(sizePolicy6);
        comboFrequency->setMinimumSize(QSize(150, 0));
        comboFrequency->setMaximumSize(QSize(16777215, 16777215));
        QPalette palette6;
        QBrush brush2(QColor(11, 11, 11, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette6.setBrush(QPalette::Active, QPalette::ButtonText, brush2);
        palette6.setBrush(QPalette::Active, QPalette::Shadow, brush1);
        palette6.setBrush(QPalette::Inactive, QPalette::ButtonText, brush2);
        palette6.setBrush(QPalette::Inactive, QPalette::Shadow, brush1);
        QBrush brush3(QColor(119, 119, 119, 255));
        brush3.setStyle(Qt::SolidPattern);
        palette6.setBrush(QPalette::Disabled, QPalette::ButtonText, brush3);
        palette6.setBrush(QPalette::Disabled, QPalette::Shadow, brush1);
        comboFrequency->setPalette(palette6);

        layoutSave_2->addWidget(comboFrequency);

        horizontalSpacer_14 = new QSpacerItem(80, 20, QSizePolicy::Preferred, QSizePolicy::Minimum);

        layoutSave_2->addItem(horizontalSpacer_14);

        stackWidget = new QWidget(horizontalLayoutWidget_4);
        stackWidget->setObjectName(QString::fromUtf8("stackWidget"));
        QSizePolicy sizePolicy7(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy7.setHorizontalStretch(0);
        sizePolicy7.setVerticalStretch(0);
        sizePolicy7.setHeightForWidth(stackWidget->sizePolicy().hasHeightForWidth());
        stackWidget->setSizePolicy(sizePolicy7);
        stackWidget->setMaximumSize(QSize(140, 21));

        layoutSave_2->addWidget(stackWidget);

        boxSnapshot = new QGroupBox(TabPlotObject);
        boxSnapshot->setObjectName(QString::fromUtf8("boxSnapshot"));
        boxSnapshot->setEnabled(true);
        boxSnapshot->setGeometry(QRect(410, 135, 351, 51));
        boxSnapshot->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        boxSnapshot->setFlat(false);
        gridLayoutWidget_4 = new QWidget(boxSnapshot);
        gridLayoutWidget_4->setObjectName(QString::fromUtf8("gridLayoutWidget_4"));
        gridLayoutWidget_4->setGeometry(QRect(10, 15, 331, 31));
        gridLayout_4 = new QGridLayout(gridLayoutWidget_4);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        gridLayout_4->setContentsMargins(0, 0, 0, 0);
        btnClone = new QPushButton(gridLayoutWidget_4);
        btnClone->setObjectName(QString::fromUtf8("btnClone"));
        QSizePolicy sizePolicy8(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy8.setHorizontalStretch(0);
        sizePolicy8.setVerticalStretch(0);
        sizePolicy8.setHeightForWidth(btnClone->sizePolicy().hasHeightForWidth());
        btnClone->setSizePolicy(sizePolicy8);
        QPalette palette7;
        palette7.setBrush(QPalette::Active, QPalette::Shadow, brush1);
        palette7.setBrush(QPalette::Inactive, QPalette::Shadow, brush1);
        palette7.setBrush(QPalette::Disabled, QPalette::Shadow, brush1);
        btnClone->setPalette(palette7);
        btnClone->setFocusPolicy(Qt::NoFocus);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/images/new.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnClone->setIcon(icon2);
        btnClone->setFlat(false);

        gridLayout_4->addWidget(btnClone, 0, 0, 1, 1);

        btnCloseClones = new QPushButton(gridLayoutWidget_4);
        btnCloseClones->setObjectName(QString::fromUtf8("btnCloseClones"));
        sizePolicy8.setHeightForWidth(btnCloseClones->sizePolicy().hasHeightForWidth());
        btnCloseClones->setSizePolicy(sizePolicy8);
        QPalette palette8;
        palette8.setBrush(QPalette::Active, QPalette::Shadow, brush1);
        palette8.setBrush(QPalette::Inactive, QPalette::Shadow, brush1);
        palette8.setBrush(QPalette::Disabled, QPalette::Shadow, brush1);
        btnCloseClones->setPalette(palette8);
        btnCloseClones->setFocusPolicy(Qt::NoFocus);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/images/close.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCloseClones->setIcon(icon3);
        btnCloseClones->setFlat(false);

        gridLayout_4->addWidget(btnCloseClones, 0, 2, 1, 1);

        horizontalSpacer_12 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_4->addItem(horizontalSpacer_12, 0, 1, 1, 1);

        horizontalSpacer_13 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_4->addItem(horizontalSpacer_13, 0, 3, 1, 1);

        btnSaveClones = new QPushButton(gridLayoutWidget_4);
        btnSaveClones->setObjectName(QString::fromUtf8("btnSaveClones"));
        btnSaveClones->setEnabled(true);
        sizePolicy8.setHeightForWidth(btnSaveClones->sizePolicy().hasHeightForWidth());
        btnSaveClones->setSizePolicy(sizePolicy8);
        QPalette palette9;
        palette9.setBrush(QPalette::Active, QPalette::Shadow, brush1);
        palette9.setBrush(QPalette::Inactive, QPalette::Shadow, brush1);
        palette9.setBrush(QPalette::Disabled, QPalette::Shadow, brush1);
        btnSaveClones->setPalette(palette9);
        btnSaveClones->setFocusPolicy(Qt::NoFocus);
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/images/saveAll.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnSaveClones->setIcon(icon4);
        btnSaveClones->setFlat(false);

        gridLayout_4->addWidget(btnSaveClones, 0, 4, 1, 1);

        btnRight = new QPushButton(TabPlotObject);
        btnRight->setObjectName(QString::fromUtf8("btnRight"));
        btnRight->setGeometry(QRect(362, 95, 16, 16));
        sizePolicy1.setHeightForWidth(btnRight->sizePolicy().hasHeightForWidth());
        btnRight->setSizePolicy(sizePolicy1);
        btnRight->setFocusPolicy(Qt::NoFocus);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/images/rightArrow.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnRight->setIcon(icon5);
        btnRight->setIconSize(QSize(16, 16));
        btnRight->setFlat(true);
        btnLeft = new QPushButton(TabPlotObject);
        btnLeft->setObjectName(QString::fromUtf8("btnLeft"));
        btnLeft->setGeometry(QRect(23, 94, 16, 16));
        sizePolicy1.setHeightForWidth(btnLeft->sizePolicy().hasHeightForWidth());
        btnLeft->setSizePolicy(sizePolicy1);
        btnLeft->setFocusPolicy(Qt::NoFocus);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/icons/images/leftArrow.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnLeft->setIcon(icon6);
        btnLeft->setIconSize(QSize(16, 16));
        btnLeft->setFlat(true);
        boxHistogram = new QGroupBox(TabPlotObject);
        boxHistogram->setObjectName(QString::fromUtf8("boxHistogram"));
        boxHistogram->setGeometry(QRect(410, 5, 351, 51));
        boxHistogram->setCheckable(false);
        boxHistogram->setChecked(false);
        horizontalLayoutWidget_15 = new QWidget(boxHistogram);
        horizontalLayoutWidget_15->setObjectName(QString::fromUtf8("horizontalLayoutWidget_15"));
        horizontalLayoutWidget_15->setGeometry(QRect(10, 20, 351, 26));
        horizontalLayout_3 = new QHBoxLayout(horizontalLayoutWidget_15);
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        radioHistIntensity = new QRadioButton(horizontalLayoutWidget_15);
        radioHistIntensity->setObjectName(QString::fromUtf8("radioHistIntensity"));
        sizePolicy1.setHeightForWidth(radioHistIntensity->sizePolicy().hasHeightForWidth());
        radioHistIntensity->setSizePolicy(sizePolicy1);
        radioHistIntensity->setChecked(true);

        horizontalLayout_3->addWidget(radioHistIntensity);

        horizontalSpacer_10 = new QSpacerItem(7, 10, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_10);

        radioHistLevel0 = new QRadioButton(horizontalLayoutWidget_15);
        radioHistLevel0->setObjectName(QString::fromUtf8("radioHistLevel0"));
        sizePolicy1.setHeightForWidth(radioHistLevel0->sizePolicy().hasHeightForWidth());
        radioHistLevel0->setSizePolicy(sizePolicy1);

        horizontalLayout_3->addWidget(radioHistLevel0);

        horizontalSpacer_11 = new QSpacerItem(7, 10, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_11);

        radioHistLevel1 = new QRadioButton(horizontalLayoutWidget_15);
        radioHistLevel1->setObjectName(QString::fromUtf8("radioHistLevel1"));
        sizePolicy1.setHeightForWidth(radioHistLevel1->sizePolicy().hasHeightForWidth());
        radioHistLevel1->setSizePolicy(sizePolicy1);

        horizontalLayout_3->addWidget(radioHistLevel1);

        box1D->raise();
        box2D->raise();
        boxSave->raise();
        boxScan->raise();
        boxPlotAxis->raise();
        groupBox_3->raise();
        boxFrequency->raise();
        boxSnapshot->raise();
        btnRight->raise();
        btnLeft->raise();
        boxHistogram->raise();
        QWidget::setTabOrder(radioNoPlot, radioHistogram);
        QWidget::setTabOrder(radioHistogram, radioDataGraph);
        QWidget::setTabOrder(radioDataGraph, boxScan);
        QWidget::setTabOrder(boxScan, radioLevel0);
        QWidget::setTabOrder(radioLevel0, radioLevel1);
        QWidget::setTabOrder(radioLevel1, radioFileIndex);
        QWidget::setTabOrder(radioFileIndex, radioAllFrames);
        QWidget::setTabOrder(radioAllFrames, chkInterpolate);
        QWidget::setTabOrder(chkInterpolate, chkContour);
        QWidget::setTabOrder(chkContour, chkLogz);
        QWidget::setTabOrder(chkLogz, chkSaveAll);
        QWidget::setTabOrder(chkSaveAll, comboFrequency);
        QWidget::setTabOrder(comboFrequency, chkTitle);
        QWidget::setTabOrder(chkTitle, dispTitle);
        QWidget::setTabOrder(dispTitle, chkXAxis);
        QWidget::setTabOrder(chkXAxis, dispXAxis);
        QWidget::setTabOrder(dispXAxis, chkXMin);
        QWidget::setTabOrder(chkXMin, dispXMin);
        QWidget::setTabOrder(dispXMin, chkXMax);
        QWidget::setTabOrder(chkXMax, dispXMax);
        QWidget::setTabOrder(dispXMax, chkYAxis);
        QWidget::setTabOrder(chkYAxis, dispYAxis);
        QWidget::setTabOrder(dispYAxis, chkYMin);
        QWidget::setTabOrder(chkYMin, dispYMin);
        QWidget::setTabOrder(dispYMin, chkYMax);
        QWidget::setTabOrder(chkYMax, dispYMax);
        QWidget::setTabOrder(dispYMax, chkZAxis);
        QWidget::setTabOrder(chkZAxis, dispZAxis);
        QWidget::setTabOrder(dispZAxis, chkZMin);
        QWidget::setTabOrder(chkZMin, dispZMin);
        QWidget::setTabOrder(dispZMin, chkZMax);
        QWidget::setTabOrder(chkZMax, dispZMax);
        QWidget::setTabOrder(dispZMax, spinPersistency);
        QWidget::setTabOrder(spinPersistency, chk1DLog);
        QWidget::setTabOrder(chk1DLog, chkPoints);
        QWidget::setTabOrder(chkPoints, chkLines);
        QWidget::setTabOrder(chkLines, chkSuperimpose);

        retranslateUi(TabPlotObject);

        stackedWidget_2->setCurrentIndex(3);
        stackedWidget->setCurrentIndex(4);
        comboFrequency->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(TabPlotObject);
    } // setupUi

    void retranslateUi(QWidget *TabPlotObject)
    {
        TabPlotObject->setWindowTitle(QApplication::translate("TabPlotObject", "Form", 0, QApplication::UnicodeUTF8));
        box2D->setTitle(QApplication::translate("TabPlotObject", "2D Plot Options 1", 0, QApplication::UnicodeUTF8));
        chkInterpolate->setText(QApplication::translate("TabPlotObject", "Interpolate", 0, QApplication::UnicodeUTF8));
        chkContour->setText(QApplication::translate("TabPlotObject", "Contour", 0, QApplication::UnicodeUTF8));
        chkLogz->setText(QApplication::translate("TabPlotObject", "Log Scale (Z)", 0, QApplication::UnicodeUTF8));
        chkAccumulate_2->setText(QApplication::translate("TabPlotObject", "Accumulate", 0, QApplication::UnicodeUTF8));
        btnResetAccumulate_2->setText(QApplication::translate("TabPlotObject", "Reset Accumulation  ", 0, QApplication::UnicodeUTF8));
        chkPedestal_2->setText(QApplication::translate("TabPlotObject", "Pedestal", 0, QApplication::UnicodeUTF8));
        btnRecalPedestal_2->setText(QApplication::translate("TabPlotObject", "Recalculate Pedestal  ", 0, QApplication::UnicodeUTF8));
        chkBinary_2->setText(QApplication::translate("TabPlotObject", "Binary", 0, QApplication::UnicodeUTF8));
        lblFrom_2->setText(QApplication::translate("TabPlotObject", "from ", 0, QApplication::UnicodeUTF8));
        lblTo_2->setText(QApplication::translate("TabPlotObject", " to ", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        chkStatistics_2->setToolTip(QApplication::translate("TabPlotObject", "<nobr>\n"
"Displays minimum, maximum and sum of values for each plot.\n"
"<nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        chkStatistics_2->setText(QApplication::translate("TabPlotObject", "Display Statistics", 0, QApplication::UnicodeUTF8));
        lblHistFrom_2->setText(QApplication::translate("TabPlotObject", "from ", 0, QApplication::UnicodeUTF8));
        lblHistTo_2->setText(QApplication::translate("TabPlotObject", " to ", 0, QApplication::UnicodeUTF8));
        lblHistSize_2->setText(QApplication::translate("TabPlotObject", " size ", 0, QApplication::UnicodeUTF8));
        box1D->setTitle(QApplication::translate("TabPlotObject", "1D Plot Options 1", 0, QApplication::UnicodeUTF8));
        chkSuperimpose->setText(QApplication::translate("TabPlotObject", "Superimpose", 0, QApplication::UnicodeUTF8));
        lblPersistency->setText(QApplication::translate("TabPlotObject", "Persistency:", 0, QApplication::UnicodeUTF8));
        chk1DLog->setText(QApplication::translate("TabPlotObject", "Log Scale (Y)", 0, QApplication::UnicodeUTF8));
        chkPoints->setText(QApplication::translate("TabPlotObject", "Points", 0, QApplication::UnicodeUTF8));
        chkLines->setText(QApplication::translate("TabPlotObject", "Lines", 0, QApplication::UnicodeUTF8));
        btnResetAccumulate->setText(QApplication::translate("TabPlotObject", "Reset Accumulation  ", 0, QApplication::UnicodeUTF8));
        chkAccumulate->setText(QApplication::translate("TabPlotObject", "Accumulate", 0, QApplication::UnicodeUTF8));
        chkPedestal->setText(QApplication::translate("TabPlotObject", "Pedestal", 0, QApplication::UnicodeUTF8));
        btnRecalPedestal->setText(QApplication::translate("TabPlotObject", "Recalculate Pedestal  ", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        page_8->setToolTip(QApplication::translate("TabPlotObject", "<nobr>\n"
"All values between <b>from</b> and <b>to</b> will be reset to 1, others to 0.\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        chkBinary->setToolTip(QApplication::translate("TabPlotObject", "<nobr>\n"
"All values between <b>from</b> and <b>to</b> will be reset to 1, others to 0.\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        chkBinary->setText(QApplication::translate("TabPlotObject", "Binary", 0, QApplication::UnicodeUTF8));
        lblFrom->setText(QApplication::translate("TabPlotObject", "from ", 0, QApplication::UnicodeUTF8));
        lblTo->setText(QApplication::translate("TabPlotObject", " to ", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        chkStatistics->setToolTip(QApplication::translate("TabPlotObject", "<nobr>\n"
"Displays minimum, maximum and sum of values for each plot.\n"
"<nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        chkStatistics->setText(QApplication::translate("TabPlotObject", "Display Statistics", 0, QApplication::UnicodeUTF8));
        lblHistFrom->setText(QApplication::translate("TabPlotObject", "from ", 0, QApplication::UnicodeUTF8));
        lblHistTo->setText(QApplication::translate("TabPlotObject", " to ", 0, QApplication::UnicodeUTF8));
        lblHistSize->setText(QApplication::translate("TabPlotObject", " size ", 0, QApplication::UnicodeUTF8));
        boxSave->setTitle(QApplication::translate("TabPlotObject", "Save Image", 0, QApplication::UnicodeUTF8));
        chkSaveAll->setText(QApplication::translate("TabPlotObject", "Save All with Automatic File Name", 0, QApplication::UnicodeUTF8));
        btnSave->setText(QApplication::translate("TabPlotObject", "Save  ", 0, QApplication::UnicodeUTF8));
        boxScan->setTitle(QApplication::translate("TabPlotObject", "2D Scan  -  Y Axis Values", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        radioLevel0->setToolTip(QApplication::translate("TabPlotObject", "<nobr>Enabled only when there is a Scan Level 0</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        radioLevel0->setText(QApplication::translate("TabPlotObject", "Level 0", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        radioLevel1->setToolTip(QApplication::translate("TabPlotObject", "<nobr>Enabled only when there is a Scan Level 1</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        radioLevel1->setText(QApplication::translate("TabPlotObject", "Level 1", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        radioFileIndex->setToolTip(QApplication::translate("TabPlotObject", "<nobr>Enabled only when there is a Scan Level 0 or a Scan Level 1, not both</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        radioFileIndex->setText(QApplication::translate("TabPlotObject", "Frame Index", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        radioAllFrames->setToolTip(QApplication::translate("TabPlotObject", "<nobr>Disabled only for Angle Plots, Moench and Eiger Detectors</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        radioAllFrames->setText(QApplication::translate("TabPlotObject", "All Frames", 0, QApplication::UnicodeUTF8));
        boxPlotAxis->setTitle(QApplication::translate("TabPlotObject", "Plot Axis", 0, QApplication::UnicodeUTF8));
        chkTitle->setText(QApplication::translate("TabPlotObject", "Title Prefix:", 0, QApplication::UnicodeUTF8));
        chkZMax->setText(QApplication::translate("TabPlotObject", "Z Max:", 0, QApplication::UnicodeUTF8));
        chkYMax->setText(QApplication::translate("TabPlotObject", "Y Max:", 0, QApplication::UnicodeUTF8));
        chkXMax->setText(QApplication::translate("TabPlotObject", "X Max:", 0, QApplication::UnicodeUTF8));
        chkZMin->setText(QApplication::translate("TabPlotObject", "Z Min:", 0, QApplication::UnicodeUTF8));
        chkXMin->setText(QApplication::translate("TabPlotObject", "X Min:", 0, QApplication::UnicodeUTF8));
        chkYMin->setText(QApplication::translate("TabPlotObject", "Y Min:", 0, QApplication::UnicodeUTF8));
        chkXAxis->setText(QApplication::translate("TabPlotObject", "X Axis:", 0, QApplication::UnicodeUTF8));
        chkYAxis->setText(QApplication::translate("TabPlotObject", "Y Axis:", 0, QApplication::UnicodeUTF8));
        chkZAxis->setText(QApplication::translate("TabPlotObject", "Z Axis:", 0, QApplication::UnicodeUTF8));
        groupBox_3->setTitle(QApplication::translate("TabPlotObject", "Plot Arguments", 0, QApplication::UnicodeUTF8));
        radioNoPlot->setText(QApplication::translate("TabPlotObject", "No Plot", 0, QApplication::UnicodeUTF8));
        radioDataGraph->setText(QApplication::translate("TabPlotObject", "Data Graph", 0, QApplication::UnicodeUTF8));
        radioHistogram->setText(QApplication::translate("TabPlotObject", "Histogram", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        boxFrequency->setToolTip(QApplication::translate("TabPlotObject", "<nobr>\n"
"Interval between plots has 2 modes. A condition to be satisfied, in order to avoid losing images:\n"
"</nobr><br><br><nobr>\n"
"<b>Time Interval</b>: (Acquisition Period) * (nth Image) >= 250ms.\n"
"</nobr><br><nobr>\n"
"<b>Every nth Image</b>: minimum of 250ms.\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        boxFrequency->setTitle(QApplication::translate("TabPlotObject", "Interval between Plots", 0, QApplication::UnicodeUTF8));
        comboFrequency->clear();
        comboFrequency->insertItems(0, QStringList()
         << QApplication::translate("TabPlotObject", "Time Interval", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabPlotObject", "Every nth Image", 0, QApplication::UnicodeUTF8)
        );
        boxSnapshot->setTitle(QApplication::translate("TabPlotObject", "Snapshot", 0, QApplication::UnicodeUTF8));
        btnClone->setText(QApplication::translate("TabPlotObject", "Create  ", 0, QApplication::UnicodeUTF8));
        btnCloseClones->setText(QApplication::translate("TabPlotObject", "Close All  ", 0, QApplication::UnicodeUTF8));
        btnSaveClones->setText(QApplication::translate("TabPlotObject", "Save All  ", 0, QApplication::UnicodeUTF8));
        btnRight->setText(QString());
        btnLeft->setText(QString());
        boxHistogram->setTitle(QApplication::translate("TabPlotObject", "Histogram  -  X Axis Values", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        radioHistIntensity->setToolTip(QApplication::translate("TabPlotObject", "<nobr>Enabled only when there is a Scan Level 0</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        radioHistIntensity->setText(QApplication::translate("TabPlotObject", "Intensity", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        radioHistLevel0->setToolTip(QApplication::translate("TabPlotObject", "<nobr>Enabled only when there is a Scan Level 1</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        radioHistLevel0->setText(QApplication::translate("TabPlotObject", "Level 0", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        radioHistLevel1->setToolTip(QApplication::translate("TabPlotObject", "<nobr>Enabled only when there is a Scan Level 0 or a Scan Level 1, not both</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        radioHistLevel1->setText(QApplication::translate("TabPlotObject", "Level 1", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TabPlotObject: public Ui_TabPlotObject {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORM_TAB_PLOT_H
