#include "qTabDataOutput.h"

#include "multiSlsDetector.h"

#include <QFileDialog>
#include <QStandardItemModel>

#include <iostream>
#include <string>


qTabDataOutput::qTabDataOutput(QWidget *parent, multiSlsDetector *&detector) : QWidget(parent), myDet(detector) {
	setupUi(this);
	SetupWidgetWindow();
	Refresh();
	FILE_LOG(logDEBUG) << "DataOutput ready";
}


qTabDataOutput::~qTabDataOutput() {
	delete myDet;
}


void qTabDataOutput::SetupWidgetWindow() {
	red = QPalette();
	red.setColor(QPalette::Active, QPalette::WindowText, Qt::red);
	black = QPalette();
	black.setColor(QPalette::Active, QPalette::WindowText, Qt::black);

	red1 = new QPalette();
	red1->setColor(QPalette::Text, Qt::red);
	black1 = new QPalette();
	black1->setColor(QPalette::Text, Qt::black);

	outDirTip = lblOutputDir->toolTip();

	// Detector Type
	detType = myDet->getDetectorTypeAsEnum();

	// disabling depening on detector type
	chkRate->setEnabled(false);
	widgetEiger->setVisible(false);
	if (detType == slsDetectorDefs::EIGER) {
		chkRate->setEnabled(true);
		widgetEiger->setVisible(true);
	}
	chkTenGiga->setEnabled(false);
	if (detType == slsDetectorDefs::EIGER || detType == slsDetectorDefs::MOENCH) {
		chkTenGiga->setEnabled(true);
	}

	Initialization();

	// populate detectors, get output dir
	PopulateDetectors();

	VerifyOutputDirectory();

	// over write
	UpdateFileOverwriteFromServer();

	// file format
	UpdateFileFormatFromServer();

	// rate correction
	if (chkRate->isEnabled()) {
		UpdateRateCorrectionFromServer();
	}

	// 10 gbe
	if (chkTenGiga->isEnabled()) {
		EnableTenGigabitEthernet(-1, 1);
	}

	//Eiger specific
	if (widgetEiger->isVisible()) {
		//speed
		UpdateSpeedFromServer();
		//flags
		UpdateFlagsFromServer();
	}

	qDefs::checkErrorMessage(myDet, "qTabDataOutput::SetupWidgetWindow");
}


void qTabDataOutput::Initialization() {
	//output dir
	connect(comboDetector, SIGNAL(currentIndexChanged(int)), this, SLOT(GetOutputDir()));
	connect(dispOutputDir, SIGNAL(editingFinished()), this, SLOT(SetOutputDir()));
	connect(btnOutputBrowse, SIGNAL(clicked()), this, SLOT(BrowseOutputDir()));

	//overwrite enable
	connect(chkOverwriteEnable, SIGNAL(toggled(bool)), this, SLOT(SetOverwriteEnable(bool)));

	//file format
	connect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFileFormat(int)));

	//rate correction
	connect(chkRate, SIGNAL(toggled(bool)), this, SLOT(SetRateCorrection()));
	connect(btnDefaultRate, SIGNAL(clicked()), this, SLOT(SetDefaultRateCorrection()));
	connect(spinDeadTime, SIGNAL(editingFinished()), this, SLOT(SetRateCorrection()));

	//10GbE
	connect(chkTenGiga, SIGNAL(toggled(bool)), this, SLOT(EnableTenGigabitEthernet(bool)));

	//eiger
	if (widgetEiger->isVisible()) {
		//speed
		connect(comboEigerClkDivider, SIGNAL(currentIndexChanged(int)), this, SLOT(SetSpeed()));
		//flags
		connect(comboEigerFlags1, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFlags()));
		connect(comboEigerFlags2, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFlags()));
	}
}


void qTabDataOutput::BrowseOutputDir() {
	QString directory = QFileDialog::getExistingDirectory(this, tr("Choose Output Directory "), dispOutputDir->text());
	if (!directory.isEmpty())
		dispOutputDir->setText(directory);
	SetOutputDir();
}


