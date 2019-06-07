#include "qTabMeasurement.h"
#include "qDefs.h"
#include "qDrawPlot.h"

#include <QStandardItemModel>

#include <iostream>

qTabMeasurement::qTabMeasurement(QWidget *parent, multiSlsDetector *detector, qDrawPlot *plot) : QWidget(parent), myDet(detector), myPlot(plot) {
	setupUi(this);
	SetupWidgetWindow();
	FILE_LOG(logDEBUG) << "Measurement ready";
}

qTabMeasurement::~qTabMeasurement() {}

bool qTabMeasurement::GetStartStatus(){
	return (!btnStart->isEnabled());
}

void qTabMeasurement::ClentStartAcquisition(){
	StartAcquisition();
	myPlot->SetClientInitiated();
}

int qTabMeasurement::GetProgress(){
	return progressBar->value();
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
	if (spinSamples->isEnabled()) {
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
	connect(chkFile, SIGNAL(toggled(bool)), this, SLOT(SetFileWriteEnable(bool)));
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

	// to let qdrawplot know that triggers or frames are used
	myPlot->setFrameEnabled(lblNumFrames->isEnabled());
	myPlot->setTriggerEnabled(lblNumTriggers->isEnabled());

	CheckAcqPeriodGreaterThanExp();
}

void qTabMeasurement::GetTimingMode(bool startup) {
	FILE_LOG(logDEBUG) << "Getting timing mode";
	connect(comboTimingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(SetTimingMode(int)));

	try {
		auto oldMode = comboTimingMode->currentIndex();
        auto retval = myDet->setExternalCommunicationMode();
		switch(retval) {
			case GET_EXTERNAL_COMMUNICATION_MODE:
				qDefs::Message(qDefs::WARNING, "Timing Mode is inconsistent for all detectors.", "qTabMeasurement::GetTimingMode");
				break;
			case AUTO:
			case TRIGGER:
			case GATED:
			case BURST_TRIGGER:
				comboTimingMode->setCurrentIndex((int)retval);
				// update widget enable only if different 
				if (oldMode != comboTimingMode->currentIndex()) {
					EnableWidgetsforTimingMode();
				}
				break;
			default:
				qDefs::ExceptionMessage("Could not get timing mode.", e.what(), "qTabMeasurement::GetTimingMode");
				break;
		}
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get timing mode.", e.what(), "qTabMeasurement::GetTimingMode");
    }

	disconnect(comboTimingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(SetTimingMode(int)));
}

void qTabMeasurement::SetTimingMode(int mode) {
	FILE_LOG(logINFO) << "Setting timing mode:" << comboTimingMode->currentText().toAscii().data();
	
	try {
        myDet->setExternalCommunicationMode(static_cast<slsDetectorDefs::externalCommunicationMode>(value));
		EnableWidgetsforTimingMode();
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set timing mode.", e.what(), "qTabMeasurement::SetTimingMode");
		GetTimingMode();
	}
}

void qTabMeasurement::GetNumMeasurements() {
	FILE_LOG(logDEBUG) << "Getting number of measurements";
	disconnect(spinNumMeasurements, SIGNAL(valueChanged(int)), this, SLOT(SetNumMeasurements(int)));

	try {
        auto retval = myDet->setTimer(slsDetectorDefs::MEASUREMENTS_NUMBER);
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Number of measurements is inconsistent for all detectors.", "qTabMeasurement::GetNumMeasurements");
		} 
		spinNumMeasurements->seValue(retval);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get number of measurements.", e.what(), "qTabMeasurement::GetNumMeasurements");
    }

	connect(spinNumMeasurements, SIGNAL(valueChanged(int)), this, SLOT(SetNumMeasurements(int)));
}

void qTabMeasurement::SetNumMeasurements(int val) {
	FILE_LOG(logINFO) << "Setting Number of Measurements to " << val;

	try {
        myDet->setTimer(slsDetectorDefs::MEASUREMENTS_NUMBER, val);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set number of measurements.", e.what(), "qTabMeasurement::SetNumMeasurements");
        GetNumMeasurements();
    }
}

void qTabMeasurement::GetNumFrames() {
	FILE_LOG(logDEBUG) << "Getting number of frames";
	disconnect(spinNumFrames, SIGNAL(valueChanged(int)), this, SLOT(SetNumFrames(int)));

	try {
        auto retval = myDet->setTimer(slsDetectorDefs::FRAME_NUMBER);
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Number of frames is inconsistent for all detectors.", "qTabMeasurement::GetNumFrames");
		} 
		spinNumFrames->seValue(retval);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get number of frames.", e.what(), "qTabMeasurement::GetNumFrames");
    }

	connect(spinNumFrames, SIGNAL(valueChanged(int)), this, SLOT(SetNumFrames(int)));
}

void qTabMeasurement::SetNumFrames(int val) {
	FILE_LOG(logINFO) << "Setting number of frames to " << val;

	try {
        myDet->setTimer(slsDetectorDefs::FRAME_NUMBER, val);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set number of frames.", e.what(), "qTabMeasurement::SetNumFrames");
        GetNumFrames();
    }
}

