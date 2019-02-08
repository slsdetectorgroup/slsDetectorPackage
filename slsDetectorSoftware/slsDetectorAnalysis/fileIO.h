#ifndef FILEIO_H
#define FILEIO_H

#include "slsDetectorBase.h"
#include "fileIOStatic.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <sstream>
#include <queue>
#include <math.h>
#include <semaphore.h>

// 
/**
   @short class handling the data file I/O flags
*/

class fileIO :  public fileIOStatic, public virtual slsDetectorBase  {


 public:

  /* enum rawFileFormat { */
/*     ASCII, */
/*     BINARY */
/*   } */


  /** default constructor */
  fileIO(): fileIOStatic(){
    currentFrameIndex=-1;
    frameIndex=-1;
    detIndex=-1; 
    framesPerFile=&nframes; 
    nframes=1; 
    filefd = NULL;  
    pthread_mutex_t mp1 = PTHREAD_MUTEX_INITIALIZER;
    mf=mp1;
    pthread_mutex_init(&mf, NULL); 
  };

  /** virtual destructor */
  virtual ~fileIO(){};

  using fileIOStatic::readDataFile;
  using fileIOStatic::writeDataFile;
  using fileIOStatic::createFileName;

  int getFileIndexFromFileName(std::string fname){
    int ret; 
    pthread_mutex_lock(&mf);
    ret=fileIOStatic::getFileIndexFromFileName(fname);
    pthread_mutex_unlock(&mf); 
    return ret;
  };
  int getIndicesFromFileName(std::string fname, int &index){
    int ret; 
    pthread_mutex_lock(&mf);
    ret=fileIOStatic::getIndicesFromFileName(fname,index);
    pthread_mutex_unlock(&mf); 
    return ret;
  };



  int getVariablesFromFileName(std::string fname, int &index, int &p_index, double &sv0, double &sv1){
    
    int ret; 
    pthread_mutex_lock(&mf);
    ret=fileIOStatic::getVariablesFromFileName(fname, index, p_index, sv0, sv1);
    pthread_mutex_unlock(&mf); 
    return ret;
  };

  int getVariablesFromFileName(std::string fname, int &index, int &f_index, int &p_index, double &sv0, double &sv1, int &detindex){

    
    int ret; 
    pthread_mutex_lock(&mf);
    ret=fileIOStatic::getVariablesFromFileName(fname, f_index, index, p_index, sv0, sv1, detindex);
    pthread_mutex_unlock(&mf); 
    return ret;
  };

  /**
     sets the default output files path
     \param s file path
     \return actual file path
  */
  virtual std::string setFilePath(std::string s) {
    pthread_mutex_lock(&mf);
    sprintf(filePath, "%s", s.c_str()); 
    pthread_mutex_unlock(&mf); 
    return std::string(filePath);
  };

  /**
     sets the default output files root name
     \param s file name to be set
     \returns actual file name
  */
  virtual  std::string setFileName(std::string s) {
    pthread_mutex_lock(&mf);
    sprintf(fileName, "%s", s.c_str()); 
    pthread_mutex_unlock(&mf); 
    return std::string(fileName);};

  /**
     sets the default output file index
     \param i start file index to be set
     \returns actual file index
  */
  virtual int setFileIndex(int i) {
    pthread_mutex_lock(&mf);
    *fileIndex=i; 
    pthread_mutex_unlock(&mf); 
    return *fileIndex;
  };
  
  /**
     sets the default output file frame index
     \param i file frame index to be set
     \returns actual file frame index
  */
  virtual int setFrameIndex(int i) {
    pthread_mutex_lock(&mf);
    frameIndex=i; 
    pthread_mutex_unlock(&mf); 
    return frameIndex;};

  /**
     sets the default output current frame index
     \param i current frame index to be set
     \returns actual current frame index
  */
  virtual int setCurrentFrameIndex(int i) {
    pthread_mutex_lock(&mf);
    currentFrameIndex=i; 
    pthread_mutex_unlock(&mf); 
    return currentFrameIndex;
  };

  /**
     sets the default output file index
     \param i frame index to be set
     \returns actual frame index
  */
  virtual int setFramesPerFile(int i) {
    pthread_mutex_lock(&mf);
    if (i>0) *framesPerFile=i;
    pthread_mutex_unlock(&mf); 
    return *framesPerFile;};

  /**
     sets the default output file index
     \param i detector index to be set
     \returns actual detector index
  */
  virtual  int setDetectorIndex(int i) {
	  pthread_mutex_lock(&mf);
	  detIndex=i;
	  pthread_mutex_unlock(&mf);
	  return detIndex;};

  /**
     sets the default file format
     \param i file format to be set
     \returns actual file frame format
  */
  virtual fileFormat setFileFormat(int i) {
	  pthread_mutex_lock(&mf);
	  *fileFormatType=(fileFormat)i;
	  pthread_mutex_unlock(&mf);
	  return *fileFormatType;  };



  /**
     \returns the  output files path
     
  */
  virtual std::string getFilePath() {return std::string(filePath);};
  /**
    \returns the  output files root name
  */
  virtual std::string getFileName() {return std::string(fileName);};

  /**
     \returns the output file index
  */
  virtual int getFileIndex() {return *fileIndex;};
  
  /**
     \returns the output file frame index
  */
  virtual int getFrameIndex() {return frameIndex;};

  /**
     \returns the output current frame index
  */
  virtual int getCurrentFrameIndex() {return currentFrameIndex;};

  /**
     \returns the detector index
  */
  virtual int getDetectorIndex() {return detIndex;};

  /**
     \returns the max frames per file
  */
  virtual int getFramesPerFile() {return *framesPerFile;};

