
/**
 * @author Ian Johnson
 * @version 1.0
 * @developed for running Eiger at cSAXS
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <algorithm>    // std::remove_if

#include <iostream>
#include <string>
#include <map>
#include <time.h>
#include <string.h>

#include "FebControl.h"
#include "slsDetectorServer_defs.h" //include port number

using namespace std;

enum cmd_string {evNotFound,
	evReinitialize,evReset,

	evSetInputDelays,
	evSetDACValue,evGetDACValue,evSetDACVoltage,evGetDACVoltage,evSetHighVoltage,//evGetHighVoltage,

	evSetTrimBits,
	evSetAllTrimBits,
	evGetTrimBits,
	//evLoadTrimBitFile,

	evSetBitMode,
	evSetPhotonEnergy,
	//		 evSetPhotonEnergyCalibrationParameters,evActivateRateCorrection,evDeactivateRateCorrection,evSetRateCorrectionTau,

	evSetReadoutSpeed,evSetReadoutMode,

	//temp solution
	//		 evNotFound1,evNotFound2,evNotFound3,

	evSetNumberOfExposures,evSetExposureTime,evSetExposurePeriod,
	//		 evSetTriggerPolarityToPositive,evSetTriggerPolarityToNegative,
	evSetTriggerMode,
	evSetExternalGating,
	evStartAcquisition,evStopAcquisition,evIsDaqStillRunning,
	evWaitUntilDaqFinished,
	evExitServer
};

map<string, cmd_string> enum_map;

void init(){

	enum_map["reinitialize"]          = evReinitialize;
	enum_map["reset"]                 = evReset;
	enum_map["setinputdelays"]        = evSetInputDelays;
	enum_map["setdacvalue"]           = evSetDACValue;
	enum_map["getdacvalue"]           = evGetDACValue;
	enum_map["setdacvoltage"]         = evSetDACVoltage;
	enum_map["getdacvoltage"]         = evGetDACVoltage;
	enum_map["sethighvoltage"]        = evSetHighVoltage;
	enum_map["settrimbits"]           = evSetTrimBits;
	enum_map["setalltrimbits"]        = evSetAllTrimBits;
	enum_map["gettrimbits"]           = evGetTrimBits;
	//  enum_map["loadtrimbitfile"]        = evLoadTrimBitFile;
	enum_map["setbitmode"]            = evSetBitMode;
	enum_map["setphotonenergy"]       = evSetPhotonEnergy;
	//  enum_map["setphotonenergycalibrationparameters"] = evSetPhotonEnergyCalibrationParameters;
	//  enum_map["activateratecorrection"]    = evActivateRateCorrection;
	//  enum_map["deactivateratecorrection"]  = evDeactivateRateCorrection;
	//  enum_map["setratecorrectiontau"]      = evSetRateCorrectionTau;
	enum_map["setreadoutspeed"]        = evSetReadoutSpeed;
	enum_map["setreadoutmode"]         = evSetReadoutMode;
	enum_map["setnumberofexposures"]   = evSetNumberOfExposures;
	enum_map["setexposuretime"]        = evSetExposureTime;
	enum_map["setexposureperiod"]      = evSetExposurePeriod;
	// enum_map["settriggerpolaritytopositive"] = evSetTriggerPolarityToPositive;
	// enum_map["settriggerpolaritytonegative"] = evSetTriggerPolarityToNegative;
	enum_map["settriggermode"]         = evSetTriggerMode;
	enum_map["setexternalgating"] 	   = evSetExternalGating;
	enum_map["startacquisition"]       = evStartAcquisition;
	enum_map["stopacquisition"]        = evStopAcquisition;
	enum_map["isdaqstillrunning"]      = evIsDaqStillRunning;
	enum_map["waituntildaqfinished"]   = evWaitUntilDaqFinished;
	enum_map["exitserver"]             = evExitServer;
}

int server_list_s;
int server_conn_s;
int AccpetConnectionAndWaitForData(char* buffer, int maxlength);
bool WriteNClose(const char* buffer, int length);
bool SetupListenSocket(unsigned short int port);


string LowerCase(string str);
string GetNextString(string str,bool start_from_beginning=0);
void AddNumber(string& str, int n, int location=-1, bool space_after=0);//-1 means append
void AddNumber(string& str, float v, int location=-1, bool space_after=0);//-1 means append

int main(int argc, char* argv[]){
	cout<<endl<<endl;

	/*
  if(argc<2){
    cout<<"Usage: feb_server port_number"<<endl<<endl;
    return 1;
  }
	 */

	init();

	FebControl *feb_controler = new FebControl();

	unsigned short int port_number = FEB_PORT;
	if(!SetupListenSocket(port_number)) return 1;


	int  length=270000;
	char data[270000];

	int stop = 0;
	time_t rawtime;
	struct tm *timeinfo;


	while(!stop){

		/*cout<<"Waiting for command -> "<<flush;*/
		int nread = AccpetConnectionAndWaitForData(data,length);

		if(nread<=0) return 0;

		time(&rawtime); timeinfo=localtime(&rawtime);
		cout<<asctime(timeinfo);
		/*cout<<"  Command received: "<<data<<endl;*/



		string tmp_str[5];
		float v[4];//,v2,v3,v4,v5;
		int n[5];



		string cmd  = GetNextString(data,1);
		int    ret_val = 1;

		string return_message = "";/*\n\n\tCommand recieved: ";
		return_message.append(data);
		return_message.append("\n");
*/
		int return_start_pos;
		while(cmd.length()>0){
			int ret_parameter = 0;
			return_start_pos = return_message.length();

			switch(enum_map.find(LowerCase(cmd))->second){

			case evReinitialize :
				if(feb_controler->Init()){
					return_message.append("\tExecuted: Reinitialize\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: Reinitialize\n");
					ret_val = 1;
				}
				break;

			case evReset :
				if(feb_controler->Reset()){
					return_message.append("\tExecuted: Reset\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: Reset\n");
					ret_val = 1;
				}
				break;



			case evSetInputDelays :
				tmp_str[0] = GetNextString(data);
				n[0] = atoi(tmp_str[0].data());

				if(tmp_str[0].length()>0&&feb_controler->SetIDelays(0,n[0])){
					return_message.append("\tExecuted:  SetInputDelays "); AddNumber(return_message,n[0]); return_message.append("\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: SetInputDelays <delay>\n");
					ret_val = 1;
				}
				break;



			case evSetDACValue :
				tmp_str[0] = GetNextString(data);
				tmp_str[1] = GetNextString(data);
				n[0] = atoi(tmp_str[1].data());

				if(tmp_str[0].length()>0&&tmp_str[1].length()>0&&feb_controler->SetDAC(tmp_str[0],n[0])){
					return_message.append("\tExecuted:  SetDACValue "); return_message.append(tmp_str[0]+" "); AddNumber(return_message,n[0]); return_message.append("\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: SetDACValue <dac_name> <value>\n");
					ret_val = 1;
				}
				break;

			case evGetDACValue :
				tmp_str[0] = GetNextString(data);

				if(tmp_str[0].length()>0&&feb_controler->GetDAC(tmp_str[0],ret_parameter)){
					return_message.append("\tExecuted:  GetDACValue "); return_message.append(tmp_str[0]+"  ->  ");AddNumber(return_message,ret_parameter); return_message.append("\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: GetDACValue <dac_name>\n");
					ret_val = 1;
				}
				break;

			case evSetDACVoltage :
				tmp_str[0] = GetNextString(data);
				tmp_str[1] = GetNextString(data);
				n[0] = atoi(tmp_str[1].data());

				if(tmp_str[0].length()>0&&tmp_str[1].length()>0&&feb_controler->SetDAC(tmp_str[0],n[0],1)){
					return_message.append("\tExecuted:  SetDACVoltage "); return_message.append(tmp_str[0]+" "); AddNumber(return_message,n[0]); return_message.append("\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: SetDACVoltage <dac_name> <voltage_mV>\n");
					ret_val = 1;
				}
				break;

			case evGetDACVoltage :
				tmp_str[0] = GetNextString(data);

				if(tmp_str[0].length()>0&&feb_controler->GetDAC(tmp_str[0],ret_parameter,1)){
					return_message.append("\tExecuted:  GetDACVoltage "); return_message.append(tmp_str[0]+"  ->  ");AddNumber(return_message,ret_parameter); return_message.append(" mV\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: GetDACVoltage <dac_name>\n");
					ret_val = 1;
				}
				break;

			case evSetHighVoltage :
				tmp_str[0] = GetNextString(data);
				v[0] = atof(tmp_str[0].data());

				if(tmp_str[0].length()>0&&feb_controler->SetHighVoltage(v[0])){
					return_message.append("\tExecuted:  SetHighVoltage "); AddNumber(return_message,v[0]); return_message.append("\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: SetHighVoltage <voltage>\n");
					ret_val = 1;
				}
				break;

			case evSetTrimBits :
				tmp_str[0] = GetNextString(data);
				if(feb_controler->LoadTrimbitFile()){
			/*	if(1){*/
					/*tmp_str[0] = GetNextString(data);
					feb_controler->SetTrimbits(0,(unsigned char*)(tmp_str[0].c_str()));*/
					return_message.append("\tExecuted:  SetTrimBits\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: SetTrimBits \n");
					ret_val = 1;
				}
				break;

			case evSetAllTrimBits :
				tmp_str[0] = GetNextString(data);
				n[0] = atoi(tmp_str[0].data());
				if(feb_controler->SaveAllTrimbitsTo(n[0])){
					/*feb_controler->SetTrimbits(0,(unsigned char*)(tmp_str[0].c_str()));*/
				/*if(1){*/
					return_message.append("\tExecuted:  SetAllTrimBits\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: SetAllTrimBits \n");
					ret_val = 1;
				}
				break;


			case evGetTrimBits :
				if(feb_controler->SaveTrimbitFile()){
				/*if(1){*/
					/*tmp_str[0] = GetNextString(data);
					feb_controler->GetTrimbits();*/
					return_message.append("\tExecuted:  GetTrimBits\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: GetTrimBits \n");
					ret_val = 1;
				}
				break;


				//      case evLoadTrimBitFile :

			case evSetBitMode :
				tmp_str[0] = GetNextString(data);
				n[0] = atoi(tmp_str[0].data());

				if(tmp_str[0].length()>0&&feb_controler->SetDynamicRange(n[0])){
					return_message.append("\tExecuted:  SetBitMode "); AddNumber(return_message,n[0]); return_message.append("\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: SetBitMode <mode 4,8,16,32>\n");
					ret_val = 1;
				}
				break;

			case evSetPhotonEnergy :
				tmp_str[0] = GetNextString(data);
				n[0] = atoi(tmp_str[0].data());
				if(tmp_str[0].length()>0&&feb_controler->SetPhotonEnergy(n[0])){
					return_message.append("\tExecuted:  SetPhotonEnergy "); AddNumber(return_message,n[0]); return_message.append("\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: SetPhotonEnergy <energy eV>\n");
					ret_val = 1;
				}
				break;

				//      case evSetPhotonEnergyCalibrationParameters :
				//      case evActivateRateCorrection :
				//      case evDeactivateRateCorrection :
				//      case evSetRateCorrectionTau :


			case evSetReadoutSpeed :
				tmp_str[0] = GetNextString(data);
				n[0] = atoi(tmp_str[0].data());
				if(tmp_str[0].length()>0&&feb_controler->SetReadoutSpeed(n[0])){
					return_message.append("\tExecuted:  SetReadoutSpeed "); AddNumber(return_message,n[0]); return_message.append("\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: SetReadoutSpeed <speed 0-full 1-half 2-quarter 3-super_slow>\n");
					ret_val = 1;
				}
				break;

			case evSetReadoutMode :
				tmp_str[0] = GetNextString(data);
				n[0] = atoi(tmp_str[0].data());
				if(tmp_str[0].length()>0&&feb_controler->SetReadoutMode(n[0])){
					return_message.append("\tExecuted:  SetReadoutMode "); AddNumber(return_message,n[0]); return_message.append("\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: SetReadoutMode <mode 0->parallel,1->non-parallel,2-> safe_mode>\n");
					ret_val = 1;
				}
				break;

			case evSetNumberOfExposures :
				tmp_str[0] = GetNextString(data);
				n[0] = atoi(tmp_str[0].data());
				if(tmp_str[0].length()>0&&feb_controler->SetNExposures(n[0])){
					return_message.append("\tExecuted:  SetNumberOfExposures "); AddNumber(return_message,n[0]); return_message.append("\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: SetNumberOfExposures <n>\n");
					ret_val = 1;
				}
				break;

			case evSetExposureTime :
				tmp_str[0] = GetNextString(data);
				v[0] = atof(tmp_str[0].data());
				if(tmp_str[0].length()>0&&feb_controler->SetExposureTime(v[0])){
					return_message.append("\tExecuted:  SetExposureTime "); AddNumber(return_message,v[0]); return_message.append("\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: SetExposureTime <t_seconds>\n");
					ret_val = 1;
				}
				break;

			case evSetExposurePeriod :
				tmp_str[0] = GetNextString(data);
				v[0] = atof(tmp_str[0].data());
				if(tmp_str[0].length()>0&&feb_controler->SetExposurePeriod(v[0])){
					return_message.append("\tExecuted:  SetExposurePeriod "); AddNumber(return_message,v[0]); return_message.append("\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: SetExposurePeriod <t_seconds>\n");
					ret_val = 1;
				}
				break;
				//  case evSetTriggerPolarityToPositive :
				//  case evSetTriggerPolarityToNegative :
			case evSetTriggerMode :
				tmp_str[0] = GetNextString(data);
				n[0] = atoi(tmp_str[0].data());
				if(tmp_str[0].length()>0&&feb_controler->SetTriggerMode(n[0])){
					return_message.append("\tExecuted:  SetTriggerMode "); AddNumber(return_message,n[0]); return_message.append("\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: SetTriggerMode <n>\n");
					ret_val = 1;
				}
				break;

			case evSetExternalGating :
				tmp_str[0] = GetNextString(data);
				tmp_str[1] = GetNextString(data);
				n[0] = atoi(tmp_str[0].data());
				n[1] = atoi(tmp_str[1].data());
				if(tmp_str[0].length()<1 || tmp_str[1].length()<1 || (n[0]!=0&&n[0]!=1) || (n[1]!=0&&n[1]!=1)){
					return_message.append("\tError executing: setexternalgating <enable> <polarity>\n");
					ret_val = 1;
				}
				feb_controler->SetExternalEnableMode(n[0],n[1]);
				ret_val = 0;
				break;

			case evStartAcquisition :
				if(feb_controler->StartAcquisition()){
					return_message.append("\tExecuted: StartAcquisition\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: StartAcquisition\n");
					ret_val = 1;
				}
				break;

			case evStopAcquisition :
				if(feb_controler->StopAcquisition()){
					return_message.append("\tExecuted: StopAcquisition\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: StopAcquisition\n");
					ret_val = 1;
				}
				break;


			case evIsDaqStillRunning :
				return_message.append("\tExecuted: evIsDaqStillRunning\n");
				ret_parameter = feb_controler->AcquisitionInProgress();
				ret_val = 0;
				break;


			case evWaitUntilDaqFinished :
				if(feb_controler->WaitForFinishedFlag()){
					return_message.append("\tExecuted: WaitUntilDaqFinished\n");
					ret_val = 0;
				}else{
					return_message.append("\tError executing: WaitUntilDaqFinished\n");
					ret_val = 1;
				}
				break;


			case evExitServer :
				return_message.append("\tExiting Server ....\n");
				stop = 1;
				ret_val = -200;
				break;


			default :
				return_message.append("\tWarning command \"");
				return_message.append(cmd);
				return_message.append("\" not found.\n");
				return_message.append("\t\tValid commands: ");
				map<string, cmd_string>::iterator it = enum_map.begin();
				while(it!=enum_map.end()){
					return_message.append((it++)->first);
					return_message.append(" ");
				}

				ret_val=-100;
				break;
			}

			//      return_message.append("\n");
			//AddNumber(return_message,ret_parameter,return_start_pos);
			AddNumber(return_message,ret_val,return_start_pos,1);
			AddNumber(return_message,ret_parameter,0,1);
			if(ret_val!=0) break;

			cmd = GetNextString(data);
		}
		/*return_message.append("\n\n\n");*/

		AddNumber(return_message,ret_val,0,1);
		cout<<return_message.c_str()<<"\t\t";
		cout<<"return: "<<ret_val<<endl;

		if(!WriteNClose(return_message.c_str(),return_message.length())) return 0;
	}


	delete feb_controler;

	return 0;
}

