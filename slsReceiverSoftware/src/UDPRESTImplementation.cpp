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
#include "logger.h"

//#include "utilities.h"
using namespace std;

/*
 TODO
 + filePath != getFilePath
 + better state handling. Now it is only IDLE - RUNNING - IDLE
 */


UDPRESTImplementation::UDPRESTImplementation(){
        FILE_LOG(logINFO) << "PID: " + __AT__ + " called";

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
  FILE_LOG(logINFO) << __AT__ << "configure called";

	map<string, string>::const_iterator pos;

	pos = config_map.find("rest_hostname");
	if (pos != config_map.end() ){
	  rest_hostname = config_map["rest_hostname"];
	  /*
	  string host_port_str = pos->second;
		std::size_t pos = host_port_str.find(":");      // position of "live" in str
		if(pos != string::npos){
			istringstream (host_port_str.substr (pos)) >> rest_port;
			rest_hostname = host_port_str.substr(0, pos);
			std::cout << "YEEEEEEEEEEEEEEEE" << rest_hostname << " " << rest_port << std::endl;

			}	*/
	  FILE_LOG(logINFO) << "REST hostname " << rest_hostname <<  std::endl;
	  
	}

	//initialize_REST();

	/*
	for(map<string, string>::const_iterator i=config_map.begin(); i != config_map.end(); i++){
		std::cout << i->first << " " << i->second<< std::endl;
	}
	 */

}


string UDPRESTImplementation::get_rest_state(RestHelper * rest/*, string *rest_state*/){

	JsonBox::Value answer;
	string rest_state = "";
	
	int code = rest->get_json("api/v1/state", &answer);
	if ( code != -1 ){ 
	  //rest_state = answer["global_state"].getString();
	  rest_state = answer["state"]["status"].getString();
	}
	//rest_state = *prs;
	std::cout << "REST STATE " << rest_state << std::endl;

	return rest_state;
}


void UDPRESTImplementation::initialize_REST(){

  FILE_LOG(logDEBUG1) << __AT__ << " called";
  FILE_LOG(logINFO) << __AT__ << " REST status is initialized: " + std::string(isInitialized ? "True" : "False");

	
	string rest_state = "";
	std::string answer = "";

	/*
	 * HORRIBLE FIX to get the main receiver
	 * TODO: use detID (from baseclass)
	 *  it i set by the client before calling initialize()
	 */
	string filename = getFileName();
	int code;


	//if (filename.substr(filename.length() - 2) == "d0"){
	if(detID == 0){
	  is_main_receiver = true;
	}
	
	if (rest_hostname.empty()) {
	  FILE_LOG(logWARNING) << __AT__ << "can't initialize with empty string or NULL for detectorHostname";
	  throw;
	} 
	rest = new RestHelper() ;
	//std::cout << rest_hostname << " - " << rest_port << std::endl;
	rest->init(rest_hostname);
	rest->set_connection_params(1, 3);
	FILE_LOG(logINFO) << "REST init called";

	if (!is_main_receiver){
	  isInitialized = true;
	  status = slsReceiverDefs::IDLE;
	  return;
	}

	if (isInitialized == true) {
	  FILE_LOG(logWARNING) << "already initialized, can't initialize several times";
	} 
	else {
	  FILE_LOG(logINFO) << "with receiverHostName=" << rest_hostname;
	  
	  try{
	    rest_state = get_rest_state(rest);
	    
	    if (rest_state == ""){
	      FILE_LOG(logERROR) << " REST state returned: " << rest_state;
	      throw;
	    }
	    else{
	      isInitialized = true;
	      status = slsReceiverDefs::IDLE;
	    }
              FILE_LOG(logDEBUG1) << "Answer: " <<  answer;
	  }
	  catch(std::string e){
	    FILE_LOG(logERROR) << __AT__ << ": " << e;
	    throw;
	  }

	  // HORRIBLE FIX to get the main receiver
	  string filename = getFileName();
	  int code;
	  
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
	  
	  //code = rest->get_json("api/v1/state", &answer);
	  //FILE_LOG(logDEBUG1, __AT__ << " state got " + std::string(code) << " " + answer + "\n";
	  if (rest_state != "INITIALIZED"){
	    test =  "{\"path\":\"" + string( getFilePath() ) + "\", \"n_frames\":10}";
	    code = rest->post_json("api/v1/initialize", &answer, test);
	  }
	  else{
	    test =  "{\"path\":\"" + string( getFilePath() ) + "\"}";
	    test =  "{\"path\":\"" + string( getFilePath() ) + "\", \"n_frames\":10}";
	    code = rest->post_json("api/v1/configure", &answer, test);
	  }
	  //FILE_LOG(logDEBUG1) << " state/configure got " + std::string(code);
	  
	  rest_state = get_rest_state(rest);
	  
	  FILE_LOG(logINFO) << " state got " + std::string(rest_state) << "\n";
	  
	  
	  /*
	    std::std::cout << string << std::endl; << "---- REST test 3: true, json object "<< std::endl;
	    JsonBox::Value json_value;
	      code = rest.get_json("status", &json_value);
	      std::cout << "JSON " << json_value["status"] << std::endl;
	  */
	}
	FILE_LOG(logDEBUG1) << ": configure() done";
}





/** acquisition functions */


