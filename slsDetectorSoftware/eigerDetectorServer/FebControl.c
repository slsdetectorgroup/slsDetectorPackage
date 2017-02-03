
/**
 * @author Ian Johnson
 * @version 1.0
 */



#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <termios.h>  // POSIX terminal control definitions(CS8, CREAD, CLOCAL..)

#include "FebRegisterDefs.h"
#include "FebControl.h"
#include "Beb.h"



//GetDAQStatusRegister(512,current_mode_bits_from_fpga)){

unsigned int Module_ndacs = 16;
char  Module_dac_names[16][10]= {"SvP","Vtr","Vrf","Vrs","SvN","Vtgstv","Vcmp_ll","Vcmp_lr","cal","Vcmp_rl","rxb_rb","rxb_lb","Vcmp_rr","Vcp","Vcn","Vis"};;




struct Module modules[10];
int moduleSize = 0;

unsigned int Feb_Control_staticBits;   //program=1,m4=2,m8=4,test=8,rotest=16,cs_bar_left=32,cs_bar_right=64
unsigned int Feb_Control_acquireNReadoutMode; //safe or parallel, half or full speed
unsigned int Feb_Control_triggerMode;         //internal timer, external start, external window, signal polarity (external trigger and enable)
unsigned int Feb_Control_externalEnableMode;  //external enabling engaged and it's polarity
unsigned int Feb_Control_subFrameMode;


unsigned int Feb_Control_nimages;
double Feb_Control_exposure_time_in_sec;
int64_t Feb_Control_subframe_exposure_time_in_10nsec;
double Feb_Control_exposure_period_in_sec;

int64_t Feb_Control_RateTable_Tau_in_nsec = -1;
int64_t Feb_Control_RateTable_Period_in_nsec = -1;

unsigned int   Feb_Control_trimbit_size;
unsigned int* Feb_Control_last_downloaded_trimbits;


int Feb_Control_module_number;
int Feb_Control_current_index;

int Feb_Control_counter_bit = 1;
int Feb_control_master = 0;
int Feb_control_normal = 0;

unsigned int Feb_Control_rate_correction_table[1024];
double Feb_Control_rate_meas[16384];

double ratemax=-1;
int Feb_Control_activated = 1;
int Feb_Control_hv_fd = -1;


void Module_Module(struct Module* mod,unsigned int number, unsigned int address_top){
	unsigned int i;
	mod->module_number        = number;
	mod->top_address_valid    = 1;
	mod->top_left_address     = 0x100 | (0xff & address_top);
	mod->top_right_address    = (0x200 | (0xff & address_top));
	mod-> bottom_address_valid = 0;
	mod-> bottom_left_address  = 0;
	mod-> bottom_right_address = 0;

	mod->high_voltage          = -1;
	mod->top_dac              = malloc(Module_ndacs * sizeof(int));
	mod->bottom_dac           = malloc(Module_ndacs * sizeof(int));
	for(i=0;i<Module_ndacs;i++) mod->top_dac[i]    = mod->top_address_valid    ? -1:0;
	for(i=0;i<Module_ndacs;i++) mod->bottom_dac[i] = mod->bottom_address_valid ? -1:0;
}


void Module_ModuleBottom(struct Module* mod,unsigned int number, unsigned int address_bottom){
	unsigned int i;
	mod->module_number        = number;
	mod->top_address_valid    = 0;
	mod->top_left_address     = 0;
	mod->top_right_address    = 0;
	mod-> bottom_address_valid = 1;
	mod-> bottom_left_address  = 0x100 | (0xff & address_bottom);
	mod-> bottom_right_address = (0x200 | (0xff & address_bottom));

	mod->high_voltage          = -1;

	for(i=0;i<4;i++) mod->idelay_top[i]=mod->idelay_bottom[i]=0;

	mod->top_dac              = malloc(Module_ndacs * sizeof(int));
	mod->bottom_dac           = malloc(Module_ndacs * sizeof(int));
	for(i=0;i<Module_ndacs;i++) mod->top_dac[i]    = mod->top_address_valid    ? -1:0;
	for(i=0;i<Module_ndacs;i++) mod->bottom_dac[i] = mod->bottom_address_valid ? -1:0;
}



void Module_Module1(struct Module* mod,unsigned int number, unsigned int address_top, unsigned int address_bottom){
	unsigned int i;
	mod->module_number        = number;
	mod->top_address_valid    = 1;
	mod->top_left_address     = 0x100 | (0xff & address_top);
	mod->top_right_address    = 0x200 | (0xff & address_top);
	mod->bottom_address_valid = 1;
	mod->bottom_left_address  = 0x100 | (0xff & address_bottom);
	mod->bottom_right_address = 0x200 | (0xff & address_bottom);

	mod->high_voltage         = -1;

	for(i=0;i<4;i++) mod->idelay_top[i]=mod->idelay_bottom[i]=0;

	mod->top_dac              = malloc(Module_ndacs * sizeof(int));
	mod->bottom_dac           = malloc(Module_ndacs * sizeof(int));
	for(i=0;i<Module_ndacs;i++) mod->top_dac[i]    = mod->top_address_valid    ? -1:0;
	for(i=0;i<Module_ndacs;i++) mod->bottom_dac[i] = mod->bottom_address_valid ? -1:0;
}


unsigned int Module_GetModuleNumber(struct Module* mod)       {return mod->module_number;}
int         Module_TopAddressIsValid(struct Module* mod)     {return mod->top_address_valid;}
unsigned int Module_GetTopBaseAddress(struct Module* mod)     {return (mod->top_left_address&0xff);}
unsigned int Module_GetTopLeftAddress(struct Module* mod)     {return mod->top_left_address;}
unsigned int Module_GetTopRightAddress(struct Module* mod)    {return mod->top_right_address;}
unsigned int Module_GetBottomBaseAddress(struct Module* mod)  {return (mod->bottom_left_address&0xff);}
int         Module_BottomAddressIsValid(struct Module* mod)  {return mod->bottom_address_valid;}
unsigned int Module_GetBottomLeftAddress(struct Module* mod)  {return mod->bottom_left_address;}
unsigned int Module_GetBottomRightAddress(struct Module* mod) {return mod->bottom_right_address;}

unsigned int Module_SetTopIDelay(struct Module* mod,unsigned int chip,unsigned int value)    { return Module_TopAddressIsValid(mod) &&chip<4        ? (mod->idelay_top[chip]=value)    : 0;} //chip 0=ll,1=lr,0=rl,1=rr
unsigned int Module_GetTopIDelay(struct Module* mod,unsigned int chip)                       { return chip<4                              			?  mod->idelay_top[chip]           : 0;} //chip 0=ll,1=lr,0=rl,1=rr
unsigned int Module_SetBottomIDelay(struct Module* mod,unsigned int chip,unsigned int value) { return Module_BottomAddressIsValid(mod) &&chip<4     ? (mod->idelay_bottom[chip]=value) : 0;} //chip 0=ll,1=lr,0=rl,1=rr
unsigned int Module_GetBottomIDelay(struct Module* mod,unsigned int chip)                    { return chip<4                              			?  mod->idelay_bottom[chip]        : 0;} //chip 0=ll,1=lr,0=rl,1=rr

float        Module_SetHighVoltage(struct Module* mod,float value)                  { return Feb_control_master ? (mod->high_voltage=value) : -1;}// Module_TopAddressIsValid(mod) ? (mod->high_voltage=value) : -1;}
float        Module_GetHighVoltage(struct Module* mod)                              { return mod->high_voltage;}

int          Module_SetTopDACValue(struct Module* mod,unsigned int i, int value) 	{ return (i<Module_ndacs && Module_TopAddressIsValid(mod))		? (mod->top_dac[i]=value)   : -1;}
int          Module_GetTopDACValue(struct Module* mod,unsigned int i)               { return (i<Module_ndacs) 										? mod->top_dac[i]			: -1;}
int          Module_SetBottomDACValue(struct Module* mod,unsigned int i, int value) { return (i<Module_ndacs && Module_BottomAddressIsValid(mod)) 	? (mod->bottom_dac[i]=value): -1;}
int          Module_GetBottomDACValue(struct Module* mod,unsigned int i)            { return (i<Module_ndacs) 									   	? mod->bottom_dac[i]		: -1;}



void Feb_Control_activate(int activate){
	Feb_Control_activated = activate;
}

int Feb_Control_IsBottomModule(){
	if(Module_BottomAddressIsValid(&modules[Feb_Control_current_index]))
		return 1;
	return 0;
}


int Feb_Control_GetModuleNumber(){
	return Feb_Control_module_number;
}


void Feb_Control_FebControl(){
	Feb_Control_staticBits=Feb_Control_acquireNReadoutMode=Feb_Control_triggerMode=Feb_Control_externalEnableMode=Feb_Control_subFrameMode=0;
	Feb_Control_trimbit_size=263680;
	Feb_Control_last_downloaded_trimbits = malloc(Feb_Control_trimbit_size * sizeof(int));
	moduleSize = 0;
}




int Feb_Control_Init(int master, int top, int normal, int module_num){
	unsigned int i;
	Feb_Control_module_number = 0;
	Feb_Control_current_index = 0;
	Feb_control_master = master;
	Feb_control_normal =  normal;

	//global send
	Feb_Control_AddModule1(0,1,0xff,0,1);
	Feb_Control_PrintModuleList();
	Feb_Control_module_number = (module_num & 0xFF);

	int serial = !top;
	printf("serial: %d\n",serial);

	Feb_Control_current_index = 1;


	//Add the half module
	Feb_Control_AddModule1(Feb_Control_module_number,top,serial,serial,1);
	Feb_Control_PrintModuleList();


	unsigned int nfebs = 0;
	unsigned int* feb_list = malloc(moduleSize*4 * sizeof(unsigned int));
	for(i=1;i<moduleSize;i++){
		if(Module_TopAddressIsValid(&modules[i])){
			feb_list[nfebs++] = Module_GetTopRightAddress(&modules[i]);
			feb_list[nfebs++] = Module_GetTopLeftAddress(&modules[i]);
		}
		if(Module_BottomAddressIsValid(&modules[i])){
			feb_list[nfebs++] = Module_GetBottomRightAddress(&modules[i]);
			feb_list[nfebs++] = Module_GetBottomLeftAddress(&modules[i]);
		}
	}

	Feb_Interface_SendCompleteList(nfebs,feb_list);
	free(feb_list);
	printf("\n");
	if(Feb_Control_activated)
		Feb_Interface_SetByteOrder();

	return 1;
}


