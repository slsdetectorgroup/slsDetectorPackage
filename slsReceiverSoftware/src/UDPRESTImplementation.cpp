#ifdef SLS_RECEIVER_UDP_FUNCTIONS
/********************************************//**
 * @file UDPRESTImplementation.cpp
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/

#include "UDPRESTImplementation.h"

#include <stdlib.h>		// exit()
#include <iomanip>		// set precision
#include <map>			// map
#include <iostream>
#include <string.h>
#include <stdint.h>
#include <sstream>
//#include "utilities.h"
using namespace std;

/*
 TODO
 + filePath != getFilePath
 + better state handling. Now it is only IDLE - RUNNING - IDLE
 */


UDPRESTImplementation::UDPRESTImplementation(){
	FILE_LOG(logDEBUG) <<  "PID: " << getpid() << __AT__ << " called";

	//TODO I do not really know what to do with bottom...
	// Default values
	isInitialized = true;
	rest = NULL;
	rest_hostname = "localhost";
	rest_port = 8080;
	is_main_receiver = false;
}


UDPRESTImplementation::~UDPRESTImplementation(){
	delete rest;
}


void UDPRESTImplementation::configure(map<string, string> config_map){
  // am I ever getting there?
	FILE_LOG(logWARNING) << __AT__ << " called";

	map<string, string>::const_iterator pos;

	pos = config_map.find("rest_hostname");
	if (pos != config_map.end() ){
		string host_port_str = pos->second;
		std::size_t pos = host_port_str.find(":");      // position of "live" in str
		if(pos != string::npos){
			istringstream (host_port_str.substr (pos)) >> rest_port;
			rest_hostname = host_port_str.substr(0, pos);
		}	
	}

	/*
	for(map<string, string>::const_iterator i=config_map.begin(); i != config_map.end(); i++){
		std::cout << i->first << " " << i->second<< std::endl;
	}
	 */

}


string UDPRESTImplementation::get_rest_state(RestHelper * rest/*, string *rest_state*/){

	JsonBox::Value answer;
	string rest_state = "";
	
	int code = rest->get_json("v1/state", &answer);
	if ( code != -1 ){ 
	  rest_state = answer["global_state"].getString();
	}
	//rest_state = *prs;

	return rest_state;
}


void UDPRESTImplementation::initialize_REST(){

	FILE_LOG(logDEBUG) << __AT__ << " called";
	FILE_LOG(logDEBUG) << __AT__ << " REST status is initialized: " << isInitialized;

	
	string rest_state = "";
	std::string answer = "";

	// HORRIBLE FIX to get the main receiver
	string filename = getFileName();
	if (filename.substr(filename.length() - 2) == "d0"){
	  is_main_receiver = true;
	}
	
	if (rest_hostname.empty()) {
	  FILE_LOG(logDEBUG) << __AT__ <<"can't initialize with empty string or NULL for detectorHostname";
	  throw;
	} 
	rest = new RestHelper() ;
	rest->init(rest_hostname, rest_port);
	rest->set_connection_params(1, 3);
	
	int code;

	if (!is_main_receiver){
	  isInitialized = true;
	  status = slsReceiverDefs::IDLE;
	  return;
	}

	if (isInitialized == true) {
	  FILE_LOG(logDEBUG) << __AT__ << "already initialized, can't initialize several times";
	} 
	else {
	  FILE_LOG(logDEBUG) << __AT__ << "with receiverHostName=" << rest_hostname << ":" << rest_port;
	  
	  try{
	    rest_state = get_rest_state(rest);
	    
	    if (rest_state == ""){
	      FILE_LOG(logERROR) << __AT__ << " REST state returned: " << rest_state;
	      throw;
	    }
	    else{
	      isInitialized = true;
	      status = slsReceiverDefs::IDLE;
	    }
	    FILE_LOG(logDEBUG) << __func__ << "Answer: " <<  answer;
	  }
	  catch(std::string e){
	    FILE_LOG(logERROR) <<  __func__ << ": " << e;
	    throw;
	  }
	  
	  
	  //JsonBox::Object json_object;
	  //json_object["configfile"] = JsonBox::Value("FILENAME");
	  JsonBox::Value json_request;
	  //json_request["configfile"] = "config.py";
	  json_request["path"] = filePath;
	  
	  stringstream ss;
	  string test;
	  //std::cout << "GetSTring: " << json_request << std::endl;
	  json_request.writeToStream(ss, false);
	  //ss << json_request;
	  ss >> test;
	  
	  rest_state = get_rest_state(rest);
	  
	  //code = rest->get_json("v1/state", &answer);
	  FILE_LOG(logDEBUG) << __AT__ << " state got " << code << " " << answer << "\n";
	  if (rest_state != "INITIALIZED"){
	    test =  "{\"path\":\"" + string( getFilePath() ) + "\", \"n_frames\":10}";
	    code = rest->post_json("v1/state/initialize", &answer, test);
	  }
	  else{
	    test =  "{\"path\":\"" + string( getFilePath() ) + "\"}";
	    test =  "{\"path\":\"" + string( getFilePath() ) + "\", \"n_frames\":10}";
	    code = rest->post_json("v1/state/configure", &answer, test);
	  }
	  FILE_LOG(logDEBUG) << __AT__ << " state/configure got " << code;
	  
	  rest_state = get_rest_state(rest);
	  
	  FILE_LOG(logDEBUG) << __AT__ << " state got " << rest_state << "\n";
	  
	  
	  /*
	    std::std::cout << string << std::endl; << "---- REST test 3: true, json object "<< std::endl;
	    JsonBox::Value json_value;
	      code = rest.get_json("status", &json_value);
	      std::cout << "JSON " << json_value["status"] << std::endl;
	  */
	}
	FILE_LOG(logDEBUG) << __func__ << ": initialize() done";
}





