#include "qTabDataOutput.h"
#include "qDefs.h"

#include <QFileDialog>
#include <QStandardItemModel>
#include <QButtonGroup>
#include <QString>

#include <iostream>
#include <string>


qTabDataOutput::qTabDataOutput(QWidget *parent, multiSlsDetector *detector) : QWidget(parent), myDet(detector), btnGroupRate(nullptr) {
	setupUi(this);
	SetupWidgetWindow();
	FILE_LOG(logDEBUG) << "DataOutput ready";
}

qTabDataOutput::~qTabDataOutput() {
	if (btnGroupRate)
		delete btnGroupRate;
}

void qTabDataOutput::SetupWidgetWindow() {
	// button group for rate
	btnGroupRate = new QButtonGroup(this);
	btnGroupRate->addButton(radioDefaultDeadtime, 0);
	btnGroupRate->addButton(radioCustomDeadtime, 1);

	// enabling according to det type
	switch(myDet->getDetectorTypeAsEnum()) {
		case slsDetectorDefs::EIGER:
			chkTenGiga->setEnabled(true);
			chkRate->setEnabled(true);
			radioDefaultDeadtime->setEnabled(true);
			radioCustomDeadtime->setEnabled(true);
			// flags and speed
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

	Initialization();

	Refresh();
}

void qTabDataOutput::Initialization() {
	// ourdir, fileformat, overwrite enable
	connect(comboDetector, SIGNAL(currentIndexChanged(int)), this, SLOT(GetOutputDir()));
	connect(dispOutputDir, SIGNAL(editingFinished()), this, SLOT(SetOutputDir()));
	connect(btnOutputBrowse, SIGNAL(clicked()), this, SLOT(BrowseOutputDir()));
	connect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFileFormat(int)));
	connect(chkOverwriteEnable, SIGNAL(toggled(bool)), this, SLOT(SetOverwriteEnable(bool)));
	if (chkTenGiga->isEnabled()) {
		connect(chkTenGiga, SIGNAL(toggled(bool)), this, SLOT(SetTenGigaEnable(bool)));
	}
	// rate
	if (chkRate->isEnabled()) {
		connect(chkRate, SIGNAL(toggled(bool)), this, SLOT(EnableRateCorrection()));
		connect(btnGroupRate, SIGNAL(buttonClicked(int)), this, SLOT(SetRateCorrection()));
		connect(spinCustomDeadTime, SIGNAL(editingFinished()), this, SLOT(SetRateCorrection()));
	}
	// flags, speed
	if (widgetEiger->isEnabled()) {
		connect(comboEigerClkDivider, SIGNAL(currentIndexChanged(int)), this, SLOT(SetSpeed(int)));
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
	FILE_LOG(logDEBUG) << "Getting browse enable";
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

void qTabDataOutput::GetFileWrite() {
		FILE_LOG(logDEBUG) << "Getting file write enable";
	try {
		int retval = myDet->getFileWrite();
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "File write is inconsistent for all detectors.", "qTabDataOutput::GetFileWrite");
			boxFileWriteEnabled->setEnabled(true);
		} else {
			boxFileWriteEnabled->setEnabled(retval == 0 ? false : true);
		}
	} catch (const sls::RuntimeError &e) {
		qDefs::ExceptionMessage("Could not get file enable.", e.what(), "qTabDataOutput::GetFileWrite");
		boxFileWriteEnabled->setEnabled(true);
    }
}

