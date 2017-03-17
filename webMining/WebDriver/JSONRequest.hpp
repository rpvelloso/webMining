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
    std::cerr << std::endl << methodStr[static_cast<int>(method)] << " " << url << std::endl;
    if (method == HTTPMethod::mPOST) std::cerr << post << std::endl;

    HTTPClient httpClient(method,url,post);

    std::cerr << httpClient.getResponse() << std::endl;

    return nlohmann::json::parse(httpClient.getResponse());
  };
};

#endif /* WEBDRIVER_JSONREQUEST_HPP_ */
