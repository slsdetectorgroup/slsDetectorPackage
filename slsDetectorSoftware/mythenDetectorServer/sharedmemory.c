#include "sharedmemory.h"

struct statusdata *stdata;

int inism(int clsv) {

static int scansmid;

    if (clsv==SMSV) {
	if ( (scansmid=shmget(SMKEY,1024,IPC_CREAT | 0666 ))==-1 ) {
	    return -1;
	}
	if ( (stdata=shmat(scansmid,NULL,0))==(void*)-1) {
	    return -2;
	}
    }

    if (clsv==SMCL) {
	if ( (scansmid=shmget(SMKEY,0,0) )==-1 ) {
	    return -3;
	}
	if ( (stdata=shmat(scansmid,NULL,0))==(void*)-1) {
	    return -4;
	}
    }
    return 1;
}

void write_status_sm(char *status) {
  strcpy(stdata->status,status);
}

void write_istatus_sm(int i) {
  stdata->istatus=i;
}
int read_istatus_sm() {
  return stdata->istatus;
}

void write_stop_sm(int v) {
  stdata->stop=v;
}

void write_runnumber_sm(int v) {
  stdata->runnumber=v;
}
