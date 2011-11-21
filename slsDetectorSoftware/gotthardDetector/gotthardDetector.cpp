#include "gotthardDetector.h"
#include <unistd.h>

#include <sstream>

//using namespace std;

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

int gotthardDetector::readConfigurationFile(string const fname){
  
  string ans;
  string str;
  ifstream infile;
  int iargval;
  int interrupt=0;
  char *args[100];
  for (int ia=0; ia<100; ia++) {
    args[ia]=new char[1000];
  }


  string sargname, sargval;
  int iline=0;
  std::cout<< "config file name "<< fname << std::endl;
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    while (infile.good() and interrupt==0) {
      sargname="none";
      sargval="0";
      getline(infile,str);
      iline++;
#ifdef VERBOSE
      std::cout<<  str << std::endl;
#endif
      if (str.find('#')!=string::npos) {
#ifdef VERBOSE
	std::cout<< "Line is a comment " << std::endl;
	std::cout<< str << std::endl;
#endif
	continue;
      } else if (str.length()<2) {
#ifdef VERBOSE
	std::cout<< "Empty line " << std::endl;
#endif
	continue;
      } else {
	istringstream ssstr(str);
	iargval=0;
	while (ssstr.good()) {
	  ssstr >> sargname;
	  //if (ssstr.good()) {
#ifdef VERBOSE 
	    std::cout<< iargval << " " << sargname  << std::endl;
#endif
	    strcpy(args[iargval],sargname.c_str());
	    iargval++;
	    //}
	}
	ans=executeLine(iargval,args,PUT_ACTION);
#ifdef VERBOSE 
	std::cout<< ans << std::endl;
#endif
      }
      iline++;
    }
    infile.close();
  } else {
    std::cout<< "Error opening configuration file " << fname << " for reading" << std::endl;
    return FAIL;
  }
#ifdef VERBOSE
  std::cout<< "Read configuration file of " << iline << " lines" << std::endl;
#endif
  return iline;
};


int gotthardDetector::writeConfigurationFile(string const fname){
  
  string names[]={\
    "hostname",\
    "caldir",\
    "settingsdir",\
    "trimen",\
    "outdir",\
    "ffdir",\
    "headerbefore",\
    "headerafter",\
    "headerbeforepar",\
    "headerafterpar",\
    "nmod",\
    "badchannels",\
    "angconv",\
    "globaloff",\
    "binsize",\
    "threaded",\
    "waitstates",\
    "setlength",\
    "clkdivider"};
  int nvar=19;
  ofstream outfile;
  int iv=0;
  char *args[100];
  for (int ia=0; ia<100; ia++) {
    args[ia]=new char[1000];
  }


  outfile.open(fname.c_str(),ios_base::out);
  if (outfile.is_open()) {
    for (iv=0; iv<nvar; iv++) {
      strcpy(args[0],names[iv].c_str());
      outfile << names[iv] << " " << executeLine(1,args,GET_ACTION) << std::endl;
    }
    outfile.close();
  }
  else {
    std::cout<< "Error opening configuration file " << fname << " for writing" << std::endl;
    return FAIL;
  }
#ifdef VERBOSE
  std::cout<< "wrote " <<iv << " lines to configuration file " << std::endl;
#endif
  return iv;
};
  /* 
     It should be possible to dump all the settings of the detector (including trimbits, threshold energy, gating/triggering, acquisition time etc.
     in a file and retrieve it for repeating the measurement with identicals ettings, if necessary
  */
