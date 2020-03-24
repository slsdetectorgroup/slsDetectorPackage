#pragma once
/************************************************
 * @file GeneralData.h
 * @short abstract for setting/getting properties of detector data
 ***********************************************/
/**
 *@short abstract for setting/getting properties of detector data
 */

#include "sls_detector_defs.h"
#include "receiver_defs.h"
#include "logger.h"
#include <cmath>			//ceil
#include <vector>
#include "ToString.h"


class GeneralData {
	
public:

	/** DetectorType */
	slsDetectorDefs::detectorType myDetectorType;

	/** Number of Pixels in x axis */
	uint32_t nPixelsX;

	/** Number of Pixels in y axis */
	uint32_t nPixelsY;

	/** Size of header in Packet */
	uint32_t headerSizeinPacket;

	/** Size of just data in 1 packet (in bytes) */
	uint32_t dataSize;

	/** Size of 1 packet (in bytes) */
	uint32_t packetSize;

	/** Number of packets in an image (for each listening UDP port) */
	uint32_t packetsPerFrame;

	/** Image size (in bytes, for each listening UDP port) */
	uint32_t imageSize;

	/** Frame Number Mask */
	uint64_t frameIndexMask;

	/** Frame Index Offset */
	uint32_t frameIndexOffset;

	/** Packet Index Mask */
	uint32_t packetIndexMask;

	/** Packet Index Offset */
	uint32_t packetIndexOffset;

	/** Max Frames per binary file */
	uint32_t maxFramesPerFile;

	/** Header size of data saved into fifo buffer at a time*/
	uint32_t fifoBufferHeaderSize;

	/** Default Fifo depth */
	uint32_t defaultFifoDepth;

	/** Threads per receiver */
	uint32_t threadsPerReceiver;

	/** Size of a header packet */
	uint32_t headerPacketSize;

	/** Streaming (for ROI - mainly short Gotthard) - Number of Pixels in x axis */
	uint32_t nPixelsXComplete;

	/** Streaming (for ROI - mainly short Gotthard) - Number of Pixels in y axis */
	uint32_t nPixelsYComplete;

	/** Streaming (for ROI - mainly short Gotthard) - Image size (in bytes) */
	uint32_t imageSizeComplete;

	/** if standard header implemented in firmware */
	bool standardheader;

	/** default udp socket buffer size */
	uint32_t defaultUdpSocketBufferSize;




	/** Cosntructor */
	GeneralData():
		myDetectorType(slsDetectorDefs::GENERIC),
		nPixelsX(0),
		nPixelsY(0),
		headerSizeinPacket(0),
		dataSize(0),
		packetSize(0),
		packetsPerFrame(0),
		imageSize(0),
		frameIndexMask(0),
		frameIndexOffset(0),
		packetIndexMask(0),
		packetIndexOffset(0),
		maxFramesPerFile(0),
		fifoBufferHeaderSize(0),
		defaultFifoDepth(0),
		threadsPerReceiver(1),
		headerPacketSize(0),
		nPixelsXComplete(0),
		nPixelsYComplete(0),
		imageSizeComplete(0),
		standardheader(false),
		defaultUdpSocketBufferSize(RECEIVE_SOCKET_BUFFER_SIZE)
		{};

	/** Destructor */
	virtual ~GeneralData(){};

	/**
	 * Get Header Infomation (frame number, packet number)
	 * @param index thread index for debugging purposes
	 * @param packetData pointer to data
	 * @param oddStartingPacket odd starting packet (gotthard)
	 * @param frameNumber frame number
	 * @param packetNumber packet number
	 */
	virtual void GetHeaderInfo(int index, char* packetData, bool oddStartingPacket,
			uint64_t& frameNumber, uint32_t& packetNumber) const
	{
		frameNumber = ((uint32_t)(*((uint32_t*)(packetData))));
		frameNumber++;
		packetNumber = frameNumber&packetIndexMask;
		frameNumber = (frameNumber & frameIndexMask) >> frameIndexOffset;
	}

	/**
	 * Set ROI
	 * @param i ROI
	 */
	virtual void SetROI(slsDetectorDefs::ROI i) {
		LOG(logERROR) << "SetROI is a generic function that should be overloaded by a derived class";
	};

