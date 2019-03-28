#include "logger.h"
#include "Beb.h"
#include "FebRegisterDefs.h"
#include "xparameters.h"

#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))


struct BebInfo beb_infos[10];
int bebInfoSize = 0;

struct LocalLinkInterface ll_beb_local,* ll_beb;

struct udp_header_type udp_header;

int           Beb_send_ndata;
unsigned int  Beb_send_buffer_size;
unsigned int* Beb_send_data_raw;
unsigned int* Beb_send_data;

int           Beb_recv_ndata;
unsigned int  Beb_recv_buffer_size;
unsigned int* Beb_recv_data_raw;
unsigned int* Beb_recv_data;

short Beb_bit_mode;
int BEB_MMAP_SIZE = 0x1000;

int Beb_activated = 1;

uint32_t Beb_detid = 0;
int Beb_top =0;



void BebInfo_BebInfo(struct BebInfo* bebInfo, unsigned int beb_num) {
	bebInfo->beb_number=beb_num;
	bebInfo->serial_address=0;
	strcpy(bebInfo->src_mac_1GbE,"");
	strcpy(bebInfo->src_mac_10GbE,"");
	strcpy(bebInfo->src_ip_1GbE,"");
	strcpy(bebInfo->src_ip_10GbE,"");
	bebInfo->src_port_1GbE=bebInfo->src_port_10GbE=0;
}


int BebInfo_SetSerialAddress(struct BebInfo* bebInfo, unsigned int a) {
	//address pre shifted
	if (a>0xff) return 0;
	bebInfo->serial_address = 0x04000000 | ((a&0xff)<<16);
	return 1;
}


int BebInfo_SetHeaderInfo(struct BebInfo* bebInfo, int ten_gig, char* src_mac, char* src_ip, unsigned int src_port) {
	if (ten_gig) { strcpy(bebInfo->src_mac_10GbE,src_mac); strcpy(bebInfo->src_ip_10GbE,src_ip); bebInfo->src_port_10GbE = src_port;}
	else       { strcpy(bebInfo->src_mac_1GbE,src_mac); strcpy(bebInfo->src_ip_1GbE,src_ip); bebInfo->src_port_1GbE  = src_port;}
	return 1;
}



unsigned int BebInfo_GetBebNumber(struct BebInfo* bebInfo)           {return bebInfo->beb_number;}
unsigned int BebInfo_GetSerialAddress(struct BebInfo* bebInfo)       {return bebInfo->serial_address;}
char*  BebInfo_GetSrcMAC(struct BebInfo* bebInfo, int ten_gig)  {return ten_gig ? bebInfo->src_mac_10GbE  : bebInfo->src_mac_1GbE;}
char*  BebInfo_GetSrcIP(struct BebInfo* bebInfo, int ten_gig)   {return ten_gig ? bebInfo->src_ip_10GbE   : bebInfo->src_ip_1GbE;}
unsigned int BebInfo_GetSrcPort(struct BebInfo* bebInfo, int ten_gig) {return ten_gig ? bebInfo->src_port_10GbE : bebInfo->src_port_1GbE;}


void BebInfo_Print(struct BebInfo* bebInfo) {
	FILE_LOG(logINFO, (
			"%d) Beb Info:\n"
			"\tSerial Add: 0x%x\n"
			"\tMAC   1GbE: %s\n"
			"\tIP    1GbE: %s\n"
			"\tPort  1GbE: %d\n"
			"\tMAC  10GbE: %s\n"
			"\tIP   10GbE: %s\n"
			"\tPort 10GbE: %d\n",
			bebInfo->beb_number,
			bebInfo->serial_address,
			bebInfo->src_mac_1GbE,
			bebInfo->src_ip_1GbE,
			bebInfo->src_port_1GbE,
			bebInfo->src_mac_10GbE,
			bebInfo->src_ip_10GbE,
			bebInfo->src_port_10GbE));
}


void Beb_Beb(int id) {
	Beb_detid = id;
	Beb_send_ndata = 0;
	Beb_send_buffer_size = 1026;
	Beb_send_data_raw = malloc((Beb_send_buffer_size+1) * sizeof(unsigned int));
	Beb_send_data     = &Beb_send_data_raw[1];

	Beb_recv_ndata = 0;
	Beb_recv_buffer_size = 1026;
	Beb_recv_data_raw = malloc((Beb_recv_buffer_size+1) * sizeof(unsigned int));
	Beb_recv_data     = &Beb_recv_data_raw[1];

	udp_header= (struct udp_header_type) {
		{0x00, 0x50, 0xc5, 0xb2, 0xcb, 0x46},  // DST MAC
		{0x00, 0x50, 0xc2, 0x46, 0xd9, 0x02},  // SRC MAC
		{0x08, 0x00},
		{0x45},
		{0x00},
		{0x00, 0x00},
		{0x00, 0x00},
		{0x40},
		{0x00},
		{0xff},
		{0x11},
		{0x00, 0x00},
		{129, 205, 205, 128},  // Src IP
		{129, 205, 205, 122},  // Dst IP
		{0x0f, 0xa1},
		{0x13, 0x89},
		{0x00, 0x00}, //{0x00, 0x11},
		{0x00, 0x00}
	};


	if (!Beb_InitBebInfos()) exit(1);

	FILE_LOG(logDEBUG1, ("Printing Beb infos:\n"));
	unsigned int i;
	for(i=1;i<bebInfoSize;i++) BebInfo_Print(&beb_infos[i]);


	Beb_bit_mode = 4;

	//  ll_beb = &ll_beb_local;
	//  Local_LocalLinkInterface1(ll_beb,XPAR_PLB_LL_FIFO_AURORA_DUAL_CTRL_FEB_LEFT_BASEADDR);

	//  Beb_SetByteOrder();
}



void Beb_GetModuleConfiguration(int* master, int* top, int* normal) {
	*top = 0;
	*master = 0;
	//mapping new memory to read master top module configuration
	u_int32_t* csp0base=0;
	int ret;
	//open file pointer
	int fd = Beb_open(&csp0base,XPAR_PLB_GPIO_SYS_BASEADDR);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Module Configuration FAIL\n"));
	} else {
		//read data
		ret = Beb_Read32(csp0base, MODULE_CONFIGURATION_MASK);
		FILE_LOG(logDEBUG1, ("Module Configuration OK\n"));
		FILE_LOG(logDEBUG1, ("Beb: value =0x%x\n",ret));
		if (ret&TOP_BIT_MASK) {
			*top = 1;
			Beb_top = 1;
		}
		if (ret&MASTER_BIT_MASK)
			*master = 1;
		if (ret&NORMAL_MODULE_BIT_MASK)
			*normal = 1;
		//close file pointer
		Beb_close(fd,csp0base);
	}
}