void qTabDataOutput::GetOutputDir() {
	FILE_LOG(logDEBUG) << "Getting output directory";

	disconnect(dispOutputDir, SIGNAL(editingFinished()), this, SLOT(SetOutputDir()));
	//all
	if (!comboDetector->currentIndex()) {
		dispOutputDir->setText(QString(myDet->getFilePath().c_str()));
		qDefs::checkErrorMessage(myDet, "qTabDataOutput::GetOutputDir");

		//multi file path blank means sls file paths are different
		if (dispOutputDir->text().isEmpty()) {
			qDefs::Message(qDefs::INFORMATION, "The file path for individual readouts are different.\n"
					"Hence, leaving the common field blank.", "qTabDataOutput::GetOutputDir");
			FILE_LOG(logWARNING) << "The file path for individual units are different.";
			QString errTip = QString("<br><nobr><font color=\"red\">"
					"<b>Output Directory</b> Information only: The file path for individual readouts are different.<br>"
					"Hence, leaving the common field blank.</font></nobr>");
			lblOutputDir->setText("Path*:");
			lblOutputDir->setPalette(red);
			lblOutputDir->setToolTip(errTip);
			btnOutputBrowse->setToolTip(errTip);
			dispOutputDir->setToolTip(errTip);
		} else {
			lblOutputDir->setText("Path:");
			lblOutputDir->setPalette(*black1);
			lblOutputDir->setToolTip(outDirTip);
			btnOutputBrowse->setToolTip(outDirTip);
			dispOutputDir->setToolTip(outDirTip);
		}
	}

	//specific
	else {
		dispOutputDir->setText(QString(myDet->getFilePath(comboDetector->currentIndex() - 1).c_str()));
		qDefs::checkErrorMessage(myDet, comboDetector->currentIndex() - 1, "qTabDataOutput::GetOutputDir");
	}

	connect(dispOutputDir, SIGNAL(editingFinished()), this, SLOT(SetOutputDir()));
}


int qTabDataOutput::VerifyOutputDirectory() {
	FILE_LOG(logDEBUG) << "Verifying output directory";

	GetOutputDir();

	bool error = false;
	std::string detName = "";
	std::string mess = "";

	//common  (check only if no +, different values)
	std::string fpath = myDet->getFilePath();
	if (fpath.find('+') == std::string::npos) {
		myDet->setFilePath(myDet->getFilePath());
		if (!qDefs::checkErrorMessage(myDet, "qTabDataOutput::VerifyOutputDirectory").empty())
			error = true;
	}

	//for each detector
	for (int i = 0; i < myDet->getNumberOfDetectors(); ++i) {
		 detName = std::string("\n - ") + std::string(comboDetector->itemText(i+1).toAscii().constData());
		 myDet->setFilePath(myDet->getFilePath(i), i);
		 if(!qDefs::checkErrorMessage(myDet, i, "qTabDataOutput::VerifyOutputDirectory").empty()) {
		 	mess. append(detName);
		 	error = true;
		 }
	}

	//invalid
	if (error) {
		qDefs::Message(qDefs::WARNING, std::string("Invalid Output Directory ") + mess, "qTabDataOutput::VerifyOutputDirectory");
		FILE_LOG(logWARNING) << "The output path doesnt exist anymore";
		//replace all \n with <br>
		size_t pos = 0;
		while ((pos = mess.find("\n", pos)) != std::string::npos) {
			mess.replace(pos, 1, "<br>");
			pos += 1;
		}
		QString errTip = outDirTip +
				QString("<br><nobr><font color=\"red\">"
						"Invalid <b>Output Directory</b>") +
						QString(mess.c_str()) +
						QString(".</font></nobr>");
		lblOutputDir->setText("Path*:");
		lblOutputDir->setPalette(red);
		lblOutputDir->setToolTip(errTip);
		btnOutputBrowse->setToolTip(errTip);
		dispOutputDir->setToolTip(errTip);

		return slsDetectorDefs::FAIL;
	}

	//valid
	else {
		FILE_LOG(logDEBUG) << "The output path is valid";
		lblOutputDir->setText("Path:");
		lblOutputDir->setPalette(*black1);
		lblOutputDir->setToolTip(outDirTip);
		btnOutputBrowse->setToolTip(outDirTip);
		dispOutputDir->setToolTip(outDirTip);
	}

	return slsDetectorDefs::OK;
}


