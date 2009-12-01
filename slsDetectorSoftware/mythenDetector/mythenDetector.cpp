#include "mythenDetector.h"
#include "usersFunctions.h"



using namespace std;




  /* detector configuration (no communication to server) */

  /* 
     Every detector should have a basic configuration file containing:
     type (mythen, pilatus etc.)
     hostname
     portnumber
     communication type (default TCP/IP)
     eventually secondary portnumber (e.g. mythen stop function)
     number of modules installed if different from the detector size (x,y)
  */

int mythenDetector::readConfigurationFile(string const fname){

  char hostname[1000]=DEFAULT_HOSTNAME;
  int cport=DEFAULT_PORTNO, sport=DEFAULT_PORTNO+1,dport=-1;
  int nmx=-1, nmy=-1;
  float angoff=0.;
  
 
  int nnames=17;
  string names[]={"Hostname", "NmodX", "NmodY", "Detector", "ControlPort", "StopPort", "DataPort", "TrimBitDir","TrimBitEnergies", "CalDir", "BadChanList", "AngCal", "AngDirection", "FineOffset","GlobalOffset", "ClkDiv","BinSize"};
  char line[500];
  string str;
  ifstream infile;
  string::size_type pos;
  int iargval;
  int interrupt=0;

  string sargname, sargval, dtype;
  int iarg;
  float farg;
  int iline=0;
  cout << "config file name "<< fname << endl;
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    while (infile.good() and interrupt==0) {
      sargname="none";
      sargval="0";
      getline(infile,str);
      iline++;
#ifdef VERBOSE
      cout <<  str << endl;
#endif
      if (str.find('#')!=string::npos) {
#ifdef VERBOSE
	cout << "Line is a comment " << endl;
	cout << str << endl;
#endif
	continue;
      } else {
	istringstream ssstr(str);
	ssstr >> sargname;
	if (!ssstr.good()) {
#ifdef VERBOSE
	 cout << "bad stream out" << endl;
#endif
	  interrupt=1;
	  break;
	} else {
	  for (int iname=0; iname<nnames; iname++) {
	    if (sargname==names[iname]) {
	    switch(iname) {
	    case 0: 
	      ssstr>> sargval;
	      strcpy(hostname,sargval.c_str());
	      break;
	    case 1:
	      ssstr>> nmx;
	      break;
	    case 2:
	      ssstr >> nmy;
	      break;
	    case 3:
	      ssstr >> dtype;
	      break;
	    case 4:
	      ssstr >> iargval;
	      cport=iargval;
	      break;
	    case 5:
	      ssstr >> iargval;
	      sport=iargval;
	      break;
	    case 6:
	      ssstr >> iargval;
	      dport=iargval;
	      break;
	    case 7:
	      ssstr>> sargval;
	      strcpy(thisDetector->trimDir,sargval.c_str());
	      break;
	    case 8:
	      thisDetector->nTrimEn=0;
	      while (ssstr.good()) {
		ssstr >> farg;
		thisDetector->trimEnergies[thisDetector->nTrimEn]=farg;
		thisDetector->nTrimEn++;
	      }
	      break;
	    case 9:
	      ssstr>> sargval;
	      strcpy(thisDetector->calDir,sargval.c_str()); 
	      break;
	    case 10:
	      ssstr>> sargval;
	      strcpy(thisDetector->badChanFile,sargval.c_str()); 
	      break;
	    case 11:
	      ssstr>>  sargval;
	      strcpy(thisDetector->angConvFile,sargval.c_str()); 
	      setAngularConversion(thisDetector->angConvFile);
	      break;
	    case 12:
	      ssstr>>  iargval;
	      thisDetector->angDirection=iargval;
	      break;
	    case 13:
	      ssstr>>  farg;
	      thisDetector->fineOffset=farg;
	      break;
	    case 14:
	      ssstr>>  farg;
	      thisDetector->globalOffset=farg;
#ifdef VERBOSE
	      cout << "global offset is "<< farg << endl;
#endif
	      break;
	    case 15:
	      ssstr>>  iargval;
	      thisDetector->clkDiv=iargval;
#ifdef VERBOSE
	      cout << "clock divider is "<< iargval << endl;
#endif
	      break;
	    case 16:
	      ssstr>>  farg;
	      thisDetector->binSize=farg;
#ifdef VERBOSE
	      cout << "binsize is "<< farg << endl;
#endif
	      break;
	    default:
	      cout << "unknown!" << endl;
	    }
	    break;
	  }
	}
	}
      }
    }
    infile.close();
  }
  else
  {
    cout << "Error opening configuration file " << fname << " for reading" << endl;
    return FAIL;
  }
  
  setTCPSocket(hostname,cport,sport,dport);
  setDetectorType(dtype);
  setNumberOfModules(nmx,X);
  setNumberOfModules(nmy,Y);
