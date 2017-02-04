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

#ifndef DSREDATAREGION_HPP_
#define DSREDATAREGION_HPP_

//#include <crtdefs.h>
#include <functional>
#include <string>
#include <vector>

#include "../3rdparty/sol.hpp"
#include "../base/Node.hpp"
#include "../base/StructuredDataRegion.hpp"
#include "../base/util.hpp"

class DSREDataRegion : public StructuredDataRegion {
 public:
  DSREDataRegion(const std::wstring &ftps, const std::vector<pNode> &fns);
  virtual ~DSREDataRegion();
  size_t size() const noexcept;
  size_t getEndPos() const noexcept;
  void setEndPos(size_t endPos);
  size_t getStartPos() const noexcept;
  void setStartPos(size_t startPos);
  void shiftStartPos(long int shift);
  void shiftEndPos(long int shift);
  const std::vector<pNode>& getNodeSequence() const;
  void eraseNodeSequence(std::function<bool(const pNode &)>);
  void eraseTPS(std::function<bool(const wchar_t &)>);
  const std::wstring& getTps() const;
  LinearRegression getLinearRegression() const noexcept;
  void detectStructure();
  bool isStructured() const;
  void setStructured(bool structured);
  double getScore() const;
  void setScore(double score);
  bool isContent() const;
  void setContent(bool content);
  double getStdDev() const;
  void setStdDev(double stddev);
  void refreshTps();

  static void luaBinding(sol::state &lua);
  std::vector<double> getTransform() const;
  void setTransform(const std::vector<double>& transform);

 private:
  DSREDataRegion() = delete;
  std::wstring tps;
  std::vector<pNode> nodeSequence;
  LinearRegression linearRegression;
  size_t startPos = 0, endPos = 0;
  bool structured = false;
  bool content = false;
  double score = 0;
  double stdDev = 0;
  std::vector<double> transform;

  const std::wstring *fullTps;
  const std::vector<pNode> *fullNodeSequence;
};

#endif /* DSREDATAREGION_HPP_ */