void qTabDataOutput::GetFileName() {
	FILE_LOG(logDEBUG) << "Getting file name";
	try {
        auto retval = myDet->getFileName();
		dispFileName->setText(QString(retval.c_str()));
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get file name prefix.", e.what(), "qTabDataOutput::GetFileName");
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

	try {
		auto retval = myDet->getFileFormat();
		switch(retval) {
			case slsDetectorDefs::GET_FILE_FORMAT:
				qDefs::Message(qDefs::WARNING, "File Format is inconsistent for all detectors.", "qTabDataOutput::GetFileFormat");
				break;
			case slsDetectorDefs::BINARY:
			case slsDetectorDefs::HDF5:
				comboFileFormat->setCurrentIndex(static_cast<int>(retval));
				break;
			default:
				qDefs::Message(qDefs::WARNING, std::string("Unknown file format: ") + std::to_string(static_cast<int>(retval)), "qTabDataOutput::GetFileFormat");
				break;
        }
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get file format.", e.what(), "qTabDataOutput::GetFileFormat");
    }

	connect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFileFormat(int)));
}

void qTabDataOutput::SetFileFormat(int format) {
	FILE_LOG(logINFO) << "Setting File Format to " << comboFileFormat->currentText().toAscii().data();
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

void qTabDataOutput::GetRateCorrection() {
	FILE_LOG(logDEBUG) << "Getting Rate Correction";	
	disconnect(chkRate, SIGNAL(toggled(bool)), this, SLOT(EnableRateCorrection()));
	disconnect(btnGroupRate, SIGNAL(buttonClicked(int)), this, SLOT(SetRateCorrection()));
	disconnect(spinCustomDeadTime, SIGNAL(editingFinished()), this, SLOT(SetRateCorrection()));

	try {
		int64_t retval = myDet->getRateCorrection();
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Rate correction (enable/tau) is inconsistent for all detectors.", "qTabDataOutput::GetRateCorrection");
			spinCustomDeadTime->setValue(-1);
		} else {
			chkRate->setChecked(retval == 0 ? false : true);
			if (retval != 0)
				spinCustomDeadTime->setValue(retval);
		}
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get rate correction.", e.what(), "qTabDataOutput::GetRateCorrection");
    }

	connect(chkRate, SIGNAL(toggled(bool)), this, SLOT(EnableRateCorrection()));
	connect(btnGroupRate, SIGNAL(buttonClicked(int)), this, SLOT(SetRateCorrection()));
	connect(spinCustomDeadTime, SIGNAL(editingFinished()), this, SLOT(SetRateCorrection()));
}

void qTabDataOutput::EnableRateCorrection() {
	// enable
	if (chkRate->isChecked()) {
		 SetRateCorrection();
		 return;
	} 
	FILE_LOG(logINFO) << "Disabling Rate correction";
	// disable
	try {
		myDet->setRateCorrection(0);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not switch off rate correction.", e.what(), "qTabDataOutput::EnableRateCorrection");
     	GetRateCorrection();
	}
}

void qTabDataOutput::SetRateCorrection() {
	// do nothing if rate correction is disabled
	if (!chkRate->isChecked()) {
		return;
	}
	// get default or custom value
	int64_t deadtime = -1;
	if (radioCustomDeadtime->isChecked()) {
		deadtime = spinCustomDeadTime->value();
	}
	FILE_LOG(logINFO) << "Setting Rate Correction with dead time: " << deadtime;

	try {
		myDet->setRateCorrection(deadtime);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set rate correction.", e.what(), "qTabDataOutput::SetRateCorrection");
		GetRateCorrection();
    }
}

void qTabDataOutput::GetSpeed() {
	FILE_LOG(logDEBUG) << "Getting Speed";	
	disconnect(comboEigerClkDivider, SIGNAL(currentIndexChanged(int)), this, SLOT(SetSpeed()));
	
	try {
		int retval = myDet->setSpeed(slsDetectorDefs::CLOCK_DIVIDER);
		switch(retval) {
			case -1:
				qDefs::Message(qDefs::WARNING, "Speed is inconsistent for all detectors.", "qTabDataOutput::GetSpeed");
				break;
			case FULLSPEED:
			case HALFSPEED:
			case QUARTERSPEED:
				comboEigerClkDivider->setCurrentIndex(retval);
				break;
			default:
				qDefs::Message(qDefs::WARNING, std::string("Unknown speed: ") + std::to_string(retval), "qTabDataOutput::GetFileFormat");
				break;
        }
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get speed.", e.what(), "qTabDataOutput::GetSpeed");
    }

	connect(comboEigerClkDivider, SIGNAL(currentIndexChanged(int)), this, SLOT(SetSpeed()));
}

