#include "qTabMeasurement.h"
#include "qDetectorMain.h"

#include "multiSlsDetector.h"

#include <iostream>


qTabMeasurement::qTabMeasurement(QWidget *parent, multiSlsDetector *&detector, qDrawPlot *&plot) : QWidget(parent), myDet(detector), myPlot(plot) {
	setupUi(this);
	SetupWidgetWindow();
	Initialization();

	// updated after slots to enable/disable widgets
	GetTimingModeFromDetector(true);

	FILE_LOG(logDEBUG) << "Measurement ready";
}


qTabMeasurement::~qTabMeasurement() {
	delete myDet;
}


bool qTabMeasurement::GetStartStatus(){
	return (!btnStart->isEnabled());
}


void qTabMeasurement::ClickStartStop(){
	startAcquisition();
	myPlot->SetClientInitiated();
}


int qTabMeasurement::GetProgress(){
	return progressBar->value();
}


void qTabMeasurement::SetupWidgetWindow() {

	detType = myDet->getDetectorTypeAsEnum();

	//measurements
	spinNumMeasurements->setValue((int)myDet->setTimer(slsDetectorDefs::MEASUREMENTS_NUMBER, -1));
	//frames
	spinNumFrames->setValue((int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER, -1));
	//Exp Time
	qDefs::timeUnit unit;
	double time = qDefs::getCorrectTime(unit, ((double)(myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME, -1) * (1E-9))));
	spinExpTime->setValue(time);
	comboExpUnit->setCurrentIndex((int)unit);
	//period
	time = qDefs::getCorrectTime(unit, ((double)(myDet->setTimer(slsDetectorDefs::FRAME_PERIOD, -1) * (1E-9))));
	spinPeriod->setValue(time);
	comboPeriodUnit->setCurrentIndex((int)unit);
	//triggers
	spinNumTriggers->setValue((int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER, -1));
	//delay
	if (detType == slsDetectorDefs::EIGER) {
		lblDelay->setEnabled(false);
		spinDelay->setEnabled(false);
		comboDelayUnit->setEnabled(false);
	} else {
		time = qDefs::getCorrectTime(unit, ((double)(myDet->setTimer(slsDetectorDefs::DELAY_AFTER_TRIGGER, -1) * (1E-9))));
		spinDelay->setValue(time);
		comboDelayUnit->setCurrentIndex((int)unit);
	}
	//samples
	if (detType != slsDetectorDefs::MOENCH) {
		lblNumSamples->setEnabled(false);
		spinNumSamples->setEnabled(false);
	} else {
		spinNumSamples->setValue((int)myDet->setTimer(slsDetectorDefs::SAMPLES, -1));
	}
	//file name
	dispFileName->setText(QString(myDet->getFileName().c_str()));
	//file write enable
	chkFile->setChecked(myDet->enableWriteToFile());
	dispFileName->setEnabled(chkFile->isChecked());
	//file index
	spinIndex->setValue(myDet->getFileIndex());
	//only initially
	lblProgressIndex->setText(QString::number(0));
	progressBar->setValue(0);

	//Timer to update the progress bar
	progressTimer = new QTimer(this);

	//Hide the error message
	red = QPalette();
	red.setColor(QPalette::Active, QPalette::WindowText, Qt::red);
	acqPeriodTip = spinPeriod->toolTip();
	errPeriodTip = QString("<nobr>Frame period between exposures.</nobr><br>"
			"<nobr> #period#</nobr><br><br>") +
					QString("<nobr><font color=\"red\"><b>Acquisition Period</b> should be"
							" greater than or equal to <b>Exposure Time</b>.</font></nobr>");

	SetupTimingMode();
	// update timing mode after enabling slots to enable/disable widgets

	qDefs::checkErrorMessage(myDet, "qTabMeasurement::SetupWidgetWindow");
}


void qTabMeasurement::SetupTimingMode() {
	//To be able to index items on a combo box
	model = qobject_cast<QStandardItemModel *>(comboTimingMode->model());
	QModelIndex index[NUM_TIMING_MODES];
	QStandardItem *item[NUM_TIMING_MODES];
	if (model) {
		for (int i = 0; i < NUM_TIMING_MODES; i++) {
			index[i] = model->index(i, comboTimingMode->modelColumn(), comboTimingMode->rootModelIndex());
			item[i] = model->itemFromIndex(index[i]);
		}

		if (detType != slsDetectorDefs::EIGER) {
			item[(int)GATED]->setEnabled(false);
			item[(int)BURST_TRIGGER]->setEnabled(false);
		}
	}
}


