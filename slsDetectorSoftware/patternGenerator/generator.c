/****************************************************************************
usage to generate a patter test.pat from test.p

gcc -DINFILE="\"test.p\"" -DOUTFILE="\"test.pat\"" -o test.exe generator.c ; ./test.exe ; rm test.exe


*************************************************************************/

#include <stdlib.h>  
#include <stdint.h>  
#include <string.h>  
#include <sys/utsname.h>   
#include <sys/types.h>
#include <unistd.h>  
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAXLOOPS 3
#define MAXTIMERS 3
#define MAXWORDS 1024



uint64_t pat=0;
uint64_t iopat=0;
uint64_t clkpat=0;

int iaddr=0;
int waitaddr[3]={-1,-1,-1};
int startloopaddr[3]={-1,-1,-1}; 
int stoploopaddr[3]={-1,-1,-1}; 
int start=0, stop=0;
uint64_t waittime[3]={0,0,0};
int nloop[3]={0,0,0};

char infile[10000], outfile[10000];

FILE *fd, *fd1;
uint64_t PAT[MAXWORDS];


int i,ii,iii,j,jj,jjj,pixx,pixy,memx,memy,muxout,memclk,colclk,rowclk,muxclk,memcol,memrow,loopcounter;

void setstart() {
  start=iaddr;
}

void setstop() {
  stop=iaddr;
}

void setinput(int bit) {
  uint64_t mask=1;
  mask=mask<<bit;
  iopat &= ~mask;
}

void setoutput(int bit) {
  uint64_t mask=1;
  mask=mask<<bit;
  iopat |= mask;
}

void setclk(int bit) {
  uint64_t mask=1;
  mask=mask<<bit;
  iopat |= mask;
  clkpat |= mask;
}

void clearbit(int bit){
  uint64_t mask=1;
  mask=mask<<bit;
  pat &= ~mask;
}
void setbit(int bit){
  uint64_t mask=1;
  mask=mask<<bit;
  pat |= mask;
}

int checkbit(int bit) {
  uint64_t mask=1;
  mask=mask<<bit;
  return (pat & mask ) >>bit;
}

void setstartloop(int iloop) {
  if (iloop>=0 && iloop<MAXLOOPS)
    startloopaddr[iloop]=iaddr;
}


void setstoploop(int iloop) {
  if (iloop>=0 && iloop<MAXLOOPS)
    stoploopaddr[iloop]=iaddr;
}


void setnloop(int iloop, int n) {
  if (iloop>=0 && iloop<MAXLOOPS)
    nloop[iloop]=n;
}

void setwaitpoint(int iloop) {
  if (iloop>=0 && iloop<MAXTIMERS)
    waitaddr[iloop]=iaddr;
}


void setwaittime(int iloop, uint64_t t) {
  if (iloop>=0 && iloop<MAXTIMERS)
    waittime[iloop]=t;
}




void pw(){
  if (iaddr<MAXWORDS)
    PAT[iaddr]= pat;
  fprintf(fd,"patword %04x %016llx\n",iaddr, pat);
  iaddr++;
  if (iaddr>=MAXWORDS) printf("ERROR: too many word in the pattern (%d instead of %d)!",iaddr, MAXWORDS);
}




main(void) {
  int iloop=0;
  fd=fopen(OUTFILE,"w");
#include INFILE

  fprintf(fd,"patioctrl %016llx\n",iopat);
  fprintf(fd,"patclkctrl %016llx\n",clkpat);
  fprintf(fd,"patlimits %04x %04x\n",start, stop);

  for (iloop=0; iloop<MAXLOOPS; iloop++) {
    fprintf(fd,"patloop%d %04x %04x\n",iloop, startloopaddr[iloop], stoploopaddr[iloop]);
    if ( startloopaddr[iloop]<0 || stoploopaddr[iloop]<= startloopaddr[iloop]) nloop[iloop]=0;
    fprintf(fd,"patnloop%d %d\n",iloop, nloop[iloop]);
  }

  for (iloop=0; iloop<MAXTIMERS; iloop++) {
    fprintf(fd,"patwait%d %04x\n",iloop, waitaddr[iloop]);
    if (waitaddr[iloop]<0) waittime[iloop]=0;
    fprintf(fd,"patwaittime%d %lld\n",iloop, waittime[iloop]);
  }
  
  close((int)fd);
  fd1=fopen(OUTFILEBIN,"w");
  fwrite(PAT,sizeof(uint64_t),iaddr, fd1);
  close((int)fd1);
}
