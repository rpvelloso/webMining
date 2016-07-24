/*
 * CVSRE.hpp
 *
 *  Created on: 23 de jul de 2016
 *      Author: roberto
 */

#ifndef CVSRE_HPP_
#define CVSRE_HPP_

#include <crtdefs.h>
#include <vector>

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
