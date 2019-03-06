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

#include <math.h>			//ceil
#include <vector>


class GeneralData {
	
public:

	/** DetectorType */
	slsDetectorDefs::detectorType myDetectorType;

	/** Number of Pixels in x axis */
	uint32_t nPixelsX;

	/** Number of Pixels in y axis */
	uint32_t nPixelsY;

	/** emptybuffer  (mainly for jungfrau) */
	uint32_t emptyHeader;

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
		emptyHeader(0),
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
	 * @param dynamicRange dynamic range to assign subframenumber if 32 bit mode
	 * @param oddStartingPacket odd starting packet (gotthard)
	 * @param frameNumber frame number
	 * @param packetNumber packet number
	 * @param subFrameNumber sub frame number if applicable
	 * @param bunchId bunch id
	 */
	virtual void GetHeaderInfo(int index, char* packetData, uint32_t dynamicRange, bool oddStartingPacket,
			uint64_t& frameNumber, uint32_t& packetNumber, uint32_t& subFrameNumber, uint64_t& bunchId) const
	{
		subFrameNumber = -1;
		bunchId = -1;
		frameNumber = ((uint32_t)(*((uint32_t*)(packetData))));
		frameNumber++;
		packetNumber = frameNumber&packetIndexMask;
		frameNumber = (frameNumber & frameIndexMask) >> frameIndexOffset;
	}

	/**
	 * Set ROI
	 * @param i ROI
	 */
	virtual void SetROI(std::vector<slsDetectorDefs::ROI> i) {
		FILE_LOG(logERROR) << "SetROI is a generic function that should be overloaded by a derived class";
	};

	/**
	 * Get Adc configured
	 * @param index thread index for debugging purposes
	 * @param i pointer to a vector of ROI pointers
	 * @returns adc configured
	 */
	virtual int GetAdcConfigured(int index, std::vector<slsDetectorDefs::ROI>* i)  const{
		FILE_LOG(logERROR) << "GetAdcConfigured is a generic function that should be overloaded by a derived class";
		return 0;
	};

	/**
	 * Setting dynamic range changes member variables
	 * @param dr dynamic range
	 * @param tgEnable true if 10GbE is enabled, else false
	 */
	virtual void SetDynamicRange(int dr, bool tgEnable) {
		FILE_LOG(logERROR) << "SetDynamicRange is a generic function that should be overloaded by a derived class";
	};

	/**
	 * Setting ten giga enable changes member variables
	 * @param tgEnable true if 10GbE is enabled, else false
	 * @param dr dynamic range
	 */
	virtual void SetTenGigaEnable(bool tgEnable, int dr) {
		FILE_LOG(logERROR) << "SetTenGigaEnable is a generic function that should be overloaded by a derived class";
	};

	/**
	 * Setting packets per frame changes member variables
	 * @param ns number of samples
	 */
	virtual void setNumberofSamples(const uint64_t ns) {
		FILE_LOG(logERROR) << "setNumberofSamples is a generic function that should be overloaded by a derived class";
	};

	/**
	 * Enable Gap Pixels changes member variables
	 * @param enable true if gap pixels enable, else false
	 */
	virtual void SetGapPixelsEnable(bool b, int dr) {
		FILE_LOG(logERROR) << "SetGapPixelsEnable is a generic function that should be overloaded by a derived class";
	};

    /**
     * Set odd starting packet (gotthard)
     * @param index thread index for debugging purposes
     * @param packetData pointer to data
     * @returns true or false for odd starting packet number
     */
    virtual bool SetOddStartingPacket(int index, char* packetData) {
        cprintf(RED,"SetOddStartingPacket is a generic function that should be overloaded by a derived class\n");
        return false;
    };

    /**
     * Set databytes (ctb, moench)
     * @param f readout flags
     * @param r roi
     * @param s number of samples
     * @param t tengiga enable
     */
    virtual void setImageSize(std::vector<slsDetectorDefs::ROI> r, int s, bool t, slsDetectorDefs::readOutFlags f = slsDetectorDefs::GET_READOUT_FLAGS) {
        cprintf(RED,"setImageSize is a generic function that should be overloaded by a derived class\n");
    };

