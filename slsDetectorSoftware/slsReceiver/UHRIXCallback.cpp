
#include "UHRIXCallback.h"


int UHRIXCallbackDataFunc(char* d, int np, FILE* fd,  void* p){
	int i;

	//#ifdef VERBOSE
	//printf("UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU Receiver Data received \n");
	if (d==NULL)
		printf("no data received\n");
	else{
	//	printf("received %d bytes of data\n",np);
	//	printf("index:%d\n",(int)(*(int*)d));
		for ( i=0; i<(np-4)/2; i++)
		{
			d[i*2+4]=i;
			d[i*2+5]=0;
		}
	}

	//printf("UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU Finished \n");
	//#endif
	return 0;
}
