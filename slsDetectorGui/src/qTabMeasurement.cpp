#include "qTabMeasurement.h"
#include "qDefs.h"
#include "qDrawPlot.h"

#include "string_utils.h"

#include <QStandardItemModel>
#include <QTimer>

#include <cmath>
#include <iostream>

qTabMeasurement::qTabMeasurement(QWidget *parent, sls::Detector *detector, qDrawPlot *p) : QWidget(parent), det(detector), plot(p),
	progressTimer(nullptr) {
	setupUi(this);
	SetupWidgetWindow();
	FILE_LOG(logDEBUG) << "Measurement ready";
}

qTabMeasurement::~qTabMeasurement() {
	if (progressTimer)
		delete progressTimer;
}

void qTabMeasurement::SetupWidgetWindow() {
	// palette
	red = QPalette();
	red.setColor(QPalette::Active, QPalette::WindowText, Qt::red);
	acqPeriodTip = spinPeriod->toolTip();
	errPeriodTip = QString("<nobr>Frame period between exposures.</nobr><br>"
			"<nobr> #period#</nobr><br><br>") +
					QString("<nobr><font color=\"red\"><b>Acquisition Period</b> should be"
							" greater than or equal to <b>Exposure Time</b>.</font></nobr>");

	// timer to update the progress bar
	progressTimer = new QTimer(this);

	sampleImplemented = false;
	delayImplemented = true;
	startingFnumImplemented = false;
	// by default, delay and starting fnum is disabled in form
	lblDelay->setEnabled(true);
	spinDelay->setEnabled(true);
	comboDelayUnit->setEnabled(true);
	// enabling according to det type
	switch(det->getDetectorType().squash()) {
		case slsDetectorDefs::MOENCH:
			lblNumSamples->setEnabled(true);
			spinNumSamples->setEnabled(true);
			sampleImplemented = true;
			break;
		case slsDetectorDefs::EIGER:
			delayImplemented = false;
			lblStartingFrameNumber->setEnabled(true);
			spinStartingFrameNumber->setEnabled(true);
			startingFnumImplemented = true;
			break;
		case slsDetectorDefs::JUNGFRAU:
			lblStartingFrameNumber->setEnabled(true);
			spinStartingFrameNumber->setEnabled(true);
			startingFnumImplemented = true;
			break;
		default:
			break;
	}

	SetupTimingMode();

	Initialization();

	Refresh();
	// normally called only if different
	EnableWidgetsforTimingMode();
}

void qTabMeasurement::Initialization() {
	connect(comboTimingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(SetTimingMode(int)));
	connect(spinNumMeasurements, SIGNAL(valueChanged(int)), this, SLOT(SetNumMeasurements(int)));
	connect(spinNumFrames, SIGNAL(valueChanged(int)), this, SLOT(SetNumFrames(int)));
	connect(spinNumTriggers, SIGNAL(valueChanged(int)), this, SLOT(SetNumTriggers(int)));
	if (spinNumSamples->isEnabled()) {
		connect(spinNumSamples, SIGNAL(valueChanged(int)), this, SLOT(SetNumSamples(int)));
	}
	connect(spinExpTime, SIGNAL(valueChanged(double)), this, SLOT(SetExposureTime()));
	connect(comboExpUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetExposureTime()));
	connect(spinPeriod, SIGNAL(valueChanged(double)), this, SLOT(SetAcquisitionPeriod()));
	connect(comboPeriodUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetAcquisitionPeriod()));
	if (spinDelay->isEnabled()) {
		connect(spinDelay, SIGNAL(valueChanged(double)), this, SLOT(SetDelay()));
		connect(comboDelayUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetDelay()));
	}
	connect(chkFile, SIGNAL(toggled(bool)), this, SLOT(SetFileWrite(bool)));
	connect(dispFileName, SIGNAL(editingFinished()), this, SLOT(SetFileName()));
	connect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(SetRunIndex(int)));
	if (startingFnumImplemented) {
		connect(spinStartingFrameNumber, SIGNAL(valueChanged(int)), this, SLOT(SetStartingFrameNumber(int)));
	}
	connect(progressTimer, SIGNAL(timeout()), this, SLOT(UpdateProgress()));
	connect(btnStart, SIGNAL(clicked()), this, SLOT(StartAcquisition()));
	connect(btnStop, SIGNAL(clicked()), this, SLOT(StopAcquisition()));

}

