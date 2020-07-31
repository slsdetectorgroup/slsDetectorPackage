#pragma once

#include "sls_detector_defs.h"
#include "logger.h"
#include "ToString.h"

// versions
#define HDF5_WRITER_VERSION   (6.1) // 1 decimal places
#define BINARY_WRITER_VERSION (6.1) // 1 decimal places

class masterFileAttributes {

    public:
    double version{0.0};
    slsDetectorDefs::detectorType detType{slsDetectorDefs::GENERIC};
    uint32_t imageSize{0};
    slsDetectorDefs::xy nPixels{};
    uint32_t maxFramesPerFile{0};
    uint64_t totalFrames{0};
    uint64_t exptimeNs{0};
    uint64_t periodNs{0};
    uint32_t dynamicRange{0};
    uint32_t tenGiga{0};
    uint64_t subExptimeNs{0};
    uint64_t subPeriodNs{0};
    uint32_t quadEnable{0};    
    uint32_t adcmask{0};
    uint32_t analogFlag{0};
    uint32_t digitalFlag{0};
    uint32_t dbitoffset{0};
    uint64_t dbitlist{0};
    slsDetectorDefs::ROI roi{};
    uint64_t exptime1Ns{0};
    uint64_t exptime2Ns{0};
    uint64_t exptime3Ns{0};
    uint64_t gateDelay1Ns{0};
    uint64_t gateDelay2Ns{0};
    uint64_t gateDelay3Ns{0};
    uint32_t gates;        
  /*
    double version{0.0};
    slsDetectorDefs::detectorType myDetectorType{slsDetectorDefs::GENERIC};
    uint32_t imageSize{0};
    uint32_t nPixelsX{0};
    uint32_t nPixelsY{0};
    uint32_t maxFramesPerFile{0};
    uint64_t totalFrames{0};
    uint64_t exptimeNs{0};
    uint64_t periodNs{0};
*/
    /* eiger 
    uint32_t dynamicRange{0};
    uint32_t tenGiga{0};
    uint64_t subExptimeNs{0};
    uint64_t subPeriodNs{0};
    uint32_t quadEnable{0};
    */
    /** moench
    uint32_t tenGiga{0};
    uint32_t adcmask{0};
    */
    /* ctb
    uint32_t tenGiga{0};
    uint32_t adcmask{0};
    uint32_t analogFlag{0};
    uint32_t digitalFlag{0};
    uint32_t dbitoffset{0};
    uint64_t dbitlist{0};
    */
    /* gotthard 
    uint32_t roiXmin{0};
    uint32_t roiXmax{0};
    */
    /* mythen3
    uint32_t dynamicRange{0};
    uint32_t tenGiga{0};
    uint64_t exptime1Ns{0};
    uint64_t exptime2Ns{0};
    uint64_t exptime3Ns{0};
    uint64_t gateDelay1Ns{0};
    uint64_t gateDelay2Ns{0};
    uint64_t gateDelay3Ns{0};
    uint32_t gates;
*/
    masterFileAttributes(){};
    virtual ~masterFileAttributes(){};

    std::string GetBinaryMasterFileAttributes() {
        std::ostringstream oss;
        oss << "Version                    : " << std::setprecision(2) << version << '\n'
            << "Detector Type              : " << sls::ToString(detType) << '\n'
            << "Image Size                 : " << imageSize << " bytes" << '\n'
            << "nPixels                    : " << sls::ToString(nPixels) << " pixels" << '\n'
            << "Max Frames Per File        : " << maxFramesPerFile << '\n'
            << "Total Frames               : " << totalFrames << '\n'
            << "Exptime (ns)               : " << exptimeNs << '\n'
            << "Period (ns)                : " << periodNs << '\n';
        return oss.str();
    };

    // hdf5
};

class GotthardMasterFileAttributes : public masterFileAttributes {

  public:
    GotthardMasterFileAttributes() {};

    std::string GetBinaryMasterFileAttributes() {
        std::ostringstream oss;
        oss << masterFileAttributes::GetBinaryMasterFileAttributes()
            << "Roi (xmin, xmax)           : " << sls::ToString(roi) << '\n';
        return oss.str();
    };
};

