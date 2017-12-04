#pragma once
/************************************************
 * @file GeneralData.h
 * @short abstract for setting/getting properties of detector data
 ***********************************************/
/**
 *@short abstract for setting/getting properties of detector data
 */

#include "sls_receiver_defs.h"
#include "receiver_defs.h"


class GeneralData {
	
public:

	/** DetectorType */
	slsReceiverDefs::detectorType myDetectorType;

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
	uint32_t nPixelsX_Streamer;

	/** Streaming (for ROI - mainly short Gotthard) - Number of Pixels in y axis */
	uint32_t nPixelsY_Streamer;

	/** Streaming (for ROI - mainly short Gotthard) - Image size (in bytes) */
	uint32_t imageSize_Streamer;




	/** Cosntructor */
	GeneralData():
		myDetectorType(slsReceiverDefs::GENERIC),
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
		nPixelsX_Streamer(0),
		nPixelsY_Streamer(0),
		imageSize_Streamer(0)
		{};

	/** Destructor */
	virtual ~GeneralData(){};

	/**
	 * Get Header Infomation (frame number, packet number)
	 * @param index thread index for debugging purposes
	 * @param packetData pointer to data
	 * @param frameNumber frame number
	 * @param packetNumber packet number
	 */
	virtual void GetHeaderInfo(int index, char* packetData,	uint64_t& frameNumber, uint32_t& packetNumber) const
	{
		frameNumber = ((uint32_t)(*((uint32_t*)(packetData))));
		frameNumber++;
		packetNumber = frameNumber&packetIndexMask;
		frameNumber = (frameNumber & frameIndexMask) >> frameIndexOffset;
	}

	/**
	 * Get Header Infomation (frame number, packet number)
	 * @param index thread index for debugging purposes
	 * @param packetData pointer to data
	 * @param dynamicRange dynamic range to assign subframenumber if 32 bit mode
	 * @param frameNumber frame number
	 * @param packetNumber packet number
	 * @param subFrameNumber sub frame number if applicable
	 * @param bunchId bunch id
	 */
	virtual void GetHeaderInfo(int index, char* packetData, uint32_t dynamicRange,
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
	 * Setting dynamic range changes member variables
	 * @param dr dynamic range
	 * @param tgEnable true if 10GbE is enabled, else false
	 */
	virtual void SetDynamicRange(int dr, bool tgEnable) {
		bprintf(RED,"This is a generic function that should be overloaded by a derived class\n");
	};

	/**
	 * Setting ten giga enable changes member variables
	 * @param tgEnable true if 10GbE is enabled, else false
	 * @param dr dynamic range
	 */
	virtual void SetTenGigaEnable(bool tgEnable, int dr) {
		bprintf(RED,"This is a generic function that should be overloaded by a derived class\n");
	};

	/**
	 * Print all variables
	 */
	virtual void Print() const {
		FILE_LOG(logDEBUG) << "\n\nDetector Data Variables:";
		FILE_LOG(logDEBUG) << "myDetectorType: " << slsReceiverDefs::getDetectorType(myDetectorType);
		FILE_LOG(logDEBUG) << "Pixels X: " << nPixelsX;
		FILE_LOG(logDEBUG) << "Pixels Y: " << nPixelsY;
		FILE_LOG(logDEBUG) << "Empty Header: " << emptyHeader;
		FILE_LOG(logDEBUG) << "Header Size in Packet: " << headerSizeinPacket;
		FILE_LOG(logDEBUG) << "Data Size: " << dataSize;
		FILE_LOG(logDEBUG) << "Packet Size: " << packetSize;
		FILE_LOG(logDEBUG) << "Packets per Frame: " << packetsPerFrame;
		FILE_LOG(logDEBUG) << "Image Size: " << imageSize;
		FILE_LOG(logDEBUG) << "Frame Index Mask: " << frameIndexMask;
		FILE_LOG(logDEBUG) << "Frame Index Offset: " << frameIndexOffset;
		FILE_LOG(logDEBUG) << "Packet Index Mask: " << packetIndexMask;
		FILE_LOG(logDEBUG) << "Packet Index Offset: " << packetIndexOffset;
		FILE_LOG(logDEBUG) << "Max Frames Per File: " << maxFramesPerFile;
		FILE_LOG(logDEBUG) << "Fifo Buffer Header Size: " << fifoBufferHeaderSize;
		FILE_LOG(logDEBUG) << "Default Fifo Depth: " << defaultFifoDepth;
		FILE_LOG(logDEBUG) << "Threads Per Receiver: " << threadsPerReceiver;
		FILE_LOG(logDEBUG) << "Header Packet Size: " << headerPacketSize;
		FILE_LOG(logDEBUG) << "Streamer Pixels X: " << nPixelsX_Streamer;
		FILE_LOG(logDEBUG) << "Streamer Pixels Y: " << nPixelsY_Streamer;
		FILE_LOG(logDEBUG) << "Streamer Image Size: " << imageSize_Streamer;
	};
};


class GotthardData : public GeneralData {