void Beb_EndofDataSend(int tengiga) {
	//mapping new memory
	u_int32_t* csp0base=0;
	int l_framepktLsbcounter, l_framepktMsbcounter, l_txndelaycounter, l_framedelaycounter;
	int r_framepktLsbcounter, r_framepktMsbcounter, r_txndelaycounter, r_framedelaycounter;
	int l_framepktLsbcounter_new, l_framepktMsbcounter_new, l_txndelaycounter_new, l_framedelaycounter_new;
	int r_framepktLsbcounter_new, r_framepktMsbcounter_new, r_txndelaycounter_new, r_framedelaycounter_new;
	int addr_l_framepktLsbcounter,  addr_l_framepktMsbcounter, addr_l_txndelaycounter,  addr_l_framedelaycounter;
	int addr_r_framepktLsbcounter,  addr_r_framepktMsbcounter, addr_r_txndelaycounter,  addr_r_framedelaycounter;

	switch(tengiga) {
	case 0:
		addr_l_framepktLsbcounter = ONE_GIGA_LEFT_INDEX_LSB_COUNTER;
		addr_l_framepktMsbcounter = ONE_GIGA_LEFT_INDEX_MSB_COUNTER;
		addr_l_txndelaycounter = ONE_GIGA_LEFT_TXN_DELAY_COUNTER;
		addr_l_framedelaycounter = ONE_GIGA_LEFT_FRAME_DELAY_COUNTER;
		addr_r_framepktLsbcounter = ONE_GIGA_RIGHT_INDEX_LSB_COUNTER;
		addr_r_framepktMsbcounter = ONE_GIGA_RIGHT_INDEX_MSB_COUNTER;
		addr_r_txndelaycounter = ONE_GIGA_RIGHT_TXN_DELAY_COUNTER;
		addr_r_framedelaycounter = ONE_GIGA_RIGHT_FRAME_DELAY_COUNTER;
		break;
	case 1:
		addr_l_framepktLsbcounter = TEN_GIGA_LEFT_INDEX_LSB_COUNTER;
		addr_l_framepktMsbcounter = TEN_GIGA_LEFT_INDEX_MSB_COUNTER;
		addr_l_txndelaycounter = TEN_GIGA_LEFT_TXN_DELAY_COUNTER;
		addr_l_framedelaycounter = TEN_GIGA_LEFT_FRAME_DELAY_COUNTER;
		addr_r_framepktLsbcounter = TEN_GIGA_RIGHT_INDEX_LSB_COUNTER;
		addr_r_framepktMsbcounter = TEN_GIGA_RIGHT_INDEX_MSB_COUNTER;
		addr_r_txndelaycounter = TEN_GIGA_RIGHT_TXN_DELAY_COUNTER;
		addr_r_framedelaycounter = TEN_GIGA_RIGHT_FRAME_DELAY_COUNTER;
		break;
	}


	//open file pointer
	int fd = Beb_open(&csp0base,XPAR_COUNTER_BASEADDR);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Delay read counter fail\n"));
		return;
	} else {
		//read data first time
		l_framepktLsbcounter = Beb_Read32(csp0base, addr_l_framepktLsbcounter);
		l_framepktMsbcounter = Beb_Read32(csp0base, addr_l_framepktMsbcounter);
		l_txndelaycounter = Beb_Read32(csp0base, addr_l_txndelaycounter);
		l_framedelaycounter = Beb_Read32(csp0base, addr_l_framedelaycounter);
		r_framepktLsbcounter = Beb_Read32(csp0base, addr_r_framepktLsbcounter);
		r_framepktMsbcounter = Beb_Read32(csp0base, addr_r_framepktMsbcounter);
		r_txndelaycounter = Beb_Read32(csp0base, addr_r_txndelaycounter);
		r_framedelaycounter = Beb_Read32(csp0base, addr_r_framedelaycounter);
		FILE_LOG(logDEBUG1, ("\nLeft\n"
				"FramepacketLsbcounter: %d\n"
				"FramepacketMsbcounter: %d\n"
				"Txndelaycounter:%d\n"
				"Framedelaycounter:%d\n"
				"\nRight\n"
				"FramepacketLsbcounter: %d\n"
				"FramepacketMsbcounter: %d\n"
				"Txndelaycounter:%d\n"
				"Framedelaycounter:%d\n\n",
				l_framepktLsbcounter,l_framepktMsbcounter,l_txndelaycounter,l_framedelaycounter,
				r_framepktLsbcounter,r_framepktMsbcounter,r_txndelaycounter,r_framedelaycounter));

		//keep comparing with previous values
		int maxtimer;
		while(1) {
			maxtimer = MAX(MAX(l_txndelaycounter,l_framedelaycounter),MAX(r_txndelaycounter,r_framedelaycounter));
			maxtimer /= 100;
			FILE_LOG(logDEBUG1, ("Will wait for %d us\n",maxtimer));
			usleep(maxtimer);

			//read new values
			l_framepktLsbcounter_new = Beb_Read32(csp0base, addr_l_framepktLsbcounter);
			l_framepktMsbcounter_new = Beb_Read32(csp0base, addr_l_framepktMsbcounter);
			l_txndelaycounter_new = Beb_Read32(csp0base, addr_l_txndelaycounter);
			l_framedelaycounter_new = Beb_Read32(csp0base, addr_l_framedelaycounter);
			r_framepktLsbcounter_new = Beb_Read32(csp0base, addr_r_framepktLsbcounter);
			r_framepktMsbcounter_new = Beb_Read32(csp0base, addr_r_framepktMsbcounter);
			r_txndelaycounter_new = Beb_Read32(csp0base, addr_r_txndelaycounter);
			r_framedelaycounter_new = Beb_Read32(csp0base, addr_r_framedelaycounter);
			FILE_LOG(logDEBUG1, ("\nLeft\n"
					"FramepacketLsbcounter: %d\n"
					"FramepacketMsbcounter: %d\n"
					"Txndelaycounter:%d\n"
					"Framedelaycounter:%d\n"
					"\nRight\n"
					"FramepacketLsbcounter: %d\n"
					"FramepacketMsbcounter: %d\n"
					"Txndelaycounter:%d\n"
					"Framedelaycounter:%d\n\n",
					l_framepktLsbcounter_new,l_framepktMsbcounter_new,l_txndelaycounter_new,l_framedelaycounter_new,
					r_framepktLsbcounter_new,r_framepktMsbcounter_new,r_txndelaycounter_new,r_framedelaycounter_new));

			if ((l_framepktLsbcounter == l_framepktLsbcounter_new) &&
					(l_framepktMsbcounter == l_framepktMsbcounter_new) &&
					(r_framepktLsbcounter == r_framepktLsbcounter_new) &&
					(r_framepktMsbcounter == r_framepktMsbcounter_new))
				break;

			//update old values
			l_framepktLsbcounter = l_framepktLsbcounter_new;
			l_framepktMsbcounter = l_framepktMsbcounter_new;
			l_txndelaycounter = l_txndelaycounter_new;
			l_framedelaycounter = l_framedelaycounter_new;
			r_framepktLsbcounter = r_framepktLsbcounter_new;
			r_framepktMsbcounter = r_framepktMsbcounter_new;
			r_txndelaycounter = r_txndelaycounter_new;
			r_framedelaycounter = r_framedelaycounter_new;

		}

		FILE_LOG(logINFO, ("Detector has sent all data\n"));
		//close file pointer
		Beb_close(fd,csp0base);
	}
}




