#include "qTabDataOutput.h"

#include "multiSlsDetector.h"

#include <QFileDialog>
#include <QStandardItemModel>

#include <iostream>
#include <string>


qTabDataOutput::qTabDataOutput(QWidget *parent, multiSlsDetector *detector) : QWidget(parent), myDet(detector) {
	setupUi(this);
	SetupWidgetWindow();
	FILE_LOG(logDEBUG) << "DataOutput ready";
}


qTabDataOutput::~qTabDataOutput() {}


void qTabDataOutput::SetupWidgetWindow() {
	// palette
	red = QPalette();
	red.setColor(QPalette::Active, QPalette::WindowText, Qt::red);

	// enabling according to det type
	switch((int)myDet->getDetectorTypeAsEnum()) {
		case slsDetectorDefs::EIGER:
			chkTenGiga->setEnabled(true);
			chkRate->setEnabled(true);
			widgetEiger->setVisible(true);
			widgetEiger->setEnabled(true);
			break;
		case slsDetectorDefs::MOENCH:
			chkTenGiga->setEnabled(true);
			break;
		default:
			break;	
	}
	PopulateDetectors();
	EnableBrowse();
	Initialization();

	GetOutputDir();
	GetFileOverwrite();
	GetFileFormat();
	if (chkRate->isEnabled()) {
		GetRateCorrection();
	}
	if (chkTenGiga->isEnabled()) {
		GetTenGigaEnable();
	}
	if (widgetEiger->isEnabled()) {
		GetSpeed();
		GetFlags();
	}
	Refresh();
}

void qTabDataOutput::Initialization() {
	connect(comboDetector, SIGNAL(currentIndexChanged(int)), this, SLOT(GetOutputDir()));
	connect(dispOutputDir, SIGNAL(editingFinished()), this, SLOT(SetOutputDir()));
	connect(btnOutputBrowse, SIGNAL(clicked()), this, SLOT(BrowseOutputDir()));
	connect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFileFormat(int)));
	connect(chkOverwriteEnable, SIGNAL(toggled(bool)), this, SLOT(SetOverwriteEnable(bool)));
	if (chkTenGiga->isEnabled()) {
		connect(chkTenGiga, SIGNAL(toggled(bool)), this, SLOT(SetTenGigaEnable(bool)));
	}
	if (chkRate->isEnabled()) {
		connect(chkRate, SIGNAL(toggled(bool)), this, SLOT(SetRateCorrection()));
		connect(btnDefaultRate, SIGNAL(clicked()), this, SLOT(SetDefaultRateCorrection()));
		connect(spinDeadTime, SIGNAL(editingFinished()), this, SLOT(SetRateCorrection()));
	}
	if (widgetEiger->isEnabled()) {
		connect(comboEigerClkDivider, SIGNAL(currentIndexChanged(int)), this, SLOT(SetSpeed()));
		connect(comboEigerFlags1, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFlags()));
		connect(comboEigerFlags2, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFlags()));
	}
}

void qTabDataOutput::PopulateDetectors() {
	FILE_LOG(logDEBUG) << "Populating detectors";

	comboDetector->clear();
	comboDetector->addItem("All");
	if (myDet->getNumberOfDetectors() > 1) {
		for (int i = 0; i < myDet->getNumberOfDetectors(); ++i)
			comboDetector->addItem(QString(myDet->getHostname(i).c_str()));
	}
}

void qTabDataOutput::EnableBrowse() {
	try {
		std::string receiverHostname = myDet->getReceiverHostname(comboDetector->currentIndex() - 1);
		if (receiverHostname == "localhost") {
			btnOutputBrowse->setEnabled(true);
		} else {
			std::string hostname;
			const size_t len = 15;
			char host[len]{};
			if (gethostname(host, len) == 0) {
				hostname.assign(host);
			}
			// client pc (hostname) same as reciever hostname
			if (hostname == receiverHostname) {
				btnOutputBrowse->setEnabled(true);
			} else {
				btnOutputBrowse->setEnabled(false);
			}
		}
	} catch (const sls::RuntimeError &e) {
		qDefs::ExceptionMessage("Could not get receiver hostname.", e.what(), "qTabDataOutput::EnableBrowse");
		btnOutputBrowse->setEnabled(false);
    }
}


