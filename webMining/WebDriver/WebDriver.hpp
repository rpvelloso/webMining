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
#include "../3rdparty/sol.hpp"

class WebDriver;
extern WebDriver *chromeWebDriver;
extern WebDriver *fireFoxWebDriver;

class WebDriver {
public:
  WebDriver() {};
  virtual ~WebDriver() {};

  virtual const std::string &getDriverAddress() const = 0;
  virtual void setDriverAddress(const std::string &addr) = 0;

  virtual nlohmann::json status() = 0;
  virtual void newSession() = 0;
  virtual void go(std::string url) = 0;
  virtual const std::string &getPageSource() = 0;
  virtual void takeScreenshot(const std::string &filename) = 0;
  virtual void executeScript(const std::string script, bool async) = 0;
  virtual void deleteSession() = 0;

  static void luaBinding(sol::state &lua) {
  	lua.new_usertype<WebDriver>("WebDriver",
      "getDriverAddress",&WebDriver::getDriverAddress,
      "setDriverAddress",WebDriver::setDriverAddress,
      "newSession", WebDriver::newSession,
      "go", WebDriver::go,
      "getPageSource", &WebDriver::getPageSource,
      "takeScreenshot", WebDriver::takeScreenshot,
      "executeScript", WebDriver::executeScript,
      "deleteSession", WebDriver::deleteSession
  	);

    lua["webDriver"] = lua.create_table_with(
      "chrome",chromeWebDriver,
      "fireFox",fireFoxWebDriver
    );
  }
};

#endif /* WEBDRIVER_WEBDRIVER_HPP_ */
