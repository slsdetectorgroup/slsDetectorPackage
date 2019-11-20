#pragma once
#include "container_utils.h"
#include "logger.h"
#include "receiver_defs.h"
#include "network_utils.h"
class GeneralData;
class Listener;
class DataProcessor;
class DataStreamer;
class Fifo;
class slsDetectorDefs;

#include <atomic>
#include <exception>
#include <memory>
#include <vector>

class Implementation : private virtual slsDetectorDefs {
  public:
    //*** cosntructor & destructor ***
    /**
     * Constructor
     */
    Implementation();

    /**
     * Destructor
     */
    virtual ~Implementation();

    /*************************************************************************
     * Getters ***************************************************************
     * They access local cache of configuration or detector parameters *******
     *************************************************************************/

    //**initial parameters***
    int *getMultiDetectorSize() const;
    int getDetectorPositionId() const;
    std::string getDetectorHostname() const;
    int getFlippedDataX() const;
    bool getGapPixelsEnable() const;
    bool getQuad() const;
    int getReadNLines() const;
    readoutMode getReadoutMode() const;

    //***file parameters***
    fileFormat getFileFormat() const;
    std::string getFileName() const;
    std::string getFilePath() const;
    uint64_t getFileIndex() const;
    uint32_t getFramesPerFile() const;
    frameDiscardPolicy getFrameDiscardPolicy() const;
    bool getFramePaddingEnable() const;
    bool getFileWriteEnable() const;
    bool getMasterFileWriteEnable() const;
    bool getOverwriteEnable() const;

    //***acquisition count parameters***
    uint64_t getFramesCaught() const;
    uint64_t getAcquisitionIndex() const;
    std::vector<uint64_t> getNumMissingPackets() const;

    //***connection parameters***
    uint32_t getUDPPortNumber() const;
    uint32_t getUDPPortNumber2() const;
    std::string getEthernetInterface() const;
    std::string getEthernetInterface2() const;
    int getNumberofUDPInterfaces() const;

    //***acquisition parameters***
    ROI getROI() const;
    uint32_t getADCEnableMask() const;
    uint32_t getStreamingFrequency() const;
    uint32_t getStreamingTimer() const;
    bool getDataStreamEnable() const;
    uint64_t getAcquisitionPeriod() const;
    uint64_t getAcquisitionTime() const;
    uint64_t getSubExpTime() const;
    uint64_t getSubPeriod() const;
    uint64_t getNumberOfFrames() const;
    uint32_t getNumberofAnalogSamples() const;
    uint32_t getNumberofDigitalSamples() const;
    uint32_t getDynamicRange() const;
    bool getTenGigaEnable() const;
    uint32_t getFifoDepth() const;

    //***receiver status***
    runStatus getStatus() const;
    bool getSilentMode() const;
    std::vector<int> getDbitList() const;
    int getDbitOffset() const;
    bool getActivate() const;
    bool getDeactivatedPadding() const;
    uint32_t getStreamingPort() const;
    sls::IpAddr getStreamingSourceIP() const;
    std::string getAdditionalJsonHeader() const;
    int64_t getUDPSocketBufferSize() const;
    int64_t getActualUDPSocketBufferSize() const;

    /*************************************************************************
     * Setters ***************************************************************
     * They modify the local cache of configuration or detector parameters ***
     *************************************************************************/

    //**initial parameters***
    void setDetectorHostname(const std::string& c);
    void setMultiDetectorSize(const int *size);
    void setFlippedDataX(int enable = -1);
    /* [Eiger] */
    int setGapPixelsEnable(const bool b);
    /* [Eiger] */
    int setQuad(const bool b);
    /* [Eiger] */
    void setReadNLines(const int value);
    /* [Ctb] */
    int setReadoutMode(const readoutMode f);

    //***file parameters***
    void setFileFormat(slsDetectorDefs::fileFormat f);
    void setFileName(const std::string& c);
    /* check for existence */
    void setFilePath(const std::string& c);
    void setFileIndex(const uint64_t i);
    /* 0 means infinite */
    void setFramesPerFile(const uint32_t i);
    void setFrameDiscardPolicy(const frameDiscardPolicy i);
    void setFramePaddingEnable(const bool i);
    void setFileWriteEnable(const bool b);
    void setMasterFileWriteEnable(const bool b);
    void setOverwriteEnable(const bool b);

    //***connection parameters***
    void setUDPPortNumber(const uint32_t i);
    /* [Eiger][Jungfrau] */
    void setUDPPortNumber2(const uint32_t i);
    void setEthernetInterface(const std::string &c);
    /* [Jungfrau] */
    void setEthernetInterface2(const std::string &c);
    /* [Jungfrau] */
    int setNumberofUDPInterfaces(const int n);
    int setUDPSocketBufferSize(const int64_t s);

    //***acquisition parameters***
    /* [Gotthard] */
    int setROI(ROI arg);
    /* [Ctb][Moench] */
    int setADCEnableMask(const uint32_t mask);
    /* 0 for timer */    
    int setStreamingFrequency(const uint32_t freq);
    void setStreamingTimer(const uint32_t time_in_ms);
    int setDataStreamEnable(const bool enable);
    void setStreamingPort(const uint32_t i);
    void setStreamingSourceIP(const  sls::IpAddr ip);
    void setAdditionalJsonHeader(const std::string& c);
    void setAcquisitionPeriod(const uint64_t i);
    void setAcquisitionTime(const uint64_t i);
    void setSubExpTime(const uint64_t i);
    void setSubPeriod(const uint64_t i);
    void setNumberOfFrames(const uint64_t i);
    int setNumberofAnalogSamples(const uint32_t i);
    int setNumberofDigitalSamples(const uint32_t i);
    int setDynamicRange(const uint32_t i);
    /* [Eiger][Ctb] */    
    int setTenGigaEnable(const bool b);
    int setFifoDepth(const uint32_t i);

