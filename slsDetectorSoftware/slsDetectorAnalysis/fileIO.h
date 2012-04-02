#ifndef FILEIO_H
#define FILEIO_H

#include "sls_detector_defs.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <sstream>
#include <queue>
#include <math.h>

using namespace std;

class fileIO : public slsDetectorDefs {



 public:
  fileIO(){};
  virtual ~fileIO(){};


  /**
     sets the default output files path
  */
  string setFilePath(string s) {sprintf(filePath, s.c_str()); return string(filePath);};

  /**
     sets the default output files root name
  */
  string setFileName(string s) {sprintf(fileName, s.c_str()); return string(fileName);}; 

  /**
     sets the default output file index
  */
  int setFileIndex(int i) {*fileIndex=i; return *fileIndex;}; 
  
  /**
     returns the default output files path
  \sa  sharedSlsDetector
  */
  string getFilePath() {return string(filePath);};
  
  /**
     returns the default output files root name
  */
  string getFileName() {return string(fileName);};

  /**
     returns the default output file index
  */
  int getFileIndex() {return *fileIndex;};
  





  /** generates file name without extension

      always appends to file path and file name the run index.

      in case also appends the position index 
       
      Filenames will be of the form: filepath/filename(_px)_i
      where x is the position index and i is the run index
      \param filepath outdir
      \param filename file root name
      \param aMask action mask (scans, positions)
      \param sv0 scan variable 0
      \param prec0 scan precision 0
      \param sv1 scan variable 1
      \param prec1 scan precision 1
      \param pindex position index
      \param number of positions
      \param findex file index
      \returns file name without extension
  */
  static string  createFileName(char *filepath, char *filename, int aMask, float sv0, int prec0, float sv1, int prec1, int pindex, int npos, int findex);

  virtual string createFileName();


    /** static function that returns the file index from the file name 
      \param fname file name
      \returns file index*/
   static int getFileIndexFromFileName(string fname);

  /** static function that returns the variables from the file name 
      \param fname file name
      \param index reference to index
      \param p_index reference to position index
      \param sv0 reference to scan variable 0
      \param sv1 reference to scan variable 1
      \returns file index
  */
   static int getVariablesFromFileName(string fname, int &index, int &p_index, float &sv0, float &sv1);
  



    /**
     
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \param err array of arrors on the data. If NULL no errors will be written
       
       \param ang array of angular values. If NULL data will be in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if -1 defaults to the number of installed channels of the detector
       \returns OK or FAIL if it could not write the file or data=NULL
 
  */
   virtual int writeDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=-1); 
   int writeDataFile(ofstream &outfile, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int nch=-1, int offset=0); 
  

  /**
   
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \returns OK or FAIL if it could not write the file or data=NULL  
       \sa mythenDetector::writeDataFile
  */
  virtual int writeDataFile(string fname, int *data);
  int writeDataFile(ofstream &outfile, int *data, int offset=0);
  


  /**

       writes a data file
       \param name of the file to be written
       \param data array of data values
       \returns OK or FAIL if it could not write the file or data=NULL
       \sa mythenDetector::writeDataFile
  */
  virtual int writeDataFile(string fname, short int *data);
  int writeDataFile(ofstream &outfile, short int *data, int offset=0);


  /**
   
       reads a data file
       \param name of the file to be read
       \param data array of data values to be filled
       \param err array of arrors on the data. If NULL no errors are expected on the file
       
       \param ang array of angular values. If NULL data are expected in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if <=0 defaults to the number of installed channels of the detector
       \returns OK or FAIL if it could not read the file or data=NULL
       
  */
  virtual int readDataFile(string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f');  
  int readDataFile(ifstream& infile, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int offset=0);  

  /**
   
       reads a data file
       \param name of the file to be read
       \param data array of data values
       \returns OK or FAIL if it could not read the file or data=NULL
  */
  virtual int readDataFile(string fname, int *data);
  int readDataFile(ifstream &infile, int *data, int offset=0);

  /**

       reads a data file
       \param name of the file to be read
       \param data array of data values
       \returns OK or FAIL if it could not read the file or data=NULL
  */
  virtual int readDataFile(string fname, short int *data);
  int readDataFile(ifstream &infile, short int *data, int offset=0);

  /**
     
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \param err array of arrors on the data. If NULL no errors will be written
       
       \param ang array of angular values. If NULL data will be in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if -1 defaults to the number of installed channels of the detector
       \returns OK or FAIL if it could not write the file or data=NULL
       \sa mythenDetector::writeDataFile
 
  */

   static  int  writeDataFile(string fname, int nch, float *data, float *err=NULL, float *ang=NULL, char dataformat='f'); 
   static  int writeDataFile(ofstream &outfile, int nch, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int offset=0); 

   /**
   
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \returns OK or FAIL if it could not write the file or data=NULL  
       \sa mythenDetector::writeDataFile
  */
  static int writeDataFile(string fname,int nch,  int *data);
  static  int writeDataFile(ofstream &outfile,int nch,  int *data, int offset=0);



  /**
   
       writes a data file
       \param name of the file to be written
       \param data array of data values
       \returns OK or FAIL if it could not write the file or data=NULL
       \sa mythenDetector::writeDataFile
  */
  static int writeDataFile(string fname,int nch, short int *data);
  static  int writeDataFile(ofstream &outfile,int nch,  short int *data, int offset=0);
   /**
   
       reads a data file
       \param name of the file to be read
       \param data array of data values to be filled
       \param err array of arrors on the data. If NULL no errors are expected on the file
       
       \param ang array of angular values. If NULL data are expected in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' float (default)
       \param nch number of channels to be written to file. if <=0 defaults to the number of installed channels of the detector
       \returns number of channels read or -1 if it could not read the file or data=NULL
       
       \sa mythenDetector::readDataFile
  */
   static int readDataFile(int nch, string fname, float *data, float *err=NULL, float *ang=NULL, char dataformat='f');  
   static int readDataFile(int nch, ifstream &infile, float *data, float *err=NULL, float *ang=NULL, char dataformat='f', int offset=0);  

  /**
   
       reads a data file
       \param name of the file to be read
       \param data array of data values
       \returns OK or FAIL if it could not read the file or data=NULL
  */
   static int readDataFile(string fname, int *data, int nch);
   static int readDataFile(ifstream &infile, int *data, int nch, int offset);

   /**

        reads a data file
        \param name of the file to be read
        \param data array of data values
        \returns OK or FAIL if it could not read the file or data=NULL
        \sa mythenDetector::readDataFile
   */
    static int readDataFile(string fname, short int *data, int nch);
    static int readDataFile(ifstream &infile, short int *data, int nch, int offset);


    virtual int getActionMask() {return 0;};

    virtual float getCurrentScanVariable(int index) {return 0;};
    virtual int getScanPrecision(int index) {return 0;};

    virtual int getCurrentPositionIndex() {return 0;};
    virtual int getNumberOfPositions() {return 0;};

    virtual int getTotalNumberOfChannels()=0;
    
 protected:




  char *filePath;
  char *fileName;
  int *fileIndex; 


};

#endif