#ifdef VERBOSE
  cout << "Finished reading configuration file " << endl;
#endif
  return OK;
};


int mythenDetector::writeConfigurationFile(string const fname){
  

  string names[]={"Hostname", "NmodX", "NmodY", "Detector", "ControlPort", "StopPort", "DataPort", "TrimBitDir", "TrimBitEnergies","CalDir", "BadChanList", "AngCal", "AngDirection", "AngOffset", "ClkDiv"};
  ofstream outfile;
  char hostname[1000]="";


  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
  {
    while (outfile.good()) {
      if (controlSocket) {
	controlSocket->getHostname(hostname);
      }
      outfile << "Hostname " << hostname << endl;
      outfile << "NModX " << setNumberOfModules(GET_FLAG, X) << endl;
      outfile << "NModY " << setNumberOfModules(GET_FLAG, Y) << endl;
      switch (thisDetector->myDetectorType) {
      case MYTHEN:
	outfile << "Detector Mythen" << endl;
	break;
      case PILATUS:
	outfile << "Detector Pilatus" << endl;
	break;
      case EIGER:
	outfile << "Detector EIGER" << endl;
	break;
      case GOTTHARD:
	outfile << "Detector Gotthard" << endl;
	break;
      case AGIPD:
	outfile << "Detector Agipd" << endl;
	break;
	default:
	outfile << "Detector Generic" << endl;
	break;
	
      }
      if (controlSocket) 
	outfile << "ControlPort " << controlSocket->getPortNumber() << endl;
      if (stopSocket) 
	outfile << "StopPort " << stopSocket->getPortNumber() << endl;
      if (dataSocket) 
	outfile << "DataPort" << dataSocket->getPortNumber() << endl;
      outfile << "TrimBitDir " << thisDetector->trimDir << endl;
      outfile << "TrimBitEnergies" ;
      for (int ien=0; ien< thisDetector->nTrimEn; ien++)
	outfile << " " << thisDetector->trimEnergies[ien]<< endl;
	outfile << endl;
      outfile << "CalDir " << thisDetector->calDir << endl;
      //  outfile << "BadChanList " << thisDetector->badChanFile << endl;
      //outfile << "AngCal " << thisDetector->angConvFile << endl;
      outfile << "AngDirection " << thisDetector->angDirection << endl;
      outfile << "FineOffset " << thisDetector->fineOffset<< endl;
      outfile << "GlobalOffset " << thisDetector->globalOffset<< endl;
      outfile << "ClkDiv " << thisDetector->clkDiv << endl;
    }
    outfile.close();
  }
  else
  {
    cout << "Error opening configuration file " << fname << " for writing" << endl;
    return FAIL;
  }
  return OK;
};
  /* 
     It should be possible to dump all the settings of the detector (including trimbits, threshold energy, gating/triggering, acquisition time etc.
     in a file and retrieve it for repeating the measurement with identicals ettings, if necessary
  */