int Feb_Control_OpenSerialCommunication(){
	cprintf(BG_BLUE,"opening serial communication of hv\n");
	if(Feb_Control_hv_fd != -1)
		close(Feb_Control_hv_fd);
	Feb_Control_hv_fd = open(SPECIAL9M_HIGHVOLTAGE_PORT, O_RDWR | O_NOCTTY);
	if(Feb_Control_hv_fd < 0){
		cprintf(RED,"Warning: Unable to open port %s to set up high voltage serial communciation to the blackfin\n", SPECIAL9M_HIGHVOLTAGE_PORT);
		return 0;
	}

	struct termios serial_conf;
	// Get the current options for the port
	tcgetattr(Feb_Control_hv_fd, &serial_conf);
	// reset structure
	memset(&serial_conf,0,sizeof(serial_conf));
	// control options
	serial_conf.c_cflag = B2400 | CS8 | CREAD | CLOCAL;//57600 too high
	// input options
	serial_conf.c_iflag = IGNPAR;
	// output options
	serial_conf.c_oflag = 0;
	// line options
	serial_conf.c_lflag = ICANON;
	// flush input
	tcflush(Feb_Control_hv_fd, TCIFLUSH);
	// set new options for the port, TCSANOW:changes occur immediately without waiting for data to complete
	tcsetattr(Feb_Control_hv_fd, TCSANOW, &serial_conf);

	return 1;
}

void Feb_Control_CloseSerialCommunication(){
	if(Feb_Control_hv_fd != -1)
		close(Feb_Control_hv_fd);
}


void Feb_Control_PrintModuleList(){
	unsigned int i;
	printf("\tModule list:\n");
	for(i=0;i<moduleSize;i++){
		if(i==0)      printf("\t\t%d) All    modules: ",i);
		else if(i==1) printf("\t\t%d) Master module : ",i);
		else          printf("\t\t%d)        module : ",i);
		printf("%d    ",Module_GetModuleNumber(&modules[i]));
		if(Module_TopAddressIsValid(&modules[i]))    printf("0x%x (top)    ",Module_GetTopBaseAddress(&modules[i]));
		if(Module_BottomAddressIsValid(&modules[i])) printf("0x%x (bottom)    ",Module_GetBottomBaseAddress(&modules[i]));

		printf("\n");
	}
}

int Feb_Control_GetModuleIndex(unsigned int module_number, unsigned int* module_index){
	unsigned int i;
	for(i=0;i<moduleSize;i++){
		if(Module_GetModuleNumber(&modules[i])==module_number){
			*module_index=i;
			return 1;
		}
	}

	return 0;
}

int Feb_Control_CheckModuleAddresses(struct Module* m){
	unsigned int i;
	int found_t = 0;
	int found_b = 0;
	for(i=0;i<moduleSize;i++){
		if((Module_TopAddressIsValid(m)    && Module_GetTopBaseAddress(&modules[i])    && Module_GetTopBaseAddress(m)==Module_GetTopBaseAddress(&modules[i])) ||
				(Module_TopAddressIsValid(m)    && Module_GetBottomBaseAddress(&modules[i]) && Module_GetTopBaseAddress(m)==Module_GetBottomBaseAddress(&modules[i])))       found_t=1;
		if((Module_BottomAddressIsValid(m)    && Module_GetTopBaseAddress(&modules[i])    && Module_GetBottomBaseAddress(m)==Module_GetTopBaseAddress(&modules[i])) ||
				(Module_BottomAddressIsValid(m)    && Module_GetBottomBaseAddress(&modules[i]) && Module_GetBottomBaseAddress(m)==Module_GetBottomBaseAddress(&modules[i]))) found_b=1;
	}

	if(found_t) cprintf(RED,"\tWarning: top address %d already used.\n",Module_GetTopBaseAddress(m));
	if(found_b) cprintf(RED,"\tWarning: bottom address %d already used.\n",Module_GetBottomBaseAddress(m));


	int top_bottom_same = Module_TopAddressIsValid(m)&&Module_BottomAddressIsValid(m)&&Module_GetTopBaseAddress(m)==Module_GetBottomBaseAddress(m);
	if(top_bottom_same) cprintf(RED,"\tWarning: top and bottom address are the same %d.\n",Module_GetTopBaseAddress(m));

	return !(top_bottom_same||found_t||found_b);
}

int Feb_Control_AddModule(unsigned int module_number, unsigned int top_address){
	return Feb_Control_AddModule1(module_number,1,top_address,0,1);
}
int Feb_Control_AddModule1(unsigned int module_number, int top_enable, unsigned int top_address, unsigned int bottom_address, int half_module){ //bot_address 0 for half module
	int parameters_ok  = 1;
	unsigned int pre_module_index = 0;
	if(Feb_Control_GetModuleIndex(module_number,&pre_module_index)){
		printf("\tRemoving previous assignment of module number %d.\n",module_number);
		// free(modules[pre_module_index]);
		int i;
		for(i=pre_module_index;i<moduleSize-1;i++)
			modules[i] = modules[i+1];
		moduleSize--;
		parameters_ok = 0;
	}

	struct Module mod,* m;
	m= &mod;

	/* if((half_module)&& (top_address != 1)) Module_Module(m,module_number,top_address);
  else if(half_module)  Module_ModuleBottom(m,module_number,top_address);*/
	if ((half_module)&& (top_enable)) Module_Module(m,module_number,top_address);
	else if (half_module)  Module_ModuleBottom(m,module_number,bottom_address);
	else            Module_Module1(m,module_number,top_address,bottom_address);


	parameters_ok&=Feb_Control_CheckModuleAddresses(m);



	if(Module_TopAddressIsValid(m)&&Module_BottomAddressIsValid(m)){
		printf("\tAdding full module number %d with top and bottom base addresses: %d %d\n",Module_GetModuleNumber(m),Module_GetTopBaseAddress(m),Module_GetBottomBaseAddress(m));
		modules[moduleSize] = mod;
		moduleSize++;
	}else if(Module_TopAddressIsValid(m)){
		printf("\tAdding half module number %d with top base address: %d\n",Module_GetModuleNumber(m),Module_GetTopBaseAddress(m));
		modules[moduleSize] = mod;
		moduleSize++;
	}else if(Module_BottomAddressIsValid(m)){
		printf("\tAdding half module number %d with bottom base address: %d\n",Module_GetModuleNumber(m),Module_GetBottomBaseAddress(m));
		modules[moduleSize] = mod;
		moduleSize++;
	}else{
		//free(m);
	}

	return parameters_ok;
}



int Feb_Control_CheckSetup(int master){
	printf("Checking Set up\n");
	unsigned int i,j;
	int ok = 1;

	/*for(i=0;i<moduleSize;i++){*/
	i = Feb_Control_current_index;

	for(j=0;j<4;j++){
		if(Module_GetTopIDelay(&modules[i],j)<0){
			cprintf(RED,"Warning: module %d's idelay top number %d not set.\n",Module_GetModuleNumber(&modules[i]),j);
			ok=0;
		}
		if(Module_GetBottomIDelay(&modules[i],j)<0){
			cprintf(RED,"Warning: module %d's idelay bottom number %d not set.\n",Module_GetModuleNumber(&modules[i]),j);
			ok=0;
		}
	}
	int value = 0;
	if((Feb_control_master) && (!Feb_Control_GetHighVoltage(&value))){
		cprintf(RED,"Warning: module %d's high voltage not set.\n",Module_GetModuleNumber(&modules[i]));
		ok=0;
	}
	for(j=0;j<Module_ndacs;j++){
		if(Module_GetTopDACValue(&modules[i],j)<0){
			cprintf(RED,"Warning: module %d's top \"%s\" dac is not set.\n",Module_GetModuleNumber(&modules[i]),Module_dac_names[i]);
			ok=0;
		}
		if(Module_GetBottomDACValue(&modules[i],j)<0){
			cprintf(RED,"Warning: module %d's bottom \"%s\" dac is not set.\n",Module_GetModuleNumber(&modules[i]),Module_dac_names[i]);
			ok=0;
		}
	}
	/* }*/
	printf("Done Checking Set up\n");
	return ok;
}

unsigned int Feb_Control_GetNModules(){
	if(moduleSize<=0) return 0;
	return moduleSize - 1;
}

unsigned int Feb_Control_GetNHalfModules(){
	unsigned int n_half_modules = 0;
	unsigned int i;
	for(i=1;i<moduleSize;i++){
		if(Module_TopAddressIsValid(&modules[i]))    n_half_modules++;
		if(Module_BottomAddressIsValid(&modules[i])) n_half_modules++;
	}

	return n_half_modules;
}


int Feb_Control_SetIDelays(unsigned int module_num, unsigned int ndelay_units){
	return Feb_Control_SetIDelays1(module_num,0,ndelay_units)&&Feb_Control_SetIDelays1(module_num,1,ndelay_units)&&Feb_Control_SetIDelays1(module_num,2,ndelay_units)&&Feb_Control_SetIDelays1(module_num,3,ndelay_units);
}

