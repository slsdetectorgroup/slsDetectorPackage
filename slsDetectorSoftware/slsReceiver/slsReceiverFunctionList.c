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


char buffer[BUFFER_LENGTH];
int sd = -1;
int sockfd, sfilefd;



char filePath[MAX_STR_LENGTH]="";
char fileName[MAX_STR_LENGTH]="run";
int fileIndex=0;
int frameIndex=0;
int startFrameIndex=-1;
int framesInFile=0;
int framesCaught=0;

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





int getFramesCaught(){
	return framesCaught;
}

int getFrameIndex(){
	if(startFrameIndex==-1)
		frameIndex=0;
	else
		frameIndex=((int)(*((int*)buffer)) - startFrameIndex)/2;
	return frameIndex;
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
	char buffer2[BUFFER_LENGTH];
	char savefilename[128];
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;

	int clientaddrlen = sizeof(clientaddr);
	framesInFile=0;
	framesCaught=0;
	frameIndex = 0;
	startFrameIndex=-1;

	/***********************************************************************/
	/* Catch signal SIGINT to close files properly                         */
	/***********************************************************************/
	signal(SIGINT, closeFile);


	//create file name
	sprintf(savefilename, "%s/%s_f%09d_%d.dat", filePath,fileName,frameIndex,fileIndex);

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
				sprintf(savefilename, "%s/%s_f%09d_%d.dat", filePath,fileName,frameIndex,fileIndex);

				printf("saving to %s\t\tpacket loss %f \%\t\tframenum %d\n", savefilename,((currframenum-prevframenum-(2*framesInFile))/(double)(2*framesInFile))*100.000,currframenum);
				sfilefd = fopen((const char *) (savefilename), "w");
				prevframenum=currframenum;
				framesInFile = 0;
			}
			status = RUNNING;

			rc1 = recvfrom(sd, buffer, sizeof(buffer), 0,
					(struct sockaddr *) &clientaddr, &clientaddrlen);
			//printf("rc1 done\n");
			rc2 = recvfrom(sd, buffer2, sizeof(buffer2), 0,
					(struct sockaddr *) &clientaddr, &clientaddrlen);

			if(startFrameIndex==-1){
				startFrameIndex=(int)(*((int*)buffer))-2;
				prevframenum=startFrameIndex;
			}
			//printf("rc2 done\n");
			if ((rc1 < 0) || (rc2 < 0)) {
				perror("recvfrom() failed");
				break;
			}

			//so that it doesnt write the last frame twice
			if(gui_acquisition_thread_running){
				fwrite(buffer, 1, rc1, sfilefd);
				fwrite(buffer2, 1, rc2, sfilefd);
				framesInFile++;
				framesCaught++;
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
	printf("sfield:%d\n",sfilefd);


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




#endif
