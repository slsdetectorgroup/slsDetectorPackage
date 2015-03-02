
//Class initially from Gerd and was called mmap_test.c 
//return reversed 1 means good, 0 means failed


#include <stdio.h>
#include <unistd.h>
//#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>


#include "HardwareMMappingDefs.h"

#include "LocalLinkInterface.h"



void Local_LocalLinkInterface1(struct LocalLinkInterface* ll,unsigned int ll_fifo_badr){
	//    printf("\n    v 1     \n");
	printf("Initialize PLB LL FIFOs\n");
	ll->ll_fifo_base=0;
	ll->ll_fifo_ctrl_reg=0;
	if(Local_Init(ll,ll_fifo_badr)){
		Local_Reset(ll);
		printf("\tFIFO Status : 0x%08x\n",Local_StatusVector(ll));
	}else printf("\tError LocalLink Mappping : 0x%08x\n",ll_fifo_badr);
	printf("\n\n");
}

/*~LocalLinkInterface(){};*/

void Local_LocalLinkInterface(struct LocalLinkInterface* ll){
	printf("Initializing new memory\n");
}

int Local_GetModuleConfiguration (struct LocalLinkInterface* ll, u_int32_t baseaddr, u_int32_t offset){
	int fd = open("/dev/mem", O_RDWR | O_SYNC, 0);
	if (fd == -1) {
		printf("\nCan't find /dev/mem!\n");
		return 0;
	}
	printf("/dev/mem opened\n");

	u_int32_t CSP0BASE = (u_int32_t)mmap(0, 0x100000, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, baseaddr);
	if (CSP0BASE == (u_int32_t)MAP_FAILED) {
		printf("\nCan't map memmory area!!\n");
		return 0;
	}
	printf("CSP0 mapped\n");

	volatile u_int32_t *ptr1;
	ptr1=(u_int32_t*)(CSP0BASE + offset);
	//printf("LocalLinkInterface:: value:%d\n",*ptr1);
	close(fd);

	return *ptr1;
}



int Local_Init(struct LocalLinkInterface* ll,unsigned int ll_fifo_badr){
	int fd;
	void *plb_ll_fifo_ptr;

	if ((fd=open("/dev/mem", O_RDWR)) < 0){
		fprintf(stderr, "Could not open /dev/mem\n");
		return 0;
	}

	plb_ll_fifo_ptr =  mmap(0, getpagesize(), PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, ll_fifo_badr);
	close(fd);

	if (plb_ll_fifo_ptr == MAP_FAILED){
		perror ("mmap");
		return 0;
	}

	ll->ll_fifo_base = (xfs_u32) plb_ll_fifo_ptr;
	ll->ll_fifo_ctrl_reg = 0;

	return 1;
}



int Local_Reset(struct LocalLinkInterface* ll){
	return Local_Reset1(ll,PLB_LL_FIFO_CTRL_RESET_STD);
}

int Local_Reset1(struct LocalLinkInterface* ll,unsigned int rst_mask){
	ll->ll_fifo_ctrl_reg |= rst_mask;
	printf("\tCTRL Register bits: 0x%08x\n",ll->ll_fifo_ctrl_reg);

	HWIO_xfs_out32(ll->ll_fifo_base+4*PLB_LL_FIFO_REG_CTRL,ll->ll_fifo_ctrl_reg);
	HWIO_xfs_out32(ll->ll_fifo_base+4*PLB_LL_FIFO_REG_CTRL,ll->ll_fifo_ctrl_reg);
	HWIO_xfs_out32(ll->ll_fifo_base+4*PLB_LL_FIFO_REG_CTRL,ll->ll_fifo_ctrl_reg);
	HWIO_xfs_out32(ll->ll_fifo_base+4*PLB_LL_FIFO_REG_CTRL,ll->ll_fifo_ctrl_reg);

	ll->ll_fifo_ctrl_reg &= (~rst_mask);

	HWIO_xfs_out32(ll->ll_fifo_base+4*PLB_LL_FIFO_REG_CTRL,ll->ll_fifo_ctrl_reg);
	// printf("FIFO CTRL Address: 0x%08x\n FIFO CTRL Register: 0x%08x\n",PLB_LL_FIFO_REG_CTRL,plb_ll_fifo[PLB_LL_FIFO_REG_CTRL]);
	return 1;
}



unsigned int Local_StatusVector(struct LocalLinkInterface* ll){
	return HWIO_xfs_in32(ll->ll_fifo_base+4*PLB_LL_FIFO_REG_STATUS);
}