int UDPRESTImplementation::startReceiver(char message[]){
	//int i;

	FILE_LOG(logINFO) << __AT__ << " starting";
	initialize_REST();
	FILE_LOG(logINFO) << __AT__ << " initialized";

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

	stringstream ss3;
	ss3 << acquisitionPeriod;
	string sAP = ss3.str();

	string rest_state = "";
	std::string request_body =  "{\"settings\": {\"bit_depth\": " + str_dr + ", \"n_frames\": " + str_n + ", \"period\": " + sAP + "}}";
	//std::string request_body =  "{\"settings\": {\"nimages\":1, \"scanid\":999, \"bit_depth\":16}}";
	if(is_main_receiver){
	  FILE_LOG(logDEBUG1) << " sending this configuration body: " << request_body;
	  code = rest->post_json("api/v1/configure", &answer, request_body);
	  code = rest->get_json("api/v1/state", &answer);
	  FILE_LOG(logDEBUG1) << " got: " << answer;
	  
	  rest_state = get_rest_state(rest);

	  code = rest->post_json("api/v1/open", &answer);
	}

	status = RUNNING;
	
	return OK;
}




void UDPRESTImplementation::stopReceiver(){

  FILE_LOG(logINFO) << "called";

	if(status == RUNNING)
		startReadout();

	/**
	 * while(status == TRANSMITTING)
	 * usleep(5000);
	 * This has been changed, you check if all the threads are done processing
	 *  and set the final status
	 */


	//change status
	status = IDLE;

	FILE_LOG(logDEBUG1) << "exited, status IDLE";

}





void UDPRESTImplementation::startReadout(){
  FILE_LOG(logINFO) << " starting";

  status = TRANSMITTING;
  
  //kill udp socket to tell the listening thread to push last packet
  shutDownUDPSockets();
  FILE_LOG(logDEBUG1) << " done";

}





/* FIXME
 * Its also called by TCP in case of illegal shut down such as Ctrl + c.
 * Upto you what you want to do with it.
 */
void UDPRESTImplementation::shutDownUDPSockets(){

  FILE_LOG(logDEBUG1) << "called";

	// this is just to be sure, it could be removed
	/*
	for(int i=0;i<numListeningThreads;i++){
		if(udpSocket[i]){
			FILE_LOG(logDEBUG1) << __AT__ << " closing UDP socket #" << i;
			udpSocket[i]->ShutDownSocket();
			delete udpSocket[i];
			udpSocket[i] = NULL;
		}
	}
	*/
	JsonBox::Value answer;
	int code;
	string rest_state = "";

	//FILE_LOG(logDEBUG1) << __AT__ << " numListeningThreads=" << numListeningThreads;
	if (rest == NULL){
	  FILE_LOG(logWARNING) << "No REST object initialized, closing...";
	  //return OK;
	}

	// getting the state
	if (is_main_receiver){
	  
	  FILE_LOG(logWARNING) << "PLEASE WAIT WHILE CHECKING AND SHUTTING DOWN ALL CONNECTIONS!";
	  rest_state = get_rest_state(rest);
	  std::cout << rest_state << std::endl;
	  while (rest_state != "OPEN"){
	    rest_state = get_rest_state(rest);
	    std::cout << rest_state << std::endl;
	    usleep(1000000);
	  }
	  //while (rest_state != "TRANSIENT"){
	  //  rest_state = get_rest_state(rest);
	  //  usleep(10000);
	  //}
	  
	  code = rest->post_json("api/v1/close", &answer);
	  rest_state = get_rest_state(rest);
	  std::cout << rest_state << std::endl;

	  while (rest_state != "CLOSED"){
	    rest_state = get_rest_state(rest);
	    std::cout << rest_state << std::endl;
	    usleep(1000000);
	  }
	  std::cout << "After close" << rest_state << std::endl;
	  code = rest->post_json("api/v1/reset", &answer);
	  
	  //rest_state = get_rest_state(rest);
	  std::cout << rest_state << std::endl;
	  std::cout << "After reset" << rest_state << std::endl;
	}
	status = slsReceiverDefs::RUN_FINISHED;
	
	//LEO: not sure it's needed
	//delete rest;

	FILE_LOG(logDEBUG1) << "finished";
	// Leo: how state is handled now?
	//return OK;
}



/* FIXME
 * do you really need this, this is called if registerDataCallback() is activated
 * in your gui to get data from receiver. you probably have a different way
 * of reconstructing complete data set from all receivers
 */
/*
void UDPRESTImplementation::readFrame(char* c,char** raw, uint64_t &startAcq, uint64_t &startFrame){
  FILE_LOG(logDEBUG1) << " called";
	strcpy(c,"");
	*raw = NULL;
>>>>>>> 3.0-rcrest
}
*/




/* FIXME
 * Its called by TCP in case of illegal shut down such as Ctrl + c.
 * Upto you what you want to do with it.
 */

// Leo: not in the base class
/*
void UDPRESTImplementation::closeFiles(){
  FILE_LOG(logDEBUG1) << "called for thread ";
  FILE_LOG(logDEBUG1) << "exited for thread ";
}
*/

uint64_t UDPRESTImplementation::getTotalFramesCaught() const{   
  FILE_LOG(logDEBUG1) << " starting";    
  return (0);
}



#endif
