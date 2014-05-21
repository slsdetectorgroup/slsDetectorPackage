/**
 * @file   RestHelper.h
 * @author Leonardo Sala <leonardo.sala@psi.ch>
 * @date   Tue Mar 25 09:28:19 2014
 * 
 * @brief  
 * 
 * 
 */

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

/// HTTP timeout in seconds, default is 8
#define HTTP_TIMEOUT 10
/// Number of connection tries
#define N_CONNECTION_TRIES 3

using namespace Poco::Net;
using namespace Poco;
using namespace std;

class RestHelper {
 public:

  ~RestHelper(){};

  void init(string hostname, int port){
    /** Initialize the RestHelper. Hostname and port parameters are not supposed to change.
     *  
     * 
     * @param hostname FQDN of the host to connect to , e.g. www.iamfake.org, or sodoi.org
     * @param port 
     * 
     * @return 
     */

    full_hostname = "http://"+hostname;
    session = new HTTPClientSession(hostname,port );
    session->setKeepAliveTimeout( Timespan( HTTP_TIMEOUT,0) );
  };


  int get_json(string request, string* answer){
    /** Retrieves a reply from the RESTful webservice. 
     * 
     * 
     * @param request Request without the hostname, e.g. if the full request would have been http://fake.org/fakemethod, request=fakemethod
     * @param answer 
     * 
     * @return 0 if successful, -1 if failure happens.
     */    
    URI * uri = new URI(full_hostname+"/"+request);
    string path(uri->getPathAndQuery());
    if (path.empty()) path = "/";

    // send request
    HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
    req.setContentType("application/json\r\n");
    int code = send_request(session, req, answer);
    delete uri;
    return code;
  };


  int get_json(string request,  JsonBox::Value* json_value){
    /** 
     * 
     * 
     * @param request 
     * @param json_value 
     * 
     * @return 
     */    
    URI *uri = new URI(full_hostname+"/"+request);
    string path(uri->getPathAndQuery());
    if (path.empty()) path = "/";
    // send request
    HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
    req.setContentType("application/json\r\n");
    string answer;
    int code = send_request(session, req, &answer);
    json_value->loadFromString(answer);
    delete uri;
    return code;
  };


  int post_json(string request, string *answer, string request_body=""){
    /** 
     * 
     * 
     * @param request 
     * @param answer 
     * @param request_body Eventual arguments to the URL, e.g. action=login&name=mammamia
     * 
     * @return 
     */
    //from: http://stackoverflow.com/questions/1499086/poco-c-net-ssl-how-to-post-https-request
    URI *uri = new URI(full_hostname+"/"+request);
    string path(uri->getPathAndQuery());
    if (path.empty()) path = "/";
    HTTPRequest req(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1 );
    req.setContentType("application/json\r\n");
    req.setContentLength( request.length() );
    int code = send_request(session, req, answer, request_body);
    delete uri;
    return code;
  }


  int post_json(string request,  JsonBox::Value* json_value, string request_body=""){
    /** 
     * 
     * 
     * @param request 
     * @param json_value 
     * @param request_body Eventual arguments to the URL, e.g. action=login&name=mammamia
     * 
     * @return 
     */

    URI *uri = new URI(full_hostname+"/"+request);
    string path(uri->getPathAndQuery());
    if (path.empty()) path = "/";
    HTTPRequest req(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1 );
    req.setContentType("application/json\r\n");
    req.setContentLength( request.length() );
    string answer;
    int code = send_request(session, req, &answer, request_body);
    json_value->loadFromString(answer);
    delete uri;
    return code;
  }
    

 private:
  //URI * uri;
  HTTPClientSession *session;
  string full_hostname;

  int send_request(HTTPClientSession *session, HTTPRequest &req, string *answer, string request_body=""){
    /** 
     * 
     * 
     * @param session 
     * @param req 
     * @param answer 
     * @param request_body 
     * 
     * @return 
     */
    
    int n=0;
    int code = -1;
    while(n<N_CONNECTION_TRIES){
      try {
	if (request_body == "")
	  session->sendRequest( (req) );
	else
	  session->sendRequest( (req) ) << request_body;

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