    //***receiver parameters***
    /** [Eiger]
     * Activate / Deactivate Receiver
     * If deactivated, receiver will create dummy data if deactivated padding is
     * enabled (as it will receive nothing from detector)
     * @param enable enable
     * @return false for disabled, true for enabled
     */
    bool setActivate(const bool enable);
    /* [Eiger] */
    bool setDeactivatedPadding(const bool enable);
    void setSilentMode(const bool i);
    /* [Ctb] */
    void setDbitList(const std::vector<int> v);
    /* [Ctb] */
    void setDbitOffset(const int s);

    /*************************************************************************
     * Behavioral functions***************************************************
     * They may modify the status of the receiver ****************************
     *************************************************************************/

    //***initial functions***
    /**
     * Set receiver type (and corresponding detector variables in derived
     * STANDARD class) It is the first function called by the client when
     * connecting to receiver
     * @param d detector type
     * @return OK or FAIL
     */
    int setDetectorType(const detectorType d);
    void setDetectorPositionId(const int id);

    //***acquisition functions***
    int startReceiver(std::string& err);
    void setStoppedFlag(bool stopped);
    void stopReceiver();
    void startReadout();
    void shutDownUDPSockets();
    void closeFiles();
    int restreamStop();

    //***callback functions***
    void registerCallBackStartAcquisition(int (*func)(std::string, std::string, uint64_t,
                                                      uint32_t, void *),
                                          void *arg);
    void registerCallBackAcquisitionFinished(void (*func)(uint64_t, void *),
                                             void *arg);
    void registerCallBackRawDataReady(void (*func)(char *, char *, uint32_t,
                                                   void *),
                                      void *arg);
    void registerCallBackRawDataModifyReady(void (*func)(char *, char *,
                                                         uint32_t &, void *),
                                            void *arg);

  private:
    void DeleteMembers();
    void InitializeMembers();
    void SetLocalNetworkParameters();
    void SetThreadPriorities();
    int SetupFifoStructure();
    void ResetParametersforNewAcquisition();
    int CreateUDPSockets();
    int SetupWriter();
    void StartRunning();

    /*************************************************************************
     * Class Members *********************************************************
     *************************************************************************/
    //**detector parameters***
    detectorType myDetectorType;
    int numDet[MAX_DIMENSIONS];
    int detID;
    std::string detHostname;
    uint64_t acquisitionPeriod;
    uint64_t acquisitionTime;
    uint64_t subExpTime;
    uint64_t subPeriod;
    uint64_t numberOfFrames;
    uint64_t numberOfAnalogSamples;
    uint64_t numberOfDigitalSamples;
    uint32_t dynamicRange;
    bool tengigaEnable;
    uint32_t fifoDepth;
    int flippedDataX;
    bool gapPixelsEnable;
    bool quadEnable;   
    int numLinesReadout;
    readoutMode readoutType;

    //*** receiver parameters ***
    int numThreads;
    const static int MAX_NUMBER_OF_LISTENING_THREADS = 2;
    std::atomic<runStatus> status;
    bool activated;
    bool deactivatedPaddingEnable;
    frameDiscardPolicy frameDiscardMode;
    bool framePadding;
    bool silentMode;
    std::vector<int> ctbDbitList;
    int ctbDbitOffset;
    int ctbAnalogDataBytes;

    //***connection parameters***
    int numUDPInterfaces;
    std::vector <std::string> eth;
    uint32_t udpPortNum[MAX_NUMBER_OF_LISTENING_THREADS];
    int64_t udpSocketBufferSize;
    int64_t actualUDPSocketBufferSize;

    //***file parameters***
    fileFormat fileFormatType;
    std::string fileName;
    std::string filePath;
    uint64_t fileIndex;
    uint32_t framesPerFile;
    bool fileWriteEnable;
    bool masterFileWriteEnable;
    bool overwriteEnable;

    //***acquisition parameters***
    ROI roi;
    uint32_t adcEnableMask;
    uint32_t streamingFrequency;
    uint32_t streamingTimerInMs;
    bool dataStreamEnable;
    uint32_t streamingPort;
     sls::IpAddr streamingSrcIP;
    std::string additionalJsonHeader;
    bool stoppedFlag;

    //** class objects ***
    GeneralData *generalData;
    std::vector<std::unique_ptr<Listener>> listener;
    std::vector<std::unique_ptr<DataProcessor>> dataProcessor;
    std::vector<std::unique_ptr<DataStreamer>> dataStreamer;
    std::vector<std::unique_ptr<Fifo>> fifo;

    //***callback parameters***
    int (*startAcquisitionCallBack)(std::string, std::string, uint64_t, uint32_t, void *);
    void *pStartAcquisition;
    void (*acquisitionFinishedCallBack)(uint64_t, void *);
    void *pAcquisitionFinished;
    void (*rawDataReadyCallBack)(char *, char *, uint32_t, void *);
    void (*rawDataModifyReadyCallBack)(char *, char *, uint32_t &, void *);
    void *pRawDataReady;
};