/* do not work at the moment */
int Beb_SetMasterViaSoftware() {

	if (!Beb_activated)
		return 0;

	//mapping new memory
	u_int32_t* csp0base=0;
	u_int32_t value = 0, ret = 1;

	//open file pointer
	int fd = Beb_open(&csp0base,XPAR_PLB_GPIO_SYS_BASEADDR);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Set Master FAIL\n"));
	} else {
		value = Beb_Read32(csp0base, MASTERCONFIG_OFFSET);
		value|=MASTER_BIT;
		value|=OVERWRITE_HARDWARE_BIT;
		int newval = Beb_Write32(csp0base, MASTERCONFIG_OFFSET,value);
		if (newval!=value) {
			FILE_LOG(logERROR, ("Could not set Master via Software\n"));
		} else {
			ret = 0;
		}
	}

	//close file pointer
	if (fd > 0)
		Beb_close(fd,csp0base);

	return ret;
}

/* do not work at the moment */
int Beb_SetSlaveViaSoftware() {

	if (!Beb_activated)
		return 0;

	//mapping new memory
	u_int32_t* csp0base=0;
	u_int32_t value = 0, ret = 1;

	//open file pointer
	int fd = Beb_open(&csp0base,XPAR_PLB_GPIO_SYS_BASEADDR);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Set Slave FAIL\n"));
	} else {
		value = Beb_Read32(csp0base, MASTERCONFIG_OFFSET);
		value&=~MASTER_BIT;
		value|=OVERWRITE_HARDWARE_BIT;
		int newval = Beb_Write32(csp0base, MASTERCONFIG_OFFSET,value);
		if (newval!=value) {
			FILE_LOG(logERROR, ("Could not set Slave via Software\n"));
		} else {
			ret = 0;
		}
	}

	//close file pointer
	if (fd > 0)
		Beb_close(fd,csp0base);

	return ret;
}

int Beb_Activate(int enable) {
	//mapping new memory
	u_int32_t* csp0base=0;
	u_int32_t value = 0, ret = -1;

	//open file pointer
	int fd = Beb_open(&csp0base,XPAR_PLB_GPIO_SYS_BASEADDR);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Deactivate FAIL\n"));
	} else {
		if (enable > -1) {
			value = Beb_Read32(csp0base, MASTERCONFIG_OFFSET);
			FILE_LOG(logINFO, ("Deactivate register value before:%d\n",value));
			if (enable)
				value&=~DEACTIVATE_BIT;
			else
				value|=DEACTIVATE_BIT;

			int newval = Beb_Write32(csp0base, MASTERCONFIG_OFFSET,value);
			if (newval!=value) {
				if (enable) {
					FILE_LOG(logERROR, ("Could not activate via Software\n"));
				} else {
					FILE_LOG(logERROR, ("Could not deactivate via Software\n"));
				}
			}
		}

		value = Beb_Read32(csp0base, MASTERCONFIG_OFFSET);
		if (value&DEACTIVATE_BIT) ret = 0;
		else ret = 1;
		if (enable == -1) {
			if (ret) {
				FILE_LOG(logINFOBLUE, ("Detector is active. Register value:%d\n", value));
			} else {
				FILE_LOG(logERROR, ("Detector is deactivated! Register value:%d\n", value));
			}
		}

	}
	//close file pointer
	if (fd > 0)
		Beb_close(fd,csp0base);

	Beb_activated = ret;

	return ret;
}


int Beb_Set32bitOverflow(int val) {
	if (!Beb_activated)
		return val;

	//mapping new memory
	u_int32_t* csp0base=0;
	u_int32_t valueread = 0;
	u_int32_t offset = FLOW_REG_OFFSET;
	if (val>0) val = 1;

	//open file pointer
	int fd = Beb_open(&csp0base,XPAR_PLB_GPIO_SYS_BASEADDR);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Could not read register to set overflow flag in 32 bit mode. FAIL\n"));
		return -1;
	}
	else {
		if (val > -1) {
			// reset bit
			valueread = Beb_Read32(csp0base, offset);
			Beb_Write32(csp0base, offset,valueread & ~FLOW_REG_OVERFLOW_32_BIT_MSK);

			// set bit
			valueread = Beb_Read32(csp0base, offset);
			Beb_Write32(csp0base, offset,valueread |
					((val << FLOW_REG_OVERFLOW_32_BIT_OFST) &  FLOW_REG_OVERFLOW_32_BIT_MSK));
		}

		valueread = (Beb_Read32(csp0base, offset) & FLOW_REG_OVERFLOW_32_BIT_MSK) >> FLOW_REG_OVERFLOW_32_BIT_OFST;
	}
	//close file pointer
	if (fd > 0)
		Beb_close(fd,csp0base);

	return valueread;
}

int Beb_SetNetworkParameter(enum NETWORKINDEX mode, int val) {

	if (!Beb_activated)
		return val;


	//mapping new memory
	u_int32_t* csp0base=0;
	u_int32_t valueread = 0;
	u_int32_t offset = TXM_DELAY_LEFT_OFFSET;
	char modename[100] = "";

	switch(mode) {
	case TXN_LEFT:
		offset = TXM_DELAY_LEFT_OFFSET;
		strcpy(modename,"Transmission Delay Left");
		break;
	case TXN_RIGHT:
		offset = TXM_DELAY_RIGHT_OFFSET;
		strcpy(modename,"Transmission Delay Right");
		break;
	case TXN_FRAME:
		offset = TXM_DELAY_FRAME_OFFSET;
		strcpy(modename,"Transmission Delay Frame");
		break;
	case FLOWCTRL_10G:
		offset = FLOW_REG_OFFSET;
		strcpy(modename,"Flow Control for 10G");
		if (val>0) val = 1;
		break;
	default: FILE_LOG(logERROR, ("Unrecognized mode in network parameter: %d\n",mode)); return -1;
	}
	//open file pointer
	int fd = Beb_open(&csp0base,XPAR_PLB_GPIO_SYS_BASEADDR);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Could not read register to set network parameter. FAIL\n"));
		return -1;
	}
	else {
		if (val > -1) {
			if (mode != FLOWCTRL_10G) {
				valueread = Beb_Read32(csp0base, offset);
				Beb_Write32(csp0base, offset,val);
			}
			// flow control reg has other bits for other control
			else {
				// reset bit
				valueread = Beb_Read32(csp0base, offset);
				Beb_Write32(csp0base, offset,valueread & ~FLOW_REG_TXM_FLOW_CNTRL_10G_MSK);

				// set bit
				valueread = Beb_Read32(csp0base, offset);
				Beb_Write32(csp0base, offset,valueread |
						((val << FLOW_REG_TXM_FLOW_CNTRL_10G_OFST) &  FLOW_REG_TXM_FLOW_CNTRL_10G_MSK));

			}

		}

		valueread = Beb_Read32(csp0base, offset);
		if (mode == FLOWCTRL_10G)
			valueread = (valueread & FLOW_REG_TXM_FLOW_CNTRL_10G_MSK) >> FLOW_REG_TXM_FLOW_CNTRL_10G_OFST;

	}
	//close file pointer
	if (fd > 0)
		Beb_close(fd,csp0base);

	return valueread;
}


