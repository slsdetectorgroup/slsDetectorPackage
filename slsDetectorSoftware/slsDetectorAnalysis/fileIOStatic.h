#ifndef FILEIO_STATIC_H
#define FILEIO_STATIC_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <sstream>
#include <queue>
#include <math.h>

#ifdef __CINT
#define MYROOT
#endif

using std::endl;
using std::cout;

#include "sls_detector_defs.h"
// 
/**
   @short class handling the data file I/O flags
*/

class fileIOStatic  {




 public:
  /** default constructor */
  fileIOStatic(){};
  /** virtual de`structor */
  virtual ~fileIOStatic(){};

  
  
  
  /** generates file name without extension

  always appends to file path and file name the run index.

  in case also appends the position index and the two level of scan varaibles with the specified precision 
       
  Filenames will be of the form: filepath/(dz_)filename(_Sy_sw_px_fv)_i
  where z is the detector index, y is the scan0 variable, W is the scan 1 variable, x is the position index, v is the frame index and i is the run index
  \param filepath outdir
  \param filename file root name
  \param aMask action mask (scans, positions)
  \param sv0 scan variable 0
  \param prec0 scan precision 0
  \param sv1 scan variable 1
  \param prec1 scan precision 1
  \param pindex position index
  \param npos number of positions
  \param findex file index
  \param frameindex frame index
  \param detindex detector id
  \returns file name without extension
  */
  static std::string  createFileName(char *filepath, char *filename, int aMask, double sv0, int prec0, double sv1, int prec1, int pindex, int npos, int findex, int frameindex=-1, int detindex=-1){ \
    std::ostringstream osfn;							\
    osfn << filepath << "/" << filename;		\
    if ( aMask& (1 << (slsDetectorDefs::MAX_ACTIONS)))  osfn << "_S" << std::fixed << std::setprecision(prec0) << sv0;		\
    if (aMask & (1 << (slsDetectorDefs::MAX_ACTIONS+1)))  osfn << "_s" << std::fixed << std::setprecision(prec1) << sv1;		\
    if (pindex>0 && pindex<=npos)  osfn << "_p" << pindex;		\
    if(detindex>=0) osfn << "_d"<< detindex;	\
    if(frameindex>=0) osfn << "_f" << frameindex;	\
    osfn << "_" << findex;						\
    return osfn.str();							\
  };


  /** generates file prefix  for receivers without file path, frame index, file index or extension

  in case also appends the position index and the two level of scan varaibles with the specified precision

  File prefix will be of the form: (dz_)filename(_Sy_sw_px_)
  where z is the detector index, y is the scan0 variable, W is the scan 1 variable and x is the position index
  \param filename file root name
  \param aMask action mask (scans, positions)
  \param sv0 scan variable 0
  \param prec0 scan precision 0
  \param sv1 scan variable 1
  \param prec1 scan precision 1
  \param pindex position index
  \param npos number of positions
  \param detindex detector id
  \returns file name without extension
  */
  static std::string  createReceiverFilePrefix(char *filename, int aMask, double sv0, int prec0, double sv1, int prec1, int pindex, int npos,int detindex=-1){ \
    std::ostringstream osfn;							\
    osfn << filename;				\
    if ( aMask& (1 << (slsDetectorDefs::MAX_ACTIONS)))  osfn << "_S" << std::fixed << std::setprecision(prec0) << sv0;		\
    if (aMask & (1 << (slsDetectorDefs::MAX_ACTIONS+1)))  osfn << "_s" << std::fixed << std::setprecision(prec1) << sv1;		\
    if (pindex>0 && pindex<=npos)  osfn << "_p" << pindex;		\
    if(detindex!=-1) osfn << "_d"<< detindex;	\
    return osfn.str();												\
  };


  /** static function that returns the file index from the file name 
      \param fname file name
      \returns file index
  */
  static int getFileIndexFromFileName(std::string fname){	\
    int i;						\
    size_t dot=fname.rfind(".");			\
    if (dot==std::string::npos)				\
      return -1;					\
    size_t uscore=fname.rfind("_");			\
    if (uscore==std::string::npos)     return -1;				\
    if (sscanf( fname.substr(uscore+1,dot-uscore-1).c_str(),"%d",&i))  return i; \
    cout << "******************************** cannot parse file index" << endl;	\
    return 0;								\
  };
  