void qTabMeasurement::SetupTimingMode() {
	QStandardItemModel* model = qobject_cast<QStandardItemModel *>(comboTimingMode->model());
	QModelIndex index[NUMTIMINGMODES];
	QStandardItem *item[NUMTIMINGMODES];
	if (model) {
		for (int i = 0; i < NUMTIMINGMODES; i++) {
			index[i] = model->index(i, comboTimingMode->modelColumn(), comboTimingMode->rootModelIndex());
			item[i] = model->itemFromIndex(index[i]);
		}

		if (det->getDetectorType().squash() != slsDetectorDefs::EIGER) {
			item[(int)GATED]->setEnabled(false);
			item[(int)BURST_TRIGGER]->setEnabled(false);
		}
	}
}

void qTabMeasurement::EnableWidgetsforTimingMode() {
	FILE_LOG(logDEBUG) << "Enabling Widgets for Timing Mode";

	// default
	lblNumFrames->setEnabled(false);
	spinNumFrames->setEnabled(false);
	lblNumTriggers->setEnabled(false);
	spinNumTriggers->setEnabled(false);
	lblExpTime->setEnabled(false);
	spinExpTime->setEnabled(false);
	comboExpUnit->setEnabled(false);
	lblPeriod->setEnabled(false);
	spinPeriod->setEnabled(false);
	comboPeriodUnit->setEnabled(false);
	lblDelay->setEnabled(false);
	spinDelay->setEnabled(false);
	comboDelayUnit->setEnabled(false);

	switch(comboTimingMode->currentIndex()) {
		case AUTO:
			// #frames, exptime, period
			spinNumTriggers->setValue(1);
			lblNumFrames->setEnabled(true);
			spinNumFrames->setEnabled(true);
			lblExpTime->setEnabled(true);
			spinExpTime->setEnabled(true);
			comboExpUnit->setEnabled(true);
			lblPeriod->setEnabled(true);
			spinPeriod->setEnabled(true);
			comboPeriodUnit->setEnabled(true);
			break;
		case TRIGGER: 
			// #triggers, exptime
			lblNumTriggers->setEnabled(true);
			spinNumTriggers->setEnabled(true);
			lblExpTime->setEnabled(true);
			spinExpTime->setEnabled(true);
			comboExpUnit->setEnabled(true);
			if (det->getDetectorType().squash() == slsDetectorDefs::EIGER) {
				spinNumFrames->setValue(1);
			} else {
				// #frames, period, delay
				lblNumFrames->setEnabled(true);
				spinNumFrames->setEnabled(true);
				lblPeriod->setEnabled(true);
				spinPeriod->setEnabled(true);
				comboPeriodUnit->setEnabled(true);
				lblDelay->setEnabled(true);
				spinDelay->setEnabled(true);
				comboDelayUnit->setEnabled(true);
			}
			break;
		case GATED:
			// #frames
			spinNumTriggers->setValue(1);
			lblNumFrames->setEnabled(true);
			spinNumFrames->setEnabled(true);
			break;
		case BURST_TRIGGER:
			// #frames, exptime, period
			spinNumTriggers->setValue(1);
			lblNumFrames->setEnabled(true);
			spinNumFrames->setEnabled(true);
			lblExpTime->setEnabled(true);
			spinExpTime->setEnabled(true);
			comboExpUnit->setEnabled(true);
			lblPeriod->setEnabled(true);
			spinPeriod->setEnabled(true);
			comboPeriodUnit->setEnabled(true);
			break;
		default:
			break;
	}

	CheckAcqPeriodGreaterThanExp();
}