string LowerCase(string str){
	string s = str;
	string::iterator i = s.begin();
	while(i!=s.end()) *i=tolower(*(i++));
	return s;
}

string GetNextString(string str,bool start_from_beginning){
	static string::size_type start_pos = 0;
	if(start_from_beginning) start_pos = 0;

	while(start_pos != string::npos){
		string::size_type found = str.find_first_of(" ",start_pos);
		string sub = str.substr(start_pos,found-start_pos);

		start_pos = found;
		if(start_pos != string::npos) start_pos+=1;

		sub.erase(remove_if(sub.begin(),sub.end(), ::isspace ),sub.end());

		if(sub.length()>0) return sub;
	}

	return "";
}


void AddNumber(string& str, int n, int location, bool space_after){
	static char retval_st[100];
	if(space_after) sprintf(retval_st,"%d ",n);
	else            sprintf(retval_st,"%d",n);

	if(location<0) str.append(retval_st);
	else           str.insert(location,retval_st);
}

void AddNumber(string& str, float v, int location, bool space_after){
	static char retval_st[100];
	if(space_after) sprintf(retval_st,"%f ",v);
	else            sprintf(retval_st,"%f",v);

	if(location<0) str.append(retval_st);
	else           str.insert(location,retval_st);
}


bool SetupListenSocket(unsigned short int port){
	server_list_s=0;
	server_conn_s=0;

	if((server_list_s = socket(AF_INET, SOCK_STREAM, 0))<0) return 0;

	struct sockaddr_in servaddr;  /*  socket address structure  */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);

	if(bind(server_list_s,(struct sockaddr *) &servaddr,sizeof(servaddr))<0) return 0;

	if(listen(server_list_s,32) < 0){      // 1024   /*  Backlog for listen()   */
		return 0;
	}

	return 1;
}


int AccpetConnectionAndWaitForData(char* buffer, int maxlength){
	if(server_list_s==0||maxlength<=0) return 0;

	if((server_conn_s = accept(server_list_s,NULL,NULL))< 0) return 0;

	int nread = read(server_conn_s,buffer,maxlength-1);

	if(nread<0) return 0;

	buffer[nread]='\0';
	return nread;
}

bool WriteNClose(const char* buffer, int length){
	if(server_conn_s==0||length<=0) return 0;

	int nsent = write(server_conn_s,buffer,length);
	if(close(server_conn_s)<0) return 0;

	server_conn_s=0;
	return (nsent==length);
}