void qTabDataOutput::SetOutputDir() {

	FILE_LOG(logDEBUG) << "Setting output directory";

	disconnect(dispOutputDir, SIGNAL(editingFinished()), this, SLOT(SetOutputDir()));

	bool error = false;
	QString path = dispOutputDir->text();

	//empty
	if (path.isEmpty()) {
		qDefs::Message(qDefs::WARNING, "Invalid Output Path. Must not be empty.", "qTabDataOutput::SetOutputDir");
		FILE_LOG(logWARNING) << "Invalid Output Path. Must not be empty.";
		error = true;
	}
	//gets rid of the end '/'s
	else if (path.endsWith('/')) {
		while (path.endsWith('/'))
			path.chop(1);
		dispOutputDir->setText(path);
	}

	//specific
	if (comboDetector->currentIndex()) {
		myDet->setFilePath(std::string(dispOutputDir->text().toAscii().constData()), comboDetector->currentIndex() - 1);
		if (!qDefs::checkErrorMessage(myDet, comboDetector->currentIndex() - 1, "qTabDataOutput::SetOutputDir").empty())
			error = true;
	}

	//multi
	else {
		myDet->setFilePath(std::string(path.toAscii().constData()));
		if (!qDefs::checkErrorMessage(myDet, "qTabDataOutput::SetOutputDir").empty())
			error = true;
	}

	if (error) {
		FILE_LOG(logWARNING) << "The output path could not be set";
		QString errTip = outDirTip + QString("<br><nobr><font color=\"red\">"
				"Invalid <b>File Path</b></font></nobr>");

		lblOutputDir->setText("Path*:");
		lblOutputDir->setPalette(red);
		lblOutputDir->setToolTip(errTip);
		btnOutputBrowse->setToolTip(errTip);
		dispOutputDir->setToolTip(errTip);
	} else {
		FILE_LOG(logINFO) << "Output dir set to " << path.toAscii().constData();
		lblOutputDir->setText("Path:");
		lblOutputDir->setPalette(*black1);
		lblOutputDir->setToolTip(outDirTip);
		btnOutputBrowse->setToolTip(outDirTip);
		dispOutputDir->setToolTip(outDirTip);
	}

	connect(dispOutputDir, SIGNAL(editingFinished()), this, SLOT(SetOutputDir()));
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
		qDefs::Message(qDefs::WARNING, "Dead time is inconsistent for all detectors. Returned Value: -1.", "qTabDataOutput::UpdateRateCorrectionFromServer");
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

	connect(btnDefaultRate, SIGNAL(clicked()), this, SLOT(SetDefaultRateCorrection()));
	connect(spinDeadTime, SIGNAL(editingFinished()), this, SLOT(SetRateCorrection()));
}


void qTabDataOutput::SetDefaultRateCorrection() {
	SetRateCorrection(-1);
}