	/**
	 * Get Adc configured
	 * @param index thread index for debugging purposes
	 * @param ROI 
	 * @returns adc configured
	 */
	virtual int GetAdcConfigured(int index, slsDetectorDefs::ROI i)  const{
		LOG(logERROR) << "GetAdcConfigured is a generic function that should be overloaded by a derived class";
		return 0;
	};

	/**
	 * Setting dynamic range changes member variables
	 * @param dr dynamic range
	 * @param tgEnable true if 10GbE is enabled, else false
	 */
	virtual void SetDynamicRange(int dr, bool tgEnable) {
		LOG(logERROR) << "SetDynamicRange is a generic function that should be overloaded by a derived class";
	};

	/**
	 * Setting ten giga enable changes member variables
	 * @param tgEnable true if 10GbE is enabled, else false
	 * @param dr dynamic range
	 */
	virtual void SetTenGigaEnable(bool tgEnable, int dr) {
		LOG(logERROR) << "SetTenGigaEnable is a generic function that should be overloaded by a derived class";
	};

    /**
     * Set odd starting packet (gotthard)
     * @param index thread index for debugging purposes
     * @param packetData pointer to data
     * @returns true or false for odd starting packet number
     */
    virtual bool SetOddStartingPacket(int index, char* packetData) {
        LOG(logERROR) << "SetOddStartingPacket is a generic function that should be overloaded by a derived class";
        return false;
    };

    /**
     * Set databytes (ctb, moench)
     * @param a adc enable mask
	 * @param as analog number of samples
     * @param ds digital number of samples
     * @param t tengiga enable
	 * @param f readout flags
	 * @returns analog data bytes
     */
    virtual int setImageSize(uint32_t a, uint32_t as, uint32_t ds, bool t, slsDetectorDefs::readoutMode) {
        LOG(logERROR) << "setImageSize is a generic function that should be overloaded by a derived class";
		return 0;
    };

    /**
     * set number of interfaces (jungfrau)
     * @param n number of interfaces
     */
    virtual void SetNumberofInterfaces(const int n) {
        LOG(logERROR) << "SetNumberofInterfaces is a generic function that should be overloaded by a derived class";
    }

    /**
     * set number of counters (mythen3)
     * @param n number of counters
	 * @param dr dynamic range
     */
    virtual void SetNumberofCounters(const int n, const int dr) {
        LOG(logERROR) << "SetNumberofCounters is a generic function that should be overloaded by a derived class";
    }	

	/**
	 * Print all variables
	 */
	virtual void Print(TLogLevel level = logDEBUG1) const {
		LOG(level) << "\n\nDetector Data Variables:";
		LOG(level) << "myDetectorType: " << sls::ToString(myDetectorType);
		LOG(level) << "Pixels X: " << nPixelsX;
		LOG(level) << "Pixels Y: " << nPixelsY;
		LOG(level) << "Header Size in Packet: " << headerSizeinPacket;
		LOG(level) << "Data Size: " << dataSize;
		LOG(level) << "Packet Size: " << packetSize;
		LOG(level) << "Packets per Frame: " << packetsPerFrame;
		LOG(level) << "Image Size: " << imageSize;
		LOG(level) << "Frame Index Mask: " << frameIndexMask;
		LOG(level) << "Frame Index Offset: " << frameIndexOffset;
		LOG(level) << "Packet Index Mask: " << packetIndexMask;
		LOG(level) << "Packet Index Offset: " << packetIndexOffset;
		LOG(level) << "Max Frames Per File: " << maxFramesPerFile;
		LOG(level) << "Fifo Buffer Header Size: " << fifoBufferHeaderSize;
		LOG(level) << "Default Fifo Depth: " << defaultFifoDepth;
		LOG(level) << "Threads Per Receiver: " << threadsPerReceiver;
		LOG(level) << "Header Packet Size: " << headerPacketSize;
		LOG(level) << "Complete Pixels X: " << nPixelsXComplete;
		LOG(level) << "Complete Pixels Y: " << nPixelsYComplete;
		LOG(level) << "Complete Image Size: " << imageSizeComplete;
		LOG(level) << "Standard Header: " << standardheader;
		LOG(level) << "UDP Socket Buffer Size: " << defaultUdpSocketBufferSize;
	};
};


class GotthardData : public GeneralData {

private:
	const int nChip = 10;
	const int nChan = 128;
	const int nChipsPerAdc = 2;
 public:

	/** Constructor */
	GotthardData(){
		myDetectorType		= slsDetectorDefs::GOTTHARD;
		nPixelsX 			= 1280;
		nPixelsY 			= 1;
		headerSizeinPacket  = 4;
		dataSize 			= 1280;
		packetSize 			= GOTTHARD_PACKET_SIZE;
		packetsPerFrame 	= 2;
		imageSize 			= dataSize*packetsPerFrame;
		frameIndexMask 		= 0xFFFFFFFE;
		frameIndexOffset 	= 1;
		packetIndexMask 	= 1;
		maxFramesPerFile 	= MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
		defaultFifoDepth 	= 50000;
	};


	/**
	 * Get Header Infomation (frame number, packet number)
	 * @param index thread index for debugging purposes
	 * @param packetData pointer to data
	 * @param oddStartingPacket odd starting packet (gotthard)
	 * @param frameNumber frame number
	 * @param packetNumber packet number
	 */
	void GetHeaderInfo(int index, char* packetData, bool oddStartingPacket,
			uint64_t& frameNumber, uint32_t& packetNumber) const
	{
		if (nPixelsX == 1280) {
			frameNumber = *reinterpret_cast<uint32_t*>(packetData);
			if (oddStartingPacket)
			    frameNumber++;
			packetNumber = frameNumber&packetIndexMask;
			frameNumber = (frameNumber & frameIndexMask) >> frameIndexOffset;
		} else  {
            frameNumber = *reinterpret_cast<uint32_t *>(packetData);
            packetNumber = 0;
		}
	}


	/**
	 * Set ROI
	 * @param i ROI
	 */
	void SetROI(slsDetectorDefs::ROI i) {
		// all adcs
		if(i.xmin == -1) {
			nPixelsX 			= 1280;
			dataSize 			= 1280;
			packetSize 			= GOTTHARD_PACKET_SIZE;
			packetsPerFrame 	= 2;
			imageSize 			= dataSize*packetsPerFrame;
			frameIndexMask 		= 0xFFFFFFFE;
			frameIndexOffset 	= 1;
			packetIndexMask 	= 1;
			maxFramesPerFile 	= MAX_FRAMES_PER_FILE;
			fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
			defaultFifoDepth 	= 50000;
			nPixelsXComplete 	= 0;
			nPixelsYComplete 	= 0;
			imageSizeComplete 	= 0;
		}

		// single adc
		else  {
			nPixelsX 			= 256;
			dataSize 			= 512;
			packetSize 			= 518;
			packetsPerFrame 	= 1;
			imageSize 			= dataSize*packetsPerFrame;
			frameIndexMask 		= 0xFFFFFFFF;
			frameIndexOffset 	= 0;
			packetIndexMask 	= 0;
			maxFramesPerFile 	= SHORT_MAX_FRAMES_PER_FILE;
			fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
			defaultFifoDepth 	= 75000;
			nPixelsXComplete 	= 1280;
			nPixelsYComplete 	= 1;
			imageSizeComplete 	= 1280 * 2;
		}
	};

	/**
	 * Get Adc configured
	 * @param index thread index for debugging purposes
	 * @param i ROI
	 * @returns adc configured
	 */
	int GetAdcConfigured(int index, slsDetectorDefs::ROI i)  const{
		int adc = -1;
		// single adc
		if(i.xmin != -1)  {
			// gotthard can have only one adc per detector enabled (or all)
			//adc = mid value/numchans also for only 1 roi
			adc = ((((i.xmax) + (i.xmin))/2)/
					(nChan * nChipsPerAdc));
			if((adc < 0) || (adc > 4)) {
				LOG(logWARNING) << index << ": Deleting ROI. "
						"Adc value should be between 0 and 4";
				adc = -1;
			}
		}
		LOG(logINFO) << "Adc Configured: " << adc;
		return adc;
	};

    /**
     * Set odd starting packet (gotthard)
     * @param index thread index for debugging purposes
     * @param packetData pointer to data
     * @returns true or false for odd starting packet number
     */
    bool SetOddStartingPacket(int index, char* packetData) {
        bool oddStartingPacket = true;
        // care only if no roi
        if  (nPixelsX == 1280) {
            uint32_t fnum = ((uint32_t)(*((uint32_t*)(packetData))));
            uint32_t firstData = ((uint32_t)(*((uint32_t*)(packetData + 4))));
            // first packet
            if (firstData == 0xCACACACA) {
                // packet number should be 0, but is 1 => so odd starting packet
                if (fnum & packetIndexMask) {
                    oddStartingPacket = true;
                } else {
                    oddStartingPacket = false;
                }
            }
            // second packet
            else {
                // packet number should be 1, but is 0 => so odd starting packet
                if (!(fnum & packetIndexMask)) {
                    oddStartingPacket = true;
                } else {
                    oddStartingPacket = false;
                }
            }
        }
        return oddStartingPacket;
    };

};