int Feb_Control_SetIDelays1(unsigned int module_num, unsigned int chip_pos, unsigned int ndelay_units){  //chip_pos 0=ll,1=lr,0=rl,1=rr
	unsigned int i;
	//currently set same for top and bottom
	if(chip_pos>3){
		cprintf(RED,"Error SetIDelay chip_pos %d doesn't exist.\n",chip_pos);;
		return 0;
	}

	unsigned int module_index=0;
	if(!Feb_Control_GetModuleIndex(module_num,&module_index)){
		cprintf(RED,"Error could not set i delay module number %d invalid.\n",module_num);
		return 0;
	}

	int ok = 1;
	if(chip_pos/2==0){ //left fpga
		if(Module_TopAddressIsValid(&modules[module_index])){
			if(Feb_Control_SendIDelays(Module_GetTopLeftAddress(&modules[module_index]),chip_pos%2==0,0xffffffff,ndelay_units)){
				if(module_index!=0) Module_SetTopIDelay(&modules[module_index],chip_pos,ndelay_units);
				else{
					for(i=0;i<moduleSize;i++) Module_SetTopIDelay(&modules[i],chip_pos,ndelay_units);
					for(i=0;i<moduleSize;i++) Module_SetBottomIDelay(&modules[i],chip_pos,ndelay_units);
				}
			}else{
				cprintf(RED,"Error could not set idelay module number %d (top_left).\n",module_num);
				ok=0;
			}
		}
		if(Module_BottomAddressIsValid(&modules[module_index])){
			if(Feb_Control_SendIDelays(Module_GetBottomLeftAddress(&modules[module_index]),chip_pos%2==0,0xffffffff,ndelay_units)){
				if(module_index!=0) Module_SetBottomIDelay(&modules[module_index],chip_pos,ndelay_units);
				else{
					for(i=0;i<moduleSize;i++) Module_SetTopIDelay(&modules[i],chip_pos,ndelay_units);
					for(i=0;i<moduleSize;i++) Module_SetBottomIDelay(&modules[i],chip_pos,ndelay_units);
				}
			}else{
				cprintf(RED,"Error could not set idelay module number %d (bottom_left).\n",module_num);
				ok=0;
			}
		}
	}else{
		if(Module_TopAddressIsValid(&modules[module_index])){
			if(Feb_Control_SendIDelays(Module_GetTopRightAddress(&modules[module_index]),chip_pos%2==0,0xffffffff,ndelay_units)){
				if(module_index!=0) Module_SetTopIDelay(&modules[module_index],chip_pos,ndelay_units);
				else for(i=0;i<moduleSize;i++) Module_SetTopIDelay(&modules[i],chip_pos,ndelay_units);
			}else{
				cprintf(RED,"Error could not set idelay module number %d (top_right).\n",module_num);
				ok=0;
			}
		}
		if(Module_BottomAddressIsValid(&modules[module_index])){
			if(Feb_Control_SendIDelays(Module_GetBottomRightAddress(&modules[module_index]),chip_pos%2==0,0xffffffff,ndelay_units)){
				if(module_index!=0) Module_SetBottomIDelay(&modules[module_index],chip_pos,ndelay_units);
				else for(i=0;i<moduleSize;i++) Module_SetBottomIDelay(&modules[i],chip_pos,ndelay_units);
			}else{
				cprintf(RED,"Error could not set idelay module number %d (bottom_right).\n",module_num);
				ok=0;
			}
		}
	}

	return ok;
}


int Feb_Control_SendIDelays(unsigned int dst_num, int chip_lr, unsigned int channels, unsigned int ndelay_units){
	//  printf("sending idelay :"<<dst_num<<" (lr-"<<chip_lr<<") to "<<ndelay_units<<endl;

	if(ndelay_units>0x3ff) ndelay_units=0x3ff;
	// this is global
	unsigned int delay_data_valid_nclks =  15 - ((ndelay_units&0x3c0)>>6); //data valid delay upto 15 clks
	ndelay_units &= 0x3f;

	unsigned int set_left_delay_channels  = chip_lr ? channels:0;
	unsigned int set_right_delay_channels = chip_lr ?        0:channels;

	printf("\tSetting delays of ");
	if(set_left_delay_channels!=0)       printf("left chips of dst_num %d",dst_num);
	else if(set_right_delay_channels!=0) printf("right chips of dst_num %d",dst_num);

	printf(", tracks 0x%x to: %d, %d clks and %d units.\n",channels,(((15-delay_data_valid_nclks)<<6)|ndelay_units),delay_data_valid_nclks,ndelay_units);

	if(Feb_Control_activated){
		if(!Feb_Interface_WriteRegister(dst_num,CHIP_DATA_OUT_DELAY_REG2, 1<<31 | delay_data_valid_nclks<<16 | ndelay_units,0,0) || //the 1<<31 time enables the setting of the data valid delays
				!Feb_Interface_WriteRegister(dst_num,CHIP_DATA_OUT_DELAY_REG3,set_left_delay_channels,0,0)  ||
				!Feb_Interface_WriteRegister(dst_num,CHIP_DATA_OUT_DELAY_REG4,set_right_delay_channels,0,0) ||
				!Feb_Interface_WriteRegister(dst_num,CHIP_DATA_OUT_DELAY_REG_CTRL,CHIP_DATA_OUT_DELAY_SET,1,1)){
			cprintf(RED,"Warning: could not SetChipDataInputDelays(...).\n");
			return 0;
		}
	}

	return 1;
}


int Feb_Control_VoltageToDAC(float value, unsigned int* digital,unsigned int nsteps,float vmin,float vmax){
	if(value<vmin||value>vmax) return 0;
	*digital = (int)(((value-vmin)/(vmax-vmin))*(nsteps-1) + 0.5);
	return 1;
}

float Feb_Control_DACToVoltage(unsigned int digital,unsigned int nsteps,float vmin,float vmax){
	return vmin+(vmax-vmin)*digital/(nsteps-1);
}


//only master gets to call this function
int Feb_Control_SetHighVoltage(int value){
	printf(" Setting High Voltage:\t");
	/*
	 * maximum voltage of the hv dc/dc converter:
	 * 300 for single module power distribution board
	 * 200 for 9M power distribution board
	 * but limit is 200V for both
	 */
	const float vmin=0;
	float vmax=200;
	if(Feb_control_normal)
		vmax=300;
	const float vlimit=200;
	const unsigned int ntotalsteps = 256;
	unsigned int nsteps = ntotalsteps*vlimit/vmax;
	unsigned int dacval = 0;

	//calculate dac value
	if(!Feb_Control_VoltageToDAC(value,&dacval,nsteps,vmin,vlimit)){
		cprintf(RED,"\nWarning: SetHighVoltage bad value, %d.  The range is 0 to %d V.\n",value, (int)vlimit);
		return -1;
	}
	printf("(%d dac):\t%dV\n", dacval, value);

	return Feb_Control_SendHighVoltage(dacval);
}


int Feb_Control_GetHighVoltage(int* value){
	printf(" Getting High Voltage:\t");
	unsigned int dacval = 0;

	if(!Feb_Control_ReceiveHighVoltage(&dacval))
		return 0;

	//ok, convert dac to v
	/*
	 * maximum voltage of the hv dc/dc converter:
	 * 300 for single module power distribution board
	 * 200 for 9M power distribution board
	 * but limit is 200V for both
	 */
	const float vmin=0;
	float vmax=200;
	if(Feb_control_normal)
		vmax=300;
	const float vlimit=200;
	const unsigned int ntotalsteps = 256;
	unsigned int nsteps = ntotalsteps*vlimit/vmax;
	*value = (int)(Feb_Control_DACToVoltage(dacval,nsteps,vmin,vlimit)+0.5);
	printf("(%d dac)\t%dV\n", dacval, *value);

	return 1;
}


