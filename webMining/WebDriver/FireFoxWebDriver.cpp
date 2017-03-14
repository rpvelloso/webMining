/*
 * FireFoxWebDriver.cpp
 *
 *  Created on: 10 de mar de 2017
 *      Author: rvelloso
 */

#include <fstream>
#include "JSONRequest.hpp"
#include "FireFoxWebDriver.hpp"
#include "../base/util.hpp"

WebDriver *fireFoxWebDriver = new FireFoxWebDriver();

FireFoxWebDriver::FireFoxWebDriver(std::string driverURL) {
	this->driverUrl = driverUrl;
};

FireFoxWebDriver::~FireFoxWebDriver() {
if (!sessionId.empty())
	deleteSession();
};

const std::string &FireFoxWebDriver::getDriverAddress() const {
	return driverUrl;
}
void FireFoxWebDriver::setDriverAddress(const std::string &addr) {
	driverUrl = addr;
}

void FireFoxWebDriver::newSession() {
	try{
	  auto response = JSONRequest::go(HTTPMethod::mPOST,driverUrl + "/session","{}");
	  auto session = response["value"]["sessionId"];
	  if (session.is_null())
		throw std::runtime_error("FireFoxWebDriver::newSession " + response.dump());
	  sessionId = session;
	} catch (std::exception &e) {
	  throw;
	}
}

void FireFoxWebDriver::go(std::string url) {
	if (!sessionId.empty()) {
	  try{
		nlohmann::json jurl = {{"url", url}};
		auto response = JSONRequest::go(HTTPMethod::mPOST,driverUrl + "/session/" + sessionId + "/url", jurl.dump());
		auto ret = response["value"];
		if (!ret.empty())
		  throw std::runtime_error("FireFoxWebDriver::go " + response.dump());
	  } catch (std::exception &e) {
		throw;
	  }
	} else
		throw std::runtime_error("FireFoxWebDriver::go no session available");
}

const std::string &FireFoxWebDriver::getPageSource() {
	if (!sessionId.empty()) {
	  try{
    auto response = JSONRequest::go(HTTPMethod::mGET,driverUrl + "/session/" + sessionId + "/source");
		auto ret = response["value"];
		if (!ret.empty() && ret.is_string())
		  pageSource = ret;
		else
		  throw std::runtime_error("FireFoxWebDriver::getPageSource " + response.dump());
	  } catch (std::exception &e) {
		throw;
	  }
	} else
		throw std::runtime_error("FireFoxWebDriver::getPageSource no session available");
	return pageSource;
}

void FireFoxWebDriver::takeScreenshot(const std::string &filename) {
  if (!sessionId.empty()) {
    try{
    auto response = JSONRequest::go(HTTPMethod::mGET,driverUrl + "/session/" + sessionId + "/screenshot");

    if (!response["value"].is_string() || response["value"].empty())
      throw std::runtime_error("FireFoxWebDriver::takeScreenshot " + response.dump());

    std::string base64Screenshot = response["value"];
    std::vector<unsigned char> decodedScreenshot;

    if (decode64(base64Screenshot, decodedScreenshot)) {
      std::fstream outputFile(filename + ".png", std::fstream::trunc | std::fstream::binary | std::fstream::out);
      outputFile.write((char *)&decodedScreenshot[0], decodedScreenshot.size());
      outputFile.close();
    }
    } catch (std::exception &e) {
    throw;
    }
  } else
    throw std::runtime_error("FireFoxWebDriver::takeScreenshot no session available");
}

void FireFoxWebDriver::deleteSession() {
	if (!sessionId.empty()) {
	  try{
	  auto response = JSONRequest::go(HTTPMethod::mDELETE,driverUrl + "/session/" + sessionId);
		auto ret = response["value"];
		if (!ret.empty())
		  throw std::runtime_error("FireFoxWebDriver::deleteSession " + response.dump());
		sessionId = "";
	  } catch (std::exception &e) {
		throw;
	  }
	} else
		throw std::runtime_error("FireFoxWebDriver::deleteSession no session available");
}

nlohmann::json FireFoxWebDriver::status() {
	return JSONRequest::go(HTTPMethod::mGET,driverUrl + "/status");
}