class EigerData : public GeneralData {

 public:

	/** Constructor */
	EigerData(){
		myDetectorType		= slsDetectorDefs::EIGER;
		nPixelsX 			= (256*2);
		nPixelsY 			= 256;
		headerSizeinPacket  = sizeof(slsDetectorDefs::sls_detector_header);
		dataSize 			= 1024;
		packetSize 			= headerSizeinPacket + dataSize;
		packetsPerFrame 	= 256;
		imageSize 			= dataSize*packetsPerFrame;
		maxFramesPerFile 	= EIGER_MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
		defaultFifoDepth 	= 100;
		threadsPerReceiver	= 2;
		headerPacketSize	= 40;
		standardheader		= true;
	};

	/**
	 * Setting dynamic range changes member variables
	 * @param dr dynamic range
	 * @param tgEnable true if 10GbE is enabled, else false
	 */
	void SetDynamicRange(int dr, bool tgEnable) {
		packetsPerFrame = (tgEnable ? 4 : 16) * dr;
		imageSize 		= dataSize*packetsPerFrame;
	}

	/**
	 * Setting ten giga enable changes member variables
	 * @param tgEnable true if 10GbE is enabled, else false
	 * @param dr dynamic range
	 */
	void SetTenGigaEnable(bool tgEnable, int dr) {
		dataSize 		= (tgEnable ? 4096 : 1024);
		packetSize 		= headerSizeinPacket + dataSize;
		packetsPerFrame = (tgEnable ? 4 : 16) * dr;
		imageSize 		= dataSize*packetsPerFrame;
	};

};



class JungfrauData : public GeneralData {

 public:

	/** Constructor */
	JungfrauData(){
		myDetectorType		= slsDetectorDefs::JUNGFRAU;
		nPixelsX 			= (256*4);
		nPixelsY 			= 512;
		headerSizeinPacket  = sizeof(slsDetectorDefs::sls_detector_header);
		dataSize 			= 8192;
		packetSize 			= headerSizeinPacket + dataSize;
		packetsPerFrame 	= 128;
		imageSize 			= dataSize * packetsPerFrame;
		maxFramesPerFile 	= JFRAU_MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
		defaultFifoDepth 	= 2500;
		standardheader		= true;
		defaultUdpSocketBufferSize = (1000 * 1024 * 1024);
	};


    /**
     * set number of interfaces (jungfrau)
     * @param number of interfaces
     */
    void SetNumberofInterfaces(const int n) {
    	// 2 interfaces
    	if (n == 2) {
    		nPixelsY 					= 256;
    		packetsPerFrame 			= 64;
    		imageSize 					= dataSize * packetsPerFrame;
    		threadsPerReceiver			= 2;
    		defaultUdpSocketBufferSize 	= (500 * 1024 * 1024);

    	}
    	// 1 interface
    	else  {
    		nPixelsY 					= 512;
    		packetsPerFrame 			= 128;
    		imageSize 					= dataSize * packetsPerFrame;
    		threadsPerReceiver			= 1;
    		defaultUdpSocketBufferSize 	= (1000 * 1024 * 1024);
    	}
    }
};

class Mythen3Data : public GeneralData {
private:
	int ncounters;
	const int NCHAN = 1280;
public:

	/** Constructor */
	Mythen3Data(){
		myDetectorType		= slsDetectorDefs::MYTHEN3;
		ncounters			= 3;
		nPixelsX 			= (NCHAN * ncounters); // max 1280 channels x 3 counters
		nPixelsY 			= 1;
		headerSizeinPacket  = sizeof(slsDetectorDefs::sls_detector_header);
		dataSize 			= 7680;
		packetSize 			= headerSizeinPacket + dataSize;
		packetsPerFrame 	= 2;
		imageSize 			= dataSize * packetsPerFrame;
		maxFramesPerFile 	= MYTHEN3_MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
		defaultFifoDepth 	= 50000;
		standardheader		= true;
		defaultUdpSocketBufferSize = (1000 * 1024 * 1024);
	};

