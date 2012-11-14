#ifdef SLS_RECEIVER_FUNCTION_LIST
#include "slsReceiverFunctionList.h"

#include "sls_detector_defs.h"

#include <stdio.h>
#include <stdlib.h>/* exit() */
#include <string.h>  /* memset(), memcpy() */
#include <sys/mman.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h> /* fork(), write(), close() */
#include <asm/page.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/socket.h>/* socket(), bind(), listen(), accept() */
#include <signal.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/utsname.h>   /* uname() */

#include <pthread.h> /* thread */

//constants
#define MAX_BUFLEN   1048576
#define TCP_PORT_NUMBER  2233

#define SRVNAME "localhost"
#define SERVER_PORT     50001
#define BUFFER_LENGTH    1286
#define FALSE              0

int         gui_acquisition_thread_running = 0;
int			err = 0;
pthread_t   gui_acquisition_thread;


char buffer[BUFFER_LENGTH*2];
char sendbuffer[BUFFER_LENGTH*2];

char onebuffer[BUFFER_LENGTH];
int sd = -1;
int sockfd;

FILE *sfilefd;



char filePath[MAX_STR_LENGTH]="";
char fileName[MAX_STR_LENGTH]="run";
int fileIndex=0;
int frameIndexNeeded=1;

//for each scan
int frameIndex=0;
int startFrameIndex=-1;
int framesCaught=0;

//for each acquisition
int acquisitionIndex=0;
int startAcquisitionIndex=-1;//to remember progress for scans
int totalFramesCaught=0;

int framesInFile=0;//to know when to start next file





enum runStatus status = IDLE;



void closeFile(){
	if(gui_acquisition_thread_running){
		printf("Closing file\n");
		fclose(sfilefd);
	}
	exit(0);
}


enum runStatus getReceiverStatus(){
#ifdef VERBOSE
	printf("Status:%d\n",status);
#endif
	return status;
}




char*	getFileName(){
	return fileName;
}

char* setFileName(char fName[]){
	if(strlen(fName)){
		strcpy(fileName,fName);
	}
	return getFileName();
}



char*	getFilePath(){
	return filePath;
}

char* setFilePath(char fPath[]){
	if(strlen(fPath)){
		/*check if filepath exists and chop off last '/'*/
		struct stat st;
		if(stat(fPath,&st) == 0)
			strcpy(filePath,fPath);
	}
	return getFilePath();
}






int	getFileIndex(){
	return fileIndex;
}

int setFileIndex(int index){
	if(index>=0){
		fileIndex=index;
	}
	return getFileIndex();
}







int getFrameIndex(){
	if(startFrameIndex==-1)
		frameIndex=0;
	else
		frameIndex=((int)(*((int*)buffer)) - startFrameIndex)/2;
	return frameIndex;
}


int getAcquisitionIndex(){
	if(startAcquisitionIndex==-1)
		acquisitionIndex=0;
	else
		acquisitionIndex=((int)(*((int*)buffer)) - startAcquisitionIndex)/2;
	return acquisitionIndex;
}



int getFramesCaught(){
	return framesCaught;
}


int getTotalFramesCaught(){
	return totalFramesCaught;
}


int resetTotalFramesCaught(int index){
	startAcquisitionIndex=-1;
	totalFramesCaught=0;
	frameIndexNeeded=index;
	return frameIndexNeeded;
}



