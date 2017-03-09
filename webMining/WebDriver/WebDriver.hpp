/*
 * WebDriver.hpp
 *
 *  Created on: 9 de mar de 2017
 *      Author: roberto
 */

#ifndef WEBDRIVER_WEBDRIVER_HPP_
#define WEBDRIVER_WEBDRIVER_HPP_

#include <exception>
#include "HTTPClient.hpp"
#include "../3rdparty/json.hpp"

class WebDriver {
public:
  WebDriver() {};
  virtual ~WebDriver() {};

  virtual std::string newSession() = 0;
  virtual void go(std::string url) = 0;
  virtual std::string getPageSource() = 0;
  virtual void deleteSession() = 0;
};

class FireFoxWebDriver : public WebDriver {
public:
  FireFoxWebDriver() : WebDriver() {
    newSession();
  };
  virtual ~FireFoxWebDriver() {
    deleteSession();
  };

  virtual void newSession() {
    try{
      HTTPClient httpClient(HTTPMethod::POST,"localhost:4444/session","{}");
      auto response = nlohmann::json::parse(httpClient.getResponse());
      auto session = response["value"]["sessionId"];
      if (session.is_null())
        throw std::runtime_error("FireFoxWebDriver::newSession " + response.dump());
      sessionId = session;
    } catch (std::exception &e) {
      throw;
    }
  }

  virtual void go(std::string url) {
    if (!sessionId.empty()) {
      try{
        nlohmann::json jurl = {{"url", url}};
        HTTPClient httpClient(HTTPMethod::POST,"localhost:4444/session/" + sessionId + "/url", jurl.dump());
        auto response = nlohmann::json::parse(httpClient.getResponse());
        auto ret = response["value"];
        if (!ret.empty())
          throw std::runtime_error("FireFoxWebDriver::go " + response.dump());
      } catch (std::exception &e) {
        throw;
      }
    }
  }

  virtual std::string getPageSource() {
    std::string source = "";
    if (!sessionId.empty()) {
      try{
        HTTPClient httpClient(HTTPMethod::GET,"localhost:4444/session/" + sessionId + "/source");
        auto response = nlohmann::json::parse(httpClient.getResponse());
        auto ret = response["value"];
        if (!ret.empty() && ret.is_string())
          source = ret;
        else
          throw std::runtime_error("FireFoxWebDriver::getPageSource " + response.dump());
      } catch (std::exception &e) {
        throw;
      }
    }
    return source;
  }

  virtual void deleteSession() {
    if (!sessionId.empty()) {
      try{
        HTTPClient httpClient(HTTPMethod::DELETE,"localhost:4444/session/" + sessionId);
        auto response = nlohmann::json::parse(httpClient.getResponse());
        auto ret = response["value"];
        if (!ret.empty())
          throw std::runtime_error("FireFoxWebDriver::deleteSession " + response.dump());
        sessionId = "";
      } catch (std::exception &e) {
        throw;
      }
    }
  }
private:
  std::string sessionId = "";
};


#endif /* WEBDRIVER_WEBDRIVER_HPP_ */