int Local_Write(struct LocalLinkInterface* ll,unsigned int buffer_len, void *buffer){
	// note: buffer must be word (4 byte) aligned
	// frame_len in byte
	int vacancy=0;
	int i;
	int words_send = 0;
	int last_word;
	unsigned int *word_ptr;
	unsigned int fifo_ctrl;
	xfs_u32 status;

	if (buffer_len < 1) return -1;

	last_word = (buffer_len-1)/4;
	word_ptr = (unsigned int *)buffer;

#ifdef MARTIN
	printf("LL Write - Len: %2d - If: %X - Data: ",buffer_len, ll->ll_fifo_base);
	for (i=0; i < buffer_len/4; i++)
		printf("%.8X ",*(((unsigned *) buffer)+i));
	printf("\n");
#endif

	while (words_send <= last_word)
	{
		while (!vacancy)//wait for Fifo to be empty again
		{
			status = HWIO_xfs_in32(ll->ll_fifo_base+4*PLB_LL_FIFO_REG_STATUS);
			if((status & PLB_LL_FIFO_STATUS_ALMOSTFULL) == 0) vacancy = 1;
		}

		//Just to know: #define PLB_LL_FIFO_ALMOST_FULL_THRESHOLD_WORDS    100
		for (i=0; ((i<PLB_LL_FIFO_ALMOST_FULL_THRESHOLD_WORDS) && (words_send <= last_word)); i++)
		{
			fifo_ctrl = 0;
			if (words_send == 0)
			{
				fifo_ctrl = PLB_LL_FIFO_CTRL_LL_SOF;//announce the start of file
			}

			if (words_send == last_word)
			{
				fifo_ctrl |= (PLB_LL_FIFO_CTRL_LL_EOF | (( (buffer_len-1)<<PLB_LL_FIFO_CTRL_LL_REM_SHIFT) & PLB_LL_FIFO_CTRL_LL_REM)  );
			}
			Local_ctrl_reg_write_mask(ll,PLB_LL_FIFO_CTRL_LL_MASK,fifo_ctrl);
			HWIO_xfs_out32(ll->ll_fifo_base+4*PLB_LL_FIFO_REG_FIFO,word_ptr[words_send++]);
		}
	}
	return buffer_len;
}


int Local_Read(struct LocalLinkInterface* ll,unsigned int buffer_len, void *buffer){
	static unsigned int buffer_ptr = 0;
	// note: buffer must be word (4 byte) aligned
	// frame_len in byte
	int len;
	unsigned int *word_ptr;
	unsigned int status;
	volatile unsigned int fifo_val;
	int sof = 0;

#ifdef MARTIN
	printf("LL Read - If: %X - Data: ",ll->ll_fifo_base);
#endif

	word_ptr = (unsigned int *)buffer;
	do
	{
		status = HWIO_xfs_in32(ll->ll_fifo_base+4*PLB_LL_FIFO_REG_STATUS);

		if (!(status & PLB_LL_FIFO_STATUS_EMPTY))
		{
			if (status & PLB_LL_FIFO_STATUS_LL_SOF)
			{
				if (buffer_ptr)
				{
					buffer_ptr = 0;
					return -1; // buffer overflow
				}
				//		printf(">>>>  SOF\n\r");
				buffer_ptr = 0;
				sof = 1;
			}

			fifo_val = HWIO_xfs_in32(ll->ll_fifo_base+4*PLB_LL_FIFO_REG_FIFO);  //read from fifo

			if ((buffer_ptr > 0) || sof)
			{
				if ( (buffer_len >> 2) > buffer_ptr)
				{
#ifdef MARTIN
					printf("%.8X ", fifo_val);
#endif
					word_ptr[buffer_ptr++] = fifo_val; //write to buffer
				}
				else
				{
					buffer_ptr = 0;
					return -2; // buffer overflow
				}

				if (status & PLB_LL_FIFO_STATUS_LL_EOF)
				{
					len = (buffer_ptr << 2) -3 + ( (status & PLB_LL_FIFO_STATUS_LL_REM)>>PLB_LL_FIFO_STATUS_LL_REM_SHIFT );
#ifdef MARTIN
					printf("Len: %d\n",len);
#endif
					//		    printf(">>>>status=0x%08x  EOF  len = %d \n\r\n\r",status, len);
					buffer_ptr = 0;
					return len;
				}

			}
		}
	}
	while(!(status & PLB_LL_FIFO_STATUS_EMPTY));

	return 0;
}

int Local_ctrl_reg_write_mask(struct LocalLinkInterface* ll,unsigned int mask, unsigned int val){
	// printf("Fifo CTRL Reg(1): 0x%08x\n",plb_ll_fifo_ctrl_reg);
	ll->ll_fifo_ctrl_reg &= (~mask);
	//printf("Fifo CTRL Reg(2): 0x%08x\n",plb_ll_fifo_ctrl_reg);
	ll->ll_fifo_ctrl_reg |= ( mask & val);
	//    printf("Fifo CTRL Reg: 0x%08x\n",plb_ll_fifo_ctrl_reg);
	HWIO_xfs_out32(ll->ll_fifo_base+4*PLB_LL_FIFO_REG_CTRL,ll->ll_fifo_ctrl_reg);
	//    printf("Fifo STAT Reg: 0x%08x\n", plb_ll_fifo[PLB_LL_FIFO_REG_STATUS]);
	return 1;
}


int Local_Test(struct LocalLinkInterface* ll,unsigned int buffer_len, void *buffer){

	int len;
	unsigned int rec_buff_len = 4096;
	unsigned int rec_buffer[4097];


	Local_Write(ll,buffer_len,buffer);
	usleep(10000);

	do{
		len = Local_Read(ll,rec_buff_len,rec_buffer);
		printf("receive length: %i\n",len);

		if (len > 0){
			rec_buffer[len]=0;
			printf((char*) rec_buffer);
			printf("\n");
		}
	} while(len > 0);

	printf("\n\n\n\n");
	return 1;
}

void Local_llfifo_print_frame(struct LocalLinkInterface* ll,unsigned char* fbuff, int len){
	int i;
	printf("\n\r----Frame of len : %d Byte\n\r",len);
	for(i=0;i<len;i++){
		printf("0x%02x ",fbuff[i] );
		if ((i&0xf) == 0x7) printf(" ");
		if ((i&0xf) == 0xf) printf("\n\r");
	}
	printf("\n\r");
}