int Feb_Control_SendHighVoltage(int dacvalue){
	//normal
	if(Feb_control_normal){
		//open file
		FILE* fd=fopen(NORMAL_HIGHVOLTAGE_OUTPUTPORT,"w");
		if(fd==NULL){
			cprintf(RED,"\nWarning: Could not open file for writing to set high voltage\n");
			return 0;
		}
		//convert to string, add 0 and write to file
		fprintf(fd, "%d0\n", dacvalue);
		fclose(fd);
	}

	//9m
	else{
		/*Feb_Control_OpenSerialCommunication();*/
		if (Feb_Control_hv_fd == -1){
			cprintf(RED,"\nWarning: High voltage serial communication not set up for 9m\n");
			return 0;
		}

		char buffer[SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE];
		buffer[SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE-2]='\0';
		buffer[SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE-1]='\n';
		int n;
		sprintf(buffer,"p%d ",dacvalue);
		n = write(Feb_Control_hv_fd, buffer, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
		if (n < 0) {
			cprintf(RED,"\nWarning: Error writing to i2c bus\n");
			return 0;
		}
#ifdef VERBOSEI
		cprintf(BLUE,"Sent %d Bytes\n", n);
#endif
		//ok/fail
		n = read(Feb_Control_hv_fd, buffer, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
		if (n < 0) {
			cprintf(RED,"\nWarning: Error reading from i2c bus\n");
			return 0;
		}
#ifdef VERBOSEI
		cprintf(BLUE,"Received %d Bytes\n", n);
#endif
		fflush(stdout);
		/*Feb_Control_CloseSerialCommunication();*/
		if(buffer[0] != 's'){
			cprintf(RED,"\nError: Failed to set high voltage\n");
			return 0;
		}
		cprintf(GREEN,"%s\n",buffer);

	}

	return 1;
}







int Feb_Control_ReceiveHighVoltage(unsigned int* value){

	//normal
	if(Feb_control_normal){
		//open file
		FILE* fd=fopen(NORMAL_HIGHVOLTAGE_INPUTPORT,"r");
		if(fd==NULL){
			cprintf(RED,"\nWarning: Could not open file for writing to get high voltage\n");
			return 0;
		}

		//read, assigning line to null and readbytes to 0 then getline allocates initial buffer
		size_t readbytes=0;
		char* line=NULL;
		if(getline(&line, &readbytes, fd) == -1){
			cprintf(RED,"\nWarning: could not read file to get high voltage\n");
			return 0;
		}
		//read again to read the updated value
		rewind(fd);
		free(line);
		readbytes=0;
		readbytes = getline(&line, &readbytes, fd);
		if(readbytes == -1){
			cprintf(RED,"\nWarning: could not read file to get high voltage\n");
			return 0;
		}
		// Remove the trailing 0
		*value = atoi(line)/10;
		free(line);
		fclose(fd);
	}


	//9m
	else{
		/*Feb_Control_OpenSerialCommunication();*/

		if (Feb_Control_hv_fd == -1){
			cprintf(RED,"\nWarning: High voltage serial communication not set up for 9m\n");
			return 0;
		}
		char buffer[SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE];
		buffer[SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE-2]='\0';
		buffer[SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE-1]='\n';
		int n = 0;
		//request
		strcpy(buffer,"g ");
		n = write(Feb_Control_hv_fd, buffer, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
		if (n < 0) {
			cprintf(RED,"\nWarning: Error writing to i2c bus\n");
			return 0;
		}
#ifdef VERBOSEI
		cprintf(BLUE,"Sent %d Bytes\n", n);
#endif

		//ok/fail
		n = read(Feb_Control_hv_fd, buffer, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
		if (n < 0) {
			cprintf(RED,"\nWarning: Error reading from i2c bus\n");
			return 0;
		}
#ifdef VERBOSEI
		cprintf(BLUE,"Received %d Bytes\n", n);
#endif
		if(buffer[0] != 's'){
			cprintf(RED,"\nWarning: failed to read high voltage\n");
			return 0;
		}

		n = read(Feb_Control_hv_fd, buffer, SPECIAL9M_HIGHVOLTAGE_BUFFERSIZE);
		if (n < 0) {
			cprintf(RED,"\nWarning: Error reading from i2c bus\n");
			return 0;
		}
#ifdef VERBOSEI
		cprintf(BLUE,"Received %d Bytes\n", n);
#endif
		/*Feb_Control_OpenSerialCommunication();*/
		if (!sscanf(buffer,"%d",value)){
			cprintf(RED,"\nWarning: failed to scan high voltage read\n");
			return 0;
		}
	}
	return 1;
}




int Feb_Control_DecodeDACString(char* dac_str, unsigned int* module_index, int* top, int* bottom, unsigned int* dac_ch){
	char*       local_s = dac_str;
	*module_index  = Feb_Control_current_index;
	*top    = 1;//make them both 1 instead of this
	*bottom = 1;

	if(Module_BottomAddressIsValid(&modules[*module_index]))
		*top=0;
	else
		*bottom=0;

	*dac_ch = 0;
	if(!Feb_Control_GetDACNumber(local_s,dac_ch)){
		cprintf(RED,"Error in dac_name: %s  (%s)\n",dac_str,local_s);
		return 0;
	}

	return 1;
}

int Feb_Control_SetDAC(char* dac_str, int value, int is_a_voltage_mv){
	unsigned int i;
	unsigned int module_index, dac_ch;
	int top, bottom;
	if(!Feb_Control_DecodeDACString(dac_str,&module_index,&top,&bottom,&dac_ch)) return 0;

	unsigned int v = value;
	if(is_a_voltage_mv&&!Feb_Control_VoltageToDAC(value,&v,4096,0,2048)){
		cprintf(RED,"Warning: SetDac bad value, %d. The range is 0 to 2048 mV.\n",value);
		return 0;
	}
	if(v<0||v>4095){
		cprintf(RED,"Warning: SetDac bad value, %d. The range is 0 to 4095.\n",v);
		return 0;
	}

	if(top&&Module_TopAddressIsValid(&modules[module_index])){

		if(!Feb_Control_SendDACValue(Module_GetTopRightAddress(&modules[module_index]),dac_ch,&v)) return 0;


		if(module_index!=0)   Module_SetTopDACValue(&modules[module_index],dac_ch,v);
		else for(i=0;i<moduleSize;i++) Module_SetTopDACValue(&modules[i],dac_ch,v);
	}

	if(bottom&&Module_BottomAddressIsValid(&modules[module_index])){
		if(!Feb_Control_SendDACValue(Module_GetBottomRightAddress(&modules[module_index]),dac_ch,&v))return 0;
		if(module_index!=0) Module_SetBottomDACValue(&modules[module_index],dac_ch,v);
		else for(i=0;i<moduleSize;i++)   Module_SetBottomDACValue(&modules[i],dac_ch,v);
	}

	return 1;
}

int Feb_Control_GetDAC(char* s, int* ret_value, int voltage_mv){

	unsigned int module_index, dac_ch;
	int top, bottom;
	if(!Feb_Control_DecodeDACString(s,&module_index,&top,&bottom,&dac_ch)) return 0;

	*ret_value = top ? Module_GetTopDACValue(&modules[module_index],dac_ch) : Module_GetBottomDACValue(&modules[module_index],dac_ch);

	if(voltage_mv) *ret_value = Feb_Control_DACToVoltage(*ret_value,4096,0,2048);

	return 1;
}


int Feb_Control_GetDACName(unsigned int dac_num, char* s){
	if(dac_num>=Module_ndacs){
		cprintf(RED,"Warning: GetDACName index out of range, %d invalid.\n",dac_num);
		return 0;
	}
	strcpy(s,Module_dac_names[dac_num]);
	return 1;
}

int Feb_Control_GetDACNumber(char* s, unsigned int* n){
	unsigned int i;
	for(i=0;i<Module_ndacs;i++){
		if(!strcmp(Module_dac_names[i],s)){
			*n=i;
			return 1;
		}
	}

	return 0;
}


int Feb_Control_SendDACValue(unsigned int dst_num, unsigned int ch, unsigned int* value){

	if(ch<0||ch>15){
		cprintf(RED,"Warning invalid ch for SetDAC.\n");
		return 0;
	}

	//if(voltage<0) return PowerDownDAC(socket_num,ch);

	*value&=0xfff;
	unsigned int dac_ic = (ch<8) ? 1:2;
	unsigned int dac_ch =  ch%8;
	unsigned int r      = dac_ic<<30 | 3<<16 | dac_ch<<12 | *value; //3 write and power up


	if(Feb_Control_activated){
		if(!Feb_Interface_WriteRegister(dst_num,0,r,1,0)){
			cprintf(RED,"Warning: trouble setting dac %d voltage.\n",ch);
			return 0;
		}
	}

	float voltage=Feb_Control_DACToVoltage(*value,4096,0,2048);

	printf("\tDac number %d (%s) of dst %d set to %d (%f mV).\n",ch,Module_dac_names[ch],dst_num,*value,voltage);
	return 1;
}



int Feb_Control_SetTrimbits(unsigned int module_num, unsigned int *trimbits){
	printf("Setting Trimbits\n");

	//for (int iy=10000;iy<20020;++iy)//263681
	//for (int iy=263670;iy<263680;++iy)//263681
	//	printf("%d:%c\t\t",iy,trimbits[iy]);

	unsigned int trimbits_to_load_l[1024];
	unsigned int trimbits_to_load_r[1024];

	unsigned int module_index=0;
	if(!Feb_Control_GetModuleIndex(module_num,&module_index)){
		cprintf(RED,"Warning could not set trimbits, bad module number.\n");
		return 0;
	}

	if(!Feb_Control_Reset()) cprintf(RED,"Warning could not reset DAQ.\n");
	int l_r;	//printf("222\n");
	for(l_r=0;l_r<2;l_r++){ // l_r loop
		//printf("\nl_r:%d\t\t",l_r);
		unsigned int disable_chip_mask = l_r ? DAQ_CS_BAR_LEFT : DAQ_CS_BAR_RIGHT;
		if(Feb_Control_activated){
			if(!(Feb_Interface_WriteRegister(0xfff,DAQ_REG_STATIC_BITS,disable_chip_mask|DAQ_STATIC_BIT_PROGRAM|DAQ_STATIC_BIT_M8,0,0)
					&&Feb_Control_SetCommandRegister(DAQ_SET_STATIC_BIT)
					&&Feb_Control_StartDAQOnlyNWaitForFinish(5000))){
				printf("Could not select chips\n");
				return 0;
			}
		}

		int row_set;
		for(row_set=0;row_set<16;row_set++){ //16 rows at a time
			//printf("row_set:%d\t\t",row_set);
			if(row_set==0){
				if(!Feb_Control_SetCommandRegister(DAQ_RESET_COMPLETELY|DAQ_SEND_A_TOKEN_IN|DAQ_LOAD_16ROWS_OF_TRIMBITS)){
					cprintf(RED,"Warning: Could not Feb_Control_SetCommandRegister for loading trim bits.\n");
					return 0;
				}
			}else{
				if(!Feb_Control_SetCommandRegister(DAQ_LOAD_16ROWS_OF_TRIMBITS)){
					cprintf(RED,"Warning: Could not Feb_Control_SetCommandRegister for loading trim bits.\n");
					return 0;
				}
			}

			int row;
			for(row=0;row<16;row++){ //row loop
				//printf("row:%d\t\t",row);
				int offset   = 2*32*row;
				int sc;
				for(sc=0;sc<32;sc++){  //supercolumn loop sc
					//printf("sc:%d\t\t",sc);
					int super_column_start_position_l = 1030*row +       l_r *258 + sc*8;
					int super_column_start_position_r = 1030*row + 516 + l_r *258 + sc*8;

					/*
					  int super_column_start_position_l = 1024*row +       l_r *256 + sc*8; //256 per row, 8 per super column
					  int super_column_start_position_r = 1024*row + 512 + l_r *256 + sc*8; //256 per row, 8 per super column
					 */
					int chip_sc = 31 - sc;
					trimbits_to_load_l[offset+chip_sc] = 0;
					trimbits_to_load_r[offset+chip_sc] = 0;
					trimbits_to_load_l[offset+chip_sc+32] = 0;
					trimbits_to_load_r[offset+chip_sc+32] = 0;
					int i;
					for(i=0;i<8;i++){ // column loop i
						//printf("i:%d\t\t",i);

						if(Module_TopAddressIsValid(&modules[1])){
							trimbits_to_load_l[offset+chip_sc]    |= ( 0x7  & trimbits[row_set*16480+super_column_start_position_l+i])<<((7-i)*4);//low
							trimbits_to_load_l[offset+chip_sc+32] |= ((0x38 & trimbits[row_set*16480+super_column_start_position_l+i])>>3)<<((7-i)*4);//upper
							trimbits_to_load_r[offset+chip_sc]    |= ( 0x7  & trimbits[row_set*16480+super_column_start_position_r+i])<<((7-i)*4);//low
							trimbits_to_load_r[offset+chip_sc+32] |= ((0x38 & trimbits[row_set*16480+super_column_start_position_r+i])>>3)<<((7-i)*4);//upper
						}else{
							trimbits_to_load_l[offset+chip_sc]    |= ( 0x7  & trimbits[263679 - (row_set*16480+super_column_start_position_l+i)])<<((7-i)*4);//low
							trimbits_to_load_l[offset+chip_sc+32] |= ((0x38 & trimbits[263679 - (row_set*16480+super_column_start_position_l+i)])>>3)<<((7-i)*4);//upper
							trimbits_to_load_r[offset+chip_sc]    |= ( 0x7  & trimbits[263679 - (row_set*16480+super_column_start_position_r+i)])<<((7-i)*4);//low
							trimbits_to_load_r[offset+chip_sc+32] |= ((0x38 & trimbits[263679 - (row_set*16480+super_column_start_position_r+i)])>>3)<<((7-i)*4);//upper

						}
					} // end column loop i
				} //end supercolumn loop sc
			} //end row loop

			if(Module_TopAddressIsValid(&modules[1])){
				if(Feb_Control_activated){
					if(!Feb_Interface_WriteMemoryInLoops(Module_GetTopLeftAddress(&modules[Feb_Control_current_index]),0,0,1024,trimbits_to_load_l)||
							!Feb_Interface_WriteMemoryInLoops(Module_GetTopRightAddress(&modules[Feb_Control_current_index]),0,0,1024,trimbits_to_load_r)||
							//if(!Feb_Interface_WriteMemory(Module_GetTopLeftAddress(&modules[0]),0,0,1023,trimbits_to_load_r)||
							//	!Feb_Interface_WriteMemory(Module_GetTopRightAddress(&modules[0]),0,0,1023,trimbits_to_load_l)||
							!Feb_Control_StartDAQOnlyNWaitForFinish(5000)){
						printf(" some errror!\n");
						return 0;
					}
				}
			}else{
				if(Feb_Control_activated){
					if(!Feb_Interface_WriteMemoryInLoops(Module_GetBottomLeftAddress(&modules[Feb_Control_current_index]),0,0,1024,trimbits_to_load_l)||
							!Feb_Interface_WriteMemoryInLoops(Module_GetBottomRightAddress(&modules[Feb_Control_current_index]),0,0,1024,trimbits_to_load_r)||
							//if(!Feb_Interface_WriteMemory(Module_GetTopLeftAddress(&modules[0]),0,0,1023,trimbits_to_load_r)||
							//	!Feb_Interface_WriteMemory(Module_GetTopRightAddress(&modules[0]),0,0,1023,trimbits_to_load_l)||
							!Feb_Control_StartDAQOnlyNWaitForFinish(5000)){
						printf(" some errror!\n");
						return 0;
					}
				}
			}

		} //end row_set loop (groups of 16 rows)
	} // end l_r loop

	memcpy(Feb_Control_last_downloaded_trimbits,trimbits,Feb_Control_trimbit_size*sizeof(unsigned int));

	return Feb_Control_SetStaticBits(); //send the static bits
}


unsigned int* Feb_Control_GetTrimbits(){
	return Feb_Control_last_downloaded_trimbits;
}




unsigned int Feb_Control_AddressToAll(){printf("in Feb_Control_AddressToAll()\n");



if(moduleSize==0) return 0;


if(Module_BottomAddressIsValid(&modules[1])){
	//printf("************* bottom\n");
	//if(Feb_Control_am_i_master)
	return Module_GetBottomLeftAddress(&modules[1])|Module_GetBottomRightAddress(&modules[1]);
	// else return 0;
}
// printf("************* top\n");

return Module_GetTopLeftAddress(&modules[1])|Module_GetTopRightAddress(&modules[1]);
//return Module_GetTopLeftAddress(&modules[0])|Module_GetTopRightAddress(&modules[0]);


}

int Feb_Control_SetCommandRegister(unsigned int cmd){
	if(Feb_Control_activated)
		return Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_CHIP_CMDS,cmd,0,0);
	else
		return 1;
}


int Feb_Control_GetDAQStatusRegister(unsigned int dst_address, unsigned int* ret_status){
	//if deactivated, should  be handled earlier and should not get into this function
	if(Feb_Control_activated){
		if(!Feb_Interface_ReadRegister(dst_address,DAQ_REG_STATUS,ret_status)){
			cprintf(RED,"Error: reading status register.\n");
			return 0;
		}
	}

	*ret_status = (0x02FF0000 & *ret_status) >> 16;
	return 1;
}


int Feb_Control_StartDAQOnlyNWaitForFinish(int sleep_time_us){
	if(Feb_Control_activated){
		if(!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_CTRL,0,0,0)||!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_CTRL,DAQ_CTRL_START,0,0)){
			cprintf(RED,"Warning: could not start.\n");
			return 0;
		}
	}
	return Feb_Control_WaitForFinishedFlag(sleep_time_us);
}


int Feb_Control_AcquisitionInProgress(){
	unsigned int status_reg_r=0,status_reg_l=0;

	//deactivated should return end of acquisition
	if(!Feb_Control_activated)
		return 0;

	int ind = Feb_Control_current_index;
	if(Module_BottomAddressIsValid(&modules[ind])){

		if(!(Feb_Control_GetDAQStatusRegister(Module_GetBottomRightAddress(&modules[ind]),&status_reg_r)))
		{cprintf(RED,"Error: Trouble reading Status register. bottom right address\n");return 0;}
		if(!(Feb_Control_GetDAQStatusRegister(Module_GetBottomLeftAddress(&modules[ind]),&status_reg_l)))
		{cprintf(RED,"Error: Trouble reading Status register. bottom left address\n");return 0;}

	}else{
		if(!(Feb_Control_GetDAQStatusRegister(Module_GetTopRightAddress(&modules[ind]),&status_reg_r)))
		{cprintf(RED,"Error: Trouble reading Status register. top right address\n");return 0;}
		if(!(Feb_Control_GetDAQStatusRegister(Module_GetTopLeftAddress(&modules[ind]),&status_reg_l)))
		{cprintf(RED,"Error: Trouble reading Status register. top left address\n");return 0;}
	}

	//running
	if((status_reg_r|status_reg_l)&DAQ_STATUS_DAQ_RUNNING) {/*printf("**runningggg\n");*/
		return 1;
	}
	//idle
	return 0;
}


int Feb_Control_AcquisitionStartedBit(){
	unsigned int status_reg_r=0,status_reg_l=0;

	//deactivated should return acquisition started/ready
	if(!Feb_Control_activated)
		return 1;

	int ind = Feb_Control_current_index;
	if(Module_BottomAddressIsValid(&modules[ind])){

		if(!(Feb_Control_GetDAQStatusRegister(Module_GetBottomRightAddress(&modules[ind]),&status_reg_r)))
		{cprintf(RED,"Error: Trouble reading Status register. bottom right address\n");return -1;}
		if(!(Feb_Control_GetDAQStatusRegister(Module_GetBottomLeftAddress(&modules[ind]),&status_reg_l)))
		{cprintf(RED,"Error: Trouble reading Status register. bottom left address\n");return -1;}

	}else{
		if(!(Feb_Control_GetDAQStatusRegister(Module_GetTopRightAddress(&modules[ind]),&status_reg_r)))
		{cprintf(RED,"Error: Trouble reading Status register. top right address\n"); return -1;}
		if(!(Feb_Control_GetDAQStatusRegister(Module_GetTopLeftAddress(&modules[ind]),&status_reg_l)))
		{cprintf(RED,"Error: Trouble reading Status register. top left address\n");return -1;}
	}

	//doesnt mean it started, just the bit
	if((status_reg_r|status_reg_l)&DAQ_STATUS_DAQ_RUN_TOGGLE)
		return 1;

	return 0;
}



int Feb_Control_WaitForFinishedFlag(int sleep_time_us){
	int is_running = Feb_Control_AcquisitionInProgress();
	while(is_running){
		usleep(sleep_time_us);
		is_running = Feb_Control_AcquisitionInProgress();
	}
	if(is_running!=0){
		printf("\n\nWarning WaitForFinishedFlag comunication problem..\n\n");
		return 0; //communication problem
	}

	return 1;
}


int Feb_Control_WaitForStartedFlag(int sleep_time_us, int prev_flag){

	//deactivated dont wait (otherwise give a toggle value back)
	if(!Feb_Control_activated)
		return 1;

	int value = prev_flag;
	while(value == prev_flag){
		usleep(sleep_time_us);
		value = Feb_Control_AcquisitionStartedBit();
	}

	//did not start
	if(value == -1)
		return 0;

	return 1;
}


int Feb_Control_Reset(){
	printf("Reset daq\n");
	if(Feb_Control_activated){
		if(!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_CTRL,0,0,0) || !Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_CTRL,DAQ_CTRL_RESET,0,0) || !Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_CTRL,0,0,0)){
			cprintf(RED,"Warning: Could not reset daq, no response.\n");
			return 0;
		}
	}

	return Feb_Control_WaitForFinishedFlag(5000);
}




