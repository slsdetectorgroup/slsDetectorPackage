 
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

//#include <fstream>
//#include <iomanip>
//#include <sstream>
//#include <time.h>



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

unsigned int Feb_Control_photon_energy_eV;

unsigned int Feb_Control_nimages;
double        Feb_Control_exposure_time_in_sec;
double        Feb_Control_exposure_period_in_sec;

unsigned int   Feb_Control_trimbit_size;
unsigned int* Feb_Control_last_downloaded_trimbits;


int Feb_Control_module_number;
int Feb_Control_current_index;


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
unsigned int Module_GetTopIDelay(struct Module* mod,unsigned int chip)                       { return chip<4                              ?  mod->idelay_top[chip]           : 0;} //chip 0=ll,1=lr,0=rl,1=rr
unsigned int Module_SetBottomIDelay(struct Module* mod,unsigned int chip,unsigned int value) { return Module_BottomAddressIsValid(mod) &&chip<4     ? (mod->idelay_bottom[chip]=value) : 0;} //chip 0=ll,1=lr,0=rl,1=rr
unsigned int Module_GetBottomIDelay(struct Module* mod,unsigned int chip)                    { return chip<4                              ?  mod->idelay_bottom[chip]        : 0;} //chip 0=ll,1=lr,0=rl,1=rr

float        Module_SetHighVoltage(struct Module* mod,float value)                           { return Module_TopAddressIsValid(mod)                 ? (mod->high_voltage=value) : -1;}
float        Module_GetHighVoltage(struct Module* mod)                                      { return mod->high_voltage;}

int          Module_SetTopDACValue(struct Module* mod,unsigned int i, int value)             { return (i<Module_ndacs && Module_TopAddressIsValid(mod))    ? (mod->top_dac[i]=value)    : -1;}
int          Module_GetTopDACValue(struct Module* mod,unsigned int i)                        { return (i<Module_ndacs) ? mod->top_dac[i]:-1;}
int          Module_SetBottomDACValue(struct Module* mod,unsigned int i, int value)          { return (i<Module_ndacs && Module_BottomAddressIsValid(mod)) ? (mod->bottom_dac[i]=value) : -1;}
int          Module_GetBottomDACValue(struct Module* mod,unsigned int i)                     { return (i<Module_ndacs) ? mod->bottom_dac[i]:-1;}



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




int Feb_Control_Init(int master, int top){
	unsigned int i;
	Feb_Control_module_number = 0;
	Feb_Control_current_index = 0;

	//global send
	Feb_Control_AddModule1(0,1,0xff,0,1);
    Feb_Control_PrintModuleList();

    //get module nummber
    int res=0;
    char hostname[100];
    if (gethostname(hostname, sizeof hostname) == 0)
        puts(hostname);
    else
        perror("gethostname");
    char *pch;
    pch = strtok(hostname,"0");
    pch = strtok(NULL,"0");
    sscanf(pch,"%d",&res);
    Feb_Control_module_number = (res & 0xFF);

    //for Gemmas modules: if master, serial 0, else 1
    int serial = 1;
    if(master)
    	serial = 0;
   /* else if(top)if slave top, serial = 2
    	serial = 2;*/



    switch(Feb_Control_module_number){
    case 34: serial = 0; break; //martin half
    case 26: serial = 0; break; //leo

    case 31: serial = 0; break; //martin
    case 32: serial = 1; break;
    case 24: serial = 2; break;
    case 25: serial = 3; break;

    case 15: serial = 0; break; //dhanya
    case 16: serial = 1; break;
    case 30: serial = 2; break;
    case 38: serial = 3; break;

    case 49: serial = 0; break; // Gemma
    case 48: serial = 1; break; // Gemma
    }
    printf("serial: %d\n",serial);

	Feb_Control_current_index = 1;

  /*for(i=1;i<moduleSize;i++){
	  if(Module_GetModuleNumber(&modules[i])==Feb_Control_module_number)
		  Feb_Control_current_index = i;
  }
printf("****current index:%d\n",i);
*/
	//Feb_Control_ReadSetUpFileToAddModules("/home/root/executables/setup.txt");

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
	  Feb_Interface_SetByteOrder();


  return 1;
}



/*
void Feb_Control_Set_Master(){
	Feb_Control_am_i_master = 1;
	unsigned int halfmastermodule = 0x80000000;
	  unsigned int reg_nums[1];
	  unsigned int reg_vals[1];
	  reg_nums[0]=DAQ_REG_CHIP_CMDS;
	  reg_vals[0]=(halfmastermodule|Feb_Control_acquireNReadoutMode|Feb_Control_triggerMode|Feb_Control_externalEnableMode|Feb_Control_subFrameMode);

	  if(!Feb_Interface_WriteRegisters((Module_GetTopLeftAddress(&modules[1])|Module_GetTopRightAddress(&modules[1])),1,reg_nums,reg_vals,0,0)){
		  printf("Trouble writing commands....\n");;
		    return 0;
	  }
	  printf("master is set\n");
}

*/



