#ifdef SLS_RECEIVER_FUNCTION_LIST


//int initializeReceiver();

void 	closeFile();

enum runStatus getReceiverStatus();

char*	getFileName();
char* 	setFileName(char fName[]);
char*	getFilePath();
char*	setFilePath(char fName[]);
int		getFileIndex();
int 	setFileIndex(int index);
int 	getFrameIndex();
int 	getAcquisitionIndex();
int		getFramesCaught();
int		getTotalFramesCaught();
int 	resetTotalFramesCaught(int index);

void* 	startListening(void *arg);

int startReceiver();
int stopReceiver();

char* readFrame();

//int setUDPPortNumber(int p=-1); //sets/gets port number to listen to for data from the detector
//int setTCPPortNumber(int p=-1); //sets/get port number for communication to client


#endif