/*  int mythenDetector::dumpDetectorSetup(string fname){};
  int mythenDetector::retrieveDetectorSetup(string fname){};
*/



  /* I/O */


 sls_detector_module* mythenDetector::readTrimFile(string fname,  sls_detector_module *myMod){
  
   int nflag=0;

   if (myMod==NULL) {
     myMod=createModule();
     nflag=1;
   }
   string myfname;
  string str;
  ifstream infile;
  ostringstream oss;
  int isgood=1;
  int iline=0;
  string names[]={"Vtrim", "Vthresh", "Rgsh1", "Rgsh2", "Rgpr", "Vcal", "outBuffEnable"};
  string sargname;
  int ival;
  int ichan=0, ichip=0, idac=0;

 

#ifdef VERBOSE
  cout <<   "reading trimfile for module number "<< myMod->module << endl;
#endif
    //im=myMod->module;
    //myMod->module=im;
    // myMod->serialnumber=getId(MODULE_SERIAL_NUMBER, im);
    //  if (im<0)
      myfname=fname;
      /* else {
      // oss << fname << ".sn" << setfill('0') <<setw(3) << hex << myMod->serialnumber
      oss << fname << ".sn" << setfill('0') <<setw(3) << hex <<  myMod->module;
      myfname=oss.str();
      
      }*/
#ifdef VERBOSE
    cout << "trim file name is "<< myfname <<   endl;
#endif
    infile.open(myfname.c_str(), ios_base::in);
    if (infile.is_open()) {
      // while (infile.good() && isgood==1) {
	for (int iarg=0; iarg<thisDetector->nDacs; iarg++) {
	  getline(infile,str);
	  iline++;
#ifdef VERBOSE
	  //  cout << str << endl;
#endif
	  istringstream ssstr(str);
	  ssstr >> sargname >> ival;
#ifdef VERBOSE
	  cout << sargname << " dac nr. " << idac << " is " << ival << endl;
#endif
	  myMod->dacs[idac]=ival;
	  idac++;
	}
	for (ichip=0; ichip<thisDetector->nChips; ichip++) { 
	  getline(infile,str); 
	  iline++;
#ifdef VERBOSE
	  //	  cout << str << endl;
#endif
	  istringstream ssstr(str);
	  ssstr >> sargname >> ival;
#ifdef VERBOSE
	  //	  cout << "chip " << ichip << " " << sargname << " is " << ival << endl;
#endif
	 
	  myMod->chipregs[ichip]=ival;
	  for (ichan=0; ichan<thisDetector->nChans; ichan++) {
	    getline(infile,str); 
#ifdef VERBOSE
	    // cout << str << endl;
#endif
	    istringstream ssstr(str);

#ifdef VERBOSE
	    //   cout << "channel " << ichan+ichip*thisDetector->nChans <<" iline " << iline<< endl;
#endif
	    iline++;
	    myMod->chanregs[ichip*thisDetector->nChans+ichan]=0;
	    for (int iarg=0; iarg<6 ; iarg++) {
	      ssstr >>  ival; 
	      //if (ssstr.good()) {
	      switch (iarg) {
	      case 0:
#ifdef VERBOSE
		//		 cout << "trimbits " << ival ;
#endif
		 myMod->chanregs[ichip*thisDetector->nChans+ichan]|=ival&0x3f;
		 break;
	      case 1:
#ifdef VERBOSE
		//cout << " compen " << ival ;
#endif
		myMod->chanregs[ichip*thisDetector->nChans+ichan]|=ival<<9;
		break;
	      case 2:
#ifdef VERBOSE
		//cout << " anen " << ival ;
#endif
		myMod->chanregs[ichip*thisDetector->nChans+ichan]|=ival<<8;
		break;
	      case 3:
#ifdef VERBOSE
		//cout << " calen " << ival  ;
#endif
		myMod->chanregs[ichip*thisDetector->nChans+ichan]|=ival<<7;
		break;
	      case 4:
#ifdef VERBOSE
		//cout << " outcomp " << ival  ;
#endif
		myMod->chanregs[ichip*thisDetector->nChans+ichan]|=ival<<10;
		break;
	      case 5:
#ifdef VERBOSE
		//cout << " counts " << ival  << endl;
#endif
		 myMod->chanregs[ichip*thisDetector->nChans+ichan]|=ival<<11;
		 break;
	      default:
		cout << " too many columns" << endl; 
		break;
	      }
	    }
	  }
	  //	}
      }
#ifdef VERBOSE
      cout << "read " << ichan*ichip << " channels" <<endl; 
#endif
      infile.close();
      return myMod;
    } else {
      cout << "could not open file " << endl;
      if (nflag)
	deleteModule(myMod);
      return NULL;
    }
 
};