void qTabMeasurement::Initialization() {
	//measurements
	connect(spinNumMeasurements, SIGNAL(valueChanged(int)), this, SLOT(setNumMeasurements(int)));
	//frames
	connect(spinNumFrames, SIGNAL(valueChanged(int)), this, SLOT(setNumFrames(int)));
	//exposure time
	connect(spinExpTime, SIGNAL(valueChanged(double)), this, SLOT(setExposureTime()));
	connect(comboExpUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(setExposureTime()));
	//frame period between exposures
	connect(spinPeriod, SIGNAL(valueChanged(double)), this, SLOT(setAcquisitionPeriod()));
	connect(comboPeriodUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(setAcquisitionPeriod()));
	//triggers
	connect(spinNumTriggers, SIGNAL(valueChanged(int)), this, SLOT(setNumTriggers(int)));
	//Delay After Trigger
	if (spinDelay->isEnabled()) {
		connect(spinDelay, SIGNAL(valueChanged(double)), this, SLOT(setDelay()));
		connect(comboDelayUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(setDelay()));
	}
	//samples
	if (spinSamples->isEnabled())
		connect(spinNumSamples, SIGNAL(valueChanged(int)), this, SLOT(setNumSamples(int)));
	//file name
	connect(dispFileName, SIGNAL(editingFinished()), this, SLOT(setFileName()));
	//file write enable
	connect(chkFile, SIGNAL(toggled(bool)), this, SLOT(EnableFileWrite(bool)));
	//file index
	connect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(setRunIndex(int)));
	//progress bar
	connect(progressTimer, SIGNAL(timeout()), this, SLOT(UpdateProgress()));
	//start acquisition
	connect(btnStart, SIGNAL(clicked()), this, SLOT(startAcquisition()));
	//stop acquisition
	connect(btnStop, SIGNAL(clicked()), this, SLOT(stopAcquisition()));
	//timing mode
	connect(comboTimingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(SetTimingMode(int)));
}



void qTabMeasurement::GetTimingModeFromDetector(bool startup) {
	FILE_LOG(logDEBUG) << "Getting timing mode";

	//Get timing mode from detector
	slsDetectorDefs::externalCommunicationMode mode = myDet->setExternalCommunicationMode();
	qDefs::checkErrorMessage(myDet, "qTabMeasurement::GetTimingModeFromDetector");

	//if the mode is enabled
	if (model && model->itemFromIndex(model->index(mode, comboTimingMode->modelColumn(), comboTimingMode->rootModelIndex()))->isEnabled()) {

		// first time
		if (startup) {
			// explicitly call SetTimingMode first time
			disconnect(comboTimingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(SetTimingMode(int)));
			comboTimingMode->setCurrentIndex((int)mode);
			SetTimingMode((int)mode);
			connect(comboTimingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(SetTimingMode(int)));
			return;
		}

		// mode is the different from current (from refresh),
		if (comboTimingMode->currentIndex() != mode) {
			comboTimingMode->setCurrentIndex((int)mode);
		}

	}
	// Mode not enabled
	else {
		//check if the detector is not even connected
		std::string offline = myDet->checkOnline();
		qDefs::checkErrorMessage(myDet, "qTabMeasurement::GetTimingModeFromDetector");
		if (!offline.empty()) {
			qDefs::Message(qDefs::CRITICAL, std::string("<nobr>The detector(s)  <b>") + offline + std::string(" </b> is/are not connected.  Exiting GUI.</nobr>"), "Main");
			FILE_LOG(logERROR) << "The detector(s)  " << offline << "  is/are not connected. Exiting GUI.";
			exit(-1);
		}

		// onlne but mismatch in timing mode
		FILE_LOG(logWARNING) << "Unknown Timing Mode " << mode << " from detector";
		qDefs::Message(qDefs::WARNING, "Unknown Timing Mode from detector.\n\nSetting timing mode to Auto.",
				"qTabMeasurement::GetTimingModeFromDetector");
		comboTimingMode->setCurrentIndex((int)AUTO);
		SetTimingMode((int)AUTO);
	}
}


