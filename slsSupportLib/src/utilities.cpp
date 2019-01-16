#include "utilities.h"
#include "logger.h"

#include <iostream>
#include <sstream>




int read_config_file(std::string fname, int *tcpip_port_no,
		std::map<std::string, std::string> * configuration_map) {

	std::ifstream infile;
	std::string sLine,sargname, sargvalue;
	int iline = 0;
	int success = slsDetectorDefs::OK;


	FILE_LOG(logINFO) << "config file name " << fname;
	try {
		infile.open(fname.c_str(), std::ios_base::in);
	} catch(...) {
		FILE_LOG(logERROR) << "Could not open configuration file " << fname ;
		success = slsDetectorDefs::FAIL;
	}

	if (success == slsDetectorDefs::OK  && infile.is_open()) {
		while(infile.good()){
			getline(infile,sLine);
			iline++;

			//VERBOSE_PRINT(sLine);

			if(sLine.find('#') != std::string::npos)
				continue;

			else if(sLine.length()<2)
				continue;

			else{
				std::istringstream sstr(sLine);

				//parameter name
				if(sstr.good()){
					sstr >> sargname;

					if (! sstr.good())
						continue;

					sstr >> sargvalue;
					(*configuration_map)[sargname] = sargvalue;
				}
				//tcp port
				if(sargname=="rx_tcpport"){
					if(sstr.good()) {
						sstr >> sargname;
						if(sscanf(sargname.c_str(),"%d",tcpip_port_no))
							cprintf(RESET, "dataport: %d\n" , *tcpip_port_no);
						else{
							cprintf(RED, "could not decode port in config file. Exiting.\n");
							success = slsDetectorDefs::FAIL;
						}
					}
				}
			}
		}
		infile.close();
	}

	return success;
}



int readDataFile(std::ifstream &infile, short int *data, int nch, int offset) {
	int ichan, iline=0;
	short int idata;
	int interrupt=0;
	std::string str;
	while (infile.good() and interrupt==0) {
		getline(infile,str);
		std::istringstream ssstr(str);
		ssstr >> ichan >> idata;
		if (ssstr.fail() || ssstr.bad()) {
			interrupt=1;
			break;
		}
		if (iline<nch) {
			if (ichan>=offset) {
				data[iline]=idata;
				iline++;
			}
		} else {
			interrupt=1;
			break;
		}
		return iline;
	};
	return iline;
}



int readDataFile(std::string fname, short int *data, int nch) {
	std::ifstream infile;
	int iline=0;
	std::string str;
	infile.open(fname.c_str(), std::ios_base::in);
	if (infile.is_open()) {
		iline=readDataFile(infile, data, nch, 0);
		infile.close();
	} else {
		FILE_LOG(logERROR) << "Could not read file " << fname;
		return -1;
	}
	return iline;
}



int writeDataFile(std::ofstream &outfile,int nch,  short int *data, int offset) {
	if (data==nullptr)
		return slsDetectorDefs::FAIL;
	for (int ichan=0; ichan<nch; ichan++)
		outfile << ichan+offset << " " << *(data+ichan) << std::endl;
	return slsDetectorDefs::OK;
}


int writeDataFile(std::string fname,int nch, short int *data) {
	std::ofstream outfile;
	if (data==nullptr)
		return slsDetectorDefs::FAIL;
	outfile.open (fname.c_str(),std::ios_base::out);
	if (outfile.is_open())       {
		writeDataFile(outfile, nch, data, 0);
		outfile.close();
		return slsDetectorDefs::OK;
	} else {
		FILE_LOG(logERROR) << "Could not open file " << fname << "for writing";
		return slsDetectorDefs::FAIL;
	}
}