int Feb_Control_SetStaticBits(){
	if(Feb_Control_activated){
		//program=1,m4=2,m8=4,test=8,rotest=16,cs_bar_left=32,cs_bar_right=64
		if(!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_STATIC_BITS,Feb_Control_staticBits,0,0) || !Feb_Control_SetCommandRegister(DAQ_SET_STATIC_BIT) || !Feb_Control_StartDAQOnlyNWaitForFinish(5000)){
			cprintf(RED,"Warning: Could not set static bits\n");
			return 0;
		}
	}

	return 1;
}
int Feb_Control_SetStaticBits1(unsigned int the_static_bits){
	Feb_Control_staticBits = the_static_bits;
	return Feb_Control_SetStaticBits();
}

int Feb_Control_SetInTestModeVariable(int on){
	if(on) Feb_Control_staticBits |=   DAQ_STATIC_BIT_CHIP_TEST;  //setting test bit to high
	else   Feb_Control_staticBits &= (~DAQ_STATIC_BIT_CHIP_TEST); //setting test bit to low
	return 1;
}

int Feb_Control_GetTestModeVariable(){
	return Feb_Control_staticBits&DAQ_STATIC_BIT_CHIP_TEST;
}

int Feb_Control_SetDynamicRange(unsigned int four_eight_sixteen_or_thirtytwo){
	static unsigned int everything_but_bit_mode = DAQ_STATIC_BIT_PROGRAM|DAQ_STATIC_BIT_CHIP_TEST|DAQ_STATIC_BIT_ROTEST;
	if(four_eight_sixteen_or_thirtytwo==4){
		Feb_Control_staticBits    = DAQ_STATIC_BIT_M4 | (Feb_Control_staticBits&everything_but_bit_mode); //leave test bits in currernt state
		Feb_Control_subFrameMode &= ~DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING;
	}else if(four_eight_sixteen_or_thirtytwo==8){
		Feb_Control_staticBits    = DAQ_STATIC_BIT_M8 | (Feb_Control_staticBits&everything_but_bit_mode);
		Feb_Control_subFrameMode &= ~DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING;
	}else if(four_eight_sixteen_or_thirtytwo==16){
		Feb_Control_staticBits    = DAQ_STATIC_BIT_M12 | (Feb_Control_staticBits&everything_but_bit_mode);
		Feb_Control_subFrameMode &= ~DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING;
	}else if(four_eight_sixteen_or_thirtytwo==32){
		Feb_Control_staticBits    = DAQ_STATIC_BIT_M12 | (Feb_Control_staticBits&everything_but_bit_mode);
		Feb_Control_subFrameMode |= DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING;
	}else{
		cprintf(RED,"Warning: dynamic range (%d) not valid, not setting bit mode.\n",four_eight_sixteen_or_thirtytwo);
		printf("Set dynamic range int must equal 4,8 16, or 32.\n");
		return 0;
	}

	printf("Dynamic range set to: %d\n",four_eight_sixteen_or_thirtytwo);
	return 1;
}

