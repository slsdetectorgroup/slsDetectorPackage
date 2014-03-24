#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <Poco/Timespan.h>

#include "JsonBox/Value.h"

#include <iostream>
#include <string>
#include <exception>

// HTTP timeout in seconds, default is 8
#define HTTP_TIMEOUT 10
// Number of connection tries
#define N_CONNECTION_TRIES 3

using namespace Poco::Net;
using namespace Poco;
using namespace std;

class RestHelper {
 public:

  ~RestHelper(){};

  void init(string hostname, int port){
    /*
      
     */
    full_hostname = "http://"+hostname;
    session = new HTTPClientSession(hostname,port );
    session->setKeepAliveTimeout( Timespan( HTTP_TIMEOUT,0) );
  };

  int get_json(string request, string* answer){
    /*

     */
    
    uri = new URI(full_hostname+"/"+request);
    string path(uri->getPathAndQuery());
    if (path.empty()) path = "/";

    // send request
    HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
    int code = send_request(session, req, answer);
    return code;
  };

  int get_json(string request,  JsonBox::Value* json_value){
    /*

     */
    
    uri = new URI(full_hostname+"/"+request);
    string path(uri->getPathAndQuery());
    if (path.empty()) path = "/";
    // send request
    HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
    string answer;
    int code = send_request(session, req, &answer);
    json_value->loadFromString(answer);
    return code;
  };


 private:
  URI * uri;
  HTTPClientSession *session;
  string full_hostname;

  int send_request(HTTPClientSession *session, HTTPRequest &req, string *answer){
    int n=0;
    int code = -1;
    while(n<N_CONNECTION_TRIES){
      try {
	session->sendRequest( (req) );
	HTTPResponse res;
	istream &is = session->receiveResponse(res);
	StreamCopier::copyToString(is, *answer);
	code = res.getStatus();
	if (code != 200){
	  cout << "HTTP ERROR " << res.getStatus() << ": " << res.getReason() << endl;
	  code = -1;
	}
	return code;
      }
      catch (exception& e){
	cout << "Exception connecting to "<< full_hostname << ": "<< e.what() << ", sleeping " << HTTP_TIMEOUT << " seconds\n";
	sleep(HTTP_TIMEOUT);
      }
      n+=1;
    }

    return code;
  }

};
