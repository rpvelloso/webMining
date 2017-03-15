/*
 * HTTPClient.cpp
 *
 *  Created on: 9 de mar de 2017
 *      Author: roberto
 */

#include "HTTPClient.hpp"

std::vector<std::string> methodStr = {"GET", "POST", "DELETE"};

HTTPClient::HTTPClient(HTTPMethod method, const std::string &url, const std::string &post) : curl(nullptr), response("") {
  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (method == HTTPMethod::mPOST)
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());
    else if (method == HTTPMethod::mDELETE)
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &(HTTPClient::curlWriteCallback));
    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      cleanup();
      throw std::runtime_error(curl_easy_strerror(res));
    }
  }
}

HTTPClient::~HTTPClient() {
  cleanup();
}

std::string HTTPClient::getResponse() const {
  return response.str();
}

size_t HTTPClient::curlWriteCallback(char *in, size_t size, size_t nmemb, HTTPClient *client) {
  return client->appendResponse(in, size*nmemb);
}

size_t HTTPClient::appendResponse(char *in, size_t len) {
  response.write(in, len);
  return len;
}

void HTTPClient::cleanup() {
  if (curl)
    curl_easy_cleanup(curl);
  curl_global_cleanup();
}
