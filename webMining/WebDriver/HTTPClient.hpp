/*
 * HTTPClient.hpp
 *
 *  Created on: 9 de mar de 2017
 *      Author: roberto
 */

#ifndef WEBDRIVER_HTTPCLIENT_HPP_
#define WEBDRIVER_HTTPCLIENT_HPP_

#include <iostream>
#include <sstream>
#include <memory>
#include <exception>
#include <curl/curl.h>

enum class HTTPMethod {
    GET,
    POST,
    DELETE
};

class HTTPClient {
public:
    HTTPClient(HTTPMethod method, const std::string &url, const std::string &post = "");

    virtual ~HTTPClient();
    std::string getResponse() const;
private:
    CURL *curl;
    std::stringstream response;

    static size_t curlWriteCallback(char *in, uint size, uint nmemb, HTTPClient *client);
    size_t appendResponse(char *in, size_t len);
    void cleanup();
};

#endif /* WEBDRIVER_HTTPCLIENT_HPP_ */
