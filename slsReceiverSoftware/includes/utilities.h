#include <iostream>
#include <string>
#include <sstream>
#include <iostream>
using namespace std;

/* uncomment next line to enable debug output */
//#define EIGER_DEBUG

/* macro for debug output http://stackoverflow.com/a/14256296 */
#ifdef EIGER_DEBUG
#define DEBUG(x) do { std::cerr << x << std::endl; } while (0)
#else
#define DEBUG(x)
#endif

#ifdef VERBOSE
#define VERBOSE_PRINT(x) do { std::cout << "[VERBOSE]" << x << std::endl; } while (0)
#else
#define VERBOSE_PRINT(x)
#endif






inline int read_config_file(string fname, int *tcpip_port_no);

