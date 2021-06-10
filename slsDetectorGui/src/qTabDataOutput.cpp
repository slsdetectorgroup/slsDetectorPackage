#include "qTabDataOutput.h"
#include "qDefs.h"
#include "sls/bit_utils.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QString>
#include <string>

#include <unistd.h>

qTabDataOutput::qTabDataOutput(QWidget *parent, sls::Detector *detector)
    : QWidget(parent), det(detector), btnGroupRate(nullptr) {
    setupUi(this);
    SetupWidgetWindow();
    LOG(logDEBUG) << "DataOutput ready";
}

qTabDataOutput::~qTabDataOutput() { delete btnGroupRate; }

void qTabDataOutput::SetupWidgetWindow() {
    // button group for rate
    btnGroupRate = new QButtonGroup(this);
    btnGroupRate->addButton(radioDefaultDeadtime, 0);
    btnGroupRate->addButton(radioCustomDeadtime, 1);
    chkCounter1->setEnabled(false);
    chkCounter2->setEnabled(false);
    chkCounter3->setEnabled(false);
    chkCounter1->hide();
    chkCounter2->hide();
    chkCounter3->hide();

    // enabling according to det type
    switch (det->getDetectorType().squash()) {
    case slsDetectorDefs::EIGER:
        chkTenGiga->setEnabled(true);
        chkRate->setEnabled(true);
        radioDefaultDeadtime->setEnabled(true);
        radioCustomDeadtime->setEnabled(true);
        // flags and speed
        lblClkDivider->setEnabled(true);
        comboClkDivider->setEnabled(true);
        chkParallel->setEnabled(true);
        break;
    case slsDetectorDefs::MOENCH:
        chkTenGiga->setEnabled(true);
        break;
    case slsDetectorDefs::MYTHEN3:
        chkParallel->setEnabled(true);
        chkCounter1->setEnabled(true);
        chkCounter2->setEnabled(true);
        chkCounter3->setEnabled(true);
        chkCounter1->show();
        chkCounter2->show();
        chkCounter3->show();
        break;
    case slsDetectorDefs::JUNGFRAU:
        lblClkDivider->setEnabled(true);
        comboClkDivider->setEnabled(true);
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
    connect(comboDetector, SIGNAL(currentIndexChanged(int)), this,
            SLOT(GetOutputDir()));
    connect(dispOutputDir, SIGNAL(editingFinished()), this,
            SLOT(SetOutputDir()));
    connect(dispOutputDir, SIGNAL(returnPressed()), this,
            SLOT(ForceSetOutputDir()));
    connect(btnOutputBrowse, SIGNAL(clicked()), this, SLOT(BrowseOutputDir()));
    connect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetFileFormat(int)));
    connect(chkOverwriteEnable, SIGNAL(toggled(bool)), this,
            SLOT(SetOverwriteEnable(bool)));
    if (chkTenGiga->isEnabled()) {
        connect(chkTenGiga, SIGNAL(toggled(bool)), this,
                SLOT(SetTenGigaEnable(bool)));
    }
    // rate
    if (chkRate->isEnabled()) {
        connect(chkRate, SIGNAL(toggled(bool)), this,
                SLOT(EnableRateCorrection()));
        connect(btnGroupRate, SIGNAL(buttonClicked(int)), this,
                SLOT(SetRateCorrection()));
        connect(spinCustomDeadTime, SIGNAL(valueChanged(int)), this,
                SLOT(SetRateCorrection()));
    }
    // parallel
    if (chkParallel->isEnabled()) {
        connect(chkParallel, SIGNAL(toggled(bool)), this,
                SLOT(SetParallel(bool)));
    }
    if (chkCounter1->isEnabled()) {
        connect(chkCounter1, SIGNAL(toggled(bool)), this,
                SLOT(SetCounterMask()));
        connect(chkCounter2, SIGNAL(toggled(bool)), this,
                SLOT(SetCounterMask()));
        connect(chkCounter3, SIGNAL(toggled(bool)), this,
                SLOT(SetCounterMask()));
    }
    // speed
    if (comboClkDivider->isEnabled()) {
        connect(comboClkDivider, SIGNAL(currentIndexChanged(int)), this,
                SLOT(SetSpeed(int)));
    }
}

void qTabDataOutput::PopulateDetectors() {
    LOG(logDEBUG) << "Populating detectors";

    comboDetector->clear();
    comboDetector->addItem("All");
    if (det->size() > 1) {
        auto res = det->getHostname();
        for (auto &it : res) {
            comboDetector->addItem(QString(it.c_str()));
        }
    }
}

