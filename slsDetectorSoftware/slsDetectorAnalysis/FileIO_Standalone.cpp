#include "FileIO_Standalone.h"

			   

	   

string  fileIO::createFileName(char *filepath, char *filename, int aMask, double sv0, int prec0, double sv1, int prec1, int pindex, int npos, int findex) {
  ostringstream osfn;
  // string fn;
  /*directory name +root file name */
  osfn << filepath << "/" << filename;

  // cout << osfn.str() << endl;

  // scan level 0
  if ( aMask& (1 << (MAX_ACTIONS)))
    osfn << "_S" << fixed << setprecision(prec0) << sv0;

  //cout << osfn.str() << endl;

  //scan level 1
  if (aMask & (1 << (MAX_ACTIONS+1)))
    osfn << "_s" << fixed << setprecision(prec1) << sv1;
  
  //cout << osfn.str() << endl;


  //position
  if (pindex>0 && pindex<=npos)
    osfn << "_p" << pindex;

  //cout << osfn.str() << endl;

  // file index
  osfn << "_" << findex;

  //cout << osfn.str() << endl;


#ifdef VERBOSE
  cout << "created file name " << osfn.str() << endl;
#endif

  //cout << osfn.str() << endl;
  //fn=oosfn.str()sfn.str();
  return osfn.str();

}
	

  /* I/O */

/* generates file name without extension*/

string fileIO::createFileName(int aMask, double sv0, int prec0, double sv1, int prec1, int pindex, int npos) {
  currentFileName=createFileName(filePath, \
			fileName, \
			aMask,	\
			sv0,	\
			prec0,		\
			sv1,	\
			prec1,		\
			pindex,	\
			npos,		\
			*fileIndex	      \
			);
  return currentFileName;
  
}


int fileIO::getFileIndexFromFileName(string fname) {
  int i;
  size_t dot=fname.rfind(".");
  if (dot==string::npos)
    return -1;
  size_t uscore=fname.rfind("_");
  if (uscore==string::npos)
    return -1;

  if (sscanf( fname.substr(uscore+1,dot-uscore-1).c_str(),"%d",&i)) {

  return i;
  } 
  //#ifdef VERBOSE
  cout << "******************************** cannot parse file index" << endl;
  //#endif
  return 0;
}

int fileIO::getVariablesFromFileName(string fname, int &index, int &p_index, double &sv0, double &sv1) {
  
  int i;
  double f;
  string s;


  index=-1;
  p_index=-1;
  sv0=-1;
  sv1=-1;


  //  size_t dot=fname.rfind(".");
  //if (dot==string::npos)
  //  return -1;
  size_t uscore=fname.rfind("_");
  if (uscore==string::npos)
    return -1;
  s=fname;

  //if (sscanf(s.substr(uscore+1,dot-uscore-1).c_str(),"%d",&i)) {
  if (sscanf(s.substr(uscore+1,s.size()-uscore-1).c_str(),"%d",&i)) {
    index=i;
#ifdef VERBOSE
    cout << "******************************** file index is " << index << endl;
#endif
    //return i;
    s=fname.substr(0,uscore);
  }
  else
    cout << "******************************** cannot parse file index" << endl;
  
#ifdef VERBOSE 
  cout << s << endl;
#endif

  
  uscore=s.rfind("_");




  if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"p%d",&i)) {
    p_index=i;
#ifdef VERBOSE
    cout << "******************************** position index is " << p_index << endl;
#endif
    s=fname.substr(0,uscore);
  }
  else 
    cout << "******************************** cannot parse position index" << endl;

#ifdef VERBOSE 
  cout << s << endl;
#endif


  
  
  uscore=s.rfind("_");




  if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"s%f",&f)) {
    sv1=f;
#ifdef VERBOSE
    cout << "******************************** scan variable 1 is " << sv1 << endl;
#endif
    s=fname.substr(0,uscore);
  }
  else 
    cout << "******************************** cannot parse scan varable 1" << endl;

#ifdef VERBOSE
  cout << s << endl;


#endif
  
  uscore=s.rfind("_");




  if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"S%f",&f)) {
    sv0=f;
#ifdef VERBOSE
    cout << "******************************** scan variable 0 is " << sv0 << endl;
#endif
  } 
  else 
    cout << "******************************** cannot parse scan varable 0" << endl;

#ifdef VERBOSE
#endif



  return index;
}





int fileIO::writeDataFile(string fname, int nch, double *data, double *err, double *ang, char dataformat){


  ofstream outfile;
  // int idata;
  if (data==NULL)
    return FAIL;

  //  args|=0x10; // one line per channel!

  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
  {
    writeDataFile(outfile, nch, data, err, ang, dataformat, 0);
    outfile.close();
    return OK;
  } else {
    std::cout<< "Could not open file " << fname << "for writing"<< std::endl;
    return FAIL;
  }
};


