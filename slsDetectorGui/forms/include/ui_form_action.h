/********************************************************************************
** Form generated from reading UI file 'form_action.ui'
**
** Created: Tue Jul 25 12:31:25 2017
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORM_ACTION_H
#define UI_FORM_ACTION_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ActionsObject
{
public:
    QGridLayout *gridLayout;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *horizontalSpacer;
    QComboBox *comboScript;
    QLineEdit *dispScript;
    QLabel *lblParameter;
    QPushButton *btnBrowse;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *horizontalSpacer_4;
    QLineEdit *dispParameter;

    void setupUi(QWidget *ActionsObject)
    {
        if (ActionsObject->objectName().isEmpty())
            ActionsObject->setObjectName(QString::fromUtf8("ActionsObject"));
        ActionsObject->resize(680, 25);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ActionsObject->sizePolicy().hasHeightForWidth());
        ActionsObject->setSizePolicy(sizePolicy);
        ActionsObject->setMinimumSize(QSize(0, 0));
        ActionsObject->setMaximumSize(QSize(1000, 1000));
        gridLayout = new QGridLayout(ActionsObject);
        gridLayout->setSpacing(0);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        horizontalSpacer_3 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_3, 0, 5, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 0, 1, 1, 1);

        comboScript = new QComboBox(ActionsObject);
        comboScript->setObjectName(QString::fromUtf8("comboScript"));

        gridLayout->addWidget(comboScript, 0, 0, 1, 1);

        dispScript = new QLineEdit(ActionsObject);
        dispScript->setObjectName(QString::fromUtf8("dispScript"));
        dispScript->setEnabled(false);

        gridLayout->addWidget(dispScript, 0, 2, 1, 1);

        lblParameter = new QLabel(ActionsObject);
        lblParameter->setObjectName(QString::fromUtf8("lblParameter"));
        lblParameter->setEnabled(false);

        gridLayout->addWidget(lblParameter, 0, 6, 1, 1);

        btnBrowse = new QPushButton(ActionsObject);
        btnBrowse->setObjectName(QString::fromUtf8("btnBrowse"));
        btnBrowse->setEnabled(false);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/images/browse.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnBrowse->setIcon(icon);

        gridLayout->addWidget(btnBrowse, 0, 4, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(5, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_2, 0, 3, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(5, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_4, 0, 7, 1, 1);

        dispParameter = new QLineEdit(ActionsObject);
        dispParameter->setObjectName(QString::fromUtf8("dispParameter"));
        dispParameter->setEnabled(false);
        sizePolicy.setHeightForWidth(dispParameter->sizePolicy().hasHeightForWidth());
        dispParameter->setSizePolicy(sizePolicy);

        gridLayout->addWidget(dispParameter, 0, 8, 1, 1);


        retranslateUi(ActionsObject);

        QMetaObject::connectSlotsByName(ActionsObject);
    } // setupUi

    void retranslateUi(QWidget *ActionsObject)
    {
        ActionsObject->setWindowTitle(QApplication::translate("ActionsObject", "Form", 0, QApplication::UnicodeUTF8));
        comboScript->clear();
        comboScript->insertItems(0, QStringList()
         << QApplication::translate("ActionsObject", "None", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ActionsObject", "Custom Script", 0, QApplication::UnicodeUTF8)
        );
        lblParameter->setText(QApplication::translate("ActionsObject", "Additional Parameter:", 0, QApplication::UnicodeUTF8));
        btnBrowse->setText(QApplication::translate("ActionsObject", "Browse", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ActionsObject: public Ui_ActionsObject {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORM_ACTION_H