void qTabMeasurement::GetNumTriggers() {
	FILE_LOG(logDEBUG) << "Getting number of triggers";
	disconnect(spinNumTriggers, SIGNAL(valueChanged(int)), this, SLOT(SetNumTriggers(int)));

	try {
        auto retval = myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER);
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Number of triggers is inconsistent for all detectors.", "qTabMeasurement::GetNumTriggers");
		} 
		spinNumTriggers->seValue(retval);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get number of frames.", e.what(), "qTabMeasurement::GetNumTriggers");
    }

	connect(spinNumTriggers, SIGNAL(valueChanged(int)), this, SLOT(SetNumTriggers(int)));
}

void qTabMeasurement::SetNumTriggers(int val) {
	FILE_LOG(logINFO) << "Setting number of triggers to " << val;

	try {
        myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER, val);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set number of triggers.", e.what(), "qTabMeasurement::SetNumTriggers");
        GetNumTriggers();
    }
}

void qTabMeasurement::GetNumSamples() {
	FILE_LOG(logDEBUG) << "Getting number of samples";
	disconnect(spinNumSamples, SIGNAL(valueChanged(int)), this, SLOT(SetNumSamples(int)));

	try {
        auto retval = myDet->setTimer(slsDetectorDefs::SAMPLES);
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Number of samples is inconsistent for all detectors.", "qTabMeasurement::GetNumSamples");
		} 
		spinNumSamples->seValue(retval);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get number of samples.", e.what(), "qTabMeasurement::GetNumSamples");
    }

	connect(spinNumSamples, SIGNAL(valueChanged(int)), this, SLOT(SetNumSamples(int)));
}

void qTabMeasurement::SetNumSamples(int val) {
	FILE_LOG(logINFO) << "Setting number of samples to " << val;

	try {
        myDet->setTimer(slsDetectorDefs::SAMPLES, val);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set number of samples.", e.what(), "qTabMeasurement::SetNumSamples");
        GetNumSamples();
    }
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
			qDefs::timeUnit unit;
			auto time = qDefs::getCorrectTime(unit, (static_cast<double>(retval) * (1E-9));
			spinExpTime->setValue(time);
			comboExpUnit->setCurrentIndex(static_cast<int>(unit));

			CheckAcqPeriodGreaterThanExp();
		}
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get exposure time.", e.what(), "qTabMeasurement::GetExposureTime");
    }

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
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set exposure time.", e.what(), "qTabMeasurement::SetExposureTime");
        GetExposureTime();
    }
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
			qDefs::timeUnit unit;
			auto time = qDefs::getCorrectTime(unit, (static_cast<double>(retval) * (1E-9));
			spinPeriod->setValue(time);
			comboPeriodUnit->setCurrentIndex(static_cast<int>(unit));

			CheckAcqPeriodGreaterThanExp();
		}
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get acquisition period.", e.what(), "qTabMeasurement::GetAcquisitionPeriod");
    }

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
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set acquisition period.", e.what(), "qTabMeasurement::SetAcquisitionPeriod");
        GetAcquisitionPeriod();
    }
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

	emit CheckPlotIntervalSignal();
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
			qDefs::timeUnit unit;
			auto time = qDefs::getCorrectTime(unit, (static_cast<double>(retval) * (1E-9));
			spinDelay->setValue(time);
			comboDelayUnit->setCurrentIndex(static_cast<int>(unit));

			CheckAcqPeriodGreaterThanExp();
		}
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get delay.", e.what(), "qTabMeasurement::GetDelay");
    }

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
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set delay.", e.what(), "qTabMeasurement::SetDelay");
        GetDelay();
    }
}

void qTabMeasurement::GetFileWriteEnable() {
	FILE_LOG(logDEBUG) << "Getting File Write Enable";
	disconnect(chkFile, SIGNAL(toggled(bool)), this, SLOT(SetFileWriteEnable(bool)));

	try {
        int retval = myDet->getFileWrite();
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "File write is inconsistent for all detectors.", "qTabMeasurement::GetFileWriteEnable");
			dispFileName->setEnabled(true);
		} else {
			chkFile->setChecked(retval == 0 ? false : true);
			dispFileName->setEnabled(chkFile->isChecked());
		}
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get file over write enable.", e.what(), "qTabMeasurement::GetFileWriteEnable");
		dispFileName->setEnabled(true);
    }

	connect(chkFile, SIGNAL(toggled(bool)), this, SLOT(SetFileWriteEnable(bool)));
}

void qTabMeasurement::SetFileWriteEnable(bool enable) {
	FILE_LOG(logINFO) << "Set File Write to " << enable;

	try {
        myDet->setFileWrite(enable);
		// for file save enable
		myPlot->SetFileWriteEnable(enable);
		dispFileName->setEnabled(chkFile->isChecked());
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set file write enable.", e.what(), "qTabMeasurement::SetFileWriteEnable");
        GetFileWriteEnable();
    }
}