void qTabDataOutput::EnableBrowse() {
    LOG(logDEBUG) << "Getting browse enable";
    try {
        btnOutputBrowse->setEnabled(false); // exception default
        std::string rxHostname =
            det->getRxHostname({comboDetector->currentIndex() - 1})
                .squash("none");
        if (rxHostname == "none") {
            btnOutputBrowse->setEnabled(false);
        } else if (rxHostname == "localhost") {
            btnOutputBrowse->setEnabled(true);
        } else {
            std::string hostname;
            const size_t len = 15;
            char host[len]{};
            if (gethostname(host, len) == 0) {
                hostname.assign(host);
            }
            // client pc (hostname) same as reciever hostname
            if (hostname == rxHostname) {
                btnOutputBrowse->setEnabled(true);
            } else {
                btnOutputBrowse->setEnabled(false);
            }
        }
    }
    CATCH_DISPLAY("Could not get receiver hostname.",
                  "qTabDataOutput::EnableBrowse")
}

void qTabDataOutput::GetFileWrite() {
    LOG(logDEBUG) << "Getting file write enable";
    try {
        boxFileWriteEnabled->setEnabled(true); // exception default
        auto retval = det->getFileWrite().tsquash(
            "File write is inconsistent for all detectors.");
        boxFileWriteEnabled->setEnabled(retval);
    }
    CATCH_DISPLAY("Could not get file enable.", "qTabDataOutput::GetFileWrite")
}

void qTabDataOutput::GetFileName() {
    LOG(logDEBUG) << "Getting file name";
    try {
        auto retval = det->getFileNamePrefix().tsquash(
            "File name is inconsistent for all detectors.");
        dispFileName->setText(QString(retval.c_str()));
    }
    CATCH_DISPLAY("Could not get file name prefix.",
                  "qTabDataOutput::GetFileName")
}

void qTabDataOutput::GetOutputDir() {
    LOG(logDEBUG) << "Getting file path";
    disconnect(dispOutputDir, SIGNAL(editingFinished()), this,
               SLOT(SetOutputDir()));
    try {
        std::string path =
            det->getFilePath({comboDetector->currentIndex() - 1})
                .tsquash("File path is different for all detectors.");
        dispOutputDir->setText(QString(path.c_str()));
    }
    CATCH_DISPLAY("Could not get file path.", "qTabDataOutput::GetOutputDir")
    connect(dispOutputDir, SIGNAL(editingFinished()), this,
            SLOT(SetOutputDir()));
}

void qTabDataOutput::BrowseOutputDir() {
    LOG(logDEBUG) << "Browsing output directory";
    QString directory = QFileDialog::getExistingDirectory(
        this, tr("Choose Output Directory "), dispOutputDir->text());
    if (!directory.isEmpty())
        dispOutputDir->setText(directory);
}

void qTabDataOutput::SetOutputDir(bool force) {
    // return forces modification (inconsistency from command line)
    if (dispOutputDir->isModified() || force) {
        dispOutputDir->setModified(false);
        QString path = dispOutputDir->text();
        LOG(logDEBUG) << "Setting output directory to "
                      << path.toAscii().constData();

        // empty
        if (path.isEmpty()) {
            qDefs::Message(qDefs::WARNING,
                           "Invalid Output Path. Must not be empty.",
                           "qTabDataOutput::SetOutputDir");
            LOG(logWARNING) << "Invalid Output Path. Must not be empty.";
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
                det->setFilePath(spath, {comboDetector->currentIndex() - 1});
            }
            CATCH_HANDLE("Could not set output file path.",
                         "qTabDataOutput::SetOutputDir", this,
                         &qTabDataOutput::GetOutputDir)
        }
    }
}

void qTabDataOutput::ForceSetOutputDir() { SetOutputDir(true); };

void qTabDataOutput::GetFileFormat() {
    LOG(logDEBUG) << "Getting File Format";
    disconnect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetFileFormat(int)));
    try {
        auto retval = det->getFileFormat().tsquash(
            "File format is inconsistent for all detectors.");
        switch (retval) {
        case slsDetectorDefs::BINARY:
        case slsDetectorDefs::HDF5:
            comboFileFormat->setCurrentIndex(static_cast<int>(retval));
            break;
        default:
            throw sls::RuntimeError(std::string("Unknown file format: ") +
                                    std::to_string(static_cast<int>(retval)));
        }
    }
    CATCH_DISPLAY("Could not get file format.", "qTabDataOutput::GetFileFormat")
    connect(comboFileFormat, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetFileFormat(int)));
}

void qTabDataOutput::SetFileFormat(int format) {
    LOG(logINFO) << "Setting File Format to "
                 << comboFileFormat->currentText().toAscii().data();
    try {
        det->setFileFormat(static_cast<slsDetectorDefs::fileFormat>(
            comboFileFormat->currentIndex()));
    }
    CATCH_HANDLE("Could not set file format.", "qTabDataOutput::SetFileFormat",
                 this, &qTabDataOutput::GetFileFormat)
}

