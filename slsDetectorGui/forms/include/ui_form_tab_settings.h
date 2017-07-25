/********************************************************************************
** Form generated from reading UI file 'form_tab_settings.ui'
**
** Created: Thu Jul 13 14:40:29 2017
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORM_TAB_SETTINGS_H
#define UI_FORM_TAB_SETTINGS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TabSettingsObject
{
public:
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QLabel *label_3;
    QComboBox *comboSettings;
    QSpinBox *spinThreshold;
    QLabel *label_5;
    QLabel *label_6;
    QComboBox *comboDynamicRange;
    QSpinBox *spinNumModules;
    QSpacerItem *horizontalSpacer;
    QLabel *lblThreshold;

    void setupUi(QWidget *TabSettingsObject)
    {
        if (TabSettingsObject->objectName().isEmpty())
            TabSettingsObject->setObjectName(QString::fromUtf8("TabSettingsObject"));
        TabSettingsObject->resize(775, 345);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(TabSettingsObject->sizePolicy().hasHeightForWidth());
        TabSettingsObject->setSizePolicy(sizePolicy);
        TabSettingsObject->setMinimumSize(QSize(0, 0));
        TabSettingsObject->setMaximumSize(QSize(1000, 1000));
        gridLayoutWidget = new QWidget(TabSettingsObject);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(30, 20, 316, 171));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        label_3 = new QLabel(gridLayoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 0, 0, 1, 1);

        comboSettings = new QComboBox(gridLayoutWidget);
        comboSettings->setObjectName(QString::fromUtf8("comboSettings"));
        sizePolicy.setHeightForWidth(comboSettings->sizePolicy().hasHeightForWidth());
        comboSettings->setSizePolicy(sizePolicy);

        gridLayout->addWidget(comboSettings, 0, 2, 1, 2);

        spinThreshold = new QSpinBox(gridLayoutWidget);
        spinThreshold->setObjectName(QString::fromUtf8("spinThreshold"));
        spinThreshold->setKeyboardTracking(false);
        spinThreshold->setMinimum(-100000);
        spinThreshold->setMaximum(100000);
        spinThreshold->setSingleStep(100);
        spinThreshold->setValue(-1);

        gridLayout->addWidget(spinThreshold, 1, 2, 1, 2);

        label_5 = new QLabel(gridLayoutWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 2, 0, 1, 1);

        label_6 = new QLabel(gridLayoutWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 3, 0, 1, 1);

        comboDynamicRange = new QComboBox(gridLayoutWidget);
        comboDynamicRange->setObjectName(QString::fromUtf8("comboDynamicRange"));

        gridLayout->addWidget(comboDynamicRange, 3, 2, 1, 2);

        spinNumModules = new QSpinBox(gridLayoutWidget);
        spinNumModules->setObjectName(QString::fromUtf8("spinNumModules"));
        spinNumModules->setMinimum(1);

        gridLayout->addWidget(spinNumModules, 2, 2, 1, 2);

        horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 1, 1, 1, 1);

        lblThreshold = new QLabel(gridLayoutWidget);
        lblThreshold->setObjectName(QString::fromUtf8("lblThreshold"));

        gridLayout->addWidget(lblThreshold, 1, 0, 1, 1);

        QWidget::setTabOrder(comboSettings, spinThreshold);
        QWidget::setTabOrder(spinThreshold, spinNumModules);
        QWidget::setTabOrder(spinNumModules, comboDynamicRange);

        retranslateUi(TabSettingsObject);

        QMetaObject::connectSlotsByName(TabSettingsObject);
    } // setupUi

    void retranslateUi(QWidget *TabSettingsObject)
    {
        TabSettingsObject->setWindowTitle(QApplication::translate("TabSettingsObject", "Form", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("TabSettingsObject", "Settings:", 0, QApplication::UnicodeUTF8));
        comboSettings->clear();
        comboSettings->insertItems(0, QStringList()
         << QApplication::translate("TabSettingsObject", "Standard", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "Fast", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "High Gain", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "Dynamic Gain", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "Low Gain", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "Medium Gain", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "Very High Gain", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "Low Noise", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "Dynamic HG0", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "Fix Gain 1", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "Fix Gain 2", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "Force Switch G1", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "Force Switch G2", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "Very Low Gain", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "Undefined", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "Uninitialized", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        comboSettings->setToolTip(QApplication::translate("TabSettingsObject", "Settings of the detector. \n"
" #settings#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        spinThreshold->setSuffix(QApplication::translate("TabSettingsObject", " eV", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("TabSettingsObject", "Number of Modules:", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("TabSettingsObject", "Dynamic Range:", 0, QApplication::UnicodeUTF8));
        comboDynamicRange->clear();
        comboDynamicRange->insertItems(0, QStringList()
         << QApplication::translate("TabSettingsObject", "1.67772e+07", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "65535", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "255", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabSettingsObject", "7", 0, QApplication::UnicodeUTF8)
        );
        lblThreshold->setText(QApplication::translate("TabSettingsObject", "Threshold:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TabSettingsObject: public Ui_TabSettingsObject {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORM_TAB_SETTINGS_H