int Beb_ResetToHardwareSettings() {

	if (!Beb_activated)
		return 1;

	//mapping new memory
	u_int32_t* csp0base=0;
	u_int32_t value = 0, ret = 1;

	//open file pointer
	int fd = Beb_open(&csp0base,XPAR_PLB_GPIO_SYS_BASEADDR);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Reset to Hardware Settings FAIL\n"));
	} else {
		value = Beb_Write32(csp0base, MASTERCONFIG_OFFSET,0);
		if (value) {
			FILE_LOG(logERROR, ("Could not reset to hardware settings\n"));
		} else {
			ret = 0;
		}
	}

	//close file pointer
	if (fd > 0)
		Beb_close(fd,csp0base);

	return ret;
}



u_int32_t Beb_GetFirmwareRevision() {
	//mapping new memory
	u_int32_t* csp0base=0;
	u_int32_t value = 0;

	//open file pointer
	int fd = Beb_open(&csp0base,XPAR_VERSION);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Firmware Revision Read FAIL\n"));
	} else {
		value = Beb_Read32(csp0base, FIRMWARE_VERSION_OFFSET);
		if (!value) {
			FILE_LOG(logERROR, ("Firmware Revision Number does not exist in this version\n"));
		}
	}

	//close file pointer
	if (fd > 0)
		Beb_close(fd,csp0base);

	return value;
}


u_int32_t Beb_GetFirmwareSoftwareAPIVersion() {
	//mapping new memory
	u_int32_t* csp0base=0;
	u_int32_t value = 0;

	//open file pointer
	int fd = Beb_open(&csp0base,XPAR_VERSION);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Firmware Software API Version Read FAIL\n"));
	} else {
		value = Beb_Read32(csp0base, FIRMWARESOFTWARE_API_OFFSET);
		if (!value) {
			FILE_LOG(logERROR, ("Firmware Software API Version does not exist in this version\n"));
		}
	}

	//close file pointer
	if (fd > 0)
		Beb_close(fd,csp0base);

	return value;
}

void Beb_ResetFrameNumber() {

	if (!Beb_activated)
		return;

	//mapping new memory to read master top module configuration
	u_int32_t* csp0base=0;
	//open file pointer
	int fd = Beb_open(&csp0base,XPAR_PLB_GPIO_SYS_BASEADDR);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Reset Frame Number FAIL\n"));
	} else {
		//write a 1
		Beb_Write32(csp0base, FRAME_NUM_RESET_OFFSET, 1);
		usleep(100000); //100ms
		//write a 0
		Beb_Write32(csp0base, FRAME_NUM_RESET_OFFSET, 0);
		FILE_LOG(logINFO, ("Frame Number Reset OK\n"));
		//close file pointer
		Beb_close(fd,csp0base);
	}
}


void Beb_ClearBebInfos() {
	//unsigned int i;
	//for(i=0;i<bebInfoSize;i++) free(beb_infos[i]);
	bebInfoSize=0;
}

int Beb_InitBebInfos() {//file name at some point
	Beb_ClearBebInfos();


	struct BebInfo b0;
	BebInfo_BebInfo(&b0,0);
	if (BebInfo_SetSerialAddress(&b0,0xff)) { //all bebs for reset and possibly get request data?
		beb_infos[bebInfoSize] = b0;
		bebInfoSize++;
	}

	int i0=Beb_detid,i1=0;
	if (Beb_GetBebInfoIndex(i0)) {
		FILE_LOG(logERROR, ("cant add beb. adding beb %d, beb number %d already added.\n",Beb_detid, i0));
		exit(0);
	}
	struct BebInfo b1;
	BebInfo_BebInfo(&b1,i0);
	BebInfo_SetSerialAddress(&b1,i1);
	BebInfo_SetHeaderInfo(&b1,0,(char*)"00:50:c2:46:d9:34",(char*)"129.129.205.78",42000+i0);
	BebInfo_SetHeaderInfo(&b1,1,(char*)"00:50:c2:46:d9:35",(char*)"10.0.26.1",52000+i0);
	beb_infos[bebInfoSize] = b1;
	bebInfoSize++;


	/*
  //loop through file to fill vector.
  BebInfo* b = new BebInfo(26);
    b->SetSerialAddress(0); //0xc4000000
    b->SetHeaderInfo(0,"00:50:c2:46:d9:34","129.129.205.78",42000 + 26); // 1 GbE, ip address can be acquire from the network "arp"
    b->SetHeaderInfo(1,"00:50:c2:46:d9:35","10.0.26.1",52000 + 26); //10 GbE, everything calculable/setable
    beb_infos.push_back(b);
	 */


	return Beb_CheckSourceStuffBebInfo();
}



int Beb_SetBebSrcHeaderInfos(unsigned int beb_number, int ten_gig, char* src_mac, char* src_ip,unsigned int src_port) {
	//so that the values can be reset externally for now....

	unsigned int i = 1;/*Beb_GetBebInfoIndex(beb_number);*/
	/******* if (!i) return 0;****************************/ //i must be greater than 0, zero is the global send
	BebInfo_SetHeaderInfo(&beb_infos[i],ten_gig,src_mac,src_ip,src_port);

	FILE_LOG(logINFO, ("Printing Beb info number (%d) :\n",i));
	BebInfo_Print(&beb_infos[i]);

	return 1;
}




int Beb_CheckSourceStuffBebInfo() {
	unsigned int i;
	for(i=1;i<bebInfoSize;i++) { //header stuff always starts from 1
		if (!Beb_SetHeaderData(
				BebInfo_GetBebNumber(&beb_infos[i]),0,"00:00:00:00:00:00","10.0.0.1",20000)||
				!Beb_SetHeaderData(BebInfo_GetBebNumber(&beb_infos[i]),1,"00:00:00:00:00:00","10.0.0.1",20000)) {
			FILE_LOG(logINFO, ("Error in BebInfo for module number %d.\n",BebInfo_GetBebNumber(&beb_infos[i])));
			BebInfo_Print(&beb_infos[i]);
			return 0;
		}
	}
	return 1;
}

unsigned int Beb_GetBebInfoIndex(unsigned int beb_numb) {
	/******************** if (!beb_numb) return 0;******************************/
	unsigned int i;
	for(i=1;i<bebInfoSize;i++)
		if (beb_numb==BebInfo_GetBebNumber(&beb_infos[i])) {
			FILE_LOG(logDEBUG1, ("*****found beb index:%d, for beb number:%d\n",i,beb_numb));
			return i;
		}
	FILE_LOG(logDEBUG1, ("*****Returning 0\n"));
	return 0;
}