int gotthardDetector::dumpDetectorSetup(string fname, int level){  
  string names[]={
    "fname",\
    "index",\
    "flags",\
    "dr",\
    "settings",\
    "threshold",\
    "exptime",\
    "period",\
    "delay",\
    "gates",\
    "frames",\
    "cycles",\
    "probes",\
    "fineoff",\
    "ratecorr",\
    "startscript",\
    "startscriptpar",\
    "stopscript",\
    "stopscriptpar",\
    "scriptbefore",\
    "scriptbeforepar",\
    "scriptafter",\
    "scriptafterpar",\
    "headerbefore",\
    "headerbeforepar",\
    "headerafter",\
    "headerafterpar",\
    "scan0script",\
    "scan0par",\
    "scan0prec",\
    "scan0steps",\
    "scan1script",\
    "scan1par",\
    "scan1prec",\
    "scan1steps",\
    "flatfield",\
    "badchannels",\
    "angconv",\
    "trimbits",\
    "extsig"
  };
  int nvar=40;
  int iv=0;
  string fname1;
  ofstream outfile;
  char *args[2];
  for (int ia=0; ia<2; ia++) {
    args[ia]=new char[1000];
  }
  int nargs;
  if (level==2)
    nargs=2;
  else
    nargs=1;


  if (level==2) {
    fname1=fname+string(".config");
    writeConfigurationFile(fname1);
    fname1=fname+string(".det");
  } else
    fname1=fname;

  outfile.open(fname1.c_str(),ios_base::out);
  if (outfile.is_open()) {
    for (iv=0; iv<nvar-5; iv++) {
      strcpy(args[0],names[iv].c_str());
      outfile << names[iv] << " " << executeLine(1,args,GET_ACTION) << std::endl;
    }


    strcpy(args[0],names[iv].c_str());
    if (level==2) {
      fname1=fname+string(".ff");
      strcpy(args[1],fname1.c_str());
    }
    outfile << names[iv] << " " << executeLine(nargs,args,GET_ACTION) << std::endl;
    iv++;

    strcpy(args[0],names[iv].c_str());
    if (level==2) {
      fname1=fname+string(".bad");
      strcpy(args[1],fname1.c_str());
    }
    outfile << names[iv] << " " << executeLine(nargs,args,GET_ACTION) << std::endl;
    iv++;

      
    strcpy(args[0],names[iv].c_str());
    if (level==2) {
      fname1=fname+string(".angoff");
      strcpy(args[1],fname1.c_str());
    }
    outfile << names[iv] << " " << executeLine(nargs,args,GET_ACTION) << std::endl;
    iv++;
    
    strcpy(args[0],names[iv].c_str());
    if (level==2) {
      size_t c=fname.rfind('/');
      if (c<string::npos) {
	fname1=fname.substr(0,c+1)+string("trim_")+fname.substr(c+1);
      } else {
	fname1=string("trim_")+fname;
      }
      strcpy(args[1],fname1.c_str());
#ifdef VERBOSE
      std::cout<< "writing to file " << fname1 << std::endl;
#endif
    }
    outfile << names[iv] << " " << executeLine(nargs,args,GET_ACTION) << std::endl;
    iv++;
    
    for (int is=0; is<4; is++) {
      sprintf(args[0],"%s:%d",names[iv].c_str(),is);
      outfile << args[0] << " " << executeLine(1,args,GET_ACTION) << std::endl;	
    }
    iv++;
    outfile.close();
  }
  else {
    std::cout<< "Error opening parameters file " << fname1 << " for writing" << std::endl;
    return FAIL;
  }
  
#ifdef VERBOSE
  std::cout<< "wrote " <<iv << " lines to  "<< fname1 << std::endl;
#endif
  return 0;
}



int gotthardDetector::retrieveDetectorSetup(string fname1, int level){ 

   
   string fname;
   string str;
   ifstream infile;
   int iargval;
   int interrupt=0;
  char *args[2];
  for (int ia=0; ia<2; ia++) {
    args[ia]=new char[1000];
  }
  string sargname, sargval;
  int iline=0;
  
  if (level==2) {
    fname=fname1+string(".config");
    readConfigurationFile(fname);
    //cout << "config file read" << endl;
    fname=fname1+string(".det");
  }  else
    fname=fname1;
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    while (infile.good() and interrupt==0) {
      sargname="none";
      sargval="0";
      getline(infile,str);
      iline++;
#ifdef VERBOSE
      std::cout<<  str << std::endl;
#endif
      if (str.find('#')!=string::npos) {
#ifdef VERBOSE
	std::cout<< "Line is a comment " << std::endl;
	std::cout<< str << std::endl;
#endif
	continue;
      } else {
	istringstream ssstr(str);
	iargval=0;
	while (ssstr.good()) {
	  ssstr >> sargname;
	  //  if (ssstr.good()) {
	    strcpy(args[iargval],sargname.c_str());
#ifdef VERBOSE
      std::cout<< args[iargval]  << std::endl;
#endif
	    iargval++;
	    // }
	}
	if (level==2) {
	  executeLine(iargval,args,PUT_ACTION);
	} else {
	  if (string(args[0])==string("flatfield"))
	    ;
	  else if  (string(args[0])==string("badchannels"))
	    ;
	  else if  (string(args[0])==string("angconv"))
	    ;
	  else if (string(args[0])==string("trimbits"))
	    ;
	  else
	    executeLine(iargval,args,PUT_ACTION);
	}
      }
      iline++;
    }
    infile.close();
  } else {
    std::cout<< "Error opening  " << fname << " for reading" << std::endl;
    return FAIL;
  }