void qTabMeasurement::GetTimingMode() {
	FILE_LOG(logDEBUG) << "Getting timing mode";
	disconnect(comboTimingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(SetTimingMode(int)));
	try {
		auto oldMode = comboTimingMode->currentIndex();
        auto retval = det->getTimingMode().tsquash("Inconsistent timing mode for all detectors.");
		switch(retval) {
			case slsDetectorDefs::AUTO_TIMING:
			case slsDetectorDefs::TRIGGER_EXPOSURE:
			case slsDetectorDefs::GATED:
			case slsDetectorDefs::BURST_TRIGGER:
				comboTimingMode->setCurrentIndex((int)retval);
				// update widget enable only if different 
				if (oldMode != comboTimingMode->currentIndex()) {
					EnableWidgetsforTimingMode();
				}
				break;
			default:
				throw sls::RuntimeError(std::string("Unknown timing mode: ")+ std::to_string(retval));
		}
    } CATCH_DISPLAY("Could not get timing mode.", "qTabMeasurement::GetTimingMode")
	connect(comboTimingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(SetTimingMode(int)));
}

void qTabMeasurement::SetTimingMode(int val) {
	FILE_LOG(logINFO) << "Setting timing mode:" << comboTimingMode->currentText().toAscii().data();
	try {
        det->setTimingMode(static_cast<slsDetectorDefs::timingMode>(val));
		EnableWidgetsforTimingMode();
    } CATCH_HANDLE("Could not set timing mode.", "qTabMeasurement::SetTimingMode", this, &qTabMeasurement::GetTimingMode)
}

void qTabMeasurement::SetNumMeasurements(int val) {
	FILE_LOG(logINFO) << "Setting Number of Measurements to " << val;
	numMeasurements = val;
}

void qTabMeasurement::GetNumFrames() {
	FILE_LOG(logDEBUG) << "Getting number of frames";
	disconnect(spinNumFrames, SIGNAL(valueChanged(int)), this, SLOT(SetNumFrames(int)));
	try {
        auto retval = det->getNumberOfFrames().tsquash("Inconsistent number of frames for all detectors.");
		spinNumFrames->setValue(retval);
    } CATCH_DISPLAY ("Could not get number of frames.", "qTabMeasurement::GetNumFrames")
	connect(spinNumFrames, SIGNAL(valueChanged(int)), this, SLOT(SetNumFrames(int)));
}

void qTabMeasurement::SetNumFrames(int val) {
	FILE_LOG(logINFO) << "Setting number of frames to " << val;
	try {
        det->setNumberOfFrames(val);
    } CATCH_HANDLE("Could not set number of frames.", "qTabMeasurement::SetNumFrames", this, &qTabMeasurement::GetNumFrames)
}

void qTabMeasurement::GetNumTriggers() {
	FILE_LOG(logDEBUG) << "Getting number of triggers";
	disconnect(spinNumTriggers, SIGNAL(valueChanged(int)), this, SLOT(SetNumTriggers(int)));
	try {
        auto retval = det->getNumberOfTriggers().tsquash("Inconsistent number of triggers for all detectors.");
		spinNumTriggers->setValue(retval);
    } CATCH_DISPLAY ("Could not get number of frames.", "qTabMeasurement::GetNumTriggers")
	connect(spinNumTriggers, SIGNAL(valueChanged(int)), this, SLOT(SetNumTriggers(int)));
}

void qTabMeasurement::SetNumTriggers(int val) {
	FILE_LOG(logINFO) << "Setting number of triggers to " << val;
	try {
        det->setNumberOfTriggers(val);
    } CATCH_HANDLE("Could not set number of triggers.", "qTabMeasurement::SetNumTriggers", this, &qTabMeasurement::GetNumTriggers)
}