int Beb_WriteTo(unsigned int index) {

	if (!Beb_activated)
		return 1;

	if (index>=bebInfoSize) {
		FILE_LOG(logERROR, ("WriteTo index error.\n"));
		return 0;
	}

	Beb_send_data_raw[0] = 0x90000000 | BebInfo_GetSerialAddress(&beb_infos[index]);
	if (Local_Write(ll_beb,4,Beb_send_data_raw)!=4) return 0;

	Beb_send_data_raw[0] = 0xc0000000;
	if ((Beb_send_ndata+1)*4!=Local_Write(ll_beb,(Beb_send_ndata+1)*4,Beb_send_data_raw)) return 0;

	return 1;
}


void Beb_SwapDataFun(int little_endian, unsigned int n, unsigned int *d) {
	unsigned int i;
	if (little_endian) for(i=0;i<n;i++) d[i] = (((d[i]&0xff)<<24) | ((d[i]&0xff00)<<8) | ((d[i]&0xff0000)>>8) | ((d[i]&0xff000000)>>24)); //little_endian
	else              for(i=0;i<n;i++) d[i] = (((d[i]&0xffff)<<16) | ((d[i]&0xffff0000)>>16));
}


int Beb_SetByteOrder() {
	return 1;
}


int Beb_SetUpUDPHeader(unsigned int beb_number, int ten_gig, unsigned int header_number, char* dst_mac, char* dst_ip, unsigned int dst_port) {

	if (!Beb_activated)
		return 1;

	u_int32_t bram_phy_addr;
	u_int32_t* csp0base=0;
	/*u_int32_t* bram_ptr = NULL;*/
	if (ten_gig) 
		bram_phy_addr = 0xC6002000;
	else
		bram_phy_addr = 0xC6001000;

	if (!Beb_SetHeaderData(beb_number,ten_gig,dst_mac,dst_ip,dst_port)) return 0;



	int fd = Beb_open(&csp0base,bram_phy_addr);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Set up UDP Header FAIL\n"));
	} else {
		//read data
		memcpy(csp0base+header_number*16, &udp_header, sizeof(udp_header));
		//close file pointer
		Beb_close(fd,csp0base);
	}
	return 1;
}



int Beb_SetHeaderData(unsigned int beb_number, int ten_gig, char* dst_mac, char* dst_ip, unsigned int dst_port) {
	unsigned int i = 1;/*Beb_GetBebInfoIndex(beb_number);*/
	/***********************************if (!i) return 0; *************************************///i must be greater than 0, zero is the global send
	return Beb_SetHeaderData1(BebInfo_GetSrcMAC(&beb_infos[i],ten_gig),BebInfo_GetSrcIP(&beb_infos[i],ten_gig),BebInfo_GetSrcPort(&beb_infos[i],ten_gig),dst_mac,dst_ip,dst_port);
}

int Beb_SetHeaderData1(char* src_mac, char* src_ip, unsigned int src_port, char* dst_mac, char* dst_ip, unsigned int dst_port) {
	/* example header*/
	//static unsigned int*   word_ptr   = new unsigned int [16];
	/*static*/
	/*
  udp_header_type udp_header = {
	     	{0x00, 0x50, 0xc5, 0xb2, 0xcb, 0x46},  // DST MAC
		{0x00, 0x50, 0xc2, 0x46, 0xd9, 0x02},  // SRC MAC
		{0x08, 0x00},
 		{0x45},
		{0x00},
		{0x00, 0x00},
		{0x00, 0x00},
		{0x40},
		{0x00},
		{0xff},
		{0x11},
		{0x00, 0x00}, 
		{129, 205, 205, 128},  // Src IP
	 	{129, 205, 205, 122},  // Dst IP
		{0x0f, 0xa1}, 
		{0x13, 0x89}, 		
		{0x00, 0x00}, //{0x00, 0x11},
		{0x00, 0x00}
	};
	 */

	if (!Beb_SetMAC(src_mac,&(udp_header.src_mac[0])))           return 0;
	FILE_LOG(logINFO, ("Setting Source MAC to %s\n",src_mac));
	if (!Beb_SetIP(src_ip,&(udp_header.src_ip[0])))              return 0;
	FILE_LOG(logINFO, ("Setting Source IP to %s\n",src_ip));
	if (!Beb_SetPortNumber(src_port,&(udp_header.src_port[0])))  return 0;
	FILE_LOG(logINFO, ("Setting Source port to %d\n",src_port));

	if (!Beb_SetMAC(dst_mac,&(udp_header.dst_mac[0])))           return 0;
	FILE_LOG(logINFO, ("Setting Destination MAC to %s\n",dst_mac));
	if (!Beb_SetIP(dst_ip,&(udp_header.dst_ip[0])))              return 0;
	FILE_LOG(logINFO, ("Setting Destination IP to %s\n",dst_ip));
	if (!Beb_SetPortNumber(dst_port,&(udp_header.dst_port[0])))  return 0;
	FILE_LOG(logINFO, ("Setting Destination port to %d\n",dst_port));


	Beb_AdjustIPChecksum(&udp_header);

	unsigned int* base_ptr  = (unsigned int *) &udp_header;
	unsigned int  num_words = ( sizeof(struct udp_header_type) + 3 ) / 4;
	//  for(unsigned int i=0; i<num_words; i++)  word_ptr[i] = base_ptr[i];
	//  for(unsigned int i=num_words; i<16; i++) word_ptr[i] = 0;
	//  return word_ptr;
	unsigned int i;
	for(i=0; i<num_words; i++)  Beb_send_data[i+2] = base_ptr[i];
	for(i=num_words; i<16; i++) Beb_send_data[i+2] = 0;

	return 1;
}


int Beb_SetMAC(char* mac, uint8_t* dst_ptr) {
	char macVal[50];strcpy(macVal,mac);

	int i = 0;
	char *pch = strtok (macVal,":");
	while (pch != NULL) {
		if (strlen(pch)!=2) {
			FILE_LOG(logERROR, ("Error: in mac address -> %s\n",macVal));
			return 0;
		}

		int itemp;
		sscanf(pch,"%x",&itemp);
		dst_ptr[i] = (u_int8_t)itemp;
		pch = strtok (NULL, ":");
		i++;
	}
	return 1;
}

int Beb_SetIP(char* ip, uint8_t* dst_ptr) {
	char ipVal[50];strcpy(ipVal,ip);
	int i = 0;
	char *pch = strtok (ipVal,".");
	while (pch != NULL) {
		if (((i!=3) && ((strlen(pch)>3) || (strlen(pch)<1))) || ((i==3)&&((strlen(pch)<1) || (strlen(pch) > 3)))) {
			FILE_LOG(logERROR, ("Error: in ip address -> %s\n",ipVal));
			return 0;
		}

		int itemp;
		sscanf(pch,"%d",&itemp);
		dst_ptr[i] = (u_int8_t)itemp;
		pch = strtok (NULL, ".");
		i++;
	}
	return 1;
}