void qTabMeasurement::setNumMeasurements(int val) {
	FILE_LOG(logINFO) << "Setting Number of Measurements to " << val;

	myDet->setTimer(slsDetectorDefs::MEASUREMENTS_NUMBER, val);

	disconnect(spinNumMeasurements, SIGNAL(valueChanged(int)), this, SLOT(setNumMeasurements(int)));
	spinNumMeasurements->setValue((int)myDet->setTimer(slsDetectorDefs::MEASUREMENTS_NUMBER, -1));
	connect(spinNumMeasurements, SIGNAL(valueChanged(int)), this, SLOT(setNumMeasurements(int)));

	qDefs::checkErrorMessage(myDet, "qTabMeasurement::setNumMeasurements");
}


void qTabMeasurement::setNumFrames(int val) {
	FILE_LOG(logINFO) << "Setting number of frames to " << val;

	myDet->setTimer(slsDetectorDefs::FRAME_NUMBER, val);

	disconnect(spinNumFrames, SIGNAL(valueChanged(int)), this, SLOT(setNumFrames(int)));
	spinNumFrames->setValue((int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER, -1));
	connect(spinNumFrames, SIGNAL(valueChanged(int)), this, SLOT(setNumFrames(int)));

	qDefs::checkErrorMessage(myDet, "qTabMeasurement::setNumFrames");
}


void qTabMeasurement::setExposureTime() {
	//Get the value of timer in ns
	double exptimeNS = qDefs::getNSTime((qDefs::timeUnit)comboExpUnit->currentIndex(), spinExpTime->value());
	FILE_LOG(logINFO) << "Setting acquisition time to " << exptimeNS << " ns"
			<< "/" << spinExpTime->value() << qDefs::getUnitString((qDefs::timeUnit)comboExpUnit->currentIndex());

	myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME, (int64_t)exptimeNS);
	qDefs::checkErrorMessage(myDet, "qTabMeasurement::setExposureTime");

	CheckAcqPeriodGreaterThanExp();

	//Check if the interval between plots is ok
	emit CheckPlotIntervalSignal();
}


void qTabMeasurement::setAcquisitionPeriod() {
	//Get the value of timer in ns
	double acqtimeNS = qDefs::getNSTime((qDefs::timeUnit)comboPeriodUnit->currentIndex(), spinPeriod->value());
	FILE_LOG(logINFO) << "Setting frame period between exposures to " << acqtimeNS << " ns"
			<< "/" << spinPeriod->value() << qDefs::getUnitString((qDefs::timeUnit)comboPeriodUnit->currentIndex());

	myDet->setTimer(slsDetectorDefs::FRAME_PERIOD, (int64_t)acqtimeNS);
	qDefs::checkErrorMessage(myDet, "qTabMeasurement::setAcquisitionPeriod");

	CheckAcqPeriodGreaterThanExp();
	//Check if the interval between plots is ok
	emit CheckPlotIntervalSignal();
}


void qTabMeasurement::setNumTriggers(int val) {
	FILE_LOG(logINFO) << "Setting number of triggers to " << val;

	myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER, val);

	disconnect(spinNumTriggers, SIGNAL(valueChanged(int)), this, SLOT(setNumTriggers(int)));
	spinNumTriggers->setValue((int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER, -1));
	connect(spinNumTriggers, SIGNAL(valueChanged(int)), this, SLOT(setNumTriggers(int)));

	qDefs::checkErrorMessage(myDet, "qTabMeasurement::setNumTriggers");
}


void qTabMeasurement::setDelay() {
	//Get the value of timer in ns
	double exptimeNS = qDefs::getNSTime((qDefs::timeUnit)comboDelayUnit->currentIndex(), spinDelay->value());
	FILE_LOG(logINFO) << "Setting delay after trigger to " << exptimeNS << " ns"
			<< "/" << spinDelay->value() << qDefs::getUnitString((qDefs::timeUnit)comboDelayUnit->currentIndex());

	myDet->setTimer(slsDetectorDefs::DELAY_AFTER_TRIGGER, (int64_t)exptimeNS);

	qDefs::checkErrorMessage(myDet, "qTabMeasurement::setDelay");
}


