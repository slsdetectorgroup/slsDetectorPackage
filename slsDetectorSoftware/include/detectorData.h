#include <cstring>
#include <unistd.h>
#pragma once
/**
   @short data structure to hold the detector data after postprocessing
 */
class detectorData {
  public:
    /**
     * Constructor
     * @param progress progress index
     * @param fname file name prefix
     * @param nx number of detector channels (1D detector) or dimension in x (2D
     * detector)
     * @param ny dimension in y (2D detector)
     * @param d pointer to data in char* format
     * @param dbytes number of bytes of image pointed to by cval pointer
     * @param dr dynamic range or bits per pixel
     * @param fIndex file index
     * @param complete true if complete image, else missing packets
     */
    detectorData(double progress, std::string fname, int x, int y, char *d,
                 int dbytes, int dr, uint64_t fIndex, bool complete)
        : progressIndex(progress), fileName(fname), fileIndex(fIndex), nx(x),
          ny(y), data(d), databytes(dbytes), dynamicRange(dr),
          completeImage(complete){};

    /**
     * Destructor
     * data has to be deleted by caller
     */
    ~detectorData(){};

    int64_t getChannel(int i) {
        int off = dynamicRange / 8;
        if (off == 1) {
            char val = *(data + i);
            return val;
        }
        if (off == 2) {
            int16_t val = *((int16_t *)(data + i * off));
            return val;
        }
        if (off == 4) {
            int32_t val = *((int32_t *)(data + i * off));
            return val;
        }
        if (off == 8) {
            int64_t val = *((int64_t *)(data + i * off));
            return val;
        }
        return -1;
    }
    // private:
    double progressIndex;
    std::string fileName;
    uint64_t fileIndex;
    int nx;
    int ny;
    char *data;
    int databytes;
    int dynamicRange;
    bool completeImage;
};
