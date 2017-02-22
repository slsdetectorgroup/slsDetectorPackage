#pragma once
/************************************************
 * @file GeneralData.h
 * @short abstract for setting/getting properties of detector data
 ***********************************************/
/**
 *@short abstract for setting/getting properties of detector data
 */

#include "receiver_defs.h"


class GeneralData {
	
public:

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

	/** Data size that is saved into the fifo buffer at a time*/
	uint32_t fifoBufferSize;

	/** Header size of data saved into fifo buffer at a time*/
	uint32_t fifoBufferHeaderSize;

	/** Default Fifo depth */
	uint32_t defaultFifoDepth;

	/** Threads per receiver */
	uint32_t threadsPerReceiver;

	/** Size of a header packet */
	uint32_t headerPacketSize;

	/** Cosntructor */
	GeneralData():
		packetIndexMask(0),
		packetIndexOffset(0),
		threadsPerReceiver(1),
		headerPacketSize(0){};

	/** Destructor */
	virtual ~GeneralData(){};

	/**
	 * Get Header Infomation (frame number, packet number)
	 * @param index thread index for debugging purposes
	 * @param packetData pointer to data
	 * @param frameNumber frame number
	 * @param packetNumber packet number
	 */
	virtual void GetHeaderInfo(int index, char* packetData,	uint64_t& frameNumber, uint32_t& packetNumber) const {
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
			uint64_t& frameNumber, uint32_t& packetNumber, uint32_t& subFrameNumber, uint64_t bunchId) const {
		cprintf(RED,"This is a generic function that should be overloaded by a derived class\n");
	}

	/**
	 * Setting dynamic range changes member variables
	 * @param dr dynamic range
	 * @param tgEnable true if 10GbE is enabled, else false
	 */
	virtual void SetDynamicRange(int dr, bool tgEnable) {
		cprintf(RED,"This is a generic function that should be overloaded by a derived class\n");
	};

	/**
	 * Setting ten giga enable changes member variables
	 * @param tgEnable true if 10GbE is enabled, else false
	 * @param dr dynamic range
	 */
	virtual void SetTenGigaEnable(bool tgEnable, int dr) {
		cprintf(RED,"This is a generic function that should be overloaded by a derived class\n");
	};

	/**
	 * Print all variables
	 */
	virtual void Print() const {
		printf("\n\nDetector Data Variables:\n");
		printf(	"Pixels X: %d\n"
				"Pixels Y: %d\n"
				"Header Size in Packet: %d\n"
				"Data Size: %d\n"
				"Packet Size: %d\n"
				"Packets per Frame: %d\n"
				"Image Size: %d\n"
				"Frame Index Mask: 0x%llx\n"
				"Frame Index Offset: %d\n"
				"Packet Index Mask: 0x%x\n"
				"Packet Index Offset: %d\n"
				"Max Frames Per File: %d\n"
				"Fifo Buffer Size: %d\n"
				"Fifo Buffer Header Size: %d\n"
				"Default Fifo Depth: %d\n"
				"Threads Per Receiver: %d\n"
				"Header Packet Size: %d\n",
				nPixelsX,
				nPixelsY,
				headerSizeinPacket,
				dataSize,
				packetSize,
				packetsPerFrame,
				imageSize,
				(long long int)frameIndexMask,
				frameIndexOffset,
				packetIndexMask,
				packetIndexOffset,
				maxFramesPerFile,
				fifoBufferSize,
				fifoBufferHeaderSize,
				defaultFifoDepth,
				threadsPerReceiver,
				headerPacketSize);
	};
};


class GotthardData : public GeneralData {

 public:

	/** Constructor */
	GotthardData(){
		nPixelsX 			= 1280;
		nPixelsY 			= 1;
		headerSizeinPacket  = 4;
		dataSize 			= 1280;
		packetSize 			= 1286;
		packetsPerFrame 	= 2;
		imageSize 			= dataSize*packetsPerFrame;
		frameIndexMask 		= 0xFFFFFFFE;
		frameIndexOffset 	= 1;
		packetIndexMask 	= 1;
		maxFramesPerFile 	= MAX_FRAMES_PER_FILE;
		fifoBufferSize		= imageSize;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES;
		defaultFifoDepth 	= 25000;
	};
};


class ShortGotthardData : public GeneralData {

 public:

	/** Constructor */
	ShortGotthardData(){
		nPixelsX 			= 256;
		nPixelsY 			= 1;
		headerSizeinPacket  = 4;
		dataSize 			= 512;
		packetSize 			= 518;
		packetsPerFrame 	= 1;
		imageSize 			= dataSize*packetsPerFrame;
		frameIndexMask 		= 0xFFFFFFFF;
		maxFramesPerFile 	= SHORT_MAX_FRAMES_PER_FILE;
		fifoBufferSize		= imageSize;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES;
		defaultFifoDepth 	= 25000;
	};
};


class PropixData : public GeneralData {

 private:

	/**bytes per pixel for calculating image size */
	const static uint32_t bytesPerPixel = 2;

 public:

	/** Constructor */
	PropixData(){
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
		fifoBufferSize		= imageSize;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES;
		defaultFifoDepth 	= 25000;
	};
};


class Moench02Data : public GeneralData {

 public:

	/** Bytes Per Adc */
	const static uint32_t bytesPerAdc = (40*2);

	/** Constructor */
	Moench02Data(){
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
		fifoBufferSize		= imageSize;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + FILE_FRAME_HEADER_SIZE;
		defaultFifoDepth 	= 2500;
	};

	/**
	 * Print all variables
	 */
	void Print() const {
		GeneralData::Print();
		printf("Bytes Per Adc: %d\n",bytesPerAdc);
	}
};


class Moench03Data : public GeneralData {

 public:

	/** Constructor */
	Moench03Data(){
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
		fifoBufferSize		= imageSize;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + FILE_FRAME_HEADER_SIZE;
		defaultFifoDepth 	= 2500;
	};
};


class JCTBData : public GeneralData {

 public:

	/** Bytes Per Adc */
	const static uint32_t bytesPerAdc = 2;

	/** Constructor */
	JCTBData(){
		nPixelsX 			= 32;
		nPixelsY 			= 128;
		headerSizeinPacket  = 22;
		dataSize 			= 8192;
		packetSize 			= headerSizeinPacket + dataSize;
		packetsPerFrame 	= 1;
		imageSize 			= dataSize*packetsPerFrame;
		maxFramesPerFile 	= JFCTB_MAX_FRAMES_PER_FILE;
		fifoBufferSize		= imageSize;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + FILE_FRAME_HEADER_SIZE;
		defaultFifoDepth 	= 2500;
	};

	/**
	 * Print all variables
	 */
	void Print() const {
		GeneralData::Print();
		printf("Bytes Per Adc: %d\n",bytesPerAdc);
	}
};


class JungfrauData : public GeneralData {

private:

	/** Structure of an jungfrau packet header */
	typedef struct {
		unsigned char emptyHeader[6];
		unsigned char reserved[4];
		unsigned char packetNumber[1];
		unsigned char frameNumber[3];
		unsigned char bunchid[8];
	} jfrau_packet_header_t;

 public:

	/** Size of packet header */
	const static uint32_t packetHeaderSize	= 22;

	/** Constructor */
	JungfrauData(){
		nPixelsX 			= (256*4);
		nPixelsY 			= 256;
		headerSizeinPacket  = 22;
		dataSize 			= 8192;
		packetSize 			= headerSizeinPacket + dataSize;
		packetsPerFrame 	= 128;
		imageSize 			= dataSize*packetsPerFrame;
		maxFramesPerFile 	= JFRAU_MAX_FRAMES_PER_FILE;
		fifoBufferSize		= imageSize;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + FILE_FRAME_HEADER_SIZE;
		defaultFifoDepth 	= 2500;
	};

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
	void GetHeaderInfo(int index, char* packetData, uint32_t dynamicRange,
			uint64_t& frameNumber, uint32_t& packetNumber, uint32_t& subFrameNumber, uint64_t bunchId) const {
		subFrameNumber = 0;
		jfrau_packet_header_t* header = (jfrau_packet_header_t*)(packetData);
		frameNumber = (uint64_t)(*( (uint32_t*) header->frameNumber));
		packetNumber = (uint32_t)(*( (uint8_t*) header->packetNumber));
		bunchId = (*((uint64_t*) header->bunchid));
	}

	/**
	 * Print all variables
	 */
	void Print() const {
		GeneralData::Print();
		printf("Packet Header Size: %d\n",packetHeaderSize);
	}
};