void qTabMeasurement::setNumSamples(int val) {
	FILE_LOG(logINFO) << "Setting number of samples to " << val;

	myDet->setTimer(slsDetectorDefs::SAMPLES, val);

	disconnect(spinNumSamples, SIGNAL(valueChanged(int)), this, SLOT(setNumSamples(int)));
	spinNumSamples->setValue((int)myDet->setTimer(slsDetectorDefs::SAMPLES, -1));
	connect(spinNumSamples, SIGNAL(valueChanged(int)), this, SLOT(setNumSamples(int)));

	qDefs::checkErrorMessage(myDet, "qTabMeasurement::setNumSamples");
}


void qTabMeasurement::setFileName() {
	QString fName = dispFileName->text();
	FILE_LOG(logINFO) << "Setting File name to " << fName.toAscii().constData();

	myDet->setFileName(fName.toAscii().data());

	disconnect(dispFileName, SIGNAL(editingFinished()), this, SLOT(setFileName()));
	dispFileName->setText(QString(myDet->getFileName().c_str()));
	connect(dispFileName, SIGNAL(editingFinished()), this, SLOT(setFileName()));

	qDefs::checkErrorMessage(myDet, "qTabMeasurement::setFileName");
}


void qTabMeasurement::EnableFileWrite(bool enable) {
	FILE_LOG(logINFO) << "Enable File Write:" << enable;

	myDet->enableWriteToFile(enable);
	disconnect(chkFile, SIGNAL(toggled(bool)), this, SLOT(EnableFileWrite(bool)));
	chkFile->setChecked(myDet->enableWriteToFile());
	connect(chkFile, SIGNAL(toggled(bool)), this, SLOT(EnableFileWrite(bool)));

	bool ret = chkFile->isChecked();
	dispFileName->setEnabled(ret);
	if (ret)
		setFileName();
	myPlot->SetEnableFileWrite(ret);
	qDefs::checkErrorMessage(myDet, "qTabMeasurement::EnableFileWrite");
}


void qTabMeasurement::setRunIndex(int index) {
	FILE_LOG(logINFO) << "Setting File Index to " << index;

	myDet->setFileIndex(index);

	disconnect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(setRunIndex(int)));
	spinIndex->setValue(myDet->getFileIndex());
	connect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(setRunIndex(int)));

	qDefs::checkErrorMessage(myDet, "qTabMeasurement::setRunIndex");
}


void qTabMeasurement::UpdateProgress() {
	progressBar->setValue((int)myPlot->GetProgress());
	lblProgressIndex->setText(QString::number(myPlot->GetFrameIndex()));
	qDefs::checkErrorMessage(myDet, "qTabMeasurement::UpdateProgress");
}


void qTabMeasurement::startAcquisition() {
	btnStart->setEnabled(false);
	//if file write enabled and output dir doesnt exist
	if ((chkFile->isChecked()) && (VerifyOutputDirectoryError() == slsDetectorDefs::FAIL)) {
		if (qDefs::Message(qDefs::QUESTION,
				"<nobr>Your data will not be saved.</nobr><br><nobr>Disable File write and Proceed with acquisition anyway?</nobr>",
				"qTabMeasurement::startAcquisition") == slsDetectorDefs::FAIL) {
			btnStart->setEnabled(true);
			return;
		} else {
			//done because for receiver it cant save a file with blank file path and returns without acquiring even to the gui
			disconnect(chkFile, SIGNAL(toggled(bool)), this, SLOT(EnableFileWrite(bool)));
			EnableFileWrite(false);
			connect(chkFile, SIGNAL(toggled(bool)), this, SLOT(EnableFileWrite(bool)));
		}
	}

	FILE_LOG(logINFOBLUE) << "Starting Acquisition";
	lblProgressIndex->setText(QString::number(0));
	Enable(0);
	progressBar->setValue(0);
	progressTimer->start(100);

	emit StartSignal();
	qDefs::checkErrorMessage(myDet, "qTabMeasurement::startAcquisition");
}


void qTabMeasurement::stopAcquisition() {
	FILE_LOG(logINFORED) << "Stopping Acquisition";
	myDet->stopAcquisition();
	qDefs::checkErrorMessage(myDet, "qTabMeasurement::stopAcquisition");
}


void qTabMeasurement::Enable(bool enable) {
	frameTimeResolved->setEnabled(enable);
	frameNotTimeResolved->setEnabled(enable);

	//shortcut each time, else it doesnt work a second time
	btnStart->setShortcut(QApplication::translate("TabMeasurementObject", "Shift+Space", 0, QApplication::UnicodeUTF8));
}