int fileIO::writeDataFile(ofstream &outfile, int nch, double *data, double *err, double *ang, char dataformat, int offset){


  int idata;
  if (data==NULL)
    return FAIL;

  //  args|=0x10; // one line per channel!

  cout << "Static file " << endl;


    for (int ichan=0; ichan<nch; ichan++) {
      if (ang==NULL) {
	outfile << ichan+offset << " ";
      } else {
	outfile << ang[ichan] << " ";
      }
	
      switch (dataformat) {
      case 'f':
	outfile << *(data+ichan)<< " ";
	break;
      case 'i':
      default:
	idata=(int)(*(data+ichan));
	outfile << idata << " ";
      }
     if (err) {
       outfile << *(err+ichan)<< " ";
     }
     //   if (args&0x10) {
       outfile << std::endl;
       // }
    }

    return OK;
}













/*writes raw data file */
int fileIO::writeDataFile(string fname, int nch, int *data){
  ofstream outfile;
  if (data==NULL)
    return FAIL;

  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
  {
    writeDataFile(outfile, nch, data, 0);
    outfile.close();
    return OK;
  } else {
    std::cout<< "Could not open file " << fname << "for writing"<< std::endl;
    return FAIL;
  }
};



/*writes raw data file */
int fileIO::writeDataFile(ofstream &outfile, int nch, int *data, int offset){
  if (data==NULL)
    return FAIL;

    for (int ichan=0; ichan<nch; ichan++)
      outfile << ichan+offset << " " << *(data+ichan) << std::endl;
   
    return OK;
};








/*writes raw data file */
int fileIO::writeDataFile(string fname, int nch, short int *data){
  ofstream outfile;
  if (data==NULL)
    return FAIL;

  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
  {
    writeDataFile(outfile, nch, data, 0);
    outfile.close();
    return OK;
  } else {
    std::cout<< "Could not open file " << fname << "for writing"<< std::endl;
    return FAIL;
  }
};



/*writes raw data file */
int fileIO::writeDataFile(ofstream &outfile, int nch, short int *data, int offset){
  if (data==NULL)
    return FAIL;

    for (int ichan=0; ichan<nch; ichan++)
      outfile << ichan+offset << " " << *(data+ichan) << std::endl;

    return OK;
};







int fileIO::readDataFile(int nch, string fname, double *data, double *err, double *ang, char dataformat){


  ifstream infile;
  int  iline=0;//ichan,
  //int interrupt=0;
  //double fdata, ferr, fang;
  int maxchans;
  //int ich;
  string str;


  maxchans=nch;
    
#ifdef VERBOSE
  std::cout<< "Opening file "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    iline=readDataFile(nch, infile, data, err, ang, dataformat, 0);
    infile.close();
  } else {
    std::cout<< "Could not read file " << fname << std::endl;
    return -1;
  }
  return iline;
};


int fileIO::readDataFile(int nch, ifstream &infile, double *data, double *err, double *ang, char dataformat, int offset){


  int  ichan,iline=0; 
  int interrupt=0;
  double fdata, ferr, fang;
  int maxchans;
  int ich;
  string str;


  maxchans=nch;
    

    while (infile.good() and interrupt==0) {
      getline(infile,str);
#ifdef VERBOSE
      std::cout<< str << std::endl;
#endif
      istringstream ssstr(str);
      if (ang==NULL) {
	ssstr >> ichan >> fdata;
	//ich=ichan;
	if (ssstr.fail() || ssstr.bad()) {
	  interrupt=1;
	  break;
	}
	//	if (ich!=iline) 
	//  std::cout<< "Channel number " << ichan << " does not match with line number " << iline << " " << dataformat << std::endl;
	ich=iline;
	if (ichan<offset)
	  continue;
      } else {
	ssstr >> fang >> fdata;
	ich=iline;
      }
      if (ssstr.fail() || ssstr.bad()) {
	interrupt=1;
	break;
      }
       if (err)
	 ssstr >> ferr;
      if (ssstr.fail() || ssstr.bad()) {
	interrupt=1;
	break;
      }
      if (ich<maxchans) { 
	if (ang) {
	 ang[ich]=fang;
       } 
       data[ich]=fdata; 
       if (err)
	 err[ich]=ferr;
       iline++;
      } // else {
//        std::cout<< " too many lines in file: "<< iline << " instead of " << maxchans << std::endl;
//        interrupt=1;
//        break;
//      }
      if (iline>=nch) {
	interrupt=1;
	break;
      }
    }
  return iline;
};



