/*
 * ChromeWebDriver.cpp
 *
 *  Created on: 10 de mar de 2017
 *      Author: rvelloso
 */

#include <fstream>
#include "JSONRequest.hpp"
#include "ChromeWebDriver.hpp"
#include "../base/util.hpp"

WebDriver *chromeWebDriver = new ChromeWebDriver();

ChromeWebDriver::ChromeWebDriver(std::string driverURL) {
	this->driverUrl = driverUrl;
};

ChromeWebDriver::~ChromeWebDriver() {
if (!getSession().empty())
	deleteSession();
};

const std::string &ChromeWebDriver::getDriverAddress() const {
	return driverUrl;
}
void ChromeWebDriver::setDriverAddress(const std::string &addr) {
	driverUrl = addr;
}

void ChromeWebDriver::newSession() {
	try{
	  nlohmann::json jcap = {{"desiredCapabilities", {
			  {"binary",""}
	  }}};
	  auto response = JSONRequest::go(HTTPMethod::mPOST,driverUrl + "/session",jcap.dump());
	  int status = response["status"];
	  auto session = response["sessionId"];
	  if (status != 0)
		throw std::runtime_error("ChromeWebDriver::newSession " + response.dump());
	  setSession(session);
	} catch (std::exception &e) {
	  throw;
	}
}

void ChromeWebDriver::go(std::string url) {
	if (!getSession().empty()) {
	  try{
		nlohmann::json jurl = {{"url", url}};
		auto response = JSONRequest::go(HTTPMethod::mPOST,driverUrl + "/session/" + getSession() + "/url", jurl.dump());
		int status = response["status"];
		if (status != 0)
		  throw std::runtime_error("ChromeWebDriver::go " + response.dump());
	  } catch (std::exception &e) {
		throw;
	  }
	} else
		throw std::runtime_error("ChromeWebDriver::go no session available");
}

const std::string &ChromeWebDriver::getPageSource() {
	if (!getSession().empty()) {
	  try{
	  auto response = JSONRequest::go(HTTPMethod::mGET,driverUrl + "/session/" + getSession() + "/source");
		int status = response["status"];
		if (status == 0)
		  pageSource = response["value"];
		else
		  throw std::runtime_error("ChromeWebDriver::getPageSource " + response.dump());
	  } catch (std::exception &e) {
		throw;
	  }
	} else
		throw std::runtime_error("ChromeWebDriver::getPageSource no session available");
	return pageSource;
}

void ChromeWebDriver::takeScreenshot(const std::string &filename) {
	if (!getSession().empty()) {
	  try{
      auto response = JSONRequest::go(HTTPMethod::mGET,driverUrl + "/session/" + getSession() + "/screenshot");
      int status = response["status"];
      if (status != 0)
        throw std::runtime_error("ChromeWebDriver::takeScreenshot " + response.dump());

      std::string base64Screenshot = response["value"];
      std::vector<unsigned char> decodedScreenshot;

      decode64(base64Screenshot, decodedScreenshot);
      std::fstream outputFile(filename + ".png", std::fstream::trunc | std::fstream::binary | std::fstream::out);
      outputFile.write((char *)&decodedScreenshot[0], decodedScreenshot.size());
      outputFile.close();
	  } catch (std::exception &e) {
	    throw;
	  }
	} else
		throw std::runtime_error("ChromeWebDriver::takeScreenshot no session available");
}

nlohmann::json ChromeWebDriver::status() {
	return JSONRequest::go(HTTPMethod::mGET,driverUrl + "/status");
}

std::string ChromeWebDriver::executeScript(const std::string script, bool async) {
  if (!getSession().empty()) {
    try{
      nlohmann::json jscript = {{"script", script}, {"args",nlohmann::json::array()}};
      auto response = JSONRequest::go(
          HTTPMethod::mPOST,
          driverUrl + "/session/" + getSession() + (async?"/execute_async":"/execute"),
          jscript.dump());
      int status = response["status"];
      if (status != 0)
        throw std::runtime_error("ChromeWebDriver::executeScript " + response.dump());
      return response["value"].dump();
    } catch (std::exception &e) {
      throw;
    }
  } else
    throw std::runtime_error("ChromeWebDriver::executeScript no session available");
}

const std::string& ChromeWebDriver::getSession() {
	return sessionId;
}

void ChromeWebDriver::setSession(const std::string& session) {
	sessionId = session;
}

std::string ChromeWebDriver::getCurrentURL() {
  if (!getSession().empty()) {
    try{
      auto response = JSONRequest::go(HTTPMethod::mGET,driverUrl + "/session/" + getSession() + "/url");
      int status = response["status"];
      if (status != 0)
        throw std::runtime_error("ChromeWebDriver::getCurrentURL " + response.dump());
      return response["value"];
    } catch (std::exception &e) {
      throw;
    }
  } else
    throw std::runtime_error("ChromeWebDriver::getCurrentURL no session available");
}

void ChromeWebDriver::deleteSession() {
	if (!getSession().empty()) {
	  try{
	    auto response = JSONRequest::go(HTTPMethod::mDELETE,driverUrl + "/session/" + getSession());
	    int status = response["status"];
	    if (status != 0)
	      throw std::runtime_error("ChromeWebDriver::deleteSession " + response.dump());
	    setSession("");
	  } catch (std::exception &e) {
	    throw;
	  }
	} else
		throw std::runtime_error("ChromeWebDriver::deleteSession no session available");
}

