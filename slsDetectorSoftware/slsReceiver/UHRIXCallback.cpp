
#include "UHRIXCallback.h"


int UHRIXCallbackDataFunc(char* d, int np, FILE* fd,  void* p){
	//#ifdef VERBOSE
	printf("UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU Receiver Data received \n");
	if (d==NULL)
		printf("no data received\n");
	else{
		printf("received %d bytes of data\n",np);
		printf("index:%d\n",(int)(*(int*)d));
	}

	printf("UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU Finished \n");
	//#endif
	return 0;
}
