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

using namespace std;
/**
   @short class handling the data file I/O flags
*/

class fileIO :  public fileIOStatic, public virtual slsDetectorBase  {


 public:
  /** default constructor */
  fileIO(): fileIOStatic() {};
  /** virtual destructor */
  virtual ~fileIO(){};

  using fileIOStatic::readDataFile;
  using fileIOStatic::writeDataFile;
  using fileIOStatic::createFileName;

  int getFileIndexFromFileName(string fname){return fileIOStatic::getFileIndexFromFileName(fname);};
  int getVariablesFromFileName(string fname, int &index, int &p_index, double &sv0, double &sv1){return fileIOStatic::getVariablesFromFileName(fname, index, p_index, sv0, sv1);};
  /**
     sets the default output files path
     \param s file path
     \return actual file path
  */
  string setFilePath(string s) {sprintf(filePath, s.c_str()); return string(filePath);};

  /**
     sets the default output files root name
     \param s file name to be set
     \returns actual file name
  */
  string setFileName(string s) {sprintf(fileName, s.c_str()); return string(fileName);}; 

  /**
     sets the default output file index
     \param i start file index to be set
     \returns actual file index
  */
  int setFileIndex(int i) {*fileIndex=i; return *fileIndex;}; 
  


  /**
     \returns the  output files path
     
  */
  string getFilePath() {return string(filePath);};
  
  /**
    \returns the  output files root name
  */
  string getFileName() {return string(fileName);};

  /**
     \returns the output file index
  */
  int getFileIndex() {return *fileIndex;};
  

  string createFileName();




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
   virtual int writeDataFile(string fname, double *data, double *err=NULL, double *ang=NULL, char dataformat='f', int nch=-1);
  

  /**
     
  writes a data file
  \paramoutfile output file stream
  \param data array of data values
  \param err array of arrors on the data. If NULL no errors will be written
  
  \param ang array of angular values. If NULL data will be in the form chan-val(-err) otherwise ang-val(-err)
  \param dataformat format of the data: can be 'i' integer or 'f' double (default)
  \param nch number of channels to be written to file. if -1 defaults to the number of installed channels of the detector
  \param offset start channel number
  \returns OK or FAIL if it could not write the file or data=NULL
  
  */
   virtual int writeDataFile(ofstream &outfile, double *data, double *err=NULL, double *ang=NULL, char dataformat='f', int nch=-1, int offset=0);
  

   /**
      writes a data file
      \param fname of the file to be written
      \param data array of data values
      \returns OK or FAIL if it could not write the file or data=NULL  
  */
  virtual int writeDataFile(string fname, int *data);

     /**
      writes a data file
      \param outfile output file stream
      \param data array of data values
      \param offset start channel number
      \returns OK or FAIL if it could not write the file or data=NULL  
  */
  virtual int writeDataFile(ofstream &outfile, int *data, int offset=0);
  


  /**
       writes a data file  of short ints
       \param fname of the file to be written
       \param data array of data values
       \returns OK or FAIL if it could not write the file or data=NULL
  */
  virtual int writeDataFile(string fname, short int *data);    
  
  /**
      writes a data file of short ints
      \param outfile output file stream
      \param data array of data values
      \param offset start channel number
      \returns OK or FAIL if it could not write the file or data=NULL  
  */
  virtual int writeDataFile(ofstream &outfile, short int *data, int offset=0);


  /**
       reads a data file
       \param fname of the file to be read
       \param data array of data values to be filled
       \param err array of arrors on the data. If NULL no errors are expected on the file
       
       \param ang array of angular values. If NULL data are expected in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' double (default)
       \returns OK or FAIL if it could not read the file or data=NULL
  */
  virtual int readDataFile(string fname, double *data, double *err=NULL, double *ang=NULL, char dataformat='f');

  /**
       reads a data file
       \param ifstream input file stream
       \param data array of data values to be filled
       \param err array of arrors on the data. If NULL no errors are expected on the file
       
       \param ang array of angular values. If NULL data are expected in the form chan-val(-err) otherwise ang-val(-err)
       \param offset start channel number to be expected
       \returns OK or FAIL if it could not read the file or data=NULL
  */
  int readDataFile(ifstream& infile, double *data, double *err=NULL, double *ang=NULL, char dataformat='f', int offset=0);

  /**
       reads a raw data file
       \param fname of the file to be read
       \param data array of data values
       \returns OK or FAIL if it could not read the file or data=NULL
  */
  virtual int readDataFile(string fname, int *data);  /**
       reads a raw data file
       \param infile input file stream
       \param data array of data values
       \param offset first channel number to be expected
       \returns OK or FAIL if it could not read the file or data=NULL
  */
  int readDataFile(ifstream &infile, int *data, int offset=0);

  /**

       reads a short int raw data file
       \param fname of the file to be read
       \param data array of data values
       \returns OK or FAIL if it could not read the file or data=NULL
  */
  virtual int readDataFile(string fname, short int *data); 
  /**
       reads a short int raw data file
       \param infile input file stream
       \param data array of data values
       \param offset first channel number to be expected
       \returns OK or FAIL if it could not read the file or data=NULL
  */
  int readDataFile(ifstream &infile, short int *data, int offset=0);


 protected:


  void incrementFileIndex() { (*fileIndex)++; };

    string getCurrentFileName(){return currentFileName;};

    string currentFileName;

    
    /** output directory */
    char *filePath;
    /** file root name */
    char *fileName;
    /** file index */
    int *fileIndex; 


};

#endif
