#include "MySocketTCP.h"

#include "sls_detector_defs.h"
#include "sls_receiver_defs.h"

#include "sls_detector_funcs.h"

#include <stdio.h>
#include <stdlib.h>

#define INVALID	-999

enum detectorFunctions{
	F_GET_ADC=13
};

void help() {
	cerr << "Usage:\n"
			"slsAdcReader [hostname] [adcval]" << endl;
	exit(EXIT_FAILURE);
}


slsDetectorDefs::dacIndex getADCEnum(char* cval){
	int idac;
	string cmd;
	cmd.assign(cval);

	if (sscanf(cval,"adc:%d",&idac)==1) {
		return (slsDetectorDefs::dacIndex)(idac+1000);
	} else if (cmd=="temp_adc")
		return slsDetectorDefs::TEMPERATURE_ADC;
	else if (cmd=="temp_fpga")
		return slsDetectorDefs::TEMPERATURE_FPGA;
	else if (cmd=="temp_fpgaext")
		return slsDetectorDefs::TEMPERATURE_FPGAEXT;
	else if (cmd=="temp_10ge")
		return slsDetectorDefs::TEMPERATURE_10GE;
	else if (cmd=="temp_dcdc")
		return slsDetectorDefs::TEMPERATURE_DCDC;
	else if (cmd=="temp_sodl")
		return slsDetectorDefs::TEMPERATURE_SODL;
	else if (cmd=="temp_sodr")
		return slsDetectorDefs::TEMPERATURE_SODR;
	else if (cmd=="temp_fpgafl")
		return slsDetectorDefs::TEMPERATURE_FPGA2;
	else if (cmd=="temp_fpgafr")
		return slsDetectorDefs::TEMPERATURE_FPGA3;
	else if (cmd=="i_a")
		return slsDetectorDefs::I_POWER_A;
	else if (cmd=="i_b")
		return slsDetectorDefs::I_POWER_B;
	else if (cmd=="i_c")
		return slsDetectorDefs::I_POWER_C;
	else if (cmd=="i_d")
		return slsDetectorDefs::I_POWER_D;
	else if (cmd=="vm_a")
		return slsDetectorDefs::V_POWER_A;
	else if (cmd=="vm_b")
		return slsDetectorDefs::V_POWER_B;
	else if (cmd=="vm_c")
		return slsDetectorDefs::V_POWER_C;
	else if (cmd=="vm_d")
		return slsDetectorDefs::V_POWER_D;
	else if (cmd=="vm_io")
		return slsDetectorDefs::V_POWER_IO;
	else if (cmd=="i_io")
		return slsDetectorDefs::I_POWER_IO;
	else {
		cerr << "cannot decode dac " << cmd << endl;
		help();
		return slsDetectorDefs::I_POWER_IO;
	}
};



int main(int argc, char* argv[])
{
	if (argc < 3)
		help();
	slsDetectorDefs::dacIndex idx=getADCEnum(argv[2]);


	char mess[MAX_STR_LENGTH]="";
	detectorFunctions fnum=F_GET_ADC;
	int retval=-1;
	int ret=slsReceiverDefs::FAIL;
	int arg[2]={idx,0};

	MySocketTCP* mySocket = 0;

	try	{
		mySocket = new MySocketTCP(argv[1],1952);
    } catch (...) {
    	cerr << "could not create socket with " << argv[1] << endl;
    	help();
    }

    if (mySocket->Connect()) {
    	mySocket->SendDataOnly(&fnum, sizeof(fnum));
    	mySocket->SendDataOnly(arg,sizeof(arg));
    	mySocket->ReceiveDataOnly(&ret, sizeof(ret));
    	if (ret != slsReceiverDefs::FAIL) {
    		mySocket->ReceiveDataOnly(&retval, sizeof(retval));
    	} else {
    		mySocket->ReceiveDataOnly(mess,sizeof(mess));
    		printf("Detector returned Error: %s",mess);
    	}
    	mySocket->Disconnect();
    } else
    	cerr << "could not connect to " << argv[1] << endl;

	if (idx <= 100) {
		printf("%.2f°C\n",(double)retval/1000.00);
	}else
		printf("%dmV\n",retval);

    return EXIT_SUCCESS;
}
