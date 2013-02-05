

#include "sls_detector_defs.h"

#include "slsDetector_firmware.h"
#include "slsDetectorServer_defs.h"
#include "registers.h"


#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/mman.h>		//PROT_READ,PROT_WRITE,MAP_FILE,MAP_SHARED,MAP_FAILED
#include <fcntl.h>			//O_RDWR


u_int32_t CSP0BASE;
u_int32_t fifo_control_reg;


int nModBoard;
int nModY		=	NMAXMOD;
int nModX		=	NMAXMOD;
int dynamicRange=	DYNAMIC_RANGE;
int dataBytes	=	NMAXMOD*NCHIP*NCHAN*2;
int masterMode	=	NO_MASTER;
int syncMode	=	NO_SYNCHRONIZATION;
int timingMode	=	AUTO_TIMING;


#ifdef SLS_DETECTOR_FUNCTION_LIST
extern const int nChans;
extern const int nChips;
extern const int nDacs;
extern const int nAdcs;
#endif
#ifndef SLS_DETECTOR_FUNCTION_LIST
const int nChans	=	NCHAN;
const int nChips	=	NCHIP;
const int nDacs		=	NDAC;
const int nAdcs		=	NADC;
#endif



int64_t dummy=0;

/* Gerd example
	if ((fd=open("/dev/mem", O_RDWR)) < 0){
		printf("Cant find /dev/mem!\n");
		return FAIL;
	}
	printf("/dev/mem opened\n");

	void *plb_ll_fifo_ptr;
	plb_ll_fifo_ptr =  mmap(0, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, CSP0);
	if (plb_ll_fifo_ptr == MAP_FAILED){
		printf("\nCan't map memmory area!!\n");
		return FAIL;
	}
	CSP0BASE = (u_int32_t) plb_ll_fifo_ptr;
	//plb_ll_fifo_ctrl_reg = 0;
*/

int mapCSP0(void) {
	int fd;
	printf("Mapping memory\n");

#ifdef VIRTUAL
	CSP0BASE = (u_int32_t)malloc(MEM_SIZE);
	printf("memory allocated\n");
#else

	if ((fd=open("/dev/mem", O_RDWR | O_SYNC)) < 0){
		printf("Cant find /dev/mem!\n");
		return FAIL;
	}
	printf("/dev/mem opened\n");

	CSP0BASE = (u_int32_t)mmap(0, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, CSP0);
	if (CSP0BASE == (u_int32_t)MAP_FAILED) {
		printf("\nCan't map memmory area!!\n");
		return FAIL;
	}
#endif
	printf("CSPOBASE is 0x%x \n",CSP0BASE);
	printf("CSPOBASE=from %08x to %x\n",CSP0BASE,CSP0BASE+MEM_SIZE);

	fifo_control_reg = 0;

	return OK;
}





u_int32_t bus_w(u_int32_t offset, u_int32_t data) {
	__asm__ volatile ("stw %0,0(%1); eieio"::"r" (data), "b"(CSP0BASE+4*offset));
	return OK;
}



u_int32_t bus_r(u_int32_t offset) {
	u_int32_t ptr1;
    __asm__ volatile ("eieio; lwz %0,0(%1)":"=r" (ptr1):"b"(CSP0BASE+4*offset));
    return ptr1;
}









int fifoReset(){

	u_int32_t mask = FIFOCNTRL_RESET_MASK;

	fifo_control_reg	|= mask;
	//printf("CTRL Register bits: 0x%08x\n",fifo_control_reg);

	bus_w(FIFO_CNTRL_REG,fifo_control_reg);
	bus_w(FIFO_CNTRL_REG,fifo_control_reg);
	bus_w(FIFO_CNTRL_REG,fifo_control_reg);
	bus_w(FIFO_CNTRL_REG,fifo_control_reg);

	fifo_control_reg &= (~mask);
	bus_w(FIFO_CNTRL_REG,fifo_control_reg);

	printf("fifo has been reset\n\n");

	return OK;
}




int fifoTest(void){

	int buffer_length = 256;
	int rec_buffer_length = 4096;
	char cmd[] = "help";
	unsigned int buffer[buffer_length];
	unsigned int rec_buffer[rec_buffer_length];
	unsigned int send_len;
	int rec_len;
	char *char_ptr;
	char_ptr = (char *)buffer;

	//fill the buffer with numbers 	for(i=0; i < BUFF_LEN; i++)	{char_ptr[i]=i+1;}

	//sending command
	strcpy(char_ptr,cmd);
	send_len = strlen(cmd);
	fifoSend(char_ptr,send_len);

	//  printf("status : 0x%08x \n",PLB_LL_fifo_get_status_vector());
	usleep(10000);

	do{
		rec_len = fifoReceive(rec_buffer,rec_buffer_length);
		if (rec_len > 0){
			//printf("receive buffer 0x%08x length: %i\n",rec_buffer,rec_len);
			char_ptr = (char*) &rec_buffer[0];
			char_ptr[rec_len]=0;
			printf(char_ptr);
		}
	} while(rec_len > 0);

	return OK;
}


