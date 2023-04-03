
/**************************************************************************/
/* Header files   section needs cleanup                     */
/**************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "sls/ZmqSocket.h"

#include "sls/tiffIO.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>  /* exit() */
#include <string.h>  /* memset(), memcpy() */
#include <sys/utsname.h>   /* uname() */
#include <sys/types.h>
#include <sys/socket.h>   /* socket(), bind(),
			     listen(), accept() */
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>  /* fork(), write(), close() */
#include <time.h> 
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cmath>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rapidjson/document.h> //json header in zmq stream
#include <omp.h>
#define NTHREADS 2

#include <chrono>
#include <cstdio>
#include <ctime> // time_t

using namespace std;
using namespace std::chrono;
using namespace sls;

#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TH2F.h"
#include "TMath.h"
#include "TFile.h"
#include "TStyle.h"
#include "TBox.h"
#include "TSystem.h"
#include "TTimer.h"
#include "TProfile.h"
#include "TColor.h"
#include <iostream>
#include <fstream> 
#include <termios.h>
#include <TApplication.h>
#include <stdio.h>
#include <math.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include "sls/ansi.h"
#define SLS_DETECTOR_JSON_HEADER_VERSION 0x4

#define PI 3.14159265

#define FALSE              0
#define OFFSET 0
#define NBIN 500  
#define MIN_POS  -500.5   // 400.5   
#define MAX_POS 8499.5   //-100.5


#define NCH 524288
// #define NCH 262144 in case of half_frames 

char serverip[256];
int portnum;
FILE * sfilefd;

short* hchptr; // photon counted map "histogram"
int value,oldvalue;
float factor=1.84;
int npacket=0;
int totalnpacket=0;
float vgamma;
 
struct sockaddr_in serveraddr;
struct sockaddr_in clientaddr;
struct in_addr      inadr;
struct hostent      *server;
 
int i=0;
int ipx=0;
int ipy=0;
bool haveconnection;


TStyle *gStyle;
TApplication* rootapp;
TCanvas *A2;
TCanvas *A3;
TPad *p1;
TPad *p2;
TPad *p3;
TPad *p4;
TPad *p5;
TPad *p6;
TCanvas *A4;
TCanvas *A5;
TCanvas *A6;

TH1I **hchip;
TH2F **h4500chip;
short image_data[NCH*2];
short imaged[NCH*2];
float fpeded[NCH*2];
float fpedeG2d[NCH*2];
float fpedeG1d[NCH*2];
short ipeded[NCH*2];
short pcimaged[NCH*2];

float fgaind[NCH*2];


float  adcpedecorr,adcpedecorrold;

bool gain_flag;
bool bw_flag;
bool fill2Ds;
bool show2Ds;
bool fill1Ds;
bool pede_flag;
bool dophotonmap;

      typedef struct {
        uint16_t xmin;
     	uint16_t xmax;
     	uint16_t ymin;
     	uint16_t ymax;
      } receriverRoi_compact;   
     receriverRoi_compact croi;
TBox *box;
uint16_t ROIxmin;
uint16_t ROIxmax;
uint16_t ROIymin;
uint16_t ROIymax;     
int nx, ny;
int nframes;
int goout;
int framesinstream;
int ifp;
float threshold;
int phene;
int  adcvalue;
int gain;
int ichip;
int frameIndex_old;

char pedefilename[128];
int  framenum,bunchid;

TH2F* his1000;TH2F* his1001;TH2F* his1002;
TH2F* his1060;TH2F* his1061;TH2F* his1062;
TH2F* his2000;
TH2F* his3000;
TH2F* his4500;
TH1I* hproj;
TH1I* hchcum;


using namespace std;
void printhelp(void);
void processifp(int ifp);
void historeset(void);
void SetRanges(void);
void startsocket(void);
void stopsocket(void);
void axisreset(void);
int kbhit(void);
void myloop(void);   
void loadpede(void);
void loadallpede(void);
void loadgain(void);
void nonblock(int state);
void LoadPaletteFalse(void);
void LoadPaletteBW(float);
void Plot1DHistos(void);
void Plot2DHistos(void);
void savepede(void);
void readpede(void);
int findinterpoindex(int startindex);
int findclumax(int startindex);
void tryconnect(void) ;

#define NB_ENABLE 1 
#define NB_DISABLE 0
char c; 
int HDraw_every;


float  oldh0xfirst,oldh0xlast;
int idx;
int GXPoffset,G1Poffset,G2Poffset;
int ix,iy;
int adcmin,adcmax;
int pmmin,pmmax; //min/mnx for the photon map
bool fixranges;


sls::ZmqSocket *zmqSocket= NULL;
