#include "qTabMeasurement.h"
#include "qDefs.h"
#include "qDrawPlot.h"

#include "string_utils.h"

#include <QStandardItemModel>
#include <QTimer>

#include <cmath>
#include <iostream>

qTabMeasurement::qTabMeasurement(QWidget *parent, multiSlsDetector *detector, qDrawPlot *plot) : QWidget(parent), myDet(detector), myPlot(plot),
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

	delayImplemented = false;
	sampleImplemented = false;
	// enabling according to det type
	switch(myDet->getDetectorTypeAsEnum()) {
		case slsDetectorDefs::MOENCH:
			lblNumSamples->setEnabled(false);
			spinNumSamples->setEnabled(false);
			sampleImplemented = true;
			break;
		case slsDetectorDefs::EIGER:
			lblDelay->setEnabled(false);
			spinDelay->setEnabled(false);
			comboDelayUnit->setEnabled(false);
			delayImplemented = true;
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

		if (myDet->getDetectorTypeAsEnum() != slsDetectorDefs::EIGER) {
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
			if (myDet->getDetectorTypeAsEnum() == slsDetectorDefs::EIGER) {
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
	connect(comboTimingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(SetTimingMode(int)));

	try {
		auto oldMode = comboTimingMode->currentIndex();
        auto retval = myDet->setExternalCommunicationMode();
		switch(retval) {
			case slsDetectorDefs::GET_EXTERNAL_COMMUNICATION_MODE:
				qDefs::Message(qDefs::WARNING, "Timing Mode is inconsistent for all detectors.", "qTabMeasurement::GetTimingMode");
				break;
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
				qDefs::Message(qDefs::WARNING, std::string("Unknown timing mode: ")+ std::to_string(retval), "qTabMeasurement::GetTimingMode");
				break;
		}
    } CATCH_DISPLAY("Could not get timing mode.", "qTabMeasurement::GetTimingMode")

	disconnect(comboTimingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(SetTimingMode(int)));
}

void qTabMeasurement::SetTimingMode(int val) {
	FILE_LOG(logINFO) << "Setting timing mode:" << comboTimingMode->currentText().toAscii().data();
	
	try {
        myDet->setExternalCommunicationMode(static_cast<slsDetectorDefs::externalCommunicationMode>(val));
		EnableWidgetsforTimingMode();
    } CATCH_HANDLE("Could not set timing mode.", "qTabMeasurement::SetTimingMode", this, &qTabMeasurement::GetTimingMode)
}

void qTabMeasurement::GetNumMeasurements() {
	FILE_LOG(logDEBUG) << "Getting number of measurements";
	disconnect(spinNumMeasurements, SIGNAL(valueChanged(int)), this, SLOT(SetNumMeasurements(int)));
	spinNumMeasurements->setValue(myPlot->GetNumMeasurements());
	connect(spinNumMeasurements, SIGNAL(valueChanged(int)), this, SLOT(SetNumMeasurements(int)));
}

void qTabMeasurement::SetNumMeasurements(int val) {
	FILE_LOG(logINFO) << "Setting Number of Measurements to " << val;
	myPlot->SetNumMeasurements(val);
}

void qTabMeasurement::GetNumFrames() {
	FILE_LOG(logDEBUG) << "Getting number of frames";
	disconnect(spinNumFrames, SIGNAL(valueChanged(int)), this, SLOT(SetNumFrames(int)));

	try {
        auto retval = myDet->setTimer(slsDetectorDefs::FRAME_NUMBER);
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Number of frames is inconsistent for all detectors.", "qTabMeasurement::GetNumFrames");
		} 
		spinNumFrames->setValue(retval);
    } CATCH_DISPLAY ("Could not get number of frames.", "qTabMeasurement::GetNumFrames")

	connect(spinNumFrames, SIGNAL(valueChanged(int)), this, SLOT(SetNumFrames(int)));
}

void qTabMeasurement::SetNumFrames(int val) {
	FILE_LOG(logINFO) << "Setting number of frames to " << val;

	try {
        myDet->setTimer(slsDetectorDefs::FRAME_NUMBER, val);
    } CATCH_HANDLE("Could not set number of frames.", "qTabMeasurement::SetNumFrames", this, &qTabMeasurement::GetNumFrames)
}

void qTabMeasurement::GetNumTriggers() {
	FILE_LOG(logDEBUG) << "Getting number of triggers";
	disconnect(spinNumTriggers, SIGNAL(valueChanged(int)), this, SLOT(SetNumTriggers(int)));

	try {
        auto retval = myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER);
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Number of triggers is inconsistent for all detectors.", "qTabMeasurement::GetNumTriggers");
		} 
		spinNumTriggers->setValue(retval);
    } CATCH_DISPLAY ("Could not get number of frames.", "qTabMeasurement::GetNumTriggers")

	connect(spinNumTriggers, SIGNAL(valueChanged(int)), this, SLOT(SetNumTriggers(int)));
}