#ifdef VERBOSE
  std::cout<< "Read  " << iline << " lines" << std::endl;
#endif
  return iline;

};



  /* I/O */


 sls_detector_module* gotthardDetector::readSettingsFile(string fname,  sls_detector_module *myMod){
 
   int nflag=0;

   if (myMod==NULL) {
     myMod=createModule();
     nflag=1;
   }
  string myfname;
  string str;
  ifstream infile;
  ostringstream oss;
  int iline=0;
  string sargname,sargname2;
  int ival;
  int ichan=0, ichip=0, idac=0;
  string::size_type pos=0;

 

#ifdef VERBOSE
  std::cout<<   "reading settings file for module number "<< myMod->module << std::endl;
#endif
      myfname=fname;
#ifdef VERBOSE
    std::cout<< "settings file name is "<< myfname <<   std::endl;
#endif
    infile.open(myfname.c_str(), ios_base::in);
    if (infile.is_open()) {
      //dacs---------------
	for (int iarg=0; iarg<thisDetector->nDacs; iarg++) {
	  getline(infile,str);
	  iline++;
#ifdef VERBOSE
	  std::cout<< str << std::endl;
#endif
	  istringstream ssstr(str);
	  ssstr >> sargname >> ival;
#ifdef VERBOSE
	  std::cout<< sargname << " dac nr. " << idac << " is " << ival << std::endl;
#endif
	  myMod->dacs[idac]=ival;
	  idac++;
	}

	//config---------------
	  getline(infile,str);
	  iline++;
#ifdef VERBOSE
	  std::cout<< str << std::endl;
#endif
	  istringstream ssstr(str);
	  ssstr >> sargname >> ival;
#ifdef VERBOSE
	  std::cout<< sargname << " is " << ival << std::endl;
#endif
	   int configval = ival;//myMod->dacs[idac]=ival;

	   //HV---------------
	  getline(infile,str);
	  iline++;
#ifdef VERBOSE
	  std::cout<< str << std::endl;
#endif
	  ssstr.str(str);
	  ssstr >> sargname >> ival;
#ifdef VERBOSE
	  std::cout<< sargname << " is " << ival << std::endl;
#endif
	   int HVval = ival;//myMod->dacs[idac]=ival;

	   //mac address----------
	  getline(infile,str);
	  iline++;
#ifdef VERBOSE
	  std::cout<< str << std::endl;
#endif
	  istringstream sstr(str);
	  sstr >> sargname >> sargname2;
#ifdef VERBOSE
	  std::cout<< sargname << " is " << sargname2 << std::endl;
#endif
	  //getting rid of dots
	  pos = sargname2.find(".");
	  while(pos != string::npos)
	    {
	      sargname2.erase( pos, 1 );
	      pos = sargname2.find(".");
	    }
	  strcpy(thisDetector->clientMacAddress,sargname2.c_str());
	  cout<<"macaddress:"<<thisDetector->clientMacAddress<<endl;

	  //ip address---------------
	  getline(infile,str);
	  iline++;
#ifdef VERBOSE
	  std::cout<< str << std::endl;
#endif
	   istringstream sssstr(str);
	  sssstr >> sargname >> sargname2;
#ifdef VERBOSE
	  std::cout<< sargname << " is " << sargname2 << std::endl;
#endif
	  //getting rid of dots
	  pos = sargname2.find(".");
	  while(pos != string::npos)
	    {
	      sargname2.erase( pos, 1 );
	      pos = sargname2.find(".");
	    }
	  strcpy(thisDetector->clientIPAddress,sargname2.c_str());
	  cout<<"ipaddress:"<<thisDetector->clientIPAddress<<endl;



      infile.close();
      strcpy(thisDetector->settingsFile,fname.c_str());
      return myMod;
    } else {
      std::cout<< "could not open settings file " <<  myfname << std::endl;
      if (nflag)
	deleteModule(myMod);
      return NULL;
    }
 
};