// note: buffer must be word (4 byte) aligned, frameLength in byte
int fifoSend(void *buffer, unsigned int frameLength){

	int vacancy=0;
	int i;
	int words_send = 0;
	int last_word;
	unsigned int *word_ptr;
	unsigned int val,mask;
	u_int32_t status;

	if (frameLength < 1)
		return -1;

	/**4?*/
	last_word = (frameLength-1)/4;
	word_ptr = (unsigned int *)buffer;

	/*what does this do*/
	while (words_send <= last_word){

		//wait for Fifo to be empty again
		while (!vacancy){
			status = bus_r(FIFO_STATUS_REG);
			if(!(status & FIFOSTATUS_ALMOST_FULL_BIT))
				vacancy = 1;
		}

		/**fifo threshold words?*/
		for (i=0; ((i<FIFO_THRESHOLD_WORDS) && (words_send <= last_word)); i++){
			val = 0;

			//announce the start of file
			if (words_send == 0)
				val = FIFOCNTRL_SOF_BIT;

			if (words_send == last_word) /**rem_offset??*/
				val |= (FIFOCNTRL_EOF_BIT | (( (frameLength-1)<<FIFOCNTRL_REM_OFFSET) & FIFOCNTRL_REM_MASK)  );


			//control reg write mask
			mask = FIFOCNTRL_MASK;
			fifo_control_reg &= (~mask);
			fifo_control_reg |= ( mask & val);
			bus_w(FIFO_CNTRL_REG,fifo_control_reg);

			bus_w(FIFO_FIFO_REG,word_ptr[words_send++]);
		}
	}

	return frameLength;
}




int fifoReceive(void *buffer, unsigned int bufflen){

	static unsigned int buffer_ptr = 0;
	int len;
	unsigned int *word_ptr;
	unsigned int status;
	volatile unsigned int fifo_val;
	int sof = 0;

	word_ptr = (unsigned int *)buffer;

	//repeat while fifo status not empty
	do{
		status = bus_r(FIFO_STATUS_REG);
		if (!(status & FIFOSTATUS_EMPTY_BIT)){

			if (status & FIFOSTATUS_SOF_BIT){
				//if SOF, buffer_ptr should be zero, else buffer overflow
				if (buffer_ptr){
					buffer_ptr = 0;
					return -1;
				}
				//		printf(">>>>  SOF\n\r");
				buffer_ptr = 0;/**not needed */
				sof = 1;
			}


			//read from fifo
			fifo_val = bus_r(FIFO_FIFO_REG);


			if ((buffer_ptr > 0) || sof){
				if ( (bufflen >> 2) > buffer_ptr)
					word_ptr[buffer_ptr++] = fifo_val; //write to buffer
				else{
					buffer_ptr = 0;
					return -2; // buffer overflow
				}

				if (status & FIFOSTATUS_EOF_BIT){
					len = (buffer_ptr << 2) -3 + ( (status & FIFOSTATUS_REM_MASK)>>FIFOSTATUS_REM_OFFSET );
					//  printf(">>>>status=0x%08x  EOF  len = %d \n\r\n\r",status, len);
					buffer_ptr = 0;
					return len;
				}

			}
		}
	}
	while(!(status & FIFOSTATUS_EMPTY_BIT));


	return OK;
}






int64_t set64BitReg(int64_t value, int aLSB, int aMSB){
	int64_t v64;
	u_int32_t vLSB,vMSB;
	if (value!=-1) {
		vLSB=value&(0xffffffff);
		bus_w(aLSB,vLSB);
		v64=value>> 32;
		vMSB=v64&(0xffffffff);
		bus_w(aMSB,vMSB);
	}
	return get64BitReg(aLSB, aMSB);

}

int64_t get64BitReg(int aLSB, int aMSB){
	int64_t v64;
	u_int32_t vLSB,vMSB;
	vLSB=bus_r(aLSB);
	vMSB=bus_r(aMSB);
	v64=vMSB;
	v64=(v64<<32) | vLSB;
	return v64;
}


int64_t setFrames(int64_t value){//dummy = value;return dummy;
	return set64BitReg(value,  SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
}
int64_t getFrames(){//return dummy;
	return get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG);
}


int64_t setExposureTime(int64_t value){
	/* time is in ns */
	if (value!=-1)
		value*=(1E-9*CLK_FREQ);
	return set64BitReg(value,SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG)/(1E-9*CLK_FREQ);
}
int64_t getExposureTime(){
	return get64BitReg(GET_EXPTIME_LSB_REG, GET_EXPTIME_MSB_REG)/(1E-9*CLK_FREQ);
}


int64_t setGates(int64_t value){
	return set64BitReg(value, SET_GATES_LSB_REG, SET_GATES_MSB_REG);
}
int64_t getGates(){
	return get64BitReg(GET_GATES_LSB_REG, GET_GATES_MSB_REG);
}


int64_t setPeriod(int64_t value){
	/* time is in ns */
	if (value!=-1)
		value*=(1E-9*CLK_FREQ);
	return set64BitReg(value,SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG)/(1E-9*CLK_FREQ);
}
int64_t getPeriod(){
	return get64BitReg(GET_PERIOD_LSB_REG, GET_PERIOD_MSB_REG)/(1E-9*CLK_FREQ);
}


int64_t setDelay(int64_t value){
	/* time is in ns */
	if (value!=-1) {
		value*=(1E-9*CLK_FREQ);
	}
	return set64BitReg(value,SET_DELAY_LSB_REG, SET_DELAY_MSB_REG)/(1E-9*CLK_FREQ);
}
int64_t getDelay(){
	return get64BitReg(GET_DELAY_LSB_REG, GET_DELAY_MSB_REG)/(1E-9*CLK_FREQ);
}


int64_t setTrains(int64_t value){
	return set64BitReg(value,  SET_TRAINS_LSB_REG, SET_TRAINS_MSB_REG);
}
int64_t getTrains(){
	return get64BitReg(GET_TRAINS_LSB_REG, GET_TRAINS_MSB_REG);
}



int64_t setProbes(int64_t value){
	return 0;
}
int64_t getProbes(){
	return 0;
}


