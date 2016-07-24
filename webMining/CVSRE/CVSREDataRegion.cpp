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

#include "CVSREDataRegion.hpp"

#include <string>
#include <vector>

#include "../3rdparty/sol.hpp"
#include "../base/util.hpp"

void CVSREDataRegion::luaBinding(sol::state &lua) {
  lua.new_usertype < CVSREDataRegion
      > ("CVSREDataRegion", "getRecord", &CVSREDataRegion::getRecord, "recordCount", &CVSREDataRegion::recordCount, "recordSize", &CVSREDataRegion::recordSize, "size", &CVSREDataRegion::size, "getEndPos", &CVSREDataRegion::getEndPos, "getStartPos", &CVSREDataRegion::getStartPos, "getTps", &CVSREDataRegion::getTps, "getLinearRegression", &CVSREDataRegion::getLinearRegression, "isStructured", &CVSREDataRegion::isStructured, "getScore", &CVSREDataRegion::getScore, "isContent", &CVSREDataRegion::isContent, "getEstPeriod", &CVSREDataRegion::getEstPeriod, "getPeriodEstimator", &CVSREDataRegion::getPeriodEstimator);
}

CVSREDataRegion::CVSREDataRegion(size_t offset, double period, std::wstring tps,
                                 std::vector<pNode>::iterator b,
                                 std::vector<pNode>::iterator e)
    : startPos(offset),
      period(period),
      tps(tps),
      nodeSequence(b, e) {
  endPos = startPos + tps.size() - 1;
}

CVSREDataRegion::~CVSREDataRegion() {
}

const std::vector<pNode>&
CVSREDataRegion::getNodeSequence() const {
  return nodeSequence;
}

size_t CVSREDataRegion::size() {
  return endPos - startPos + 1;
}
size_t CVSREDataRegion::getEndPos() {
  return endPos;
}
size_t CVSREDataRegion::getStartPos() {
  return startPos;
}
const std::wstring &
CVSREDataRegion::getTps() {
  return tps;
}
LinearRegression CVSREDataRegion::getLinearRegression() {
  return LinearRegression { 0, 0, 0 };
}
bool CVSREDataRegion::isStructured() {
  return true;
}
double CVSREDataRegion::getScore() {
  return 0;
}
bool CVSREDataRegion::isContent() {
  return true;
}
double CVSREDataRegion::getEstPeriod() {
  return period;
}
int CVSREDataRegion::getPeriodEstimator() {
  return 0;
}