int Beb_SetPortNumber(unsigned int port_number, uint8_t* dst_ptr) {
	dst_ptr[0] = (port_number >> 8) & 0xff ;
	dst_ptr[1] = port_number & 0xff;
	return 1;
}


void Beb_AdjustIPChecksum(struct udp_header_type *ip) {
	unsigned char *cptr = (unsigned char *) ip->ver_headerlen;

	ip->ip_header_checksum[0] = 0;
	ip->ip_header_checksum[1] = 0;
	ip->total_length[0] = 0;
	ip->total_length[1] = 28; // IP + UDP Header Length

	// calc ip checksum
	unsigned int ip_checksum = 0;
	unsigned int i;
	for(i=0; i<10; i++) {
		ip_checksum += ( (cptr[2*i] << 8)  + (cptr[2*i + 1]) );
		if (ip_checksum & 0x00010000) ip_checksum = (ip_checksum + 1) & 0x0000ffff;
	}

	ip->ip_header_checksum[0] = (ip_checksum >> 8) & 0xff ;
	ip->ip_header_checksum[1] = ip_checksum & 0xff ;
}



int Beb_SendMultiReadRequest(unsigned int beb_number, unsigned int left_right, int ten_gig, unsigned int dst_number, unsigned int npackets, unsigned int packet_size, int stop_read_when_fifo_empty) {

	// This is a dead function, will be removed in future
	// ==================================================

	unsigned int i = 1;/*Beb_GetBebInfoIndex(beb_number); //zero is the global send*/

	Beb_send_ndata   = 3;
	if (left_right == 1)      Beb_send_data[0] = 0x00040000;
	else if (left_right == 2) Beb_send_data[0] = 0x00080000;
	else if (left_right == 3) Beb_send_data[0] = 0x000c0000;
	else                     return 0;

	//packet_size/=2;
	if (dst_number>0x3f)   return 0;
	if (packet_size>0x3ff) return 0;
	if (npackets==0||npackets>0x100) return 0;
	npackets--;


	Beb_send_data[1] = 0x62000000 | (!stop_read_when_fifo_empty) << 27 | (ten_gig==1) << 24 | packet_size << 14 | dst_number << 8 | npackets;
	FILE_LOG(logDEBUG1, ("Beb_send_data[1]:%X\n",Beb_send_data[1]));
	Beb_send_data[2] = 0;

	Beb_SwapDataFun(0,2,&(Beb_send_data[1]));
	FILE_LOG(logDEBUG1, ("Beb_send_data[1] Swapped:%X\n",Beb_send_data[1]));

	if (Beb_activated) {
		if (!Beb_WriteTo(i)) return 0;
	}

	return 1;
}


int Beb_SetUpTransferParameters(short the_bit_mode) {
	if (the_bit_mode!=4&&the_bit_mode!=8&&the_bit_mode!=16&&the_bit_mode!=32) return 0;
	Beb_bit_mode = the_bit_mode;

	//nimages = the_number_of_images;
	//  on_dst = 0;

	return 1;
}


int Beb_StopAcquisition()
{
	if (!Beb_activated)
		return 1;

	u_int32_t* csp0base=0;
	volatile u_int32_t valuel,valuer;
	//open file pointer
	int fd = Beb_open(&csp0base,XPAR_CMD_GENERATOR);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Beb Stop Acquisition FAIL\n"));
		return 0;
	} else {
		//find value
		valuel = Beb_Read32(csp0base, (LEFT_OFFSET+STOP_ACQ_OFFSET));
		valuer = Beb_Read32(csp0base, (RIGHT_OFFSET+STOP_ACQ_OFFSET));
		//high
		Beb_Write32(csp0base, (LEFT_OFFSET + STOP_ACQ_OFFSET),(valuel|STOP_ACQ_BIT));
		Beb_Write32(csp0base, (RIGHT_OFFSET + STOP_ACQ_OFFSET),(valuer|STOP_ACQ_BIT));
		//low
		Beb_Write32(csp0base, (LEFT_OFFSET + STOP_ACQ_OFFSET),(valuel&(~STOP_ACQ_BIT)));
		Beb_Write32(csp0base, (RIGHT_OFFSET + STOP_ACQ_OFFSET),(valuer&(~STOP_ACQ_BIT)));

		FILE_LOG(logINFO, ("Beb Stop Acquisition OK\n"));
		//close file pointer
		Beb_close(fd,csp0base);
	}
	return 1;
}