int mythenDetector::writeTrimFile(string fname, sls_detector_module mod){

  ofstream outfile;
  string names[]={"Vtrim", "Vthresh", "Rgsh1", "Rgsh2", "Rgpr", "Vcal", "outBuffEnable"};
  int iv, ichan, ichip;
  int iv1, idac;
  int nb;
    outfile.open(fname.c_str(), ios_base::out);

    if (outfile.is_open()) {
      for (idac=0; idac<mod.ndac; idac++) {
	iv=mod.dacs[idac];
	outfile << names[idac] << " " << iv << endl;
      }
      
	for (ichip=0; ichip<mod.nchip; ichip++) {
	  iv1=mod.chipregs[ichip]&1;
	  outfile << names[idac] << " " << iv1 << endl;
	  for (ichan=0; ichan<thisDetector->nChans; ichan++) {
	    iv=mod.chanregs[ichip*thisDetector->nChans+ichan];
	    iv1= (iv&0x3f);
	    outfile <<iv1 << " ";
	    nb=9;
	    iv1=((iv&(1<<nb))>>nb);
	    outfile << iv1 << " ";
	    nb=8;
	    iv1=((iv&(1<<nb))>>nb);
	    outfile << iv1 << " ";
	    nb=7;
	    iv1=((iv&(1<<nb))>>nb);
	    outfile <<iv1  << " ";
	    nb=10;
	    iv1=((iv&(1<<nb))>>nb);
	    outfile << iv1 << " ";
	    nb=11;
	    iv1= ((iv&0xfffff800)>>nb);
	    outfile << iv1  << endl;
	  }
	}
      outfile.close();
      return OK;
    } else {
      cout << "could not open file " << fname << endl;
      return FAIL;
    }

};




int mythenDetector::writeTrimFile(string fname, int imod){

  return writeTrimFile(fname,detectorModules[imod]);

};


int mythenDetector::writeDataFile(string fname, float *data, float *err, float *ang, char dataformat, int nch){
  if (nch==-1)
    nch=thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;


  ofstream outfile;
  int idata;
  if (data==NULL)
    return FAIL;
#ifdef VERBOSE
  cout << "writing data to file " << fname << endl;
#endif
  //  args|=0x10; // one line per channel!

  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
  {
#ifdef VERBOSE
    cout << "Writing to file " << fname << endl;
#endif
    for (int ichan=0; ichan<nch; ichan++) {
      if (ang==NULL) {
	outfile << ichan << " ";
      } else {
	outfile << ang[ichan] << " ";
      }
	
      switch (dataformat) {
      case 'f':
	outfile << *(data+ichan)<< " ";
	break;
      case 'i':
      default:
	idata=*(data+ichan);
	outfile << idata << " ";
      }
     if (err) {
       outfile << *(err+ichan)<< " ";
     }
     //   if (args&0x10) {
       outfile << endl;
       // }
    }

    outfile.close();
    return OK;
  } else {
    cout << "Could not open file " << fname << "for writing"<< endl;
    return FAIL;
  }
};