    /**
     * set number of counters (mythen3)
     * @param n number of counters
	 * @param dr dynamic range
     */
    virtual void SetNumberofCounters(const int n, const int dr) {
		if (n < 1 || n > 3) {
			throw sls::RuntimeError("Invalid number of counters " + std::to_string(n));
		}
		ncounters = n;
		nPixelsX = NCHAN * ncounters;
		LOG(logINFO) << "nPixelsX: " << nPixelsX;
		imageSize = nPixelsX * nPixelsY * 	((dr > 16) ? 4 : 	// 32 bit
											((dr > 8)  ? 2 : 	// 16 bit
											((dr > 4)  ? 0.5 : 	// 4 bit
											0.125)));			// 1 bit
		dataSize = imageSize / packetsPerFrame;
		packetSize 			= headerSizeinPacket + dataSize;
		LOG(logINFO) << "PacketSize: " << packetSize;
	}

	/**
	 * Setting dynamic range changes member variables
	 * @param dr dynamic range
	 * @param tgEnable (discarded, of no value to mythen3)
	 */
	void SetDynamicRange(int dr, bool tgEnable) {
		imageSize = nPixelsX * nPixelsY * 	((dr > 16) ? 4 : 	// 32 bit
											((dr > 8)  ? 2 : 	// 16 bit
											((dr > 4)  ? 0.5 : 	// 4 bit
											0.125)));			// 1 bit
		dataSize = imageSize / packetsPerFrame;
		packetSize 			= headerSizeinPacket + dataSize;
		LOG(logINFO) << "PacketSize: " << packetSize;
	}	
};


class Gotthard2Data : public GeneralData {
public:

	/** Constructor */
	Gotthard2Data(){
		myDetectorType		= slsDetectorDefs::GOTTHARD2;
		nPixelsX 			= 128 * 10;
		nPixelsY 			= 1;
		headerSizeinPacket  = sizeof(slsDetectorDefs::sls_detector_header);
		dataSize 			= 2560; // 1280 channels * 2 bytes
		packetSize 			= headerSizeinPacket + dataSize;
		packetsPerFrame 	= 1;
		imageSize 			= dataSize * packetsPerFrame;
		maxFramesPerFile 	= GOTTHARD2_MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
		defaultFifoDepth 	= 50000;
		standardheader		= true;
		defaultUdpSocketBufferSize = (1000 * 1024 * 1024);
	};
};


class ChipTestBoardData : public GeneralData {
private:
	/** Number of analog channels */
	const int NCHAN_ANALOG = 32;
	/** Number of digital channels */
	const int NCHAN_DIGITAL = 64;
	/** Number of bytes per analog channel */
	const int NUM_BYTES_PER_ANALOG_CHANNEL = 2;

public:


	/** Constructor */
	ChipTestBoardData(){
		myDetectorType		= slsDetectorDefs::CHIPTESTBOARD;
		nPixelsX 			= 36; // total number of channels
		nPixelsY 			= 1; // number of samples
		headerSizeinPacket  = sizeof(slsDetectorDefs::sls_detector_header);
		dataSize 			= UDP_PACKET_DATA_BYTES;
		packetSize 			= headerSizeinPacket + dataSize;
		//packetsPerFrame 	= 1;
		imageSize 			= nPixelsX * nPixelsY * 2;
		frameIndexMask 		= 0xFFFFFF; // 10g
		frameIndexOffset	= 8; // 10g
		packetIndexMask 	= 0xFF; //10g
		packetsPerFrame		= ceil((double)imageSize / (double)UDP_PACKET_DATA_BYTES);
		maxFramesPerFile 	= CTB_MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
		defaultFifoDepth 	= 2500;
		standardheader		= true;
	};

