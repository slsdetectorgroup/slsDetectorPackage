/********************************************************************************
** Form generated from reading UI file 'form_tab_dataoutput.ui'
**
** Created: Tue Jul 25 12:31:25 2017
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORM_TAB_DATAOUTPUT_H
#define UI_FORM_TAB_DATAOUTPUT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TabDataOutputObject
{
public:
    QGroupBox *boxCorrection_2;
    QWidget *gridLayoutWidget_3;
    QGridLayout *gridLayout_3;
    QCheckBox *chkTenGiga;
    QCheckBox *chkCompression;
    QCheckBox *chkAngular;
    QCheckBox *chkDiscardBad;
    QWidget *gridLayoutWidget_2;
    QGridLayout *gridLayout_2;
    QSpacerItem *horizontalSpacer_2;
    QCheckBox *chkRate;
    QPushButton *btnFlatField;
    QLineEdit *dispFlatField;
    QCheckBox *chkFlatField;
    QSpacerItem *horizontalSpacer_3;
    QLabel *lblDeadTime;
    QSpinBox *spinDeadTime;
    QPushButton *btnDefaultRate;
    QFrame *line;
    QWidget *widgetEiger;
    QWidget *gridLayoutWidget;
    QGridLayout *gridEiger;
    QComboBox *comboEigerFlags1;
    QLabel *lblClkDivider;
    QSpacerItem *horizontalSpacer_4;
    QComboBox *comboEigerClkDivider;
    QSpacerItem *horizontalSpacer_5;
    QLabel *lblEigerFlags;
    QComboBox *comboEigerFlags2;
    QGroupBox *boxFileWriteEnabled;
    QWidget *horizontalLayoutWidget;
    QGridLayout *gridLayout;
    QPushButton *btnOutputBrowse;
    QComboBox *comboFileFormat;
    QSpacerItem *horizontalSpacer_10;
    QComboBox *comboDetector;
    QLabel *lblOutputDir;
    QLabel *lblFileFormat;
    QCheckBox *chkOverwriteEnable;
    QSpacerItem *horizontalSpacer_9;
    QLabel *lblFileName;
    QLineEdit *dispFileName;
    QLineEdit *dispOutputDir;

    void setupUi(QWidget *TabDataOutputObject)
    {
        if (TabDataOutputObject->objectName().isEmpty())
            TabDataOutputObject->setObjectName(QString::fromUtf8("TabDataOutputObject"));
        TabDataOutputObject->resize(775, 345);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(TabDataOutputObject->sizePolicy().hasHeightForWidth());
        TabDataOutputObject->setSizePolicy(sizePolicy);
        TabDataOutputObject->setMinimumSize(QSize(0, 0));
        TabDataOutputObject->setMaximumSize(QSize(1000, 1000));
        boxCorrection_2 = new QGroupBox(TabDataOutputObject);
        boxCorrection_2->setObjectName(QString::fromUtf8("boxCorrection_2"));
        boxCorrection_2->setGeometry(QRect(15, 170, 746, 170));
        gridLayoutWidget_3 = new QWidget(boxCorrection_2);
        gridLayoutWidget_3->setObjectName(QString::fromUtf8("gridLayoutWidget_3"));
        gridLayoutWidget_3->setGeometry(QRect(17, 21, 192, 137));
        gridLayout_3 = new QGridLayout(gridLayoutWidget_3);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout_3->setVerticalSpacing(6);
        gridLayout_3->setContentsMargins(0, 0, 0, 0);
        chkTenGiga = new QCheckBox(gridLayoutWidget_3);
        chkTenGiga->setObjectName(QString::fromUtf8("chkTenGiga"));
        chkTenGiga->setEnabled(false);

        gridLayout_3->addWidget(chkTenGiga, 3, 0, 1, 1);

        chkCompression = new QCheckBox(gridLayoutWidget_3);
        chkCompression->setObjectName(QString::fromUtf8("chkCompression"));

        gridLayout_3->addWidget(chkCompression, 2, 0, 1, 1);

        chkAngular = new QCheckBox(gridLayoutWidget_3);
        chkAngular->setObjectName(QString::fromUtf8("chkAngular"));
        chkAngular->setEnabled(false);

        gridLayout_3->addWidget(chkAngular, 0, 0, 1, 1);

        chkDiscardBad = new QCheckBox(gridLayoutWidget_3);
        chkDiscardBad->setObjectName(QString::fromUtf8("chkDiscardBad"));

        gridLayout_3->addWidget(chkDiscardBad, 1, 0, 1, 1);

        gridLayoutWidget_2 = new QWidget(boxCorrection_2);
        gridLayoutWidget_2->setObjectName(QString::fromUtf8("gridLayoutWidget_2"));
        gridLayoutWidget_2->setGeometry(QRect(255, 21, 476, 76));
        gridLayout_2 = new QGridLayout(gridLayoutWidget_2);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setVerticalSpacing(6);
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer_2 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_2, 1, 4, 1, 1);

        chkRate = new QCheckBox(gridLayoutWidget_2);
        chkRate->setObjectName(QString::fromUtf8("chkRate"));
        chkRate->setEnabled(false);

        gridLayout_2->addWidget(chkRate, 1, 0, 1, 1);

        btnFlatField = new QPushButton(gridLayoutWidget_2);
        btnFlatField->setObjectName(QString::fromUtf8("btnFlatField"));
        btnFlatField->setEnabled(false);
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(btnFlatField->sizePolicy().hasHeightForWidth());
        btnFlatField->setSizePolicy(sizePolicy1);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/images/browse.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnFlatField->setIcon(icon);

        gridLayout_2->addWidget(btnFlatField, 0, 8, 1, 1);

        dispFlatField = new QLineEdit(gridLayoutWidget_2);
        dispFlatField->setObjectName(QString::fromUtf8("dispFlatField"));
        dispFlatField->setEnabled(false);
        dispFlatField->setFocusPolicy(Qt::ClickFocus);

        gridLayout_2->addWidget(dispFlatField, 0, 2, 1, 6);

        chkFlatField = new QCheckBox(gridLayoutWidget_2);
        chkFlatField->setObjectName(QString::fromUtf8("chkFlatField"));

        gridLayout_2->addWidget(chkFlatField, 0, 0, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_3, 0, 1, 1, 1);

        lblDeadTime = new QLabel(gridLayoutWidget_2);
        lblDeadTime->setObjectName(QString::fromUtf8("lblDeadTime"));
        lblDeadTime->setEnabled(false);
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(lblDeadTime->sizePolicy().hasHeightForWidth());
        lblDeadTime->setSizePolicy(sizePolicy2);
        QPalette palette;
        QBrush brush(QColor(2, 2, 2, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
        QBrush brush1(QColor(0, 0, 0, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush1);
        QBrush brush2(QColor(119, 119, 119, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush2);
        lblDeadTime->setPalette(palette);

        gridLayout_2->addWidget(lblDeadTime, 1, 5, 1, 1);

        spinDeadTime = new QSpinBox(gridLayoutWidget_2);
        spinDeadTime->setObjectName(QString::fromUtf8("spinDeadTime"));
        sizePolicy.setHeightForWidth(spinDeadTime->sizePolicy().hasHeightForWidth());
        spinDeadTime->setSizePolicy(sizePolicy);
        spinDeadTime->setMinimum(-1);
        spinDeadTime->setMaximum(200000000);

        gridLayout_2->addWidget(spinDeadTime, 1, 6, 1, 3);

        btnDefaultRate = new QPushButton(gridLayoutWidget_2);
        btnDefaultRate->setObjectName(QString::fromUtf8("btnDefaultRate"));
        btnDefaultRate->setEnabled(false);
        sizePolicy1.setHeightForWidth(btnDefaultRate->sizePolicy().hasHeightForWidth());
        btnDefaultRate->setSizePolicy(sizePolicy1);
        btnDefaultRate->setMaximumSize(QSize(16777215, 30));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/images/calculate.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnDefaultRate->setIcon(icon1);

        gridLayout_2->addWidget(btnDefaultRate, 1, 2, 1, 2);

        line = new QFrame(boxCorrection_2);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(230, 21, 20, 134));
        line->setFrameShadow(QFrame::Raised);
        line->setFrameShape(QFrame::VLine);
        widgetEiger = new QWidget(boxCorrection_2);
        widgetEiger->setObjectName(QString::fromUtf8("widgetEiger"));
        widgetEiger->setGeometry(QRect(244, 92, 491, 72));
        gridLayoutWidget = new QWidget(widgetEiger);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(11, 4, 475, 62));
        gridEiger = new QGridLayout(gridLayoutWidget);
        gridEiger->setObjectName(QString::fromUtf8("gridEiger"));
        gridEiger->setContentsMargins(0, 0, 0, 0);
        comboEigerFlags1 = new QComboBox(gridLayoutWidget);
        comboEigerFlags1->setObjectName(QString::fromUtf8("comboEigerFlags1"));
        comboEigerFlags1->setMaximumSize(QSize(200, 16777215));

        gridEiger->addWidget(comboEigerFlags1, 1, 2, 1, 1);

        lblClkDivider = new QLabel(gridLayoutWidget);
        lblClkDivider->setObjectName(QString::fromUtf8("lblClkDivider"));
        lblClkDivider->setMaximumSize(QSize(105, 16777215));

        gridEiger->addWidget(lblClkDivider, 0, 0, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridEiger->addItem(horizontalSpacer_4, 0, 1, 1, 1);

        comboEigerClkDivider = new QComboBox(gridLayoutWidget);
        comboEigerClkDivider->setObjectName(QString::fromUtf8("comboEigerClkDivider"));
        comboEigerClkDivider->setMaximumSize(QSize(200, 16777215));

        gridEiger->addWidget(comboEigerClkDivider, 0, 2, 1, 1);

        horizontalSpacer_5 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridEiger->addItem(horizontalSpacer_5, 1, 3, 1, 1);

        lblEigerFlags = new QLabel(gridLayoutWidget);
        lblEigerFlags->setObjectName(QString::fromUtf8("lblEigerFlags"));
        lblEigerFlags->setMaximumSize(QSize(105, 16777215));

        gridEiger->addWidget(lblEigerFlags, 1, 0, 1, 1);

        comboEigerFlags2 = new QComboBox(gridLayoutWidget);
        comboEigerFlags2->setObjectName(QString::fromUtf8("comboEigerFlags2"));

        gridEiger->addWidget(comboEigerFlags2, 1, 4, 1, 1);

        boxFileWriteEnabled = new QGroupBox(TabDataOutputObject);
        boxFileWriteEnabled->setObjectName(QString::fromUtf8("boxFileWriteEnabled"));
        boxFileWriteEnabled->setGeometry(QRect(20, 40, 735, 100));
        horizontalLayoutWidget = new QWidget(boxFileWriteEnabled);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(12, 20, 714, 68));
        gridLayout = new QGridLayout(horizontalLayoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setVerticalSpacing(4);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        btnOutputBrowse = new QPushButton(horizontalLayoutWidget);
        btnOutputBrowse->setObjectName(QString::fromUtf8("btnOutputBrowse"));
        sizePolicy1.setHeightForWidth(btnOutputBrowse->sizePolicy().hasHeightForWidth());
        btnOutputBrowse->setSizePolicy(sizePolicy1);
        btnOutputBrowse->setIcon(icon);

        gridLayout->addWidget(btnOutputBrowse, 0, 16, 1, 1);

        comboFileFormat = new QComboBox(horizontalLayoutWidget);
        comboFileFormat->setObjectName(QString::fromUtf8("comboFileFormat"));
        sizePolicy1.setHeightForWidth(comboFileFormat->sizePolicy().hasHeightForWidth());
        comboFileFormat->setSizePolicy(sizePolicy1);
        comboFileFormat->setMinimumSize(QSize(105, 0));

        gridLayout->addWidget(comboFileFormat, 1, 1, 1, 1);

        horizontalSpacer_10 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_10, 1, 2, 1, 1);

        comboDetector = new QComboBox(horizontalLayoutWidget);
        comboDetector->setObjectName(QString::fromUtf8("comboDetector"));
        sizePolicy1.setHeightForWidth(comboDetector->sizePolicy().hasHeightForWidth());
        comboDetector->setSizePolicy(sizePolicy1);
        comboDetector->setMinimumSize(QSize(105, 0));

        gridLayout->addWidget(comboDetector, 0, 1, 1, 1);

        lblOutputDir = new QLabel(horizontalLayoutWidget);
        lblOutputDir->setObjectName(QString::fromUtf8("lblOutputDir"));
        sizePolicy2.setHeightForWidth(lblOutputDir->sizePolicy().hasHeightForWidth());
        lblOutputDir->setSizePolicy(sizePolicy2);
        lblOutputDir->setMaximumSize(QSize(70, 16777215));
        QPalette palette1;
        palette1.setBrush(QPalette::Active, QPalette::WindowText, brush);
        palette1.setBrush(QPalette::Inactive, QPalette::WindowText, brush1);
        palette1.setBrush(QPalette::Disabled, QPalette::WindowText, brush2);
        lblOutputDir->setPalette(palette1);

        gridLayout->addWidget(lblOutputDir, 0, 0, 1, 1);

        lblFileFormat = new QLabel(horizontalLayoutWidget);
        lblFileFormat->setObjectName(QString::fromUtf8("lblFileFormat"));
        sizePolicy2.setHeightForWidth(lblFileFormat->sizePolicy().hasHeightForWidth());
        lblFileFormat->setSizePolicy(sizePolicy2);
        lblFileFormat->setMaximumSize(QSize(70, 16777215));
        QPalette palette2;
        palette2.setBrush(QPalette::Active, QPalette::WindowText, brush);
        palette2.setBrush(QPalette::Inactive, QPalette::WindowText, brush1);
        palette2.setBrush(QPalette::Disabled, QPalette::WindowText, brush2);
        lblFileFormat->setPalette(palette2);

        gridLayout->addWidget(lblFileFormat, 1, 0, 1, 1);

        chkOverwriteEnable = new QCheckBox(horizontalLayoutWidget);
        chkOverwriteEnable->setObjectName(QString::fromUtf8("chkOverwriteEnable"));
        sizePolicy1.setHeightForWidth(chkOverwriteEnable->sizePolicy().hasHeightForWidth());
        chkOverwriteEnable->setSizePolicy(sizePolicy1);
        chkOverwriteEnable->setMaximumSize(QSize(16777215, 16777215));

        gridLayout->addWidget(chkOverwriteEnable, 1, 16, 1, 1);

        horizontalSpacer_9 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_9, 1, 15, 1, 1);

        lblFileName = new QLabel(horizontalLayoutWidget);
        lblFileName->setObjectName(QString::fromUtf8("lblFileName"));
        sizePolicy2.setHeightForWidth(lblFileName->sizePolicy().hasHeightForWidth());
        lblFileName->setSizePolicy(sizePolicy2);
        lblFileName->setMaximumSize(QSize(75, 16777215));
        QPalette palette3;
        palette3.setBrush(QPalette::Active, QPalette::WindowText, brush);
        palette3.setBrush(QPalette::Inactive, QPalette::WindowText, brush1);
        palette3.setBrush(QPalette::Disabled, QPalette::WindowText, brush2);
        lblFileName->setPalette(palette3);

        gridLayout->addWidget(lblFileName, 1, 3, 1, 1);

        dispFileName = new QLineEdit(horizontalLayoutWidget);
        dispFileName->setObjectName(QString::fromUtf8("dispFileName"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(dispFileName->sizePolicy().hasHeightForWidth());
        dispFileName->setSizePolicy(sizePolicy3);
        dispFileName->setMaximumSize(QSize(16777215, 16777215));
        dispFileName->setFocusPolicy(Qt::NoFocus);
        dispFileName->setFrame(false);
        dispFileName->setEchoMode(QLineEdit::Normal);
        dispFileName->setReadOnly(true);

        gridLayout->addWidget(dispFileName, 1, 4, 1, 11);

        dispOutputDir = new QLineEdit(horizontalLayoutWidget);
        dispOutputDir->setObjectName(QString::fromUtf8("dispOutputDir"));
        dispOutputDir->setFocusPolicy(Qt::StrongFocus);

        gridLayout->addWidget(dispOutputDir, 0, 3, 1, 12);

        QWidget::setTabOrder(chkFlatField, dispFlatField);
        QWidget::setTabOrder(dispFlatField, btnFlatField);
        QWidget::setTabOrder(btnFlatField, chkRate);

        retranslateUi(TabDataOutputObject);

        QMetaObject::connectSlotsByName(TabDataOutputObject);
    } // setupUi

    void retranslateUi(QWidget *TabDataOutputObject)
    {
        TabDataOutputObject->setWindowTitle(QApplication::translate("TabDataOutputObject", "Form", 0, QApplication::UnicodeUTF8));
        boxCorrection_2->setTitle(QApplication::translate("TabDataOutputObject", "Options", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        chkTenGiga->setToolTip(QApplication::translate("TabDataOutputObject", "<nobr>\n"
"Compression using Root. Available only for Gotthard in Expert Mode.\n"
"</nobr><br><nobr>\n"
" #r_compression#\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        chkTenGiga->setText(QApplication::translate("TabDataOutputObject", "10GbE", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        chkCompression->setToolTip(QApplication::translate("TabDataOutputObject", "<nobr>\n"
"Compression using Root. Available only for Gotthard in Expert Mode.\n"
"</nobr><br><nobr>\n"
" #r_compression#\n"
"</nobr>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        chkCompression->setText(QApplication::translate("TabDataOutputObject", "Compression", 0, QApplication::UnicodeUTF8));
        chkAngular->setText(QApplication::translate("TabDataOutputObject", "Angular Conversion", 0, QApplication::UnicodeUTF8));
        chkDiscardBad->setText(QApplication::translate("TabDataOutputObject", "Discard Bad Channels", 0, QApplication::UnicodeUTF8));
        chkRate->setText(QApplication::translate("TabDataOutputObject", "Rate:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        btnFlatField->setToolTip(QApplication::translate("TabDataOutputObject", "Flat field corrections. \n"
" #flatfield# filename", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnFlatField->setText(QApplication::translate("TabDataOutputObject", "Browse", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        dispFlatField->setToolTip(QApplication::translate("TabDataOutputObject", "Flat field corrections. \n"
" #flatfield# filename", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        chkFlatField->setToolTip(QApplication::translate("TabDataOutputObject", "Flat field corrections. \n"
" #flatfield# filename", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        chkFlatField->setText(QApplication::translate("TabDataOutputObject", "Flat Field File:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lblDeadTime->setToolTip(QApplication::translate("TabDataOutputObject", "<nobr>\n"
"Directory where one saves the data.\n"
"</nobr><br>\n"
" #outdir#\n"
"<br>\n"
"", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        lblDeadTime->setText(QApplication::translate("TabDataOutputObject", "Dead Time:", 0, QApplication::UnicodeUTF8));
        spinDeadTime->setSuffix(QApplication::translate("TabDataOutputObject", "ns", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        btnDefaultRate->setToolTip(QApplication::translate("TabDataOutputObject", "<nobr>\n"
"Directory where one saves the data.\n"
"</nobr><br>\n"
" #outdir#\n"
"<br>\n"
"", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnDefaultRate->setText(QApplication::translate("TabDataOutputObject", "Default", 0, QApplication::UnicodeUTF8));
        comboEigerFlags1->clear();
        comboEigerFlags1->insertItems(0, QStringList()
         << QApplication::translate("TabDataOutputObject", "Continous", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabDataOutputObject", "Store in RAM", 0, QApplication::UnicodeUTF8)
        );
        lblClkDivider->setText(QApplication::translate("TabDataOutputObject", "Clock Divider:", 0, QApplication::UnicodeUTF8));
        comboEigerClkDivider->clear();
        comboEigerClkDivider->insertItems(0, QStringList()
         << QApplication::translate("TabDataOutputObject", "Full Speed", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabDataOutputObject", "Half Speed", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabDataOutputObject", "Quarter Speed", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabDataOutputObject", "Super Slow Speed", 0, QApplication::UnicodeUTF8)
        );
        lblEigerFlags->setText(QApplication::translate("TabDataOutputObject", "Flags:", 0, QApplication::UnicodeUTF8));
        comboEigerFlags2->clear();
        comboEigerFlags2->insertItems(0, QStringList()
         << QApplication::translate("TabDataOutputObject", "Parallel", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabDataOutputObject", "Non Parallel", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabDataOutputObject", "Safe", 0, QApplication::UnicodeUTF8)
        );
        boxFileWriteEnabled->setTitle(QApplication::translate("TabDataOutputObject", "File", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        btnOutputBrowse->setToolTip(QApplication::translate("TabDataOutputObject", "<nobr>\n"
"Directory where one saves the data.\n"
"</nobr><br>\n"
" #outdir#\n"
"<br>\n"
"Disabled if a receiver is utilized in acquisition.\n"
"<br>\n"
"", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        btnOutputBrowse->setText(QApplication::translate("TabDataOutputObject", "Browse", 0, QApplication::UnicodeUTF8));
        comboFileFormat->clear();
        comboFileFormat->insertItems(0, QStringList()
         << QApplication::translate("TabDataOutputObject", "Binary", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabDataOutputObject", "ASCII", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("TabDataOutputObject", "HDF5", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        comboFileFormat->setToolTip(QApplication::translate("TabDataOutputObject", "<html><head/><body><p>File Format<br/>#fileformat# <br/></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        lblOutputDir->setToolTip(QApplication::translate("TabDataOutputObject", "<nobr>\n"
"Directory where one saves the data.\n"
"</nobr><br>\n"
" #outdir#\n"
"<br>\n"
"", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        lblOutputDir->setText(QApplication::translate("TabDataOutputObject", "Path:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lblFileFormat->setToolTip(QApplication::translate("TabDataOutputObject", "<html><head/><body><p>File Format<br/>#fileformat# <br/></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        lblFileFormat->setText(QApplication::translate("TabDataOutputObject", "Format:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        chkOverwriteEnable->setToolTip(QApplication::translate("TabDataOutputObject", "Overwrite Enable\n"
" #overwrite#", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        chkOverwriteEnable->setText(QApplication::translate("TabDataOutputObject", "Overwrite", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lblFileName->setToolTip(QApplication::translate("TabDataOutputObject", "<html><head/><body><p>Name of file. Can be modified in Measurement tab.<br/>#fname# <br/></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        lblFileName->setText(QApplication::translate("TabDataOutputObject", "Name Prefix:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        dispFileName->setToolTip(QApplication::translate("TabDataOutputObject", "<html><head/><body><p>Name of file. Can be modified in Measurement tab.<br/>#fname# <br/></p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        dispOutputDir->setToolTip(QApplication::translate("TabDataOutputObject", "<nobr>\n"
"Directory where one saves the data.\n"
"</nobr><br>\n"
" #outdir#\n"
"<br>\n"
"", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
    } // retranslateUi

};

namespace Ui {
    class TabDataOutputObject: public Ui_TabDataOutputObject {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORM_TAB_DATAOUTPUT_H