int qTabMeasurement::VerifyOutputDirectoryError() {
	for (int i = 0; i < myDet->getNumberOfDetectors(); i++) {
		if (getModuleErrorMask(i) == FILE_PATH_DOES_NOT_EXIST) {
			return slsDetectorDefs:: FAIL;
		}
		return slsDetectorDefs:: OK;
	}
}


void qTabMeasurement::UpdateFinished() {
	UpdateProgress();
	disconnect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(setRunIndex(int)));
	spinIndex->setValue(myDet->getFileIndex());
	connect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(setRunIndex(int)));
	progressTimer->stop();

	Enable(1);
	btnStart->setEnabled(true);
	qDefs::checkErrorMessage(myDet, "qTabMeasurement::UpdateFinished");
}


void qTabMeasurement::SetCurrentMeasurement(int val) {
	if ((val) < spinNumMeasurements->value())
		lblCurrentMeasurement->setText(QString::number(val));
}


void qTabMeasurement::CheckAcqPeriodGreaterThanExp() {
	bool error = false;
	if (lblPeriod->isEnabled()) {
		double exptimeNS = qDefs::getNSTime((qDefs::timeUnit)comboExpUnit->currentIndex(), spinExpTime->value());
		double acqtimeNS = qDefs::getNSTime((qDefs::timeUnit)comboPeriodUnit->currentIndex(), spinPeriod->value());
		if (exptimeNS > acqtimeNS) {
			error = true;
			spinPeriod->setToolTip(errPeriodTip);
			lblPeriod->setToolTip(errPeriodTip);
			lblPeriod->setPalette(red);
			lblPeriod->setText("Acquisition Period:*");
		}
	}

	// no error or period disabled
	if (!error) {
		spinPeriod->setToolTip(acqPeriodTip);
		lblPeriod->setToolTip(acqPeriodTip);
		lblPeriod->setPalette(lblTimingMode->palette());
		lblPeriod->setText("Acquisition Period:");
	}
}


void qTabMeasurement::SetTimingMode(int mode) {
	FILE_LOG(logINFO) << "Setting Timing mode to " << comboTimingMode->currentText().toAscii().data();

	//Default settings
	lblNumFrames->setEnabled(false);
	spinNumFrames->setEnabled(false);
	lblExpTime->setEnabled(false);
	spinExpTime->setEnabled(false);
	comboExpUnit->setEnabled(false);
	lblPeriod->setEnabled(false);
	spinPeriod->setEnabled(false);
	comboPeriodUnit->setEnabled(false);
	lblNumTriggers->setEnabled(false);
	spinNumTriggers->setEnabled(false);
	lblDelay->setEnabled(false);
	spinDelay->setEnabled(false);
	comboDelayUnit->setEnabled(false);

	bool success = false;
	switch (mode) {
	case AUTO: //#Frames, ExpTime, Period
		spinNumTriggers->setValue(1);
		lblNumFrames->setEnabled(true);
		spinNumFrames->setEnabled(true);
		lblExpTime->setEnabled(true);
		spinExpTime->setEnabled(true);
		comboExpUnit->setEnabled(true);
		lblPeriod->setEnabled(true);
		spinPeriod->setEnabled(true);
		comboPeriodUnit->setEnabled(true);
		if (myDet->setExternalCommunicationMode(slsDetectorDefs::AUTO_TIMING) == slsDetectorDefs::AUTO_TIMING)
			success = true;
		break;
	case TRIGGER:                   // #Triggers, ExpTime, (#Frames, Period, Delay)
		if (detType == slsDetectorDefs::EIGER) //only 1 frame for each trigger for eiger
			spinNumFrames->setValue(1);
		else {
			lblNumFrames->setEnabled(true);
			spinNumFrames->setEnabled(true);
			lblDelay->setEnabled(true);
			spinDelay->setEnabled(true);
			comboDelayUnit->setEnabled(true);
			lblPeriod->setEnabled(true);
			spinPeriod->setEnabled(true);
			comboPeriodUnit->setEnabled(true);
		}
		lblExpTime->setEnabled(true);
		spinExpTime->setEnabled(true);
		comboExpUnit->setEnabled(true);
		lblNumTriggers->setEnabled(true);
		spinNumTriggers->setEnabled(true);
		if (myDet->setExternalCommunicationMode(slsDetectorDefs::TRIGGER_EXPOSURE) == slsDetectorDefs::TRIGGER_EXPOSURE)
			success = true;
		break;

	case GATED: //#Frames (Only Eiger)

		spinNumTriggers->setValue(1);
		lblNumFrames->setEnabled(true);
		spinNumFrames->setEnabled(true);

		if (myDet->setExternalCommunicationMode(slsDetectorDefs::GATED) == slsDetectorDefs::GATED)
			success = true;
		break;

	case BURST_TRIGGER: //#Frames, ExpTime, Period (Only Eiger)
		spinNumTriggers->setValue(1);

		lblNumFrames->setEnabled(true);
		spinNumFrames->setEnabled(true);
		lblExpTime->setEnabled(true);
		spinExpTime->setEnabled(true);
		comboExpUnit->setEnabled(true);
		lblPeriod->setEnabled(true);
		spinPeriod->setEnabled(true);
		comboPeriodUnit->setEnabled(true);
		if (myDet->setExternalCommunicationMode(slsDetectorDefs::BURST_TRIGGER) == slsDetectorDefs::BURST_TRIGGER)
			success = true;
		break;
	default:
		FILE_LOG(logERROR) << "Timing mode unknown to GUI";
		//This should never happen
		qDefs::Message(qDefs::CRITICAL, "Timing mode unknown to GUI", "qTabMeasurement::SetTimingMode");
		qDefs::checkErrorMessage(myDet, "qTabMeasurement::SetTimingMode");
		exit(-1);
	}
	qDefs::checkErrorMessage(myDet, "qTabMeasurement::SetTimingMode");
	if (!success) {

		if (mode != AUTO) {
			qDefs::Message(qDefs::WARNING, "The detector timing mode could not be set.\n\nSetting timing mode to Auto",
					"qTabMeasurement::SetTimingMode");
			comboTimingMode->setCurrentIndex((int)AUTO);
			return;
		} else {
			// can't do anything. just ignore
			qDefs::Message(qDefs::ERROR, "The detector timing mode could not be set.", "qTabMeasurement::SetTimingMode");
		}
	}

	//Frame Period between exposures
	CheckAcqPeriodGreaterThanExp();

	// to let qdrawplot know that triggers or frames are used
	myPlot->setFrameEnabled(lblNumFrames->isEnabled());
	myPlot->setTriggerEnabled(lblNumTriggers->isEnabled());

	qDefs::checkErrorMessage(myDet, "qTabMeasurement::SetTimingMode");

	emit CheckPlotIntervalSignal();
}


