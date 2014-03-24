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
#define N_CONNECTION_TRIES 0

using namespace Poco::Net;
using namespace Poco;
using namespace std;

class RestHelper {
 public:

  void init(string hostname, int port){
    /*
      
     */
    full_hostname = "http://"+hostname;
    cout << full_hostname << endl;
    session = new HTTPClientSession(hostname,port );
    session->setKeepAliveTimeout( Timespan( HTTP_TIMEOUT,0) );
  };

  int get_json(string request, string* answer){
    //TODO: implement a timeout and max_retries

    uri = new URI(full_hostname+"/"+request);
    string path(uri->getPathAndQuery());
    if (path.empty()) path = "/";

    // send request
    //HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
    req = new HTTPRequest(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
    
    try {
      session->sendRequest(*req);
    }
    catch (exception& e){
      cout << "Exception:"<< e.what() << '\n';
    }
    
    int code = send_request(session, req);
    
    // get response
    //if (code!=0){
      HTTPResponse res;
      cout << res.getStatus() << " " << res.getReason() << endl;
      istream &is = session->receiveResponse(res);
      StreamCopier::copyToString(is, *answer);
    
      return res.getStatus();
      //}
      //else
      //return code;
  };

  int get_json(string request, JsonBox::Value* json_value){
    //TODO: implement a timeout and max_retries

    uri = new URI(full_hostname+"/"+request);
    string path(uri->getPathAndQuery());
    if (path.empty()) path = "/";

    // send request
    HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
    session->sendRequest(req);

    // get response
    HTTPResponse res;
    //cout << res.getStatus() << " " << res.getReason() << endl;
    string answer;
    istream &is = session->receiveResponse(res);
    StreamCopier::copyToString(is, answer);

    cout << answer << endl;
    //returning a Json struct
    json_value->loadFromString(answer);

    return res.getStatus();
  };

 private:
  URI * uri;
  HTTPClientSession *session;
  HTTPRequest *req;

  string full_hostname;

  int send_request(HTTPClientSession *session, HTTPRequest *req){
    int n=0;
    int code = -1;
    while(n<N_CONNECTION_TRIES){
      try {
	session->sendRequest( (*req) );
	code = 0;
      }
      catch (exception& e){
	cout << "Exception:"<< e.what() << ", sleeping " << HTTP_TIMEOUT << " seconds\n";
	sleep(HTTP_TIMEOUT);
      }
      n+=1;
    }

    return code;
  }

};