void qTabDataOutput::GetOutputDir() {
	FILE_LOG(logDEBUG) << "Getting file path";

	disconnect(dispOutputDir, SIGNAL(editingFinished()), this, SLOT(SetOutputDir()));

	try {
        std::string path = myDet->getFilePath(comboDetector->currentIndex() - 1);
		dispOutputDir->setText(QString(path.c_str()));
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get file path.", e.what(), "qTabDataOutput::GetOutputDir");
    }

	connect(dispOutputDir, SIGNAL(editingFinished()), this, SLOT(SetOutputDir()));
}

void qTabDataOutput::BrowseOutputDir() {
	FILE_LOG(logDEBUG) << "Browsing output directory";
	QString directory = QFileDialog::getExistingDirectory(this, tr("Choose Output Directory "), dispOutputDir->text());
	if (!directory.isEmpty())
		dispOutputDir->setText(directory);
}

void qTabDataOutput::SetOutputDir() {
	QString path = dispOutputDir->text();
	FILE_LOG(logDEBUG) << "Setting output directory to " << path.toAscii().constData();

	// empty
	if (path.isEmpty()) {
		qDefs::Message(qDefs::WARNING, "Invalid Output Path. Must not be empty.", "qTabDataOutput::SetOutputDir");
		FILE_LOG(logWARNING) << "Invalid Output Path. Must not be empty.";
		GetOutputDir();
	} else {
		// chop off trailing '/'
		if (path.endsWith('/')) {
			while (path.endsWith('/')) {
				path.chop(1);
			}
		}
		std::string spath = std::string(path.toAscii().constData());
		try {
			myDet->setFilePath(spath, comboDetector->currentIndex() - 1);
		} catch (const sls::NonCriticalError &e) {
			qDefs::ExceptionMessage("Could not set output file path.", e.what(), "qTabDataOutput::SetOutputDir");
			GetOutputDir();
		}
	}
}

void qTabDataOutput::GetFileFormat() {
	FILE_LOG(logDEBUG) << "Getting File Format";
	disconnect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFileFormat(int)));

	qDefs::IgnoreNonCriticalExceptionsandMinus1<QComboBox>(
            myDet,
            "Could not get file format.",
            "qTabDataOutput::GetFileFormat",
            comboFileFormat,
            &QComboBox::setCurrentIndex,
            &multiSlsDetector::getFileFormat, -1);

	connect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFileFormat(int)));
}

void qTabDataOutput::SetFileFormat(int format) {
	FILE_LOG(logINFO) << "Setting File Format to " << slsDetectorDefs::getFileFormatType((slsDetectorDefs::fileFormat)format);
	try {
        myDet->setFileFormat((slsDetectorDefs::fileFormat)comboFileFormat->currentIndex());
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set file format.", e.what(), "qTabDataOutput::SetFileFormat");
        GetFileFormat();
    }
}

void qTabDataOutput::GetFileOverwrite() {
	FILE_LOG(logDEBUG) << "Getting File Over Write Enable";
	disconnect(chkOverwriteEnable, SIGNAL(toggled(bool)), this, SLOT(SetOverwriteEnable(bool)));

	try {
        int retval = myDet->getFileOverWrite();
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "File over write is inconsistent for all detectors.", "qTabDataOutput::GetFileOverwrite");
		} else {
			chkOverwriteEnable->setChecked(retval == 0 ? false : true);
		}
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get file over write enable.", e.what(), "qTabDataOutput::GetFileOverwrite");
    }

	connect(chkOverwriteEnable, SIGNAL(toggled(bool)), this, SLOT(SetOverwriteEnable(bool)));
}

void qTabDataOutput::SetOverwriteEnable(bool enable) {
	FILE_LOG(logINFO) << "Setting File Over Write Enable to " << enable;

	try {
        myDet->setFileOverWrite(enable);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set file over write enable.", e.what(), "qTabDataOutput::SetOverwriteEnable");
        GetFileOverwrite();
    }
}

