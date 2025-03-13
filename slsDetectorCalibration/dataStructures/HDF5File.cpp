#include "HDF5File.h"

#include "ansi.h"

#include <algorithm>
#include <fmt/ranges.h>

/*
 * No class member helper functions
 */
std::string vectorToString(std::vector<hsize_t> const& v) {
    return fmt::format("({})", fmt::join(v, ", "));
}

/*
 * increment frame offset (if s dimension exists, loop through all before incrementing z) 
 * should also work if file_dims[1] is not s but x (in that case we ignore it)
 */
void conditionalIncrement(std::vector<hsize_t>& vec, hsize_t max_value) {

    if (vec.size() < 3) {
        throw std::invalid_argument("Vector must have at least 3 elements.");
    }

    // If vector has 4 elements, increment vec[1] first
    if (vec.size() == 4) {
        if (++vec[1] >= max_value) { //max_value is never reached!
            vec[1] = 0;  // Reset and increment vec[0]
            ++vec[0];
        }
    }
    // If vector has 3 elements, increment vec[0] directly
    else if (vec.size() == 3) {
        ++vec[0];
    }
}

void printDatatypeSize(hid_t dataset) {

	hid_t datatype = H5Dget_type(dataset);
	H5T_class_t class_id = H5Tget_class(datatype);
	size_t type_size = H5Tget_size(datatype);

	std::cout << " dataset type class: " << class_id 
          	  << ", size: " << type_size << " bytes\n";

}


/* **********************
 * Class member functions
 * **********************
 */

// Default constructor
/*
HDF5File::HDF5File () {
  //InitializeParameters(); //old
}
*/

/*
HDF5File::~HDF5File () {
	
	if(current_image)
		delete [] current_image;
	
}
*/

void HDF5File::SetImageDataPath (std::string const& name) { 
	std::cout << "Image dataset path set to " << name << std::endl;
	data_datasetname = name; 
}

void HDF5File::SetFrameIndexPath (std::string const& name) { 
	std::cout << "Frame index dataset path set to " << name << std::endl;
	index_datasetname = name; 
}

void HDF5File::InitializeDimensions () {

	rank = H5Sget_simple_extent_ndims(dataspace);
    file_dims.resize(rank);
    H5Sget_simple_extent_dims(dataspace, file_dims.data(), nullptr);

    std::cout << "Dataset dimensions: " << vectorToString(file_dims) << "\n";

}

std::vector<hsize_t> HDF5File::GetDatasetDimensions() {
	return file_dims;
}

std::vector<hsize_t> HDF5File::GetChunkDimensions() {
	return chunk_dims;
}

hsize_t HDF5File::GetRank() {
	return rank;
}

bool HDF5File::ValidateDimensions () {

	// validate rank
	if(rank != RANK) {
		cprintf(RED,"rank found %llu. Expected %d\n", rank, RANK);
		std::cerr << "Error: Rank could not be validated\n";
		return false;
	}

	// validate file dimensions of x and y (assuming those are the last two dimensions of the dataset)
	if ( (file_dims[file_dims.size()-2] != DEFAULT_X_DIMS) || (file_dims[file_dims.size()-1] != DEFAULT_Y_DIMS) ) {
		cprintf(RED,"file dimensions of x found %llu. Expected %d\n", file_dims[file_dims.size()-2], DEFAULT_X_DIMS);
		cprintf(RED,"file dimensions of y found %llu. Expected %d\n", file_dims[file_dims.size()-1], DEFAULT_Y_DIMS);
		std::cerr << "Error: Dataset dimensions could not be validated\n";
		return false;
	}

	cprintf(GREEN, "File rank & dimensions validated.");
	return true;	
}

bool HDF5File::ReadChunkDimensions () {

	// Get layout
	hid_t plist_id = H5Dget_create_plist(dataset);

	if (H5Pget_layout (plist_id) != H5D_CHUNKED)  {
		cprintf(RED,"NOTE: Dataset is not chunked!\n");
		std::cerr << "Error: Dataset is not chunked\n";
		return false;
	}

	// Get Chunk Dimensions
	int rank_chunk = H5Pget_chunk (plist_id, 0, nullptr);
	chunk_dims.resize(rank_chunk);
	H5Pget_chunk (plist_id, rank_chunk, chunk_dims.data());

	std::cout << "Chunk dimensions: " << vectorToString(chunk_dims) << "\n";

	H5Pclose (plist_id);

	return true;

}