int gotthardDetector::writeSettingsFile(string fname, sls_detector_module mod){
  
  ofstream outfile;
  string names[]={"Vref", "VcascN","VcascP", "Vout", "Vcasc", "Vin", "Vref_comp", "Vib_test", "config", "HV", "macaddress","ipaddress"};
  int iv, ichan, ichip;
  int iv1, idac;
  int nb;
    outfile.open(fname.c_str(), ios_base::out);

    if (outfile.is_open()) {
      for (idac=0; idac<mod.ndac; idac++) {
	iv=(int)mod.dacs[idac];
	outfile << names[idac] << " " << iv << std::endl;
      }
      
	for (ichip=0; ichip<mod.nchip; ichip++) {
	  iv1=mod.chipregs[ichip]&1;
	  outfile << names[idac] << " " << iv1 << std::endl;
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
	    outfile << iv1  << std::endl;
	  }
	}
      outfile.close();
      return OK;
    } else {
      std::cout<< "could not open trim file " << fname << std::endl;
      return FAIL;
    }
};




int gotthardDetector::writeSettingsFile(string fname, int imod){

  return writeSettingsFile(fname,detectorModules[imod]);

};


int gotthardDetector::writeDataFile(string fname, float *data, float *err, float *ang, char dataformat, int nch){

  if (nch==-1)
    nch=thisDetector->nChans*thisDetector->nChips*thisDetector->nMods;


  ofstream outfile;
  int idata;
  if (data==NULL)
    return FAIL;

  //  args|=0x10; // one line per channel!

  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
  {
#ifdef VERBOSE
    std::cout<< "writeDataFile Writing to file " << fname << std::endl;
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

    outfile.close();
    return OK;
  } else {
    std::cout<< "Could not open file " << fname << "for writing"<< std::endl;
    return FAIL;
  }
};







/*writes raw data file */
int gotthardDetector::writeDataFile(string fname, int *data){

  ofstream outfile;
  if (data==NULL)
    return FAIL;

  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
  {
#ifdef VERBOSE
    std::cout<< "Writing to file " << fname << std::endl;
#endif
    for (int ichan=0; ichan<thisDetector->nChans*thisDetector->nChips*thisDetector->nMods; ichan++)
      outfile << ichan << " " << *(data+ichan) << std::endl;
    outfile.close();
    return OK;
  } else {
    std::cout<< "Could not open file " << fname << "for writing"<< std::endl;
    return FAIL;
  }
};





int gotthardDetector::readDataFile(string fname, float *data, float *err, float *ang, char dataformat, int nch){


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
  std::cout<< "Opening file "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    while (infile.good() and interrupt==0) {
      getline(infile,str);
#ifdef VERBOSE
      std::cout<< str << std::endl;
#endif
      istringstream ssstr(str);
      if (ang==NULL) {
	ssstr >> ichan >> fdata;
	ich=ichan;
	if (!ssstr.good()) {
	  interrupt=1;
	  break;
	}
	if (ich!=iline) 
	  std::cout<< "Channel number " << ichan << " does not match with line number " << iline << " " << dataformat << std::endl;
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
       std::cout<< " too many lines in file: "<< iline << " instead of " << maxchans << std::endl;
       interrupt=1;
       break;
     }
    }
  } else {
    std::cout<< "Could not read file " << fname << std::endl;
    return -1;
  }
  return iline;




};

int gotthardDetector::readDataFile(string fname, int *data){
  ifstream infile;
  int ichan, idata, iline=0;
  int interrupt=0;
  string str;

#ifdef VERBOSE
  std::cout<< "Opening file "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    while (infile.good() and interrupt==0) {
      getline(infile,str);
#ifdef VERBOSE
      std::cout<< str << std::endl;
#endif
      istringstream ssstr(str);
      ssstr >> ichan >> idata;
      if (!ssstr.good()) {
	interrupt=1;
	break;
      }
      if (ichan!=iline) {
	std::cout<< " Expected channel "<< iline <<" but read channel "<< ichan << std::endl;
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
    std::cout<< "Could not read file " << fname << std::endl;
    return -1;
  }
  return iline;
};




int gotthardDetector::readCalibrationFile(string fname, float &gain, float &offset){

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
  } else {
    std::cout<< "Could not open calibration file "<< fname << std::endl;
    gain=0.;
    offset=0.;
    return -1;
  }
  return 0;

};

int gotthardDetector::writeCalibrationFile(string fname, float gain, float offset){

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