void* startListening(void *arg){
#ifdef VERYVERBOSE
	printf("In startListening()\n");
#endif
	/***********************************************************************/
	/* Variable and structure definitions.                                 */
	/***********************************************************************/
	sd = -1;
	int rc1, rc2, rc;
	int currframenum, prevframenum;
//	char buffer2[BUFFER_LENGTH];
	char savefilename[128];
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;

	socklen_t clientaddrlen = sizeof(clientaddr);
	framesInFile=0;
	frameIndex=0;
	startFrameIndex=-1;
	framesCaught=0;


	/***********************************************************************/
	/* Catch signal SIGINT to close files properly                         */
	/***********************************************************************/
	signal(SIGINT, closeFile);


	//create file name
	if(!frameIndexNeeded)
		sprintf(savefilename, "%s/%s_%d.dat", filePath,fileName,fileIndex);
	else
		sprintf(savefilename, "%s/%s_f%012d_%d.dat", filePath,fileName,framesCaught,fileIndex);

	/***********************************************************************/
	/* A do/while(FALSE) loop is used to make error cleanup easier.  The   */
	/* close() of each of the socket descriptors is only done once at the  */
	/* very end of the program.                                            */
	/***********************************************************************/
	do {
		/********************************************************************/
		/* The socket() function returns a socket descriptor, which represents   */
		/* an endpoint.  The statement also identifies that the INET        */
		/* (Internet Protocol) address family with the UDP transport        */
		/* (SOCK_DGRAM) will be used for this socket.                       */
		/********************************************************************/
		sd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sd < 0) {
			perror("socket() failed");
			break;
		}

		/********************************************************************/
		/* After the socket descriptor is created, a bind() function gets a */
		/* unique name for the socket.  In this example, the user sets the  */
		/* s_addr to zero, which means that the UDP port of 3555 will be    */
		/* bound to all IP addresses on the system.                         */
		/********************************************************************/
		memset(&serveraddr, 0, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(SERVER_PORT);
		//serveraddr.sin_addr.s_addr = inet_addr(server_ip);
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

		rc = bind(sd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));
		if (rc < 0) {
			perror("bind() failed");
			break;
		}


		sfilefd = fopen((const char *) (savefilename), "w");
		printf("Saving to  ... %s. Ready! \n", savefilename);

		while (gui_acquisition_thread_running) {
			/********************************************************************/
			/* The server uses the recvfrom() function to receive that data.    */
			/* The recvfrom() function waits indefinitely for data to arrive.   */
			/********************************************************************/


			if (framesInFile == 20000) {
				fclose(sfilefd);

				currframenum=(int)(*((int*)buffer));
				getFrameIndex();
				//create file name
				if(!frameIndexNeeded)
					sprintf(savefilename, "%s/%s_%d.dat", filePath,fileName,fileIndex);
				else
					sprintf(savefilename, "%s/%s_f%012d_%d.dat", filePath,fileName,framesCaught,fileIndex);

				printf("saving to %s\t\tpacket loss %f %%\t\tframenum %d\n", savefilename,((currframenum-prevframenum-(2*framesInFile))/(double)(2*framesInFile))*100.000,currframenum);
				sfilefd = fopen((const char *) (savefilename), "w");
				prevframenum=currframenum;
				framesInFile = 0;
			}
			status = RUNNING;

			rc1 = recvfrom(sd, buffer, sizeof(onebuffer), 0,
					(struct sockaddr *) &clientaddr, &clientaddrlen);
			//printf("rc1 done\n");
			rc2 = recvfrom(sd, buffer+sizeof(onebuffer), sizeof(onebuffer), 0,
					(struct sockaddr *) &clientaddr, &clientaddrlen);


			//for each scan
			if(startFrameIndex==-1){
				startFrameIndex=(int)(*((int*)buffer))-2;
				prevframenum=startFrameIndex;
			}
			//start of acquisition
			if(startAcquisitionIndex==-1)
				startAcquisitionIndex=startFrameIndex;

			//printf("rc2 done\n");
			if ((rc1 < 0) || (rc2 < 0)) {
				perror("recvfrom() failed");
				break;
			}

			//so that it doesnt write the last frame twice
			if(gui_acquisition_thread_running){
				fwrite(buffer, 1, rc1, sfilefd);
				fwrite(buffer+sizeof(onebuffer), 1, rc2, sfilefd);
				framesInFile++;
				framesCaught++;
				totalFramesCaught++;
				//printf("saving\n");
			}
		}
	} while (gui_acquisition_thread_running);
	gui_acquisition_thread_running=0;
	status = IDLE;


	/***********************************************************************/
	/* Close down any open socket descriptors                              */
	/***********************************************************************/
	if (sd != -1){
		printf("Closing sd\n");fflush(stdout);
		close(sd);
	}

	//close file
	fclose(sfilefd);
	printf("sfield:%d\n",(int)sfilefd);


	return NULL;
}









int startReceiver(){
#ifdef VERBOSE
	printf("Starting Receiver\n");
#endif

	if(!gui_acquisition_thread_running){
		printf("Starting new acquisition threadddd ....\n");
		gui_acquisition_thread_running=1;
		//status = RUNNING;
		err = pthread_create(&gui_acquisition_thread, NULL,&startListening, NULL);
		if(!err){
			while(status!=RUNNING);
			printf("\n Thread created successfully.\n");
		}else{
			gui_acquisition_thread_running=0;
			status = IDLE;
			printf("\n Cant create thread. Status:%d\n",status);
			return FAIL;
		}
	}

	return OK;
}





int stopReceiver(){
#ifdef VERBOSE
	printf("Stopping Receiver\n");
#endif

	if(gui_acquisition_thread_running){
		printf("Stopping new acquisition threadddd ....\n");
		//stop thread
		gui_acquisition_thread_running=0;
		if (sd != -1)
			shutdown(sd, SHUT_RDWR);
		pthread_join(gui_acquisition_thread,NULL);
		status = IDLE;
	}
	printf("Status:%d\n",status);

	return OK;
}


char* readFrame(){
	int i;
	for(i=0;i<10;i++){
		if ((((int)*((int*)buffer))%2)!=0)
			break;
		else
			usleep(20000);
	}
	//printf("freamenum%d\n",*((int*) sendbuffer));
	return buffer;
}

#endif