void qTabMeasurement::Refresh() {
	FILE_LOG(logDEBUG) << "**Updating Measurement Tab";

	if (!myPlot->isRunning()) {

		//timing mode - will also check if exptime>acq period
		GetTimingModeFromDetector();

		//to prevent it from recalculating forever
		disconnect(spinNumMeasurements, SIGNAL(valueChanged(int)), this, SLOT(setNumMeasurements(int)));
		disconnect(spinNumFrames, SIGNAL(valueChanged(int)), this, SLOT(setNumFrames(int)));
		disconnect(spinExpTime, SIGNAL(valueChanged(double)), this, SLOT(setExposureTime()));
		disconnect(comboExpUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(setExposureTime()));
		disconnect(spinPeriod, SIGNAL(valueChanged(double)), this, SLOT(setAcquisitionPeriod()));
		disconnect(comboPeriodUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(setAcquisitionPeriod()));
		disconnect(spinNumTriggers, SIGNAL(valueChanged(int)), this, SLOT(setNumTriggers(int)));
		if (spinDelay->isEnabled()) {
			disconnect(spinDelay, SIGNAL(valueChanged(double)), this, SLOT(setDelay()));
			disconnect(comboDelayUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(setDelay()));
		}
		if (spinSamples->isEnabled())
			disconnect(spinNumSamples, SIGNAL(valueChanged(int)), this, SLOT(setNumSamples(int)));
		disconnect(dispFileName, SIGNAL(editingFinished()), this, SLOT(setFileName()));
		disconnect(chkFile, SIGNAL(toggled(bool)), this, SLOT(EnableFileWrite(bool)));
		disconnect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(setRunIndex(int)));
		disconnect(progressTimer, SIGNAL(timeout()), this, SLOT(UpdateProgress()));


		FILE_LOG(logDEBUG) << "Getting number of measurements & frames";
		//measurements
		spinNumMeasurements->setValue((int)myDet->setTimer(slsDetectorDefs::MEASUREMENTS_NUMBER, -1));
		//frames
		spinNumFrames->setValue((int)myDet->setTimer(slsDetectorDefs::FRAME_NUMBER, -1));

		FILE_LOG(logDEBUG) << "Getting Exposure time";
		qDefs::timeUnit unit;
		//Exp Time
		double oldExptimeNs = qDefs::getNSTime((qDefs::timeUnit)comboExpUnit->currentIndex(), spinExpTime->value());
		double time = qDefs::getCorrectTime(unit, ((double)(myDet->setTimer(slsDetectorDefs::ACQUISITION_TIME, -1) * (1E-9))));
		spinExpTime->setValue(time);
		comboExpUnit->setCurrentIndex((int)unit);
		double newExptimeNs = qDefs::getNSTime((qDefs::timeUnit)comboExpUnit->currentIndex(), spinExpTime->value());

		//period
		FILE_LOG(logDEBUG) << "Getting Acquisition Period";
		double oldAcqtimeNS = qDefs::getNSTime((qDefs::timeUnit)comboPeriodUnit->currentIndex(), spinPeriod->value());
		time = qDefs::getCorrectTime(unit, ((double)(myDet->setTimer(slsDetectorDefs::FRAME_PERIOD, -1) * (1E-9))));
		spinPeriod->setValue(time);
		comboPeriodUnit->setCurrentIndex((int)unit);
		double newAcqtimeNS = qDefs::getNSTime((qDefs::timeUnit)comboPeriodUnit->currentIndex(), spinPeriod->value());

		// change in period or exptime
		if (newExptimeNs != oldExptimeNs || newAcqtimeNS != oldAcqtimeNS) {
			//Frame Period between exposures
			CheckAcqPeriodGreaterThanExp();

			emit CheckPlotIntervalSignal();
		}

		FILE_LOG(logDEBUG) << "Getting #triggers and delay";
		//triggers
		spinNumTriggers->setValue((int)myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER, -1));

		//delay
		if (spinDelay->isEnabled())
			time = qDefs::getCorrectTime(unit, ((double)(myDet->setTimer(slsDetectorDefs::DELAY_AFTER_TRIGGER, -1) * (1E-9))));

		// samples
		if (spinSamples->isEnabled()) {
			spinNumSamples->setValue((int)myDet->setTimer(slsDetectorDefs::SAMPLES, -1));
		}

		FILE_LOG(logDEBUG) << "Getting file name prefix, file index, file write enable and progress index";
		//file name
		dispFileName->setText(QString(myDet->getFileName().c_str()));
		//file write enable
		chkFile->setChecked(myDet->enableWriteToFile());
		//file index
		spinIndex->setValue(myDet->getFileIndex());

		//progress label index
		lblProgressIndex->setText("0");

		connect(spinNumMeasurements, SIGNAL(valueChanged(int)), this, SLOT(setNumMeasurements(int)));
		connect(spinNumFrames, SIGNAL(valueChanged(int)), this, SLOT(setNumFrames(int)));
		connect(spinExpTime, SIGNAL(valueChanged(double)), this, SLOT(setExposureTime()));
		connect(comboExpUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(setExposureTime()));
		connect(spinPeriod, SIGNAL(valueChanged(double)), this, SLOT(setAcquisitionPeriod()));
		connect(comboPeriodUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(setAcquisitionPeriod()));
		connect(spinNumTriggers, SIGNAL(valueChanged(int)), this, SLOT(setNumTriggers(int)));
		if (spinDelay->isEnabled()) {
			connect(spinDelay, SIGNAL(valueChanged(double)), this, SLOT(setDelay()));
			connect(comboDelayUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(setDelay()));
		}
		if (spinSamples->isEnabled())
			connect(spinNumSamples, SIGNAL(valueChanged(int)), this, SLOT(setNumSamples(int)));
		disconnect(dispFileName, SIGNAL(editingFinished()), this, SLOT(setFileName()));
		connect(chkFile, SIGNAL(toggled(bool)), this, SLOT(EnableFileWrite(bool)));
		connect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(setRunIndex(int)));
		connect(progressTimer, SIGNAL(timeout()), this, SLOT(UpdateProgress()));

		qDefs::checkErrorMessage(myDet, "qTabMeasurement::Refresh");
	}

	FILE_LOG(logDEBUG) << "**Updated Measurement Tab";
}