int fileIO::readDataFile(string fname, int *data, int nch){

  ifstream infile;
  int  iline=0;//ichan, idata,
  //int interrupt=0;
  string str;

#ifdef VERBOSE
  std::cout<< "Opening file "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    iline=readDataFile(infile, data, nch, 0);
    infile.close();
  } else {
    std::cout<< "Could not read file " << fname << std::endl;
    return -1;
  }
  return iline;
};

int fileIO::readDataFile(ifstream &infile, int *data, int nch, int offset){

  int ichan, idata, iline=0;
  int interrupt=0;
  string str;


  while (infile.good() and interrupt==0) {
      getline(infile,str);
#ifdef VERBOSE
      std::cout<< str << std::endl;
#endif
      istringstream ssstr(str);
      ssstr >> ichan >> idata;
      if (ssstr.fail() || ssstr.bad()) {
	interrupt=1;
	break;
      }
 //      if (ichan!=iline) {
// 	std::cout<< " Expected channel "<< iline <<" but read channel "<< ichan << std::endl;
// 	interrupt=1;
// 	break;
//       } else {
      if (iline<nch) {
	if (ichan>=offset) {
	  data[iline]=idata;
	  iline++;
	} 
      } else {
	  interrupt=1;
	  break;
	}
	//  }
    }
  return iline;
};


int fileIO::readDataFile(string fname, short int *data, int nch){

  ifstream infile;
  int iline=0;//ichan, 
  //int interrupt=0;
  string str;

#ifdef VERBOSE
  std::cout<< "Opening file "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    iline=readDataFile(infile, data, nch, 0);
    infile.close();
  } else {
    std::cout<< "Could not read file " << fname << std::endl;
    return -1;
  }
  return iline;
};

int fileIO::readDataFile(ifstream &infile, short int *data, int nch, int offset){

  int ichan, iline=0;
  short int idata;
  int interrupt=0;
  string str;
  while (infile.good() and interrupt==0) {
      getline(infile,str);
#ifdef VERBOSE
      ;//std::cout<< str << std::endl;
#endif
      istringstream ssstr(str);
      ssstr >> ichan >> idata;
      if (ssstr.fail() || ssstr.bad()) {
	interrupt=1;
	break;
      }
     //  if (ichan!=iline) {
// 	std::cout<< " Expected channel "<< iline <<" but read channel "<< ichan << std::endl;
// 	interrupt=1;
// 	break;
//       } else {
      if (iline<nch) {
	if (ichan>=offset) {
	  data[iline]=idata;
	  iline++;
	}
      } else {
	interrupt=1;
	break;
      }
	// }
#ifdef VERBOSE
	;//std::cout<< "read " << iline <<" channels " << std::endl;
#endif
  }
  return iline;
}



/*writes raw data file */

int fileIO::writeDataFile(string fname, double *data, double *err, double *ang, char dataformat, int nch){
  if (nch==-1)
    nch=getTotalNumberofChannels();//getTotalNumberOfChannels();
  
  return writeDataFile(fname, nch, data, err, ang, dataformat);

}
int fileIO::writeDataFile(ofstream &outfile, double *data, double *err, double *ang, char dataformat, int nch, int offset){
  if (nch==-1)
    nch=getTotalNumberofChannels();
  
  return writeDataFile(outfile, nch, data, err, ang, dataformat, offset);

}




int fileIO::writeDataFile(string fname, int *data){
  
  return writeDataFile(fname, getTotalNumberofChannels(), data);
}

int fileIO::writeDataFile(ofstream &outfile, int *data, int offset){
  
  return writeDataFile(outfile, getTotalNumberofChannels(), data, offset);
}





int fileIO::writeDataFile(string fname, short int *data){

  return writeDataFile(fname, getTotalNumberofChannels(), data);
}

int fileIO::writeDataFile(ofstream &outfile, short int *data, int offset){

  return writeDataFile(outfile,getTotalNumberofChannels() , data, offset);
}




int fileIO::readDataFile(string fname, double *data, double *err, double *ang, char dataformat) {
  return readDataFile(getTotalNumberofChannels(), fname, data, err, ang, dataformat);

}

int fileIO::readDataFile(ifstream &infile, double *data, double *err, double *ang, char dataformat, int offset) {
  return readDataFile(getTotalNumberofChannels(), infile, data, err, ang, dataformat, offset);

}



int fileIO::readDataFile(string fname, int *data){

  return readDataFile(fname, data, getTotalNumberofChannels());
};


int fileIO::readDataFile(ifstream &infile, int *data, int offset){

  return readDataFile(infile, data, getTotalNumberofChannels(), offset);
};





int fileIO::readDataFile(string fname, short int *data){

  return readDataFile(fname, data, getTotalNumberofChannels());
};


int fileIO::readDataFile(ifstream &infile, short int *data, int offset){

  return readDataFile(infile, data, getTotalNumberofChannels(),offset);
};

