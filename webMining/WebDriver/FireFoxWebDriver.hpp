/*
 * FireFoxWebDriver.hpp
 *
 *  Created on: 10 de mar de 2017
 *      Author: rvelloso
 */

#ifndef WEBDRIVER_FIREFOXWEBDRIVER_HPP_
#define WEBDRIVER_FIREFOXWEBDRIVER_HPP_

#include "WebDriver.hpp"
#include "../3rdparty/json.hpp"

class FireFoxWebDriver : public WebDriver {
public:
  FireFoxWebDriver(std::string driverURL = "localhost:4444");
  virtual ~FireFoxWebDriver();

  const std::string &getDriverAddress() const;
  void setDriverAddress(const std::string &addr);

  virtual nlohmann::json status();
  virtual void newSession();
  virtual void go(std::string url);
  virtual const std::string &getPageSource();
  virtual void takeScreenshot(const std::string &filename);
  virtual void deleteSession();
private:
  std::string sessionId = "";
  std::string driverUrl = "localhost:4444";
  std::string pageSource;
};

#endif /* WEBDRIVER_FIREFOXWEBDRIVER_HPP_ */