	/**
	 * Print all variables
	 */
	virtual void Print(TLogLevel level = logDEBUG1) const {
		FILE_LOG(level) << "\n\nDetector Data Variables:";
		FILE_LOG(level) << "myDetectorType: " << slsDetectorDefs::detectorTypeToString(myDetectorType);
		FILE_LOG(level) << "Pixels X: " << nPixelsX;
		FILE_LOG(level) << "Pixels Y: " << nPixelsY;
		FILE_LOG(level) << "Empty Header: " << emptyHeader;
		FILE_LOG(level) << "Header Size in Packet: " << headerSizeinPacket;
		FILE_LOG(level) << "Data Size: " << dataSize;
		FILE_LOG(level) << "Packet Size: " << packetSize;
		FILE_LOG(level) << "Packets per Frame: " << packetsPerFrame;
		FILE_LOG(level) << "Image Size: " << imageSize;
		FILE_LOG(level) << "Frame Index Mask: " << frameIndexMask;
		FILE_LOG(level) << "Frame Index Offset: " << frameIndexOffset;
		FILE_LOG(level) << "Packet Index Mask: " << packetIndexMask;
		FILE_LOG(level) << "Packet Index Offset: " << packetIndexOffset;
		FILE_LOG(level) << "Max Frames Per File: " << maxFramesPerFile;
		FILE_LOG(level) << "Fifo Buffer Header Size: " << fifoBufferHeaderSize;
		FILE_LOG(level) << "Default Fifo Depth: " << defaultFifoDepth;
		FILE_LOG(level) << "Threads Per Receiver: " << threadsPerReceiver;
		FILE_LOG(level) << "Header Packet Size: " << headerPacketSize;
		FILE_LOG(level) << "Complete Pixels X: " << nPixelsXComplete;
		FILE_LOG(level) << "Complete Pixels Y: " << nPixelsYComplete;
		FILE_LOG(level) << "Complete Image Size: " << imageSizeComplete;
		FILE_LOG(level) << "Standard Header: " << standardheader;
		FILE_LOG(level) << "UDP Socket Buffer Size: " << defaultUdpSocketBufferSize;
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
	 * @param dynamicRange dynamic range to assign subframenumber if 32 bit mode
	 * @param oddStartingPacket odd starting packet (gotthard)
	 * @param frameNumber frame number
	 * @param packetNumber packet number
	 * @param subFrameNumber sub frame number if applicable
	 * @param bunchId bunch id
	 */
	void GetHeaderInfo(int index, char* packetData, uint32_t dynamicRange, bool oddStartingPacket,
			uint64_t& frameNumber, uint32_t& packetNumber, uint32_t& subFrameNumber, uint64_t& bunchId) const
	{
		if (nPixelsX == 1280) {
			subFrameNumber = -1;
			bunchId = -1;
			frameNumber = ((uint32_t)(*((uint32_t*)(packetData))));
			if (oddStartingPacket)
			    frameNumber++;
			packetNumber = frameNumber&packetIndexMask;
			frameNumber = (frameNumber & frameIndexMask) >> frameIndexOffset;
		} else  {
			subFrameNumber = -1;
			bunchId = -1;
			frameNumber = ((uint32_t)(*((uint32_t*)(packetData))));
			packetNumber = 0;
		}
	}


	/**
	 * Set ROI
	 * @param i ROI
	 */
	void SetROI(std::vector<slsDetectorDefs::ROI> i) {
		// all adcs
		if(!i.size()) {
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
			defaultFifoDepth 	= 25000;
			nPixelsXComplete 	= 1280;
			nPixelsYComplete 	= 1;
			imageSizeComplete 	= 1280 * 2;
		}
	};

	/**
	 * Get Adc configured
	 * @param index thread index for debugging purposes
	 * @param i pointer to a vector of ROI
	 * @returns adc configured
	 */
	int GetAdcConfigured(int index, std::vector<slsDetectorDefs::ROI>* i)  const{
		int adc = -1;
		// single adc
		if(i->size())  {
			// gotthard can have only one adc per detector enabled (or all)
			// so just looking at the first roi is enough (more not possible at the moment)

			//if its for 1 adc or general
			if ((i->at(0).xmin == 0) && (i->at(0).xmax == nChip * nChan))
				adc = -1;
			else {
				//adc = mid value/numchans also for only 1 roi
				adc = ((((i->at(0).xmax) + (i->at(0).xmin))/2)/
						(nChan * nChipsPerAdc));
				if((adc < 0) || (adc > 4)) {
					FILE_LOG(logWARNING) << index << ": Deleting ROI. "
							"Adc value should be between 0 and 4";
					adc = -1;
				}
			}
		}
		FILE_LOG(logINFO) << "Adc Configured: " << adc;
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

	/**
	 * Enable Gap Pixels changes member variables
	 * @param enable true if gap pixels enable, else false
	 * @param dr dynamic range
	 */
	void SetGapPixelsEnable(bool b, int dr) {
		if (dr == 4)
			b = 0;
		switch((int)b) {
		case 1:
			nPixelsX	= (256 * 2) + 3;
			nPixelsY 	= 256 + 1;
			imageSize	= nPixelsX * nPixelsY * ((dr > 16) ? 4 : // 32 bit
												((dr > 8)  ? 2 : // 16 bit
												((dr > 4)  ? 1 : // 8 bit
												0.5)));			 // 4 bit
			break;
		default:
			nPixelsX 	= (256*2);
			nPixelsY 	= 256;
			imageSize	= nPixelsX * nPixelsY * ((dr > 16) ? 4 : // 32 bit
												((dr > 8)  ? 2 : // 16 bit
												((dr > 4)  ? 1 : // 8 bit
												0.5)));			 // 4 bit
			break;
		}
	};


};



class JungfrauData : public GeneralData {

 public:

	/** Constructor */
	JungfrauData(){
		myDetectorType		= slsDetectorDefs::JUNGFRAU;
		nPixelsX 			= (256*4);
		nPixelsY 			= 512;
		emptyHeader			= 6;
		headerSizeinPacket  = emptyHeader + sizeof(slsDetectorDefs::sls_detector_header);
		dataSize 			= 8192;
		packetSize 			= headerSizeinPacket + dataSize;
		packetsPerFrame 	= 128;
		imageSize 			= dataSize*packetsPerFrame;
		maxFramesPerFile 	= JFRAU_MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
		defaultFifoDepth 	= 2500;
		standardheader		= true;
		defaultUdpSocketBufferSize = (2000 * 1024 * 1024);
	};

};



class ChipTestBoardData : public GeneralData {
private:
	/** Number of analog channels */
	const int NCHAN_ANALOG = 32;
	/** Number of digital channels */
	const int NCHAN_DIGITAL = 4;
	/** Number of bytes per pixel */
	const int NUM_BYTES_PER_PIXEL = 2;
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
		packetsPerFrame		= ceil((double)imageSize / (double)UDP_PACKET_DATA_BYTES);
		maxFramesPerFile 	= CTB_MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
		defaultFifoDepth 	= 2500;
		standardheader		= true;
	};

	/**
	 * Set databytes (ctb, moench)
	 * @param f readout flags
	 * @param r roi
	 * @param s number of samples
	 * @param t tengiga enable
	 */
	void setImageSize(std::vector<slsDetectorDefs::ROI> r, int s, bool t, slsDetectorDefs::readOutFlags f = slsDetectorDefs::GET_READOUT_FLAGS) {
		 int nchans = 0;
		 if (f != slsDetectorDefs::GET_READOUT_FLAGS) {
			 // analog channels
			 if (f == slsDetectorDefs::NORMAL_READOUT || f & slsDetectorDefs::ANALOG_AND_DIGITAL) {
				 nchans += NCHAN_ANALOG;
				 // if roi
				 if (r.size()) {
					 nchans = 0;
					 for (auto &roi : r) {
						 nchans += (roi.xmax - roi.xmin + 1);
					 }
				 }
			 }
			 // digital channels
			 if (f & slsDetectorDefs::DIGITAL_ONLY || f & slsDetectorDefs::ANALOG_AND_DIGITAL) {
				 nchans += NCHAN_DIGITAL;
			 }
		 }
	    nPixelsX = nchans;
	    nPixelsY = s;
	    // 10G
	    if (t) {
			headerSizeinPacket 	= 22;
			dataSize 			= 8192;
			packetSize 			= headerSizeinPacket + dataSize;
			imageSize 			= nPixelsX * nPixelsY * 2;
			packetsPerFrame 	= ceil((double)imageSize / (double)packetSize);
			standardheader		= false;	    }
	    // 1g udp (via fifo readout)
	    else {
			headerSizeinPacket 	= sizeof(slsDetectorDefs::sls_detector_header);
			dataSize 			= UDP_PACKET_DATA_BYTES;
			packetSize 			= headerSizeinPacket + dataSize;
			imageSize 			= nPixelsX * nPixelsY * 2;
			packetsPerFrame 	= ceil((double)imageSize / (double)UDP_PACKET_DATA_BYTES);
			standardheader		= true;
	    }
	}

};


class MoenchData : public GeneralData {


private:
	/** Number of analog channels */
	const int NCHAN_ANALOG = 32;
	/** Number of bytes per pixel */
	const int NUM_BYTES_PER_PIXEL = 2;

