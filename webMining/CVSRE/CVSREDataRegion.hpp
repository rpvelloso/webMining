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

#ifndef CVSREDATAREGION_HPP_
#define CVSREDATAREGION_HPP_

#include <crtdefs.h>
#include <string>
#include <vector>

#include "../3rdparty/sol.hpp"
#include "../base/Node.hpp"
#include "../base/StructuredDataRegion.hpp"

struct LinearRegression;

class CVSREDataRegion : public StructuredDataRegion {
 public:
  CVSREDataRegion(size_t offset, double period, std::wstring tps,
                  std::vector<pNode>::iterator b,
                  std::vector<pNode>::iterator e);
  virtual ~CVSREDataRegion();
  const std::vector<pNode>& getNodeSequence() const;

  size_t size();
  size_t getEndPos();
  size_t getStartPos();
  const std::wstring &getTps();
  LinearRegression getLinearRegression();
  bool isStructured();
  double getScore();
  bool isContent();
  double getEstPeriod();
  int getPeriodEstimator();

  static void luaBinding(sol::state &lua);
 private:
  size_t endPos, startPos;
  double period;
  std::wstring tps;
  std::vector<pNode> nodeSequence;
};

#endif /* CVSREDATAREGION_HPP_ */
