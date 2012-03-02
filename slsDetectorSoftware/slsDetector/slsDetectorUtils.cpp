#include "slsDetectorUtils.h"
#include "usersFunctions.h"

#include  <sys/ipc.h>
#include  <sys/shm.h>


slsDetectorUtils::slsDetectorUtils() :    queuesize(0),
					  currentPosition(0), 
					  currentPositionIndex(0), 
					  currentI0(0), 
					  mergingBins(NULL), 
					  mergingCounts(NULL), 
					  mergingErrors(NULL), 
					  mergingMultiplicity(NULL), 
					  badChannelMask(NULL)
  
{							
  pthread_mutex_t mp1 = PTHREAD_MUTEX_INITIALIZER;
  mp=mp1;
  pthread_mutex_init(&mp, NULL);  
}


int slsDetectorUtils::getPointers(int * const l_stoppedFlag,		\
				  int * const l_threadedProcessing,	\
				  int * const l_actionMask,		\
				  mystring * const l_actionScript,	\
				  mystring * const l_actionParameter,	\
				  int * const l_nScanSteps,		\
				  int * const l_scanMode,		\
				  mystring * const l_scanScript,	\
				  mystring * const l_scanParameter,	\
				  mysteps * const l_scanSteps,		\
				  int * const l_scanPrecision,		\
				  int * const l_numberOfPositions,	\
				  float * const l_detPositions,		\
				  char * const l_angConvFile,		\
				  int * const l_correctionMask,		\
				  float * const l_binSize,		\
				  float * const l_fineOffset,		\
				  float * const l_globalOffset,		\
				  int * const l_angDirection,		\
				  char * const l_flatFieldDir,		\
				  char * const l_flatFieldFile,		\
				  char * const l_badChanFile,		\
				  int64_t * const l_timerValue,		\
				  detectorSettings * const l_currentSettings,	\
				  int * const l_currentThresholdEV,	\
				  char * const l_filePath,		\
				  char * const l_fileName,		\
				  int * const l_fileIndex)
{

  stoppedFlag=l_stoppedFlag;
  threadedProcessing=l_threadedProcessing;
  actionMask=l_actionMask;
  actionScript=l_actionScript;
  actionParameter=l_actionParameter;

  nScanSteps=l_nScanSteps;		      
  scanSteps=l_scanSteps;
  scanScript=l_scanScript;
  scanParameter=l_scanParameter;
  scanMode=l_scanMode;		    
  scanPrecision=l_scanPrecision;
		    
  numberOfPositions=l_numberOfPositions;
  detPositions=l_detPositions;
  angConvFile=l_angConvFile;
  correctionMask=l_correctionMask;
  binSize=l_binSize;
  fineOffset=l_fineOffset;
  globalOffset=l_globalOffset;
  angDirection=l_angDirection;
  flatFieldDir=l_flatFieldDir;
  flatFieldFile=l_flatFieldFile;
  badChanFile=l_badChanFile;
  // nBadChans=l_nBadChans;
  // badChansList=l_badChansList;
  // nBadFF=l_nBadFF;
  // badFFList=l_badFFList;
  timerValue=l_timerValue;
  currentSettings=l_currentSettings;
  currentThresholdEV=l_currentThresholdEV;
  filePath=l_filePath;
  fileName=l_fileName;
  fileIndex=l_fileIndex;

#ifdef VERBOSE
  cout << "pointer to badChannelMask is "<< badChannelMask << endl;
#endif
  fillBadChannelMask();
#ifdef VERBOSE
  cout << "pointer to badChannelMask is "<< badChannelMask << endl;
#endif
  return OK;


}
				   
				   