/*writes raw data file */
int mythenDetector::writeDataFile(string fname, int *data){
  ofstream outfile;
  if (data==NULL)
    return FAIL;

  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
  {
#ifdef VERBOSE
    cout << "Writing to file " << fname << endl;
#endif
    for (int ichan=0; ichan<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods; ichan++)
      outfile << ichan << " " << *(data+ichan) << endl;
    outfile.close();
    return OK;
  } else {
    cout << "Could not open file " << fname << "for writing"<< endl;
    return FAIL;
  }
};





int mythenDetector::readDataFile(string fname, float *data, float *err, float *ang, char dataformat, int nch){


  ifstream infile;
  int ichan, iline=0;
  int interrupt=0;
  float fdata, ferr, fang;
  int maxchans;
  int ich;
  string str;

  if (nch==0)
    maxchans=thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;
  else if (nch<0)
    maxchans=0xfffffff;
  else
    maxchans=nch;

#ifdef VERBOSE
  cout << "Opening file "<< fname << endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    while (infile.good() and interrupt==0) {
      getline(infile,str);
#ifdef VERBOSE
      cout << str << endl;
#endif
      istringstream ssstr(str);
      if (ang==NULL) {
	ssstr >> ichan >> fdata;
	ich=ichan;
	if (ich!=iline) 
	  cout << "Channel number " << ichan << " does not match with line number " << iline << endl;
      } else {
	ssstr >> fang >> fdata;
	ich=iline;
      }
      if (!ssstr.good()) {
	interrupt=1;
	break;
      }
       if (err)
	 ssstr >> ferr;
      if (!ssstr.good()) {
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
     } else {
       cout << " too many lines in file: "<< iline << " instead of " << maxchans << endl;
       interrupt=1;
       break;
     }
    }
  } else {
    cout << "Could not read file " << fname << endl;
    return -1;
  }
  return iline;





};

int mythenDetector::readDataFile(string fname, int *data){

  ifstream infile;
  int ichan, idata, iline=0;
  int interrupt=0;
  string str;

#ifdef VERBOSE
  cout << "Opening file "<< fname << endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    while (infile.good() and interrupt==0) {
      getline(infile,str);
#ifdef VERBOSE
      cout << str << endl;
#endif
      istringstream ssstr(str);
      ssstr >> ichan >> idata;
      if (!ssstr.good()) {
	interrupt=1;
	break;
      }
      if (ichan!=iline) {
	cout << " Expected channel "<< iline <<" but read channel "<< ichan << endl;
	interrupt=1;
	break;
      } else {
	if (iline<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods) {
	  data[iline]=idata;
	  iline++;
	} else {
	  interrupt=1;
	  break;
	}
      }
    }
  } else {
    cout << "Could not read file " << fname << endl;
    return -1;
  }
  return iline;
};




int mythenDetector::readCalibrationFile(string fname, float &gain, float &offset){

  char line[500];
  string str;
  ifstream infile;
#ifdef VERBOSE
  cout << "Opening file "<< fname << endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    getline(infile,str);
#ifdef VERBOSE
    cout << str << endl;
#endif
    istringstream ssstr(str);
    ssstr >> offset >> gain;
  } else {
    cout << "Could not open calibration file "<< fname << endl;
    gain=0.;
    offset=0.;
    return -1;
  }
  return 0;
};
/*
int mythenDetector::writeCalibrationFile(string fname, float gain, float offset){};
*/
  /* Communication to server */



  // calibration functions
/*
  really needed?

int mythenDetector::setCalibration(int imod,  detectorSettings isettings, float gain, float offset){
  cout << "function not yet implemented " << endl; 
  
  

  return OK;

}
int mythenDetector::getCalibration(int imod,  detectorSettings isettings, float &gain, float &offset){

  cout << "function not yet implemented " << endl; 



}
*/

/*

int mythenDetector::setROI(int nroi, int *xmin, int *xmax, int *ymin, int *ymax){


};
*/

//Corrections

