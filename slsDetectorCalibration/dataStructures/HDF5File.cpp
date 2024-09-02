#include "HDF5File.h"

#include "ansi.h"

#include <string.h>

HDF5File::HDF5File () {
  //InitializeParameters();
}

HDF5File::~HDF5File () {
	if(frame_index_list)
		delete [] frame_index_list;
	if(current_image)
		delete [] current_image;
}

void HDF5File::InitializeParameters () {
  /*
    memset(file_name, 0, MAX_STR_LENGTH); //awkward, initializes all file_name characters to 0
    file = -1;
    dataspace = -1;
    //memspace = -1;
    dataset = -1;
    number_of_frames = 0;
    frame_index_list = NULL;
    current_image = NULL;
  */
  for (int i = 0; i < RANK; ++i) {
    file_dims[i] = 0; //also awkward
    chunk_dims[i] = 0;
    frame_offset[i] = 0;
  }
}


int HDF5File::OpenResources (char const*const fname, bool validate) {
	// Initialize
	//InitializeParameters();


	// Open File
	file = H5Fopen (fname, H5F_ACC_RDONLY, H5P_DEFAULT);
	if (file < 0) {
	  cprintf(RED,"could not open hdf5 file\n");
	  return 0;
	}
	cprintf(BLUE, "Opened File: %s\n", fname);


	// Open Dataset
	dataset = H5Dopen2 (file, DATA_DATASETNAME, H5P_DEFAULT);
	if (dataset < 0){
	  cprintf(RED,"could not open dataset\n");
	  CloseResources ();
	  return 0;
	}
	cprintf(BLUE, "Opened Dataset: %s\n", DATA_DATASETNAME);
	

	// Create Dataspace
	dataspace = H5Dget_space (dataset);
	if (dataspace < 0){
	  cprintf(RED,"could not open dataspace\n");
	  CloseResources ();
	  return 0;
	}


	// Get Dimensions
	int rank = H5Sget_simple_extent_dims (dataspace, file_dims, NULL);
	cprintf (BLUE, "Number of Images: %llu\n", file_dims[Z]);

	// validate file dimensions
	if (validate) {

		// validate rank
		if(rank != RANK) {
			cprintf(RED,"rank found %d. Expected %d\n", rank, RANK);
			CloseResources ();
			return 0;
		}

		// validate file dimensions of x and y
		if (file_dims[X] != DEFAULT_X_DIMS) {
			cprintf(RED,"file dimensions of x found %llu. Expected %d\n", file_dims[X], DEFAULT_X_DIMS);
			CloseResources ();
			return 0;
		}
		if (file_dims[Y] != DEFAULT_Y_DIMS) {
			cprintf(RED,"file dimensions of y found %llu. Expected %d\n", file_dims[Y], DEFAULT_Y_DIMS);
			CloseResources ();
			return 0;
		}
		cprintf(GREEN, "File rank & dimensions validated. "
				"Rank: %d, Dimensions: %llu x %llu x %llu\n",
					rank, file_dims[Z], file_dims[Y], file_dims[X]);
	}



	// Get layout
	hid_t cparms = H5Dget_create_plist(dataset);

	// validate chunk layout
	if (validate) {
		if (H5D_CHUNKED != H5Pget_layout (cparms))  {
			cprintf(RED,"not chunked data file\n");
			H5Pclose(cparms);
			CloseResources ();
			return 0;
		}
		cprintf(GREEN, "Chunk layout validated\n");
	}

	// Get Chunk Dimensions
	int rank_chunk = H5Pget_chunk (cparms, RANK, chunk_dims);

	// validate dimensions
	if (validate) {

		// validate rank
		if(rank_chunk != RANK) {
			cprintf(RED,"chunk rank found %d. Expected %d\n", rank, RANK);
			H5Pclose(cparms);
			CloseResources ();
			return 0;
		}

		// validate file dimensions of x, y and z
		if (chunk_dims[X] != DEFAULT_CHUNK_X_DIMS) {
			cprintf(RED,"chunk dimensions of x found %llu. Expected %d\n", chunk_dims[X], DEFAULT_CHUNK_X_DIMS);
			H5Pclose(cparms);
			CloseResources ();
			return 0;
		}
		if (chunk_dims[Y] != DEFAULT_CHUNK_Y_DIMS) {
			cprintf(RED,"chunk dimensions of y found %llu. Expected %d\n", chunk_dims[Y], DEFAULT_CHUNK_Y_DIMS);
			H5Pclose(cparms);
			CloseResources ();
			return 0;
		}
		/*if (chunk_dims[Z] != DEFAULT_CHUNK_Z_DIMS) {
			cprintf(RED,"chunk dimensions of z found %llu. Expected %d\n", chunk_dims[Z], DEFAULT_CHUNK_Z_DIMS);
			H5Pclose(cparms);
			CloseResources ();
			return 0;
		}*/

		cprintf(GREEN, "Chunk rank & dimensions validated. "
					"Rank: %d, Dimensions: %llu x %llu x %llu\n",
					rank_chunk, chunk_dims[Z], chunk_dims[Y], chunk_dims[X]);

	}

	H5Pclose (cparms);

	// allocate chunk memory
	//current_image = new uint16_t[chunk_dims[Z]*DEFAULT_CHUNK_Y_DIMS*DEFAULT_CHUNK_X_DIMS];
	//current_image = new uint16_t[DEFAULT_X_DIMS*DEFAULT_Y_DIMS];

	// Define memory space
	//memspace = H5Screate_simple (RANK, chunk_dims, NULL);


	// Get all the frame numbers
	// Open frame index dataset
	hid_t fi_dataset = H5Dopen2 (file, INDEX_DATASETNAME, H5P_DEFAULT);
	if (fi_dataset < 0){
		cprintf (RED,"could not open frame index dataset %s\n", INDEX_DATASETNAME);
		CloseResources ();
		return 0;
	}

	// validate size of frame index dataset
	if (validate) {
		hsize_t fi_dims[2];
		hid_t fi_dataspace = H5Dget_space (fi_dataset);
		int fi_rank = H5Sget_simple_extent_dims (fi_dataspace, fi_dims, NULL);

		// validate rank
		if(fi_rank != 2) {
			cprintf(RED,"Frame index dataset rank found %d. Expected %d\n", fi_rank, 2);
			H5Sclose (fi_dataspace);
			H5Dclose (fi_dataset);
			CloseResources ();
			return 0;
		}

		// validate size
		if (fi_dims[Z] != file_dims[Z]) {
			cprintf (RED,"Frame index dimensions of z found %llu. Expected %llu\n", fi_dims[Z], file_dims[Z]);
			H5Sclose (fi_dataspace);
			H5Dclose (fi_dataset);
			CloseResources ();
			return 0;
		}
		H5Sclose (fi_dataspace);
	}


	// allocate frame index memory
	frame_index_list = new unsigned int[file_dims[Z]];

	//read frame index values
	//Is u32 correct? I would think not. But I get a segmentation fault if I use u64.
	if (H5Dread (fi_dataset, H5T_STD_U32LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, frame_index_list) < 0) {
		cprintf (RED,"Could not read frame index dataset %s\n", INDEX_DATASETNAME);
		H5Dclose (fi_dataset);
		CloseResources ();
	}
	H5Dclose(fi_dataset);

	return 1;
}


