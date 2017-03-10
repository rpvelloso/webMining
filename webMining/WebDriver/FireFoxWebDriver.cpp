/*
 * FireFoxWebDriver.cpp
 *
 *  Created on: 10 de mar de 2017
 *      Author: rvelloso
 */

#include "FireFoxWebDriver.hpp"

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

void FireFoxWebDriver::go(std::string url) {
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
	} else
		throw std::runtime_error("FireFoxWebDriver::go no session available");
}

const std::string &FireFoxWebDriver::getPageSource() {
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
	} else
		throw std::runtime_error("FireFoxWebDriver::getPageSource no session available");
	return pageSource;
}

void FireFoxWebDriver::deleteSession() {
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
	} else
		throw std::runtime_error("FireFoxWebDriver::deleteSession no session available");
}