class EigerData : public GeneralData {

private:
	/** Structure of an eiger packet header	 */
	typedef struct {
		unsigned char subFrameNumber[4];
		unsigned char missingPacket[2];
		unsigned char portIndex[1];
		unsigned char dynamicRange[1];
	} eiger_packet_header_t;

	/** Structure of an eiger packet footer  */
	typedef struct	{
		unsigned char frameNumber[6];
		unsigned char packetNumber[2];
	} eiger_packet_footer_t;

 public:

	/** Size of packet header */
	const static uint32_t packetHeaderSize	= 8;

	/** Footer offset */
	uint32_t footerOffset;

	/** Constructor */
	EigerData(){
		nPixelsX 			= (256*2);
		nPixelsY 			= 256;
		headerSizeinPacket  = 8;
		dataSize 			= 1024;
		packetSize 			= headerSizeinPacket + dataSize + 8;
		packetsPerFrame 	= 256;
		imageSize 			= dataSize*packetsPerFrame;
		frameIndexMask 		= 0xffffff;
		maxFramesPerFile 	= 5;//EIGER_MAX_FRAMES_PER_FILE;
		fifoBufferSize		= imageSize;
		fifoBufferHeaderSize= FIFO_HEADER_NUMBYTES + FILE_FRAME_HEADER_SIZE;
		defaultFifoDepth 	= 100;
		footerOffset		= headerSizeinPacket + dataSize;
		threadsPerReceiver	= 2;
		headerPacketSize	= 48;
	};

	/**
	 * Get Header Infomation (frame number, packet number)
	 * @param index thread index for debugging purposes
	 * @param packetData pointer to data
	 * @param frameNumber frame number
	 * @param packetNumber packet number
	 */
	void GetHeaderInfo(int index, char* packetData,	uint64_t& frameNumber, uint32_t& packetNumber) const {
		eiger_packet_footer_t* footer = (eiger_packet_footer_t*)(packetData + footerOffset);
		frameNumber = (uint64_t)((*( (uint64_t*) footer)) & frameIndexMask);
		packetNumber = (uint32_t)(*( (uint16_t*) footer->packetNumber))-1;
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
	void GetHeaderInfo(int index, char* packetData, uint32_t dynamicRange,
			uint64_t& frameNumber, uint32_t& packetNumber, uint32_t& subFrameNumber, uint64_t bunchId) const {
		bunchId = 0;
		subFrameNumber = 0;
		eiger_packet_footer_t* footer = (eiger_packet_footer_t*)(packetData + footerOffset);
		frameNumber = (uint64_t)((*( (uint64_t*) footer)) & frameIndexMask);
		packetNumber = (uint32_t)(*( (uint16_t*) footer->packetNumber))-1;
		if (dynamicRange == 32) {
			eiger_packet_header_t* header = (eiger_packet_header_t*) (packetData);
			subFrameNumber = (uint64_t) *( (uint32_t*) header->subFrameNumber);
		}
	}

	/**
	 * Setting dynamic range changes member variables
	 * @param dr dynamic range
	 * @param tgEnable true if 10GbE is enabled, else false
	 */
	void SetDynamicRange(int dr, bool tgEnable) {
		packetsPerFrame = (tgEnable ? 4 : 16) * dr;
		imageSize 		= dataSize*packetsPerFrame;
		fifoBufferSize	= packetSize*packetsPerFrame;
	}

	/**
	 * Setting ten giga enable changes member variables
	 * @param tgEnable true if 10GbE is enabled, else false
	 * @param dr dynamic range
	 */
	void SetTenGigaEnable(bool tgEnable, int dr) {
		dataSize 		= (tgEnable ? 4096 : 1024);
		packetSize 		= (tgEnable ? 4112 : 1040);;
		packetsPerFrame = (tgEnable ? 4 : 16) * dr;
		imageSize 		= dataSize*packetsPerFrame;
		fifoBufferSize	= packetSize*packetsPerFrame;
		footerOffset	= packetHeaderSize+dataSize;
	};

	/**
	 * Print all variables
	 */
	void Print() const {
		GeneralData::Print();
		printf(  "Packet Header Size: %d\n"
				"Footer Offset : %d\n",
				packetHeaderSize,
				footerOffset);
	}
};

