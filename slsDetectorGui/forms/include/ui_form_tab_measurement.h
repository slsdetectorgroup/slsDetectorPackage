/********************************************************************************
** Form generated from reading UI file 'form_tab_measurement.ui'
**
** Created: Thu Jul 13 14:40:29 2017
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORM_TAB_MEASUREMENT_H
#define UI_FORM_TAB_MEASUREMENT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TabMeasurementObject
{
public:
    QGroupBox *boxProgress;
    QProgressBar *progressBar;
    QLabel *label;
    QLabel *lblProgressIndex;
    QLabel *lblCurrentMeasurement;
    QLabel *label_2;
    QPushButton *btnStartStop;
    QFrame *frameTimeResolved;
    QWidget *gridLayoutWidget;
    QGridLayout *gridTimeResolved;
    QLabel *lblNumTriggers;
    QSpinBox *spinNumTriggers;
    QLabel *lblDelay;
    QDoubleSpinBox *spinDelay;
    QComboBox *comboDelayUnit;
    QLabel *lblNumGates;
    QSpinBox *spinNumGates;
    QLabel *lblNumProbes;
    QSpinBox *spinNumProbes;
    QComboBox *comboTimingMode;
    QLabel *lblNumFrames;
    QSpinBox *spinNumFrames;
    QLabel *lblExpTime;
    QDoubleSpinBox *spinExpTime;
    QComboBox *comboExpUnit;
    QLabel *lblPeriod;
    QDoubleSpinBox *spinPeriod;
    QComboBox *comboPeriodUnit;
    QSpacerItem *horizontalSpacer;
    QLabel *lblTimingMode;
    QFrame *frameNotTimeResolved;
    QWidget *gridLayoutWidget_3;
    QGridLayout *gridLayout;
    QLabel *label_5;
    QSpinBox *spinNumMeasurements;
    QLineEdit *dispFileName;
    QLabel *label_8;
    QSpinBox *spinIndex;
    QCheckBox *chkFile;

    void setupUi(QWidget *TabMeasurementObject)
    {
        if (TabMeasurementObject->objectName().isEmpty())
            TabMeasurementObject->setObjectName(QString::fromUtf8("TabMeasurementObject"));
        TabMeasurementObject->resize(775, 345);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(TabMeasurementObject->sizePolicy().hasHeightForWidth());
        TabMeasurementObject->setSizePolicy(sizePolicy);
        TabMeasurementObject->setMinimumSize(QSize(0, 0));
        TabMeasurementObject->setMaximumSize(QSize(1000, 345));
        boxProgress = new QGroupBox(TabMeasurementObject);
        boxProgress->setObjectName(QString::fromUtf8("boxProgress"));
        boxProgress->setGeometry(QRect(30, 153, 319, 116));
        sizePolicy.setHeightForWidth(boxProgress->sizePolicy().hasHeightForWidth());
        boxProgress->setSizePolicy(sizePolicy);
        boxProgress->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        progressBar = new QProgressBar(boxProgress);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setGeometry(QRect(10, 75, 300, 23));
        progressBar->setValue(24);
        label = new QLabel(boxProgress);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 25, 91, 16));
        lblProgressIndex = new QLabel(boxProgress);
        lblProgressIndex->setObjectName(QString::fromUtf8("lblProgressIndex"));
        lblProgressIndex->setGeometry(QRect(101, 25, 101, 16));
        lblCurrentMeasurement = new QLabel(boxProgress);
        lblCurrentMeasurement->setObjectName(QString::fromUtf8("lblCurrentMeasurement"));
        lblCurrentMeasurement->setGeometry(QRect(150, 45, 76, 16));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lblCurrentMeasurement->sizePolicy().hasHeightForWidth());
        lblCurrentMeasurement->setSizePolicy(sizePolicy1);
        lblCurrentMeasurement->setMinimumSize(QSize(60, 0));
        lblCurrentMeasurement->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label_2 = new QLabel(boxProgress);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 45, 136, 16));
        btnStartStop = new QPushButton(TabMeasurementObject);
        btnStartStop->setObjectName(QString::fromUtf8("btnStartStop"));
        btnStartStop->setGeometry(QRect(30, 289, 319, 31));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(btnStartStop->sizePolicy().hasHeightForWidth());
        btnStartStop->setSizePolicy(sizePolicy2);
        btnStartStop->setFocusPolicy(Qt::NoFocus);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/images/start.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnStartStop->setIcon(icon);
        btnStartStop->setCheckable(true);
        btnStartStop->setFlat(false);
        frameTimeResolved = new QFrame(TabMeasurementObject);
        frameTimeResolved->setObjectName(QString::fromUtf8("frameTimeResolved"));
        frameTimeResolved->setGeometry(QRect(390, 8, 362, 342));
        frameTimeResolved->setFrameShape(QFrame::NoFrame);
        frameTimeResolved->setFrameShadow(QFrame::Plain);
        gridLayoutWidget = new QWidget(frameTimeResolved);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(35, 10, 326, 321));
        gridTimeResolved = new QGridLayout(gridLayoutWidget);
        gridTimeResolved->setObjectName(QString::fromUtf8("gridTimeResolved"));
        gridTimeResolved->setContentsMargins(0, 0, 0, 0);
        lblNumTriggers = new QLabel(gridLayoutWidget);
        lblNumTriggers->setObjectName(QString::fromUtf8("lblNumTriggers"));
        lblNumTriggers->setEnabled(false);

        gridTimeResolved->addWidget(lblNumTriggers, 4, 0, 1, 1);

        spinNumTriggers = new QSpinBox(gridLayoutWidget);
        spinNumTriggers->setObjectName(QString::fromUtf8("spinNumTriggers"));
        spinNumTriggers->setEnabled(false);
        sizePolicy2.setHeightForWidth(spinNumTriggers->sizePolicy().hasHeightForWidth());
        spinNumTriggers->setSizePolicy(sizePolicy2);
        spinNumTriggers->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinNumTriggers->setKeyboardTracking(false);
        spinNumTriggers->setMinimum(0);
        spinNumTriggers->setMaximum(2000000000);
        spinNumTriggers->setValue(1);

        gridTimeResolved->addWidget(spinNumTriggers, 4, 2, 1, 2);

        lblDelay = new QLabel(gridLayoutWidget);
        lblDelay->setObjectName(QString::fromUtf8("lblDelay"));
        lblDelay->setEnabled(false);

        gridTimeResolved->addWidget(lblDelay, 5, 0, 1, 1);

        spinDelay = new QDoubleSpinBox(gridLayoutWidget);
        spinDelay->setObjectName(QString::fromUtf8("spinDelay"));
        spinDelay->setEnabled(false);
        sizePolicy2.setHeightForWidth(spinDelay->sizePolicy().hasHeightForWidth());
        spinDelay->setSizePolicy(sizePolicy2);
        spinDelay->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinDelay->setKeyboardTracking(false);
        spinDelay->setDecimals(9);
        spinDelay->setMaximum(2e+09);
        spinDelay->setValue(0);

        gridTimeResolved->addWidget(spinDelay, 5, 2, 1, 1);

        comboDelayUnit = new QComboBox(gridLayoutWidget);
        comboDelayUnit->setObjectName(QString::fromUtf8("comboDelayUnit"));
        comboDelayUnit->setEnabled(false);
        sizePolicy2.setHeightForWidth(comboDelayUnit->sizePolicy().hasHeightForWidth());
        comboDelayUnit->setSizePolicy(sizePolicy2);
        comboDelayUnit->setLayoutDirection(Qt::LeftToRight);

        gridTimeResolved->addWidget(comboDelayUnit, 5, 3, 1, 1);

        lblNumGates = new QLabel(gridLayoutWidget);
        lblNumGates->setObjectName(QString::fromUtf8("lblNumGates"));
        lblNumGates->setEnabled(false);
        sizePolicy2.setHeightForWidth(lblNumGates->sizePolicy().hasHeightForWidth());
        lblNumGates->setSizePolicy(sizePolicy2);

        gridTimeResolved->addWidget(lblNumGates, 6, 0, 1, 1);

        spinNumGates = new QSpinBox(gridLayoutWidget);
        spinNumGates->setObjectName(QString::fromUtf8("spinNumGates"));
        spinNumGates->setEnabled(false);
        sizePolicy2.setHeightForWidth(spinNumGates->sizePolicy().hasHeightForWidth());
        spinNumGates->setSizePolicy(sizePolicy2);
        spinNumGates->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinNumGates->setKeyboardTracking(false);
        spinNumGates->setMinimum(0);
        spinNumGates->setMaximum(2000000000);
        spinNumGates->setValue(1);

        gridTimeResolved->addWidget(spinNumGates, 6, 2, 1, 2);

        lblNumProbes = new QLabel(gridLayoutWidget);
        lblNumProbes->setObjectName(QString::fromUtf8("lblNumProbes"));
        lblNumProbes->setEnabled(false);
        sizePolicy2.setHeightForWidth(lblNumProbes->sizePolicy().hasHeightForWidth());
        lblNumProbes->setSizePolicy(sizePolicy2);

        gridTimeResolved->addWidget(lblNumProbes, 7, 0, 1, 1);

        spinNumProbes = new QSpinBox(gridLayoutWidget);
        spinNumProbes->setObjectName(QString::fromUtf8("spinNumProbes"));
        spinNumProbes->setEnabled(false);
        sizePolicy2.setHeightForWidth(spinNumProbes->sizePolicy().hasHeightForWidth());
        spinNumProbes->setSizePolicy(sizePolicy2);
        spinNumProbes->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinNumProbes->setKeyboardTracking(false);
        spinNumProbes->setMinimum(0);
        spinNumProbes->setMaximum(3);
        spinNumProbes->setValue(1);

        gridTimeResolved->addWidget(spinNumProbes, 7, 2, 1, 2);

        comboTimingMode = new QComboBox(gridLayoutWidget);
        comboTimingMode->setObjectName(QString::fromUtf8("comboTimingMode"));
        sizePolicy2.setHeightForWidth(comboTimingMode->sizePolicy().hasHeightForWidth());
        comboTimingMode->setSizePolicy(sizePolicy2);

        gridTimeResolved->addWidget(comboTimingMode, 0, 2, 1, 2);

        lblNumFrames = new QLabel(gridLayoutWidget);
        lblNumFrames->setObjectName(QString::fromUtf8("lblNumFrames"));
        lblNumFrames->setEnabled(false);

        gridTimeResolved->addWidget(lblNumFrames, 1, 0, 1, 1);

        spinNumFrames = new QSpinBox(gridLayoutWidget);
        spinNumFrames->setObjectName(QString::fromUtf8("spinNumFrames"));
        spinNumFrames->setEnabled(false);
        sizePolicy2.setHeightForWidth(spinNumFrames->sizePolicy().hasHeightForWidth());
        spinNumFrames->setSizePolicy(sizePolicy2);
        spinNumFrames->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinNumFrames->setKeyboardTracking(false);
        spinNumFrames->setMinimum(1);
        spinNumFrames->setMaximum(2000000000);
        spinNumFrames->setValue(1);

        gridTimeResolved->addWidget(spinNumFrames, 1, 2, 1, 2);

        lblExpTime = new QLabel(gridLayoutWidget);
        lblExpTime->setObjectName(QString::fromUtf8("lblExpTime"));
        lblExpTime->setEnabled(true);

        gridTimeResolved->addWidget(lblExpTime, 2, 0, 1, 1);

        spinExpTime = new QDoubleSpinBox(gridLayoutWidget);
        spinExpTime->setObjectName(QString::fromUtf8("spinExpTime"));
        spinExpTime->setEnabled(true);
        sizePolicy2.setHeightForWidth(spinExpTime->sizePolicy().hasHeightForWidth());
        spinExpTime->setSizePolicy(sizePolicy2);
        spinExpTime->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinExpTime->setKeyboardTracking(false);
        spinExpTime->setDecimals(5);
        spinExpTime->setMinimum(0);
        spinExpTime->setMaximum(2e+09);
        spinExpTime->setValue(1);

        gridTimeResolved->addWidget(spinExpTime, 2, 2, 1, 1);

        comboExpUnit = new QComboBox(gridLayoutWidget);
        comboExpUnit->setObjectName(QString::fromUtf8("comboExpUnit"));
        comboExpUnit->setEnabled(true);
        sizePolicy2.setHeightForWidth(comboExpUnit->sizePolicy().hasHeightForWidth());
        comboExpUnit->setSizePolicy(sizePolicy2);
        comboExpUnit->setLayoutDirection(Qt::LeftToRight);

        gridTimeResolved->addWidget(comboExpUnit, 2, 3, 1, 1);

        lblPeriod = new QLabel(gridLayoutWidget);
        lblPeriod->setObjectName(QString::fromUtf8("lblPeriod"));
        lblPeriod->setEnabled(false);

        gridTimeResolved->addWidget(lblPeriod, 3, 0, 1, 1);

        spinPeriod = new QDoubleSpinBox(gridLayoutWidget);
        spinPeriod->setObjectName(QString::fromUtf8("spinPeriod"));
        spinPeriod->setEnabled(false);
        sizePolicy2.setHeightForWidth(spinPeriod->sizePolicy().hasHeightForWidth());
        spinPeriod->setSizePolicy(sizePolicy2);
        spinPeriod->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinPeriod->setKeyboardTracking(false);
        spinPeriod->setDecimals(5);
        spinPeriod->setMaximum(2e+09);
        spinPeriod->setValue(0);

        gridTimeResolved->addWidget(spinPeriod, 3, 2, 1, 1);

        comboPeriodUnit = new QComboBox(gridLayoutWidget);
        comboPeriodUnit->setObjectName(QString::fromUtf8("comboPeriodUnit"));
        comboPeriodUnit->setEnabled(false);
        sizePolicy.setHeightForWidth(comboPeriodUnit->sizePolicy().hasHeightForWidth());
        comboPeriodUnit->setSizePolicy(sizePolicy);
        comboPeriodUnit->setLayoutDirection(Qt::LeftToRight);

        gridTimeResolved->addWidget(comboPeriodUnit, 3, 3, 1, 1);

        horizontalSpacer = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridTimeResolved->addItem(horizontalSpacer, 1, 1, 1, 1);

        lblTimingMode = new QLabel(gridLayoutWidget);
        lblTimingMode->setObjectName(QString::fromUtf8("lblTimingMode"));

        gridTimeResolved->addWidget(lblTimingMode, 0, 0, 1, 1);

        frameNotTimeResolved = new QFrame(TabMeasurementObject);
        frameNotTimeResolved->setObjectName(QString::fromUtf8("frameNotTimeResolved"));
        frameNotTimeResolved->setGeometry(QRect(20, 5, 336, 159));
        frameNotTimeResolved->setFrameShape(QFrame::NoFrame);
        frameNotTimeResolved->setFrameShadow(QFrame::Plain);
        gridLayoutWidget_3 = new QWidget(frameNotTimeResolved);
        gridLayoutWidget_3->setObjectName(QString::fromUtf8("gridLayoutWidget_3"));
        gridLayoutWidget_3->setGeometry(QRect(10, 13, 321, 130));
        gridLayout = new QGridLayout(gridLayoutWidget_3);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setVerticalSpacing(6);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        label_5 = new QLabel(gridLayoutWidget_3);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 0, 0, 1, 1);

        spinNumMeasurements = new QSpinBox(gridLayoutWidget_3);
        spinNumMeasurements->setObjectName(QString::fromUtf8("spinNumMeasurements"));
        sizePolicy2.setHeightForWidth(spinNumMeasurements->sizePolicy().hasHeightForWidth());
        spinNumMeasurements->setSizePolicy(sizePolicy2);
        spinNumMeasurements->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinNumMeasurements->setKeyboardTracking(false);
        spinNumMeasurements->setMinimum(1);
        spinNumMeasurements->setMaximum(2000000000);
        spinNumMeasurements->setValue(1);

        gridLayout->addWidget(spinNumMeasurements, 0, 1, 1, 2);

        dispFileName = new QLineEdit(gridLayoutWidget_3);
        dispFileName->setObjectName(QString::fromUtf8("dispFileName"));
        sizePolicy2.setHeightForWidth(dispFileName->sizePolicy().hasHeightForWidth());
        dispFileName->setSizePolicy(sizePolicy2);

        gridLayout->addWidget(dispFileName, 1, 1, 1, 2);

        label_8 = new QLabel(gridLayoutWidget_3);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout->addWidget(label_8, 2, 0, 1, 1);

        spinIndex = new QSpinBox(gridLayoutWidget_3);
        spinIndex->setObjectName(QString::fromUtf8("spinIndex"));
        sizePolicy2.setHeightForWidth(spinIndex->sizePolicy().hasHeightForWidth());
        spinIndex->setSizePolicy(sizePolicy2);
        spinIndex->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        spinIndex->setKeyboardTracking(false);
        spinIndex->setMinimum(0);
        spinIndex->setMaximum(2000000000);
        spinIndex->setValue(0);

        gridLayout->addWidget(spinIndex, 2, 1, 1, 2);

        chkFile = new QCheckBox(gridLayoutWidget_3);
        chkFile->setObjectName(QString::fromUtf8("chkFile"));
        chkFile->setChecked(true);

        gridLayout->addWidget(chkFile, 1, 0, 1, 1);

        QWidget::setTabOrder(spinNumMeasurements, chkFile);
        QWidget::setTabOrder(chkFile, dispFileName);
        QWidget::setTabOrder(dispFileName, spinIndex);
        QWidget::setTabOrder(spinIndex, comboTimingMode);
        QWidget::setTabOrder(comboTimingMode, spinNumFrames);
        QWidget::setTabOrder(spinNumFrames, spinExpTime);
        QWidget::setTabOrder(spinExpTime, comboExpUnit);
        QWidget::setTabOrder(comboExpUnit, spinPeriod);
        QWidget::setTabOrder(spinPeriod, comboPeriodUnit);
        QWidget::setTabOrder(comboPeriodUnit, spinNumTriggers);
        QWidget::setTabOrder(spinNumTriggers, spinDelay);
        QWidget::setTabOrder(spinDelay, comboDelayUnit);
        QWidget::setTabOrder(comboDelayUnit, spinNumGates);
        QWidget::setTabOrder(spinNumGates, spinNumProbes);

        retranslateUi(TabMeasurementObject);

        comboDelayUnit->setCurrentIndex(2);
        comboExpUnit->setCurrentIndex(2);
        comboPeriodUnit->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(TabMeasurementObject);
    } // setupUi

    void retranslateUi(QWidget *TabMeasurementObject)
    {
        TabMeasurementObject->setWindowTitle(QApplication::translate("TabMeasurementObject", "Form", 0, QApplication::UnicodeUTF8));
        boxProgress->setTitle(QApplication::translate("TabMeasurementObject", "Progress Monitor", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("TabMeasurementObject", "Current Frame: ", 0, QApplication::UnicodeUTF8));
        lblProgressIndex->setText(QApplication::translate("TabMeasurementObject", "0", 0, QApplication::UnicodeUTF8));
        lblCurrentMeasurement->setText(QApplication::translate("TabMeasurementObject", "0", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("TabMeasurementObject", "Current Measurement:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        btnStartStop->setToolTip(QApplication::translate("TabMeasurementObject", "Starts or Stops Acquisition", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnStartStop->setText(QApplication::translate("TabMeasurementObject", "Start", 0, QApplication::UnicodeUTF8));
        btnStartStop->setShortcut(QApplication::translate("TabMeasurementObject", "Shift+Space", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lblNumTriggers->setToolTip(QApplication::translate("TabMeasurementObject", "Number of Triggers to be expected.\n"
" #cycles#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        lblNumTriggers->setText(QApplication::translate("TabMeasurementObject", "Number of Triggers:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinNumTriggers->setToolTip(QApplication::translate("TabMeasurementObject", "Number of Triggers to be expected.\n"
" #cycles#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        spinNumTriggers->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        spinNumTriggers->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        spinNumTriggers->setSuffix(QString());
#ifndef QT_NO_TOOLTIP
        lblDelay->setToolTip(QApplication::translate("TabMeasurementObject", "The Delay between Trigger Edge and Start of Exposure ( or Readout). \n"
"#delay#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        lblDelay->setText(QApplication::translate("TabMeasurementObject", "Delay After Trigger:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinDelay->setToolTip(QApplication::translate("TabMeasurementObject", "The Delay between Trigger Edge and Start of Exposure ( or Readout). \n"
"#delay#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        comboDelayUnit->clear();
        comboDelayUnit->insertItems(0, QStringList()
         << QApplication::translate("TabMeasurementObject", "hr", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "min", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "s", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "ms", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "us", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "ns", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        comboDelayUnit->setToolTip(QApplication::translate("TabMeasurementObject", "The Delay between Trigger Edge and Start of Exposure ( or Readout). \n"
"#delay#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        lblNumGates->setToolTip(QApplication::translate("TabMeasurementObject", "Number of Gate Signals per Frame.\n"
" #gates#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        lblNumGates->setText(QApplication::translate("TabMeasurementObject", "Number of Gates:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinNumGates->setToolTip(QApplication::translate("TabMeasurementObject", "Number of Gate Signals per Frame.\n"
" #gates#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        spinNumGates->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        spinNumGates->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        spinNumGates->setSuffix(QString());
#ifndef QT_NO_TOOLTIP
        lblNumProbes->setToolTip(QApplication::translate("TabMeasurementObject", "<nobr>\n"
"The data are accumulated over several (frames) pump cycles.\n"
"</nobr>\n"
"<br>\n"
"<nobr> \n"
"Enabled only in <b>Expert Mode</b> and if<b> Number of Frames</b> > 1.\n"
"</nobr>\n"
"<br>\n"
"<nobr>Setting <b>Number of Probes</b> will reset <b>Number of Triggers</b> to 1.\n"
"</nobr>\n"
"<br>\n"
"Maximum value is 3. <br>\n"
"#probes#\n"
"", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        lblNumProbes->setText(QApplication::translate("TabMeasurementObject", "Number of Probes:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinNumProbes->setToolTip(QApplication::translate("TabMeasurementObject", "<nobr>\n"
"The data are accumulated over several (frames) pump cycles.\n"
"</nobr>\n"
"<br>\n"
"<nobr> \n"
"Enabled only in <b>Expert Mode</b> and if<b> Number of Frames</b> > 1.\n"
"</nobr>\n"
"<br>\n"
"<nobr>Setting <b>Number of Probes</b> will reset <b>Number of Triggers</b> to 1.\n"
"</nobr>\n"
"<br>\n"
"Maximum value is 3. <br>\n"
"#probes#\n"
"", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        spinNumProbes->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        spinNumProbes->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        spinNumProbes->setSuffix(QString());
        comboTimingMode->clear();
        comboTimingMode->insertItems(0, QStringList()
         << QApplication::translate("TabMeasurementObject", "None", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "Auto", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "Trigger Exposure Series", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "Trigger Readout", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "Gated with Fixed Number", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "Gated with Start Trigger", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "Burst Trigger", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        comboTimingMode->setToolTip(QApplication::translate("TabMeasurementObject", "Timing Mode of the detector. \n"
" #timing#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        lblNumFrames->setText(QApplication::translate("TabMeasurementObject", "Number of  Frames:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinNumFrames->setToolTip(QApplication::translate("TabMeasurementObject", "Number of measurements (not in real time) that will be acquired. \n"
" #frames#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        spinNumFrames->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        spinNumFrames->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        spinNumFrames->setSuffix(QString());
#ifndef QT_NO_TOOLTIP
        lblExpTime->setToolTip(QApplication::translate("TabMeasurementObject", "<nobr>\n"
"Exposure Time of a frame.\n"
"</nobr><br><nobr>\n"
" #exptime#\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        lblExpTime->setText(QApplication::translate("TabMeasurementObject", "Exposure Time:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinExpTime->setToolTip(QApplication::translate("TabMeasurementObject", "<nobr>\n"
"Exposure Time of a frame.\n"
"</nobr><br><nobr>\n"
" #exptime#\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        comboExpUnit->clear();
        comboExpUnit->insertItems(0, QStringList()
         << QApplication::translate("TabMeasurementObject", "hr", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "min", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "s", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "ms", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "us", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "ns", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        comboExpUnit->setToolTip(QApplication::translate("TabMeasurementObject", "<nobr>\n"
"Exposure Time of a frame.\n"
"</nobr><br><nobr>\n"
" #exptime#\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        lblPeriod->setToolTip(QApplication::translate("TabMeasurementObject", "<nobr>\n"
"Frame period between exposures. \n"
"</nobr><br><nobr>\n"
" #period#\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        lblPeriod->setText(QApplication::translate("TabMeasurementObject", "Acquisition Period:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinPeriod->setToolTip(QApplication::translate("TabMeasurementObject", "<nobr>\n"
"Frame period between exposures. \n"
"</nobr><br><nobr>\n"
" #period#\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        comboPeriodUnit->clear();
        comboPeriodUnit->insertItems(0, QStringList()
         << QApplication::translate("TabMeasurementObject", "hr", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "min", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "s", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "ms", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "us", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabMeasurementObject", "ns", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        comboPeriodUnit->setToolTip(QApplication::translate("TabMeasurementObject", "<nobr>\n"
"Frame period between exposures. \n"
"</nobr><br><nobr>\n"
" #period#\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        lblTimingMode->setText(QApplication::translate("TabMeasurementObject", "Timing Mode:", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("TabMeasurementObject", "Number of Measurements:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinNumMeasurements->setToolTip(QApplication::translate("TabMeasurementObject", "Number of measurements (not in real time) that will be acquired. \n"
" #frames#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        spinNumMeasurements->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        spinNumMeasurements->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        spinNumMeasurements->setSuffix(QString());
#ifndef QT_NO_TOOLTIP
        dispFileName->setToolTip(QApplication::translate("TabMeasurementObject", "Root of the file name  - please check that the output directory is correctly set and select the file name format. \n"
" #fname#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        dispFileName->setText(QString());
        label_8->setText(QApplication::translate("TabMeasurementObject", "Run Index:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spinIndex->setToolTip(QApplication::translate("TabMeasurementObject", "Run index (automatically incremented) \n"
" #index#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        spinIndex->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        spinIndex->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        spinIndex->setSuffix(QString());
#ifndef QT_NO_TOOLTIP
        chkFile->setToolTip(QApplication::translate("TabMeasurementObject", "<nobr>Sets output file name prefix</nobr><br>\n"
"<nobr>Check the box to enable write to file. </nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        chkFile->setText(QApplication::translate("TabMeasurementObject", "File Name:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TabMeasurementObject: public Ui_TabMeasurementObject {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORM_TAB_MEASUREMENT_H
