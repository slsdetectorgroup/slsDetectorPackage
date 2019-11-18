#pragma once
/************************************************
 * @file BinaryFileStatic.h
 * @short creating, closing, writing and reading
 * from binary files
 ***********************************************/
/**
 *@short creating, closing, writing and reading from binary files
 */


#include "ansi.h"
#include "logger.h"

#include <string>
#include <iomanip>
#include <string.h>

#define MAX_MASTER_FILE_LENGTH 2000


class BinaryFileStatic {
	
 public:

	/** Constructor */
	BinaryFileStatic(){};
	/** Destructor */
	virtual ~BinaryFileStatic(){};

	/**
	 * Create File Name in format fpath/fnameprefix_fx_dy_z.raw,
	 * where x is fnum, y is (dindex * numunits + unitindex) and z is findex
	 * @param fpath file path
	 * @param fnameprefix file name prefix 
	 * @param findex file index
	 * @param subfindex sub file index
	 * @param dindex readout index
	 * @param numunits number of units per readout. eg. eiger has 2 udp units per readout
	 * @param unitindex unit index
	 * @returns complete file name created
	 */
	static std::string CreateFileName(std::string fpath, std::string fprefix,
										uint64_t findex, uint64_t subfindex,
										int dindex, int numunits = 1,
										int unitindex = 0) {
		std::ostringstream os;
		os << fpath << "/" << fprefix << "_d"
			<< (dindex * numunits + unitindex) << "_f" << subfindex << '_'
			<< findex << ".raw";
		return os.str();
	}

        /**
	 * Create file names for master file
	 * @param fpath file path
	 * @param fnameprefix file name prefix 
	 * @param findex file index
	 * @returns master file name
	 */
	static std::string CreateMasterFileName(std::string fpath, std::string fnameprefix,
											uint64_t findex) {
		std::ostringstream os;
		os << fpath << "/" << fnameprefix << "_master"
			<< "_" << findex << ".raw";
		return os.str();
	}

        /**
	 * Close File
	 * @param fd file pointer
	 */
	static void CloseDataFile(FILE*& fd)
	{
		if (fd)
			fclose(fd);
		fd = 0;
	}

	/**
	 * Write data to file
	 * @param fd file pointer
	 * @param buf buffer to write from
	 * @param bsize size of buffer
	 * @returns number of elements written
	 */
	static int WriteDataFile(FILE* fd, char* buf, int bsize)
	{
		if (!fd)
			return 0;
		return fwrite(buf, 1, bsize, fd);
	}


	/**
	 * Create master files
	 * @param fd pointer to file handle
	 * @param fname master file name
	 * @param owenable overwrite enable
	 * @param attr master file attributes
	 * @returns 0 for success and 1 for fail
	 */
	static int CreateMasterDataFile(FILE*& fd, std::string fname, bool owenable,
					masterAttributes& attr)
	{
		if(!owenable){
			if (NULL == (fd = fopen((const char *) fname.c_str(), "wx"))){
				FILE_LOG(logERROR) << "Could not create binary master file "
						"(without overwrite enable) " << fname;
				fd = 0;
				return 1;
			}
		}else if (NULL == (fd = fopen((const char *) fname.c_str(), "w"))){
			FILE_LOG(logERROR) << "Could not create binary master file "
					"(with overwrite enable) " << fname;
			fd = 0;
			return 1;
		}
		time_t t = time(0);
		char message[MAX_MASTER_FILE_LENGTH];
		sprintf(message,
				"Version                    : %.1f\n"
				"Detector Type              : %d\n"				
				"Dynamic Range              : %d\n"
				"Ten Giga                   : %d\n"
				"Image Size                 : %d bytes\n"
				"nPixelsX                   : %d pixels\n"
				"nPixelsY                   : %d pixels\n"
				"Max Frames Per File        : %u\n"
				"Total Frames               : %lld\n"
				"Exptime (ns)               : %lld\n"
				"SubExptime (ns)            : %lld\n"
				"SubPeriod(ns)              : %lld\n"
				"Period (ns)                : %lld\n"
				"Gap Pixels Enable          : %d\n"
				"Quad Enable                : %d\n"
				"Analog Flag                : %d\n"
				"Digital Flag               : %d\n"
				"ADC Mask                   : %d\n"
				"Dbit Offset                : %d\n"
				"Dbit Bitset                : %lld\n"
				"Roi (xmin, xmax)           : %d %d\n"
				"Timestamp                  : %s\n\n"

				"#Frame Header\n"
				"Frame Number               : 8 bytes\n"
				"SubFrame Number/ExpLength  : 4 bytes\n"
				"Packet Number              : 4 bytes\n"
				"Bunch ID                   : 8 bytes\n"
				"Timestamp                  : 8 bytes\n"
				"Module Id                  : 2 bytes\n"
				"Row                        : 2 bytes\n"
				"Column                     : 2 bytes\n"
				"Reserved                   : 2 bytes\n"
				"Debug                      : 4 bytes\n"
				"Round Robin Number         : 2 bytes\n"
				"Detector Type              : 1 byte\n"
				"Header Version             : 1 byte\n"
				"Packets Caught Mask        : 64 bytes\n"
				,
				attr.version,
				attr.detectorType,
				attr.dynamicRange,
				attr.tenGiga,
				attr.imageSize,
				attr.nPixelsX,
				attr.nPixelsY,
				attr.maxFramesPerFile,
				(long long int)attr.totalFrames,
				(long long int)attr.exptimeNs,
				(long long int)attr.subExptimeNs,
				(long long int)attr.subPeriodNs,
				(long long int)attr.periodNs,
				attr.gapPixelsEnable,
				attr.quadEnable,
    			attr.analogFlag,
   	 			attr.digitalFlag,
    			attr.adcmask,
    			attr.dbitoffset,
    			(long long int)attr.dbitlist,
				attr.roiXmin,
				attr.roiXmax,
				ctime(&t));
		if (strlen(message) > MAX_MASTER_FILE_LENGTH) {
			FILE_LOG(logERROR) << "Master File Size " << strlen(message) <<
					" is greater than max str size " << MAX_MASTER_FILE_LENGTH;
			return 1;
		}

		if (fwrite((void*)message, 1, strlen(message), fd) !=  strlen(message))
			return 1;

		BinaryFileStatic::CloseDataFile(fd);
		return 0;
	}


	/**
	 * Create File
	 * @param fd file pointer
	 * @param owenable overwrite enable
	 * @param fname complete file name
	 * @param filebuffersize file buffer size
	 * @returns 0 for success and 1 for fail
	 */
	static int CreateDataFile(FILE*& fd, bool owenable, std::string fname, size_t filebuffersize)
	 {
		if(!owenable){
			if (NULL == (fd = fopen((const char *) fname.c_str(), "wx"))){
				FILE_LOG(logERROR) << "Could not create/overwrite file" << fname;
				fd = 0;
				return 1;
			}
		}else if (NULL == (fd = fopen((const char *) fname.c_str(), "w"))){
			FILE_LOG(logERROR) << "Could not create file" << fname;
			fd = 0;
			return 1;
		}
		//setting file buffer size to 16mb
		setvbuf(fd,NULL,_IOFBF,filebuffersize);
		return 0;
	}

};

