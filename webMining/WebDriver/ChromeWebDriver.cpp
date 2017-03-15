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
if (!sessionId.empty())
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
	  sessionId = session;
	} catch (std::exception &e) {
	  throw;
	}
}

void ChromeWebDriver::go(std::string url) {
	if (!sessionId.empty()) {
	  try{
		nlohmann::json jurl = {{"url", url}};
		auto response = JSONRequest::go(HTTPMethod::mPOST,driverUrl + "/session/" + sessionId + "/url", jurl.dump());
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
	if (!sessionId.empty()) {
	  try{
	  auto response = JSONRequest::go(HTTPMethod::mGET,driverUrl + "/session/" + sessionId + "/source");
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
	if (!sessionId.empty()) {
	  try{
      auto response = JSONRequest::go(HTTPMethod::mGET,driverUrl + "/session/" + sessionId + "/screenshot");
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

void ChromeWebDriver::executeScript(const std::string script, bool async) {
  if (!sessionId.empty()) {
    try{
      nlohmann::json jscript = {{"script", script}, {"args",nlohmann::json::array()}};
      auto response = JSONRequest::go(
          HTTPMethod::mPOST,
          driverUrl + "/session/" + sessionId + (async?"/execute_async":"/execute"),
          jscript.dump());
      int status = response["status"];
      if (status != 0)
        throw std::runtime_error("ChromeWebDriver::executeScript " + response.dump());
    } catch (std::exception &e) {
      throw;
    }
  } else
    throw std::runtime_error("ChromeWebDriver::executeScript no session available");
}

void ChromeWebDriver::deleteSession() {
	if (!sessionId.empty()) {
	  try{
	    auto response = JSONRequest::go(HTTPMethod::mDELETE,driverUrl + "/session/" + sessionId);
	    int status = response["status"];
	    if (status != 0)
	      throw std::runtime_error("ChromeWebDriver::deleteSession " + response.dump());
	    sessionId = "";
	  } catch (std::exception &e) {
	    throw;
	  }
	} else
		throw std::runtime_error("ChromeWebDriver::deleteSession no session available");
}

