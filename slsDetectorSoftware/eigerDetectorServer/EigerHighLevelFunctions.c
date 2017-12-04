
/**
 * @author Ian Johnson
 * @version 1.0
 */

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "slsDetectorServer_defs.h" //include port number



int eiger_nexposures = 1;
float eiger_exposuretime = 0;
float eiger_exposureperiod = 0;
int eiger_ncycles = 1;
int eiger_ngates = 0;
int eiger_getphotonenergy = 0;
int eiger_dynamicrange = 0;
int eiger_readoutspeed = 0;
int eiger_readoutmode = 0;
int eiger_highvoltage = 0;
int eiger_iodelay = 0;
int eiger_triggermode = 0;
int eiger_extgating = 0;
int eiger_extgatingpolarity = 0;

const unsigned int ndacs = 16;
const char* dac_names[16] = {"SvP","Vtr","Vrf","Vrs","SvN","Vtgstv","Vcmp_ll","Vcmp_lr","cal","Vcmp_rl","rxb_rb","rxb_lb","Vcmp_rr","Vcp","Vcn","Vis"};

int saved_trimbits[256*256*4];


int EigerGetNumberOfExposures(){return eiger_nexposures;}
float EigerGetExposureTime(){return eiger_exposuretime;}
float EigerGetExposurePeriod(){return eiger_exposureperiod;}
int EigerGetNumberOfCycles(){return eiger_ncycles;}
/*int EigerGetNumberOfGates(){return eiger_ngates;}*/
unsigned int EigerGetDynamicRange(){return eiger_dynamicrange;}
int EigerGetPhotonEnergy(){return eiger_getphotonenergy;}
int EigerGetReadoutSpeed(){return eiger_readoutspeed;}
int EigerGetReadoutMode(){return eiger_readoutmode;}
int EigerGetHighVoltage(){return eiger_highvoltage;}
int EigerGetIODelay(){return eiger_iodelay;}
int EigerGetTriggerMode(){return eiger_triggermode;}
int EigerGetExternalGating(){return eiger_extgating;}
int EigerGetExternalGatingPolarity(){return eiger_extgatingpolarity;}

int EigerInit(){
	saved_trimbits[0] = -1;


}


int EigerSendCMD(){
	if(!EigerInit()||eiger_message_length<=0) return 0;

	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0){
		fprintf(stderr,"ERROR opening socket\n");
		return 0;
	}

	if(connect(sockfd,(struct sockaddr *) &eiger_socket_addr,sizeof(eiger_socket_addr))<0){
		fprintf(stderr,"ERROR connecting\n");
		return 0;
	}

	int n = write(sockfd,eiger_message,eiger_message_length);

	int ret_length = read(sockfd,eiger_message,eiger_max_message_length);

	close(sockfd);

	if(n<0||ret_length<0) return 0;


	//fprintf(stdout,"%s\n",eiger_message);
	if(eiger_ret_val>0){
		int i=0;
		eiger_message[1]='\0';
		if(atoi(eiger_message)!=0) return 0;

		for(i=2;i<ret_length;i++){
			if(eiger_message[i] == ' '){
				//fprintf(stdout," in : %d \n",i);
				eiger_message[i]='\0';
				break;
			}
		}
		eiger_ret_val = atoi(&eiger_message[2]);
		//fprintf(stdout," the \"%s\" %d\n",&eiger_message[2],eiger_ret_val);
	}

	eiger_message_length = 0;

	return 1;
}

const char* EigerGetDACName(int i){
	if(i>0&&i<ndacs) return dac_names[i];
	return dac_names[0];
}


int EigerSetDAC(const char* iname,int v, int mV){
	eiger_ret_val=0;
	if(mV)
		eiger_message_length = sprintf(eiger_message,"setdacvoltage %s %d",iname,v);
	else
		eiger_message_length = sprintf(eiger_message,"setdacvalue %s %d",iname,v);
	return EigerSendCMD();
}