string  slsDetectorUtils::createFileName(char *filepath, char *filename, int aMask, float sv0, int prec0, float sv1, int prec1, int pindex, int npos, int findex) {
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

string slsDetectorUtils::createFileName() {
  return createFileName(filePath, \
			fileName, \
			*actionMask, \
			currentScanVariable[0], \
			scanPrecision[0], \
			currentScanVariable[1], \
			scanPrecision[1], \
			currentPositionIndex, \
			*numberOfPositions, \
			*fileIndex\
			);
  
}



int slsDetectorUtils::getFileIndexFromFileName(string fname) {
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

int slsDetectorUtils::getVariablesFromFileName(string fname, int &index, int &p_index, float &sv0, float &sv1) {
  
  int i;
  float f;
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
#ifdef VERBOSE 
  else
    cout << "******************************** cannot parse file index" << endl;
  
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
#ifdef VERBOSE 
  else 
    cout << "******************************** cannot parse position index" << endl;

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
#ifdef VERBOSE
  else 
    cout << "******************************** cannot parse scan varable 1" << endl;

  cout << s << endl;


#endif
  
  uscore=s.rfind("_");




  if (sscanf( s.substr(uscore+1,s.size()-uscore-1).c_str(),"S%f",&f)) {
    sv0=f;
#ifdef VERBOSE
    cout << "******************************** scan variable 0 is " << sv0 << endl;
#endif
  } 
#ifdef VERBOSE
  else 
    cout << "******************************** cannot parse scan varable 0" << endl;

#endif



  return index;
}





int slsDetectorUtils::flatFieldCorrect(float datain, float errin, float &dataout, float &errout, float ffcoefficient, float fferr){
  float e;

  dataout=datain*ffcoefficient;

  if (errin==0 && datain>=0) 
    e=sqrt(datain);
  else
    e=errin;
  
  if (dataout>0)
    errout=sqrt(e*ffcoefficient*e*ffcoefficient+datain*fferr*datain*fferr);
  else
    errout=1.;
  
  return 0;
};


 int slsDetectorUtils::rateCorrect(float datain, float errin, float &dataout, float &errout, float tau, float t){

   // float data;
   float e;
 
   dataout=(datain*exp(tau*datain/t));
   
   if (errin==0 && datain>=0) 
     e=sqrt(datain);
   else
     e=errin;
   
   if (dataout>0)
     errout=e*dataout*sqrt((1/(datain*datain)+tau*tau/(t*t)));
   else 
     errout=1.;
   return 0;

};


int slsDetectorUtils::setBadChannelCorrection(string fname, int &nbad, int *badlist){
  ifstream infile;
  string str;
  int interrupt=0;
  int ich;
  int chmin,chmax;
#ifdef VERBOSE
  std::cout << "Setting bad channel correction to " << fname << std::endl;
#endif

  if (fname=="" || fname=="none") {
    nbad=0;
    return 0;
  } else { 
    infile.open(fname.c_str(), ios_base::in);
    if (infile.is_open()==0) {
      std::cout << "could not open file " << fname <<std::endl;
      return -1;
    }
    nbad=0;
    while (infile.good() and interrupt==0) {
      getline(infile,str);
#ifdef VERBOSE
      std::cout << str << std::endl;
#endif
      istringstream ssstr;
      ssstr.str(str);
      if (ssstr.bad() || ssstr.fail() || infile.eof()) {
	interrupt=1;
	break;
      }
      if (str.find('-')!=string::npos) {
	ssstr >> chmin ;
	ssstr.str(str.substr(str.find('-')+1,str.size()));
	ssstr >> chmax;
#ifdef VERBOSE
	std::cout << "channels between"<< chmin << " and " << chmax << std::endl;
#endif
	for (ich=chmin; ich<=chmax; ich++) {
	  if (nbad<MAX_BADCHANS) {
	    badlist[nbad]=ich;
	    nbad++;
#ifdef VERBOSE
	    std::cout<< nbad << " Found bad channel "<< ich << std::endl;
#endif
	  } else
	    interrupt=1;
	}
      } else {
	ssstr >> ich;
#ifdef VERBOSE
	std::cout << "channel "<< ich << std::endl;
#endif
	if (nbad<MAX_BADCHANS) {
	  badlist[nbad]=ich;
	  nbad++;
#ifdef VERBOSE
	  std::cout << nbad << " Found bad channel "<< ich << std::endl;
#endif
	} else
	  interrupt=1;
      }


    }
  }
  infile.close();
  if (nbad>0 && nbad<MAX_BADCHANS) {
    return nbad;
  } else
  return 0;
}









   /**
     sets the value of s angular conversion parameter
     \param c can be ANGULAR_DIRECTION, GLOBAL_OFFSET, FINE_OFFSET, BIN_SIZE
     \param v the value to be set
     \returns the actual value
  */

float slsDetectorUtils::setAngularConversionParameter(angleConversionParameter c, float v){
  

  switch (c) {
  case ANGULAR_DIRECTION:
    if (v<0)
      *angDirection=-1;
    else
      *angDirection=1;
    return *angDirection;
  case GLOBAL_OFFSET:
    *globalOffset=v;
    return *globalOffset;
  case FINE_OFFSET:
    *fineOffset=v;
    return *fineOffset;
  case BIN_SIZE:
    *binSize=v;
    return *binSize;
  default:
    return 0;
  }
}

  /**
     returns the value of an angular conversion parameter
     \param c can be ANGULAR_DIRECTION, GLOBAL_OFFSET, FINE_OFFSET, BIN_SIZE
     \returns the actual value

  */

float slsDetectorUtils::getAngularConversionParameter(angleConversionParameter c) {

  switch (c) {
  case ANGULAR_DIRECTION:
    return *angDirection;
  case GLOBAL_OFFSET:
    return *globalOffset;
  case FINE_OFFSET:
    return *fineOffset;
  case BIN_SIZE:
    return *binSize;
  default:
    return 0;
  }
}











int slsDetectorUtils::readAngularConversion(string fname, int nmod, angleConversionConstant *angOff) {

  ifstream infile;
  int mod;
  float center, ecenter;
  float r_conv, er_conv;
  float off, eoff;
  string ss;
  int interrupt=0;

  //" module %i center %E +- %E conversion %E +- %E offset %f +- %f \n"
#ifdef VERBOSE
  std::cout<< "Opening file "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    readAngularConversion(infile, nmod, angOff);
    infile.close();
  } else {
    std::cout<< "Could not open calibration file "<< fname << std::endl;
    return -1;
  }
  return 0;
}



int slsDetectorUtils::readAngularConversion( ifstream& infile, int nmod, angleConversionConstant *angOff) {
  string str;
  int mod;
  float center, ecenter;
  float r_conv, er_conv;
  float off, eoff;
  string ss;
  int interrupt=0;
  int nm=0;
  //" module %i center %E +- %E conversion %E +- %E offset %f +- %f \n"
  while (infile.good() and interrupt==0) {
    getline(infile,str);
#ifdef VERBOSE
    cout << "** mod " << nm << " " ;
    std::cout<< str << std::endl;
#endif
    istringstream ssstr(str);
    ssstr >> ss >> mod;
    ssstr >> ss >> center;
    ssstr >> ss >> ecenter;
    ssstr >> ss >> r_conv;
    ssstr >> ss >> er_conv;
    ssstr >> ss >> off;
    ssstr >> ss >> eoff;
    if (nm<nmod && nm>=0 ) {
      angOff[nm].center=center;
      angOff[nm].r_conversion=r_conv;
      angOff[nm].offset=off;
      angOff[nm].ecenter=ecenter;
      angOff[nm].er_conversion=er_conv;
      angOff[nm].eoffset=eoff;
    } else
      break;
    nm++;
    if (nm>=nmod)
      break;
  }
  return nm;
 }





int slsDetectorUtils:: writeAngularConversion(string fname, int nmod, angleConversionConstant *angOff) {

  ofstream outfile;
  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
  {  
    writeAngularConversion(outfile, nmod, angOff);
    outfile.close();
  } else {
    std::cout<< "Could not open file " << fname << "for writing"<< std::endl;
    return -1;
  }
  //" module %i center %E +- %E conversion %E +- %E offset %f +- %f \n"
  return 0;
}




int slsDetectorUtils:: writeAngularConversion(ofstream& outfile, int nmod, angleConversionConstant *angOff) {

  for (int imod=0; imod<nmod; imod++) {
      outfile << " module " << imod << " center "<< angOff[imod].center<<"  +- "<< angOff[imod].ecenter<<" conversion "<< angOff[imod].r_conversion << " +- "<< angOff[imod].er_conversion <<  " offset "<< angOff[imod].offset << " +- "<< angOff[imod].eoffset << std::endl;
  }
  return 0;
}











int slsDetectorUtils::resetMerging(float *mp, float *mv, float *me, int *mm, float binsize) {
  
   for (int ibin=0; ibin<(360./binsize); ibin++) {
     mp[ibin]=0;
     mv[ibin]=0;
     me[ibin]=0;
     mm[ibin]=0;
   }
   return OK;
 }

int slsDetectorUtils::finalizeMerging(float *mp, float *mv, float *me, int *mm, float binsize) {
   int np=0;
   for (int ibin=0; ibin<(360./binsize); ibin++) {
     if (mm[ibin]>0) {
       mp[np]=mp[ibin]/mm[ibin];
       mv[np]=mv[ibin]/mm[ibin];
      me[np]=me[ibin]/mm[ibin];
      me[np]=sqrt(me[ibin]);
      mm[np]=mm[ibin];
      np++;
    }
  }
  return np;
}





int  slsDetectorUtils::addToMerging(float *p1, float *v1, float *e1, float *mp, float *mv,float *me, int *mm, int nchans, float binsize,int angDir, int cm, int *badChanMask ) {

  float binmi=-180., binma;
  int ibin=0;
  int imod;
  float ang=0;


  binmi=-180.;
  binma=binmi+binsize;
#ifdef VERBOSE
  cout << "pointer to badchan mask is " << badChanMask << endl; 
#endif

  if (angDir>0) {
    for (int ip=0; ip<nchans; ip++) {
      if ((cm)&(1<< DISCARD_BAD_CHANNELS)) {
	if (badChanMask[ip]) {
#ifdef VERBOSE
	  cout << "channel " << ip << " is bad " << endl;
#endif
	  continue;
	}
      }
      ang=p1[ip];
      
      while (binma<ang) {
	ibin++;
	binmi+=binsize;
	binma+=binsize;
      }
      if (ibin<(360./binsize)) {
	mp[ibin]+=ang;
	mv[ibin]+=v1[ip];
	if (e1)
	  me[ibin]+=(e1[ip]*e1[ip]);
	else
	  me[ibin]+=v1[ip];
	mm[ibin]++;
      } else
	return FAIL;
    }
 
  } else {
    for (int ip=nchans-1; ip>=0; ip--) {
	if ((cm)&(1<< DISCARD_BAD_CHANNELS)) {
	  if (badChanMask[ip])
	    continue;
       
	}

      while (binma<ang) {
	ibin++;
	binmi+=binsize;
	binma+=binsize;
      }
      if (ibin<(360./binsize)) {
	mp[ibin]+=ang;
	mv[ibin]+=v1[ip];
	if (e1)
	  me[ibin]+=(e1[ip]*e1[ip]);
	else
	  me[ibin]+=v1[ip];
	mm[ibin]++;
      } else
	return FAIL;
    }
  }
  return OK;

}



int slsDetectorUtils::resetMerging(float *mp, float *mv, float *me, int *mm) {
  
  float binsize;
  if (*binSize>0)
    binsize=*binSize;
  else 
    return FAIL;

  return resetMerging(mp, mv, me, mm, binsize);
}


int slsDetectorUtils::finalizeMerging(float *mp, float *mv, float *me, int *mm) {
  float binsize;

  if (*binSize>0)
    binsize=*binSize;
  else 
    return FAIL;

  return finalizeMerging(mp, mv, me, mm, binsize);
}






int  slsDetectorUtils::addToMerging(float *p1, float *v1, float *e1, float *mp, float *mv,float *me, int *mm) {

  float binsize;



  int ibin=0;
  int imod;
  
  int del=0;

  if (*binSize>0)
    binsize=*binSize;
  else 
    return FAIL;
  
  if (p1==NULL) {

    del=1;
    p1=convertAngles(currentPosition);
    
  }
  
  int ret=addToMerging(p1, v1, e1, mp, mv,me, mm,getTotalNumberOfChannels(), binsize,*angDirection, *correctionMask, badChannelMask );
  
  
  if (del) {
    delete [] p1;
    p1=NULL;
  }
  return ret;
}




int slsDetectorUtils::writeDataFile(string fname, int nch, float *data, float *err, float *ang, char dataformat){


  ofstream outfile;
  int idata;
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


int slsDetectorUtils::writeDataFile(ofstream &outfile, int nch, float *data, float *err, float *ang, char dataformat, int offset){


  int idata;
  if (data==NULL)
    return FAIL;

  //  args|=0x10; // one line per channel!


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
int slsDetectorUtils::writeDataFile(string fname, int nch, int *data){
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
int slsDetectorUtils::writeDataFile(ofstream &outfile, int nch, int *data, int offset){
  if (data==NULL)
    return FAIL;

    for (int ichan=0; ichan<nch; ichan++)
      outfile << ichan+offset << " " << *(data+ichan) << std::endl;
   
    return OK;
};












int slsDetectorUtils::readDataFile(int nch, string fname, float *data, float *err, float *ang, char dataformat){


  ifstream infile;
  int ichan, iline=0;
  int interrupt=0;
  float fdata, ferr, fang;
  int maxchans;
  int ich;
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


int slsDetectorUtils::readDataFile(int nch, ifstream &infile, float *data, float *err, float *ang, char dataformat, int offset){


  int ichan, iline=0;
  int interrupt=0;
  float fdata, ferr, fang;
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



int slsDetectorUtils::readDataFile(string fname, int *data, int nch){

  ifstream infile;
  int ichan, idata, iline=0;
  int interrupt=0;
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

int slsDetectorUtils::readDataFile(ifstream &infile, int *data, int nch, int offset){

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


int slsDetectorUtils::readDataFile(string fname, short int *data, int nch){

  ifstream infile;
  int ichan, iline=0;
  int interrupt=0;
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

int slsDetectorUtils::readDataFile(ifstream &infile, short int *data, int nch, int offset){

  int ichan, iline=0;
  short int idata;
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
	std::cout<< "read " << iline <<" channels " << std::endl;
#endif
  }
  return iline;
}



/*writes raw data file */

int slsDetectorUtils::writeDataFile(string fname, float *data, float *err, float *ang, char dataformat, int nch){
  if (nch==-1)
    nch=getTotalNumberOfChannels();
  
  return writeDataFile(fname, nch, data, err, ang, dataformat);

}
int slsDetectorUtils::writeDataFile(ofstream &outfile, float *data, float *err, float *ang, char dataformat, int nch, int offset){
  if (nch==-1)
    nch=getTotalNumberOfChannels();
  
  return writeDataFile(outfile, nch, data, err, ang, dataformat, offset);

}




int slsDetectorUtils::writeDataFile(string fname, int *data){
  
  return writeDataFile(fname, getTotalNumberOfChannels(), data);
}

int slsDetectorUtils::writeDataFile(ofstream &outfile, int *data, int offset){
  
  return writeDataFile(outfile, getTotalNumberOfChannels(), data, offset);
}



int slsDetectorUtils::readDataFile(string fname, float *data, float *err, float *ang, char dataformat) {
  return readDataFile(getTotalNumberOfChannels(), fname, data, err, ang, dataformat);

}

int slsDetectorUtils::readDataFile(ifstream &infile, float *data, float *err, float *ang, char dataformat, int offset) {
  return readDataFile(getTotalNumberOfChannels(), infile, data, err, ang, dataformat, offset);

}



int slsDetectorUtils::readDataFile(string fname, int *data){

  return readDataFile(fname, data, getTotalNumberOfChannels());
};


int slsDetectorUtils::readDataFile(ifstream &infile, int *data, int offset){

  return readDataFile(infile, data, getTotalNumberOfChannels(), offset);
};





int slsDetectorUtils::readDataFile(string fname, short int *data){

  return readDataFile(fname, data, getTotalNumberOfChannels());
};


int slsDetectorUtils::readDataFile(ifstream &infile, short int *data, int offset){

  return readDataFile(infile, data, getTotalNumberOfChannels(),offset);
};



int slsDetectorUtils::readCalibrationFile(string fname, float &gain, float &offset){

  string str;
  ifstream infile;
#ifdef VERBOSE
  std::cout<< "Opening file "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    getline(infile,str);
#ifdef VERBOSE
    std::cout<< str << std::endl;
#endif
    istringstream ssstr(str);
    ssstr >> offset >> gain;
    infile.close();
  } else {
    std::cout<< "Could not open calibration file "<< fname << std::endl;
    gain=0.;
    offset=0.;
    return -1;
  }
  return 0;
};

int slsDetectorUtils::writeCalibrationFile(string fname, float gain, float offset){
  //std::cout<< "Function not yet implemented " << std::endl;
  ofstream outfile;

  outfile.open (fname.c_str());

  // >> i/o operations here <<
  if (outfile.is_open()) {
    outfile << offset << " " << gain << std::endl;
  } else {
    std::cout<< "Could not open calibration file "<< fname << " for writing" << std::endl;
    return -1;
  }

  outfile.close();

  return 0;
};







void  slsDetectorUtils::acquire(int delflag){



#ifdef VERBOSE
  cout << "Acquire function "<< delflag << endl;
#endif

#ifdef VERBOSE
  cout << "Stopped flag is "<< stoppedFlag << delflag << endl;
#endif


  void *status;
  int trimbit;




  char cmd[MAX_STR_LENGTH];
  int  startindex=*fileIndex;
  int nowindex=startindex, lastindex=startindex;
  string fn;




  //string sett;
  if ((*correctionMask&(1<< ANGULAR_CONVERSION)) || (*correctionMask&(1<< I0_NORMALIZATION)))
       connect_channels();
       


  setTotalProgress();
  progressIndex=0;
  *stoppedFlag=0;



  resetFinalDataQueue();
  resetDataQueue();

  
  //cout << "main mutex lock line 6188" << endl;
  pthread_mutex_lock(&mp);
  jointhread=0;
  queuesize=0;
  pthread_mutex_unlock(&mp);
  //cout << "main mutex unlock line 6188" << endl;



#ifdef VERBOSE
  cout << " starting thread " << endl;
#endif

  if (*threadedProcessing) {
    startThread(delflag);
  }




  //cout << "data thread started " << endl;
  int np=1;
  if (*numberOfPositions>0) 
    np=*numberOfPositions;

  int ns0=1;
  if (*actionMask & (1 << MAX_ACTIONS)) {
    ns0=nScanSteps[0];
  }
  if (ns0<1)
    ns0=1;


  int ns1=1;
  if (*actionMask & (1 << (MAX_ACTIONS+1))) {
    ns1=nScanSteps[1];
  }
  if (ns1<1)
    ns1=1;




  //cout << "action at start" << endl;
  if (*stoppedFlag==0) {
      if (*actionMask & (1 << startScript)) {
	//"Custom start script. The arguments are passed as nrun=n par=p.");
	sprintf(cmd,"%s nrun=%d par=%s",getActionScript(startScript).c_str(),*fileIndex,getActionParameter(startScript).c_str());
#ifdef VERBOSE
	cout << "Executing start script " << cmd << endl;
#endif
	system(cmd);
      }
  }

  for (int is0=0; is0<ns0; is0++) {
    //  cout << "scan0 loop" << endl;

  if (*stoppedFlag==0) {

    currentScanVariable[0]=getScanStep(0,is0);
    currentScanIndex[0]=is0;

    switch(scanMode[0]) {
    case 1:
      setThresholdEnergy((int)currentScanVariable[0]); //energy scan
      break;
    case 2:
      setDAC(currentScanVariable[0],THRESHOLD); // threshold scan
      break;
    case 3:
      trimbit=(int)currentScanVariable[0];
      setChannel((trimbit<<((int)TRIMBIT_OFF))|((int)COMPARATOR_ENABLE)); // trimbit scan
      break;
    case 0:
       currentScanVariable[0]=0;
       break;
    default:
    //Custom scan script level 0. The arguments are passed as nrun=n fn=filename var=v par=p"
      sprintf(cmd,"%s nrun=%d fn=%s var=%f par=%s",getScanScript(0).c_str(),*fileIndex,createFileName().c_str(),currentScanVariable[0],getScanParameter(0).c_str());
#ifdef VERBOSE
      cout << "Executing scan script 0 " << cmd << endl;
#endif
      system(cmd);
     

    }
  } else
    break;
  

  for (int is1=0; is1<ns1; is1++) {
    // cout << "scan1 loop" << endl;

     if (*stoppedFlag==0) {

       currentScanVariable[1]=getScanStep(1,is1);
    currentScanIndex[1]=is1;

    switch(scanMode[1]) {
    case 1:
      setThresholdEnergy((int)currentScanVariable[1]); //energy scan
      break;
    case 2:
      setDAC(currentScanVariable[1],THRESHOLD); // threshold scan
      break;
    case 3:
      trimbit=(int)currentScanVariable[1];
      setChannel((trimbit<<((int)TRIMBIT_OFF))|((int)COMPARATOR_ENABLE)); // trimbit scan
      break;
    case 0:
       currentScanVariable[1]=0;
       break;
    default:
    //Custom scan script level 1. The arguments are passed as nrun=n fn=filename var=v par=p"
      sprintf(cmd,"%s nrun=%d fn=%s var=%f par=%s",getScanScript(1).c_str(),*fileIndex,createFileName().c_str(),currentScanVariable[1],getScanParameter(1).c_str());
#ifdef VERBOSE
      cout << "Executing scan script 1 " << cmd << endl;
#endif
      system(cmd);
    }
  
     } else
       break;

     if (*stoppedFlag==0) {
       if (*actionMask & (1 << scriptBefore)) {
	 //Custom script before each frame. The arguments are passed as nrun=n fn=filename par=p sv0=scanvar0 sv1=scanvar1 p0=par0 p1=par1"
	 sprintf(cmd,"%s nrun=%d fn=%s par=%s sv0=%f sv1=%f p0=%s p1=%s",getActionScript(scriptBefore).c_str(),*fileIndex,createFileName().c_str(),getActionParameter(scriptBefore).c_str(),currentScanVariable[0],currentScanVariable[1],getScanParameter(0).c_str(),getScanParameter(1).c_str());
#ifdef VERBOSE
	 cout << "Executing script before " << cmd << endl;
#endif
	 system(cmd);
       }
     } else
       break;

     currentPositionIndex=0;
     
     for (int ip=0; ip<np; ip++) {
       //   cout << "positions " << endl;
       if (*stoppedFlag==0) {
	 if  (*numberOfPositions>0) {
	   go_to_position (detPositions[ip]);
	   currentPositionIndex=ip+1;
#ifdef VERBOSE
	   std::cout<< "moving to position" << std::endl;
#endif
	 } 
       } else
	 break;
       
	 //write header before?
	 //cmd=headerBeforeScript;
	 //Custom script to write the header. \n The arguments will be passed as nrun=n fn=filenam acqtime=t gainmode=g threshold=thr badfile=badf angfile=angf bloffset=blo fineoffset=fo fffile=fffn tau=deadtau par=p")
       
       //   cout << "aaaaa" << endl;
       //cout << createFileName() << endl;
       //cout << "bbbbb" << endl;
       fn=createFileName();
       //cout << fn << endl;
       nowindex=*fileIndex;

       if (*stoppedFlag==0) {


	 if (*correctionMask&(1<< I0_NORMALIZATION))
	   currentI0=get_i0();
	 
	 // cout << "header " << endl;
	 if (*actionMask & (1 << headerBefore)) {
	   //Custom script after each frame. The arguments are passed as nrun=n fn=filename par=p sv0=scanvar0 sv1=scanvar1 p0=par0 p1=par1"
	   sprintf(cmd,"%s nrun=%d fn=%s acqtime=%f gainmode=%d threshold=%d badfile=%s angfile=%s bloffset=%f fineoffset=%f fffile=%s/%s tau=%f par=%s",\
		   getActionScript(headerBefore).c_str(),\
		   nowindex,\
		   fn.c_str(),				       \
		   ((float)timerValue[ACQUISITION_TIME])*1E-9, \
		   *currentSettings, \
		   *currentThresholdEV, \
		   getBadChannelCorrectionFile().c_str(), \
		   getAngularConversion().c_str(), \
		   *globalOffset, \
		   *fineOffset,\
		   getFlatFieldCorrectionDir(),\
		   getFlatFieldCorrectionFile(), \
		   getRateCorrectionTau(),\
		   getActionParameter(headerBefore).c_str()\
		   );
#ifdef VERBOSE
	   cout << "Executing header before " << cmd << endl;
#endif
	   system(cmd);
	 }
       } else
	 break;
       
       
       
       if (*stoppedFlag==0) {

	 //	 cout << "starting???? " << endl;
	 startAndReadAll();
	 
	 if (*correctionMask&(1<< ANGULAR_CONVERSION))
	   currentPosition=get_position();  
	 
	 if (*correctionMask&(1<< I0_NORMALIZATION))
	   currentI0=get_i0()-currentI0;
	 
	 ////////////////// Reput in in case of unthreaded processing
// 	 if (*threadedProcessing==0)
// 	   processData(delflag); 
	 
	 
       } else
	 break;
       
       pthread_mutex_lock(&mp);
       while (queuesize){
	 pthread_mutex_unlock(&mp);
	 usleep(10000);
	 pthread_mutex_lock(&mp);
       }
       pthread_mutex_unlock(&mp);
    
    if (*stoppedFlag==0) {
      if (*actionMask & (1 << headerAfter)) {
	//Custom script after each frame. The arguments are passed as nrun=n fn=filename par=p sv0=scanvar0 sv1=scanvar1 p0=par0 p1=par1"
	sprintf(cmd,"%s nrun=%d fn=%s acqtime=%f gainmode=%d threshold=%d badfile=%s angfile=%s bloffset=%f fineoffset=%f fffile=%s/%s tau=%f par=%s", \
		getActionScript(headerAfter).c_str(),			\
		nowindex,				\
		fn.c_str(),				\
		((float)timerValue[ACQUISITION_TIME])*1E-9, \
		*currentSettings,			\
		*currentThresholdEV,			\
		getBadChannelCorrectionFile().c_str(),	\
		getAngularConversion().c_str(),		\
		*globalOffset,				\
		*fineOffset,				\
		getFlatFieldCorrectionDir(),		\
		getFlatFieldCorrectionFile(),		\
		getRateCorrectionTau(),			\
		getActionParameter(headerAfter).c_str());
#ifdef VERBOSE
	cout << "Executing header after " << cmd << endl;
#endif
	system(cmd);
	
      }
      if (*fileIndex>lastindex)
	lastindex=*fileIndex;
    } else {

      
      if (*fileIndex>lastindex)
	lastindex=*fileIndex;

      break;
    }
     

    
    if (*stoppedFlag) {
#ifdef VERBOSE
      std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
      break;
    } else if (ip<(np-1)) {
      *fileIndex=startindex; 
    }
     } // loop on position finished

  //script after
        if (*stoppedFlag==0) {
	  if (*actionMask & (1 << scriptAfter)) {
	    //Custom script after each frame. The arguments are passed as nrun=n fn=filename par=p sv0=scanvar0 sv1=scanvar1 p0=par0 p1=par1"
	    sprintf(cmd,"%s nrun=%d fn=%s par=%s sv0=%f sv1=%f p0=%s p1=%s",getActionScript(scriptAfter).c_str(),*fileIndex,createFileName().c_str(),getActionParameter(scriptAfter).c_str(),currentScanVariable[0],currentScanVariable[1],getScanParameter(0).c_str(),getScanParameter(1).c_str());
#ifdef VERBOSE
	    cout << "Executing script after " << cmd << endl;
#endif
	    system(cmd);
	  } 
	} else
	    break;
  

    if (*stoppedFlag) {
#ifdef VERBOSE
      std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
      break;
    } else if (is1<(ns1-1)) {
      *fileIndex=startindex; 
    }


  } 

  //end scan1 loop is1
  //currentScanVariable[MAX_SCAN_LEVELS];
  

    if (*stoppedFlag) {
#ifdef VERBOSE
      std::cout<< "exiting since the detector has been stopped" << std::endl;
#endif
      break;
    } else if (is0<(ns0-1)) {
      *fileIndex=startindex; 
    }
  } //end scan0 loop is0

  *fileIndex=lastindex;
  if (*stoppedFlag==0) {
    if (*actionMask & (1 << stopScript)) {
      //Custom stop script. The arguments are passed as nrun=n par=p.
      sprintf(cmd,"%s nrun=%d par=%s",getActionScript(stopScript).c_str(),*fileIndex,getActionParameter(stopScript).c_str());
#ifdef VERBOSE
      cout << "Executing stop script " << cmd << endl;
#endif
      system(cmd);
    }
  } 


   if (*threadedProcessing) { 
     //#ifdef VERBOSE
     //  std::cout<< " ***********************waiting for data processing thread to finish " << queuesize <<" " << thisDetector->fileIndex << std::endl ;
    //#endif
     //cout << "main mutex lock line 6488" << endl;
     pthread_mutex_lock(&mp);
     jointhread=1;
     pthread_mutex_unlock(&mp);
     //cout << "main mutex unlock line 6488" << endl;
     pthread_join(dataProcessingThread, &status);
     // std::cout<< " ***********************data processing  finished " << queuesize <<" " << thisDetector->fileIndex << std::endl ;
   }

      if ((*correctionMask&(1<< ANGULAR_CONVERSION)) || (*correctionMask&(1<< I0_NORMALIZATION)))
	disconnect_channels();
}


void* slsDetectorUtils::processData(int delflag) {


  //cout << "thread mutex lock line 6505" << endl;
  pthread_mutex_lock(&mp);
  queuesize=dataQueue.size();
  pthread_mutex_unlock(&mp);
  //cout << "thread mutex unlock line 6505" << endl;

  int *myData;
  float *fdata;
  float *rcdata=NULL, *rcerr=NULL;
  float *ffcdata=NULL, *ffcerr=NULL;
  float *ang=NULL;
  float bs=0.004;
  int imod;
  int nb;
  int np;
  detectorData *thisData;
  int dum=1;
  string ext;
  string fname;


#ifdef ACQVERBOSE
  std::cout<< " processing data - threaded mode " << *threadedProcessing;
#endif

  if (*correctionMask!=0) {
    ext=".dat";
  } else {
    ext=".raw";
  }
  while(dum | *threadedProcessing) { // ????????????????????????
    
    
    //  while( !dataQueue.empty() ) {
    //cout << "thread mutex lock line 6539" << endl;
    pthread_mutex_lock(&mp);
    while((queuesize=dataQueue.size())>0) {
      pthread_mutex_unlock(&mp);
      //cout << "thread mutex unlock line 6543" << endl;
      //queuesize=dataQueue.size();

      /** Pop data queue */
      myData=dataQueue.front(); // get the data from the queue 
      if (myData) {
	
	

	progressIndex++;
#ifdef VERBOSE
	cout << "Progress is " << getCurrentProgress() << " \%" << endl;
#endif

	//process data
	/** decode data */
	fdata=decodeData(myData);
	
	fname=createFileName();
	

	//uses static function?!?!?!?
	//	writeDataFile (fname+string(".raw"), getTotalNumberOfChannels(),fdata, NULL, NULL, 'i'); 
	writeDataFile (fname+string(".raw"),fdata, NULL, NULL, 'i'); 
 

	/** write raw data file */	   
	if (*correctionMask==0 && delflag==1) {
	  delete [] fdata;
	} else {

	  /** rate correction */
	  if (*correctionMask&(1<<RATE_CORRECTION)) {
	    rcdata=new float[getTotalNumberOfChannels()]; 
	    rcerr=new float[getTotalNumberOfChannels()];
	    rateCorrect(fdata,NULL,rcdata,rcerr);
	    delete [] fdata;
	  } else {
	    rcdata=fdata;
	    fdata=NULL;
	  }
	  
	  /** flat field correction */
	  if (*correctionMask&(1<<FLAT_FIELD_CORRECTION)) {
	    
	    ffcdata=new float[getTotalNumberOfChannels()]; 
	    ffcerr=new float[getTotalNumberOfChannels()];
#ifdef VERBOSE
	    cout << "array size " << getTotalNumberOfChannels() << endl;
#endif
	    flatFieldCorrect(rcdata,rcerr,ffcdata,ffcerr);
#ifdef VERBOSE
	    cout << "FF corr done " << endl;
#endif
	    delete [] rcdata;
	    if (rcerr)	    delete [] rcerr;
	  } else {
	    ffcdata=rcdata;
	    ffcerr=rcerr;
	    rcdata=NULL;
	    rcerr=NULL;
	  }
	  
	  if (*correctionMask&(1<< ANGULAR_CONVERSION)) {

	    if (currentPositionIndex<=1) {     
	      if (*binSize>0)
		bs=*binSize;
	    else
		*binSize=bs;
	      
	      
	      nb=(int)(360./bs);
	      
	      mergingBins=new float[nb];
	      mergingCounts=new float[nb];
	      mergingErrors=new float[nb];
	      mergingMultiplicity=new int[nb];
	      
	      resetMerging(mergingBins, mergingCounts,mergingErrors, mergingMultiplicity, bs);
	    }
	    /* it would be better to create an ang0 with 0 encoder position and add to merging/write to file simply specifying that offset so that when it cycles writing the data or adding to merging it also calculates the angular position */
	    
	    ang=convertAngles(currentPosition);

	    if (*correctionMask!=0) {
	      if (*numberOfPositions>1) {
		//uses static function?!?!?!?
		//writeDataFile (fname+string(".dat"), getTotalNumberOfChannels(), ffcdata, ffcerr,ang);
		writeDataFile (fname+string(".dat"), ffcdata, ffcerr,ang);
	      }
	    }
	    addToMerging(ang, ffcdata, ffcerr, mergingBins, mergingCounts,mergingErrors, mergingMultiplicity, getTotalNumberOfChannels(), bs, *angDirection, *correctionMask, badChannelMask );
	    
	    if ((currentPositionIndex==*numberOfPositions) || (currentPositionIndex==0)) {
	      np=finalizeMerging(mergingBins, mergingCounts,mergingErrors, mergingMultiplicity, bs);
	      /** file writing */
	      currentPositionIndex++;
	      fname=createFileName();
	      if (*correctionMask!=0) {
		//uses static function?!?!?!?
		writeDataFile (fname+string(".dat"),np,mergingCounts, mergingErrors, mergingBins,'f');
	      }
	      if (delflag) {
		delete [] mergingBins;
		delete [] mergingCounts;
		delete [] mergingErrors;
		delete [] mergingMultiplicity;
	      } else {
		thisData=new detectorData(mergingCounts,mergingErrors,mergingBins,getCurrentProgress(),(fname+string(ext)).c_str(),np);
		finalDataQueue.push(thisData);
	      }
	    }
	    
	    if (ffcdata)
	      delete [] ffcdata;
	    if (ffcerr)
	      delete [] ffcerr;
	    if (ang)
	      delete [] ang;
	  } else {
	    if (*correctionMask!=0) {
	      //uses static function?!?!?!?
	      //writeDataFile (fname+string(".dat"), getTotalNumberOfChannels(), ffcdata, ffcerr);
	      writeDataFile (fname+string(".dat"),  ffcdata, ffcerr);
	    }
	    if (delflag) {
	      if (ffcdata)
		delete [] ffcdata;
	      if (ffcerr)
		delete [] ffcerr;
	      if (ang)
		delete [] ang;
	    } else {
	      thisData=new detectorData(ffcdata,ffcerr,NULL,getCurrentProgress(),(fname+string(ext)).c_str(),getTotalNumberOfChannels());
	      finalDataQueue.push(thisData);  
	    }  
	  }
	}
	(*fileIndex)++;
#ifdef VERBOSE
	cout << "Incrementing file index " << *fileIndex << endl;
#endif


	delete [] myData;
	myData=NULL;
	dataQueue.pop(); //remove the data from the queue
	pthread_mutex_lock(&mp);
	queuesize=dataQueue.size();
	pthread_mutex_unlock(&mp);
	usleep(1000);
      }
      pthread_mutex_unlock(&mp);
      usleep(1000);
    }    
    pthread_mutex_unlock(&mp);
    pthread_mutex_lock(&mp);
    if (jointhread) {
      pthread_mutex_unlock(&mp);
      if (dataQueue.size()==0)
	break;
    } else
      pthread_mutex_unlock(&mp);
      
    dum=0;
  } // ????????????????????????
  return 0;
}


int* slsDetectorUtils::popDataQueue() {
  int *retval=NULL;
  if( !dataQueue.empty() ) {
    retval=dataQueue.front();
    dataQueue.pop();
  }
  return retval;
}

detectorData* slsDetectorUtils::popFinalDataQueue() {
  detectorData *retval=NULL;
  if( !finalDataQueue.empty() ) {
    retval=finalDataQueue.front();
    finalDataQueue.pop();
  }
  return retval;
}

void slsDetectorUtils::resetDataQueue() {
  int *retval=NULL;
  while( !dataQueue.empty() ) {
    retval=dataQueue.front();
    dataQueue.pop();
    delete [] retval;
  }
 
}

void slsDetectorUtils::resetFinalDataQueue() {
  detectorData *retval=NULL;
  while( !finalDataQueue.empty() ) {
    retval=finalDataQueue.front();
    finalDataQueue.pop();
    delete retval;
  }

}


void slsDetectorUtils::startThread(int delflag) {
  pthread_attr_t tattr;
  int ret;
  sched_param param, mparam;
  int policy= SCHED_OTHER;


  // set the priority; others are unchanged
  //newprio = 30;
  mparam.sched_priority =1;
  param.sched_priority =1;   


   /* Initialize and set thread detached attribute */
   pthread_attr_init(&tattr);
   pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);



  // param.sched_priority = 5;
  // scheduling parameters of main thread 
  ret = pthread_setschedparam(pthread_self(), policy, &mparam);
  //#ifdef VERBOSE
    // printf("current priority is %d\n",param.sched_priority);
  //#endif
  if (delflag)
    ret = pthread_create(&dataProcessingThread, &tattr,startProcessData, (void*)this);
  else
    ret = pthread_create(&dataProcessingThread, &tattr,startProcessDataNoDelete, (void*)this);
    
  pthread_attr_destroy(&tattr);
   // scheduling parameters of target thread
  ret = pthread_setschedparam(dataProcessingThread, policy, &param);
  
}


int slsDetectorUtils::fillBadChannelMask() {

  int nbad=0;



  if (*correctionMask&(1<< DISCARD_BAD_CHANNELS)) {
    nbad=getBadChannelCorrection();
#ifdef VERBOSE
    cout << "number of bad channels is " << nbad << endl;
#endif
    if (nbad>0) {
      
      int *badChansList=new int[nbad];
      getBadChannelCorrection(badChansList);

      if (badChannelMask) 
	delete [] badChannelMask;
      badChannelMask=new int[getTotalNumberOfChannels()];

#ifdef VERBOSE
      cout << " pointer to bad channel mask is " << badChannelMask << endl;
#endif
      for (int ichan=0; ichan<getTotalNumberOfChannels(); ichan++)
	badChannelMask[ichan]=0;
#ifdef VERBOSE
      cout << " badChanMask has be reset" << badChannelMask << endl;
#endif
      for (int ichan=0; ichan<nbad; ichan++) {
	if (badChansList[ichan]<getTotalNumberOfChannels() && badChansList[ichan]>=0 ) {
	  if (badChannelMask[badChansList[ichan]]==0)
	    nbad++;
	  badChannelMask[badChansList[ichan]]=1;
	  
	}
      }
      delete [] badChansList;

    } else {
      if (badChannelMask) {
#ifdef VERBOSE
      cout << "deleting bad channel mask beacuse number of bad channels is 0" << endl;
#endif
	
	delete [] badChannelMask;
	badChannelMask=NULL;
      }
    }

  } else {
#ifdef VERBOSE
    cout << "bad channel correction is disabled " << nbad << endl;
#endif
    if (badChannelMask) {
#ifdef VERBOSE
      cout << "deleting bad channel mask beacuse no bad channel correction is selected" << endl;
#endif
      delete [] badChannelMask;
      badChannelMask=NULL;
    }
  }

#ifdef VERBOSE
    cout << "number of bad channels is " << nbad << endl;
#endif
  return  nbad;
}






  /*
      set  positions for the acquisition
      \param nPos number of positions
      \param pos array with the encoder positions
      \returns number of positions
  */
int slsDetectorUtils::setPositions(int nPos, float *pos){
  if (nPos>=0)
    *numberOfPositions=nPos; 
  for (int ip=0; ip<nPos; ip++) 
    detPositions[ip]=pos[ip]; 


  //setTotalProgress();
  
  return *numberOfPositions;
}
/* 
   get  positions for the acquisition
   \param pos array which will contain the encoder positions
   \returns number of positions
*/
int slsDetectorUtils::getPositions(float *pos){ 
  if (pos ) {
    for (int ip=0; ip<*numberOfPositions; ip++) 
      pos[ip]=detPositions[ip];
  } 
  //setTotalProgress();

  return  *numberOfPositions;
}
  





  /** 
      set action 
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript, MAX_ACTIONS}
      \param fname for script ("" disable but leaves script unchanged, "none" disables and overwrites)
      \returns 0 if action disabled, >0 otherwise
  */
int slsDetectorUtils::setAction(int iaction, string fname, string par) {

  int am;


  if (iaction>=0 && iaction<MAX_ACTIONS) {

    if (fname=="") {
      am=0;
    } else if (fname=="none") {
      am=0;
      strcpy(actionScript[iaction],fname.c_str());
    } else {
      strcpy(actionScript[iaction],fname.c_str());
      am=1;
    }
    
    if (par!="") {
      strcpy(actionParameter[iaction],par.c_str());
    }
    
    if (am) {
      
#ifdef VERBOSE
      cout << iaction << "  " << hex << (1 << iaction) << " " << *actionMask << dec;
#endif
      
      *actionMask |= (1 << iaction);
      
#ifdef VERBOSE
      cout << " set " << hex << *actionMask << dec << endl;
#endif
      
    } else {
#ifdef VERBOSE
    cout << iaction << "  " << hex << *actionMask << dec;
#endif
    
    *actionMask &= ~(1 << iaction);
    
#ifdef VERBOSE
    cout << "  unset " << hex << *actionMask << dec << endl;
#endif
    }
#ifdef VERBOSE
    cout << iaction << " Action mask set to " << hex << *actionMask << dec << endl;
#endif
    
    return am; 
  } else
    return -1;
}


int slsDetectorUtils::setActionScript(int iaction, string fname) {
#ifdef VERBOSE
  
#endif
  return setAction(iaction,fname,"");
}



int slsDetectorUtils::setActionParameter(int iaction, string par) {
  int am;

  if (iaction>=0 && iaction<MAX_ACTIONS) {
    am= 1& ( (*actionMask) << iaction);

    if (par!="") {
      strcpy(actionParameter[iaction],par.c_str());
    }
    
    if ((*actionMask) & (1 << iaction))
      return 1; 
    else
      return 0;
  } else
    return -1; 
}

  /** 
      returns action script
      \param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
      \returns action script
  */
string slsDetectorUtils::getActionScript(int iaction){
  if (iaction>=0 && iaction<MAX_ACTIONS) 
    return string(actionScript[iaction]);
  else
    return string("wrong index");
};

    /** 
	returns action parameter
	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
	\returns action parameter
    */
string slsDetectorUtils::getActionParameter(int iaction){
  if (iaction>=0 && iaction<MAX_ACTIONS) 
    return string(actionParameter[iaction]);
  else
    return string("wrong index");
}

   /** 
	returns action mode
	\param iaction can be enum {startScript, scriptBefore, headerBefore, headerAfter,scriptAfter, stopScript}
	\returns action mode
    */
int slsDetectorUtils::getActionMode(int iaction){
  if (iaction>=0 && iaction<MAX_ACTIONS) {

    if ((*actionMask) & (1 << iaction))
      return 1; 
    else
      return 0;
  } else {
#ifdef VERBOSE
    cout << "slsDetetctor : wrong action index " << iaction <<  endl;
#endif
    return -1;
  }
}


  /** 
      set scan 
      \param index of the scan (0,1)
      \param fname for script ("" disable)
      \returns 0 if scan disabled, >0 otherwise
  */
int slsDetectorUtils::setScan(int iscan, string script, int nvalues, float *values, string par, int precision) {
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {

    if (script=="") {
      scanMode[iscan]=0;
    } else {
      strcpy(scanScript[iscan],script.c_str());
      if (script=="none") {
	scanMode[iscan]=0;
      } else if (script=="energy") {
	scanMode[iscan]=1;
      }  else if (script=="threshold") {
	scanMode[iscan]=2;
      } else if (script=="trimbits") {
	scanMode[iscan]=3;
      } else {
	scanMode[iscan]=4;
      }  
    }
    
  

    

    
    if (par!="")
      strcpy(scanParameter[iscan],par.c_str());
      
      if (nvalues>=0) {    
	if (nvalues==0)
	  scanMode[iscan]=0;
	else {
	  nScanSteps[iscan]=nvalues;
	  if (nvalues>MAX_SCAN_STEPS)
	    nScanSteps[iscan]=MAX_SCAN_STEPS;
	}
      }
      
      if (values && 	scanMode[iscan]>0 ) {
	for (int iv=0; iv<nScanSteps[iscan]; iv++) {
	  scanSteps[iscan][iv]=values[iv];
	}
      }

      if (precision>=0)
	scanPrecision[iscan]=precision;
      
      if (scanMode[iscan]>0){
	*actionMask |= 1<< (iscan+MAX_ACTIONS);
      } else {
	*actionMask &= ~(1 <<  (iscan+MAX_ACTIONS));
      }



      setTotalProgress();










      return scanMode[iscan];
  }  else 
    return -1;
  
}

int slsDetectorUtils::setScanScript(int iscan, string script) {
 if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    if (script=="") {
      scanMode[iscan]=0;
    } else {
      strcpy(scanScript[iscan],script.c_str());
      if (script=="none") {
	scanMode[iscan]=0;
      } else if (script=="energy") {
	scanMode[iscan]=1;
      }  else if (script=="threshold") {
	scanMode[iscan]=2;
      } else if (script=="trimbits") {
	scanMode[iscan]=3;
      } else {
	scanMode[iscan]=4;
      }  
    }
    
    if (scanMode[iscan]>0){
      *actionMask |= (1 << (iscan+MAX_ACTIONS));
    } else {
      *actionMask &= ~(1 <<  (iscan+MAX_ACTIONS));
    }

    setTotalProgress();
    
#ifdef VERBOSE
      cout << "Action mask is " << hex << actionMask << dec << endl;
#endif
    return scanMode[iscan];   
 } else 
   return -1;
}



int slsDetectorUtils::setScanParameter(int iscan, string par) {


  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
      if (par!="")
	strcpy(scanParameter[iscan],par.c_str());
      return scanMode[iscan];
  } else
    return -1;

}


int slsDetectorUtils::setScanPrecision(int iscan, int precision) {
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    if (precision>=0)
      scanPrecision[iscan]=precision;
    return scanMode[iscan];
  } else
    return -1;

}

int slsDetectorUtils::setScanSteps(int iscan, int nvalues, float *values) {

  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
  
      if (nvalues>=0) {    
	if (nvalues==0)
	  scanMode[iscan]=0;
	else {
	  nScanSteps[iscan]=nvalues;
	  if (nvalues>MAX_SCAN_STEPS)
	    nScanSteps[iscan]=MAX_SCAN_STEPS;
	}
      }
      
      if (values) {
	for (int iv=0; iv<nScanSteps[iscan]; iv++) {
	  scanSteps[iscan][iv]=values[iv];
	}
      }
     
      if (scanMode[iscan]>0){
	*actionMask |= (1 << (iscan+MAX_ACTIONS));
      } else {
	*actionMask &= ~(1 <<  (iscan+MAX_ACTIONS));
      }

#ifdef VERBOSE
      cout << "Action mask is " << hex << actionMask << dec << endl;
#endif
      setTotalProgress();




      return scanMode[iscan];
  

  } else 
      return -1;
  
  
  
  
}



  /** 
      returns scan script
      \param iscan can be (0,1) 
      \returns scan script
  */
string slsDetectorUtils::getScanScript(int iscan){
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
      if (scanMode[iscan])
	return string(scanScript[iscan]);
      else
	return string("none");
  } else
      return string("wrong index");
      
};

    /** 
	returns scan parameter
	\param iscan can be (0,1)
	\returns scan parameter
    */
string slsDetectorUtils::getScanParameter(int iscan){
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    if (scanMode[iscan])
      return string(scanParameter[iscan]);
    else
      return string("none");
  }   else
      return string("wrong index");
}


   /** 
	returns scan mode
	\param iscan can be (0,1)
	\returns scan mode
    */
