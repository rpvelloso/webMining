/*
 * DSRE.h
 *
 *  Created on: 13 de jul de 2016
 *      Author: rvelloso
 */

#ifndef DSRE_HPP_
#define DSRE_HPP_

#include <unordered_map>
#include <string>
#include <vector>
#include <list>
#include <set>

#include "DSREDataRegion.hpp"
#include "../3rdparty/sol.hpp"
#include "../base/TPSExtractor.hpp"

enum class PeriodEstimator {
  DFT = 0,
  DCT = 1,
  ModifiedDCT = 2,
  StdDev3 = 3
};

class PeriodEstimatorStrategy {
 public:
  PeriodEstimatorStrategy() {
  }
  virtual ~PeriodEstimatorStrategy() {
  }
  virtual double estimate(std::vector<double> signal) = 0;
};

class DFTPeriodEstimator : public PeriodEstimatorStrategy {
 public:
  DFTPeriodEstimator() {
  }
  ~DFTPeriodEstimator() {
  }
  double estimate(std::vector<double> signal) override;
};

class DCTPeriodEstimator : public PeriodEstimatorStrategy {
 public:
  DCTPeriodEstimator() {
  }
  ~DCTPeriodEstimator() {
  }
  double estimate(std::vector<double> signal) override;
};

class ModifiedDCTPeriodEstimator : public PeriodEstimatorStrategy {
 public:
  ModifiedDCTPeriodEstimator() {
  }
  ~ModifiedDCTPeriodEstimator() {
  }
  double estimate(std::vector<double> signal) override;
};

class StdDev3PeriodEstimator : public PeriodEstimatorStrategy {
 public:
  StdDev3PeriodEstimator() {
  }
  ~StdDev3PeriodEstimator() {
  }
  double estimate(std::vector<double> signal) override;
};

class DSRE : public TPSExtractor<DSREDataRegion> {
 public:
  DSRE();
  virtual ~DSRE();
  void extract(pDOM dom) override;
  void setPeriodEstimator(PeriodEstimator);

  static void
  luaBinding(sol::state &lua);
 private:
  void segment();
  std::vector<size_t> detectStructure();
  std::set<size_t> locateRecords(DSREDataRegion &region);
  void pruneRecords(DSREDataRegion &, std::set<size_t> &);
  void alignRecords(DSREDataRegion &, std::set<size_t> &);
  void rankRegions(const std::vector<size_t> &);
  double estimatePeriod(const std::vector<double> &signal);

  std::list<std::pair<size_t, size_t> > segmentDifference(
      const std::vector<double> &);
  void mergeRegions(std::list<std::pair<size_t, size_t> > &);
  std::unordered_map<int, int> symbolFrequency(std::wstring, std::set<int> &);
  void extractRecords(std::vector<std::wstring> &m, std::set<size_t> &recpos,
                      DSREDataRegion &);

  std::unique_ptr<PeriodEstimatorStrategy> periodEstimatorPtr;
  PeriodEstimator periodEstimator;
};

#endif /* DSRE_HPP_ */
