/*
 * DSREDataRegion.cpp
 *
 *  Created on: 13 de jul de 2016
 *      Author: rvelloso
 */

#include <algorithm>
#include "DSREDataRegion.h"
#include "util.hpp"

void DSREDataRegion::luaBinding(sol::state &lua) {
	lua.new_usertype<DSREDataRegion>("DSREDataRegion",
		"getRecord",&DSREDataRegion::getRecord,
		"recordCount",&DSREDataRegion::recordCount,
		"recordSize",&DSREDataRegion::recordSize,
		"size",&DSREDataRegion::size,
		"getEndPos",&DSREDataRegion::getEndPos,
		"getStartPos",&DSREDataRegion::getStartPos,
		"getTps",&DSREDataRegion::getTps,
		"getLinearRegression",&DSREDataRegion::getLinearRegression,
		"isStructured",&DSREDataRegion::isStructured,
		"getScore",&DSREDataRegion::getScore,
		"isContent",&DSREDataRegion::isContent
	);
}

DSREDataRegion::DSREDataRegion() {
}

DSREDataRegion::~DSREDataRegion() {
}

size_t DSREDataRegion::size() const noexcept {
	return endPos - startPos + 1;
}

size_t DSREDataRegion::getEndPos() const noexcept {
	return endPos;
}

void DSREDataRegion::setEndPos(size_t endPos) noexcept {
	this->endPos = endPos;
}

size_t DSREDataRegion::getStartPos() const noexcept {
	return startPos;
}

void DSREDataRegion::setStartPos(size_t startPos) noexcept {
	this->startPos = startPos;
}

void DSREDataRegion::shiftStartPos(long int shift) {
	auto result = startPos + shift;

	if (result < 0)
		startPos = 0;
	else
		startPos = result;
}

void DSREDataRegion::shiftEndPos(long int shift) {
	auto result = endPos + shift;

	if (result < 0)
		endPos = 0;
	else
		endPos = result;
}

const std::vector<pNode>& DSREDataRegion::getNodeSequence() const {
	return nodeSequence;
}

void DSREDataRegion::assignNodeSequence(
		std::vector<pNode>::iterator first,
		std::vector<pNode>::iterator last) {
	nodeSequence.assign(first,last);
}

void DSREDataRegion::eraseNodeSequence(std::function<bool(const pNode &)> f) {
	auto nodeSeqEnd = std::remove_if(nodeSequence.begin(), nodeSequence.end(), f);
	nodeSequence.erase(nodeSeqEnd, nodeSequence.end());
}

void DSREDataRegion::eraseTPS(std::function<bool(const wchar_t &)> f) {
	auto tpsEnd = std::remove_if(tps.begin(), tps.end(), f);
	tps.erase(tpsEnd, tps.end());
}

void DSREDataRegion::setNodeSequence(const std::vector<pNode>& nodeSequence) {
	this->nodeSequence = nodeSequence;
}

const std::wstring& DSREDataRegion::getTps() const {
	return tps;
}

void DSREDataRegion::setTps(const std::wstring& tps) {
	this->tps = tps;
}

const LinearRegression& DSREDataRegion::getLinearRegression() const {
	return linearRegression;
}

void DSREDataRegion::detectStructure() {
	this->linearRegression = computeLinearRegression(this->tps);
}

bool DSREDataRegion::isStructured() const {
	return structured;
}

void DSREDataRegion::setStructured(bool structured) {
	this->structured = structured;
}

double DSREDataRegion::getScore() const {
	return score;
}

void DSREDataRegion::setScore(double score) {
	this->score = score;
}

bool DSREDataRegion::isContent() const {
	return content;
}

void DSREDataRegion::setContent(bool content) {
	this->content = content;
}

double DSREDataRegion::getStdDev() const {
	return stdDev;
}

void DSREDataRegion::setStdDev(double stddev) {
	this->stdDev = stddev;
}
