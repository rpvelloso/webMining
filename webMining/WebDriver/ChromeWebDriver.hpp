/*
 * ChromeWebDriver.hpp
 *
 *  Created on: 10 de mar de 2017
 *      Author: rvelloso
 */

#ifndef WEBDRIVER_CHROMEWEBDRIVER_HPP_
#define WEBDRIVER_CHROMEWEBDRIVER_HPP_

#include "WebDriver.hpp"

class ChromeWebDriver : public WebDriver {
public:
  ChromeWebDriver(std::string driverURL = "localhost:9515");
  virtual ~ChromeWebDriver();

  const std::string &getDriverAddress() const;
  void setDriverAddress(const std::string &addr);

  virtual nlohmann::json status();
  virtual void newSession();
  virtual void go(std::string url);
  virtual const std::string &getPageSource();
  virtual void takeScreenshot(const std::string &filename);
  virtual void executeScript(const std::string script, bool async);
  virtual void deleteSession();
private:
  std::string sessionId = "";
  std::string driverUrl = "localhost:9515";
  std::string pageSource;
};

#endif /* WEBDRIVER_CHROMEWEBDRIVER_HPP_ */