void qTabDataOutput::GetTenGigaEnable() {
	FILE_LOG(logDEBUG) << "Getting 10GbE enable";
	disconnect(chkTenGiga, SIGNAL(toggled(bool)), this, SLOT(SetTenGigaEnable(bool)));

	try {
		int retval = myDet->enableTenGigabitEthernet();
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "10GbE enable is inconsistent for all detectors.", "qTabDataOutput::GetTenGigaEnable");
		} else {
			chkTenGiga->setChecked(retval == 0 ? false : true);
		}
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get 10GbE enable.", e.what(), "qTabDataOutput::GetTenGigaEnable");
    }

	connect(chkTenGiga, SIGNAL(toggled(bool)), this, SLOT(SetTenGigaEnable(bool)));
}

void qTabDataOutput::SetTenGigaEnable(bool enable) {
	FILE_LOG(logINFO) << "Setting 10GbE to " << enable;

	try {
        myDet->enableTenGigabitEthernet(enable);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set 10GbE enable.", e.what(), "qTabDataOutput::SetTenGigaEnable");
        GetTenGigaEnable();
    }
}







void qTabDataOutput::SetRateCorrection(int deadtime) {
	disconnect(btnDefaultRate, SIGNAL(clicked()), this, SLOT(SetDefaultRateCorrection()));
	disconnect(spinDeadTime, SIGNAL(editingFinished()), this, SLOT(SetRateCorrection()));

	FILE_LOG(logDEBUG) << "Entering Set Rate Correction function";

	if (chkRate->isChecked()) {
		if (!btnDefaultRate->isEnabled()) {
			btnDefaultRate->setEnabled(true);
			lblDeadTime->setEnabled(true);
			spinDeadTime->setEnabled(true);
		}

		if (deadtime != -1) {
			deadtime = (double)spinDeadTime->value();
			FILE_LOG(logINFO) << "Setting rate corrections with custom dead time: " << deadtime << '\n';
		} else {
			FILE_LOG(logINFO) << "Setting rate corrections with default dead time" << '\n';
		}
		myDet->setRateCorrection(deadtime);

	} //unsetting rate correction
	else {
		btnDefaultRate->setEnabled(false);
		lblDeadTime->setEnabled(false);
		spinDeadTime->setEnabled(false);
		myDet->setRateCorrection(0);
		FILE_LOG(logINFO) << "Unsetting rate correction";
	}
	qDefs::checkErrorMessage(myDet, "qTabDataOutput::SetRateCorrection");

	//update just the value
	double rate = (double)myDet->getRateCorrection();
	spinDeadTime->setValue((double)rate);
	if (rate == -1) {
		qDefs::Message(qDefs::WARNING, "Dead time is inconsistent for all detectors. Returned Value: -1.", "qTabDataOutput::GetRateCorrection");
		QString errorTip = QString("<nobr>Rate Corrections.</nobr><br>"
				"<nobr> #ratecorr# tau in seconds</nobr><br><br>") +
						QString("<nobr><font color=\"red\">"
								"Dead time is inconsistent for all detectors.</font></nobr>");
		chkRate->setToolTip(errorTip);
		spinDeadTime->setToolTip(errorTip);
		chkRate->setPalette(red);
		chkRate->setText("Rate:*");
	} else {
		QString normalTip = QString("<nobr>Rate Corrections.</nobr><br>"
				"<nobr> #ratecorr# tau in seconds</nobr><br><br>");
		chkRate->setToolTip(normalTip);
		spinDeadTime->setToolTip(normalTip);
		chkRate->setPalette(lblDeadTime->palette());
		chkRate->setText("Rate:");
	}

	connect(btnDefaultRate, SIGNAL(clicked()), this, SLOT(SetDefaultRateCorrection()));
	connect(spinDeadTime, SIGNAL(editingFinished()), this, SLOT(SetRateCorrection()));
}


void qTabDataOutput::SetDefaultRateCorrection() {
	SetRateCorrection(-1);
}