void qTabDataOutput::UpdateRateCorrectionFromServer() {
	disconnect(chkRate, SIGNAL(toggled(bool)), this, SLOT(SetRateCorrection()));
	disconnect(btnDefaultRate, SIGNAL(clicked()), this, SLOT(SetDefaultRateCorrection()));
	disconnect(spinDeadTime, SIGNAL(editingFinished()), this, SLOT(SetRateCorrection()));

	double rate;
	rate = (double)myDet->getRateCorrection();
	qDefs::checkErrorMessage(myDet, "qTabDataOutput::UpdateRateCorrectionFromServer");
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
		qDefs::Message(qDefs::WARNING, "Dead time is inconsistent for all detectors. Returned Value: -1.", "qTabDataOutput::UpdateRateCorrectionFromServer");
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


void qTabDataOutput::PopulateDetectors() {
	disconnect(comboDetector, SIGNAL(currentIndexChanged(int)), this, SLOT(GetOutputDir()));

	FILE_LOG(logDEBUG) << "Populating detectors";
	comboDetector->clear();
	comboDetector->addItem("All");
	lblOutputDir->setText("Path:");
	//add specific detector options only if more than 1 detector
	if (myDet->getNumberOfDetectors() > 1) {
		for (int i = 0; i < myDet->getNumberOfDetectors(); i++)
			comboDetector->addItem(QString(myDet->getHostname(i).c_str()));
	}
	GetOutputDir();
	connect(comboDetector, SIGNAL(currentIndexChanged(int)), this, SLOT(GetOutputDir()));
}




void qTabDataOutput::EnableTenGigabitEthernet(bool enable, int get) {
	if (get || enable == -1) {
		FILE_LOG(logDEBUG) << "Getting 10Gbe enable";
	} else {
		FILE_LOG(logINFO) << (enable == 0 ? "Disabling" : "Enabling") << "10GbE";
	}
	disconnect(chkTenGiga, SIGNAL(toggled(bool)), this, SLOT(EnableTenGigabitEthernet(bool)));
	int ret;
	if (get)
		ret = myDet->enableTenGigabitEthernet(-1);
	else
		ret = myDet->enableTenGigabitEthernet(enable);
	if (ret > 0)
		chkTenGiga->setChecked(true);
	else
		chkTenGiga->setChecked(false);
	connect(chkTenGiga, SIGNAL(toggled(bool)), this, SLOT(EnableTenGigabitEthernet(bool)));

	qDefs::checkErrorMessage(myDet, "qTabDataOutput::EnableTenGigabitEthernet");
}


void qTabDataOutput::UpdateSpeedFromServer() {
	int ret;
	if (widgetEiger->isVisible()) {
		disconnect(comboEigerClkDivider, SIGNAL(currentIndexChanged(int)), this, SLOT(SetSpeed()));

		//get speed
		ret = myDet->setSpeed(slsDetectorDefs::CLOCK_DIVIDER, -1);
		qDefs::checkErrorMessage(myDet, "qTabDataOutput::updateSpeedFromServer");

		//valid speed
		if (ret >= 0 && ret < NUMBEROFSPEEDS)
			comboEigerClkDivider->setCurrentIndex(ret);

		//invalid speed
		else {
			qDefs::Message(qDefs::WARNING, "Inconsistent value from clock divider.\n"
					"Setting it for all detectors involved to half speed.",
					"qTabDataOutput::updateSpeedFromServer");
			FILE_LOG(logWARNING) << "Inconsistent value from clock divider.";
			//set to default
			comboEigerClkDivider->setCurrentIndex(HALFSPEED);
			myDet->setSpeed(slsDetectorDefs::CLOCK_DIVIDER, HALFSPEED);
			qDefs::checkErrorMessage(myDet, "qTabDataOutput::updateSpeedFromServer");
		}
		connect(comboEigerClkDivider, SIGNAL(currentIndexChanged(int)), this, SLOT(SetSpeed()));
	}
}


void qTabDataOutput::SetSpeed() {
	FILE_LOG(logINFO) << "Setting Speed";
	myDet->setSpeed(slsDetectorDefs::CLOCK_DIVIDER, comboEigerClkDivider->currentIndex());
	qDefs::checkErrorMessage(myDet, "qTabDataOutput::SetSpeed");
	UpdateSpeedFromServer();
}



void qTabDataOutput::UpdateFlagsFromServer() {
	int ret;
	if (widgetEiger->isVisible()) {
		disconnect(comboEigerFlags1, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFlags()));
		disconnect(comboEigerFlags2, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFlags()));

		//get speed
		ret = myDet->setReadOutFlags(slsDetectorDefs::GET_READOUT_FLAGS);
		qDefs::checkErrorMessage(myDet, "qTabDataOutput::updateFlagsFromServer");

		//invalid flags
		if (ret == -1) {
			qDefs::Message(qDefs::WARNING, "Inconsistent value for readout flags.\n"
					"Setting it for all detectors involved to continous nonparallel mode.",
					"qTabDataOutput::updateFlagsFromServer");
			FILE_LOG(logWARNING) << "Inconsistent value for readout flags.";
			//set to default
			comboEigerFlags1->setCurrentIndex(CONTINUOUS);
			myDet->setReadOutFlags(slsDetectorDefs::CONTINOUS_RO);
			qDefs::checkErrorMessage(myDet, "qTabDataOutput::updateFlagsFromServer");
			comboEigerFlags2->setCurrentIndex(NONPARALLEL);
			myDet->setReadOutFlags(slsDetectorDefs::NONPARALLEL);
			qDefs::checkErrorMessage(myDet, "qTabDataOutput::updateFlagsFromServer");
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
	UpdateFlagsFromServer();

}


void qTabDataOutput::UpdateFileFormatFromServer() {
	FILE_LOG(logDEBUG) << "Getting File Format";
	disconnect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFileFormat(int)));

	comboFileFormat->setCurrentIndex((int)myDet->getFileFormat());

	connect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFileFormat(int)));
}