int Beb_RequestNImages(unsigned int beb_number, int ten_gig, unsigned int dst_number, unsigned int nimages, int test_just_send_out_packets_no_wait) {

	if (!Beb_activated)
		return 1;

	if (dst_number>64) return 0;

	unsigned int     header_size  = 4; //4*64 bits
	unsigned int     packet_size  = ten_gig ? 0x200 : 0x80; // 4k or  1k packets
	unsigned int         npackets = ten_gig ?  Beb_bit_mode*4 : Beb_bit_mode*16;
	int          in_two_requests = (!ten_gig&&Beb_bit_mode==32);

	// volatile u_int32_t* ptrl;
	// volatile u_int32_t* ptrr;
	u_int32_t send_header_command;
	u_int32_t send_frame_command;

	if (in_two_requests) npackets/=2;

	FILE_LOG(logDEBUG1, ("----Beb_RequestNImages Start----\n"));
	FILE_LOG(logDEBUG1, ("beb_number:%X, ten_gig:%X,dst_number:%X, npackets:%X, "
			"Beb_bit_mode:%X, header_size:%X, nimages:%d, test_just_send_out_packets_no_wait:%X\n",
			beb_number, ten_gig, dst_number, npackets, Beb_bit_mode, header_size,
			nimages, test_just_send_out_packets_no_wait));

	// CMD_GEN core registers
	//
	// base for left feb fpga  + 0x000
	// base for right feb fpga + 0x100 Bytes
	//
	// OFFSETs given in Bytes
	// base+00 0xC0DE0001 (static r/o)
	// base+04 0x636D6467 (static r/o, ASCII for "CMDG")
	//
	// base+08 1st 32bits of 1st command
	// base+0c 2nd 32bits of 1st command
	//
	// base+10 1st 32bits of 2nd command
	// base+14 2nd 32bits of 2nd command
	//
	// base+18 command counter (sends n commands)
	//         <32 Bit mode : 2 commands for 1 frame neccessary (header + frame) (10 frames = 20 commands)
	//          32 Bit mode : 3 commands for 1 frame neccessary (header + 1st halfframe + 2nd halfframe) (10 frames = 30 commands)
	//         if > 0 core starts operation
	//
	// base+1c command mode (for 32 bit mode)
	//         0            for 2 command mode (send 1st command and 2nd command) (header + frame)
	//         1 on bit 31  for 3 command mode (send 1st command, 2nd command, and 2nd command) (header + 1st halfframe + 2nd halfframe)
	//
	//
	// Warning: Hard coded base address 0xc5000000 (TBD)
	//


	u_int32_t right_port_value = 0x2000;
	u_int32_t* csp0base=0;
	volatile u_int32_t value;
	//open file pointer
	int fd = Beb_open(&csp0base,XPAR_CMD_GENERATOR);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Beb Request N Images FAIL\n"));
		return 0;
	} else {
		{
			int i;
			for (i=0; i < 10; i++)
				FILE_LOG(logDEBUG1, ("%X\n",Beb_Read32(csp0base, (LEFT_OFFSET + i*4))));
		}
		// Generating commands
		send_header_command = 0x62000000 | (!test_just_send_out_packets_no_wait) << 27 | (ten_gig==1) << 24 | header_size << 14 |            0;
		send_frame_command  = 0x62000000 | (!test_just_send_out_packets_no_wait) << 27 | (ten_gig==1) << 24 | packet_size << 14 | (npackets-1);
		{
			int i;
			for (i=0; i < 10; i++)
				FILE_LOG(logDEBUG1, ("%X\n",Beb_Read32(csp0base, (LEFT_OFFSET + i*4))));
			FILE_LOG(logDEBUG1, ("%d\n",in_two_requests));
		}
		//"0x20 << 8" is dst_number (0x00 for left, 0x20 for right)
		//Left
		Beb_Write32(csp0base, (LEFT_OFFSET + FIRST_CMD_PART1_OFFSET),0);
		Beb_Write32(csp0base, (LEFT_OFFSET + FIRST_CMD_PART2_OFFSET),send_header_command);
		Beb_Write32(csp0base, (LEFT_OFFSET + SECOND_CMD_PART1_OFFSET),0);
		Beb_Write32(csp0base, (LEFT_OFFSET + SECOND_CMD_PART2_OFFSET),send_frame_command);
		value = Beb_Read32(csp0base,(LEFT_OFFSET + TWO_REQUESTS_OFFSET));
		if (in_two_requests)	Beb_Write32(csp0base, (LEFT_OFFSET + TWO_REQUESTS_OFFSET),(value | TWO_REQUESTS_BIT));
		else				Beb_Write32(csp0base, (LEFT_OFFSET + TWO_REQUESTS_OFFSET),(value &~(TWO_REQUESTS_BIT)));

		// Right
		Beb_Write32(csp0base, (RIGHT_OFFSET + FIRST_CMD_PART1_OFFSET),0);
		Beb_Write32(csp0base, (RIGHT_OFFSET + FIRST_CMD_PART2_OFFSET),send_header_command | right_port_value);
		Beb_Write32(csp0base, (RIGHT_OFFSET + SECOND_CMD_PART1_OFFSET),0);
		Beb_Write32(csp0base, (RIGHT_OFFSET + SECOND_CMD_PART2_OFFSET),send_frame_command | right_port_value);
		value = Beb_Read32(csp0base,(RIGHT_OFFSET + TWO_REQUESTS_OFFSET));
		if (in_two_requests)	Beb_Write32(csp0base, (RIGHT_OFFSET + TWO_REQUESTS_OFFSET),(value | TWO_REQUESTS_BIT));
		else				Beb_Write32(csp0base, (RIGHT_OFFSET + TWO_REQUESTS_OFFSET),(value &~(TWO_REQUESTS_BIT)));


		// Set number of frames
		Beb_Write32(csp0base, (LEFT_OFFSET + COMMAND_COUNTER_OFFSET), nimages*(2+in_two_requests));
		Beb_Write32(csp0base, (RIGHT_OFFSET + COMMAND_COUNTER_OFFSET), nimages*(2+in_two_requests));
		{
			int i;
			for (i=0; i < 10; i++)
				FILE_LOG(logDEBUG1, ("%X\n",Beb_Read32(csp0base, (LEFT_OFFSET + i*4)))); //*(ptrl+i));
			FILE_LOG(logDEBUG1, ("%d\n",in_two_requests));
		}
		Beb_close(fd,csp0base);

		FILE_LOG(logDEBUG1, ("----Beb_RequestNImages----\n"));
	}

	return 1;
}


int Beb_Test(unsigned int beb_number) {
	FILE_LOG(logINFO, ("Testing module number: %d\n",beb_number));


	//int SetUpUDPHeader(unsigned int beb_number, int ten_gig, unsigned int header_number, string dst_mac, string dst_ip, unsigned int dst_port) {
	//SetUpUDPHeader(26,0,0,"60:fb:42:f4:e3:d2","129.129.205.186",22000);

	unsigned int index = Beb_GetBebInfoIndex(beb_number);
	if (!index) {
		FILE_LOG(logERROR, ("Error beb number (%d)not in list????\n",beb_number));
		return 0;
	}

	unsigned int i;
	for(i=0;i<64;i++) {
		if (!Beb_SetUpUDPHeader(beb_number,0,i,"60:fb:42:f4:e3:d2","129.129.205.186",22000+i)) {
			FILE_LOG(logERROR, ("Error setting up header table....\n"));
			return 0;
		}
	}

	//  SendMultiReadRequest(unsigned int beb_number, unsigned int left_right, int ten_gig, unsigned int dst_number, unsigned int npackets, unsigned int packet_size, int stop_read_when_fifo_empty=1);
	for(i=0;i<64;i++) {
		if (!Beb_SendMultiReadRequest(beb_number,i%3+1,0,i,1,0,1)) {
			FILE_LOG(logERROR, ("Error requesting data....\n"));
			return 0;
		}
	}


	return 1;
}

// Returns the FPGA temperature from the xps sysmon ip core
// Temperature value is cropped and not well rounded
int Beb_GetBebFPGATemp()
{

	u_int32_t* csp0base=0;
	int temperature=0;
	int ret;
	//open file pointer
	int fd = Beb_open(&csp0base,XPAR_SYSMON_0_BASEADDR);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Module Configuration FAIL\n"));
	} else {
		//read data
		ret = Beb_Read32(csp0base, FPGA_TEMP_OFFSET);
		temperature = ((((float)(ret)/65536.0f)/0.00198421639f ) - 273.15f)*1000; // Static conversation, copied from xps sysmon standalone driver
		//close file pointer
		Beb_close(fd,csp0base);
	}

	return temperature;
}