  /** static function that returns the frame index and file index from the file name
      \param fname file name
      \param index reference to index
      \returns frame index
  */
  static int getIndicesFromFileName(std::string fname,int &index){										\
	  int i;																				\
	  std::string s;  																			\
	  size_t uscore=fname.rfind("_");														\
	  if (uscore==std::string::npos)       return -1;											\
	  s=fname;																				\
	  if (sscanf(s.substr(uscore+1,s.size()-uscore-1).c_str(),"%d",&i)){ 					\
		  index=i;																			\
	  	  s=fname.substr(0,uscore);															\
	  }																						\
	  /*else      cout << "******************************** cannot parse file index" << endl; \*/
	  uscore=s.rfind("_");																	\
	  if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"f%d",&i)){ 					\
		  if(i==-1)return 0;																\
		  else return i; }																	\
	  /*cout << "******************************** cannot parse frame index" << endl;			\*/
	  return 0;																				\
  };

  
  /** static function that returns the variables from the file name 
      \param fname file name
      \param index reference to index
      \param p_index reference to position index
      \param sv0 reference to scan variable 0
      \param sv1 reference to scan variable 1
      \returns file index
  */
  static int getVariablesFromFileName(std::string fname, int &index, int &p_index, double &sv0, double &sv1) { \
    int i;								\
    double f;								\
    std::string s;								\
    index=-1;								\
    p_index=-1;								\
    sv0=-1;								\
    sv1=-1;								\
    size_t uscore=fname.rfind("_");					\
    if (uscore==std::string::npos)       return -1;				\
    s=fname;								\
    if (sscanf(s.substr(uscore+1,s.size()-uscore-1).c_str(),"%d",&i)) {	\
      index=i;								\
      s=fname.substr(0,uscore);						\
    }									\
   /* else      cout << "Warning: ******************************** cannot parse file index from " << s << endl; \*/
    uscore=s.rfind("_");						\
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"f%d",&i))   \
      s=fname.substr(0,uscore);	    \
    /*else      cout << "Warning: ******************************** cannot parse frame index from " << s << endl; \*/
    uscore=s.rfind("_");						\
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"d%d",&i))   \
      s=fname.substr(0,uscore);	    \
   /* else      cout << "Warning: ******************************** cannot parse detector index from " << s << endl; \*/
    uscore=s.rfind("_");			\
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"p%d",&i)) { \
      p_index=i;							\
      s=fname.substr(0,uscore);						\
    }									\
   /* else      cout << "Warning: ******************************** cannot parse position index from " << s << endl; \*/
    uscore=s.rfind("_");						\
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"s%lf",&f)) { \
      sv1=f;								\
      s=fname.substr(0,uscore);						\
    }									\
   /* else      cout << "Warning: ******************************** cannot parse scan varable 1 from " << s << endl; \*/
    uscore=s.rfind("_");						\
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"S%lf",&f)) { \
      sv0=f;								\
    }									\
   /* else      cout << "Warning: ******************************** cannot parse scan varable 0 from " << s << endl; \*/
    return index;							\
  };
  
	
  /** static function that returns the variables from the file name
      \param fname file name
      \param index reference to index
      \param f_index reference to frame index
      \param p_index reference to position index
      \param sv0 reference to scan variable 0
      \param sv1 reference to scan variable 1
      \param detindex reference to detector id
      \returns file index
  */
  static int getVariablesFromFileName(std::string fname, int &index, int &f_index, int &p_index, double &sv0, double &sv1, int &detindex) { \
    int i;								\
    double f;								\
    std::string s;								\
    index=-1;								\
    p_index=-1;								\
    sv0=-1;								\
    sv1=-1;								\
    size_t uscore=fname.rfind("_");					\
    if (uscore==std::string::npos)       return -1;				\
    s=fname;								\
    if (sscanf(s.substr(uscore+1,s.size()-uscore-1).c_str(),"%d",&i)) {	\
      index=i;								\
      s=fname.substr(0,uscore);						\
    }									\
    /*else      cout << "Warning: ******************************** cannot parse file index" << endl; \*/
    uscore=s.rfind("_");						\
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"f%d",&i)) { \
      f_index=i;							\
      s=fname.substr(0,uscore);						\
    }									\
    /*else      cout << "Warning: ******************************** cannot parse frame index" << endl; \*/
    uscore=s.rfind("_");						\
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"d%d",&i)) { \
      detindex=i;								\
      s=fname.substr(0,uscore);				\
    }									\
   /* else      cout << "Warning: ******************************** cannot parse detector id" << endl; \*/
    uscore=s.rfind("_");						\
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"p%d",&i)) { \
      p_index=i;							\
      s=fname.substr(0,uscore);						\
    }									\
    /*else      cout << "Warning: ******************************** cannot parse position index" << endl; \*/
    uscore=s.rfind("_");						\
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"s%lf",&f)) { \
      sv1=f;								\
      s=fname.substr(0,uscore);						\
    }									\
    /*else      cout << "Warning: ******************************** cannot parse scan varable 1" << endl; \*/
    uscore=s.rfind("_");						\
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"S%lf",&f)) { \
      sv0=f;								\
      s=fname.substr(0,uscore);						\
    }									\
    /*else      cout << "Warning: ******************************** cannot parse scan varable 0" << endl; \*/

    return index;							\
  };


  /** static function that verifies if the new file name containing new parameters matches all the given parameters
      \param fname new complete file name prefix
      \param index reference to index
      \param f_index reference to frame index
      \param p_index reference to position index
      \param sv0 reference to scan variable 0
      \param sv1 reference to scan variable 1
      \param detindex reference to detector id
      \returns file name
  */
  static int verifySameFrame(std::string fname, int index, int f_index, int p_index, double sv0, double sv1, int detindex) { \
	  int new_index=-1;
  	  int new_f_index=-1;
  	  int new_p_index=-1;
  	  int new_det_index=-1;
  	  double new_sv0=-1;
  	  double new_sv1=-1;
  	  getVariablesFromFileName(fname,new_index, new_f_index, new_p_index, new_sv0, new_sv1, new_det_index);
  	  if(index!=new_index) return 0;
  	  if(f_index!=new_f_index) return 0;
  	  if(p_index!=new_p_index) return 0;
  	  if(sv0!=new_sv0) return 0;
  	  if(sv1!=new_sv1) return 0;
  	  return 1;
  }


  /** static function that returns the name variable from the receiver complete file name prefix
      \param fname complete file name prefix
      \returns file name
  */
  static std::string getNameFromReceiverFilePrefix(std::string fname) { \
    int i;									\
    double f;								\
    std::string s;								\
    s=fname;								\
    size_t uscore=s.rfind("_");						\
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"d%d",&i))  \
        s=fname.substr(0,uscore);						\
    uscore=s.rfind("_");						\
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"p%d",&i))  \
      s=fname.substr(0,uscore);						\
    uscore=s.rfind("_");						\
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"s%lf",&f))  \
      s=fname.substr(0,uscore);						\
    uscore=s.rfind("_");						\
    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"S%lf",&f))  \
      s=fname.substr(0,uscore);		\


    return s;							\
  };




  /** static function that returns the entire filename ithout file name prefix, detector index  or extension
   	  This will be concatenated with all the other detector file names for the gui
      \param fname complete file name
      \returns file name without file name prefix, detector index  or extension
  */
  static std::string getReceiverFileNameToConcatenate(std::string fname) { \
	  int i;double f;				\
	  std::string s=fname;											\
	  if(fname.empty()) return fname;							\
	  size_t dot=s.find(".");
	  size_t uscore=s.rfind("_");	\

	    if (uscore==std::string::npos)       return "??";				\
	    if (sscanf(s.substr(uscore+1,s.size()-uscore-1).c_str(),"%d",&i)) 	\
	      s=fname.substr(0,uscore);						\
	    uscore=s.rfind("_");						\
	    if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"f%d",&i))			\
	      s=fname.substr(0,uscore);						\
		uscore=s.rfind("_");					\
	     if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"d%d",&i))  \
	         s=fname.substr(0,uscore);						\
	     uscore=s.rfind("_");						\
	     if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"p%d",&i))  \
	       s=fname.substr(0,uscore);						\
	     uscore=s.rfind("_");						\
	     if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"s%lf",&f))  \
	       s=fname.substr(0,uscore);						\
	     uscore=s.rfind("_");						\
	     if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"S%lf",&f))  \
	       s=fname.substr(0,uscore);		\
	       return(fname.substr(s.size(),dot-s.size()));\
											\
  };




  /**
     
  writes a data file
  \param fname of the file to be written
  \param nch number of channels to be written
  \param data array of data values
  \param err array of arrors on the data. If NULL no errors will be written  
  \param ang array of angular values. If NULL data will be in the form chan-val(-err) otherwise ang-val(-err)
  \param dataformat format of the data: can be 'i' integer or 'f' double
  \returns OK or FAIL if it could not write the file or data=NULL
 
  */

  static  int  writeDataFile(std::string fname, int nch, double *data, double *err=NULL, double *ang=NULL, char dataformat='f'){ \
    std::ofstream outfile;							\
    if (data==NULL)    {						\
      cout << "No data to write!" << endl;				\
      return slsDetectorDefs::FAIL;					\
    }									\
    outfile.open (fname.c_str(),std::ios_base::out);			\
    if (outfile.is_open())   {						\
      writeDataFile(outfile, nch, data, err, ang, dataformat, 0);	\
      outfile.close();							\
      return slsDetectorDefs::OK;					\
    } else {								\
      std::cout<< "Could not open file " << fname << "for writing"<< std::endl; \
      return slsDetectorDefs::FAIL;							\
    }									\
  };




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
  static  int writeDataFile(std::ofstream &outfile, int nch, double *data, double *err=NULL, double *ang=NULL, char dataformat='f', int offset=0){ 
    int idata;								\
    if (data==NULL || nch==0) {							\
      cout << "No data to write!" << endl;				\
      return slsDetectorDefs::FAIL;					\
    } 	\
    for (int ichan=0; ichan<nch; ichan++) {				\
      if (ang==NULL) {							\
	outfile << ichan+offset << " ";				\
      } else {								\
	outfile << ang[ichan] << " ";					\
      }								\
      switch (dataformat) {						\
      case 'f':							\
	outfile << *(data+ichan)<< " ";				\
	break;								\
      case 'i':							\
      default:								\
	idata=(int)(*(data+ichan));					\
	outfile << idata << " ";					\
      }								\
      if (err) {							\
	outfile << *(err+ichan)<< " ";					\
      }								\
      outfile << std::endl;						\
    }									\
    return slsDetectorDefs::OK;					\
  };
   
   
  /**
     writes a raw data file
     \param  fname of the file to be written
     \param nch number of channels
     \param data array of data values
     \returns OK or FAIL if it could not write the file or data=NULL  
  */
  static int writeDataFile(std::string fname,int nch,  int *data){	\
    std::ofstream outfile;						\
    if (data==NULL) return slsDetectorDefs::FAIL;		\
    outfile.open (fname.c_str(),std::ios_base::out);		\
    if (outfile.is_open()) {					\
      writeDataFile(outfile, nch, data, 0);			\
      outfile.close();						\
      return slsDetectorDefs::OK;					\
    } else {								\
      std::cout<< "Could not open file " << fname << "for writing"<< std::endl; \
      return slsDetectorDefs::FAIL;					\
    }									\
  };
  
  /**
     writes a raw data file in binary format
     \param  fname of the file to be written
     \param number of bytes to write
     \param data array of data values
     \returns OK or FAIL if it could not write the file or data=NULL  
  */
  static int writeBinaryDataFile(std::string fname, size_t nbytes, void *data){ \
    FILE *sfilefd;							\
    if (data==NULL) return slsDetectorDefs::FAIL;			\
    sfilefd = fopen(fname.c_str(), "w");				\
    if (sfilefd) {							\
      writeBinaryDataFile(sfilefd, nbytes,  data);			\
      fclose(sfilefd);							\
      return slsDetectorDefs::OK;					\
    }									\
    return slsDetectorDefs::FAIL;					\
  };

  static int writeBinaryDataFile(FILE *sfilefd, size_t nbytes,  void *data){ \
    // cout << "bin " << sfilefd << " " << nbytes << " " << data << endl;
    fwrite(data,  1,  nbytes, sfilefd);					\
    return slsDetectorDefs::OK;						\
  };
							
  /**
     writes a raw data file
     \param outfile output file stream
     \param nch number of channels
     \param data array of data values
     \param offset start channel number
     \returns OK or FAIL if it could not write the file or data=NULL  
  */
  static  int writeDataFile(std::ofstream &outfile,int nch,  int *data, int offset=0){ \
    if (data==NULL) return slsDetectorDefs::FAIL;			\
    for (int ichan=0; ichan<nch; ichan++)      outfile << ichan+offset << " " << *(data+ichan) << std::endl; \
    return slsDetectorDefs::OK;					\
  };
   



  /**
   
  writes a short int raw data file
  \param fname of the file to be written
  \param nch number of channels
  \param data array of data values
  \returns OK or FAIL if it could not write the file or data=NULL
  */
  static int writeDataFile(std::string fname,int nch, short int *data) {	\
    std::ofstream outfile;							\
    if (data==NULL)       return slsDetectorDefs::FAIL;		\
    outfile.open (fname.c_str(),std::ios_base::out);			\
    if (outfile.is_open())       {					\
      writeDataFile(outfile, nch, data, 0);				\
      outfile.close();							\
      return slsDetectorDefs::OK;					\
    } else {								\
      std::cout<< "Could not open file " << fname << "for writing"<< std::endl; \
      return slsDetectorDefs::FAIL;					\
    }									\
  };



  /**
     writes a short int raw data file
     \param outfile output file stream
     \param nch number of channels
     \param data array of data values
     \param offset start channel number
     \returns OK or FAIL if it could not write the file or data=NULL  
  */
  static  int writeDataFile(std::ofstream &outfile,int nch,  short int *data, int offset=0){ \
    if (data==NULL)    return slsDetectorDefs::FAIL;			\
    for (int ichan=0; ichan<nch; ichan++)      outfile << ichan+offset << " " << *(data+ichan) << std::endl; \
    return slsDetectorDefs::OK;					\
  };

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
  static int readDataFile(int nch, std::string fname, double *data, double *err=NULL, double *ang=NULL, char dataformat='f') { \
    std::ifstream infile;							\
    int  iline=0;							\
    std::string str;							\
    infile.open(fname.c_str(), std::ios_base::in);				\
    if (infile.is_open()) {						\
      iline=readDataFile(nch, infile, data, err, ang, dataformat, 0);	\
      infile.close();							\
    } else {								\
      std::cout<< "Could not read file " << fname << std::endl;	\
      return -1;							\
    }									\
    return iline;							\
  };

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
  static int readDataFile(int nch, std::ifstream &infile, double *data, double *err=NULL, double *ang=NULL, char dataformat='f', int offset=0){ \
    int  ichan,iline=0;						\
    int interrupt=0;							\
    double fdata, ferr, fang;						\
    int maxchans;							\
    int ich;								\
    std::string str;							\
    maxchans=nch;							\
    while (infile.good() and interrupt==0) {				\
      getline(infile,str);						\
      std::istringstream ssstr(str);					\
      if (ang==NULL) {							\
	ssstr >> ichan >> fdata;					\
	if (ssstr.fail() || ssstr.bad()) {				\
	  interrupt=1;							\
	  break;							\
	}								\
	ich=iline;							\
	if (ichan<offset)						\
	  continue;							\
      } else {								\
	ssstr >> fang >> fdata;					\
	ich=iline;							\
      }								\
      if (ssstr.fail() || ssstr.bad()) {				\
	interrupt=1;							\
	break;								\
      }								\
      if (err)								\
	ssstr >> ferr;							\
      if (ssstr.fail() || ssstr.bad()) {				\
	interrupt=1;							\
	break;								\
      }								\
      if (ich<maxchans) {						\
	if (ang) {							\
	  ang[ich]=fang;						\
	}								\
	data[ich]=fdata;						\
	if (err)							\
	  err[ich]=ferr;						\
	iline++;							\
      }								\
      if (iline>=nch) {						\
	interrupt=1;							\
	break;								\
      }								\
    }									\
    return iline;							\
  };
   
   
  /**
     reads a raw data file
     \param fname of the file to be read
     \param data array of data values
     \param nch number of channels
     \returns OK or FAIL if it could not read the file or data=NULL
  */
  static int readDataFile(std::string fname, int *data, int nch) { \
    std::ifstream infile;					       \
    int  iline=0;					       \
    std::string str;					       \
    infile.open(fname.c_str(), std::ios_base::in);		       \
    if (infile.is_open()) {				       \
      iline=readDataFile(infile, data, nch, 0);	       \
      infile.close();					       \
    } else {						       \
      std::cout<< "Could not read file " << fname << std::endl;	\
      return -1;							\
    }									\
    return iline;							\
  };


  /**
     reads a raw data file
     \param infile input file stream
     \param data array of data values
     \param nch number of channels
     \param offset start channel value
     \returns OK or FAIL if it could not read the file or data=NULL
  */
  static int readDataFile(std::ifstream &infile, int *data, int nch, int offset) { \
    int ichan, idata, iline=0;						\
    int interrupt=0;							\
    std::string str;							\
    while (infile.good() and interrupt==0) {				\
      getline(infile,str);						\
      std::istringstream ssstr(str);					\
      ssstr >> ichan >> idata;						\
      if (ssstr.fail() || ssstr.bad()) {				\
	interrupt=1;							\
	break;								\
      }								\
      if (iline<nch) {							\
	if (ichan>=offset) {						\
	  data[iline]=idata;						\
	  iline++;							\
	}								\
      } else {								\
	interrupt=1;							\
	break;								\
      }								\
    }									\
    return iline;							\
  };

  /**
     reads a short int rawdata file
     \param name of the file to be read
     \param data array of data values
     \param nch number of channels
     \returns OK or FAIL if it could not read the file or data=NULL
  */
  static int readDataFile(std::string fname, short int *data, int nch){	\
    std::ifstream infile;							\
    int iline=0;							\
    std::string str;							\
    infile.open(fname.c_str(), std::ios_base::in);				\
    if (infile.is_open()) {						\
      iline=readDataFile(infile, data, nch, 0);			\
      infile.close();							\
    } else {								\
      std::cout<< "Could not read file " << fname << std::endl;	\
      return -1;							\
    }									\
    return iline;							\
  };   

  /**
     reads a short int raw data file
     \param infile input file stream
     \param data array of data values
     \param nch number of channels
     \param offset start channel value
     \returns OK or FAIL if it could not read the file or data=NULL
  */
  static int readDataFile(std::ifstream &infile, short int *data, int nch, int offset) { \
    int ichan, iline=0;						\
    short int idata;							\
    int interrupt=0;							\
    std::string str;							\
    while (infile.good() and interrupt==0) {				\
      getline(infile,str);						\
      std::istringstream ssstr(str);					\
      ssstr >> ichan >> idata;						\
      if (ssstr.fail() || ssstr.bad()) {				\
	interrupt=1;							\
	break;								\
      }								\
      if (iline<nch) {							\
	if (ichan>=offset) {						\
	  data[iline]=idata;						\
	  iline++;							\
	}								\
      } else {								\
	interrupt=1;							\
	break;								\
      }									\
      return iline;							\
    };									\
    return iline;							\
  };								
};

#endif
