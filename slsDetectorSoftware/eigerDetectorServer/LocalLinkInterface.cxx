
//Class initially from Gerd and was called mmap_test.c 
//return reversed 1 means good, 0 means failed


#include <stdio.h>
#include <unistd.h>
//#include <string.h>


#include "HardwareMMappingDefs.h"

#include "LocalLinkInterface.h"



LocalLinkInterface::LocalLinkInterface(unsigned int ll_fifo_badr){
  //    printf("\n    v 1     \n");
    printf("Initialize PLB LL FIFOs\n");
      ll_fifo_base=0;
      ll_fifo_ctrl_reg=0;
      if(Init(ll_fifo_badr)){
	Reset();
	printf("\tFIFO Status : 0x%08x\n",StatusVector());
      }else printf("\tError LocalLink Mappping : 0x%08x\n",ll_fifo_badr);

      printf("\n\n");
}

LocalLinkInterface::~LocalLinkInterface(){};

LocalLinkInterface::LocalLinkInterface(){
         printf("Initialize new memory\n");
 }

int LocalLinkInterface::InitNewMemory (unsigned int addr, int ifg){
	unsigned int CSP0BASE;
	int fd;

	/*fd = open("/dev/mem", O_RDWR | O_SYNC, 0);
	if (fd == -1) {
		printf("\nCan't find /dev/mem!\n");
		return 0;
	}
	printf("/dev/mem opened\n");


	CSP0BASE = (u_int32_t)mmap(0, 0x1000, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, addr);
	if (CSP0BASE == (u_int32_t)MAP_FAILED) {
		printf("\nCan't map memmory area!!\n");
		return 0;
	}
	printf("CSP0 mapped\n");


	volatile u_int8_t *ptr1;

	ptr1=(u_int8_t*)(CSP0BASE);

	printf("pointer val=%x\n",(void*)ptr1);

	printf("ifg_control=%02x\n",*ptr1);

	*ptr1=ifg;

	printf("ifg_control new=%02x\n",*ptr1);

	close(fd);
*/
	return 1;
}



bool LocalLinkInterface::Init(unsigned int ll_fifo_badr){
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

    ll_fifo_base = (xfs_u32) plb_ll_fifo_ptr;
    ll_fifo_ctrl_reg = 0;

    return 1;
}



bool LocalLinkInterface::Reset(){
  return Reset(PLB_LL_FIFO_CTRL_RESET_STD);
}

bool LocalLinkInterface::Reset(unsigned int rst_mask){
    ll_fifo_ctrl_reg |= rst_mask;
    printf("\tCTRL Register bits: 0x%08x\n",ll_fifo_ctrl_reg);  

    xfs_out32(ll_fifo_base+4*PLB_LL_FIFO_REG_CTRL,ll_fifo_ctrl_reg);
    xfs_out32(ll_fifo_base+4*PLB_LL_FIFO_REG_CTRL,ll_fifo_ctrl_reg);
    xfs_out32(ll_fifo_base+4*PLB_LL_FIFO_REG_CTRL,ll_fifo_ctrl_reg);
    xfs_out32(ll_fifo_base+4*PLB_LL_FIFO_REG_CTRL,ll_fifo_ctrl_reg);

    ll_fifo_ctrl_reg &= (~rst_mask);
   
    xfs_out32(ll_fifo_base+4*PLB_LL_FIFO_REG_CTRL,ll_fifo_ctrl_reg);
    // printf("FIFO CTRL Address: 0x%08x\n FIFO CTRL Register: 0x%08x\n",PLB_LL_FIFO_REG_CTRL,plb_ll_fifo[PLB_LL_FIFO_REG_CTRL]);   
    return 1;
}



unsigned int LocalLinkInterface::StatusVector(){
  return xfs_in32(ll_fifo_base+4*PLB_LL_FIFO_REG_STATUS);
}

int LocalLinkInterface::Write(unsigned int buffer_len, void *buffer){
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
  
    while (words_send <= last_word)
    {
      while (!vacancy)//wait for Fifo to be empty again
      {
        status = xfs_in32(ll_fifo_base+4*PLB_LL_FIFO_REG_STATUS);
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
	    ctrl_reg_write_mask(PLB_LL_FIFO_CTRL_LL_MASK,fifo_ctrl);
	    xfs_out32(ll_fifo_base+4*PLB_LL_FIFO_REG_FIFO,word_ptr[words_send++]);
	}
    }
    return buffer_len;
}


int LocalLinkInterface::Read(unsigned int buffer_len, void *buffer){
    static unsigned int buffer_ptr = 0;
    // note: buffer must be word (4 byte) aligned
    // frame_len in byte
    int len;
    unsigned int *word_ptr;
    unsigned int status;  
    volatile unsigned int fifo_val;
    int sof = 0;
  
    word_ptr = (unsigned int *)buffer;  
    do 
    {
      status = xfs_in32(ll_fifo_base+4*PLB_LL_FIFO_REG_STATUS);

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

	    fifo_val = xfs_in32(ll_fifo_base+4*PLB_LL_FIFO_REG_FIFO);  //read from fifo
		
	    if ((buffer_ptr > 0) || sof)
	    {
		if ( (buffer_len >> 2) > buffer_ptr)
		{
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

bool LocalLinkInterface::ctrl_reg_write_mask(unsigned int mask, unsigned int val){
    // printf("Fifo CTRL Reg(1): 0x%08x\n",plb_ll_fifo_ctrl_reg);
    ll_fifo_ctrl_reg &= (~mask);
    //printf("Fifo CTRL Reg(2): 0x%08x\n",plb_ll_fifo_ctrl_reg);
    ll_fifo_ctrl_reg |= ( mask & val);   
//    printf("Fifo CTRL Reg: 0x%08x\n",plb_ll_fifo_ctrl_reg);
    xfs_out32(ll_fifo_base+4*PLB_LL_FIFO_REG_CTRL,ll_fifo_ctrl_reg);
//    printf("Fifo STAT Reg: 0x%08x\n", plb_ll_fifo[PLB_LL_FIFO_REG_STATUS]);
    return 1;
}


int LocalLinkInterface::Test(unsigned int buffer_len, void *buffer){

  int len;
  unsigned int rec_buff_len = 4096;
  unsigned int rec_buffer[4097];	


  Write(buffer_len,buffer);
  usleep(10000);

  do{
    len = Read(rec_buff_len,rec_buffer);
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

void LocalLinkInterface::llfifo_print_frame(unsigned char* fbuff, int len){
    printf("\n\r----Frame of len : %d Byte\n\r",len);
    for(int i=0;i<len;i++){
	printf("0x%02x ",fbuff[i] );
	if ((i&0xf) == 0x7) printf(" ");
	if ((i&0xf) == 0xf) printf("\n\r");
    }
    printf("\n\r");
}