void HDF5File::CloseResources () {
	if (dataspace >=0 ) 	{
		H5Sclose(dataspace);
		dataspace = -1;
	}
	if (dataset >=0 ) 	{
		H5Dclose(dataset);
		dataset = -1;
	}
	if (file >=0 ) 		{
		H5Fclose(file);
		file = -1;
	}
	//if (memspace >= 0)      H5Sclose(memspace); // VH: I am getting memory leaks after recompilation
}

/*
 * Function takes uint16_t* argument to make explicit that the caller has to handle memory allocation and deallocation.
 * This is legacy caused by the structure with which the slsDetectorCalibration cluster finder is written.
 * Best practice for modern C++ would be to rewrite using smart pointers.
 */
int HDF5File::ReadImage (uint16_t* image, int& iFrame) {
  /*
   * Originially, this function took uint16_t** but this may lead to memory management issues since image gets redirected
   * to point to current_image, which is owned by HDF5File.
   * (Usually, this would be good practice and classic C-style.)
   */

	// no images in this frame
	if (frame_index_list[frame_offset[Z]] == 0) {
		cprintf (RED,"No images in this frame offset %llu\n", frame_offset[Z]);
		CloseResources ();
		return -99;
	}

	if (frame_offset[Z] == file_dims[Z]-1) {
		printf("end of file\n");
		return -1;
	}

	hsize_t frame_size[RANK] = {1, file_dims[X], file_dims[Y]};

	// Define memory space
	hid_t memspace = H5Screate_simple (RANK, frame_size, NULL);

	// create hyperslab
	// This aligns dataspace such that we read the correct frame
	if (H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, frame_offset, NULL, frame_size, NULL) < 0 ) {
		cprintf (RED,"Could not create hyperslab for frame count %llu\n", frame_offset[Z]);
		CloseResources ();
		return -99;
	}

	// read dataset into current_image
	if (H5Dread(dataset, H5T_STD_U16LE, memspace, dataspace, H5P_DEFAULT, image /*was 'current_image'*/) < 0 ) {
		cprintf (RED,"Could not read dataset for frame count %llu\n", frame_offset[Z]);
		CloseResources ();
		return -99;
	}
	//*image = current_image; //if uint16_t** is passed, HDF5File owns the resource image points to, which is potentially dangerous

	// return frame number and then increment frame count number
	unsigned int retval = frame_index_list[frame_offset[Z]];
	iFrame = (int)frame_offset[Z];
	++frame_offset[Z];
	return retval;
}



void HDF5File::PrintCurrentImage () {
	printf("\n");
	printf("Frame %llu, Image: %d\n", frame_offset[Z]-1, frame_index_list[frame_offset[Z]-1]);

	unsigned long long int size = file_dims[Y] * file_dims[X];
	for (unsigned int i = 0; i < size; ++i){
		printf("%u ", current_image[i]);
		if (!((i+1) % file_dims[X] ))
			printf("\n\n");
	}
	printf("\n\n\n\n");
}

