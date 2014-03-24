#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>

#include "JsonBox/Value.h"

#include <iostream>
#include <string>

using namespace Poco::Net;
using namespace Poco;


class RestHelper {
 public:

  void init(std::string hostname, int port){
    /*
      
     */
    full_hostname = "http://"+hostname;
    std::cout << full_hostname << std::endl;
    session = new HTTPClientSession(hostname,port );
  };

  int get_json(std::string request, std::string* answer){
    //TODO: implement a timeout and max_retries

    uri = new URI(full_hostname+"/"+request);
    std::string path(uri->getPathAndQuery());
    if (path.empty()) path = "/";

    // send request
    HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
    session->sendRequest(req);

    // get response
    HTTPResponse res;
    //std::cout << res.getStatus() << " " << res.getReason() << std::endl;
    std::istream &is = session->receiveResponse(res);
    StreamCopier::copyToString(is, *answer);

    return res.getStatus();
  };

  int get_json(std::string request, JsonBox::Value* json_value){
    //TODO: implement a timeout and max_retries

    uri = new URI(full_hostname+"/"+request);
    std::string path(uri->getPathAndQuery());
    if (path.empty()) path = "/";

    // send request
    HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
    session->sendRequest(req);

    // get response
    HTTPResponse res;
    //std::cout << res.getStatus() << " " << res.getReason() << std::endl;
    std::string answer;
    std::istream &is = session->receiveResponse(res);
    StreamCopier::copyToString(is, answer);

    std::cout << answer << std::endl;
    //returning a Json struct
    json_value->loadFromString(answer);

    return res.getStatus();
  };

 private:
  URI * uri;
  HTTPClientSession *session;
  std::string full_hostname;
};