void qTabMeasurement::SetNumTriggers(int val) {
	FILE_LOG(logINFO) << "Setting number of triggers to " << val;

	try {
        myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER, val);
    } CATCH_HANDLE("Could not set number of triggers.", "qTabMeasurement::SetNumTriggers", this, &qTabMeasurement::GetNumTriggers)
}

void qTabMeasurement::GetNumSamples() {
	FILE_LOG(logDEBUG) << "Getting number of samples";
	disconnect(spinNumSamples, SIGNAL(valueChanged(int)), this, SLOT(SetNumSamples(int)));

	try {
        auto retval = myDet->setTimer(slsDetectorDefs::ANALOG_SAMPLES);
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Number of analog samples is inconsistent for all detectors.", "qTabMeasurement::GetNumSamples");
		} 
		retval = myDet->setTimer(slsDetectorDefs::DIGITAL_SAMPLES);
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Number of digital samples is inconsistent for all detectors.", "qTabMeasurement::GetNumSamples");
		} 
		spinNumSamples->setValue(retval);
    } CATCH_DISPLAY ("Could not get number of samples.", "qTabMeasurement::GetNumSamples")

	connect(spinNumSamples, SIGNAL(valueChanged(int)), this, SLOT(SetNumSamples(int)));
}

void qTabMeasurement::SetNumSamples(int val) {
	FILE_LOG(logINFO) << "Setting number of samples to " << val;

	try {
        myDet->setTimer(slsDetectorDefs::ANALOG_SAMPLES, val);
		myDet->setTimer(slsDetectorDefs::DIGITAL_SAMPLES, val);
    } CATCH_HANDLE("Could not set number of samples.", "qTabMeasurement::SetNumSamples", this, &qTabMeasurement::GetNumSamples)
}

void qTabMeasurement::GetExposureTime() {
	FILE_LOG(logDEBUG) << "Getting exposure time";
	disconnect(spinExpTime, SIGNAL(valueChanged(double)), this, SLOT(SetExposureTime()));
	disconnect(comboExpUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetExposureTime()));

	try {
        auto retval = myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME);
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Exposure Time is inconsistent for all detectors.", "qTabMeasurement::GetExposureTime");
			spinExpTime->setValue(-1);
		} else {
			auto time = qDefs::getCorrectTime(static_cast<double>(retval) * (1E-9));
			spinExpTime->setValue(time.first);
			comboExpUnit->setCurrentIndex(static_cast<int>(time.second));

			CheckAcqPeriodGreaterThanExp();
		}
    } CATCH_DISPLAY ("Could not get exposure time.", "qTabMeasurement::GetExposureTime")

	connect(spinExpTime, SIGNAL(valueChanged(double)), this, SLOT(SetExposureTime()));
	connect(comboExpUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetExposureTime()));
}

void qTabMeasurement::SetExposureTime() {
	auto val = spinExpTime->value();
	auto unit = static_cast<qDefs::timeUnit>(comboExpUnit->currentIndex());
	FILE_LOG(logINFO) << "Setting exposure time to " << val << " " << qDefs::getUnitString(unit);
	
	try {
		double timeNS = qDefs::getNSTime(unit, val);
		myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME, std::lround(timeNS));
		CheckAcqPeriodGreaterThanExp();
    } CATCH_HANDLE("Could not set exposure time.", "qTabMeasurement::SetExposureTime", this, &qTabMeasurement::GetExposureTime)
}

void qTabMeasurement::GetAcquisitionPeriod() {
	FILE_LOG(logDEBUG) << "Getting acquisition period";
	disconnect(spinPeriod, SIGNAL(valueChanged(double)), this, SLOT(SetAcquisitionPeriod()));
	disconnect(comboPeriodUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetAcquisitionPeriod()));

	try {
        auto retval = myDet->setTimer(slsDetectorDefs::FRAME_PERIOD);
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Acquisition Period is inconsistent for all detectors.", "qTabMeasurement::GetAcquisitionPeriod");
			spinPeriod->setValue(-1);
		} else {
			auto time = qDefs::getCorrectTime(static_cast<double>(retval) * (1E-9));
			spinPeriod->setValue(time.first);
			comboPeriodUnit->setCurrentIndex(static_cast<int>(time.second));

			CheckAcqPeriodGreaterThanExp();
		}
    } CATCH_DISPLAY ("Could not get acquisition period.", "qTabMeasurement::GetAcquisitionPeriod")

	connect(spinPeriod, SIGNAL(valueChanged(double)), this, SLOT(SetAcquisitionPeriod()));
	connect(comboPeriodUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetAcquisitionPeriod()));
}

