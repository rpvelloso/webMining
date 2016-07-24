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

#ifndef CVSRE_HPP_
#define CVSRE_HPP_

#include <crtdefs.h>
#include <string>
#include <vector>

#include "../3rdparty/sol.hpp"
#include "../base/DOM.hpp"
#include "../base/TPSExtractor.hpp"
#include "CVSREDataRegion.hpp"

class CVSRE : public TPSExtractor<CVSREDataRegion> {
 public:
  CVSRE();
  virtual ~CVSRE();
  void extract(pDOM dom) override;
  static void luaBinding(sol::state &lua);
 private:
  void segment(size_t offset);
  std::vector<size_t> checkCV(const std::wstring &tps, int tpc, double &m);
  bool checkFreq(const std::wstring &tps, size_t freq);
  std::vector<std::wstring> align(const std::wstring &tps,
                                  const std::vector<size_t> &recpos);
  void extractRecords(std::vector<std::wstring> &m, std::vector<size_t> &recpos,
                      CVSREDataRegion &region);
};

#endif /* CVSRE_HPP_ */
