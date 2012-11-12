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

#endif