	/** Structure of an jungfrau ctb packet header (10G Udp) */
	typedef struct {
		unsigned char emptyHeader[6];
		unsigned char reserved[4];
		unsigned char packetNumber[1];
		unsigned char frameNumber[3];
		unsigned char bunchid[8];
	} jfrauctb_packet_header_t;

 public:

	/** Constructor */
	MoenchData(){
		myDetectorType		= slsDetectorDefs::MOENCH;
		nPixelsX 			= 32; // total number of channels
		nPixelsY 			= 1; // number of samples
		headerSizeinPacket  = sizeof(slsDetectorDefs::sls_detector_header);
		dataSize 			= UDP_PACKET_DATA_BYTES;
		packetSize 			= headerSizeinPacket + dataSize;
		//packetsPerFrame 	= 1;
		imageSize 			= nPixelsX * nPixelsY * 2;
		packetsPerFrame		= ceil((double)imageSize / (double)UDP_PACKET_DATA_BYTES);
		frameIndexMask 		= 0xFFFFFF;
		maxFramesPerFile 	= CTB_MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsDetectorDefs::sls_receiver_header);
		defaultFifoDepth 	= 2500;
		standardheader		= true;
	};

	/**
	 * Get Header Infomation (frame number, packet number)
	 * @param index thread index for debugging purposes
	 * @param packetData pointer to data
	 * @param dynamicRange dynamic range to assign subframenumber if 32 bit mode
	 * @param oddStartingPacket odd starting packet (gotthard)
	 * @param frameNumber frame number 	 * @param packetNumber packet number
	 * @param subFrameNumber sub frame number if applicable
	 * @param bunchId bunch id
	 */
	void GetHeaderInfo(int index, char* packetData, uint32_t dynamicRange, bool oddStartingPacket,
			uint64_t& frameNumber, uint32_t& packetNumber, uint32_t& subFrameNumber, uint64_t& bunchId) const 	{
		subFrameNumber = -1;
		jfrauctb_packet_header_t* header = (jfrauctb_packet_header_t*)(packetData);
		frameNumber = (uint64_t)((*( (uint32_t*) header->frameNumber)) & frameIndexMask);
		packetNumber = (uint32_t)(*( (uint8_t*) header->packetNumber));
		bunchId = (*((uint64_t*) header->bunchid));
	}