void qTabMeasurement::SetAcquisitionPeriod() {
	auto val = spinPeriod->value();
	auto unit = static_cast<qDefs::timeUnit>(comboPeriodUnit->currentIndex());
	FILE_LOG(logINFO) << "Setting acquisition period to " << val << " " << qDefs::getUnitString(unit);
	
	try {
		double timeNS = qDefs::getNSTime(unit, val);
		myDet->setTimer(slsDetectorDefs::FRAME_PERIOD, std::lround(timeNS));
		CheckAcqPeriodGreaterThanExp();
    } CATCH_HANDLE("Could not set acquisition period.", "qTabMeasurement::SetAcquisitionPeriod", this, &qTabMeasurement::GetAcquisitionPeriod)
}

void qTabMeasurement::CheckAcqPeriodGreaterThanExp() {
	FILE_LOG(logDEBUG) << "Checking period >= exptime";
	bool error = false;
	if (lblPeriod->isEnabled()) {
		double exptimeNS = qDefs::getNSTime(static_cast<qDefs::timeUnit>(comboExpUnit->currentIndex()), spinExpTime->value());
		double acqtimeNS = qDefs::getNSTime(static_cast<qDefs::timeUnit>(comboPeriodUnit->currentIndex()), spinPeriod->value());
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
        auto retval = myDet->setTimer(slsDetectorDefs::DELAY_AFTER_TRIGGER);
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Delay is inconsistent for all detectors.", "qTabMeasurement::GetDelay");
			spinDelay->setValue(-1);
		} else {
			auto time = qDefs::getCorrectTime(static_cast<double>(retval) * (1E-9));
			spinDelay->setValue(time.first);
			comboDelayUnit->setCurrentIndex(static_cast<int>(time.second));

			CheckAcqPeriodGreaterThanExp();
		}
    } CATCH_DISPLAY ("Could not get delay.", "qTabMeasurement::GetDelay")

	connect(spinDelay, SIGNAL(valueChanged(double)), this, SLOT(SetDelay()));
	connect(comboDelayUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(SetDelay()));
}

void qTabMeasurement::SetDelay() {
	auto val = spinDelay->value();
	auto unit = static_cast<qDefs::timeUnit>(comboDelayUnit->currentIndex());
	FILE_LOG(logINFO) << "Setting delay to " << val << " " << qDefs::getUnitString(unit);
	
	try {
		double timeNS = qDefs::getNSTime(unit, val);
		myDet->setTimer(slsDetectorDefs::DELAY_AFTER_TRIGGER, std::lround(timeNS));
		CheckAcqPeriodGreaterThanExp();
    } CATCH_HANDLE("Could not set delay.", "qTabMeasurement::SetDelay", this, &qTabMeasurement::GetDelay)
}

void qTabMeasurement::GetFileWrite() {
	FILE_LOG(logDEBUG) << "Getting File Write Enable";
	disconnect(chkFile, SIGNAL(toggled(bool)), this, SLOT(SetFileWrite(bool)));

	try {
		dispFileName->setEnabled(true);	// default, even when exception
		int retval = myDet->getFileWrite();
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "File write is inconsistent for all detectors.", "qTabMeasurement::GetFileWrite");
			dispFileName->setEnabled(true);
		} else {
			chkFile->setChecked(retval == 0 ? false : true);
			dispFileName->setEnabled(chkFile->isChecked());
		}
    } CATCH_DISPLAY ("Could not get file over write enable.", "qTabMeasurement::GetFileWrite")

	connect(chkFile, SIGNAL(toggled(bool)), this, SLOT(SetFileWrite(bool)));
}

void qTabMeasurement::SetFileWrite(bool val) {
	FILE_LOG(logINFO) << "Set File Write to " << val;

	try {
        myDet->setFileWrite(val);
		dispFileName->setEnabled(chkFile->isChecked());
    } CATCH_HANDLE("Could not set file write enable.", "qTabMeasurement::SetFileWrite", this, &qTabMeasurement::GetFileWrite)
}

