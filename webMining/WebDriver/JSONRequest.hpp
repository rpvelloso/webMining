/*
 * JSONRequest.hpp
 *
 *  Created on: 11 de mar de 2017
 *      Author: roberto
 */

#ifndef WEBDRIVER_JSONREQUEST_HPP_
#define WEBDRIVER_JSONREQUEST_HPP_

#include "HTTPClient.hpp"
#include "../3rdparty/json.hpp"

class JSONRequest {
public:
  static nlohmann::json go(HTTPMethod method, const std::string &url, const std::string &post = "") {
    HTTPClient httpClient(method,url,post);
    return nlohmann::json::parse(httpClient.getResponse());
  };
};

#endif /* WEBDRIVER_JSONREQUEST_HPP_ */