    /**
     * Set databytes 
     * @param a adc enable mask
	 * @param as analog number of samples
     * @param ds digital number of samples
     * @param t tengiga enable
	 * @param f readout flags
	 * @returns analog data bytes
     */
	int setImageSize(uint32_t a, uint32_t as, uint32_t ds, bool t, slsDetectorDefs::readoutMode f) {
        int nachans = 0, ndchans = 0;
        int adatabytes = 0, ddatabytes = 0;

		// analog channels (normal, analog/digital readout)
		if (f == slsDetectorDefs::ANALOG_ONLY ||
			f == slsDetectorDefs::ANALOG_AND_DIGITAL) {
			if (a == BIT32_MASK) {
				nachans = 32;
			} else {
				for (int ich = 0; ich < 32; ++ich) {
					if (a & (1 << ich))
						++nachans;
				}
			}
			adatabytes = nachans * NUM_BYTES_PER_ANALOG_CHANNEL * as;
			LOG(logDEBUG1) << " Number of Analog Channels:" << nachans
								<< " Databytes: " << adatabytes;
		}
		// digital channels
		if (f == slsDetectorDefs::DIGITAL_ONLY ||
			f == slsDetectorDefs::ANALOG_AND_DIGITAL) {
			ndchans = NCHAN_DIGITAL;
			ddatabytes = (sizeof(uint64_t) * ds);
			LOG(logDEBUG1) << "Number of Digital Channels:" << ndchans
								<< " Databytes: " << ddatabytes;
		}
		LOG(logDEBUG1) << "Total Number of Channels:" << nachans + ndchans
							<< " Databytes: " << adatabytes + ddatabytes;

        nPixelsX = nachans + ndchans;
        nPixelsY = 1;
		
        // 10G
        if (t) {
            dataSize = 8144;
		}
	    // 1g udp (via fifo readout)
	    else {
			dataSize 			= UDP_PACKET_DATA_BYTES;
	    }

		packetSize = headerSizeinPacket + dataSize;
        imageSize = adatabytes + ddatabytes;
        packetsPerFrame = ceil((double)imageSize / (double)dataSize);

		return adatabytes;
	}

};


class MoenchData : public GeneralData {


private:
	/** Number of bytes per analog channel */
	const int NUM_BYTES_PER_ANALOG_CHANNEL = 2;

public:
  /** Constructor */
  MoenchData() {
      myDetectorType 			= slsDetectorDefs::MOENCH;
      nPixelsX 					= 32; // total number of channels
      nPixelsY 					= 1;  // number of samples
      headerSizeinPacket 		= sizeof(slsDetectorDefs::sls_detector_header);
      dataSize 					= UDP_PACKET_DATA_BYTES;
      packetSize 				= headerSizeinPacket + dataSize;
      // packetsPerFrame 		= 1;
      imageSize 				= nPixelsX * nPixelsY * 2;
      packetsPerFrame 			= ceil((double)imageSize / (double)UDP_PACKET_DATA_BYTES);
      frameIndexMask 			= 0xFFFFFF;
      maxFramesPerFile 			= MOENCH_MAX_FRAMES_PER_FILE;
      fifoBufferHeaderSize 		= FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
      defaultFifoDepth 			= 2500;
      standardheader 			= true;
	};

    /**
     * Set databytes 
     * @param a adc enable mask
	 * @param as analog number of samples
     * @param ds digital number of samples
     * @param t tengiga enable
	 * @param f readout flags
	 * @returns analog data bytes
     */
	int setImageSize(uint32_t a, uint32_t as, uint32_t ds, bool t, slsDetectorDefs::readoutMode) {
        
		// count number of channels in x, each adc has 25 channels each
        int nchanTop =  __builtin_popcount(a & 0xF0F0F0F0) * 25;
        int nchanBot = __builtin_popcount(a & 0x0F0F0F0F) * 25;
        nPixelsX = nchanTop > 0 ? nchanTop : nchanBot; 

        // if both top and bottom adcs enabled, rows = 2
        int nrows = 1;
        if (nchanTop > 0 && nchanBot > 0) {
            nrows = 2;
        }        
        nPixelsY = as / 25 * nrows;
		LOG(logINFO) << "Number of Pixels: [" << nPixelsX << ", " << nPixelsY << "]";


        // 10G
        if (t) {
            dataSize = 8144;
		}
	    // 1g udp (via fifo readout)
	    else {
			dataSize = UDP_PACKET_DATA_BYTES;
	    }

		imageSize 			= nPixelsX * nPixelsY * NUM_BYTES_PER_ANALOG_CHANNEL;
		packetSize 			= headerSizeinPacket + dataSize;
		packetsPerFrame 	= ceil((double)imageSize / (double)dataSize);
		
		LOG(logDEBUG) << "Databytes: " << imageSize;

		return imageSize;
	}
};



