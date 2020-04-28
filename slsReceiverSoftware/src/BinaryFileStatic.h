#pragma once
/************************************************
 * @file BinaryFileStatic.h
 * @short creating, closing, writing and reading
 * from binary files
 ***********************************************/
/**
 *@short creating, closing, writing and reading from binary files
 */


#include "logger.h"

#include <string>
#include <iomanip>
#include <string.h>



class BinaryFileStatic {
	
 public:



	/**
	 * Create master files
	 * @param fd pointer to file handle
	 * @param fname master file name
	 * @param owenable overwrite enable
	 * @param attr master file attributes
	 */
	static void CreateMasterDataFile(FILE*& fd, std::string fname, bool owenable,
					masterAttributes& attr)
	{
		if(!owenable){
			if (NULL == (fd = fopen((const char *) fname.c_str(), "wx"))){
				fd = 0;
				throw sls::RuntimeError("Could not create binary master file "
						"(without overwrite enable) " + fname);
			}
		}else if (NULL == (fd = fopen((const char *) fname.c_str(), "w"))){
			fd = 0;
				throw sls::RuntimeError("Could not create binary master file "
						"(with overwrite enable) " + fname);
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
			throw sls::RuntimeError("Master File Size " + std::to_string(strlen(message)) +
			" is greater than max str size " + std::to_string(MAX_MASTER_FILE_LENGTH));
		}

		if (fwrite((void*)message, 1, strlen(message), fd) !=  strlen(message)) {
			throw sls::RuntimeError("Master binary file incorrect number of bytes written to file");
		}

		BinaryFileStatic::CloseDataFile(fd);
	}