 public:

	/** Constructor */
	GotthardData(){
		myDetectorType		= slsReceiverDefs::GOTTHARD;
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
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsReceiverDefs::sls_detector_header);
		defaultFifoDepth 	= 50000;
		nPixelsX_Streamer 	= nPixelsX;
		nPixelsY_Streamer 	= nPixelsY;
		imageSize_Streamer 	= imageSize;
	};
};


class ShortGotthardData : public GeneralData {

 public:

	/** Constructor */
	ShortGotthardData(){
		myDetectorType		= slsReceiverDefs::GOTTHARD;
		nPixelsX 			= 256;
		nPixelsY 			= 1;
		headerSizeinPacket  = 4;
		dataSize 			= 512;
		packetSize 			= 518;
		packetsPerFrame 	= 1;
		imageSize 			= dataSize*packetsPerFrame;
		frameIndexMask 		= 0xFFFFFFFF;
		maxFramesPerFile 	= SHORT_MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsReceiverDefs::sls_detector_header);
		defaultFifoDepth 	= 50000;
		nPixelsX_Streamer 	= 1280;
		nPixelsY_Streamer 	= 1;
		imageSize_Streamer 	= 1280 * 2;
	};

	/**
	 * Get Header Infomation (frame number, packet number)
	 * @param index thread index for debugging purposes
	 * @param packetData pointer to data
	 * @param frameNumber frame number
	 * @param packetNumber packet number
	 */
	virtual void GetHeaderInfo(int index, char* packetData,	uint64_t& frameNumber, uint32_t& packetNumber) const
	{
		frameNumber = ((uint32_t)(*((uint32_t*)(packetData))));
		packetNumber = 0;
	}

	/**
	 * Get Header Infomation (frame number, packet number)
	 * @param index thread index for debugging purposes
	 * @param packetData pointer to data
	 * @param dynamicRange dynamic range to assign subframenumber if 32 bit mode
	 * @param frameNumber frame number
	 * @param packetNumber packet number
	 * @param subFrameNumber sub frame number if applicable
	 * @param bunchId bunch id
	 */
	virtual void GetHeaderInfo(int index, char* packetData, uint32_t dynamicRange,
			uint64_t& frameNumber, uint32_t& packetNumber, uint32_t& subFrameNumber, uint64_t& bunchId) const
	{
		subFrameNumber = -1;
		bunchId = -1;
		frameNumber = ((uint32_t)(*((uint32_t*)(packetData))));
		packetNumber = 0;
	}
};


class PropixData : public GeneralData {

 private:

	/**bytes per pixel for calculating image size */
	const static uint32_t bytesPerPixel = 2;

 public:

	/** Constructor */
	PropixData(){
		myDetectorType		= slsReceiverDefs::PROPIX;
		nPixelsX 			= 22;
		nPixelsY 			= 22;
		headerSizeinPacket  = 4;
		dataSize 			= 1280;
		packetSize 			= 1286;
		packetsPerFrame 	= 2; //not really
		imageSize 			= nPixelsX*nPixelsY*bytesPerPixel;
		frameIndexMask 		= 0xFFFFFFFE;
		frameIndexOffset 	= 1;
		packetIndexMask 	= 1;
		maxFramesPerFile 	= MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsReceiverDefs::sls_detector_header);
		defaultFifoDepth 	= 50000;
		nPixelsX_Streamer 	= nPixelsX;
		nPixelsY_Streamer 	= nPixelsY;
		imageSize_Streamer 	= imageSize;
	};
};


class Moench02Data : public GeneralData {

 public:

	/** Bytes Per Adc */
	const static uint32_t bytesPerAdc = (40*2);

