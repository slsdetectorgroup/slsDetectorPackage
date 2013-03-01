
#include "UHRIXCallback.h"


int UHRIXCallbackDataFunc(char* d, int np, FILE* fd,  void* p){
	int i,j,jmax=6;
	u_int16_t	da;		
	
	
	//#ifdef VERBOSE
	//printf("UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU Receiver Data received \n");
	if (d==NULL)
		printf("no data received\n");
	else{
	//	printf("received %d bytes of data\n",np);
	//	printf("index:%d\n",(int)(*(int*)d));
		for ( i=0; i<(np-4)/2-jmax+1; i++)
		{
			//((int16_t *)d)[i+2] -= ((int16_t *)d)[i+3];
			//((int16_t *)d)[i+2] *= ((int16_t *)d)[i+2];
			for (j=1; j<jmax; j++)	((int16_t *)d)[i+2] += ((int16_t *)d)[i+2+j];
		}
	}

	//printf("UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU Finished \n");
	//#endif
	return 0;
}