int mythenDetector::setAngularConversion(string fname) {
  if (fname=="") {
    thisDetector->correctionMask&=~(1<< ANGULAR_CONVERSION);
  } else {
    if (fname=="default")
      fname=thisDetector->angConvFile;
    if (readAngularConversion(fname)>=0) {
      thisDetector->correctionMask|=(1<< ANGULAR_CONVERSION);
      strcpy(thisDetector->angConvFile,fname.c_str());
    }
  }
  return thisDetector->correctionMask&(1<< ANGULAR_CONVERSION);
}

int mythenDetector::getAngularConversion(int &direction,  angleConversionConstant *angconv) {
  direction=thisDetector->angDirection;
    if (angconv) {
      for (int imod=0; imod<thisDetector->nMods; imod++) {
	(angconv+imod)->center=thisDetector->angOff[imod].center;
	(angconv+imod)->r_conversion=thisDetector->angOff[imod].r_conversion;
	(angconv+imod)->offset=thisDetector->angOff[imod].offset;
	(angconv+imod)->ecenter=thisDetector->angOff[imod].ecenter;
	(angconv+imod)->er_conversion=thisDetector->angOff[imod].er_conversion;
	(angconv+imod)->eoffset=thisDetector->angOff[imod].eoffset;
      }
    }
  if (thisDetector->correctionMask&(1<< ANGULAR_CONVERSION)) {
    return 1;
  } else {
    return 0;
  }
 
}



int mythenDetector::readAngularConversion(string fname) {
  char line[500];
  string str;
  ifstream infile;
  int mod;
  float center, ecenter;
  float r_conv, er_conv;
  float off, eoff;
  string ss;
  int interrupt=0;

  //" module %i center %E +- %E conversion %E +- %E offset %f +- %f \n"
#ifdef VERBOSE
  cout << "Opening file "<< fname << endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    while (infile.good() and interrupt==0) {
      getline(infile,str);
#ifdef VERBOSE
      cout << str << endl;
#endif
      istringstream ssstr(str);
      ssstr >> ss >> mod;
      ssstr >> ss >> center;
      ssstr >> ss >> ecenter;
      ssstr >> ss >> r_conv;
      ssstr >> ss >> er_conv;
      ssstr >> ss >> off;
      ssstr >> ss >> eoff;
      if (mod<thisDetector->nModsMax && mod>=0) {
	thisDetector->angOff[mod].center=center;
	thisDetector->angOff[mod].r_conversion=r_conv;
	thisDetector->angOff[mod].offset=off;
	thisDetector->angOff[mod].ecenter=ecenter;
	thisDetector->angOff[mod].er_conversion=er_conv;
	thisDetector->angOff[mod].eoffset=eoff;
      }
    }
  } else {
    cout << "Could not open calibration file "<< fname << endl;
    return -1;
  }
  return 0;
}


int mythenDetector:: writeAngularConversion(string fname) {
  ofstream outfile;
  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
  {  
    for (int imod=0; imod<thisDetector->nMods; imod++) {
      outfile << " module " << imod << " center "<< thisDetector->angOff[imod].center<<"  +- "<< thisDetector->angOff[imod].ecenter<<" conversion "<< thisDetector->angOff[imod].r_conversion << " +- "<< thisDetector->angOff[imod].er_conversion <<  " offset "<< thisDetector->angOff[imod].offset << " +- "<< thisDetector->angOff[imod].eoffset << endl;
    }
    outfile.close();
  } else {
    cout << "Could not open file " << fname << "for writing"<< endl;
    return -1;
  }
  //" module %i center %E +- %E conversion %E +- %E offset %f +- %f \n"
  return 0;
}

int mythenDetector::resetMerging(float *mp, float *mv, float *me, int *mm) {
  
  float binsize;
  if (thisDetector->binSize>0)
    binsize=thisDetector->binSize;
  else 
    return FAIL;
  
  for (int ibin=0; ibin<(360./binsize); ibin++) {
    mp[ibin]=0;
    mv[ibin]=0;
    me[ibin]=0;
    mm[ibin]=0;
  }
  return OK;
}