void qTabMeasurement::GetNumSamples() {
	FILE_LOG(logDEBUG) << "Getting number of samples";
	disconnect(spinNumSamples, SIGNAL(valueChanged(int)), this, SLOT(SetNumSamples(int)));
	try {
        auto retval = det->getNumberOfAnalogSamples().tsquash("Inconsistent number of analog samples for all detectors.");
		spinNumSamples->setValue(retval);
    } CATCH_DISPLAY ("Could not get number of samples.", "qTabMeasurement::GetNumSamples")
	connect(spinNumSamples, SIGNAL(valueChanged(int)), this, SLOT(SetNumSamples(int)));
}

void qTabMeasurement::SetNumSamples(int val) {
	FILE_LOG(logINFO) << "Setting number of samples to " << val;
	try {
        det->setNumberOfAnalogSamples(val);
    } CATCH_HANDLE("Could not set number of samples.", "qTabMeasurement::SetNumSamples", this, &qTabMeasurement::GetNumSamples)
}

void qTabMeasurement::GetExposureTime() {
	FILE_LOG(logDEBUG) << "Getting exposure time";
	disconnect(spinExpTime, SIGNAL(valueChanged(double)), this, SLOT(SetExposureTime()));
	disconnect(comboExpUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetExposureTime()));
	try {
		spinExpTime->setValue(-1);
        auto retval = det->getExptime().tsquash("Inconsistent exposure time for all detectors.");
		auto time = qDefs::getUserFriendlyTime(retval);
		spinExpTime->setValue(time.first);
		comboExpUnit->setCurrentIndex(static_cast<int>(time.second));
		CheckAcqPeriodGreaterThanExp();
    } CATCH_DISPLAY ("Could not get exposure time.", "qTabMeasurement::GetExposureTime")
	connect(spinExpTime, SIGNAL(valueChanged(double)), this, SLOT(SetExposureTime()));
	connect(comboExpUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetExposureTime()));
}

void qTabMeasurement::SetExposureTime() {
	auto val = spinExpTime->value();
	auto unit = static_cast<qDefs::timeUnit>(comboExpUnit->currentIndex());
	FILE_LOG(logINFO) << "Setting exposure time to " << val << " " << qDefs::getUnitString(unit);
	try {
		auto timeNS = qDefs::getNSTime(std::make_pair(val, unit));
		det->setExptime(timeNS);
		CheckAcqPeriodGreaterThanExp();
    } CATCH_HANDLE("Could not set exposure time.", "qTabMeasurement::SetExposureTime", this, &qTabMeasurement::GetExposureTime)
}

void qTabMeasurement::GetAcquisitionPeriod() {
	FILE_LOG(logDEBUG) << "Getting acquisition period";
	disconnect(spinPeriod, SIGNAL(valueChanged(double)), this, SLOT(SetAcquisitionPeriod()));
	disconnect(comboPeriodUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetAcquisitionPeriod()));
	try {
		spinPeriod->setValue(-1);
        auto retval = det->getPeriod().tsquash("Inconsistent acquisition period for all detectors.");
		auto time = qDefs::getUserFriendlyTime(retval);
		spinPeriod->setValue(time.first);
		comboPeriodUnit->setCurrentIndex(static_cast<int>(time.second));
		CheckAcqPeriodGreaterThanExp();
    } CATCH_DISPLAY ("Could not get acquisition period.", "qTabMeasurement::GetAcquisitionPeriod")
	connect(spinPeriod, SIGNAL(valueChanged(double)), this, SLOT(SetAcquisitionPeriod()));
	connect(comboPeriodUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetAcquisitionPeriod()));
}

void qTabMeasurement::SetAcquisitionPeriod() {
	auto val = spinPeriod->value();
	auto unit = static_cast<qDefs::timeUnit>(comboPeriodUnit->currentIndex());
	FILE_LOG(logINFO) << "Setting acquisition period to " << val << " " << qDefs::getUnitString(unit);
	try {
		auto timeNS = qDefs::getNSTime(std::make_pair(val, unit));
		det->setPeriod(timeNS);
		CheckAcqPeriodGreaterThanExp();
    } CATCH_HANDLE("Could not set acquisition period.", "qTabMeasurement::SetAcquisitionPeriod", this, &qTabMeasurement::GetAcquisitionPeriod)
}