unsigned int Feb_Control_GetDynamicRange(){
	if(Feb_Control_subFrameMode&DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING) return 32;
	else if(DAQ_STATIC_BIT_M4&Feb_Control_staticBits)                     return 4;
	else if(DAQ_STATIC_BIT_M8&Feb_Control_staticBits)                     return 8;

	return 16;
}

int Feb_Control_SetReadoutSpeed(unsigned int readout_speed){ //0->full,1->half,2->quarter or 3->super_slow
	Feb_Control_acquireNReadoutMode &= (~DAQ_CHIP_CONTROLLER_SUPER_SLOW_SPEED);
	if(readout_speed==1){
		Feb_Control_acquireNReadoutMode |= DAQ_CHIP_CONTROLLER_HALF_SPEED;
		printf("Everything at half speed, ie. reading with 50 MHz main clk (half speed) ....\n");
	}else if(readout_speed==2){
		Feb_Control_acquireNReadoutMode |= DAQ_CHIP_CONTROLLER_QUARTER_SPEED;
		printf("Everything at quarter speed, ie. reading with 25 MHz main clk (quarter speed) ....\n");
	}else if(readout_speed==3){
		Feb_Control_acquireNReadoutMode |= DAQ_CHIP_CONTROLLER_SUPER_SLOW_SPEED;
		printf("Everything at super slow speed, ie. reading with ~0.200 MHz main clk (super slow speed) ....\n");
	}else{
		if(readout_speed){
			cprintf(RED,"Warning readout speed %d unknown, defaulting to full speed.\n",readout_speed);
			printf("Everything at full speed, ie. reading with 100 MHz main clk (full speed) ....\n");
			return 0;
		}
		printf("Everything at full speed, ie. reading with 100 MHz main clk (full speed) ....\n");
	}

	return 1;
}

int Feb_Control_SetReadoutMode(unsigned int readout_mode){ //0->parallel,1->non-parallel,2-> safe_mode
	Feb_Control_acquireNReadoutMode &= (~DAQ_NEXPOSURERS_PARALLEL_MODE);
	if(readout_mode==1){
		Feb_Control_acquireNReadoutMode |= DAQ_NEXPOSURERS_NORMAL_NONPARALLEL_MODE;
		printf("Readout mode set to normal non-parallel readout mode ... \n");;
	}else if(readout_mode==2){
		Feb_Control_acquireNReadoutMode |= DAQ_NEXPOSURERS_SAFEST_MODE_ROW_CLK_BEFORE_MODE;
		printf("Readout mode set to safest mode, row clk before main clk readout sequence .... \n");;
	}else{
		Feb_Control_acquireNReadoutMode |= DAQ_NEXPOSURERS_PARALLEL_MODE;
		if(readout_mode){
			cprintf(RED,"Warning readout mode %d) unknown, defaulting to full speed.\n",readout_mode);
			printf("Readout mode set to parrallel acquire/read mode ....     \n");;
			return 0;
		}
		printf("Readout mode set to parrallel acquire/read mode ....     \n");;
	}

	return 1;
}

int Feb_Control_SetTriggerMode(unsigned int trigger_mode,int polarity){
	//"00"-> internal exposure time and period,
	//"01"-> external acquistion start and internal exposure time and period,
	//"10"-> external start trigger and internal exposure time,
	//"11"-> external triggered start and stop of exposures
	Feb_Control_triggerMode  = (~DAQ_NEXPOSURERS_EXTERNAL_IMAGE_START_AND_STOP);

	if(trigger_mode == 1){
		Feb_Control_triggerMode = DAQ_NEXPOSURERS_EXTERNAL_ACQUISITION_START;
		printf("Trigger mode: external start of acquisition sequence, internal exposure length and period.\n");;
	}else if(trigger_mode == 2){
		Feb_Control_triggerMode = DAQ_NEXPOSURERS_EXTERNAL_IMAGE_START;
		printf("Trigger mode: external image start, internal exposure time.\n");;
	}else if(trigger_mode == 3){
		Feb_Control_triggerMode = DAQ_NEXPOSURERS_EXTERNAL_IMAGE_START_AND_STOP;
		printf("Trigger mode: externally controlled, external image window (start and stop).\n");;
	}else{
		Feb_Control_triggerMode = DAQ_NEXPOSURERS_INTERNAL_ACQUISITION;
		if(trigger_mode) cprintf(RED,"Warning trigger %d) unknown, defaulting to internal triggering.\n",trigger_mode);;

		printf("Trigger mode: acquisition internally controlled exposure length and period.\n");;
		return trigger_mode==0;
	}

	if(polarity){
		Feb_Control_triggerMode |=  DAQ_NEXPOSURERS_EXTERNAL_TRIGGER_POLARITY;
		printf("External trigger polarity set to positive.\n");;
	}else{
		Feb_Control_triggerMode &= (~DAQ_NEXPOSURERS_EXTERNAL_TRIGGER_POLARITY);
		printf("External trigger polarity set to negitive.\n");;
	}

	return 1;
}


int Feb_Control_SetExternalEnableMode(int use_external_enable, int polarity){
	if(use_external_enable){
		Feb_Control_externalEnableMode  |= DAQ_NEXPOSURERS_EXTERNAL_ENABLING;
		printf("External enabling enabled, ");
		if(polarity){
			Feb_Control_externalEnableMode |= DAQ_NEXPOSURERS_EXTERNAL_ENABLING_POLARITY;
			printf(", polarity set to positive.\n");;
		}else{
			Feb_Control_externalEnableMode &= (~DAQ_NEXPOSURERS_EXTERNAL_ENABLING_POLARITY);
			printf(", polarity set to negative.\n");;
		}
	}else{
		Feb_Control_externalEnableMode = 0; /* changed by Dhanya according to old code &= (~DAQ_NEXPOSURERS_EXTERNAL_ENABLING);*/
		printf("External enabling disabled.\n");;
	}

	return 1;
}

int Feb_Control_SetNExposures(unsigned int n_images){
	if(!n_images){
		cprintf(RED,"Warning nimages must be greater than zero.%d\n",n_images);
		return 0;
	}

	Feb_Control_nimages = n_images;
	printf("Number of images set to: %d\n",Feb_Control_nimages);
	return 1;
}
unsigned int Feb_Control_GetNExposures(){return Feb_Control_nimages;}

int Feb_Control_SetExposureTime(double the_exposure_time_in_sec){
	Feb_Control_exposure_time_in_sec = the_exposure_time_in_sec;
	printf("Exposure time set to: %fs\n",Feb_Control_exposure_time_in_sec);
	return 1;
}
double Feb_Control_GetExposureTime(){return Feb_Control_exposure_time_in_sec;}
int64_t Feb_Control_GetExposureTime_in_nsec(){return (int64_t)(Feb_Control_exposure_time_in_sec*(1E9));}

int Feb_Control_SetSubFrameExposureTime(int64_t the_subframe_exposure_time_in_10nsec){
	Feb_Control_subframe_exposure_time_in_10nsec = the_subframe_exposure_time_in_10nsec;
	printf("Sub Frame Exposure time set to: %lld\n",(long long int)Feb_Control_subframe_exposure_time_in_10nsec);
	return 1;
}
int64_t Feb_Control_GetSubFrameExposureTime(){return Feb_Control_subframe_exposure_time_in_10nsec*10;}

int Feb_Control_SetExposurePeriod(double the_exposure_period_in_sec){
	Feb_Control_exposure_period_in_sec = the_exposure_period_in_sec;
	printf("Exposure period set to: %f\n",Feb_Control_exposure_period_in_sec);
	return 1;
}
double Feb_Control_GetExposurePeriod(){return Feb_Control_exposure_period_in_sec;}

unsigned int Feb_Control_ConvertTimeToRegister(float time_in_sec){
	float n_clk_cycles = round(time_in_sec/10e-9); //200 MHz ctb clk or 100 MHz feb clk

	unsigned int decoded_time;
	if(n_clk_cycles>(pow(2,29)-1)*pow(10,7)){
		float max_time = 10e-9*(pow(2,28)-1)*pow(10,7);
		cprintf(RED,"Warning: time exceeds (%f) maximum exposure time of %f sec.\n",time_in_sec,max_time);
		printf("\t Setting to maximum %f us.\n",max_time);
		decoded_time = 0xffffffff;
	}else{
		int power_of_ten = 0;
		while(n_clk_cycles>pow(2,29)-1){ power_of_ten++; n_clk_cycles = round(n_clk_cycles/10.0);}
		decoded_time = (int)(n_clk_cycles)<<3 | (int)(power_of_ten);
	}

	return decoded_time;
}

int Feb_Control_ResetChipCompletely(){
	if(!Feb_Control_SetCommandRegister(DAQ_RESET_COMPLETELY) || !Feb_Control_StartDAQOnlyNWaitForFinish(5000)){
		cprintf(RED,"Warning: could not ResetChipCompletely() with 0x%x.\n",DAQ_RESET_COMPLETELY);
		return 0;
	}
	printf("Chip reset completely\n");
	return 1;
}



int Feb_Control_ResetChipPartially(){
	if(!Feb_Control_SetCommandRegister(DAQ_RESET_PERIPHERY) || !Feb_Control_StartDAQOnlyNWaitForFinish(5000)){
		cprintf(RED,"Warning: could not ResetChipPartially with periphery\n");
		return 0;
	}
	printf("Chip reset periphery 0x%x\n",DAQ_RESET_PERIPHERY);

	if(!Feb_Control_SetCommandRegister(DAQ_RESET_COLUMN_SELECT) || !Feb_Control_StartDAQOnlyNWaitForFinish(5000)){
		cprintf(RED,"Warning: could not ResetChipPartially with column select\n");
		return 0;
	}
	printf("Chip reset column select 0x%x\n",DAQ_RESET_COLUMN_SELECT);

	return 1;
}


void Feb_Control_PrintAcquisitionSetup(){

	time_t rawtime;
	time(&rawtime);
	struct tm *timeinfo = localtime(&rawtime);

	printf("\nStarting an exposure: %s",asctime(timeinfo));
	printf("\t Dynamic range nbits: %d\n",Feb_Control_GetDynamicRange());
	printf("\t Trigger mode: 0x%x\n",Feb_Control_triggerMode);
	printf("\t Number of exposures: %d\n",Feb_Control_GetNExposures());
	printf("\t Exsposure time (if used): %f seconds.\n",Feb_Control_exposure_time_in_sec);
	printf("\t Exsposure period (if used): %f seconds.\n\n\n",Feb_Control_exposure_period_in_sec);
}

