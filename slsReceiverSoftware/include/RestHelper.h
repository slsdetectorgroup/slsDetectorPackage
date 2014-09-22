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

//#include "logger.h"

#include <iostream>
#include <sstream>
#include <string>
#include <exception>



using namespace Poco::Net;
using namespace Poco;
using namespace std;

class RestHelper {
 public:

  RestHelper(int timeout=10, int n_tries=10){    
  /** 
   * 
   * 
   * @param timeout default=10
   * @param n_tries default=3
   */

    http_timeout = timeout;
    n_connection_tries = n_tries;
  }

  ~RestHelper(){};


  void set_connection_params(int timeout, int n_tries){
    http_timeout = timeout;
    n_connection_tries = n_tries;
  }


  void get_connection_params(int *timeout, int *n_tries){
    *timeout = http_timeout;
    *n_tries = n_connection_tries;
    
  }


  void init(string hostname, int port){
	  /** Initialize the RestHelper. Hostname and port parameters are not supposed to change.
	   *  
	   * 
	   * @param hostname FQDN of the host to connect to , e.g. www.iamfake.org, or sodoi.org
	   * @param port 
	   * 
	   * @return 
	   */

	  //Check for http:// string
	  FILE_LOG(logDEBUG) << __func__ << " starting";
	  string proto_str = "http://";
	  
	  if( size_t found = hostname.find(proto_str) != string::npos ){
		  cout << hostname << endl;

		  char c1[hostname.size()-found-1];
		  cout << c1 << endl;
		  size_t length1 = hostname.copy(c1, hostname.size()-found-1, proto_str.size());
	 	  c1[length1]='\0';
		  hostname = c1;
	  }
	  
	  full_hostname = "http://"+hostname;
	  session = new HTTPClientSession(hostname, port );
	  session->setKeepAliveTimeout( Timespan( http_timeout,0) );
  };


  void init(string hostname_port){
	  /** Initialize the RestHelper. Hostname_port parameters are not supposed to change.
	   *  
	   * 
	   * @param hostname FQDN and port of the host to connect to , e.g. www.iamfake.org:8080, or sodoi.org:1111. Default port is 8080
	   * 
	   * @return 
	   */
	  
	  //Check for http:// string
	  string proto_str = "http://";
	  if( size_t found = hostname_port.find(proto_str) != string::npos ){
		  char c1[hostname_port.size()-found-1];
		  size_t length1 = hostname_port.copy(c1, hostname_port.size()-found-1, proto_str.size());
	 	  c1[length1]='\0';
		  hostname_port = c1;
	  }

	  size_t found = hostname_port.rfind(":");
	  char c1[ found ], c2[hostname_port.size()-found-1];
	  string hostname;
	  size_t length1 = hostname_port.copy(c1, found);
	 
	  c1[length1]='\0';
	  hostname = c1;
	  size_t length2 = hostname_port.copy(c2, found-1, found+1);
	  c2[length2]='\0';
	  int port = atoi(c2);

	  full_hostname = proto_str+hostname;
	  session = new HTTPClientSession(hostname,port );
	  session->setKeepAliveTimeout( Timespan( http_timeout,0) );
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
    if(code == 0 ) {
	    FILE_LOG(logDEBUG) << "ANSWER " << answer;
	    json_value->loadFromString(answer);
    }
    delete uri;
    return code;
  };


  int post_json(string request, string *answer, string request_body="{}"){
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
    cout << "REQUEST BODY " << request_body << endl;
    req.setContentLength( request_body.length() );
    int code = send_request(session, req, answer, request_body);
    
    delete uri;
    return code;
  }


  int post_json(string request,  JsonBox::Value* json_value, string request_body="{}"){
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
    //this does not work
    //req.setContentType("application/json\r\n");
    //req.setContentLength( request.length() );
    string answer;
    int code = send_request(session, req, &answer, request_body);
    if(code==0){
	    json_value->loadFromString(answer);
    }
    delete uri;
    return code;
  }
    

 private:

  HTTPClientSession *session;
  string full_hostname;
  /// HTTP timeout in seconds, default is 8
  int http_timeout; 
  /// Number of connection tries
  int n_connection_tries;


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
    while(n<n_connection_tries){

      req.setContentType("application/json");
      //without this you need to tell the lenght: http://pocoproject.org/forum/viewtopic.php?f=12&t=5741&p=10019&hilit=post+json#p10019
      // request.setContentLength(my_string.length());
      req.setChunkedTransferEncoding(true);
      try {
	//istringstream rs(request_body);
	//req.read(rs);
	//cout << " --- " <<  rs << endl;
	if (request_body == "")
	  session->sendRequest( (req) );
	else{
	  cout << request_body << endl;
	  ostream &os = session->sendRequest( req ) ;
	  os << request_body;
	}
	
	HTTPResponse res;
	istream &is = session->receiveResponse(res);
	StreamCopier::copyToString(is, *answer);
	code = res.getStatus();
	if (code != 200){
	  cout << "HTTP ERROR " << res.getStatus() << ": " << res.getReason() << endl;
	  code = -1;
	}
	else
	  code = 0;
	return code;
      }
      catch (exception& e){
	      FILE_LOG(logERROR) << "Exception connecting to "<< full_hostname << ": "<< e.what() << ", sleeping 5 seconds (" << n << "/"<<n_connection_tries << ")";
	sleep(5);
      }
      n+=1;
    }

    return code;
  }

};