bool HDF5File::ValidateChunkDimensions () {

	// validate rank
	if(chunk_dims.size() != rank) {
		cprintf(RED,"Chunk rank does not match dataset rank! Found %lu. Expected %llu\n", chunk_dims.size(), rank);
		std::cerr << "Error: Chunk rank does not match dataset rank\n";
		return false;
	}

	// validate chunk dimensions of x and y (assuming those are the last two dimensions of the dataset)
	if ( (chunk_dims[chunk_dims.size()-2] != DEFAULT_CHUNK_X_DIMS) || (chunk_dims[chunk_dims.size()-1] != DEFAULT_CHUNK_Y_DIMS) ) {
		cprintf(RED,"file dimensions of x found %llu. Expected %d\n", chunk_dims[chunk_dims.size()-2], DEFAULT_CHUNK_X_DIMS);
		cprintf(RED,"file dimensions of y found %llu. Expected %d\n", chunk_dims[chunk_dims.size()-1], DEFAULT_CHUNK_Y_DIMS);
		std::cerr << "Error: Chunk dimensions could not be validated\n";
		return false;
	}

	cprintf(GREEN, "Chunk rank & dimensions validated.");
	return true;

}

bool HDF5File::OpenFrameIndexDataset() {

	// Get all the frame numbers
	// Open frame index dataset
	hid_t fi_dataset = H5Dopen2 (file, index_datasetname.c_str(), H5P_DEFAULT);
	if (fi_dataset < 0){
		cprintf (RED,"Could not open frame index dataset %s\n", index_datasetname.c_str());
		std::cerr << "Error: Could not open frame index dataset\n";
		return false;
	}

	hid_t fi_dataspace = H5Dget_space (fi_dataset);
	int fi_rank = H5Sget_simple_extent_ndims(fi_dataspace);
	std::vector<hsize_t> fi_dims(fi_rank);
	H5Sget_simple_extent_dims (fi_dataspace, fi_dims.data(), nullptr);

	std::cout << "Frame index dataset dimensions: " << vectorToString(fi_dims) << "\n";

	// validate size
	if (fi_dims[0] != file_dims[0]) {
		cprintf (RED,"Frame index dimensions of z found %llu. Expected %llu\n", fi_dims[0], file_dims[0]);
		std::cerr << "Error: Z dimension of frame index dataset does not align with z dimension of image dataset\n";
		H5Sclose (fi_dataspace);
		H5Dclose (fi_dataset);
		return false;
	}
	H5Sclose (fi_dataspace);

	// allocate frame index memory
	frame_index_list.resize(fi_dims[0]); //file_dims

	// print datatype size of dataset
	std::cout << "Frame index";
	printDatatypeSize(fi_dataset);

	//read frame index values
	//Is u32 correct? I would think not. But I get a segmentation fault if I use u64.
	if (H5Dread (fi_dataset, H5T_STD_U64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, frame_index_list.data()) < 0) {
		cprintf (RED,"Could not read frame index dataset %s\n", index_datasetname.c_str());
		std::cerr << "Error: Could not read frame index dataset\n";
		H5Dclose (fi_dataset);
		return false;
	}
	H5Dclose(fi_dataset);
	return true;
}

int HDF5File::OpenResources (char const*const fname, bool validate) {

	std::cout << "Debug HDF5File.cpp: Attempting to open file " << fname << std::endl;
	// Open File
	file = H5Fopen (fname, H5F_ACC_RDONLY, H5P_DEFAULT);
	if (file < 0) {
	  cprintf(RED,"Could not open hdf5 file\n");
	  std::cerr << "Error: H5Fopen failed\n";
	  return 0;
	}
	cprintf(BLUE, "Opened File: %s\n", fname);

	// Open Dataset
	dataset = H5Dopen2 (file, data_datasetname.c_str(), H5P_DEFAULT);
	if (dataset < 0){
	  cprintf(RED,"Could not open dataset\n");
	  std::cerr << "Error: H5Dopen2 failed\n";
	  CloseResources ();
	  return 0;
	}
	cprintf(BLUE, "Opened Dataset: %s\n", data_datasetname.c_str());

	// print datatype size of dataset
	std::cout << "Image";
	printDatatypeSize(dataset);

	// Create Dataspace
	dataspace = H5Dget_space (dataset);
	if (dataspace < 0){
	  cprintf(RED,"Could not open dataspace\n");
	  std::cerr << "Error: H5Dget_space failed\n";
	  CloseResources ();
	  return 0;
	}

	// Get Dimensions
	InitializeDimensions();
	// Get chunk dimensions
	if (!ReadChunkDimensions()) {
		CloseResources();
		return 0;
	}

	// validate file dimensions
	if (validate) {
		if ( !ValidateDimensions() || !ValidateChunkDimensions() ) {
			CloseResources();
			return 0;		
		}
	}

	//Read frame indices
	if (!OpenFrameIndexDataset()) {
		CloseResources();
		return 0;
	}
	
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
}

/*
 * Function takes uint16_t* argument to make explicit that the caller has to handle memory allocation and deallocation.
 * This is legacy caused by the structure with which the slsDetectorCalibration cluster finder is written.
 * (Best practice for modern C++ would be using smart pointers.)
 *
 * Originially, this function took uint16_t** which may lead to memory management issues since image gets redirected
 * to point to current_image, which is owned by HDF5File.
 * (Good practice in classic C-style. HDF5File needs to clean up the resource at destruction.)
 * 
 * \param image pointer to uint16_t, buffer which the image is read into. (Memory handled by caller!)
 * \param offset contains iFrame at [0] and storage cell number at [1],
 * depending on dimensionality of the dataset, the storage cell number may not be included.
 * Note that frame number (as read from file) may (likely) differ from frame index (in the dataset)!
 */