void qTabMeasurement::GetFileName() {
	FILE_LOG(logDEBUG) << "Getting file name prefix";
	disconnect(dispFileName, SIGNAL(editingFinished()), this, SLOT(SetFileName()));

	try {
        auto retval = myDet->getFileName();
		dispFileName->setText(QString(retval.c_str()));
    } CATCH_DISPLAY ("Could not get file name prefix.", "qTabMeasurement::GetFileName")
	
	connect(dispFileName, SIGNAL(editingFinished()), this, SLOT(SetFileName()));
}

void qTabMeasurement::SetFileName() {
	std::string val = std::string(dispFileName->text().toAscii().constData());
	FILE_LOG(logINFO) << "Setting File Name Prefix:" << val;
	try {
        myDet->setFileName(val);
    } CATCH_HANDLE("Could not set file name prefix.", "qTabMeasurement::SetFileName", this, &qTabMeasurement::GetFileName)

	emit FileNameChangedSignal(dispFileName->text());
}

void qTabMeasurement::GetRunIndex() {
	FILE_LOG(logDEBUG) << "Getting Acquisition File index";
	disconnect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(SetRunIndex(int)));

	try {
        auto retval = myDet->getFileIndex();
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Acquisition File Index is inconsistent for all detectors.", "qTabMeasurement::GetRunIndex");
		} 
		spinIndex->setValue(retval);
    } CATCH_DISPLAY ("Could not get acquisition file index.", "qTabMeasurement::GetRunIndex")

	connect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(SetRunIndex(int)));
}

void qTabMeasurement::SetRunIndex(int val) {
	FILE_LOG(logINFO) << "Setting Acquisition File Index to " << val;

	try {
        myDet->setFileIndex(val);
    } CATCH_HANDLE("Could not set acquisition file index.", "qTabMeasurement::SetRunIndex", this, &qTabMeasurement::GetRunIndex)
}

void qTabMeasurement::ResetProgress() {
	FILE_LOG(logDEBUG) << "Resetting progress";
	lblCurrentFrame->setText("");
	lblCurrentMeasurement->setText("");
	progressBar->setValue(0);
}

void qTabMeasurement::UpdateProgress() {
	FILE_LOG(logDEBUG) << "Updating progress";
	progressBar->setValue(myPlot->GetProgress());
	int64_t temp = myPlot->GetCurrentFrameIndex();
	lblCurrentFrame->setText(temp >= 0 ? QString::number(temp) : "");
	temp = myPlot->GetCurrentMeasurementIndex();
	lblCurrentMeasurement->setText(temp >= 0 ? QString::number(temp) : "");
}

int qTabMeasurement::VerifyOutputDirectoryError() {
	try {
		auto retval = myDet->getFilePath();
		// multi
		if (retval.find('+') == std::string::npos) {
			myDet->setFilePath(retval);
		}
		//single
		else {
			const auto &paths = sls::split(retval, '+');
			for (size_t det = 0; det < paths.size(); ++det) {
				myDet->setFilePath(paths[det], det);
			}
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
	lblCurrentFrame->setText("");
	lblCurrentMeasurement->setText("");
	Enable(0);
	progressBar->setValue(0);
	progressTimer->start(100);
	emit StartSignal();
}


void qTabMeasurement::StopAcquisition() {
	FILE_LOG(logINFORED) << "Stopping Acquisition";
	try{
		myPlot->SetStopSignal();
		myDet->stopAcquisition();
	} CATCH_DISPLAY("Could not stop acquisition.", "qTabMeasurement::StopAcquisition")
}

void qTabMeasurement::UpdateFinished() {
	UpdateProgress();
	GetRunIndex();
	progressTimer->stop();
	Enable(1);
	btnStart->setEnabled(true);
}

void qTabMeasurement::Enable(bool enable) {
	frameTimeResolved->setEnabled(enable);
	frameNotTimeResolved->setEnabled(enable);

	//shortcut each time, else it doesnt work a second time
	btnStart->setShortcut(QApplication::translate("TabMeasurementObject", "Shift+Space", 0, QApplication::UnicodeUTF8));
}

void qTabMeasurement::Refresh() {
	FILE_LOG(logDEBUG) << "**Updating Measurement Tab";

	if (!myPlot->GetIsRunning()) {
		GetTimingMode();
		GetNumMeasurements();
		GetNumFrames();
		GetExposureTime();
		GetAcquisitionPeriod();
		GetNumTriggers();
		if (delayImplemented) {cprintf(BLUE, "Delay implemented\n");
			GetDelay();
		}
		if (sampleImplemented) {
			GetNumSamples();
		}
		GetFileWrite();
		GetFileName();
		GetRunIndex();
		ResetProgress();
	}

	FILE_LOG(logDEBUG) << "**Updated Measurement Tab";
}