int slsDetectorUtils::getScanMode(int iscan){
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS)
    return scanMode[iscan];
  else
    return -1;
}


   /** 
	returns scan steps
	\param iscan can be (0,1)
	\param v is the pointer to the scan steps
	\returns scan steps
    */
int slsDetectorUtils::getScanSteps(int iscan, float *v) {

  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    if (v) {
      for (int iv=0; iv<nScanSteps[iscan]; iv++) {
	v[iv]=scanSteps[iscan][iv];
      }
    }


    setTotalProgress();


    if (scanMode[iscan])
      return nScanSteps[iscan];
    else
      return 0;
  } else
    return -1;
}


int slsDetectorUtils::getScanPrecision(int iscan){
  if (iscan>=0 && iscan<MAX_SCAN_LEVELS) {
    return scanPrecision[iscan];
  } else
    return -1;
}







int slsDetectorUtils::setTotalProgress() {
  
  int nf=1, npos=1, nscan[MAX_SCAN_LEVELS]={1,1}, nc=1;


       

  
  if (timerValue[FRAME_NUMBER])
    nf=timerValue[FRAME_NUMBER];
  
  if (timerValue[CYCLES_NUMBER]>0)
	nc=timerValue[CYCLES_NUMBER];

      if (*numberOfPositions>0)
	npos=*numberOfPositions;

      if ((nScanSteps[0]>0) && (*actionMask & (1 << MAX_ACTIONS)))
	nscan[0]=nScanSteps[0];

      if ((nScanSteps[1]>0) && (*actionMask & (1 << (MAX_ACTIONS+1))))
	nscan[1]=nScanSteps[1];
      
      totalProgress=nf*nc*npos*nscan[0]*nscan[1];

#ifdef VERBOSE
      cout << "nc " << nc << endl;
      cout << "nf " << nf << endl;
      cout << "npos " << npos << endl;
      cout << "nscan[0] " << nscan[0] << endl;
      cout << "nscan[1] " << nscan[1] << endl;

      cout << "Set total progress " << totalProgress << endl;
#endif
      return totalProgress;
}











float slsDetectorUtils::getCurrentProgress() {
#ifdef VERBOSE
  cout << progressIndex << " / " << totalProgress << endl;
#endif
  return 100.*((float)progressIndex)/((float)totalProgress);
}