	/** Constructor */
	Moench02Data(){
		myDetectorType		= slsReceiverDefs::MOENCH;
		nPixelsX 			= 160;
		nPixelsY 			= 160;
		headerSizeinPacket  = 4;
		dataSize 			= 1280;
		packetSize 			= 1286;
		packetsPerFrame 	= 40;
		imageSize 			= dataSize*packetsPerFrame;
		frameIndexMask 		= 0xFFFFFF00;
		frameIndexOffset 	= 8;
		packetIndexMask 	= 0xFF;
		maxFramesPerFile 	= MOENCH_MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsReceiverDefs::sls_detector_header);
		defaultFifoDepth 	= 2500;
		nPixelsX_Streamer 	= nPixelsX;
		nPixelsY_Streamer 	= nPixelsY;
		imageSize_Streamer 	= imageSize;
	};

	/**
	 * Print all variables
	 */
	void Print() const {
		GeneralData::Print();
		FILE_LOG(logINFO) << "Bytes Per Adc: " << bytesPerAdc;
	}
};


class Moench03Data : public GeneralData {

 public:

	/** Constructor */
	Moench03Data(){
		myDetectorType		= slsReceiverDefs::MOENCH;
		nPixelsX 			= 400;
		nPixelsY 			= 400;
		headerSizeinPacket  = 22;
		dataSize 			= 8192;
		packetSize 			= headerSizeinPacket + dataSize;
		packetsPerFrame 	= 40;
		imageSize 			= dataSize*packetsPerFrame;
		frameIndexMask 		= 0xFFFFFFFF;
		frameIndexOffset 	= (6+8);
		packetIndexMask 	= 0xFFFFFFFF;
		maxFramesPerFile 	= JFRAU_MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsReceiverDefs::sls_detector_header);
		defaultFifoDepth 	= 2500;
		nPixelsX_Streamer 	= nPixelsX;
		nPixelsY_Streamer 	= nPixelsY;
		imageSize_Streamer 	= imageSize;
	};
};


class JCTBData : public GeneralData {

 public:

	/** Bytes Per Adc */
	const static uint32_t bytesPerAdc = 2;

	/** Constructor */
	JCTBData(){
		myDetectorType		= slsReceiverDefs::JUNGFRAUCTB;
		nPixelsX 			= 32;
		nPixelsY 			= 128;
		headerSizeinPacket  = 22;
		dataSize 			= 8192;
		packetSize 			= headerSizeinPacket + dataSize;
		packetsPerFrame 	= 1;
		imageSize 			= dataSize*packetsPerFrame;
		maxFramesPerFile 	= JFCTB_MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsReceiverDefs::sls_detector_header);
		defaultFifoDepth 	= 2500;
		nPixelsX_Streamer 	= nPixelsX;
		nPixelsY_Streamer 	= nPixelsY;
		imageSize_Streamer 	= imageSize;
	};

	/**
	 * Print all variables
	 */
	void Print() const {
		GeneralData::Print();
		FILE_LOG(logINFO) << "Bytes Per Adc: " << bytesPerAdc;
	}
};


class JungfrauData : public GeneralData {

 public:

	/** Constructor */
	JungfrauData(){
		myDetectorType		= slsReceiverDefs::JUNGFRAU;
		nPixelsX 			= (256*4);
		nPixelsY 			= 256;
		emptyHeader			= 6;
		headerSizeinPacket  = emptyHeader + sizeof(slsReceiverDefs::sls_detector_header);
		dataSize 			= 8192;
		packetSize 			= headerSizeinPacket + dataSize;
		packetsPerFrame 	= 128;
		imageSize 			= dataSize*packetsPerFrame;
		maxFramesPerFile 	= JFRAU_MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsReceiverDefs::sls_detector_header);
		defaultFifoDepth 	= 2500;
		nPixelsX_Streamer 	= nPixelsX;
		nPixelsY_Streamer 	= nPixelsY;
		imageSize_Streamer 	= imageSize;

	};

};


class EigerData : public GeneralData {

 public:

	/** Constructor */
	EigerData(){
		myDetectorType		= slsReceiverDefs::EIGER;
		nPixelsX 			= (256*2);
		nPixelsY 			= 256;
		headerSizeinPacket  = sizeof(slsReceiverDefs::sls_detector_header);
		dataSize 			= 1024;
		packetSize 			= headerSizeinPacket + dataSize;
		packetsPerFrame 	= 256;
		imageSize 			= dataSize*packetsPerFrame;
		maxFramesPerFile 	= EIGER_MAX_FRAMES_PER_FILE;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + sizeof(slsReceiverDefs::sls_detector_header);
		defaultFifoDepth 	= 100;
		threadsPerReceiver	= 2;
		headerPacketSize	= 40;
		nPixelsX_Streamer 	= nPixelsX;
		nPixelsY_Streamer 	= nPixelsY;
		imageSize_Streamer 	= imageSize;
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

