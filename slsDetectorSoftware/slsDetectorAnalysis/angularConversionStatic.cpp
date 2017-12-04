#include "angularConversionStatic.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include "angleConversionConstant.h"

#include "sls_detector_defs.h"
#include "angleFunction.h"
using namespace std;

angularConversionStatic::angularConversionStatic()
{
  //angleFunctionPointer=0;
  registerAngleFunctionCallback(&defaultAngleFunction);

}

angularConversionStatic::~angularConversionStatic(){

}



double* angularConversionStatic::convertAngles(double pos, int nch, int *chansPerMod, angleConversionConstant **angOff, int *mF, double fo, double go, int angdir) {

  //  cout << "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP convert angles " << endl;

  int imod=0;
  double    *ang=new double[nch];
  double enc=pos;
  angleConversionConstant *p=NULL;
  
  int ch0=0;
  int chlast=chansPerMod[0]-1;
  int nchmod=chansPerMod[0];
  p=angOff[imod];      
  if (mF[imod]==0)
    enc=0;
  else
    enc=pos;

  for (int ip=0; ip<nch; ip++) {
#ifdef VERBOSE
    cout << "ip " << ip << " ch0 " << ch0 << " chlast " << chlast << " imod " << imod << endl;
#endif
    if (ip>chlast) {
      imod++; 
      p=angOff[imod];      
      if (mF[imod]==0)
	enc=0;
      else
	enc=pos;
      
#ifdef VERBOSE
    if (p) 
      cout <<  enc			<< endl <<     fo+go << endl << 		    p->r_conversion << endl 	<<	    p->center				<< endl <<	    p->offset << endl << 		    p->tilt << 		    angdir	<< endl;
    else
      cout << "no ang conv " << endl;
#endif

      ch0=chlast+1;
      nchmod=chansPerMod[imod];
      if (nchmod>0)
	chlast=ch0+nchmod-1;
    }
    
    if (p)
      ang[ip]=angle(ip-ch0,				\
		    enc,				\
		    fo+go,				\
		    p->r_conversion,			\
		    p->center,				\
		    p->offset,				\
		    p->tilt,				\
		    angdir		  );
#ifdef VERBOSE
    cout << "ip " << ip << " ch0 " << ch0 << " chlast " << chlast << " imod " << imod << endl;
#endif
  }
  return ang;
}






double angularConversionStatic::convertAngle(double pos, int ich, angleConversionConstant *p, int mF, double fo, double go, int angdir) {

  //  cout << "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP convert angle " << endl;
  // if (p)
  //   cout <<  pos			<< endl <<     fo+go << endl << 		    p->r_conversion << endl 	<<	    p->center				<< endl <<	    p->offset << endl << 		    mF <<  endl <<		    angdir	<< endl;
  //   else
  //     cout << "no ang conv " << endl;

  double enc=0, trans=0;
  double  ang;

  switch (mF) {
  case 0:
    enc=0;
    trans=0;
    break;
  case 1:
    enc=pos;
    trans=0;
    break;
  case -1:
    enc=-pos;
    trans=0;
    break;
  case 2:
    enc=0;
    trans=pos;
    break;
  case -2:
    enc=0;
    trans=-pos;
    break;
  default:
    enc=0;
    trans=0;
  }

  if (p)
    ang=angle(ich,				\
	      enc,					\
	      fo+go,					\
	      p->r_conversion,				\
	      p->center,				\
	      p->offset,				\
	      trans,					\
	      angdir		  );
  //  cout << ich << " " << ang << endl << endl;
  return ang;
  


}











