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
if (!getSession().empty())
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
	  setSession(session);
	} catch (std::exception &e) {
	  throw;
	}
}

void FireFoxWebDriver::go(std::string url) {
	if (!getSession().empty()) {
	  try{
		nlohmann::json jurl = {{"url", url}};
		auto response = JSONRequest::go(HTTPMethod::mPOST,driverUrl + "/session/" + getSession() + "/url", jurl.dump());
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
	if (!getSession().empty()) {
	  try{
    auto response = JSONRequest::go(HTTPMethod::mGET,driverUrl + "/session/" + getSession() + "/source");
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
  if (!getSession().empty()) {
    try{
      auto response = JSONRequest::go(HTTPMethod::mGET,driverUrl + "/session/" + getSession() + "/screenshot");

      if (!response["value"].is_string() || response["value"].empty())
        throw std::runtime_error("FireFoxWebDriver::takeScreenshot " + response.dump());

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
    throw std::runtime_error("FireFoxWebDriver::takeScreenshot no session available");
}

std::string FireFoxWebDriver::executeScript(const std::string script, bool async) {
  if (!getSession().empty()) {
    try{
      nlohmann::json jscript = {{"script", script}, {"args", nlohmann::json::array()}};
      auto response = JSONRequest::go(
          HTTPMethod::mPOST,
          driverUrl + "/session/" + getSession() + "/execute/" + (async?"async":"sync"),
          jscript.dump());
   	  if (response["value"].is_structured() && !response["value"]["error"].empty())
   		  throw std::runtime_error("FireFoxWebDriver::executeScript " + response.dump());
      return response["value"].dump();
    } catch (std::exception &e) {
      throw;
    }
  } else
    throw std::runtime_error("FireFoxWebDriver::executeScript no session available");
}

const std::string& FireFoxWebDriver::getSession() {
	return sessionId;
}

void FireFoxWebDriver::setSession(const std::string& session) {
	sessionId = session;
}

std::string FireFoxWebDriver::getCurrentURL() {
  if (!getSession().empty()) {
    try{
      auto response = JSONRequest::go(HTTPMethod::mGET,driverUrl + "/session/" + getSession() + "/url");
      if (response["value"].is_structured() && !response["value"]["error"].empty())
        throw std::runtime_error("FireFoxWebDriver::getCurrentURL " + response.dump());
      return response["value"];
    } catch (std::exception &e) {
      throw;
    }
  } else
    throw std::runtime_error("FireFoxWebDriver::getCurrentURL no session available");
}

void FireFoxWebDriver::deleteSession() {
	if (!getSession().empty()) {
	  try{
	  auto response = JSONRequest::go(HTTPMethod::mDELETE,driverUrl + "/session/" + getSession());
		auto ret = response["value"];
		if (!ret.empty())
		  throw std::runtime_error("FireFoxWebDriver::deleteSession " + response.dump());
		setSession("");
	  } catch (std::exception &e) {
		throw;
	  }
	} else
		throw std::runtime_error("FireFoxWebDriver::deleteSession no session available");
}

nlohmann::json FireFoxWebDriver::status() {
	return JSONRequest::go(HTTPMethod::mGET,driverUrl + "/status");
}