int mythenDetector::finalizeMerging(float *mp, float *mv, float *me, int *mm) {
  float binsize;
  int np=0;

  if (thisDetector->binSize>0)
    binsize=thisDetector->binSize;
  else 
    return FAIL;
  
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
  return OK;
}

int  mythenDetector::addToMerging(float *p1, float *v1, float *e1, float *mp, float *mv,float *me, int *mm) {

  float binsize;
  float binmi=-180., binma;
  int ibin=0;
  int imod;
  float ang;

  if (thisDetector->binSize>0)
    binsize=thisDetector->binSize;
  else 
    return FAIL;
  binmi=-180.;
  binma=binmi+binsize;
  

  if (thisDetector->angDirection>0) {
    for (int ip=0; ip<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods; ip++) {
      if (thisDetector->correctionMask&DISCARD_BAD_CHANNELS) {
	if (badChannelMask[ip])
	  continue;
      }
      imod=ip/(thisDetector->nChans*thisDetector->nChips);
      if (p1)
	ang=p1[ip];
      else
	ang=angle(ip,currentPosition,thisDetector->fineOffset+thisDetector->globalOffset,thisDetector->angOff[imod].r_conversion,thisDetector->angOff[imod].center, thisDetector->angOff[imod].offset,thisDetector->angOff[imod].tilt,thisDetector->angDirection);
      
      
      
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
    for (int ip=thisDetector->nChans*thisDetector->nChips*thisDetector->nMods-1; ip>=0; ip--) {
      if (thisDetector->correctionMask&(1<< DISCARD_BAD_CHANNELS)) {
	if (badChannelMask[ip])
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

void  mythenDetector::acquire(){
  
  int np=1;
  if (thisDetector->numberOfPositions>0) 
    np=thisDetector->numberOfPositions;

  currentPositionIndex=0;

  for (int ip=0; ip<np; ip++) {
    if  (thisDetector->numberOfPositions>0) {
      go_to_position (thisDetector->detPositions[ip]);
      currentPositionIndex=ip+1;
    }
    //write header before?
    startAndReadAll();
    if (thisDetector->correctionMask&(1<< I0_NORMALIZATION))
      currentI0=get_i0();
    //write header after?
    if (thisDetector->correctionMask&(1<< ANGULAR_CONVERSION))
      currentPosition=get_position();
    
    processData(); 
  }
}


void* mythenDetector::processData() {


  int *myData;
  float *fdata;
  //  float *dataout=new float[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods];
  float *rcdata=NULL, *rcerr=NULL;
  float *ffcdata=NULL, *ffcerr=NULL;
  float *ang=NULL;
  float bs=0.004;
  int i=0;
  int imod;
  int nb;
  int np;
  while(1) {
    
    
    if( !dataQueue.empty() ) {

      /** Pop data queue */
      myData=dataQueue.front(); // get the data from the queue 
      dataQueue.pop(); //remove the data from the queue
      if (myData) {
	//process data

	/** decode data */
	fdata=decodeData(myData);

	delete [] myData;
	myData=NULL;
	/** write raw data file */	   
	writeDataFile (createFileName().append(".raw"), fdata, NULL, NULL, 'i');
	
	/** rate correction */
	if (thisDetector->correctionMask&(1<<RATE_CORRECTION)) {
	  rcdata=new float[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods]; 
	  rcerr=new float[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods];
	  rateCorrect(fdata,NULL,rcdata,rcerr);
	  delete [] fdata;
	} else {
	  rcdata=fdata;
	  fdata=NULL;
	}
	
	/** flat field correction */
	if (thisDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION)) {
	  
	  ffcdata=new float[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods]; 
	  ffcerr=new float[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods];
	  flatFieldCorrect(rcdata,rcerr,ffcdata,ffcerr);
	  delete [] rcdata;
	  delete [] rcerr;
	} else {
	  ffcdata=rcdata;
	  ffcerr=rcerr;
	  rcdata=NULL;
	  rcerr=NULL;
	}

	if (thisDetector->correctionMask&(1<< ANGULAR_CONVERSION)) {
	  
	  /** angular conversion */
	  /** data merging */
	  // if (thisDetector->numberOfPositions) {
	    
	    if (currentPositionIndex<=1) {     
	      if (thisDetector->binSize>0)
		bs=thisDetector->binSize;
	      else if (thisDetector->angOff[0].r_conversion>0) {
		bs=180./PI*atan(thisDetector->angOff[0].r_conversion);
		thisDetector->binSize=bs;
	      } else
		thisDetector->binSize=bs;
	      
	      
	      nb=360./bs;
	      
	      mergingBins=new float[nb];
	      mergingCounts=new float[nb];
	      mergingErrors=new float[nb];
	      mergingMultiplicity=new int[nb];
	      
	      resetMerging(mergingBins, mergingCounts,mergingErrors, mergingMultiplicity);
	    }
	    addToMerging(ang, ffcdata, ffcerr, mergingBins, mergingCounts,mergingErrors, mergingMultiplicity);
	 
	    ang=new float[thisDetector->nChans*thisDetector->nChips*thisDetector->nMods]; 
	    for (int ip=0; ip<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods; ip++) {
	      imod=ip/(thisDetector->nChans*thisDetector->nChips);
	      ang[ip]=angle(ip,currentPosition,thisDetector->fineOffset+thisDetector->globalOffset,thisDetector->angOff[imod].r_conversion,thisDetector->angOff[imod].center, thisDetector->angOff[imod].offset,thisDetector->angOff[imod].tilt,thisDetector->angDirection);
	    }
	    writeDataFile (createFileName().append(".dat"), ffcdata, ffcerr,ang);
	    if (currentPositionIndex==thisDetector->numberOfPositions || (currentPositionIndex==0) {
	      
	      np=finalizeMerging(mergingBins, mergingCounts,mergingErrors, mergingMultiplicity);
	      
	      /** file writing */
	      currentPositionIndex++;
	      writeDataFile (createFileName().append(".dat"),mergingCounts, mergingErrors, mergingBins,'f',np);
	      
	      delete [] mergingBins;
	      delete [] mergingCounts;
	      delete [] mergingErrors;
	      delete [] mergingMultiplicity;
	      thisDetector->fileIndex++;

	    }
	    //}
	  
	} else {
	  writeDataFile (createFileName().append(".dat"), ffcdata, ffcerr);
	  thisDetector->fileIndex++;
	}

	if (ffcdata)
	  delete [] ffcdata;
	if (ffcerr)
	  delete [] ffcerr;
	if (ang)
	  delete [] ang;
	
      }
    }
  }
}


/*
void mythenDetector::startThread() {
  pthread_attr_t tattr, mattr;
  int ret;
  int newprio;
  sched_param param, mparam;
  void *arg;
  int policy= SCHED_OTHER;

  ret = pthread_attr_init(&tattr);

  // set the priority; others are unchanged
  //newprio = 30;
  mparam.sched_priority = 30;
  // scheduling parameters of main thread 
  ret = pthread_setschedparam(pthread_self(), policy, &mparam);


  printf("current priority is %d\n",param.sched_priority);
  ret = pthread_create(&dataProcessingThread, NULL,startProcessData, (void*)this);


  param.sched_priority = 1;
  // scheduling parameters of target thread
  ret = pthread_setschedparam(dataProcessingThread, policy, &param);


}

void* startProcessData(void *n) {

  //void* processData(void *n) {
  void *w;
  mythenDetector *myDet=(mythenDetector*)n;
  myDet->processData(0);
}
*/
