#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <iostream>
#include <map>

using namespace std;
#include "sls_receiver_defs.h"

/* uncomment next line to enable debug output */
//#define EIGER_DEBUG


int read_config_file(string fname, int *tcpip_port_no, map<string, string> * configuration_map);