void qTabDataOutput::GetRateCorrection() {
	disconnect(chkRate, SIGNAL(toggled(bool)), this, SLOT(SetRateCorrection()));
	disconnect(btnDefaultRate, SIGNAL(clicked()), this, SLOT(SetDefaultRateCorrection()));
	disconnect(spinDeadTime, SIGNAL(editingFinished()), this, SLOT(SetRateCorrection()));

	double rate;
	rate = (double)myDet->getRateCorrection();
	qDefs::checkErrorMessage(myDet, "qTabDataOutput::GetRateCorrection");
	FILE_LOG(logDEBUG) << "Getting rate correction from server: " << rate << '\n';
	if (rate == 0) {
		chkRate->setChecked(false);
		btnDefaultRate->setEnabled(false);
		lblDeadTime->setEnabled(false);
		spinDeadTime->setEnabled(false);
	}

	else {
		chkRate->setChecked(true);
		btnDefaultRate->setEnabled(true);
		lblDeadTime->setEnabled(true);
		spinDeadTime->setEnabled(true);
		spinDeadTime->setValue((double)rate);
	}

	if (rate == -1) {
		qDefs::Message(qDefs::WARNING, "Dead time is inconsistent for all detectors. Returned Value: -1.", "qTabDataOutput::GetRateCorrection");
		FILE_LOG(logWARNING) << "Dead time is inconsistent for all detectors.";
		QString errorTip = QString("<nobr>Rate Corrections.</nobr><br>"
				"<nobr> #ratecorr# tau in seconds</nobr><br><br>") +
						QString("<nobr><font color=\"red\">"
								"Dead time is inconsistent for all detectors.</font></nobr>");
		chkRate->setToolTip(errorTip);
		spinDeadTime->setToolTip(errorTip);
		chkRate->setPalette(red);
		chkRate->setText("Rate:*");
	} else {
		QString normalTip = QString("<nobr>Rate Corrections.</nobr><br>"
				"<nobr> #ratecorr# tau in seconds</nobr><br><br>");
		chkRate->setToolTip(normalTip);
		spinDeadTime->setToolTip(normalTip);
		chkRate->setPalette(chkDiscardBad->palette());
		chkRate->setText("Rate:");
	}

	connect(chkRate, SIGNAL(toggled(bool)), this, SLOT(SetRateCorrection()));
	connect(btnDefaultRate, SIGNAL(clicked()), this, SLOT(SetDefaultRateCorrection()));
	connect(spinDeadTime, SIGNAL(editingFinished()), this, SLOT(SetRateCorrection()));
}










void qTabDataOutput::GetSpeed() {
	int ret;
	if (widgetEiger->isVisible()) {
		disconnect(comboEigerClkDivider, SIGNAL(currentIndexChanged(int)), this, SLOT(SetSpeed()));

		//get speed
		ret = myDet->setSpeed(slsDetectorDefs::CLOCK_DIVIDER, -1);
		qDefs::checkErrorMessage(myDet, "qTabDataOutput::GetSpeed");

		//valid speed
		if (ret >= 0 && ret < NUMBEROFSPEEDS)
			comboEigerClkDivider->setCurrentIndex(ret);

		//invalid speed
		else {
			qDefs::Message(qDefs::WARNING, "Inconsistent value from clock divider.\n"
					"Setting it for all detectors involved to half speed.",
					"qTabDataOutput::GetSpeed");
			FILE_LOG(logWARNING) << "Inconsistent value from clock divider.";
			//set to default
			comboEigerClkDivider->setCurrentIndex(HALFSPEED);
			myDet->setSpeed(slsDetectorDefs::CLOCK_DIVIDER, HALFSPEED);
			qDefs::checkErrorMessage(myDet, "qTabDataOutput::GetSpeed");
		}
		connect(comboEigerClkDivider, SIGNAL(currentIndexChanged(int)), this, SLOT(SetSpeed()));
	}
}


void qTabDataOutput::SetSpeed() {
	FILE_LOG(logINFO) << "Setting Speed";
	myDet->setSpeed(slsDetectorDefs::CLOCK_DIVIDER, comboEigerClkDivider->currentIndex());
	qDefs::checkErrorMessage(myDet, "qTabDataOutput::SetSpeed");
	GetSpeed();
}



