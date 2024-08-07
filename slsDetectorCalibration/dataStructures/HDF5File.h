#pragma once
/************************************************
 * @file HDF5Fle.h
 * @short functions to open/close/read HDF5 File
 ***********************************************/
/**
 *@short functions to open/close/read HDF5 File
 */


#include "hdf5.h"
#include "hdf5_hl.h"


#define MAX_STR_LENGTH	1000

#define RANK	3                       // Dimension of the image
#define DEFAULT_Z_DIMS	10000
#define DEFAULT_Y_DIMS	1024
#define DEFAULT_X_DIMS	512
//#define DEFAULT_S_DIMS	1       // Storage cells

#define DEFAULT_CHUNK_Z_DIMS	1
#define DEFAULT_CHUNK_Y_DIMS	1024
#define DEFAULT_CHUNK_X_DIMS	512
//#define DEFAULT_CHUNK_S_DIMS	1

/** Assuming each chunk is one image 1024 x 512*/

#define DATA_DATASETNAME	"/data/JF18T01V01/data" //Furka JF
#define INDEX_DATASETNAME	"/data/JF18T01V01/frame_index"

enum{Z,X,Y}; //S is the storage cell

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

	/**
	 * Initialize Parameters for each new file
	 */
	void InitializeParameters ();

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
	 * Read an image into current_image
	 * @returns frame number read,
	 */
	int ReadImage (uint16_t** image, int& iFrame);

	/**
	 * Print current image in memory
	 */
	void PrintCurrentImage ();

private:

	/** file name */
	std::string file_name{};

	/** file handle */
	hid_t file{};

	/** dataspace handle */
	hid_t dataspace{};

	/** memory space handle */
	//hid_t memspace;

	/** dataset handle */
	hid_t dataset{};


	/** file dimensions */
	hsize_t file_dims[RANK]{}; //static array (dimensions are known) //I think the {} initialization should work...

	/** chunk dimensions */
	hsize_t chunk_dims[RANK]{};

	/** number of frames */
	unsigned int number_of_frames{};

	/** frame index list */
	unsigned int* frame_index_list{NULL}; //dynamic array


	/** current image */
	uint16_t* current_image{NULL}; //dynamic array
	//uint16_t current_chunk[DEFAULT_CHUNK_Z_DIMS][DEFAULT_CHUNK_Y_DIMS][DEFAULT_CHUNK_X_DIMS];

	/** current frame offset */
	hsize_t frame_offset[RANK]{}; //array (frame_offset[Z], 0, 0) I believe

};
