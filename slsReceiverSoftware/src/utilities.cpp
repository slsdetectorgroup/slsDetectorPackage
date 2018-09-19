#include <iostream>
#include <string>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <fstream>

#include <map>

#include "utilities.h"
#include "logger.h"




int read_config_file(std::string fname, int *tcpip_port_no, std::map<std::string, std::string> * configuration_map ){
	
	std::ifstream infile;
	std::string sLine,sargname, sargvalue;
	int iline = 0;
	int success = slsReceiverDefs::OK;


	FILE_LOG(logINFO) << "config file name " << fname;
	try {
		infile.open(fname.c_str(), std::ios_base::in);
	} catch(...) {
		FILE_LOG(logERROR) << "Could not open configuration file " << fname ;
		success = slsReceiverDefs::FAIL;
	}

	if (success == slsReceiverDefs::OK  && infile.is_open()) {
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
							success = slsReceiverDefs::FAIL;
						}
					}
				}
			}
		}
		infile.close();
	}
	
	return success;
}




