#pragma once
/************************************************
 * @file HDF5Fle.h
 * @short functions to open/close/read HDF5 File
 * Adapted for generalization, accepts rank 3 and 4
 * Supports control over storage cells
 ***********************************************/
/**
 *@short functions to open/close/read HDF5 File
 */


#include <iostream>
#include <vector>
#include <string>

#include "hdf5.h"
#include "hdf5_hl.h"


//#define MAX_STR_LENGTH	1000

#define RANK	4                       // Dimension of the image dataset, only for validation
#define DEFAULT_Z_DIMS	10000			// only for validation
#define DEFAULT_Y_DIMS	1024			// only for validation
#define DEFAULT_X_DIMS	512				// only for validation
//#define DEFAULT_S_DIMS	1       	// Storage cells

#define DEFAULT_CHUNK_Z_DIMS	100		// only for validation
#define DEFAULT_CHUNK_Y_DIMS	1024	// only for validation
#define DEFAULT_CHUNK_X_DIMS	512		// only for validation
//#define DEFAULT_CHUNK_S_DIMS	1


#define DATA_DATASETNAME	"/data/JF18T01V01/data" //Furka JF
#define INDEX_DATASETNAME	"/data/JF18T01V01/frame_index"

//enum{Z,S,X,Y}; //S is the storage cell //enum is not used

class HDF5File {

public:

	/**
	 * Constructor
	 */
	HDF5File ();

	/**
	 * Destructor
	 */
	~HDF5File ();


	std::vector<hsize_t> GetDatasetDimensions ();

	std::vector<hsize_t> GetChunkDimensions ();

	void SetImageDataPath (std::string const& name);

	void SetFrameIndexPath (std::string const& name);



	/**
	 * Open HDF5 file and dataset,
	 * reads frame index dataset to array
	 * @param fname file name
	 * @param validate true if one must validate if file is
	 * chunked with dims [? x 128 x 512] and chunk dims [1 x 128 x 512]
	 * @returns 1 if successful, else 0 if fail
	 */
	int OpenResources (const char* const fname, bool validate); 

	/**
	 * Close Open resources
	 */
	void CloseResources ();

	/**
	 * Read an image into current_image,
	 * increment Z-offset (frame) and (if rank==4) storage cell
	 * @returns frame number read,
	 */
	int ReadImage (uint16_t* image, std::vector<hsize_t>& offset);

	/**
	 * Print current image in memory
	 */
	void PrintCurrentImage (uint16_t* image);

private:

	/**
	 * Initialize dimensions of image dataset for each new file
	 */
	void InitializeDimensions ();

	bool ReadChunkDimensions ();

	bool ValidateDimensions ();

	bool ValidateChunkDimensions ();

	/**
	 * Open dataset containing the frame numbers
	 */
	bool OpenFrameIndexDataset ();


	/** file name */
	std::string file_name{};
	/** dataset name for image data */
	std::string data_datasetname = DATA_DATASETNAME;
	/** dataset name for frame index data */
	std::string index_datasetname = INDEX_DATASETNAME;

	/** file handle */
	hid_t file{};
	/** dataspace handle */
	hid_t dataspace{};
	/** memory space handle */
	//hid_t memspace; //old
	/** dataset handle */
	hid_t dataset{};

	/** file dimensions */
	std::vector<hsize_t> file_dims{};
	//hsize_t file_dims[RANK]{}; //static array (dimensions are known)

	/** chunk dimensions 
	 ** not necessarily required
	 ** useful for optimization or validation */
	std::vector<hsize_t> chunk_dims{};
	//hsize_t chunk_dims[RANK]{};

	/** Rank of the image dataset */
	hsize_t rank{};

	/** number of frames */
	unsigned int number_of_frames{};

	/** frame index list */
	std::vector<hsize_t> frame_index_list{};

	/** Current image
	 ** dynamic array 
	 ** uint16_t pointer format is chosen to support use with slsDetectorCalibration cluster finder */
	uint16_t* current_image{nullptr};
	//uint16_t current_chunk[DEFAULT_CHUNK_Z_DIMS][DEFAULT_CHUNK_Y_DIMS][DEFAULT_CHUNK_X_DIMS];

	/** Current frame offset
	 ** (Z-offset, S-offset, 0, 0) or (Z-offset, 0, 0), increments automatically with ReadImage */
	std::vector<hsize_t> frame_offset{};
	//hsize_t frame_offset[RANK]{};

};
