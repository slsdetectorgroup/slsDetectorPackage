/********************************************************************************
** Form generated from reading UI file 'form_scan.ui'
**
** Created: Tue Jul 25 12:31:25 2017
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORM_SCAN_H
#define UI_FORM_SCAN_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
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
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ScanObject
{
public:
    QGridLayout *gridLayout;
    QGroupBox *group;
    QGridLayout *gridLayout_2;
    QHBoxLayout *horizontalLayout_7;
    QSpacerItem *horizontalSpacer_5;
    QRadioButton *radioRange;
    QRadioButton *radioCustom;
    QRadioButton *radioFile;
    QSpacerItem *horizontalSpacer_6;
    QHBoxLayout *horizontalLayout_8;
    QWidget *stackedWidget;
    QHBoxLayout *horizontalLayout_4;
    QLabel *lblPrecision;
    QSpacerItem *horizontalSpacer_7;
    QSpinBox *spinPrecision;
    QHBoxLayout *horizontalLayout_2;
    QLabel *lblParameter;
    QSpacerItem *horizontalSpacer_4;
    QLineEdit *dispParameter;
    QHBoxLayout *horizontalLayout_5;
    QLabel *lblSteps;
    QSpacerItem *horizontalSpacer_8;
    QSpinBox *spinSteps;
    QComboBox *comboScript;
    QHBoxLayout *horizontalLayout;
    QLineEdit *dispScript;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *btnBrowse;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *horizontalSpacer_3;

    void setupUi(QWidget *ScanObject)
    {
        if (ScanObject->objectName().isEmpty())
            ScanObject->setObjectName(QString::fromUtf8("ScanObject"));
        ScanObject->resize(724, 125);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ScanObject->sizePolicy().hasHeightForWidth());
        ScanObject->setSizePolicy(sizePolicy);
        ScanObject->setMinimumSize(QSize(0, 0));
        ScanObject->setMaximumSize(QSize(1000, 1000));
        gridLayout = new QGridLayout(ScanObject);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setHorizontalSpacing(0);
        gridLayout->setVerticalSpacing(5);
        group = new QGroupBox(ScanObject);
        group->setObjectName(QString::fromUtf8("group"));
        group->setEnabled(false);
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(group->sizePolicy().hasHeightForWidth());
        group->setSizePolicy(sizePolicy1);
        group->setMinimumSize(QSize(180, 0));
        gridLayout_2 = new QGridLayout(group);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setHorizontalSpacing(0);
        gridLayout_2->setVerticalSpacing(5);
        gridLayout_2->setContentsMargins(0, 5, 0, 2);
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        horizontalSpacer_5 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_5);

        radioRange = new QRadioButton(group);
        radioRange->setObjectName(QString::fromUtf8("radioRange"));
        radioRange->setChecked(true);

        horizontalLayout_7->addWidget(radioRange);

        radioCustom = new QRadioButton(group);
        radioCustom->setObjectName(QString::fromUtf8("radioCustom"));
        radioCustom->setChecked(false);

        horizontalLayout_7->addWidget(radioCustom);

        radioFile = new QRadioButton(group);
        radioFile->setObjectName(QString::fromUtf8("radioFile"));
        sizePolicy.setHeightForWidth(radioFile->sizePolicy().hasHeightForWidth());
        radioFile->setSizePolicy(sizePolicy);
        radioFile->setChecked(false);

        horizontalLayout_7->addWidget(radioFile);

        horizontalSpacer_6 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_6);


        gridLayout_2->addLayout(horizontalLayout_7, 0, 0, 1, 1);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        stackedWidget = new QWidget(group);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));

        horizontalLayout_8->addWidget(stackedWidget);


        gridLayout_2->addLayout(horizontalLayout_8, 1, 0, 1, 1);


        gridLayout->addWidget(group, 2, 2, 3, 3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(0);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        lblPrecision = new QLabel(ScanObject);
        lblPrecision->setObjectName(QString::fromUtf8("lblPrecision"));
        lblPrecision->setEnabled(false);
        sizePolicy.setHeightForWidth(lblPrecision->sizePolicy().hasHeightForWidth());
        lblPrecision->setSizePolicy(sizePolicy);

        horizontalLayout_4->addWidget(lblPrecision);

        horizontalSpacer_7 = new QSpacerItem(5, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_7);

        spinPrecision = new QSpinBox(ScanObject);
        spinPrecision->setObjectName(QString::fromUtf8("spinPrecision"));
        spinPrecision->setEnabled(false);
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(spinPrecision->sizePolicy().hasHeightForWidth());
        spinPrecision->setSizePolicy(sizePolicy2);
        spinPrecision->setMinimum(0);
        spinPrecision->setMaximum(10);
        spinPrecision->setValue(0);

        horizontalLayout_4->addWidget(spinPrecision);


        gridLayout->addLayout(horizontalLayout_4, 1, 4, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        lblParameter = new QLabel(ScanObject);
        lblParameter->setObjectName(QString::fromUtf8("lblParameter"));
        lblParameter->setEnabled(false);
        sizePolicy.setHeightForWidth(lblParameter->sizePolicy().hasHeightForWidth());
        lblParameter->setSizePolicy(sizePolicy);

        horizontalLayout_2->addWidget(lblParameter);

        horizontalSpacer_4 = new QSpacerItem(5, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_4);

        dispParameter = new QLineEdit(ScanObject);
        dispParameter->setObjectName(QString::fromUtf8("dispParameter"));
        dispParameter->setEnabled(false);
        sizePolicy.setHeightForWidth(dispParameter->sizePolicy().hasHeightForWidth());
        dispParameter->setSizePolicy(sizePolicy);

        horizontalLayout_2->addWidget(dispParameter);


        gridLayout->addLayout(horizontalLayout_2, 0, 4, 1, 1);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(0);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        lblSteps = new QLabel(ScanObject);
        lblSteps->setObjectName(QString::fromUtf8("lblSteps"));
        lblSteps->setEnabled(false);
        sizePolicy.setHeightForWidth(lblSteps->sizePolicy().hasHeightForWidth());
        lblSteps->setSizePolicy(sizePolicy);

        horizontalLayout_5->addWidget(lblSteps);

        horizontalSpacer_8 = new QSpacerItem(5, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_8);

        spinSteps = new QSpinBox(ScanObject);
        spinSteps->setObjectName(QString::fromUtf8("spinSteps"));
        spinSteps->setEnabled(false);
        sizePolicy2.setHeightForWidth(spinSteps->sizePolicy().hasHeightForWidth());
        spinSteps->setSizePolicy(sizePolicy2);
        spinSteps->setKeyboardTracking(false);
        spinSteps->setMinimum(0);
        spinSteps->setMaximum(1000000);
        spinSteps->setValue(0);

        horizontalLayout_5->addWidget(spinSteps);


        gridLayout->addLayout(horizontalLayout_5, 1, 2, 1, 1);

        comboScript = new QComboBox(ScanObject);
        comboScript->setObjectName(QString::fromUtf8("comboScript"));

        gridLayout->addWidget(comboScript, 0, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        dispScript = new QLineEdit(ScanObject);
        dispScript->setObjectName(QString::fromUtf8("dispScript"));
        dispScript->setEnabled(false);

        horizontalLayout->addWidget(dispScript);

        horizontalSpacer_2 = new QSpacerItem(5, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        btnBrowse = new QPushButton(ScanObject);
        btnBrowse->setObjectName(QString::fromUtf8("btnBrowse"));
        btnBrowse->setEnabled(false);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/images/browse.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnBrowse->setIcon(icon);

        horizontalLayout->addWidget(btnBrowse);


        gridLayout->addLayout(horizontalLayout, 0, 2, 1, 1);

        horizontalSpacer = new QSpacerItem(30, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 0, 1, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_3, 0, 3, 1, 1);

        QWidget::setTabOrder(comboScript, dispScript);
        QWidget::setTabOrder(dispScript, btnBrowse);
        QWidget::setTabOrder(btnBrowse, dispParameter);
        QWidget::setTabOrder(dispParameter, spinSteps);
        QWidget::setTabOrder(spinSteps, spinPrecision);
        QWidget::setTabOrder(spinPrecision, radioRange);
        QWidget::setTabOrder(radioRange, radioCustom);
        QWidget::setTabOrder(radioCustom, radioFile);

        retranslateUi(ScanObject);

        QMetaObject::connectSlotsByName(ScanObject);
    } // setupUi

    void retranslateUi(QWidget *ScanObject)
    {
        ScanObject->setWindowTitle(QApplication::translate("ScanObject", "Form", 0, QApplication::UnicodeUTF8));
        group->setTitle(QString());
#ifndef QT_NO_TOOLTIP
        radioRange->setToolTip(QApplication::translate("ScanObject", "<nobr>\n"
"Defines scan range for a <b>Constant Step Size</b> with the following constraints:\n"
"</nobr><br><nobr>\n"
"1. <b>Number of Steps</b> >=2.\n"
"</nobr><br><nobr>\n"
"2. <b>Size</b> not equal to 0.\n"
"</nobr><br><nobr>\n"
"3. <b>From</b> not equal to <b>To</b>.\n"
"</nobr><br>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        radioRange->setText(QApplication::translate("ScanObject", "Constant Step Size", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        radioCustom->setToolTip(QApplication::translate("ScanObject", "<nobr>Measures only at specific values listed by the user.</nobr><br>\n"
"<nobr>Number of entries is restricted to <b>Number of Steps</b> field.</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        radioCustom->setText(QApplication::translate("ScanObject", "Specific Values", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        radioFile->setToolTip(QApplication::translate("ScanObject", "<nobr>Measures only at the specific values listed in a file.</nobr><br>\n"
"<nobr>Select the file, where these values are listed.</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        radioFile->setText(QApplication::translate("ScanObject", "Values from File", 0, QApplication::UnicodeUTF8));
        lblPrecision->setText(QApplication::translate("ScanObject", "Precision:", 0, QApplication::UnicodeUTF8));
        lblParameter->setText(QApplication::translate("ScanObject", "Additional Parameter:", 0, QApplication::UnicodeUTF8));
        lblSteps->setText(QApplication::translate("ScanObject", "Number of Steps:", 0, QApplication::UnicodeUTF8));
        comboScript->clear();
        comboScript->insertItems(0, QStringList()
         << QApplication::translate("ScanObject", "None", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ScanObject", "Energy Scan (eV)", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ScanObject", "Threshold Scan", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ScanObject", "Trimbits Scan", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ScanObject", "Position Scan", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ScanObject", "Custom Script", 0, QApplication::UnicodeUTF8)
        );
        btnBrowse->setText(QApplication::translate("ScanObject", "Browse", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ScanObject: public Ui_ScanObject {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORM_SCAN_H
