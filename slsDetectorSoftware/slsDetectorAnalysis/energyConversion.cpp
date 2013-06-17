#include "energyConversion.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>


using namespace std;




int energyConversion::readCalibrationFile(string fname, double &gain, double &offset){

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
#ifndef MYROOT
    return FAIL;
#endif
    return -1;
  }
#ifndef MYROOT
  return OK;
#endif
  return 0;
};

int energyConversion::writeCalibrationFile(string fname, double gain, double offset){
  //std::cout<< "Function not yet implemented " << std::endl;
  ofstream outfile;

  outfile.open (fname.c_str());

  // >> i/o operations here <<
  if (outfile.is_open()) {
    outfile << offset << " " << gain << std::endl;
  } else {
    std::cout<< "Could not open calibration file "<< fname << " for writing" << std::endl;
#ifndef MYROOT
    return FAIL;
#endif
    return -1;
  }

  outfile.close();

#ifndef MYROOT
  return OK;
#endif
  return 0;
};


#ifndef MYROOT

/* I/O */


slsDetectorDefs::sls_detector_module* energyConversion::readSettingsFile(string fname,  detectorType myDetectorType, sls_detector_module *myMod){

	int nflag=0;


	if (myMod==NULL) {
		myMod=createModule(myDetectorType);
		nflag=1;
	}

	int id=0,i;
	string names[100];
	string myfname;
	string str;
	ifstream infile;
	ostringstream oss;
	int iline=0;
	string sargname;
	int ival;
	int ichan=0, ichip=0, idac=0;
	int nch=((myMod->nchan)/(myMod->nchip));


	//to verify dac
	switch (myDetectorType) {
	case MYTHEN:
		break;
	case MOENCH:
	case GOTTHARD:
		names[id++]="Vref";
		names[id++]="VcascN";
		names[id++]="VcascP";
		names[id++]="Vout";
		names[id++]="Vcasc";
		names[id++]="Vin";
		names[id++]="Vref_comp";
		names[id++]="Vib_test";
		break;
	case EIGER:
		names[id++]="SvP";
		names[id++]="SvN";
		names[id++]="Vtr";
		names[id++]="Vrf";
		names[id++]="Vrs";
		names[id++]="Vin";
		names[id++]="Vtgstv";
		names[id++]="Vcmp_ll";
		names[id++]="Vcmp_lr";
		names[id++]="cal";
		names[id++]="Vcmp_rl";
		names[id++]="Vcmp_rr";
		names[id++]="rxb_rb";
		names[id++]="rxb_lb";
		names[id++]="Vcmp_lr";
		names[id++]="Vcp";
		names[id++]="Vcmp_rl";
		names[id++]="Vcn";
		names[id++]="Vis";
		names[id++]="iodelay";
		break;
	default:
		cout << "Unknown detector type - unknown format for settings file" << endl;
		return NULL;
	}

#ifdef VERBOSE
	std::cout<<   "reading settings file for module number "<< myMod->module << std::endl;
#endif
	myfname=fname;
#ifdef VERBOSE
	std::cout<< "file name is "<< myfname <<   std::endl;
#endif
	infile.open(myfname.c_str(), ios_base::in);
	if (infile.is_open()) {


		switch (myDetectorType) {

		case MYTHEN:

			for (int iarg=0; iarg<myMod->ndac; iarg++) {
				getline(infile,str);
				iline++;
				istringstream ssstr(str);
				ssstr >> sargname >> ival;
#ifdef VERBOSE
				std::cout<< sargname << " dac nr. " << idac << " is " << ival << std::endl;
#endif
				myMod->dacs[idac]=ival;
				idac++;
			}
			for (ichip=0; ichip<myMod->nchip; ichip++) {
				getline(infile,str);
				iline++;
#ifdef VERBOSE
				//	  std::cout<< str << std::endl;
#endif
				istringstream ssstr(str);
				ssstr >> sargname >> ival;
#ifdef VERBOSE
				//	  std::cout<< "chip " << ichip << " " << sargname << " is " << ival << std::endl;
#endif

				myMod->chipregs[ichip]=ival;
				for (ichan=0; ichan<nch; ichan++) {
					getline(infile,str);
#ifdef VERBOSE
					// std::cout<< str << std::endl;
#endif
					istringstream ssstr(str);

#ifdef VERBOSE
					//   std::cout<< "channel " << ichan+ichip*thisDetector->nChans <<" iline " << iline<< std::endl;
#endif
					iline++;
					myMod->chanregs[ichip*nch+ichan]=0;
					for (int iarg=0; iarg<6 ; iarg++) {
						ssstr >>  ival;
						//if (ssstr.good()) {
						switch (iarg) {
						case 0:
#ifdef VERBOSE
							//		 std::cout<< "trimbits " << ival ;
#endif
							myMod->chanregs[ichip*nch+ichan]|=ival&TRIMBITMASK;
							break;
						case 1:
#ifdef VERBOSE
							//std::cout<< " compen " << ival ;
#endif
							myMod->chanregs[ichip*nch+ichan]|=ival<<9;
							break;
						case 2:
#ifdef VERBOSE
							//std::cout<< " anen " << ival ;
#endif
							myMod->chanregs[ichip*nch+ichan]|=ival<<8;
							break;
						case 3:
#ifdef VERBOSE
							//std::cout<< " calen " << ival  ;
#endif
							myMod->chanregs[ichip*nch+ichan]|=ival<<7;
							break;
						case 4:
#ifdef VERBOSE
							//std::cout<< " outcomp " << ival  ;
#endif
							myMod->chanregs[ichip*nch+ichan]|=ival<<10;
							break;
						case 5:
#ifdef VERBOSE
							//std::cout<< " counts " << ival  << std::endl;
#endif
							myMod->chanregs[ichip*nch+ichan]|=ival<<11;
							break;
						default:
							std::cout<< " too many columns" << std::endl;
							break;
						}
					}
				}
				//	}
			}
#ifdef VERBOSE
			std::cout<< "read " << ichan*ichip << " channels" <<std::endl;
#endif


			break;

		case EIGER:
		case MOENCH:
		case GOTTHARD:
			//---------------dacs---------------
			while(infile.good()) {
				getline(infile,str);
				iline++;
#ifdef VERBOSE
				std::cout<< str << std::endl;
#endif
				istringstream ssstr(str);
				ssstr >> sargname >> ival;
				for (i=0;i<id;i++){
					if (!strcasecmp(sargname.c_str(),names[i].c_str())){
						myMod->dacs[i]=ival;
						idac++;
#ifdef VERBOSE
						std::cout<< sargname << " dac nr. " << idac << " is " << ival << std::endl;
#endif
						break;
					}
				}
				if (i < id) {
#ifdef VERBOSE
					std::cout<< sargname << " dac nr. " << idac << " is " << ival << std::endl;
#endif
				}else{
					std::cout<< "Unknown dac " << sargname << std::endl;
					infile.close();
					deleteModule(myMod);
					return NULL;
				}
			}
			break;


		default:
			std::cout<< "Unknown detector type - don't know how to read file" <<  myfname << std::endl;
			infile.close();
			deleteModule(myMod);
			return NULL;

		}

		infile.close();
		strcpy(settingsFile,fname.c_str());
		return myMod;

	} else {
		std::cout<< "could not open settings file " <<  myfname << std::endl;

		if (nflag)
			deleteModule(myMod);

		return NULL;
	}

};