void qTabMeasurement::GetFileName() {
	FILE_LOG(logDEBUG) << "Getting file name prefix";
	disconnect(dispFileName, SIGNAL(editingFinished()), this, SLOT(SetFileName()));

	try {
        auto retval = myDet->getFileName();
		dispFileName->setText(QString(retval.c_str()));
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get file name prefix.", e.what(), "qTabMeasurement::GetFileName");
	}	
	
	connect(dispFileName, SIGNAL(editingFinished()), this, SLOT(SetFileName()));
}

void qTabMeasurement::SetFileName() {
	auto val = dispFileName->text().toAscii().constData();
	FILE_LOG(logINFO) << "Setting File Name Prefix:" << val;
	try {
        myDet->setFileName(val);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set file name prefix.", e.what(), "qTabMeasurement::SetFileName");
        GetFileName();
    }
}

void qTabMeasurement::GetRunIndex() {
	FILE_LOG(logDEBUG) << "Getting Acquisition File index";
	disconnect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(SetRunIndex(int)));

	try {
        auto retval = myDet->getFileIndex();
		if (retval == -1) {
			qDefs::Message(qDefs::WARNING, "Acquisition File Index is inconsistent for all detectors.", "qTabMeasurement::GetRunIndex");
		} 
		spinIndex->seValue(retval);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not get acquisition file index.", e.what(), "qTabMeasurement::GetRunIndex");
    }

	connect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(SetRunIndex(int)));
}

void qTabMeasurement::SetRunIndex(int val) {
	FILE_LOG(logINFO) << "Setting Acquisition File Index to " << val;

	try {
        myDet->setFileIndex(val);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set acquisition file index.", e.what(), "qTabMeasurement::SetRunIndex");
        GetRunIndex();
    }
	myDet->setFileIndex(index);
}

void qTabMeasurement::SetCurrentMeasurement(int val) {
	FILE_LOG(logDEBUG) << "Setting Current Measurement";	
	if ((val) < spinNumMeasurements->value()) {
		lblCurrentMeasurement->setText(QString::number(val));
	}
}

void ResetProgress() {
	FILE_LOG(logDEBUG) << "Resetting progress";
	lblCurrentFrame->setText(QString::number(0));
	lblCurrentMeasurement->setText(QString::number(0));
	progressBar->setValue(0);
}

void qTabMeasurement::UpdateProgress() {
	FILE_LOG(logDEBUG) << "Updating progress";
	progressBar->setValue(myPlot->GetProgress());
	lblCurrentFrame->setText(QString::number(myPlot->GetFrameIndex()));
}

void qTabMeasurement::UpdateFinished() {
	UpdateProgress();
	disconnect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(SetRunIndex(int)));
	spinIndex->setValue(myDet->getFileIndex());
	connect(spinIndex, SIGNAL(valueChanged(int)), this, SLOT(SetRunIndex(int)));
	progressTimer->stop();

	Enable(1);
	btnStart->setEnabled(true);
	qDefs::checkErrorMessage(myDet, "qTabMeasurement::UpdateFinished");
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
			//done because for receiver it cant save a file with blank file path and returns without acquiring even to the gui
			disconnect(chkFile, SIGNAL(toggled(bool)), this, SLOT(SetFileWriteEnable(bool)));
			SetFileWriteEnable(false);
			connect(chkFile, SIGNAL(toggled(bool)), this, SLOT(SetFileWriteEnable(bool)));
		}
	}

	FILE_LOG(logINFOBLUE) << "Starting Acquisition";
	lblProgressIndex->setText(QString::number(0));
	Enable(0);
	progressBar->setValue(0);
	progressTimer->start(100);

	emit StartSignal();
	qDefs::checkErrorMessage(myDet, "qTabMeasurement::StartAcquisition");
}


void qTabMeasurement::StopAcquisition() {
	FILE_LOG(logINFORED) << "Stopping Acquisition";
	myDet->stopAcquisition();
	qDefs::checkErrorMessage(myDet, "qTabMeasurement::StopAcquisition");
}


void qTabMeasurement::Enable(bool enable) {
	frameTimeResolved->setEnabled(enable);
	frameNotTimeResolved->setEnabled(enable);

	//shortcut each time, else it doesnt work a second time
	btnStart->setShortcut(QApplication::translate("TabMeasurementObject", "Shift+Space", 0, QApplication::UnicodeUTF8));
}

int qTabMeasurement::VerifyOutputDirectoryError() {
	try {
		auto retval = myDet->getFilePath();
		// if path has no  +, set multi path, else set all individually
	//        myDet->setFileIndex(val);
    } catch (const sls::NonCriticalError &e) {
        qDefs::ExceptionMessage("Could not set path.", e.what(), "qTabMeasurement::VerifyOutputDirectoryError");
		return slsDetectorDefs::FAIL;
    }
	return slsDetectorDefs::OK;
}

void qTabMeasurement::Refresh() {
	FILE_LOG(logDEBUG) << "**Updating Measurement Tab";

	if (!myPlot->isRunning()) {

		//timing mode - will also check if exptime>acq period
		GetTimingMode();
		GetNumMeasurements();
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
		GetFileWriteEnable();
		GetFileName();
		GetRunIndex();
		ResetProgress();

	FILE_LOG(logDEBUG) << "**Updated Measurement Tab";
}