double angularConversionStatic::convertAngle(double pos, int ich, int *chansPerMod, angleConversionConstant **angOff, int *mF, double fo, double go, int angdir) {

  //  cout << "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP convert angles xx" << endl;
  int imod=0;
  double    ang;
  // double enc=0, trans=0;
  angleConversionConstant *p=NULL;
  
  int ch0=0;
  int chlast=chansPerMod[0]-1;
  int nchmod=chansPerMod[0];

  

  while (ich>chlast) {
    imod++;
    ch0=chlast+1;
    nchmod=chansPerMod[imod];
    chlast=ch0+nchmod-1;
  }
  
  p=angOff[imod];
  

  ang=convertAngle(pos, ich-ch0, p, mF[imod], fo, go, angdir);

  return ang;

}



//static!
int angularConversionStatic::readAngularConversion(string fname, int nmod, angleConversionConstant *angOff) {

  ifstream infile;
  string ss;

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


//static
int angularConversionStatic::readAngularConversion( ifstream& infile, int nmod, angleConversionConstant *angOff) {
  string str;
  int mod;
  double center, ecenter, pitch, epitch;
  double r_conv, er_conv;
  double off, eoff;
  string ss;
  int interrupt=0;
  int nm=0;
  int newangconv=0;
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
    if (ss==string("center")) 
      newangconv=0;
    else
      newangconv=1;
    ssstr >> ss >> ecenter;
    if (newangconv) {
      ssstr >> ss >> pitch;
      ssstr >> ss >> epitch;
    }
    ssstr >> ss >> r_conv;
    ssstr >> ss >> er_conv;
    ssstr >> ss >> off;
    ssstr >> ss >> eoff;
#ifdef VERBOSE
    cout << nm << " " << nmod << endl;
#endif
    if (nm<nmod && nm>=0 ) {
	angOff[nm].center=center;
	angOff[nm].r_conversion=r_conv;
	angOff[nm].offset=off;
	angOff[nm].ecenter=ecenter;
	angOff[nm].er_conversion=er_conv;
	angOff[nm].eoffset=eoff;

      if (newangconv!=0) {
	//  } else {

	angOff[nm].tilt=pitch;
	angOff[nm].etilt=epitch;

      }
	// cout << 	angOff[nm].center << " " <<
	// angOff[nm].r_conversion << " " <<
	//   angOff[nm].offset << endl;

    } else
      break;
#ifdef VERBOSE
    cout << nm<<"  " << angOff[nm].offset << endl;
#endif
    nm++;
    if (nm>=nmod)
      break;

    


  }
  return nm;
 }