void qTabDataOutput::SetSpeed(int speed) {
	FILE_LOG(logINFO) << "Setting Speed to " << comboEigerClkDivider->currentText().toAscii().data();;
	try {
        myDet->setSpeed(slsDetectorDefs::CLOCK_DIVIDER, speed);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set speed.", e.what(), "qTabDataOutput::SetSpeed");
        GetSpeed();
    }
}

void qTabDataOutput::GetFlags() {
	FILE_LOG(logDEBUG) << "Getting readout flags";	
	disconnect(comboEigerFlags1, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFlags()));
	disconnect(comboEigerFlags2, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFlags()));

	try {
		int retval = myDet->setReadOutFlags(slsDetectorDefs::GET_READOUT_FLAGS);
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Readout flags are inconsistent for all detectors.", "qTabDataOutput::GetFlags");
		} else {
			// store in ram or continuous
			if (retval & slsDetectorDefs::STORE_IN_RAM)
				comboEigerFlags1->setCurrentIndex(STOREINRAM);
			else if (retval & slsDetectorDefs::CONTINOUS_RO)
				comboEigerFlags1->setCurrentIndex(CONTINUOUS);
			else {
				qDefs::Message(qDefs::WARNING, std::string("Unknown flag (Not Store in ram or Continous): ") + std::to_string(retval), "qTabDataOutput::GetFlags");
			}

			// parallel or non parallel
			if (retval & slsDetectorDefs::PARALLEL)
				comboEigerFlags2->setCurrentIndex(PARALLEL);
			else if (retval & slsDetectorDefs::NONPARALLEL)
				comboEigerFlags2->setCurrentIndex(NONPARALLEL);
			else {
				qDefs::Message(qDefs::WARNING, std::string("Unknown flag (Not Parallel or Non Parallel): ") + std::to_string(retval), "qTabDataOutput::GetFlags");
			}
		}
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get speed.", e.what(), "qTabDataOutput::GetSpeed");
    }

	connect(comboEigerFlags1, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFlags()));
	connect(comboEigerFlags2, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFlags()));
}


void qTabDataOutput::SetFlags() {
	auto flag1 = slsDetectorDefs::GET_READOUT_FLAGS;
	auto flag2 = slsDetectorDefs::GET_READOUT_FLAGS;
	
	//set to continous or storeinram
	switch (comboEigerFlags1->currentIndex()) {
	case STOREINRAM:
		flag1 = slsDetectorDefs::STORE_IN_RAM;
		break;
	default:
		flag1 = slsDetectorDefs::CONTINOUS_RO;
		break;
	}

	//set to parallel or nonparallel
	switch (comboEigerFlags2->currentIndex()) {
	case PARALLEL:
		flag2 = slsDetectorDefs::PARALLEL;
		break;
	default:
		flag2 = slsDetectorDefs::NONPARALLEL;
		break;
	}

	try {
		FILE_LOG(logINFO) << "Setting Readout Flags to " << comboEigerFlags1->currentText().toAscii().data();
        myDet->setReadOutFlags(flag1);
		FILE_LOG(logINFO) << "Setting Readout Flags to " << comboEigerFlags2->currentText().toAscii().data();
		myDet->setReadOutFlags(flag2);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set readout flags.", e.what(), "qTabDataOutput::SetFlags");
        GetFlags();
    }
}







void qTabDataOutput::Refresh() {
	FILE_LOG(logDEBUG) << "**Updating DataOutput Tab";

	EnableBrowse();
	GetFileWrite();
	GetFileName();
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

	FILE_LOG(logDEBUG) << "**Updated DataOutput Tab";
}