int Feb_Control_SendBitModeToBebServer(){

	unsigned int just_bit_mode = (DAQ_STATIC_BIT_M4|DAQ_STATIC_BIT_M8) & Feb_Control_staticBits;
	unsigned int bit_mode = 16; //default
	if(just_bit_mode == DAQ_STATIC_BIT_M4)  bit_mode = 4;
	else if(just_bit_mode == DAQ_STATIC_BIT_M8) bit_mode = 8;
	else if(Feb_Control_subFrameMode&DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING) bit_mode = 32;


	if(!Beb_SetUpTransferParameters(bit_mode)){
		cprintf(RED,"Error: sending bit mode ...\n");
		return 0;
	}

	return 1;
}


int Feb_Control_PrepareForAcquisition(){//return 1;
	static unsigned int reg_nums[20];
	static unsigned int reg_vals[20];

	Feb_Control_PrintAcquisitionSetup();

	//  if(!Reset()||!ResetDataStream()){
	if(!Feb_Control_Reset()){
		printf("Trouble reseting daq or data stream...\n");;
		return 0;
	}

	if(!Feb_Control_SetStaticBits1(Feb_Control_staticBits&(DAQ_STATIC_BIT_M4|DAQ_STATIC_BIT_M8))){
		printf("Trouble setting static bits ...\n");;
		return 0;
	}

	if(!Feb_Control_SendBitModeToBebServer()){
		printf("Trouble sending static bits to server ...\n");;
		return 0;
	}

	int ret=0;
	if(Feb_Control_counter_bit)
		ret = Feb_Control_ResetChipCompletely();
	else
		ret = Feb_Control_ResetChipPartially();
	if(!ret){
		printf("Trouble resetting chips ...\n");;
		return 0;
	}


	reg_nums[0]=DAQ_REG_CTRL;
	reg_vals[0]=0;
	reg_nums[1]=DAQ_REG_NEXPOSURES;
	reg_vals[1]=Feb_Control_nimages;
	reg_nums[2]=DAQ_REG_EXPOSURE_TIMER;
	reg_vals[2]=Feb_Control_ConvertTimeToRegister(Feb_Control_exposure_time_in_sec);
	reg_nums[3]=DAQ_REG_EXPOSURE_REPEAT_TIMER;
	reg_vals[3]=Feb_Control_ConvertTimeToRegister(Feb_Control_exposure_period_in_sec);
	reg_nums[4]=DAQ_REG_CHIP_CMDS;
	reg_vals[4]=(Feb_Control_acquireNReadoutMode|Feb_Control_triggerMode|Feb_Control_externalEnableMode|Feb_Control_subFrameMode);
	reg_nums[5]=DAQ_REG_SUBFRAME_EXPOSURES;
	reg_vals[5]= Feb_Control_subframe_exposure_time_in_10nsec; //(1 means 10ns, 100 means 1000ns)
	// if(!Feb_Interface_WriteRegisters((Module_GetTopLeftAddress(&modules[1])|Module_GetTopRightAddress(&modules[1])),20,reg_nums,reg_vals,0,0)){
	if(Feb_Control_activated){
		if(!Feb_Interface_WriteRegisters(Feb_Control_AddressToAll(),6,reg_nums,reg_vals,0,0)){
			printf("Trouble starting acquisition....\n");;
			return 0;
		}
	}

	return 1;
}



int Feb_Control_StartAcquisition(){
	printf("****** starting acquisition********* \n");

	static unsigned int reg_nums[20];
	static unsigned int reg_vals[20];


	int i;
	for(i=0;i<14;i++){
		reg_nums[i]=DAQ_REG_CTRL;
		reg_vals[i]=0;
	}
	reg_nums[14]=DAQ_REG_CTRL;
	reg_vals[14]=ACQ_CTRL_START;

	if(Feb_Control_activated){
		if(!Feb_Interface_WriteRegisters(Feb_Control_AddressToAll(),15,reg_nums,reg_vals,0,0)){
			cprintf(RED,"Trouble starting acquisition....\n");;
			return 0;
		}
	}


	return 1;
}

int Feb_Control_StopAcquisition(){
	return Feb_Control_Reset();
}



int Feb_Control_SaveAllTrimbitsTo(int value){
	unsigned int chanregs[Feb_Control_trimbit_size];
	int i;
	for(i=0;i<Feb_Control_trimbit_size;i++)
		chanregs[i] = value;
	return Feb_Control_SetTrimbits(0,chanregs);
}


void Feb_Control_Set_Counter_Bit(int value){
	Feb_Control_counter_bit = value;
}

int Feb_Control_Get_Counter_Bit(){
	return Feb_Control_counter_bit;
}

int Feb_Control_Pulse_Pixel(int npulses, int x, int y){
	//this function is not designed for speed

	int pulse_multiple = 0;  //has to be 0 or 1
	int i;

	if(x<0){
		x=-x;
		pulse_multiple=1;
		printf("Pulsing pixel %d in all super columns below number %d.\n",x%8,x/8);
	}
	if(x<0||x>255||y<0||y>255){
		cprintf(RED,"Warning: Pixel out of range.\n");
		return 0;
	}

	//  y = 255 - y;
	int nrowclocks = 0;
	nrowclocks += (Feb_Control_staticBits&DAQ_STATIC_BIT_M4) ? 0 : 2*y;
	nrowclocks += (Feb_Control_staticBits&DAQ_STATIC_BIT_M8) ? 0 : y;

	Feb_Control_SetInTestModeVariable(1); //on
	Feb_Control_SetStaticBits();
	Feb_Control_SetCommandRegister(DAQ_RESET_PERIPHERY|DAQ_RESET_COLUMN_SELECT);
	Feb_Control_StartDAQOnlyNWaitForFinish(5000);

	unsigned int serial_in = 8<<(4*(7-x%8));
	if(!Feb_Control_Shift32InSerialIn(serial_in)){
		cprintf(RED,"Warning ChipController::PulsePixel: could shift in the initail 32.\n");
		return 0;
	}

	if(!pulse_multiple)
		serial_in=0;
	for(i=0;i<x/8;i++)
		Feb_Control_Shift32InSerialIn(serial_in);

	Feb_Control_SendTokenIn();
	Feb_Control_ClockRowClock(nrowclocks);
	Feb_Control_PulsePixelNMove(npulses,0,0);


	return 1;
}


int Feb_Control_PulsePixelNMove(int npulses, int inc_x_pos, int inc_y_pos){
	unsigned int c = DAQ_SEND_N_TEST_PULSES;
	c |= (inc_x_pos) ? DAQ_CLK_MAIN_CLK_TO_SELECT_NEXT_PIXEL : 0;
	c |= (inc_y_pos) ? DAQ_CLK_ROW_CLK_TO_SELECT_NEXT_ROW    : 0;

	if(Feb_Control_activated){
		if(!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_SEND_N_TESTPULSES,npulses,0,0) ||
				!Feb_Control_SetCommandRegister(c) ||
				!Feb_Control_StartDAQOnlyNWaitForFinish(5000)){
			cprintf(RED,"Warning: could not PulsePixelNMove(...).\n");
			return 0;
		}
	}

	return 1;
}

/**new*/
int Feb_Control_Shift32InSerialIn(unsigned int value_to_shift_in){
	if(Feb_Control_activated){
		if(!Feb_Control_SetCommandRegister(DAQ_SERIALIN_SHIFT_IN_32) ||
				!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_SHIFT_IN_32,value_to_shift_in,0,0) ||
				!Feb_Control_StartDAQOnlyNWaitForFinish(5000)){
			cprintf(RED,"Warning: could not shift in 32.\n");
			return 0;
		}
	}
	return 1;
}

int Feb_Control_SendTokenIn(){
	if(!Feb_Control_SetCommandRegister(DAQ_SEND_A_TOKEN_IN) ||
			!Feb_Control_StartDAQOnlyNWaitForFinish(5000)){
		cprintf(RED,"Warning: could not SendTokenIn().\n");
		return 0;
	}
	return 1;
}

int Feb_Control_ClockRowClock(unsigned int ntimes){
	if(ntimes>1023){
		cprintf(RED,"Warning: Clock row clock ntimes (%d) exceeds the maximum value of 1023.\n\t Setting ntimes to 1023.\n",ntimes);
		ntimes=1023;
	}

	if(Feb_Control_activated){
		if(!Feb_Control_SetCommandRegister(DAQ_CLK_ROW_CLK_NTIMES) ||
				!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_CLK_ROW_CLK_NTIMES,ntimes,0,0) ||
				!Feb_Control_StartDAQOnlyNWaitForFinish(5000)){
			cprintf(RED,"Warning: could not clock row clock.\n");
			return 0;
		}
	}

	return 1;
}


int Feb_Control_PulseChip(int npulses){
	int i;
	int on = 1;

	if(npulses == -1){
		on = 0;
		printf("\nResetting to normal mode\n");
	}else{
		printf("\n\nPulsing Chip.\n");//really just toggles the enable
		printf("Vcmp should be set to 2.0 and Vtrim should be 2.\n");
	}


	Feb_Control_SetInTestModeVariable(on);
	Feb_Control_SetStaticBits(); //toggle the enable 2x times
	Feb_Control_ResetChipCompletely();

	for(i=0;i<npulses;i++){
		if(!Feb_Control_SetCommandRegister(DAQ_CHIP_CONTROLLER_SUPER_SLOW_SPEED|DAQ_RESET_PERIPHERY|DAQ_RESET_COLUMN_SELECT))
			cprintf(RED,"some set command register error\n");
		if(!Feb_Control_StartDAQOnlyNWaitForFinish(5000))
			cprintf(RED,"some wait error\n");
	}
	Feb_Control_SetExternalEnableMode(on,1);
	Feb_Control_counter_bit = (on?0:1);
	printf("Feb_Control_counter_bit:%d\n",Feb_Control_counter_bit);

	if(on)
		printf("Pulse chip success\n\n");
	else
		printf("Reset to normal mode success\n\n");
	return 1;
}



int64_t Feb_Control_Get_RateTable_Tau_in_nsec(){ return Feb_Control_RateTable_Tau_in_nsec;}
int64_t Feb_Control_Get_RateTable_Period_in_nsec(){ return Feb_Control_RateTable_Period_in_nsec;}

