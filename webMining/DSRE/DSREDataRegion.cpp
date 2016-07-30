/*
 Copyright 2011 Roberto Panerai Velloso.
 This file is part of webMining.
 webMining is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 webMining is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with webMining.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DSREDataRegion.hpp"

#include <algorithm>
#include <iterator>
#include <string>

void DSREDataRegion::luaBinding(sol::state &lua) {
  lua.new_usertype<DSREDataRegion>("DSREDataRegion",
		  "getRecord", &DSREDataRegion::getRecord,
		  "recordCount", &DSREDataRegion::recordCount,
		  "recordSize", &DSREDataRegion::recordSize,
		  "size", &DSREDataRegion::size,
		  "getEndPos", &DSREDataRegion::getEndPos,
		  "getStartPos", &DSREDataRegion::getStartPos,
		  "getTps", &DSREDataRegion::getTps,
		  "getLinearRegression", &DSREDataRegion::getLinearRegression,
		  "isStructured", &DSREDataRegion::isStructured,
		  "getScore", &DSREDataRegion::getScore,
		  "isContent", &DSREDataRegion::isContent,
		  "getTransform",  &DSREDataRegion::getTransform
      );

  lua.new_usertype<LinearRegression>("LinearRegression",
		  "a", &LinearRegression::a,
		  "b", &LinearRegression::b,
		  "e", &LinearRegression::e);

}

DSREDataRegion::DSREDataRegion(const std::wstring &ftps,
                               const std::vector<pNode> &fns)
    : StructuredDataRegion(),
      fullTps(&ftps),
      fullNodeSequence(&fns) {

}

DSREDataRegion::~DSREDataRegion() {
}

size_t DSREDataRegion::size() const noexcept {
  return endPos - startPos + 1;
}

size_t DSREDataRegion::getEndPos() const noexcept {
  return endPos;
}

void DSREDataRegion::setEndPos(size_t endPos) {
  if (endPos < fullTps->size())
    this->endPos = endPos;
  else
    throw new std::out_of_range("tps end position out of range");
}

size_t DSREDataRegion::getStartPos() const noexcept {
  return startPos;
}

void DSREDataRegion::setStartPos(size_t startPos) {
  if (startPos < fullTps->size())
    this->startPos = startPos;
  else
    throw new std::out_of_range("tps start position out of range");
}

void DSREDataRegion::shiftStartPos(long int shift) {
  auto result = startPos + shift;

  if (result >= 0 && result < fullTps->size())
    startPos = result;
  else
    throw new std::out_of_range("tps start position shifted out of range");
}

void DSREDataRegion::shiftEndPos(long int shift) {
  auto result = endPos + shift;

  if (result >= 0 && result < fullTps->size())
    endPos = result;
  else
    throw new std::out_of_range("tps end position shifted out of range");
}

const std::vector<pNode>& DSREDataRegion::getNodeSequence() const {
  return nodeSequence;
}

void DSREDataRegion::eraseNodeSequence(std::function<bool(const pNode &)> f) {
  auto nodeSeqEnd = std::remove_if(nodeSequence.begin(), nodeSequence.end(), f);
  nodeSequence.erase(nodeSeqEnd, nodeSequence.end());
}

void DSREDataRegion::eraseTPS(std::function<bool(const wchar_t &)> f) {
  auto tpsEnd = std::remove_if(tps.begin(), tps.end(), f);
  tps.erase(tpsEnd, tps.end());
}

const std::wstring& DSREDataRegion::getTps() const {
  return tps;
}

LinearRegression DSREDataRegion::getLinearRegression() const noexcept {
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

std::vector<double> DSREDataRegion::getTransform() const {
  return transform;
}

void DSREDataRegion::setTransform(const std::vector<double>& transform) {
  this->transform = transform;
}

void DSREDataRegion::refreshTps() {
  tps = fullTps->substr(startPos, size());
  nodeSequence.assign(fullNodeSequence->begin() + startPos,
                      fullNodeSequence->begin() + startPos + size());
}
