#ifndef FILEIOSTD_H
#define FILEIOSTD_H

//#include "slsDetectorBase.h"
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
/**
   @short class handling the data file I/O flags
*/

class fileIO : public virtual slsDetectorDefs {//public virtual slsDetectorBase {



 public:
  /** default constructor */
  fileIO(){};
  /** virtual destructor */
  virtual ~fileIO(){};

    int setTotalNumberofChannels(int i){ if (i>=0) {totalNumberofChannels=i; return totalNumberofChannels;} else return -1;};
    int getTotalNumberofChannels(){ return totalNumberofChannels; };
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
  





  /** generates file name without extension

      always appends to file path and file name the run index.

      in case also appends the position index and the two level of scan varaibles with the specified precision 
       
      Filenames will be of the form: filepath/filename(_Sy_sw_px)_i
      where y is the scan0 variable, W is the scan 1 variable, x is the position index and i is the run index
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
  static string  createFileName(char *filepath, char *filename, int aMask, double sv0, int prec0, double sv1, int prec1, int pindex, int npos, int findex);


  string createFileName(int aMask, double sv0, int prec0, double sv1, int prec1, int pindex, int npos);


  /** static function that returns the file index from the file name 
      \param fname file name
      \returns file index
  */
  int getFileIndexFromFileName(string fname);

  /** static function that returns the variables from the file name 
      \param fname file name
      \param index reference to index
      \param p_index reference to position index
      \param sv0 reference to scan variable 0
      \param sv1 reference to scan variable 1
      \returns file index
  */
  static int getVariablesFromFileName(string fname, int &index, int &p_index, double &sv0, double &sv1);
  



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
   int writeDataFile(ofstream &outfile, double *data, double *err=NULL, double *ang=NULL, char dataformat='f', int nch=-1, int offset=0); 
  

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
  int writeDataFile(ofstream &outfile, int *data, int offset=0);
  


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
  int writeDataFile(ofstream &outfile, short int *data, int offset=0);


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

  /**
     
       writes a data file
       \param fname of the file to be written
       \param nch number of channels to be written
       \param data array of data values
       \param err array of arrors on the data. If NULL no errors will be written  
       \param ang array of angular values. If NULL data will be in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' double (default)
       \returns OK or FAIL if it could not write the file or data=NULL
 
  */

   static  int  writeDataFile(string fname, int nch, double *data, double *err=NULL, double *ang=NULL, char dataformat='f');   
   /**  
	writes a data file
	\param outfile output file stream
	\param nch number of channels to be written
	\param data array of data values
	\param err array of arrors on the data. If NULL no errors will be written  
	\param ang array of angular values. If NULL data will be in the form chan-val(-err) otherwise ang-val(-err)
	\param dataformat format of the data: can be 'i' integer or 'f' double (default)
	\param offset start channel number
	\returns OK or FAIL if it could not write the file or data=NULL
   */
   static  int writeDataFile(ofstream &outfile, int nch, double *data, double *err=NULL, double *ang=NULL, char dataformat='f', int offset=0); 

   /**
       writes a raw data file
       \param  fname of the file to be written
       \param nch number of channels
       \param data array of data values
       \returns OK or FAIL if it could not write the file or data=NULL  
  */
  static int writeDataFile(string fname,int nch,  int *data);  

  /**
       writes a raw data file
       \param outfile output file stream
       \param nch number of channels
       \param data array of data values
       \param offset start channel number
       \returns OK or FAIL if it could not write the file or data=NULL  
  */
  static  int writeDataFile(ofstream &outfile,int nch,  int *data, int offset=0);


  /**
   
       writes a short int raw data file
       \param fname of the file to be written
       \param nch number of channels
       \param data array of data values
       \returns OK or FAIL if it could not write the file or data=NULL
  */
  static int writeDataFile(string fname,int nch, short int *data);  

  /**
       writes a short int raw data file
       \param outfile output file stream
       \param nch number of channels
       \param data array of data values
       \param offset start channel number
       \returns OK or FAIL if it could not write the file or data=NULL  
  */
  static  int writeDataFile(ofstream &outfile,int nch,  short int *data, int offset=0);


  /**
       reads a data file
       \param nch number of channels
       \param fname of the file to be read
       \param data array of data values to be filled
       \param err array of arrors on the data. If NULL no errors are expected on the file
       \param ang array of angular values. If NULL data are expected in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' double (default)
       \returns number of channels read or -1 if it could not read the file or data=NULL
       
  */
   static int readDataFile(int nch, string fname, double *data, double *err=NULL, double *ang=NULL, char dataformat='f'); 
  /**
       reads a data file
       \param nch number of channels
       \param infile input file stream
       \param data array of data values to be filled
       \param err array of arrors on the data. If NULL no errors are expected on the file
       \param ang array of angular values. If NULL data are expected in the form chan-val(-err) otherwise ang-val(-err)
       \param dataformat format of the data: can be 'i' integer or 'f' double (default)
       \param offset start channel number
       \returns number of channels read or -1 if it could not read the file or data=NULL
       
  */ 
   static int readDataFile(int nch, ifstream &infile, double *data, double *err=NULL, double *ang=NULL, char dataformat='f', int offset=0);  

  /**
       reads a raw data file
       \param fname of the file to be read
       \param data array of data values
       \param nch number of channels
       \returns OK or FAIL if it could not read the file or data=NULL
  */
   static int readDataFile(string fname, int *data, int nch);

   /**
       reads a raw data file
       \param infile input file stream
       \param data array of data values
       \param nch number of channels
       \param offset start channel value
       \returns OK or FAIL if it could not read the file or data=NULL
  */
   static int readDataFile(ifstream &infile, int *data, int nch, int offset);

   /**
        reads a short int rawdata file
        \param name of the file to be read
        \param data array of data values
       \param nch number of channels
        \returns OK or FAIL if it could not read the file or data=NULL
   */
    static int readDataFile(string fname, short int *data, int nch);   
    /**
       reads a short int raw data file
       \param infile input file stream
       \param data array of data values
       \param nch number of channels
       \param offset start channel value
       \returns OK or FAIL if it could not read the file or data=NULL
  */
    static int readDataFile(ifstream &infile, short int *data, int nch, int offset);

    
    


    void incrementFileIndex() { (*fileIndex)++;};

    string getCurrentFileName(){return currentFileName;};
 
    protected:
    string currentFileName;

    
    int mask_action;
    double currentscan_variable[1];
    int scan_precision[1];
    int currentpostion_i;
    int noposition;
    
    /** output directory */
    char *filePath;
    /** file root name */
    char *fileName;
    /** file index */
    int *fileIndex; 
    
    private:

    int totalNumberofChannels;


};

#endif