  /**
     \returns the max frames per file
  */
  virtual fileFormat getFileFormat() {return *fileFormatType;};


  std::string createFileName();

  std::string createReceiverFilePrefix();


  /**
  writes a data file
  \param fname of the file to be written
  \param data array of data values
  \param err array of arrors on the data. If NULL no errors will be written
  \param ang array of angular values. If NULL data will be in the form chan-val(-err) otherwise ang-val(-err)
  \param dataformat format of the data: can be 'i' integer or 'f' double (default)
  \param nch number of channels to be written to file. if -1 defaults to the number of installed channels of the detector
  \returns OK or FAIL if it could not write the file or data=NULL
  */
   virtual int writeDataFile(std::string fname, double *data, double *err=NULL, double *ang=NULL, char dataformat='f', int nch=-1);
  

  /**
     
  writes a data file
  \param outfile output file stream
  \param data array of data values
  \param err array of arrors on the data. If NULL no errors will be written
  
  \param ang array of angular values. If NULL data will be in the form chan-val(-err) otherwise ang-val(-err)
  \param dataformat format of the data: can be 'i' integer or 'f' double (default)
  \param nch number of channels to be written to file. if -1 defaults to the number of installed channels of the detector
  \param offset start channel number
  \returns OK or FAIL if it could not write the file or data=NULL
  
  */
   virtual int writeDataFile(std::ofstream &outfile, double *data, double *err=NULL, double *ang=NULL, char dataformat='f', int nch=-1, int offset=0);
  

   /**
      writes a data file
      \param fname of the file to be written
      \param data array of data values
      \returns OK or FAIL if it could not write the file or data=NULL  
  */
  virtual int writeDataFile(std::string fname, int *data);

  
  virtual int writeDataFile(void *data, int iframe=-1);
    
  int closeDataFile();
     /**
      writes a data file
      \param outfile output file stream
      \param data array of data values
      \param offset start channel number
      \returns OK or FAIL if it could not write the file or data=NULL  
  */
  virtual int writeDataFile(std::ofstream &outfile, int *data, int offset=0);
  


  /**
       writes a data file  of short ints
       \param fname of the file to be written
       \param data array of data values
       \returns OK or FAIL if it could not write the file or data=NULL
  */
  virtual int writeDataFile(std::string fname, short int *data);    
  


   



  /**
      writes a data file of short ints
      \param outfile output file stream
      \param data array of data values
      \param offset start channel number
      \returns OK or FAIL if it could not write the file or data=NULL  
  */
  virtual int writeDataFile(std::ofstream &outfile, short int *data, int offset=0);


  /**
       reads a data file
       \param fname of the file to be read
       \param data array of data values to be filled
       \param err array of arrors on the data. If NULL no errors are expected on the file
       
       \param ang array of angular values. If NULL data are expected in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' double (default)
       \returns OK or FAIL if it could not read the file or data=NULL
  */
  virtual int readDataFile(std::string fname, double *data, double *err=NULL, double *ang=NULL, char dataformat='f');

  /**
       reads a data file
       \param ifstream input file stream
       \param data array of data values to be filled
       \param err array of arrors on the data. If NULL no errors are expected on the file
       
       \param ang array of angular values. If NULL data are expected in the form chan-val(-err) otherwise ang-val(-err)
       \param offset start channel number to be expected
       \returns OK or FAIL if it could not read the file or data=NULL
  */
  int readDataFile(std::ifstream& infile, double *data, double *err=NULL, double *ang=NULL, char dataformat='f', int offset=0);

  /**
       reads a raw data file
       \param fname of the file to be read
       \param data array of data values
       \returns OK or FAIL if it could not read the file or data=NULL
yes  */
  virtual int readDataFile(std::string fname, int *data);  

/**
       reads a raw data file
       \param infile input file stream
       \param data array of data values
       \param offset first channel number to be expected
       \returns OK or FAIL if it could not read the file or data=NULL
  */
  int readDataFile(std::ifstream &infile, int *data, int offset=0);

  /**

       reads a short int raw data file
       \param fname of the file to be read
       \param data array of data values
       \returns OK or FAIL if it could not read the file or data=NULL
  */
  virtual int readDataFile(std::string fname, short int *data); 
  /**
       reads a short int raw data file
       \param infile input file stream
       \param data array of data values
       \param offset first channel number to be expected
       \returns OK or FAIL if it could not read the file or data=NULL
  */
  int readDataFile(std::ifstream &infile, short int *data, int offset=0);

  virtual int getDataBytes   (   )=0;
  friend class slsDetector;

  std::string getCurrentFileName(){return currentFileName;};
 protected:


  void incrementFileIndex() { (*fileIndex)++; };

  void incrementFrameIndex(int i) { frameIndex=frameIndex+i; };

  void incrementCurrentFrameIndex() { (currentFrameIndex)++; };

  void incrementDetectorIndex() { (detIndex)++; };


  std::string getCurrentReceiverFilePrefix(){return currentReceiverFilePrefix;};


  std::string currentFileName;

  std::string currentReceiverFilePrefix;

    
  /** output directory */
  char *filePath;
  /** file root name */
  char *fileName;
  /** file index */
  int *fileIndex;
  /** file frame index */
  int frameIndex;
  /** current frame index */
  int currentFrameIndex;
  /** detector id */
  int detIndex;
  /** frames per file */
  int *framesPerFile;
  /** file format */
  fileFormat *fileFormatType;

 private:


  FILE *filefd;
  std::ofstream fstream;

  int nframes;
  // int fformat;
  

  int nBytes;

 /** mutex to synchronize read/write fname */
   pthread_mutex_t mf;
};

#endif