void qTabMeasurement::CheckAcqPeriodGreaterThanExp() {
	FILE_LOG(logDEBUG) << "Checking period >= exptime";
	bool error = false;
	if (lblPeriod->isEnabled()) {
		auto exptimeNS = qDefs::getNSTime(std::make_pair(spinExpTime->value(), static_cast<qDefs::timeUnit>(comboExpUnit->currentIndex())));
		auto acqtimeNS = qDefs::getNSTime(std::make_pair(spinPeriod->value(), static_cast<qDefs::timeUnit>(comboPeriodUnit->currentIndex())));
		if (exptimeNS > acqtimeNS) {
			error = true;
			spinPeriod->setToolTip(errPeriodTip);
			lblPeriod->setToolTip(errPeriodTip);
			lblPeriod->setPalette(red);
			lblPeriod->setText("Acquisition Period:*");
		}
	}

	if (!error) {
		spinPeriod->setToolTip(acqPeriodTip);
		lblPeriod->setToolTip(acqPeriodTip);
		lblPeriod->setPalette(lblTimingMode->palette());
		lblPeriod->setText("Acquisition Period:");
	}
}

void qTabMeasurement::GetDelay() {
	FILE_LOG(logDEBUG) << "Getting delay";
	disconnect(spinDelay, SIGNAL(valueChanged(double)), this, SLOT(SetDelay()));
	disconnect(comboDelayUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetDelay()));
	try {
		spinDelay->setValue(-1);
        auto retval = det->getDelayAfterTrigger().tsquash("Inconsistent delay for all detectors.");
		auto time = qDefs::getUserFriendlyTime(retval);
		spinDelay->setValue(time.first);
		comboDelayUnit->setCurrentIndex(static_cast<int>(time.second));
    } CATCH_DISPLAY ("Could not get delay.", "qTabMeasurement::GetDelay")
	connect(spinDelay, SIGNAL(valueChanged(double)), this, SLOT(SetDelay()));
	connect(comboDelayUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetDelay()));
}

void qTabMeasurement::SetDelay() {
	auto val = spinDelay->value();
	auto unit = static_cast<qDefs::timeUnit>(comboDelayUnit->currentIndex());
	FILE_LOG(logINFO) << "Setting delay to " << val << " " << qDefs::getUnitString(unit);
	try {
		auto timeNS = qDefs::getNSTime(std::make_pair(val, unit));
		det->setDelayAfterTrigger(timeNS);
    } CATCH_HANDLE("Could not set delay.", "qTabMeasurement::SetDelay", this, &qTabMeasurement::GetDelay)
}

void qTabMeasurement::GetFileWrite() {
	FILE_LOG(logDEBUG) << "Getting File Write Enable";
	disconnect(chkFile, SIGNAL(toggled(bool)), this, SLOT(SetFileWrite(bool)));
	try {
		dispFileName->setEnabled(true);	// default, even when exception
		lblIndex->setEnabled(true);
		spinIndex->setEnabled(true);
		auto retval = det->getFileWrite().tsquash("Inconsistent file write for all detectors.");
		chkFile->setChecked(retval);
		dispFileName->setEnabled(retval);
		lblIndex->setEnabled(retval);
		spinIndex->setEnabled(retval);
    } CATCH_DISPLAY ("Could not get file over write enable.", "qTabMeasurement::GetFileWrite")
	connect(chkFile, SIGNAL(toggled(bool)), this, SLOT(SetFileWrite(bool)));
}

void qTabMeasurement::SetFileWrite(bool val) {
	FILE_LOG(logINFO) << "Set File Write to " << val;
	try {
        det->setFileWrite(val);
		dispFileName->setEnabled(val);
		lblIndex->setEnabled(val);
		spinIndex->setEnabled(val);
    } CATCH_HANDLE("Could not set file write enable.", "qTabMeasurement::SetFileWrite", this, &qTabMeasurement::GetFileWrite)
}