int EigerSetTrimbits(const int *data){
	eiger_ret_val=0;
	/*char tt[263681];
	tt[263680]='\0';
	int ip=0, ich=0;
	int iy, ix;
	int ichip;

	// convert the trimbits from int32 to chars and add border pixels.
	for(iy=0;iy<256;iy++) {
	  for (ichip=0; ichip<4; ichip++) {
	    for(ix=0;ix<256;ix++) {
	      tt[ip++]=(char)((data[ich++]&(0x3f))+'0');
	    }
	    if (ichip<3) {
	      tt[ip++]=(char)(0+'0');
	      tt[ip++]=(char)(0+'0');
	    }
	  }
	}
	eiger_message_length = sprintf(eiger_message,"settrimbits %s", tt);
	memcpy(saved_trimbits,data,256*256*4*sizeof(int));*/

	eiger_message_length = sprintf(eiger_message,"settrimbits %d", 0);

	return EigerSendCMD();
}



int EigerSetAllTrimbits(unsigned int value){
	eiger_ret_val=0;
	/*char tt[263681];
	tt[263680]='\0';
	int ip=0, ich=0;
	int iy, ix;
	int ichip;
	int sl=0;


	// convert the trimbits from int32 to chars and add border pixels.
	for(iy=0;iy<256;iy++) {
		for (ichip=0; ichip<4; ichip++) {
			for(ix=0;ix<256;ix++) {
				tt[ip++]=(char)((value&0x3f)+'0');
			}
			if (ichip<3) {
				tt[ip++]=(char)(0+'0');
				tt[ip++]=(char)(0+'0');
			}
		}
	}

	eiger_message_length = sprintf(eiger_message,"settrimbits %s", tt);
	for(iy=0;iy<256*256*4;++iy)
		saved_trimbits[iy] = value;*/
	eiger_message_length = sprintf(eiger_message,"setalltrimbits %d", value);
	return EigerSendCMD();
}



 int EigerGetTrimbits(const int *data){
 	eiger_ret_val=0;
 	/*char tt[263681];
 	tt[263680]='\0';
 	int ip=0, ich=0;
 	int iy, ix;
 	int ichip;

 	eiger_message_length = sprintf(eiger_message,"gettrimbits ");
 	memcpy(data,saved_trimbits,256*256*4*sizeof(int));*/

 	eiger_message_length = sprintf(eiger_message,"gettrimbits ");
 	return EigerSendCMD();
 }



int EigerGetDAC(const char* iname){
	eiger_ret_val=1;
	eiger_message_length = sprintf(eiger_message,"getdacvalue %s",iname);
	if(!EigerSendCMD()) return -1;
	return eiger_ret_val;
}

int EigerGetDACmV(const char* iname){
	eiger_ret_val=1;
	eiger_message_length = sprintf(eiger_message,"getdacvoltage %s",iname);
	if(!EigerSendCMD()) return -1;
	return eiger_ret_val;
}

int EigerSetNumberOfExposures(unsigned int n){
	eiger_nexposures = n;
	eiger_ret_val=0;
	eiger_message_length = sprintf(eiger_message,"setnumberofexposures %u",eiger_nexposures*eiger_ncycles);
	return EigerSendCMD();
}

int EigerSetExposureTime(float v){
	eiger_exposuretime = v;
	eiger_ret_val=0;
	eiger_message_length = sprintf(eiger_message,"setexposuretime %f",v);
	return EigerSendCMD();
}


int EigerSetExposurePeriod(float v){
	eiger_exposureperiod = v;
	eiger_ret_val=0;
	eiger_message_length = sprintf(eiger_message,"setexposureperiod %f",v);
	return EigerSendCMD();
}

int EigerSetNumberOfCycles(unsigned int n){
	eiger_ncycles = n;
	eiger_ret_val=0;
	eiger_message_length = sprintf(eiger_message,"setnumberofexposures %u",eiger_nexposures*eiger_ncycles);
	return EigerSendCMD();
}
/*
int EigerSetNumberOfGates(unsigned int n){
  eiger_ngates = n;
  eiger_ret_val=0;
  eiger_message_length = sprintf(eiger_message,"setnumberofexposures %u",n);
  return EigerSendCMD();
}
 */