void qTabDataOutput::GetFlags() {
	int ret;
	if (widgetEiger->isVisible()) {
		disconnect(comboEigerFlags1, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFlags()));
		disconnect(comboEigerFlags2, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFlags()));

		//get speed
		ret = myDet->setReadOutFlags(slsDetectorDefs::GET_READOUT_FLAGS);
		qDefs::checkErrorMessage(myDet, "qTabDataOutput::GetFlags");

		//invalid flags
		if (ret == -1) {
			qDefs::Message(qDefs::WARNING, "Inconsistent value for readout flags.\n"
					"Setting it for all detectors involved to continous nonparallel mode.",
					"qTabDataOutput::GetFlags");
			FILE_LOG(logWARNING) << "Inconsistent value for readout flags.";
			//set to default
			comboEigerFlags1->setCurrentIndex(CONTINUOUS);
			myDet->setReadOutFlags(slsDetectorDefs::CONTINOUS_RO);
			qDefs::checkErrorMessage(myDet, "qTabDataOutput::GetFlags");
			comboEigerFlags2->setCurrentIndex(NONPARALLEL);
			myDet->setReadOutFlags(slsDetectorDefs::NONPARALLEL);
			qDefs::checkErrorMessage(myDet, "qTabDataOutput::GetFlags");
		}

		//valid flags
		else {
			if (ret & slsDetectorDefs::STORE_IN_RAM)
				comboEigerFlags1->setCurrentIndex(STOREINRAM);
			else if (ret & slsDetectorDefs::CONTINOUS_RO)
				comboEigerFlags1->setCurrentIndex(CONTINUOUS);
			if (ret & slsDetectorDefs::PARALLEL)
				comboEigerFlags2->setCurrentIndex(PARALLEL);
			else if (ret & slsDetectorDefs::NONPARALLEL)
				comboEigerFlags2->setCurrentIndex(NONPARALLEL);
			else if (ret & slsDetectorDefs::SAFE)
				comboEigerFlags2->setCurrentIndex(SAFE);
		}

		connect(comboEigerFlags1, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFlags()));
		connect(comboEigerFlags2, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFlags()));
	}
}


void qTabDataOutput::SetFlags() {
	FILE_LOG(logINFO) << "Setting Readout Flags";
	slsDetectorDefs::readOutFlags val = slsDetectorDefs::GET_READOUT_FLAGS;

	//set to continous or storeinram
	switch (comboEigerFlags1->currentIndex()) {
	case STOREINRAM:
		val = slsDetectorDefs::STORE_IN_RAM;
		break;
	default:
		val = slsDetectorDefs::CONTINOUS_RO;
		break;
	}
	myDet->setReadOutFlags(val);
	qDefs::checkErrorMessage(myDet, "qTabDataOutput::setFlags");

	//set to parallel, nonparallel or safe
	switch (comboEigerFlags2->currentIndex()) {
	case PARALLEL:
		val = slsDetectorDefs::PARALLEL;
		break;
	case SAFE:
		val = slsDetectorDefs::SAFE;
		break;
	default:
		val = slsDetectorDefs::NONPARALLEL;
		break;
	}
	myDet->setReadOutFlags(val);
	qDefs::checkErrorMessage(myDet, "qTabDataOutput::setFlags");

	//update flags
	GetFlags();

}







void qTabDataOutput::Refresh() {
	FILE_LOG(logDEBUG) << "**Updating DataOutput Tab";

	EnableBrowse();
	if (!myDet->enableWriteToFile())
		boxFileWriteEnabled->setEnabled(false);
	else
		boxFileWriteEnabled->setEnabled(true);

	// output dir
	GetOutputDir();

	//overwrite
	GetFileOverwrite();

	//file format
	GetFileFormat();

	//file name
	dispFileName->setText(QString(myDet->getFileName().c_str()));

	// rate correction
	if (chkRate->isEnabled()) {
		GetRateCorrection();
	}

	if (myDet->setReceiverOnline() == slsDetectorDefs::ONLINE_FLAG) {
		btnOutputBrowse->setEnabled(false);
		btnOutputBrowse->setToolTip("<font color=\"red\">This button is disabled as receiver PC is different from "
				"client PC and hence different directory structures.</font><br><br>" +
				dispOutputDir->toolTip());
	} else {
		btnOutputBrowse->setEnabled(true);
		btnOutputBrowse->setToolTip(dispOutputDir->toolTip());
	}

	// 10GbE
	if (chkTenGiga->isEnabled()) {
		GetTenGigaEnable(-1, 1);
	}

	//Eiger specific
	if (widgetEiger->isVisible()) {
		//speed
		GetSpeed();
		//flags
		GetFlags();
	}

	FILE_LOG(logDEBUG) << "**Updated DataOutput Tab";

	qDefs::checkErrorMessage(myDet, "qTabDataOutput::Refresh");
}