int HDF5File::ReadImage (uint16_t* image, std::vector<hsize_t>& offset ) {

	// Validate input arguments
    if (!image) {
        std::cerr << "Error: image buffer is null.\n";
        return -99;
    }
  	if ( offset.size() != rank-2 ) {
		cprintf ( RED,"Offset vector must have size %llu. Found %lu\n", rank-2, offset.size() );
        std::cerr << "Error: Wrong offset vector size\n";
		CloseResources ();
        return -99;
    }

	// Initialize frame_offset
	if (frame_offset.empty())
		frame_offset.resize(rank,0);

	// Check if we reached the end of file
	// Compares that the offsets of frame and storage cell (Z and S) have reached the end of file
	// Excludes X and Y indices (of the image dataset) from the comparison
	if( std::equal( frame_offset.cbegin(), frame_offset.cend()-2, file_dims.cbegin() ) ) {
		printf("End of file reached\n");
		return -1;
	}
	/* //old
	if (frame_offset[0] == file_dims[0]-1) {
		printf("end of file\n");
		return -1;
	}
	*/

	// Validate frame_offset index
    if (frame_offset[0] >= frame_index_list.size()) {
        std::cerr << "Error: frame_offset[0] = " << frame_offset[0] << " of bounds.\n";
        return -99;
    }

	// Check if images exist at the current frame offset
	if (frame_index_list[frame_offset[0]] == 0) {
		cprintf (RED,"No images at this frame offset %llu\n", frame_offset[0]);
		std::cerr << "Error: Framenumber 0 at this frame offset\n";
		CloseResources ();
		return -99;
	}

	// Optional: Ensure dataset and dataspace are valid
    if (dataset < 0) {
        std::cerr << "Error: Invalid dataset ID.\n";
        return -99;
    }
    if (dataspace < 0) {
        std::cerr << "Error: Invalid dataspace.\n";
        return -99;
    }

	// Define the size of the hyperslab to read
	std::vector<hsize_t> frame_size(rank, 1);
	std::copy(file_dims.begin() + rank-2, file_dims.end(), frame_size.begin() + rank-2);
	/*
	for ( int d=0; d < rank; ++d ) {
		if (d < rank-2)
			frame_size[d] = 1;
		if ( d >= rank-2 )
			frame_size[d] = file_dims[d];
	}
	*/

	// Define memory space
	hid_t memspace = H5Screate_simple (rank, frame_size.data(), nullptr);
	if (memspace < 0) {
    	std::cerr << "Error: Failed to create memory space for HDF5 read operation\n";
    	CloseResources();
    	return -99;
	}

	// Create hyperslab selection
	// This aligns dataspace such that we read the correct frame
	if (H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, frame_offset.data(), nullptr, frame_size.data(), nullptr) < 0 ) {
		cprintf (RED,"Could not create hyperslab for frame offset %s\n", vectorToString(frame_offset).c_str());
		std::cerr << "Error: Hyperslab creation failed for frame offset " << vectorToString(frame_offset) << "\n";
		CloseResources();
		H5Sclose(memspace);
		return -99;
	}

	// Read dataset into image buffer (previously read to current_image owned by HDF5File)
	if (H5Dread(dataset, H5T_STD_U16LE, memspace, dataspace, H5P_DEFAULT, image) < 0 ) {
		cprintf (RED,"Could not read dataset for frame offset %s\n", vectorToString(frame_offset).c_str());
		std::cerr << "Error: Reading of dataset failed for given start frame offset " << vectorToString(frame_offset) << "\n";
		CloseResources ();
		H5Sclose(memspace);
		return -99;
	}

	// Clean up memory space
	H5Sclose(memspace);

	//*image = current_image; //if uint16_t** is passed, HDF5File owns the resource image points to, which is potentially dangerous

	// Return frame number
	unsigned int retval = frame_index_list[frame_offset[0]];
	
	// Pass updated frame offset value(s) via offset parameter vector
	std::copy_n(frame_offset.begin(), offset.size(), offset.begin());
	/*
	std::transform( offset.begin(), offset.end(), offset.begin(), 
    	[&, i = 0](size_t) mutable { return frame_offset[i++]; } );
	*/

	// Increment frame offset correctly
	conditionalIncrement(frame_offset, file_dims[1]);
	//++frame_offset[0]; //old

	return retval;
}

void HDF5File::PrintCurrentImage (uint16_t* image) {
	printf("\n");
	printf("Frame %llu, Image: %llu\n", frame_offset[0]-1, frame_index_list[frame_offset[0]-1]);

	hsize_t size = file_dims[rank-1] * file_dims[rank-2];
	for (hsize_t i = 0; i < size; ++i){
		printf("%u ", image[i]);
		if (!((i+1) % file_dims[rank-2] ))
			printf("\n\n");
	}
	printf("\n\n\n\n");
}