void qTabDataOutput::GetFileOverwrite() {
    LOG(logDEBUG) << "Getting File Over Write Enable";
    disconnect(chkOverwriteEnable, SIGNAL(toggled(bool)), this,
               SLOT(SetOverwriteEnable(bool)));
    try {
        auto retval = det->getFileOverWrite().tsquash(
            "File over write is inconsistent for all detectors.");
        chkOverwriteEnable->setChecked(retval);
    }
    CATCH_DISPLAY("Could not get file over write enable.",
                  "qTabDataOutput::GetFileOverwrite")

    connect(chkOverwriteEnable, SIGNAL(toggled(bool)), this,
            SLOT(SetOverwriteEnable(bool)));
}

void qTabDataOutput::SetOverwriteEnable(bool enable) {
    LOG(logINFO) << "Setting File Over Write Enable to " << enable;
    try {
        det->setFileOverWrite(enable);
    }
    CATCH_HANDLE("Could not set file over write enable.",
                 "qTabDataOutput::SetOverwriteEnable", this,
                 &qTabDataOutput::GetFileOverwrite)
}

void qTabDataOutput::GetTenGigaEnable() {
    LOG(logDEBUG) << "Getting 10GbE enable";
    disconnect(chkTenGiga, SIGNAL(toggled(bool)), this,
               SLOT(SetTenGigaEnable(bool)));
    try {
        auto retval = det->getTenGiga().tsquash(
            "10GbE enable is inconsistent for all detectors.");
        chkTenGiga->setChecked(retval);
    }
    CATCH_DISPLAY("Could not get 10GbE enable.",
                  "qTabDataOutput::GetTenGigaEnable")
    connect(chkTenGiga, SIGNAL(toggled(bool)), this,
            SLOT(SetTenGigaEnable(bool)));
}

void qTabDataOutput::SetTenGigaEnable(bool enable) {
    LOG(logINFO) << "Setting 10GbE to " << enable;
    try {
        det->setTenGiga(enable);
    }
    CATCH_HANDLE("Could not set 10GbE enable.",
                 "qTabDataOutput::SetTenGigaEnable", this,
                 &qTabDataOutput::GetTenGigaEnable)
}

void qTabDataOutput::GetRateCorrection() {
    LOG(logDEBUG) << "Getting Rate Correction";
    disconnect(chkRate, SIGNAL(toggled(bool)), this,
               SLOT(EnableRateCorrection()));
    disconnect(btnGroupRate, SIGNAL(buttonClicked(int)), this,
               SLOT(SetRateCorrection()));
    disconnect(spinCustomDeadTime, SIGNAL(valueChanged(int)), this,
               SLOT(SetRateCorrection()));
    try {
        spinCustomDeadTime->setValue(-1);
        int64_t retval = det->getRateCorrection()
                             .tsquash("Rate correction (enable/tau) is "
                                      "inconsistent for all detectors.")
                             .count();
        chkRate->setChecked(retval == 0 ? false : true);
        if (retval != 0)
            spinCustomDeadTime->setValue(retval);
    }
    CATCH_DISPLAY("Could not get rate correction.",
                  "qTabDataOutput::GetRateCorrection")
    connect(chkRate, SIGNAL(toggled(bool)), this, SLOT(EnableRateCorrection()));
    connect(btnGroupRate, SIGNAL(buttonClicked(int)), this,
            SLOT(SetRateCorrection()));
    connect(spinCustomDeadTime, SIGNAL(valueChanged(int)), this,
            SLOT(SetRateCorrection()));
}

void qTabDataOutput::EnableRateCorrection() {
    // enable
    if (chkRate->isChecked()) {
        SetRateCorrection();
        return;
    }
    LOG(logINFO) << "Disabling Rate correction";
    // disable
    try {
        det->setRateCorrection(sls::ns(0));
    }
    CATCH_HANDLE("Could not switch off rate correction.",
                 "qTabDataOutput::EnableRateCorrection", this,
                 &qTabDataOutput::GetRateCorrection)
}

void qTabDataOutput::SetRateCorrection() {
    // do nothing if rate correction is disabled
    if (!chkRate->isChecked()) {
        return;
    }
    try {
        // custom dead time
        if (radioCustomDeadtime->isChecked()) {
            int64_t deadtime = spinCustomDeadTime->value();
            LOG(logINFO) << "Setting Rate Correction with custom dead time: "
                         << deadtime;
            det->setRateCorrection(sls::ns(deadtime));
        }
        // default dead time
        else {
            LOG(logINFO) << "Setting Rate Correction with default dead time";
            det->setDefaultRateCorrection();
        }
    }
    CATCH_HANDLE("Could not set rate correction.",
                 "qTabDataOutput::SetRateCorrection", this,
                 &qTabDataOutput::GetRateCorrection)
}