	/**
	 * Set databytes (ctb, moench)
	 * @param f readout flags
	 * @param r roi
	 * @param s number of samples
	 * @param t tengiga enable
	 */
	void setImageSize(std::vector<slsDetectorDefs::ROI> r, int s, bool t,
			slsDetectorDefs::readOutFlags f = slsDetectorDefs::GET_READOUT_FLAGS) {
		int nchans = NCHAN_ANALOG;
		// if roi
		if (r.size()) {
			 nchans = 0;
			 for (auto &roi : r) {
				 nchans += abs(roi.xmax - roi.xmin) + 1;
			 }
		}

		nPixelsX = nchans;
		nPixelsY = s;
		// 10G
		if (t) {
			headerSizeinPacket 	= 22;
			dataSize 			= 8192;
			packetSize 			= headerSizeinPacket + dataSize;
			imageSize 			= nPixelsX * nPixelsY * 2;
			packetsPerFrame 	= ceil((double)imageSize / (double)packetSize);
			standardheader		= false;
		}
		// 1g udp (via fifo readout)
		else {
			headerSizeinPacket 	= sizeof(slsDetectorDefs::sls_detector_header);
			dataSize 			= UDP_PACKET_DATA_BYTES;
			packetSize 			= headerSizeinPacket + dataSize;
			imageSize 			= nPixelsX * nPixelsY * 2;
			packetsPerFrame 	= ceil((double)imageSize / (double)UDP_PACKET_DATA_BYTES);
			standardheader		= true;
		}
	}
};