void qTabMeasurement::GetFileName() {
	FILE_LOG(logDEBUG) << "Getting file name prefix";
	disconnect(dispFileName, SIGNAL(editingFinished()), this, SLOT(SetFileName()));
	try {
        auto retval = det->getFileNamePrefix().tsquash("Inconsistent file name prefix for all detectors.");
		dispFileName->setText(QString(retval.c_str()));
    } CATCH_DISPLAY ("Could not get file name prefix.", "qTabMeasurement::GetFileName")
	connect(dispFileName, SIGNAL(editingFinished()), this, SLOT(SetFileName()));
}

void qTabMeasurement::SetFileName() {
	std::string val = std::string(dispFileName->text().toAscii().constData());
	FILE_LOG(logINFO) << "Setting File Name Prefix:" << val;
	try {
        det->setFileNamePrefix(val);
    } CATCH_HANDLE("Could not set file name prefix.", "qTabMeasurement::SetFileName", this, &qTabMeasurement::GetFileName)

	emit FileNameChangedSignal(dispFileName->text());
}

void qTabMeasurement::GetRunIndex() {
	FILE_LOG(logDEBUG) << "Getting Acquisition File index";
	disconnect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(SetRunIndex(int)));
	try {
        auto retval = det->getAcquisitionIndex().tsquash("Inconsistent file index for all detectors.");
		spinIndex->setValue(retval);
    } CATCH_DISPLAY ("Could not get acquisition file index.", "qTabMeasurement::GetRunIndex")
	connect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(SetRunIndex(int)));
}

void qTabMeasurement::SetRunIndex(int val) {
	FILE_LOG(logINFO) << "Setting Acquisition File Index to " << val;
	try {
        det->setAcquisitionIndex(val);
    } CATCH_HANDLE("Could not set acquisition file index.", "qTabMeasurement::SetRunIndex", this, &qTabMeasurement::GetRunIndex)
}

void qTabMeasurement::GetStartingFrameNumber() {
	FILE_LOG(logDEBUG) << "Getting Starting Frame Number";
	disconnect(spinStartingFrameNumber, SIGNAL(valueChanged(int)), this, SLOT(SetStartingFrameNumber(int)));
	try {
        auto retval = det->getStartingFrameNumber().tsquash("Inconsistent starting frame number for all detectors.");
		spinStartingFrameNumber->setValue(retval);
    } CATCH_DISPLAY ("Could not get starting frame number.", "qTabMeasurement::GetStartingFrameNumber")
	connect(spinStartingFrameNumber, SIGNAL(valueChanged(int)), this, SLOT(SetStartingFrameNumber(int)));
}

void qTabMeasurement::SetStartingFrameNumber(int val) {
	FILE_LOG(logINFO) << "Setting Starting frame number to " << val;
	try {
        det->setStartingFrameNumber(val);
    } CATCH_HANDLE("Could not set starting frame number.", "qTabMeasurement::SetStartingFrameNumber", this, &qTabMeasurement::GetStartingFrameNumber)
}

void qTabMeasurement::ResetProgress() {
	FILE_LOG(logDEBUG) << "Resetting progress";
	lblCurrentFrame->setText("0");
	lblCurrentMeasurement->setText("0");
	progressBar->setValue(0);
}

void qTabMeasurement::UpdateProgress() {
	FILE_LOG(logDEBUG) << "Updating progress";
	progressBar->setValue(plot->GetProgress());
	lblCurrentFrame->setText(QString::number(plot->GetCurrentFrameIndex()));
	lblCurrentMeasurement->setText(QString::number(currentMeasurement));
}

