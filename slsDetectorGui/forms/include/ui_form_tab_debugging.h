/********************************************************************************
** Form generated from reading UI file 'form_tab_debugging.ui'
**
** Created: Thu Jul 13 14:40:29 2017
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORM_TAB_DEBUGGING_H
#define UI_FORM_TAB_DEBUGGING_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TabDebuggingObject
{
public:
    QGroupBox *groupBox;
    QWidget *gridLayoutWidget_2;
    QGridLayout *gridLayout_2;
    QCheckBox *chkDetectorFirmware;
    QCheckBox *chkDetectorSoftware;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *btnTest;
    QWidget *gridLayoutWidget_5;
    QGridLayout *gridLayout_5;
    QLabel *lblModule;
    QComboBox *comboModule;
    QSpacerItem *horizontalSpacer_3;
    QWidget *gridLayoutWidget_3;
    QGridLayout *gridLayout_3;
    QCheckBox *chkDetectorBus;
    QCheckBox *chkDetectorMemory;
    QWidget *gridLayoutWidget_6;
    QGridLayout *gridLayout_6;
    QCheckBox *chkChip;
    QWidget *gridLayoutWidget_4;
    QGridLayout *gridLayout_4;
    QLabel *lblDetector;
    QComboBox *comboDetector;
    QSpacerItem *horizontalSpacer;
    QWidget *horizontalLayoutWidget_4;
    QHBoxLayout *horizontalLayout_4;
    QLabel *lblStatusLabel;
    QSpacerItem *horizontalSpacer_2;
    QLabel *lblStatus;
    QWidget *gridLayoutWidget_7;
    QGridLayout *gridLayout_7;
    QCheckBox *chkModuleFirmware;
    QFrame *line;
    QFrame *line_2;
    QWidget *horizontalLayoutWidget_3;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *btnGetInfo;

    void setupUi(QWidget *TabDebuggingObject)
    {
        if (TabDebuggingObject->objectName().isEmpty())
            TabDebuggingObject->setObjectName(QString::fromUtf8("TabDebuggingObject"));
        TabDebuggingObject->resize(775, 345);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(TabDebuggingObject->sizePolicy().hasHeightForWidth());
        TabDebuggingObject->setSizePolicy(sizePolicy);
        TabDebuggingObject->setMinimumSize(QSize(0, 0));
        TabDebuggingObject->setMaximumSize(QSize(1000, 1000));
        groupBox = new QGroupBox(TabDebuggingObject);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(45, 75, 686, 231));
        groupBox->setFocusPolicy(Qt::NoFocus);
        gridLayoutWidget_2 = new QWidget(groupBox);
        gridLayoutWidget_2->setObjectName(QString::fromUtf8("gridLayoutWidget_2"));
        gridLayoutWidget_2->setGeometry(QRect(15, 110, 141, 51));
        gridLayout_2 = new QGridLayout(gridLayoutWidget_2);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        chkDetectorFirmware = new QCheckBox(gridLayoutWidget_2);
        chkDetectorFirmware->setObjectName(QString::fromUtf8("chkDetectorFirmware"));
        chkDetectorFirmware->setFocusPolicy(Qt::NoFocus);

        gridLayout_2->addWidget(chkDetectorFirmware, 0, 0, 1, 1);

        chkDetectorSoftware = new QCheckBox(gridLayoutWidget_2);
        chkDetectorSoftware->setObjectName(QString::fromUtf8("chkDetectorSoftware"));
        chkDetectorSoftware->setFocusPolicy(Qt::NoFocus);

        gridLayout_2->addWidget(chkDetectorSoftware, 1, 0, 1, 1);

        horizontalLayoutWidget = new QWidget(groupBox);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(15, 185, 656, 36));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        btnTest = new QPushButton(horizontalLayoutWidget);
        btnTest->setObjectName(QString::fromUtf8("btnTest"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(btnTest->sizePolicy().hasHeightForWidth());
        btnTest->setSizePolicy(sizePolicy1);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/images/start.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnTest->setIcon(icon);

        horizontalLayout->addWidget(btnTest);

        gridLayoutWidget_5 = new QWidget(groupBox);
        gridLayoutWidget_5->setObjectName(QString::fromUtf8("gridLayoutWidget_5"));
        gridLayoutWidget_5->setGeometry(QRect(415, 30, 256, 31));
        gridLayout_5 = new QGridLayout(gridLayoutWidget_5);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        gridLayout_5->setContentsMargins(0, 0, 0, 0);
        lblModule = new QLabel(gridLayoutWidget_5);
        lblModule->setObjectName(QString::fromUtf8("lblModule"));
        QSizePolicy sizePolicy2(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(lblModule->sizePolicy().hasHeightForWidth());
        lblModule->setSizePolicy(sizePolicy2);
        lblModule->setMinimumSize(QSize(0, 0));
        lblModule->setMaximumSize(QSize(100, 16777215));

        gridLayout_5->addWidget(lblModule, 0, 0, 1, 1);

        comboModule = new QComboBox(gridLayoutWidget_5);
        comboModule->setObjectName(QString::fromUtf8("comboModule"));
        sizePolicy.setHeightForWidth(comboModule->sizePolicy().hasHeightForWidth());
        comboModule->setSizePolicy(sizePolicy);
        comboModule->setMaximumSize(QSize(16777215, 16777215));
        comboModule->setFrame(true);

        gridLayout_5->addWidget(comboModule, 0, 2, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_5->addItem(horizontalSpacer_3, 0, 1, 1, 1);

        gridLayoutWidget_3 = new QWidget(groupBox);
        gridLayoutWidget_3->setObjectName(QString::fromUtf8("gridLayoutWidget_3"));
        gridLayoutWidget_3->setGeometry(QRect(280, 110, 141, 51));
        gridLayout_3 = new QGridLayout(gridLayoutWidget_3);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout_3->setContentsMargins(0, 0, 0, 0);
        chkDetectorBus = new QCheckBox(gridLayoutWidget_3);
        chkDetectorBus->setObjectName(QString::fromUtf8("chkDetectorBus"));
        chkDetectorBus->setFocusPolicy(Qt::NoFocus);

        gridLayout_3->addWidget(chkDetectorBus, 0, 0, 1, 1);

        chkDetectorMemory = new QCheckBox(gridLayoutWidget_3);
        chkDetectorMemory->setObjectName(QString::fromUtf8("chkDetectorMemory"));
        chkDetectorMemory->setFocusPolicy(Qt::NoFocus);

        gridLayout_3->addWidget(chkDetectorMemory, 1, 0, 1, 1);

        gridLayoutWidget_6 = new QWidget(groupBox);
        gridLayoutWidget_6->setObjectName(QString::fromUtf8("gridLayoutWidget_6"));
        gridLayoutWidget_6->setGeometry(QRect(530, 110, 141, 26));
        gridLayout_6 = new QGridLayout(gridLayoutWidget_6);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        gridLayout_6->setContentsMargins(0, 0, 0, 0);
        chkChip = new QCheckBox(gridLayoutWidget_6);
        chkChip->setObjectName(QString::fromUtf8("chkChip"));
        chkChip->setFocusPolicy(Qt::NoFocus);

        gridLayout_6->addWidget(chkChip, 0, 0, 1, 1);

        gridLayoutWidget_4 = new QWidget(groupBox);
        gridLayoutWidget_4->setObjectName(QString::fromUtf8("gridLayoutWidget_4"));
        gridLayoutWidget_4->setGeometry(QRect(15, 30, 276, 31));
        gridLayout_4 = new QGridLayout(gridLayoutWidget_4);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        gridLayout_4->setContentsMargins(0, 0, 0, 0);
        lblDetector = new QLabel(gridLayoutWidget_4);
        lblDetector->setObjectName(QString::fromUtf8("lblDetector"));
        sizePolicy2.setHeightForWidth(lblDetector->sizePolicy().hasHeightForWidth());
        lblDetector->setSizePolicy(sizePolicy2);
        lblDetector->setMinimumSize(QSize(0, 0));
        lblDetector->setMaximumSize(QSize(150, 16777215));

        gridLayout_4->addWidget(lblDetector, 0, 0, 1, 1);

        comboDetector = new QComboBox(gridLayoutWidget_4);
        comboDetector->setObjectName(QString::fromUtf8("comboDetector"));
        sizePolicy.setHeightForWidth(comboDetector->sizePolicy().hasHeightForWidth());
        comboDetector->setSizePolicy(sizePolicy);
        comboDetector->setMaximumSize(QSize(16777215, 16777215));

        gridLayout_4->addWidget(comboDetector, 0, 2, 1, 1);

        horizontalSpacer = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_4->addItem(horizontalSpacer, 0, 1, 1, 1);

        horizontalLayoutWidget_4 = new QWidget(groupBox);
        horizontalLayoutWidget_4->setObjectName(QString::fromUtf8("horizontalLayoutWidget_4"));
        horizontalLayoutWidget_4->setGeometry(QRect(15, 60, 276, 31));
        horizontalLayout_4 = new QHBoxLayout(horizontalLayoutWidget_4);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        lblStatusLabel = new QLabel(horizontalLayoutWidget_4);
        lblStatusLabel->setObjectName(QString::fromUtf8("lblStatusLabel"));
        QSizePolicy sizePolicy3(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(lblStatusLabel->sizePolicy().hasHeightForWidth());
        lblStatusLabel->setSizePolicy(sizePolicy3);
        lblStatusLabel->setMaximumSize(QSize(150, 16777215));

        horizontalLayout_4->addWidget(lblStatusLabel);

        horizontalSpacer_2 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_2);

        lblStatus = new QLabel(horizontalLayoutWidget_4);
        lblStatus->setObjectName(QString::fromUtf8("lblStatus"));
        sizePolicy.setHeightForWidth(lblStatus->sizePolicy().hasHeightForWidth());
        lblStatus->setSizePolicy(sizePolicy);

        horizontalLayout_4->addWidget(lblStatus);

        gridLayoutWidget_7 = new QWidget(groupBox);
        gridLayoutWidget_7->setObjectName(QString::fromUtf8("gridLayoutWidget_7"));
        gridLayoutWidget_7->setGeometry(QRect(530, 135, 141, 26));
        gridLayout_7 = new QGridLayout(gridLayoutWidget_7);
        gridLayout_7->setObjectName(QString::fromUtf8("gridLayout_7"));
        gridLayout_7->setContentsMargins(0, 0, 0, 0);
        chkModuleFirmware = new QCheckBox(gridLayoutWidget_7);
        chkModuleFirmware->setObjectName(QString::fromUtf8("chkModuleFirmware"));
        chkModuleFirmware->setFocusPolicy(Qt::NoFocus);

        gridLayout_7->addWidget(chkModuleFirmware, 0, 0, 1, 1);

        line = new QFrame(groupBox);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(5, 95, 676, 16));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        line_2 = new QFrame(groupBox);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setGeometry(QRect(5, 165, 676, 16));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);
        horizontalLayoutWidget_3 = new QWidget(TabDebuggingObject);
        horizontalLayoutWidget_3->setObjectName(QString::fromUtf8("horizontalLayoutWidget_3"));
        horizontalLayoutWidget_3->setGeometry(QRect(45, 25, 686, 31));
        horizontalLayout_3 = new QHBoxLayout(horizontalLayoutWidget_3);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        btnGetInfo = new QPushButton(horizontalLayoutWidget_3);
        btnGetInfo->setObjectName(QString::fromUtf8("btnGetInfo"));
        sizePolicy1.setHeightForWidth(btnGetInfo->sizePolicy().hasHeightForWidth());
        btnGetInfo->setSizePolicy(sizePolicy1);
        btnGetInfo->setMinimumSize(QSize(0, 0));
        btnGetInfo->setMaximumSize(QSize(16777215, 16777215));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/images/download.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnGetInfo->setIcon(icon1);

        horizontalLayout_3->addWidget(btnGetInfo);

        QWidget::setTabOrder(btnGetInfo, comboDetector);
        QWidget::setTabOrder(comboDetector, comboModule);
        QWidget::setTabOrder(comboModule, btnTest);

        retranslateUi(TabDebuggingObject);

        QMetaObject::connectSlotsByName(TabDebuggingObject);
    } // setupUi

    void retranslateUi(QWidget *TabDebuggingObject)
    {
        TabDebuggingObject->setWindowTitle(QApplication::translate("TabDebuggingObject", "Form", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("TabDebuggingObject", "Tests", 0, QApplication::UnicodeUTF8));
        chkDetectorFirmware->setText(QApplication::translate("TabDebuggingObject", "Readout Firmware", 0, QApplication::UnicodeUTF8));
        chkDetectorSoftware->setText(QApplication::translate("TabDebuggingObject", "Readout Software", 0, QApplication::UnicodeUTF8));
        btnTest->setText(QApplication::translate("TabDebuggingObject", "Run Test   ", 0, QApplication::UnicodeUTF8));
        lblModule->setText(QApplication::translate("TabDebuggingObject", "Module:", 0, QApplication::UnicodeUTF8));
        comboModule->clear();
        comboModule->insertItems(0, QStringList()
         << QApplication::translate("TabDebuggingObject", "All Modules", 0, QApplication::UnicodeUTF8)
        );
        chkDetectorBus->setText(QApplication::translate("TabDebuggingObject", "Readout Bus", 0, QApplication::UnicodeUTF8));
        chkDetectorMemory->setText(QApplication::translate("TabDebuggingObject", "Readout Memory", 0, QApplication::UnicodeUTF8));
        chkChip->setText(QApplication::translate("TabDebuggingObject", "Chip", 0, QApplication::UnicodeUTF8));
        lblDetector->setText(QApplication::translate("TabDebuggingObject", "Readout:", 0, QApplication::UnicodeUTF8));
        lblStatusLabel->setText(QApplication::translate("TabDebuggingObject", "Status:", 0, QApplication::UnicodeUTF8));
        lblStatus->setText(QApplication::translate("TabDebuggingObject", "IDLE", 0, QApplication::UnicodeUTF8));
        chkModuleFirmware->setText(QApplication::translate("TabDebuggingObject", "Module Firmware", 0, QApplication::UnicodeUTF8));
        btnGetInfo->setText(QApplication::translate("TabDebuggingObject", "Get  ID Information   ", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TabDebuggingObject: public Ui_TabDebuggingObject {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORM_TAB_DEBUGGING_H