void qTabDataOutput::SetFileFormat(int format) {
	FILE_LOG(logINFO) << "Setting File Format";
	disconnect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFileFormat(int)));

	int ret = (int)myDet->setFileFormat((slsDetectorDefs::fileFormat)comboFileFormat->currentIndex());
	if (ret != comboFileFormat->currentIndex()) {
		qDefs::Message(qDefs::WARNING, "Could not set file format.", "qTabDataOutput::SetFileFormat");
		comboFileFormat->setCurrentIndex((int)ret);
	}

	connect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(SetFileFormat(int)));
}


void qTabDataOutput::UpdateFileOverwriteFromServer() {
	FILE_LOG(logDEBUG) << "Getting File Over Write Enable";
	disconnect(chkOverwriteEnable, SIGNAL(toggled(bool)), this, SLOT(SetOverwriteEnable(bool)));

	chkOverwriteEnable->setChecked(myDet->overwriteFile());

	connect(chkOverwriteEnable, SIGNAL(toggled(bool)), this, SLOT(SetOverwriteEnable(bool)));
}


void qTabDataOutput::SetOverwriteEnable(bool enable) {
	FILE_LOG(logINFO) << "Setting File Over Write Enable";
	disconnect(chkOverwriteEnable, SIGNAL(toggled(bool)), this, SLOT(SetOverwriteEnable(bool)));

	int valid = (enable ? 1 : 0);
	if (myDet->overwriteFile(enable) != valid)
		qDefs::Message(qDefs::WARNING, "Could not over write enable.", "qTabDataOutput::SetOverwriteEnable");

	connect(chkOverwriteEnable, SIGNAL(toggled(bool)), this, SLOT(SetOverwriteEnable(bool)));

	UpdateFileOverwriteFromServer();
}


void qTabDataOutput::Refresh() {
	FILE_LOG(logDEBUG) << "**Updating DataOutput Tab";

	if (!myDet->enableWriteToFile())
		boxFileWriteEnabled->setEnabled(false);
	else
		boxFileWriteEnabled->setEnabled(true);

	// output dir
	PopulateDetectors();

	//overwrite
	UpdateFileOverwriteFromServer();

	//file format
	UpdateFileFormatFromServer();

	//file name
	dispFileName->setText(QString(myDet->getFileName().c_str()));

	// rate correction
	if (chkRate->isEnabled()) {
		UpdateRateCorrectionFromServer();
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
		EnableTenGigabitEthernet(-1, 1);
	}

	//Eiger specific
	if (widgetEiger->isVisible()) {
		//speed
		UpdateSpeedFromServer();
		//flags
		UpdateFlagsFromServer();
	}

	FILE_LOG(logDEBUG) << "**Updated DataOutput Tab";

	qDefs::checkErrorMessage(myDet, "qTabDataOutput::Refresh");
}