/** acquisition functions */


int UDPRESTImplementation::startReceiver(char message[]){
	int i;

	FILE_LOG(logDEBUG) << __FILE__ << "::" << __func__ << " starting";
	initialize_REST();
	FILE_LOG(logDEBUG) << __FILE__ << "::" << __func__ << " initialized";

	std::string answer;
	int code;
	//char *intStr = itoa(a);
	//string str = string(intStr);
	// TODO: remove hardcode!!!
	stringstream ss;
	ss << getDynamicRange();
	string str_dr = ss.str();
	stringstream ss2;
	ss2 << getNumberOfFrames();
	string str_n = ss2.str();


	string rest_state = "";
	std::string request_body =  "{\"settings\": {\"bit_depth\": " + str_dr + ", \"n_frames\": " + str_n + "}}";
	//std::string request_body =  "{\"settings\": {\"nimages\":1, \"scanid\":999, \"bit_depth\":16}}";
	if(is_main_receiver){
	  FILE_LOG(logDEBUG) << __FILE__ << "::" << " sending this configuration body: " << request_body;
	  code = rest->post_json("v1/state/configure", &answer, request_body);
	  code = rest->get_json("v1/state", &answer);
	  FILE_LOG(logDEBUG) << __FILE__ << "::" << " got: " << answer;
	  
	  rest_state = get_rest_state(rest);

	  code = rest->post_json("v1/state/open", &answer);
	}

	status = RUNNING;
	
	return OK;
}




void UDPRESTImplementation::stopReceiver(){

	FILE_LOG(logDEBUG) << __AT__ << "called";

	if(status == RUNNING)
		startReadout();

	while(status == TRANSMITTING)
		usleep(5000);

	//change status
	status = IDLE;

	FILE_LOG(logDEBUG) << __AT__ << "exited, status " << endl;

}





void UDPRESTImplementation::startReadout(){
	FILE_LOG(logDEBUG) << __AT__ << " starting";

	status = TRANSMITTING;

	//kill udp socket to tell the listening thread to push last packet
	shutDownUDPSockets();
	FILE_LOG(logDEBUG) << __FILE__ << "::" << __func__ << " done";

}





/* FIXME
 * Its also called by TCP in case of illegal shut down such as Ctrl + c.
 * Upto you what you want to do with it.
 */
int UDPRESTImplementation::shutDownUDPSockets(){

	FILE_LOG(logDEBUG) << __AT__ << "called";

	// this is just to be sure, it could be removed
	/*
	for(int i=0;i<numListeningThreads;i++){
		if(udpSocket[i]){
			FILE_LOG(logDEBUG) << __AT__ << " closing UDP socket #" << i;
			udpSocket[i]->ShutDownSocket();
			delete udpSocket[i];
			udpSocket[i] = NULL;
		}
	}
	*/
	JsonBox::Value answer;
	int code;
	string rest_state = "";

	//FILE_LOG(logDEBUG) << __AT__ << " numListeningThreads=" << numListeningThreads;
	if (rest == NULL){
		FILE_LOG(logWARNING) << __AT__ << "No REST object initialized, closing...";
		return OK;
	}

	// getting the state
	if (is_main_receiver){
	  
	  FILE_LOG(logWARNING) << "PLEASE WAIT WHILE CHECKING AND SHUTTING DOWN ALL CONNECTIONS!";
	  rest_state = get_rest_state(rest);
	  
	  while (rest_state != "OPEN"){
	    rest_state = get_rest_state(rest);
	    usleep(10000);
	  }
	  //while (rest_state != "TRANSIENT"){
	  //  rest_state = get_rest_state(rest);
	  //  usleep(10000);
	  //}
	  
	  code = rest->post_json("v1/state/close", &answer);
	  code = rest->post_json("v1/state/reset", &answer);
	  
	  //rest_state = get_rest_state(rest);
	  //std::cout << rest_state << std::endl;
	}
	status = slsReceiverDefs::RUN_FINISHED;
	
	//LEO: not sure it's needed
	//delete rest;

	FILE_LOG(logDEBUG) << __AT__ << "finished";
	return OK;
}



/* FIXME
 * do you really need this, this is called if registerDataCallback() is activated
 * in your gui to get data from receiver. you probably have a different way
 * of reconstructing complete data set from all receivers
 */
void UDPRESTImplementation::readFrame(char* c,char** raw, uint64_t &startAcq, uint64_t &startFrame){
	FILE_LOG(logDEBUG) << __AT__ << " called";
	strcpy(c,"");
	*raw = NULL;
}





/* FIXME
 * Its called by TCP in case of illegal shut down such as Ctrl + c.
 * Upto you what you want to do with it.
 */
void UDPRESTImplementation::closeFile(int ithr){
	FILE_LOG(logDEBUG) << __AT__ << "called for thread " << ithr;
	FILE_LOG(logDEBUG) << __AT__ << "exited for thread " << ithr;
}


uint64_t UDPRESTImplementation::getTotalFramesCaught() const{   
  FILE_LOG(logDEBUG) << __AT__ << " starting";    
  return (0);
}



#endif