int EigerSetDynamicRange(unsigned int i){
	eiger_dynamicrange = i;
	eiger_ret_val=0;
	eiger_message_length = sprintf(eiger_message,"setbitmode %u",i);
	return EigerSendCMD();
}


int EigerSetPhotonEnergy(int in_eV){
	eiger_getphotonenergy = in_eV;
	eiger_ret_val=0;
	eiger_message_length = sprintf(eiger_message,"setphotonenergy %d",in_eV);
	return EigerSendCMD();
}

int EigerSetReadoutSpeed(int speed){
	eiger_readoutspeed = speed;
	eiger_ret_val=0;
	eiger_message_length = sprintf(eiger_message,"setreadoutspeed %d",speed);
	return EigerSendCMD();
}

int EigerSetReadoutMode(int mode){
	eiger_readoutmode = mode;
	eiger_ret_val=0;
	eiger_message_length = sprintf(eiger_message,"setreadoutmode %d",mode);
	return EigerSendCMD();
}

int EigerSetHighVoltage(int hv){
	eiger_highvoltage = hv;
	eiger_ret_val=0;
	eiger_message_length = sprintf(eiger_message,"sethighvoltage %d",hv);
	return EigerSendCMD();
}

int EigerSetIODelay(int io){
	eiger_iodelay = io;
	eiger_ret_val=0;
	eiger_message_length = sprintf(eiger_message,"setinputdelays %d",io);
	return EigerSendCMD();
}

int EigerSetTriggerMode(int m){
	eiger_triggermode = m;
	eiger_ret_val=0;
	eiger_message_length = sprintf(eiger_message,"settriggermode %d",m);
	return EigerSendCMD();
}

int EigerSetExternalGating(int e, int p){
	eiger_extgating = e;
	eiger_extgatingpolarity = p;
	eiger_ret_val=0;
	eiger_message_length = sprintf(eiger_message,"setexternalgating %d %d",e,p);
	return EigerSendCMD();
}

int EigerStartAcquisition(){
	eiger_ret_val=0;
	eiger_message_length = sprintf(eiger_message,"startacquisition");
	return EigerSendCMD();
}


int EigerRunStatus(){
	eiger_ret_val=1;
	eiger_message_length = sprintf(eiger_message,"isdaqstillrunning");
	if(!EigerSendCMD()) return -1;
	return eiger_ret_val;
}

int EigerStopAcquisition(){
	eiger_ret_val=0;
	eiger_message_length = sprintf(eiger_message,"stopacquisition");
	return EigerSendCMD();
}

int EigerWaitForAcquisitionFinish(){
	eiger_ret_val=0;
	eiger_message_length = sprintf(eiger_message,"waituntildaqfinished");
	return EigerSendCMD();
}


#ifdef TESTEIGERFUNCTIONS
int main(){

	int v=220;
	char n[2000] = "Vcmp_lr";

	fprintf(stdout," ret : %d\n",EigerSetDAC(EigerGetDACName(7),2200));
	int t = EigerGetDAC(EigerGetDACName(7));
	fprintf(stdout," v : %d\n",t);

	fprintf(stdout," ret : %d\n",EigerSetDAC(n,v));
	t = EigerGetDAC(n);
	fprintf(stdout," ret : %d\n",t);

	float f=0.12;
	fprintf(stdout," ret : %d\n",EigerSetNumberOfExposures(120));
	fprintf(stdout," ret : %d\n",EigerSetExposureTime(0.12));
	fprintf(stdout," ret : %d\n",EigerSetExposurePeriod(0.22));
	fprintf(stdout," ret : %d\n",EigerSetPhotonEnergy(9200));
	fprintf(stdout," ret : %d\n",EigerSetDynamicRange(16));
	fprintf(stdout," ret : %d\n",EigerStartAcquisition());
	fprintf(stdout," aret : %d\n",EigerRunStatus());
	sleep(1);
	fprintf(stdout," ret : %d\n",EigerStopAcquisition());
	fprintf(stdout," bret : %d\n",EigerRunStatus());

	return 0;
}
#endif


