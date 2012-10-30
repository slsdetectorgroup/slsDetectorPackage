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
int		getFramesCaught();
int 	getFrameIndex();

void* 	startListening(void *arg);

int startReceiver();
int stopReceiver();


#endif