void qTabDataOutput::GetSpeed() {
    LOG(logDEBUG) << "Getting Speed";
    disconnect(comboClkDivider, SIGNAL(currentIndexChanged(int)), this,
               SLOT(SetSpeed(int)));
    try {
        auto retval =
            det->getSpeed().tsquash("Speed is inconsistent for all detectors.");
        comboClkDivider->setCurrentIndex(static_cast<int>(retval));
    }
    CATCH_DISPLAY("Could not get speed.", "qTabDataOutput::GetSpeed")
    connect(comboClkDivider, SIGNAL(currentIndexChanged(int)), this,
            SLOT(SetSpeed(int)));
}

void qTabDataOutput::SetSpeed(int speed) {
    LOG(logINFO) << "Setting Speed to "
                 << comboClkDivider->currentText().toAscii().data();
    try {
        det->setSpeed(static_cast<slsDetectorDefs::speedLevel>(speed));
    }
    CATCH_HANDLE("Could not set speed.", "qTabDataOutput::SetSpeed", this,
                 &qTabDataOutput::GetSpeed)
}

void qTabDataOutput::GetParallel() {
    LOG(logDEBUG) << "Getting parallel readout";
    disconnect(chkParallel, SIGNAL(toggled(bool)), this,
               SLOT(SetParallel(bool)));
    try {
        auto retval = det->getParallelMode().tsquash(
            "Parallel Flag is inconsistent for all detectors.");
        chkParallel->setChecked(retval);
    }
    CATCH_DISPLAY("Could not get parallel readout.",
                  "qTabDataOutput::GetParallel")
    connect(chkParallel, SIGNAL(toggled(bool)), this, SLOT(SetParallel(bool)));
}

void qTabDataOutput::SetParallel(bool enable) {
    LOG(logINFO) << "Setting PArallel readout to " << enable;
    try {
        det->setParallelMode(enable);
    }
    CATCH_HANDLE("Could not set parallel readout.",
                 "qTabDataOutput::SetParallel", this,
                 &qTabDataOutput::GetParallel)
}

void qTabDataOutput::GetCounterMask() {
    LOG(logDEBUG) << "Getting counter mask";
    disconnect(chkCounter1, SIGNAL(toggled(bool)), this,
               SLOT(SetCounterMask()));
    disconnect(chkCounter2, SIGNAL(toggled(bool)), this,
               SLOT(SetCounterMask()));
    disconnect(chkCounter3, SIGNAL(toggled(bool)), this,
               SLOT(SetCounterMask()));
    try {
        auto retval = sls::getSetBits(det->getCounterMask().tsquash(
            "Counter mask is inconsistent for all detectors."));
        std::vector<QCheckBox *> counters = {chkCounter1, chkCounter2,
                                             chkCounter3};
        // default to unchecked
        for (unsigned int i = 0; i < counters.size(); ++i) {
            counters[i]->setChecked(false);
        }
        // if retva[i] = 2, chkCounter2 is checked
        for (unsigned int i = 0; i < retval.size(); ++i) {
            if (retval[i] > 3) {
                throw sls::RuntimeError(
                    std::string("Unknown counter index : ") +
                    std::to_string(static_cast<int>(retval[i])));
            }
            counters[retval[i]]->setChecked(true);
        }
    }
    CATCH_DISPLAY("Could not get parallel readout.",
                  "qTabDataOutput::GetParallel")
    connect(chkCounter1, SIGNAL(toggled(bool)), this, SLOT(SetCounterMask()));
    connect(chkCounter2, SIGNAL(toggled(bool)), this, SLOT(SetCounterMask()));
    connect(chkCounter3, SIGNAL(toggled(bool)), this, SLOT(SetCounterMask()));
}

void qTabDataOutput::SetCounterMask() {
    std::vector<QCheckBox *> counters = {chkCounter1, chkCounter2, chkCounter3};
    uint32_t mask = 0;
    for (unsigned int i = 0; i < counters.size(); ++i) {
        if (counters[i]->isChecked()) {
            mask |= (1 << i);
        }
    }
    LOG(logINFO) << "Setting counter mask to " << mask;
    try {
        det->setCounterMask(mask);
    }
    CATCH_HANDLE("Could not set counter mask.",
                 "qTabDataOutput::SetCounterMask", this,
                 &qTabDataOutput::GetCounterMask)
}

void qTabDataOutput::Refresh() {
    LOG(logDEBUG) << "**Updating DataOutput Tab";

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
    if (chkParallel->isEnabled()) {
        GetParallel();
    }
    if (chkCounter1->isEnabled()) {
        GetCounterMask();
    }
    if (comboClkDivider->isEnabled()) {
        GetSpeed();
    }

    LOG(logDEBUG) << "**Updated DataOutput Tab";
}