int Feb_Control_ReadSetUpFileToAddModules(char* file_name){
	   char line[100];
	   char str[100];
	   int i0,i1,i2;
	  // int memaddress = 1;
	FILE* fp = fopen(file_name, "r");
	if( fp == NULL ){
		perror("Error while opening the file.\n");
		return 0;
	}

	 printf("\nSetting up detectors:\n");

	 while ( fgets (line , 255 , fp) != NULL ){
		 if(strlen(line)<=1)
			 continue;
		 sscanf (line, "%s", str);
		 if (str[0]=='#')
		 	continue;


		 if(!strcmp(str,"add_module")){
			 if( sscanf (line,"%s %d %d %d", str,&i0,&i1,&i2) < 4){
				 printf("Error adding module from %s.\n",file_name);
				 exit(0);
			 }
			 printf ("str:%s len:%d i0:%d i1:%d i2:%d\n", str, strlen(str),i0,i1,i2);
			  if(!Feb_Control_AddModule1(i0,1,i1,i2,0)){
				  printf("Error adding module, parameter was assigned twice in setup file: %s.\n",file_name);
				  exit(0);
			  }
		 }

		 else if(!strcmp(str,"add_half_module")){
			 if( sscanf (line,"%s %d %d %d", str,&i0,&i1,&i2) < 4){
				 printf("Error adding half module from %s.\n",file_name);
				 exit(0);
			 }
			 printf ("str:%s len:%d i0:%d i1:%d i2:%d\n", str, strlen(str),i0,i1,i2);

			  if(!Feb_Control_AddModule1(i0,i1,i2,i2,1)){
				  printf("Error adding module, parameter was assigned twice in setup file: %s.\n",file_name);
				  exit(0);
			  }

			 //memaddress++;
			 Feb_Control_PrintModuleList();

			/*  if(!Feb_Control_AddModule1(i0,i1)){
				  printf("Error adding module, parameter was assigned twice in setup file: %s.\n",file_name);
				  exit(0);
			  }*/
		 }
	 }
	fclose(fp);


  Feb_Control_PrintModuleList();
  unsigned int nfebs = 0;
  unsigned int* feb_list = malloc(moduleSize*4 * sizeof(unsigned int));
  unsigned int i;
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

  return Feb_Interface_SetByteOrder();
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

  if(found_t) printf("\tWarning: top address %d already used.\n",Module_GetTopBaseAddress(m));
  if(found_b) printf("\tWarning: bottom address %d already used.\n",Module_GetBottomBaseAddress(m));


  int top_bottom_same = Module_TopAddressIsValid(m)&&Module_BottomAddressIsValid(m)&&Module_GetTopBaseAddress(m)==Module_GetBottomBaseAddress(m);
  if(top_bottom_same) printf("\tWarning: top and bottom address are the same %d.\n",Module_GetTopBaseAddress(m));

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


int Feb_Control_ReadSetUpFile(unsigned int module_num, char* file_name){
printf("Reading Setup file for module number:%d\n",module_num);
	char line[100];
	char str[100];
	int i0;
	float f0;

	FILE* fp = fopen(file_name, "r");
	if( fp == NULL ){
		perror("Error while opening the file.\n");
		return 0;
	}

	 while ( fgets (line , 255 , fp) != NULL ){
		 if(strlen(line)<=1)
			 continue;
		 sscanf (line, "%s", str);
		 if ((str[0]=='#') || (!strcmp(str,"add_module")) || (!strcmp(str,"add_half_module")))
		 	continue;

		 if(!strcmp("iodelay",str)){
			 if(sscanf (line,"%s %d", str,&i0) < 2){
				 printf("Error reading io_delay\n");
				 exit(0);
			 }
			 Feb_Control_SetIDelays(module_num,i0);
		 }

		 else if(!strcmp("high_voltage",str)){
			 if(sscanf (line,"%s %f", str,&f0) < 2){
				 printf("Error reading high_voltage\n");
				 exit(0);
			 }
			 Feb_Control_SetHighVoltage(f0);
		 }

		 else if(!strcmp("photon_energy",str)){
			 if(sscanf (line,"%s %f", str,&f0) < 2){
				 printf("Error reading photon_energy\n");
				 exit(0);
			 }
			 Feb_Control_SetPhotonEnergy(f0);
		 }

		 else if(!strcmp("dynamic_range",str)){
			 if(sscanf (line,"%s %d", str,&i0) < 2){
				 printf("Error reading dynamic_range\n");
				 exit(0);
			 }
			 Feb_Control_SetDynamicRange(i0);
		 }

		 else if(!strcmp("readout_speed",str)){
			 if(sscanf (line,"%s %d", str,&i0) < 2){
				 printf("Error reading readout_speed\n");
				 exit(0);
			 }
			 Feb_Control_SetReadoutSpeed(i0);
		 }

		 else if(!strcmp("readout_mode",str)){
			 if(sscanf (line,"%s %d", str,&i0) < 2){
				 printf("Error reading readout_mode\n");
				 exit(0);
			 }
			 Feb_Control_SetReadoutMode(i0);
		 }

		 else {
			 if( sscanf (line,"%s %f", str,&f0) < 2){
				 printf("Error reading dac\n");
				 exit(0);
			 }

			 if(module_num>0)
				 sprintf(str,"%s",str); /*sprintf(str,"mod%d::%s",module_num,str);*/
			 if(!Feb_Control_SetDAC(str,f0,1))
				 printf("error in string: %s",str);

		 }
	 }
	fclose(fp);
printf("Done reading set up file\n");
  return 1;
}


int Feb_Control_CheckSetup(){
	printf("Checking Set up\n");
	unsigned int i,j;
	int ok = 1;

  /*for(i=0;i<moduleSize;i++){*/
	  i = Feb_Control_current_index;

    for(j=0;j<4;j++){
      if(Module_GetTopIDelay(&modules[i],j)<0){
	printf("Warning: module %d's idelay top number %d not set.\n",Module_GetModuleNumber(&modules[i]),j);
	ok=0;
      }
      if(Module_GetBottomIDelay(&modules[i],j)<0){
	printf("Warning: module %d's idelay bottom number %d not set.\n",Module_GetModuleNumber(&modules[i]),j);
	ok=0;
      }
    }
    if(Module_GetHighVoltage(&modules[i])<0){
      printf("Warning: module %d's high voltage not set.\n",Module_GetModuleNumber(&modules[i]));
      ok=0;
    }
    for(j=0;j<Module_ndacs;j++){
      if(Module_GetTopDACValue(&modules[i],j)<0){
	printf("Warning: module %d's top \"%s\" dac is not set.\n",Module_GetModuleNumber(&modules[i]),Module_dac_names[i]);
	ok=0;
      }
      if(Module_GetBottomDACValue(&modules[i],j)<0){
	printf("Warning: module %d's bottom \"%s\" dac is not set.\n",Module_GetModuleNumber(&modules[i]),Module_dac_names[i]);
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

int Feb_Control_SetPhotonEnergy(unsigned int full_energy_eV){
	Feb_Control_photon_energy_eV = full_energy_eV;
  printf("Setting photon energy to: %d eV\n",Feb_Control_photon_energy_eV);

  return 1;
}

unsigned int Feb_Control_GetPhotonEnergy(){return Feb_Control_photon_energy_eV;}

int Feb_Control_SetIDelays(unsigned int module_num, unsigned int ndelay_units){
  return Feb_Control_SetIDelays1(module_num,0,ndelay_units)&&Feb_Control_SetIDelays1(module_num,1,ndelay_units)&&Feb_Control_SetIDelays1(module_num,2,ndelay_units)&&Feb_Control_SetIDelays1(module_num,3,ndelay_units);
}

int Feb_Control_SetIDelays1(unsigned int module_num, unsigned int chip_pos, unsigned int ndelay_units){  //chip_pos 0=ll,1=lr,0=rl,1=rr
	unsigned int i;
	//currently set same for top and bottom
  if(chip_pos>3){
    printf("Error SetIDelay chip_pos %d doesn't exist.\n",chip_pos);;
    return 0;
  }

  unsigned int module_index=0;
  if(!Feb_Control_GetModuleIndex(module_num,&module_index)){
    printf("Error could not set i delay module number %d invalid.\n",module_num);
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
	printf("Error could not set idelay module number %d (top_left).\n",module_num);
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
	printf("Error could not set idelay module number %d (bottom_left).\n",module_num);
	ok=0;
      }
    }
  }else{
    if(Module_TopAddressIsValid(&modules[module_index])){
      if(Feb_Control_SendIDelays(Module_GetTopRightAddress(&modules[module_index]),chip_pos%2==0,0xffffffff,ndelay_units)){
	if(module_index!=0) Module_SetTopIDelay(&modules[module_index],chip_pos,ndelay_units);
	else for(i=0;i<moduleSize;i++) Module_SetTopIDelay(&modules[i],chip_pos,ndelay_units);
      }else{
	printf("Error could not set idelay module number %d (top_right).\n",module_num);
	ok=0;
      }
    }
    if(Module_BottomAddressIsValid(&modules[module_index])){
      if(Feb_Control_SendIDelays(Module_GetBottomRightAddress(&modules[module_index]),chip_pos%2==0,0xffffffff,ndelay_units)){
	if(module_index!=0) Module_SetBottomIDelay(&modules[module_index],chip_pos,ndelay_units);
	else for(i=0;i<moduleSize;i++) Module_SetBottomIDelay(&modules[i],chip_pos,ndelay_units);
      }else{
	printf("Error could not set idelay module number %d (bottom_right).\n",module_num);
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

  if(!Feb_Interface_WriteRegister(dst_num,CHIP_DATA_OUT_DELAY_REG2, 1<<31 | delay_data_valid_nclks<<16 | ndelay_units,0,0) || //the 1<<31 time enables the setting of the data valid delays
     !Feb_Interface_WriteRegister(dst_num,CHIP_DATA_OUT_DELAY_REG3,set_left_delay_channels,0,0)  ||
     !Feb_Interface_WriteRegister(dst_num,CHIP_DATA_OUT_DELAY_REG4,set_right_delay_channels,0,0) ||
     !Feb_Interface_WriteRegister(dst_num,CHIP_DATA_OUT_DELAY_REG_CTRL,CHIP_DATA_OUT_DELAY_SET,1,1)){
    printf("Warning: could not SetChipDataInputDelays(...).\n");
    return 0;
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


int Feb_Control_SetHighVoltage(float value){
	return Feb_Control_SetHighVoltage1(Feb_Control_module_number,value);
}

int Feb_Control_SetHighVoltage1(unsigned int module_num,float value){
  unsigned int module_index=0;
  unsigned int i;

  if(Module_TopAddressIsValid(&modules[module_index])){
	  if(!Feb_Control_GetModuleIndex(module_num,&module_index)){/*||!Module_TopAddressIsValid(&modules[module_index])){*/
		  printf("Error could not set high voltage module number %d invalid.\n",module_num);
		  return 0;
	  }
  }else
	  return 0;

  if(!Feb_Control_SendHighVoltage(Module_GetTopRightAddress(&modules[module_index]),&value)) return 0;

  if(module_index!=0)  Module_SetHighVoltage(&modules[module_index],value);
  else for(i=0;i<moduleSize;i++) Module_SetHighVoltage(&modules[i],value);

  printf("\tHigh voltage of dst %d set to %f.\n",Module_GetTopRightAddress(&modules[module_index]),Module_GetHighVoltage(&modules[module_index]));
  return 1;
}


int Feb_Control_SendHighVoltage(unsigned int dst_num,float* value){
  //  printf("sending high voltage to dst_num "<<dst_num<<".\n");;

  static const unsigned int nsteps = 256;
  static const float vmin=0;
  static const float vmax=300;

  unsigned int b = 0;
  if(!Feb_Control_VoltageToDAC(*value,&b,nsteps,vmin,vmax)){
    printf("Waring: SetHighVoltage bad value, %f.  The range is 0 to 300 V.\n",*value);
    return 0;
  }

  unsigned int r = 0x20000000 | (b&0xff);
  if(!Feb_Interface_WriteRegister(dst_num,0,r,0,0)){
    printf("Warning: trouble setting high voltage for dst_num %d.\n",dst_num);
    return 0;
  }

  *value = Feb_Control_DACToVoltage(b,nsteps,vmin,vmax);

  return 1;
}



int Feb_Control_DecodeDACString(char* dac_str, unsigned int* module_index, int* top, int* bottom, unsigned int* dac_ch){
  char*       local_s = dac_str;
  //char temp[50];
  *module_index  = Feb_Control_current_index;

/*
  char* p1 = strstr(local_s,"mod");//size_t p1 = local_s.find("mod");
  char* p2 = strstr(local_s,"::");//size_t p2 =local_s.find("::");
  if(p1!=NULL&&p2!=NULL&&(p1+3)<p2){//if(p1!=string::npos&&p2!=string::npos&&(p1+3)<p2){
	  strncpy(temp, p1+3, (p2-p1));
	  temp[p2-p1] = '\0';
	  unsigned int number = atoi(temp); //unsigned int number = atoi((local_s.substr(p1+3,p2-3)).c_str());

    if(!Feb_Control_GetModuleIndex(number,module_index)){
      printf("Error in dac_name \"%s\", module number %d not in list.\n",dac_str,number);
      return 0;
    }
    strcpy(local_s,p2+2);//local_s    = local_s.substr(p2+2);
  }
*/

  *top    = 1;//make them both 1 instead of this
  *bottom = 1;
  /*if(p1 = strstr(local_s,"top::")!=NULL){
    strcpy(local_s,p1+5);
    *bottom=0;
  }else if(p1 = strstr(local_s,"bottom::")!=NULL){
    strcpy(local_s,p1+8);
    *top=0;
  }*/

  if(Module_BottomAddressIsValid(&modules[*module_index]))
	  *top=0;
  else
	  *bottom=0;




  *dac_ch = 0;
  if(!Feb_Control_GetDACNumber(local_s,dac_ch)){
    printf("Error in dac_name: %s  (%s)\n",dac_str,local_s);
    return 0;
  }

  return 1;
}

int Feb_Control_SetDAC(char* dac_str, int value, int is_a_voltage_mv){
  /*
  string       local_s = dac_str;
  unsigned int module_index  = 0;

  size_t p1 = local_s.find("mod");
  size_t p2 = local_s.find("::");
  if(p1!=string::npos&&p2!=string::npos&&(p1+3)<p2){
    unsigned int number = atoi((local_s.substr(p1+3,p2-3)).c_str()); 
    if(!GetModuleIndex(number,module_index)){
      printf("Error in dac_name \""<<dac_str<<"\", module number "<<number<<" not in list.\n");;
      return 0;
    }
    local_s    = local_s.substr(p2+2);
  }

  int top    = 1;
  int bottom = 1;
  if((p1 = local_s.find("top::"))!=string::npos){
    local_s = local_s.substr(p1+5);
    bottom=0;
  }else if((p1 = local_s.find("bottom::"))!=string::npos){
    local_s = local_s.substr(p1+8);
    top=0;
  }
  
  unsigned int dac_ch = 0;
  if(!GetDACNumber(local_s,dac_ch)){
    printf("Error in dac_name: "<<dac_str<<"  ("<<local_s<<")\n");;
    return 0;
  }
  */
	unsigned int i;
  unsigned int module_index, dac_ch;
  int top, bottom;
  if(!Feb_Control_DecodeDACString(dac_str,&module_index,&top,&bottom,&dac_ch)) return 0;

  unsigned int v = value;
  if(is_a_voltage_mv&&!Feb_Control_VoltageToDAC(value,&v,4096,0,2048)){
    printf("Warning: SetDac bad value, %d. The range is 0 to 2048 mV.\n",value);
    return 0;
  }
  if(v<0||v>4095){
    printf("Warning: SetDac bad value, %d. The range is 0 to 4095.\n",v);
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
    printf("Warning: GetDACName index out of range, %d invalid.\n",dac_num);
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

  //static unsigned int nsteps = 4096; //12 bit dac
  //static float vmin = 0;
  //static float vmax = 2;

  if(ch<0||ch>15){
    printf("Warning invalid ch for SetDAC.\n");
    return 0;
  }

  //if(voltage<0) return PowerDownDAC(socket_num,ch);

  *value&=0xfff;
  unsigned int dac_ic = (ch<8) ? 1:2;
  unsigned int dac_ch =  ch%8;
  unsigned int r      = dac_ic<<30 | 3<<16 | dac_ch<<12 | *value; //3 write and power up


  if(!Feb_Interface_WriteRegister(dst_num,0,r,1,0)){
     printf("Warning: trouble setting dac %d voltage.\n",ch);
     return 0;
  }

  float voltage=Feb_Control_DACToVoltage(*value,4096,0,2048);

  printf("\tDac number %d (%s) of dst %d set to %d (%f mV).\n",ch,Module_dac_names[ch],dst_num,*value,voltage);
  return 1;
}

/*
float GetDAC(string s){
  static unsigned int n;
  if(!GetDACNumber(s,n)) return 0;

  return dac[n];
}
*/

int Feb_Control_SetTrimbits(unsigned int module_num, unsigned int *trimbits){
	printf("Setting Trimbits\n");

	//for (int iy=10000;iy<20020;++iy)//263681
	//for (int iy=263670;iy<263680;++iy)//263681
	//	printf("%d:%c\t\t",iy,trimbits[iy]);

	unsigned int trimbits_to_load_l[1024];
	unsigned int trimbits_to_load_r[1024];

	unsigned int module_index=0;
	if(!Feb_Control_GetModuleIndex(module_num,&module_index)){
		printf("Warning could not set trimbits, bad module number.\n");
		return 0;
	}

	if(!Feb_Control_Reset()) printf("Warning could not reset DAQ.\n");
	int l_r;	//printf("222\n");
	for(l_r=0;l_r<2;l_r++){ // l_r loop
		//printf("\nl_r:%d\t\t",l_r);
		unsigned int disable_chip_mask = l_r ? DAQ_CS_BAR_LEFT : DAQ_CS_BAR_RIGHT;
		if(!(Feb_Interface_WriteRegister(0xfff,DAQ_REG_STATIC_BITS,disable_chip_mask|DAQ_STATIC_BIT_PROGRAM|DAQ_STATIC_BIT_M8,0,0)&&Feb_Control_SetCommandRegister(DAQ_SET_STATIC_BIT)&&Feb_Control_StartDAQOnlyNWaitForFinish(5000))){
			printf("Could not select chips\n");
			return 0;
		}
		int row_set;
		for(row_set=0;row_set<16;row_set++){ //16 rows at a time
			//printf("row_set:%d\t\t",row_set);
			if(row_set==0){
				if(!Feb_Control_SetCommandRegister(DAQ_RESET_COMPLETELY|DAQ_SEND_A_TOKEN_IN|DAQ_LOAD_16ROWS_OF_TRIMBITS)){
					printf("Warning: Could not Feb_Control_SetCommandRegister for loading trim bits.\n");
					return 0;
				}
			}else{
				if(!Feb_Control_SetCommandRegister(DAQ_LOAD_16ROWS_OF_TRIMBITS)){
					printf("Warning: Could not Feb_Control_SetCommandRegister for loading trim bits.\n");
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
				if(!Feb_Interface_WriteMemoryInLoops(Module_GetTopLeftAddress(&modules[Feb_Control_current_index]),0,0,1024,trimbits_to_load_r)||
						!Feb_Interface_WriteMemoryInLoops(Module_GetTopRightAddress(&modules[Feb_Control_current_index]),0,0,1024,trimbits_to_load_l)||
						//if(!Feb_Interface_WriteMemory(Module_GetTopLeftAddress(&modules[0]),0,0,1023,trimbits_to_load_r)||
						//	!Feb_Interface_WriteMemory(Module_GetTopRightAddress(&modules[0]),0,0,1023,trimbits_to_load_l)||
						!Feb_Control_StartDAQOnlyNWaitForFinish(5000)){
					printf(" some errror!\n");
					return 0;
				}
			}else{
				if(!Feb_Interface_WriteMemoryInLoops(Module_GetBottomLeftAddress(&modules[Feb_Control_current_index]),0,0,1024,trimbits_to_load_r)||
						!Feb_Interface_WriteMemoryInLoops(Module_GetBottomRightAddress(&modules[Feb_Control_current_index]),0,0,1024,trimbits_to_load_l)||
						//if(!Feb_Interface_WriteMemory(Module_GetTopLeftAddress(&modules[0]),0,0,1023,trimbits_to_load_r)||
						//	!Feb_Interface_WriteMemory(Module_GetTopRightAddress(&modules[0]),0,0,1023,trimbits_to_load_l)||
						!Feb_Control_StartDAQOnlyNWaitForFinish(5000)){
					printf(" some errror!\n");
					return 0;
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

/*
   if(Module_BottomAddressIsValid(&modules[1])){
	  printf("************* bottom\n");
	  //if(Feb_Control_am_i_master)
		  return Module_GetBottomLeftAddress(&modules[1])|Module_GetBottomRightAddress(&modules[1]);
	 // else return 0;
  }
  printf("************* top\n");
*/
	//return Module_GetTopLeftAddress(&modules[1])|Module_GetTopRightAddress(&modules[1]);
	  return Module_GetTopLeftAddress(&modules[0])|Module_GetTopRightAddress(&modules[0]);


}

int Feb_Control_SetCommandRegister(unsigned int cmd){
  return Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_CHIP_CMDS,cmd,0,0);
}


int Feb_Control_GetDAQStatusRegister(unsigned int dst_address, unsigned int* ret_status){
  if(!Feb_Interface_ReadRegister(dst_address,DAQ_REG_STATUS,ret_status)){
    printf("Error: reading status register.\n");
    return 0;
  }

  *ret_status = (0x00FF0000 & *ret_status) >> 16;
  return 1;
}


int Feb_Control_StartDAQOnlyNWaitForFinish(int sleep_time_us){
  if(!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_CTRL,0,0,0)||!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_CTRL,DAQ_CTRL_START,0,0)){
    printf("Warning: could not start.\n");
    return 0;
  }
  
  return Feb_Control_WaitForFinishedFlag(sleep_time_us);
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

int Feb_Control_AcquisitionInProgress(){
	unsigned int status_reg_r=0,status_reg_l=0;

	int ind = Feb_Control_current_index;
	if(Module_BottomAddressIsValid(&modules[ind])){

		if(!(Feb_Control_GetDAQStatusRegister(Module_GetBottomRightAddress(&modules[ind]),&status_reg_r)))
		{printf("ERROR: Trouble reading Status register. bottom right address\n");return 0;}
		if(!(Feb_Control_GetDAQStatusRegister(Module_GetBottomLeftAddress(&modules[ind]),&status_reg_l)))
		{printf("ERROR: Trouble reading Status register. bottom left address\n");return 0;}

	}else{
		if(!(Feb_Control_GetDAQStatusRegister(Module_GetTopRightAddress(&modules[ind]),&status_reg_r)))
		{printf("ERROR: Trouble reading Status register. top right address\n");
			if(!(Feb_Control_GetDAQStatusRegister(Module_GetTopRightAddress(&modules[0]),&status_reg_r)))
			printf("ERROR: error with normal register\n");
			else
				printf("**********NO error reading normal register\n");
		return 0;}
		if(!(Feb_Control_GetDAQStatusRegister(Module_GetTopLeftAddress(&modules[ind]),&status_reg_l)))
		{printf("ERROR: Trouble reading Status register. top left address\n");return 0;}
	}
	if((status_reg_r|status_reg_l)&DAQ_STATUS_DAQ_RUNNING) {/*printf("**runningggg\n");*/return 1;}

	/*
    if(!(GetDAQStatusRegister(modules[i]->Module_GetTopLeftAddress(),status_reg_r)&&GetDAQStatusRegister(modules[i]->Module_GetTopRightAddress(),status_reg_l))){
      for(int i=0;i<2;i++) printf("Waring trouble reading status register. Returning zero to avoid inifite loops, this could cause trouble!"\n");;
      return 0; //to avoid inifite loops
    }
    if((status_reg_r|status_reg_l)&DAQ_STATUS_DAQ_RUNNING) return 1;
  }
	 */

	/*printf("**idle\n");*/
	return 0; //i.e. not running (status_reg_r|status_reg_l)&DAQ_STATUS_DAQ_RUNNING;
}

int Feb_Control_Reset(){
  if(!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_CTRL,0,0,0) || !Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_CTRL,DAQ_CTRL_RESET,0,0) || !Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_CTRL,0,0,0)){
    printf("Warning: Could not reset daq, no response.\n");
    return 0;
  }

  return Feb_Control_WaitForFinishedFlag(5000);
}




int Feb_Control_SetStaticBits(){
  //program=1,m4=2,m8=4,test=8,rotest=16,cs_bar_left=32,cs_bar_right=64
  if(!Feb_Interface_WriteRegister(Feb_Control_AddressToAll(),DAQ_REG_STATIC_BITS,Feb_Control_staticBits,0,0) || !Feb_Control_SetCommandRegister(DAQ_SET_STATIC_BIT) || !Feb_Control_StartDAQOnlyNWaitForFinish(5000)){
    printf("Warning: Could not set static bits\n");
    return 0;
  }

  return 1;
}
int Feb_Control_SetStaticBits1(unsigned int the_static_bits){
	Feb_Control_staticBits = the_static_bits;
  return Feb_Control_SetStaticBits();
}

int Feb_Control_SetTestModeVariable(int on){
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
    printf("Warning: dynamic range (%d) not valid, not setting bit mode.\n",four_eight_sixteen_or_thirtytwo);
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
      printf("Warning readout speed %d unknown, defaulting to full speed.\n",readout_speed);
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
      printf("Warning readout mode %d) unknown, defaulting to full speed.\n",readout_mode);
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
    if(trigger_mode) printf("Warning trigger %d) unknown, defaulting to internal triggering.\n",trigger_mode);;

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
	  Feb_Control_externalEnableMode  &= (~DAQ_NEXPOSURERS_EXTERNAL_ENABLING);
    printf("External enabling disabled.\n");;
  }

  return 1;
}

int Feb_Control_SetNExposures(unsigned int n_images){
  if(!n_images){
    printf("Warning nimages must be greater than zero.%d\n",n_images);
    return 0;
  }

  Feb_Control_nimages = n_images;
  printf("Number of images set to: %d\n",Feb_Control_nimages);
  return 1;
}
unsigned int Feb_Control_GetNExposures(){return Feb_Control_nimages;}

int Feb_Control_SetExposureTime(double the_exposure_time_in_sec){
	Feb_Control_exposure_time_in_sec = the_exposure_time_in_sec;
  printf("Exposure time set to: %f\n",Feb_Control_exposure_time_in_sec);
  return 1;
}
double Feb_Control_GetExposureTime(){return Feb_Control_exposure_time_in_sec;}

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
    printf("Warning: time exceeds (%f) maximum exposure time of %f sec.\n",time_in_sec,max_time);
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
    printf("Warning: could not ResetChipCompletely().\n");;
    return 0;
  }   

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
/*
  static int first_pass = 1;
  static char buffer[1024];

  if(first_pass&&!Feb_Control_SetupSendToSocket("localhost",43212)) return 0;
  else first_pass=0;
*/
  unsigned int just_bit_mode = (DAQ_STATIC_BIT_M4|DAQ_STATIC_BIT_M8) & Feb_Control_staticBits;
  unsigned int bit_mode = 16; //default
  if(just_bit_mode == DAQ_STATIC_BIT_M4)  bit_mode = 4;
  else if(just_bit_mode == DAQ_STATIC_BIT_M8) bit_mode = 8;
  else if(Feb_Control_subFrameMode&DAQ_NEXPOSURERS_ACTIVATE_AUTO_SUBIMAGING) bit_mode = 32;


  if(!Beb_SetUpTransferParameters(bit_mode)){
	  printf("Error: sending bit mode ...\n");
	  return 0;
  }

  /*
  bzero(buffer,1024);
  sprintf(buffer,"setbitmode %d",bit_mode);

  if(Feb_Control_WriteNRead(buffer,strlen(buffer),1024)<1||strncmp(buffer,"0",1)){
    printf("Error: sending bit mode ...\n");
    return 0;
  }
*/
  return 1;
}

/*
int Feb_Control_SetupSendToSocket(const char* ip_address_hostname, unsigned short int port){

    struct hostent *server;
    if((server = gethostbyname(ip_address_hostname)) == NULL){  //or look into getaddrinfo(3)
      fprintf(stderr,"ERROR, no such host\n");
      return 0;
    }

    //struct sockaddr_in serv_addr;
    bzero((char *) &Feb_Control_serv_addr, sizeof(Feb_Control_serv_addr));
    Feb_Control_serv_addr.sin_family = AF_INET;
      bcopy((char *)server->h_addr,(char *)&Feb_Control_serv_addr.sin_addr.s_addr,server->h_length);
      Feb_Control_serv_addr.sin_port = htons(port);

  return 1;
}

int Feb_Control_WriteNRead(char* message, int length, int max_length){

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd <0){
    fprintf(stderr,"ERROR opening socket\n");
    return 0;
  }

  if(connect(sockfd,(struct sockaddr *) &Feb_Control_serv_addr,sizeof(Feb_Control_serv_addr)) < 0){
    fprintf(stderr,"ERROR connecting\n");
    return 0;
  }

  int n = write(sockfd,message,length);
    if(n<0) printf("ERROR writing to socket");
 
  length = read(sockfd,message,max_length);
    if(length<0) printf("ERROR reading to socket");

  close(sockfd);

  return length;
}
*/

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

	   if(!Feb_Control_ResetChipCompletely()){
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

	  // if(!Feb_Interface_WriteRegisters((Module_GetTopLeftAddress(&modules[1])|Module_GetTopRightAddress(&modules[1])),20,reg_nums,reg_vals,0,0)){
	   if(!Feb_Interface_WriteRegisters(Feb_Control_AddressToAll(),5,reg_nums,reg_vals,0,0)){
	     printf("Trouble starting acquisition....\n");;
	     return 0;
	   }
	 //*/

	  /* if(!Feb_Control_am_i_master)
		   Feb_Control_StartAcquisition();*/
	   return 1;
}



int Feb_Control_StartAcquisition(){printf("****** starting acquisition********* \n");

  static unsigned int reg_nums[20];
  static unsigned int reg_vals[20];
/*
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

  if(!Feb_Control_ResetChipCompletely()){
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
  */

  /*
    reg_nums[4]=DAQ_REG_CHIP_CMDS;
    reg_vals[4]=(Feb_Control_acquireNReadoutMode|Feb_Control_triggerMode|Feb_Control_externalEnableMode|Feb_Control_subFrameMode);
   */

/*
  if(!Feb_Interface_WriteRegisters(Feb_Control_AddressToAll(),4,reg_nums,reg_vals,0,0)){
    printf("Trouble starting acquisition....\n");;
    return 0;
  }
  unsigned int masterHalfModuleMode = 0;
  reg_nums[0]=DAQ_REG_CHIP_CMDS;
  reg_vals[0]=(masterHalfModuleMode|Feb_Control_acquireNReadoutMode|Feb_Control_triggerMode|Feb_Control_externalEnableMode|Feb_Control_subFrameMode);
  if(!Feb_Interface_WriteRegisters(Feb_Control_AddressToAll(),1,reg_nums,reg_vals,0,0)){
	  printf("Trouble writing commands....\n");;
	    return 0;
  }
  masterHalfModuleMode = 0x80000000;
  reg_nums[0]=DAQ_REG_CHIP_CMDS;
  reg_vals[0]=(masterHalfModuleMode|Feb_Control_acquireNReadoutMode|Feb_Control_triggerMode|Feb_Control_externalEnableMode|Feb_Control_subFrameMode);
  if(!Feb_Interface_WriteRegisters((Module_GetTopLeftAddress(&modules[1])|Module_GetTopRightAddress(&modules[1])),1,reg_nums,reg_vals,0,0)){
	  printf("Trouble writing commands....\n");;
	    return 0;
  }

  int i;
  for(i=0;i<14;i++){
    reg_nums[i]=DAQ_REG_CTRL;
    reg_vals[i]=0;
  }
  reg_nums[14]=DAQ_REG_CTRL;
  reg_vals[14]=ACQ_CTRL_START;

  if(!Feb_Interface_WriteRegisters(Feb_Control_AddressToAll(),15,reg_nums,reg_vals,0,0)){
    printf("Trouble starting acquisition....\n");;
    return 0;
  }
*/
  ///*
  int i;
  for(i=0;i<14;i++){
    reg_nums[i]=DAQ_REG_CTRL;
    reg_vals[i]=0;
  }
  reg_nums[14]=DAQ_REG_CTRL;
  reg_vals[14]=ACQ_CTRL_START;

  if(!Feb_Interface_WriteRegisters(Feb_Control_AddressToAll(),15,reg_nums,reg_vals,0,0)){
    printf("Trouble starting acquisition....\n");;
    return 0;
  }
  //*/
  /*
  int i;
  for(i=5;i<19;i++){
    reg_nums[i]=DAQ_REG_CTRL;
    reg_vals[i]=0;
  }
  reg_nums[19]=DAQ_REG_CTRL;
  reg_vals[19]=ACQ_CTRL_START;
  
  if(!Feb_Interface_WriteRegisters(Feb_Control_AddressToAll(),20,reg_nums,reg_vals,0,0)){
  printf("Trouble starting acquisition....\n");;
  return 0;
}
*/

//*/
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