int Feb_Control_SetRateCorrectionTau(int64_t tau_in_Nsec){

	//period = exptime if 16bit, period = subexptime if 32 bit
	int dr = Feb_Control_GetDynamicRange();
	double period_in_sec = (double)(Feb_Control_GetSubFrameExposureTime())/(double)1e9;
	if(dr == 16)
		period_in_sec = Feb_Control_GetExposureTime();


	double tau_in_sec = (double)tau_in_Nsec/(double)1e9;
	printf(" tau %lf %lf ", (double)tau_in_Nsec, (double) tau_in_sec); 

	unsigned int np = 16384; //max slope 16 * 1024
	double b0[1024];
	double m[1024];

	if(tau_in_sec<0||period_in_sec<0){
		if(dr == 32)
			printf("Error tau %lf and sub_exposure_time %lf must be greater than 0.\n", tau_in_sec, period_in_sec);
		else
			printf("Error tau %lf and exposure_time %lf must be greater than 0.\n", tau_in_sec, period_in_sec);
		return 0;
	}

	cprintf(BLUE, "Changing Rate Correction Table tau:%0.8f sec, period:%f sec",tau_in_sec,period_in_sec);

	printf("\tCalculating table for tau of %lld ns.\n", tau_in_Nsec);
	int i;
	for(i=0;i<np;i++){
	  Feb_Control_rate_meas[i]  = i*exp(-i/period_in_sec*tau_in_sec);
	  if(Feb_Control_rate_meas[i] > ratemax) ratemax= Feb_Control_rate_meas[i];
	}

	/*
	  b  :  index/address of block ram/rate correction table
	  b0 :  base in vhdl
	  m  :  slope in vhdl
	  
	  Firmware:
	  data_in(11..2) -> memory address  --> memory
	  data_in( 1..0) -> lsb
	  
	  mem_data_out(13.. 0) -> base
	  mem_data_out(17..14) -> slope
	  
	  delta = slope*lsb
	  corr  = base+delta
	*/
	
	int next_i=0;
	double beforemax;
	b0[0] = 0;
	m[0]  = 1;
	
	Feb_Control_rate_correction_table[0]  = (((int)(m[0]+0.5)&0xf)<<14) | ((int)(b0[0]+0.5)&0x3fff);

	int b=0;
	for(b=1;b<1024;b++){
		if(m[b-1]<15){
			double s=0,sx=0,sy=0,sxx=0,sxy=0;
			for(;;next_i++){
			  if(next_i>=np){
			    for(; b<1024; b++){
			      if(beforemax>ratemax) b0[b] = beforemax;
			      else b0[b] = ratemax;
			      m[b]  = 15; 
			      Feb_Control_rate_correction_table[b]  = (((int)(m[b]+0.5)&0xf)<<14) | ((int)(b0[b]+0.5)&0x3fff);
			    }
			    b=1024;
			    break;
			  }
			  
			  double x    = Feb_Control_rate_meas[next_i] - b*4;
			  double y    = next_i;
			  /*printf("Start Loop  x: %f,\t y: %f,\t  s: %f,\t  sx: %f,\t  sy: %f,\t  sxx: %f,\t  sxy: %f,\t  "
			    "next_i: %d,\t  b: %d,\t  Feb_Control_rate_meas[next_i]: %f\n",
			    x, y, s, sx, sy, sxx, sxy, next_i, b, Feb_Control_rate_meas[next_i]);*/
			  
			  if(x < -0.5) continue;
			  if(x >  3.5) break;
			  s   += 1;
			  sx  += x;
			  sy  += y;
			  sxx += x*x;
			  sxy += x*y;
			  /*printf("End   Loop  x: %f,\t y: %f,\t  s: %f,\t  sx: %f,\t  sy: %f,\t  sxx: %f,\t  sxy: %f,\t  "
			    "next_i: %d,\t  b: %d,\t  Feb_Control_rate_meas[next_i]: %f\n",
						x, y, s, sx, sy, sxx, sxy, next_i, b, Feb_Control_rate_meas[next_i]);*/
			}
			double delta = s*sxx - sx*sx;
			b0[b] = (sxx*sy - sx*sxy)/delta;
			m[b]  = (s*sxy  - sx*sy) /delta;
			beforemax= b0[b];

			if(m[b]<0||m[b]>15){
			  m[b]=15;
			  if(beforemax>ratemax) b0[b] = beforemax;
			  else b0[b] = ratemax;
			}
			/*printf("After Loop  s: %f,\t  sx: %f,\t  sy: %f,\t  sxx: %f,\t  sxy: %f,\t  "
			  "next_i: %d,\t  b: %d,\t  Feb_Control_rate_meas[next_i]: %f\n",
			  s, sx, sy, sxx, sxy, next_i, b, Feb_Control_rate_meas[next_i]);*/
			//	cout<<s<<"   "<<sx<<"   "<<sy<<"   "<<sxx<<"   "<<"   "<<sxy<<"   "<<delta<<"   "<<m[b]<<"    "<<b0[b]<<endl;
		}else{
		  if(beforemax>ratemax) b0[b] = beforemax;
		  else b0[b] = ratemax;
		  m[b]  = 15;
		}
		Feb_Control_rate_correction_table[b]  = (((int)(m[b]+0.5)&0xf)<<14) | ((int)(b0[b]+0.5)&0x3fff);
		/*printf("After Loop  4*b: %d\tbase:%d\tslope:%d\n",4*b, (int)(b0[b]+0.5), (int)(m[b]+0.5) );*/
	}

	if(Feb_Control_SetRateCorrectionTable(Feb_Control_rate_correction_table)){
		Feb_Control_RateTable_Tau_in_nsec = tau_in_Nsec;
		Feb_Control_RateTable_Period_in_nsec = period_in_sec;
		return 1;
	}else{
		Feb_Control_RateTable_Tau_in_nsec = -1;
		Feb_Control_RateTable_Period_in_nsec = -1;
		return 0;
	}


}




int Feb_Control_SetRateCorrectionTable(unsigned int *table){
  if(!table){
	  printf("Error: could not set rate correction table, point is zero.\n");
	  Feb_Control_SetRateCorrectionVariable(0);
    return 0;
  }


  printf("Setting rate correction table. %d %d %d %d ....\n",
		  table[0],table[1],table[2],table[3]);

  //was added otherwise after an acquire, startdaqonlywatiforfinish waits forever
  if(!Feb_Control_SetCommandRegister(DAQ_RESET_COMPLETELY)){
	  cprintf(RED,"Warning: Could not Feb_Control_SetCommandRegister for loading trim bits.\n");
	  return 0;
  }
  printf("daq reset completely\n");

  if(Module_TopAddressIsValid(&modules[1])){
	  if(Feb_Control_activated){
		  if(!Feb_Interface_WriteMemoryInLoops(Module_GetTopLeftAddress(&modules[Feb_Control_current_index]),1,0,1024,Feb_Control_rate_correction_table)||
				  !Feb_Interface_WriteMemoryInLoops(Module_GetTopRightAddress(&modules[Feb_Control_current_index]),1,0,1024,Feb_Control_rate_correction_table)||
				  !Feb_Control_StartDAQOnlyNWaitForFinish(5000)){
			  cprintf(BG_RED,"Error in Top Writing to Memory ::Feb_Control_SetRateCorrectionTable\n");
			  return 0;
		  }
	  }
  }else{
	  if(Feb_Control_activated){
		  if(!Feb_Interface_WriteMemoryInLoops(Module_GetBottomLeftAddress(&modules[Feb_Control_current_index]),1,0,1024,Feb_Control_rate_correction_table)||
				  !Feb_Interface_WriteMemoryInLoops(Module_GetBottomRightAddress(&modules[Feb_Control_current_index]),1,0,1024,Feb_Control_rate_correction_table)||
				  !Feb_Control_StartDAQOnlyNWaitForFinish(5000)){
			  cprintf(BG_RED,"Error in Bottom Writing to Memory ::Feb_Control_SetRateCorrectionTable\n");
			  return 0;
		  }
	  }
  }
   return 1;
}


int Feb_Control_GetRateCorrectionVariable(){ return (Feb_Control_subFrameMode&DAQ_NEXPOSURERS_ACTIVATE_RATE_CORRECTION);}


void Feb_Control_SetRateCorrectionVariable(int activate_rate_correction){
  if(activate_rate_correction){
	  Feb_Control_subFrameMode |= DAQ_NEXPOSURERS_ACTIVATE_RATE_CORRECTION;
	  printf("Rate correction activated.\n");
  }else{
	  Feb_Control_subFrameMode &= ~DAQ_NEXPOSURERS_ACTIVATE_RATE_CORRECTION;
	  printf("Rate correction deactivated.\n");
  }
}


int Feb_Control_PrintCorrectedValues(){
	int i;
	int delta, slope, base, lsb, corr;
	for (i=0; i < 4096; i++){
		lsb   = i&3;
		base  = Feb_Control_rate_correction_table[i>>2] & 0x3fff;
		slope = ((Feb_Control_rate_correction_table[i>>2] & 0x3c000) >> 14);
		
		delta = slope*lsb;
		corr  = delta+base;		
		if(slope==15) corr= 3*slope+base;

		printf("Readout Input: %d,\tBase:%d,\tSlope:%d,\tLSB:%d,\tDelta:%d\tResult:%d\tReal:%lf\n",
		       i, base, slope, lsb, delta, corr, Feb_Control_rate_meas[i]);
	}
	return 1;
}


int Feb_Control_GetLeftFPGATemp(){
	unsigned int temperature=0;
	if(Module_TopAddressIsValid(&modules[1]))
		Feb_Interface_ReadRegister(Module_GetTopLeftAddress (&modules[1]),FEB_REG_STATUS, &temperature);
	else
		Feb_Interface_ReadRegister(Module_GetBottomLeftAddress (&modules[1]),FEB_REG_STATUS, &temperature);
	temperature = temperature >> 16;
	//division done in client to send int over network
	return (int)temperature;
}

int Feb_Control_GetRightFPGATemp(){
	unsigned int temperature=0;
	if(Module_TopAddressIsValid(&modules[1]))
		Feb_Interface_ReadRegister(Module_GetTopRightAddress (&modules[1]),FEB_REG_STATUS, &temperature);
	else
		Feb_Interface_ReadRegister(Module_GetBottomRightAddress (&modules[1]),FEB_REG_STATUS, &temperature);
	temperature = temperature >> 16;
	//division done in client to send int over network
	return (int)temperature;
}