void Beb_SetDetectorNumber(uint32_t detid) {
	if (!Beb_activated)
		return;

	uint32_t swapid = Beb_swap_uint16(detid);
	//FILE_LOG(logINFO, "detector id %d swapped %d\n", detid, swapid));
	u_int32_t* csp0base=0;
	int fd = Beb_open(&csp0base,XPAR_PLB_GPIO_TEST_BASEADDR);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Set Detector ID FAIL\n"));
		return;
	} else {
		uint32_t value = Beb_Read32(csp0base, UDP_HEADER_A_LEFT_OFST);
		value &= UDP_HEADER_X_MSK;	// to keep previous x value
		Beb_Write32(csp0base, UDP_HEADER_A_LEFT_OFST, value | ((swapid << UDP_HEADER_ID_OFST) & UDP_HEADER_ID_MSK));
		value = Beb_Read32(csp0base, UDP_HEADER_A_LEFT_OFST);
		if ((value & UDP_HEADER_ID_MSK) != ((swapid << UDP_HEADER_ID_OFST) & UDP_HEADER_ID_MSK)) {
			FILE_LOG(logERROR, ("Set Detector ID FAIL\n"));
		}
		value = Beb_Read32(csp0base, UDP_HEADER_A_RIGHT_OFST);
		value &= UDP_HEADER_X_MSK;	// to keep previous x value
		Beb_Write32(csp0base, UDP_HEADER_A_RIGHT_OFST, value | ((swapid << UDP_HEADER_ID_OFST) & UDP_HEADER_ID_MSK));
		value = Beb_Read32(csp0base, UDP_HEADER_A_RIGHT_OFST);
		if ((value & UDP_HEADER_ID_MSK) != ((swapid << UDP_HEADER_ID_OFST) & UDP_HEADER_ID_MSK)) {
			FILE_LOG(logERROR, ("Set Detector ID FAIL\n"));
		}
		Beb_close(fd,csp0base);
	}
	FILE_LOG(logINFO, ("Detector id %d set in UDP Header\n\n", detid));
}



int Beb_SetDetectorPosition(int pos[]) {
	if (!Beb_activated)
		return OK;
	FILE_LOG(logINFO, ("Got Position values %d %d...\n", pos[0],pos[1]));

	pos[0] = Beb_swap_uint16(pos[0]);
	//pos[1] = Beb_swap_uint16(pos[1]);

	int ret = FAIL;
	//mapping new memory to read master top module configuration
	u_int32_t* csp0base=0;
	//open file pointer
	int fd = Beb_open(&csp0base,XPAR_PLB_GPIO_TEST_BASEADDR);
	if (fd < 0) {
		FILE_LOG(logERROR, ("Set Detector Position FAIL\n"));
		return FAIL;
	} else {
		uint32_t value = 0;
		ret = OK;
		// x left
		value = Beb_Read32(csp0base, UDP_HEADER_A_LEFT_OFST);
		value &= UDP_HEADER_ID_MSK;	// to keep previous id value
		Beb_Write32(csp0base, UDP_HEADER_A_LEFT_OFST, value | ((pos[0] << UDP_HEADER_X_OFST) & UDP_HEADER_X_MSK));
		value = Beb_Read32(csp0base, UDP_HEADER_A_LEFT_OFST);
		if ((value & UDP_HEADER_X_MSK) != ((pos[0] << UDP_HEADER_X_OFST) & UDP_HEADER_X_MSK)) {
			FILE_LOG(logERROR, ("Could not set row position for left port\n"));
			ret = FAIL;
		}
		// x right
		value = Beb_Read32(csp0base, UDP_HEADER_A_RIGHT_OFST);
		value &= UDP_HEADER_ID_MSK;	// to keep previous id value
		Beb_Write32(csp0base, UDP_HEADER_A_RIGHT_OFST, value | ((pos[0] << UDP_HEADER_X_OFST) & UDP_HEADER_X_MSK));
		value = Beb_Read32(csp0base, UDP_HEADER_A_RIGHT_OFST);
		if ((value & UDP_HEADER_X_MSK) != ((pos[0] << UDP_HEADER_X_OFST) & UDP_HEADER_X_MSK)) {
			FILE_LOG(logERROR, ("Could not set row position for right port\n"));
			ret = FAIL;
		}



		// y left (column)
		int posval = Beb_swap_uint16(Beb_top ? pos[1] : (pos[1]+1));
		value = Beb_Read32(csp0base, UDP_HEADER_B_LEFT_OFST);
		value &= UDP_HEADER_Z_MSK;	// to keep previous z value
		Beb_Write32(csp0base, UDP_HEADER_B_LEFT_OFST, value | ((posval << UDP_HEADER_Y_OFST) & UDP_HEADER_Y_MSK));
		value = Beb_Read32(csp0base, UDP_HEADER_B_LEFT_OFST);
		if ((value & UDP_HEADER_Y_MSK) != ((posval << UDP_HEADER_Y_OFST) & UDP_HEADER_Y_MSK)) {
			FILE_LOG(logERROR, ("Could not set column position for left port\n"));
			ret = FAIL;
		}

		// y right
		value = Beb_Read32(csp0base, UDP_HEADER_B_RIGHT_OFST);
		value &= UDP_HEADER_Z_MSK;	// to keep previous z value
		posval = Beb_swap_uint16(Beb_top ? (pos[1]+1) : pos[1]);
		Beb_Write32(csp0base, UDP_HEADER_B_RIGHT_OFST, value | ((posval << UDP_HEADER_Y_OFST) & UDP_HEADER_Y_MSK));
		value = Beb_Read32(csp0base, UDP_HEADER_B_RIGHT_OFST);
		if ((value & UDP_HEADER_Y_MSK) != ((posval << UDP_HEADER_Y_OFST) & UDP_HEADER_Y_MSK)) {
			FILE_LOG(logERROR, ("Could not set column position for right port\n"));
			ret = FAIL;
		}


		//close file pointer
		Beb_close(fd,csp0base);
	}
	if (ret == OK) {
		FILE_LOG(logINFO, ("Position set to...\n"
				"\tLeft: [%d, %d]\n"
				"\tRight:[%d, %d]\n",
				Beb_swap_uint16(pos[0]), Beb_top ? pos[1] : (pos[1]+1)),
						Beb_swap_uint16(pos[0]), Beb_top ? (pos[1]+1) : pos[1]);
	}

	return ret;
}


uint16_t Beb_swap_uint16( uint16_t val) {
	return (val << 8) | (val >> 8 );
}

int Beb_open(u_int32_t** csp0base, u_int32_t offset) {

	int fd = open("/dev/mem", O_RDWR | O_SYNC, 0);
	if (fd == -1) {
		FILE_LOG(logERROR, ("\nCan't find /dev/mem!\n"));
	} else {
		FILE_LOG(logDEBUG1, ("/dev/mem opened\n"));
		*csp0base = (u_int32_t*)mmap(0, BEB_MMAP_SIZE, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, offset);
		if (*csp0base == MAP_FAILED) {
			FILE_LOG(logERROR, ("\nCan't map memmory area!!\n"));
			fd = -1;
		}
		else FILE_LOG(logDEBUG1, ("CSP0 mapped %p\n",(void*)*csp0base));
	}
	return fd;
}

u_int32_t Beb_Read32 (u_int32_t* baseaddr, u_int32_t offset) {
	volatile u_int32_t value;
	value=* (u_int32_t*)(baseaddr + offset/(sizeof(u_int32_t)));
	return value;
}


u_int32_t Beb_Write32 (u_int32_t* baseaddr, u_int32_t offset, u_int32_t data) {
	volatile u_int32_t *ptr1;
	ptr1=(u_int32_t*)(baseaddr + offset/(sizeof(u_int32_t)));
	*ptr1 = data;
	return *ptr1;
}

void Beb_close(int fd,u_int32_t* csp0base) {
	if (fd >= 0)
		close(fd);
	munmap(csp0base,BEB_MMAP_SIZE);
}