int qTabMeasurement::VerifyOutputDirectoryError() {
	try {
		auto retval = det->getFilePath();
		for (auto &it : retval) {
			det->setFilePath(it);
		}
		return slsDetectorDefs::OK;
    } CATCH_DISPLAY ("Could not set path.", "qTabMeasurement::VerifyOutputDirectoryError")
	return slsDetectorDefs::FAIL; // for exception
}

void qTabMeasurement::StartAcquisition() {
	btnStart->setEnabled(false);
	//if file write enabled and output dir doesnt exist
	if ((chkFile->isChecked()) && (VerifyOutputDirectoryError() == slsDetectorDefs::FAIL)) {
		if (qDefs::Message(qDefs::QUESTION,
				"<nobr>Your data will not be saved.</nobr><br><nobr>Disable File write and Proceed with acquisition anyway?</nobr>",
				"qTabMeasurement::StartAcquisition") == slsDetectorDefs::FAIL) {
			btnStart->setEnabled(true);		
			return;
		} else {
			disconnect(chkFile, SIGNAL(toggled(bool)), this, SLOT(SetFileWrite(bool)));
			chkFile->setChecked(false);
			// cannot wait for signals from chkFile
			SetFileWrite(false);
			connect(chkFile, SIGNAL(toggled(bool)), this, SLOT(SetFileWrite(bool)));
		}
	}

	FILE_LOG(logINFOBLUE) << "Starting Acquisition";
	plot->SetRunning(true);
	isAcquisitionStopped = false;
	currentMeasurement = 0;
	ResetProgress();
	Enable(0);
	progressBar->setValue(0);
	progressTimer->start(100);
	emit EnableTabsSignal(false);
}


void qTabMeasurement::StopAcquisition() {
	FILE_LOG(logINFORED) << "Stopping Acquisition";
	try{
		isAcquisitionStopped = true;
		det->stopDetector();
	} CATCH_DISPLAY("Could not stop acquisition.", "qTabMeasurement::StopAcquisition")
}

void qTabMeasurement::AcquireFinished() {
	// to catch only once (if abort acquire also calls acq finished call back)
	if (!btnStart->isEnabled()) {
		FILE_LOG(logDEBUG) << "Acquire Finished";
		UpdateProgress();
		GetRunIndex();
		if (startingFnumImplemented) {
			GetStartingFrameNumber();
		}
		FILE_LOG(logDEBUG) << "Measurement " << currentMeasurement << " finished";
		// next measurement if acq is not stopped
		if (!isAcquisitionStopped && ((currentMeasurement + 1) < numMeasurements)) {
			++currentMeasurement;
			plot->StartAcquisition();
		}
		// end of acquisition
		else {
			progressTimer->stop();
			Enable(1);
			plot->SetRunning(false);
			btnStart->setEnabled(true);
			emit EnableTabsSignal(true);
		}
	}
}

void qTabMeasurement::AbortAcquire() {
	FILE_LOG(logINFORED) << "Abort Acquire";
	isAcquisitionStopped = true;
	AcquireFinished();
}

void qTabMeasurement::Enable(bool enable) {
	frameTimeResolved->setEnabled(enable);
	frameNotTimeResolved->setEnabled(enable);

	//shortcut each time, else it doesnt work a second time
	btnStart->setShortcut(QApplication::translate("TabMeasurementObject", "Shift+Space", 0, QApplication::UnicodeUTF8));
}

void qTabMeasurement::Refresh() {
	FILE_LOG(logDEBUG) << "**Updating Measurement Tab";

	if (!plot->GetIsRunning()) {
		GetTimingMode();
		GetNumFrames();
		GetExposureTime();
		GetAcquisitionPeriod();
		GetNumTriggers();
		if (delayImplemented) {
			GetDelay();
		}
		if (sampleImplemented) {
			GetNumSamples();
		}
		GetFileWrite();
		GetFileName();
		GetRunIndex();
		if (startingFnumImplemented) {
			GetStartingFrameNumber();
		}
		ResetProgress();
	}

	FILE_LOG(logDEBUG) << "**Updated Measurement Tab";
}
