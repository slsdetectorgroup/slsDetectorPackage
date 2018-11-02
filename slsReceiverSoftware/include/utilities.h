#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <iostream>
#include <map>

#include "sls_receiver_defs.h"

/* uncomment next line to enable debug output */
//#define EIGER_DEBUG


int read_config_file(std::string fname, int *tcpip_port_no, std::map<std::string, std::string> * configuration_map);

