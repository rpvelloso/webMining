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

  virtual void newSession() = 0;
  virtual void go(std::string url) = 0;
  virtual const std::string &getPageSource() = 0;
  virtual void deleteSession() = 0;

  static void luaBinding(sol::state &lua) {
  	lua.new_usertype<WebDriver>("WebDriver",
  			"go", &WebDriver::go,
  			"getPageSource", &WebDriver::getPageSource
  			);
  }
};

class FireFoxWebDriver : public WebDriver {
public:
  FireFoxWebDriver(std::string driverURL = "localhost:4444") {
	  this->driverUrl = driverUrl;
    newSession();
  };
  virtual ~FireFoxWebDriver() {
    deleteSession();
  };

  virtual void newSession() {
    try{
      HTTPClient httpClient(HTTPMethod::mPOST,driverUrl + "/session","{}");
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
        HTTPClient httpClient(HTTPMethod::mPOST,driverUrl + "/session/" + sessionId + "/url", jurl.dump());
        auto response = nlohmann::json::parse(httpClient.getResponse());
        auto ret = response["value"];
        if (!ret.empty())
          throw std::runtime_error("FireFoxWebDriver::go " + response.dump());
      } catch (std::exception &e) {
        throw;
      }
    }
  }

  virtual const std::string &getPageSource() {
    if (!sessionId.empty()) {
      try{
        HTTPClient httpClient(HTTPMethod::mGET,driverUrl + "/session/" + sessionId + "/source");
        auto response = nlohmann::json::parse(httpClient.getResponse());
        auto ret = response["value"];
        if (!ret.empty() && ret.is_string())
          pageSource = ret;
        else
          throw std::runtime_error("FireFoxWebDriver::getPageSource " + response.dump());
      } catch (std::exception &e) {
        throw;
      }
    }
    return pageSource;
  }

  virtual void deleteSession() {
    if (!sessionId.empty()) {
      try{
        HTTPClient httpClient(HTTPMethod::mDELETE,driverUrl + "/session/" + sessionId);
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
  std::string driverUrl = "localhost:4444";
  std::string pageSource;
};

static WebDriver *fireFoxWebDriver = new FireFoxWebDriver();

#endif /* WEBDRIVER_WEBDRIVER_HPP_ */