//static
int angularConversionStatic:: writeAngularConversion(string fname, int nmod, angleConversionConstant *angOff) {

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



//static 
int angularConversionStatic:: writeAngularConversion(ofstream& outfile, int nmod, angleConversionConstant *angOff) {

  for (int imod=0; imod<nmod; imod++) {
      outfile << " module " << imod << " center "<< angOff[imod].center<<"  +- "<< angOff[imod].ecenter<<" conversion "<< angOff[imod].r_conversion << " +- "<< angOff[imod].er_conversion <<  " offset "<< angOff[imod].offset << " +- "<< angOff[imod].eoffset << std::endl;
  }
  return 0;
}


//static
int angularConversionStatic::resetMerging(double *mp, double *mv, double *me, int *mm, int nb) {
  
  //  cout << "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP reset merging " << endl;
		
#ifdef VERBOSE
  cout << "creating merging arrays "<<  nb << endl;
#endif
  

  for (int ibin=0; ibin<nb; ibin++) {
    mp[ibin]=0;
    mv[ibin]=0;
    me[ibin]=0;
    mm[ibin]=0;
  }
  return slsDetectorDefs::OK;
}


//static
int angularConversionStatic::finalizeMerging(double *mp, double *mv, double *me, int *mm,int nb) {
  //  cout << "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP finalize merging " << endl;
   int np=0;
   for (int ibin=0; ibin<nb; ibin++) {
     if (mm[ibin]>0) {
 #ifdef VERBOSE 
       cout << "finalize " << ibin << "  "<< mm[ibin] << " " << mp[ibin]<< " " << mv[ibin] << " " << me[ibin] << endl;
 #endif
       mp[np]=mp[ibin]/mm[ibin];
       mv[np]=mv[ibin]/mm[ibin];
       me[np]=me[ibin]/mm[ibin];
       me[np]=sqrt(me[ibin]);
       mm[np]=mm[ibin];
       np++;
     }
   } 
   //  cout << endl ;
  return np;
}

//static
int  angularConversionStatic::addToMerging(double *p1, double *v1, double *e1, double *mp, double *mv,double *me, int *mm, int nchans, double binsize,int nbins, int *badChanMask ) {


  //  cout << "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP add to merging " << endl;
  double binmi=-180.;
  int ibin=0;

  if (p1==NULL)
    return 0;
  if (v1==NULL)
    return slsDetectorDefs::FAIL;

  if (mp==NULL) //can be changed if we want to use a fixed bin algorithm!
    return slsDetectorDefs::FAIL;

  if (mv==NULL)
    return slsDetectorDefs::FAIL;
  if (me==NULL)
    return slsDetectorDefs::FAIL;
  if (mm==NULL)
    return slsDetectorDefs::FAIL;
  if (nchans==0)
    return slsDetectorDefs::FAIL;
  
  if (binsize<=0)
    return slsDetectorDefs::FAIL;

  if (nbins<=0)
    return slsDetectorDefs::FAIL;
  
  for (int ip=0; ip<nchans; ip++) {
    if (badChanMask) {
      if (badChanMask[ip]) {
#ifdef VERBOSE
	cout << "channel " << ip << " is bad " << endl;
#endif
	  continue;
      }
    }
    ibin=(int)((p1[ip]-binmi)/binsize);
   
 
    if (ibin<nbins && ibin>=0) {
      mp[ibin]+=p1[ip];
      mv[ibin]+=v1[ip];
      if (e1)
	me[ibin]+=(e1[ip]*e1[ip]);
      else
	me[ibin]+=v1[ip];
      mm[ibin]++;

// #ifdef VERBOSE
//       cout << "add " << ibin << "  "<< mm[ibin] << " " << mp[ibin]<< mv[ibin] << me[ibin] << endl;
// #endif
    } else
      return slsDetectorDefs::FAIL;
  }
  

  return slsDetectorDefs::OK;
  
}

int  angularConversionStatic::addPointToMerging(double p1, double v1, double e1, double *mp, double *mv,double *me, int *mm,  double binsize,int nbins) {


  //  cout << "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP add point to merging "<< v1 << " " << e1 << endl;
  double binmi=-180.;
  int ibin=0;


  if (mp==NULL) //can be changed if we want to use a fixed bin algorithm!
    return slsDetectorDefs::FAIL;

  if (mv==NULL)
    return slsDetectorDefs::FAIL;
  if (me==NULL)
    return slsDetectorDefs::FAIL;
  if (mm==NULL)
    return slsDetectorDefs::FAIL;
  
  if (binsize<=0)
    return slsDetectorDefs::FAIL;

  if (nbins<=0)
    return slsDetectorDefs::FAIL;
  

    ibin=(int)((p1-binmi)/binsize);
   
 
    if (ibin<nbins && ibin>=0) {
      //  cout << "before " << ibin << " " <<  mp[ibin] << " " << mv[ibin] << " " << me[ibin] << endl;
      mp[ibin]+=p1;
      mv[ibin]+=v1;
      if (e1)
	me[ibin]+=(e1*e1);
      else
	me[ibin]+=v1;
      mm[ibin]++;
      //   cout << "after " << ibin << " " <<  mp[ibin] << " " << mv[ibin] << " " << me[ibin] << endl;

// #ifdef VERBOSE
//       cout << "add " << ibin << "  "<< mm[ibin] << " " << mp[ibin]<< mv[ibin] << me[ibin] << endl;
// #endif
    }  else {
      cout << "Bin out of range! " << ibin << endl;
      return slsDetectorDefs::FAIL;
    }

  return slsDetectorDefs::OK;
  
}
