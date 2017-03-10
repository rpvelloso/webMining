/*
 * ChromeWebDriver.cpp
 *
 *  Created on: 10 de mar de 2017
 *      Author: rvelloso
 */

#include "ChromeWebDriver.hpp"

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
	  HTTPClient httpClient(HTTPMethod::mPOST,driverUrl + "/session",jcap.dump());
	  auto response = nlohmann::json::parse(httpClient.getResponse());
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
		HTTPClient httpClient(HTTPMethod::mPOST,driverUrl + "/session/" + sessionId + "/url", jurl.dump());
		auto response = nlohmann::json::parse(httpClient.getResponse());
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
		HTTPClient httpClient(HTTPMethod::mGET,driverUrl + "/session/" + sessionId + "/source");
		auto response = nlohmann::json::parse(httpClient.getResponse());
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

void ChromeWebDriver::deleteSession() {
	if (!sessionId.empty()) {
	  try{
		HTTPClient httpClient(HTTPMethod::mDELETE,driverUrl + "/session/" + sessionId);
		auto response = nlohmann::json::parse(httpClient.getResponse());
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