int energyConversion::writeSettingsFile(string fname, detectorType myDetectorType, sls_detector_module mod){

  ofstream outfile;

  int nch=((mod.nchan)/(mod.nchip));

  string names[100];
  int id=0;
  switch (myDetectorType) {
  case MYTHEN:
    names[id++]="Vtrim";
    names[id++]="Vthresh"; 
    names[id++]="Rgsh1";
    names[id++]="Rgsh2";
    names[id++]="Rgpr";
    names[id++]="Vcal";
    names[id++]="outBuffEnable";
    break;
  case MOENCH:
  case GOTTHARD:
    names[id++]="Vref";
    names[id++]="VcascN";
    names[id++]="VcascP";
    names[id++]="Vout";
    names[id++]="Vcasc";
    names[id++]="Vin";
    names[id++]="Vref_comp";
    names[id++]="Vib_test";
    break;
  case EIGER:
	names[id++]="SvP";
	names[id++]="SvN";
	names[id++]="Vtr";
	names[id++]="Vrf";
	names[id++]="Vrs";
	names[id++]="Vin";
	names[id++]="Vtgstv";
	names[id++]="Vcmp_ll";
	names[id++]="Vcmp_lr";
	names[id++]="cal";
	names[id++]="Vcmp_rl";
	names[id++]="Vcmp_rr";
	names[id++]="rxb_rb";
	names[id++]="rxb_lb";
	names[id++]="Vcmp_lr";
	names[id++]="Vcp";
	names[id++]="Vcmp_rl";
	names[id++]="Vcn";
	names[id++]="Vis";
	names[id++]="iodelay";
	break;
  default:
    cout << "Unknown detector type - unknown format for settings file" << endl;
    return FAIL;
  }

  int iv, ichan, ichip;
  int iv1, idac;
  int nb;
  outfile.open(fname.c_str(), ios_base::out);

  if (outfile.is_open()) {
    for (idac=0; idac<mod.ndac; idac++) {
      iv=(int)mod.dacs[idac];
      outfile << names[idac] << " " << iv << std::endl;
    }
      
    if((myDetectorType!=GOTTHARD)&&(myDetectorType!=MOENCH)&&(myDetectorType!=EIGER)){
      for (ichip=0; ichip<mod.nchip; ichip++) {
	iv1=mod.chipregs[ichip]&1;
	outfile << names[idac] << " " << iv1 << std::endl;
	for (ichan=0; ichan<nch; ichan++) {
	  iv=mod.chanregs[ichip*nch+ichan];
	  iv1= (iv&TRIMBITMASK);
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
    }
    outfile.close();
    return OK;
  } else {
    std::cout<< "could not open SETTINGS file " << fname << std::endl;
    return FAIL;
  }

};



#endif
