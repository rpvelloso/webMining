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

#include <algorithm>
#include <iomanip>
#include "DSRE.hpp"
#include "../base/util.hpp"

void DSRE::luaBinding(sol::state &lua) {
  lua.new_usertype<DSRE>("DSRE",
		  "extract", &DSRE::extract,
		  "cleanup", &DSRE::cleanup,
		  "getTps", &DSRE::getTps,
		  "printTps", &DSRE::printTps,
		  "regionCount", &DSRE::regionCount,
		  "getDataRegion", &DSRE::getDataRegion,
		  "setPeriodEstimator", &DSRE::setPeriodEstimator);
  DSREDataRegion::luaBinding(lua);
}

DSRE::DSRE()
    : periodEstimatorPtr(new ModifiedDCTPeriodEstimator()),
      periodEstimator(PeriodEstimator::ModifiedDCT) {
}

DSRE::~DSRE() {
}

void DSRE::extract(pDOM dom) {
  cleanup();
  std::cerr << std::endl << "Processing " << dom->getURI() << " ..."
      << std::endl;

  buildTagPath("", dom->body(), true);
  segment();
  auto structured = detectStructure();

  for (auto i : structured) {
    auto &region = dataRegions[i];

    std::cerr << "TPS: " << std::endl;
    for (auto j : region.getTps())
      std::cerr << j << " ";
    std::cerr << std::endl;

    auto recpos = locateRecords(region);
    pruneRecords(region, recpos);
    alignRecords(region, recpos);
  }

  // remove regions with only a single record or single column
  auto struEnd =
      remove_if(
          structured.begin(),
          structured.end(),
          [&](long int i)->bool
          {
            return (this->dataRegions[i].recordCount() < 2) || (this->dataRegions[i].recordSize() < 2);
          });
  structured.erase(struEnd, structured.end());

  rankRegions(structured);
}

std::list<std::pair<size_t, size_t> > DSRE::segmentDifference(
    const std::vector<double> &diff) {
  std::list < std::pair<size_t, size_t> > ret;
  size_t start = 0, end = 0;

  for (size_t i = 0; i < diff.size(); ++i) {
    if (diff[i] != 0) {
      if ((end - start + 1) > 3)
        ret.push_back(std::make_pair(start, end));

      start = end = i + 1;
    } else
      end = i;
  }

  if (start != end)
    ret.push_back(std::make_pair(start, end));

  return ret;
}

std::unordered_map<int, int> DSRE::symbolFrequency(std::wstring s,
                                                   std::set<int> &alphabet) {
  std::unordered_map<int, int> symbolCount;

  alphabet.clear();
  // compute symbol frequency
  for (auto c : s) {
    if (c != 0) {
      if (alphabet.count(c) == 0) {
        symbolCount[c] = 0;
        alphabet.insert(c);
      }
      symbolCount[c]++;
    }
  }
  return symbolCount;
}

void DSRE::mergeRegions(std::list<std::pair<size_t, size_t> > &regions) {
// merge regions with common alphabet
  auto r = ++regions.begin();
  for (; r != regions.end(); ++r) {
    std::set<int> alpha, palpha, intersect, setUnion;
    auto prev = r;
    --prev;

    // avoid merging regions too far apart
    /*
     if ((r->pos - (prev->pos+prev->len-1)) > 5)
     continue;
     */
    auto tps = tagPathSequence.substr((*r).first, (*r).second - (*r).first + 1);
    auto ptps = tagPathSequence.substr((*prev).first,
                                       (*prev).second - (*prev).first + 1);
    symbolFrequency(tps, alpha);
    symbolFrequency(ptps, palpha);
    std::set_intersection(palpha.begin(), palpha.end(), alpha.begin(),
                          alpha.end(),
                          std::inserter(intersect, intersect.begin()));

    std::set_union(palpha.begin(), palpha.end(), alpha.begin(), alpha.end(),
                   std::inserter(setUnion, setUnion.begin()));
    if (!intersect.empty()) {
      //if (((double)intersect.size()/(double)setUnion.size()) > 0.05) {
      (*r).first = (*prev).first;
      regions.erase(prev);
    }
  }
}

void DSRE::segment() {
  auto s = tagPathSequence;
  s = contour(s);
  auto diff = difference(s);
  auto segs = segmentDifference(diff);

  mergeRegions(segs);

  for (auto i = segs.begin(); i != segs.end();) {
    auto seg = *i;
    auto tps = tagPathSequence.substr(seg.first, seg.second - seg.first + 1);
    std::reverse(tps.begin(), tps.end());
    auto c = contour(tps);
    auto d = difference(c);
    auto ss = segmentDifference(d);
    if (ss.size() > 1) {
      std::reverse(ss.begin(), ss.end());
      for (auto j : ss) {
        j.first = (tps.size() - 1 - j.first) + seg.first;
        j.second = (tps.size() - 1 - j.second) + seg.first;
        std::swap(j.first, j.second);
        segs.insert(i, j);
      }
      i = segs.erase(i);
    } else
      ++i;
  }

  mergeRegions(segs);

  for (auto seg : segs) {
    DSREDataRegion r;

    r.setStartPos(seg.first);
    r.setEndPos(seg.second);
    //r.setTps(tagPathSequence.substr(r.getStartPos(), r.size()));
    dataRegions.emplace_back(r);
  }

  // correct region boundaries
  for (auto &r : dataRegions) {
    std::set<int> alpha;

    symbolFrequency(r.getTps(), alpha);

    /*auto minTPC = *alpha.begin();
     auto maxTPC = *alpha.rbegin();*/

    // pad left
    //while (r.getStartPos() > 0 && tagPathSequence[r.getStartPos() - 1] >= minTPC && tagPathSequence[r.getStartPos() - 1] <= maxTPC) {
    while (r.getStartPos() > 0
        && alpha.count(tagPathSequence[r.getStartPos() - 1]) > 0) {
      r.shiftStartPos(-1);
    }

    // pad right
    //while ((static_cast<size_t>(r.getEndPos()) < tagPathSequence.size()-1) && tagPathSequence[r.getEndPos()+1] >= minTPC && tagPathSequence[r.getEndPos()+1] <= maxTPC) {
    while ((r.getEndPos() < tagPathSequence.size() - 1)
        && alpha.count(tagPathSequence[r.getEndPos() + 1]) > 0) {
      r.shiftEndPos(+1);
    }

    r.setTps(tagPathSequence.substr(r.getStartPos(), r.size()));
  }
}

std::vector<size_t> DSRE::detectStructure() {
  float angCoeffThreshold = 0.17453;  // 10 degrees
  auto sizeThreshold = (tagPathSequence.size() * 3) / 100;  // % page size
  std::vector < size_t > structured;

  for (size_t i = 0; i < dataRegions.size(); ++i) {
    auto &r = dataRegions[i];
    //for (auto &r:dataRegions) {
    r.detectStructure();
    auto lr = r.getLinearRegression();

    std::cerr << "size: " << r.size() << " ang.coeff.: " << lr.a << std::endl;

    if ((abs(lr.a) < angCoeffThreshold) &&  // test for structure
        (r.size() >= sizeThreshold)  // test for size
        ) {
      structured.emplace_back(i);

      auto firstNode = nodeSequence.begin() + r.getStartPos();
      auto lastNode = firstNode + r.size();
      r.assignNodeSequence(firstNode, lastNode);

      r.setStructured(true);
    }
  }
  return structured;
}

std::set<size_t> DSRE::locateRecords(DSREDataRegion &region) {
  std::wstring s = region.getTps();
  std::set < size_t > result;

  std::vector<double> signal(s.begin(), s.end());

  auto dc = mean(signal);

  for (auto &i : signal)
    i -= dc;

  auto spectrum = fct(signal);
  auto spectrumSd = stddev(spectrum);
  double rcount = 0;

  {
	  auto ss = signal;
	  ss.resize(signal.size() + 4000, 0);
	  auto f = fft(ss);
	  auto fSD = stddev(f);
	  double maxAmpl = std::numeric_limits<double>::min();
	  size_t peakFreq = 0;
	  for (size_t i = 1; i < f.size()/2; ++i) {
		  if (f[i] > maxAmpl) {
			  maxAmpl = f[i];
			  peakFreq = i;
		  }
	  }
	  rcount = ((double)peakFreq*(double)signal.size())/(double)ss.size();
	  std::cerr << peakFreq << std::endl;
  }

  std::set<int> symbols(s.begin(), s.end());

  for (auto symbol : symbols) {
    size_t pos = 0;
    std::vector < size_t > recpos;
    while ((pos = s.find(symbol, pos)) != std::string::npos)
      recpos.emplace_back(pos++);

    if (recpos.size() > 2) {
      auto recsize = difference(recpos);
      std::sort(recsize.begin(), recsize.end());
      auto m = mean(recsize);
      auto sd = stddev(recsize);
      auto cv = sd / m;
      double regionCoverage = (double)std::min((size_t)(recpos[recpos.size()-1] - recpos[0] + m), s.size())/(double)s.size();
      auto firstPos = recpos.begin();
      auto lastPos = recpos.end() - 1;
      if (s[*firstPos] != symbol)
        ++firstPos;
      if (s[*lastPos] == symbol)
        ++lastPos;
      std::cerr << "CV: " << cv << ", RC: " << regionCoverage << ", #: " << rcount << std::endl;
      if (
    		  std::distance(firstPos, lastPos) > 1 &&
    		  cv < 0.3 &&
			  regionCoverage > 0.80
      	  ) {
        bool foundPeak = false;
        std::for_each(spectrum.begin() + 2*(recpos.size()-1) - 3,
                      spectrum.begin() + 2*(recpos.size()-1) + 3,
                      [&foundPeak, spectrumSd](auto p)
                      {
                        if (abs(p) > 3.0*spectrumSd)
                        foundPeak = true;
                      });
        if (foundPeak) {
          result.insert(firstPos, lastPos);
          break;
        }
      }
    }
  }

  return result;
}

/*
 std::set<size_t> DSRE::locateRecords(size_t regNum) {
 auto &region = dataRegions[regNum];
 std::wstring s = region.getTps();
 std::vector<double> signal(s.size());
 float avg;
 std::set<size_t> recpos,ret;
 std::set<int> alphabet;
 std::unordered_map<int,int> reencode;
 double maxScore = 0;

 // reencode signal
 symbolFrequency(s,alphabet);
 for (size_t i=0; i < s.size(); ++i) {
 auto c = s[i];

 if (reencode.count(c) == 0)
 reencode[c]=reencode.size()+1;
 signal[i]=reencode[c];
 //signal[i]=c;
 }

 avg = mean(signal);

 std::set<double> candidates;
 // remove DC & compute signal's std.dev.
 double stddev = 0;
 for (size_t i=0; i < s.size(); ++i) {
 signal[i] = signal[i] - avg;
 stddev += signal[i]*signal[i];
 //if (signal[i] < 0)
 candidates.insert(signal[i]);
 }

 region.setStdDev(sqrt(stddev/std::max((double)1,(double)(s.size()-2))));

 auto estPeriod = estimatePeriod(signal);
 region.setEstPeriod(estPeriod);
 region.setPeriodEstimator(periodEstimator);

 if (estPeriod == 0)
 return ret;

 auto estFreq = ((double)signal.size() / estPeriod);

 for (auto value:candidates) {
 double stddev = 0, avgsize = 0, avgSizeDiff = 0;

 recpos.clear();
 for (size_t i=0; i < s.size(); ++i) {
 if ((signal[i] == value))
 recpos.insert(i);
 }

 if (recpos.size() > 1) {
 auto prev = recpos.begin();
 for (auto rp = ++(recpos.begin()); rp != recpos.end(); ++rp, ++prev) {
 avgsize += (*rp) - (*prev);
 avgSizeDiff += std::abs(((*rp) - (*prev)) - estPeriod);
 }
 avgsize /= (float)(recpos.size()-1);

 prev = recpos.begin();
 for (auto rp = ++(recpos.begin()); rp != recpos.end(); ++rp, ++prev) {
 float diff = (float)((*rp) - (*prev)) - avgsize;
 stddev += diff * diff;
 }
 stddev = sqrt(stddev/std::max((float)(recpos.size()-2),(float)1));

 //auto cv = 1.0 - std::min(stddev, avgsize)/std::max(stddev, avgsize);
 double regionCoverage =
 (double)(*recpos.rbegin() - *recpos.begin()) /
 (double)(signal.size());

 double freqRatio =
 std::min( (double)recpos.size() ,estFreq) /
 std::max( (double)recpos.size() ,estFreq);

 double recSizeRatio = 1 - (
 std::min( avgSizeDiff, (double)(signal.size()) )/
 std::max( avgSizeDiff, (double)(signal.size()) ));
 //std::min( avgsize, estPeriod )/
 //std::max( avgsize, estPeriod );

 double score = (regionCoverage+recSizeRatio+freqRatio)/3.0;
 fprintf(stderr, "value=%d, cov=%.2f, #=%.2f, size=%.2f, s=%.4f - %.2f\n",int(value+avg),regionCoverage,freqRatio,recSizeRatio,score,estPeriod);

 double haltScore = 0.75;
 if (score > maxScore) {
 maxScore = score;
 ret = recpos;
 if (regionCoverage > haltScore &&
 recSizeRatio > haltScore &&
 freqRatio > haltScore)
 break;
 }
 }
 }

 return ret.size()?ret:recpos;
 }
 */

double DSRE::estimatePeriod(const std::vector<double> &signal) {
  return periodEstimatorPtr->estimate(signal);
}

void DSRE::setPeriodEstimator(PeriodEstimator strategy) {
  switch (strategy) {
    case PeriodEstimator::DCT:
      periodEstimatorPtr.reset(new DCTPeriodEstimator());
      periodEstimator = strategy;
      break;
    case PeriodEstimator::ModifiedDCT:
      periodEstimatorPtr.reset(new ModifiedDCTPeriodEstimator());
      periodEstimator = strategy;
      break;
    case PeriodEstimator::DFT:
      periodEstimatorPtr.reset(new DFTPeriodEstimator());
      periodEstimator = strategy;
      break;
    case PeriodEstimator::StdDev3:
      periodEstimatorPtr.reset(new StdDev3PeriodEstimator());
      periodEstimator = strategy;
      break;
  }
}

void DSRE::pruneRecords(DSREDataRegion &region, std::set<size_t> &recpos) {
  // consider only leaf nodes when performing field alignment
  std::vector<bool> remove(region.getTps().size());
  size_t pos = 0;

  region.eraseNodeSequence([&remove, recpos, &pos](const pNode &n)->bool
  {
    remove[pos] = !(n->isImage() || n->isLink() || n->isText());
    remove[pos] = remove[pos] && (recpos.count(pos) == 0);
    return remove[pos++];  // remove node if not content and not record start
    });

  pos = 0;
  region.eraseTPS([remove, &pos](wchar_t c)->bool
  {
    return remove[pos++];
  });

  // adjust records start position after removing nodes
  std::unordered_map < size_t, size_t > newRecPos;
  for (auto r : recpos)
    newRecPos[r] = r;

  for (size_t i = 0; i < remove.size(); ++i) {
    if (remove[i]) {
      for (auto rp = std::upper_bound(recpos.begin(), recpos.end(), i);
          rp != recpos.end(); ++rp)
        --newRecPos[*rp];
    }
  }

  recpos.clear();
  for (auto r : newRecPos)
    recpos.insert(r.second);
}

void DSRE::alignRecords(DSREDataRegion &region, std::set<size_t> &recpos) {
  std::vector < std::wstring > m;

  // create a sequence for each record found
  size_t max_size = 0;
  if (recpos.size() > 1) {
    auto prev = recpos.begin();
    for (auto rp = ++(recpos.begin()); rp != recpos.end(); ++rp, ++prev) {
      if (((*rp) - (*prev)) > 0) {
        m.emplace_back(region.getTps().substr(*prev, (*rp) - (*prev)));
        max_size = std::max((*rp) - (*prev), max_size);
      }
    }

    if (*prev < region.getTps().size() - 1)
      m.emplace_back(region.getTps().substr(*prev, max_size));
  }

  if (m.size() > 1 && m[0].size() > 1) {
    // align the records (one alternative to 'center star' algorithm is ClustalW)
    //align(m);
    std::cerr << "1-RxC = " << m.size() << " " << m[0].size() << std::endl;
    region.setScore(centerStar(m));
    std::cerr << "2-RxC = " << m.size() << " " << m[0].size() << std::endl;

    // and extracts them
    extractRecords(m, recpos, region);
  }
}

void DSRE::rankRegions(const std::vector<size_t>& structured) {
  if (structured.size()) {
    // compute content score
    float tpsSize = tagPathSequence.size();
    float tpsCenter = (tpsSize - 1) / 2;
    float maxDistance = tpsSize / 2;

    for (auto i : structured) {
      float recCount = dataRegions[i].recordCount();
      float recSize = dataRegions[i].recordSize();
      /*auto stddev = regs[i].stddev;*/
      float regionCenter = dataRegions[i].getStartPos()
          + (dataRegions[i].size() / 2);

      /*regs[i].score = log(//stddev;
       ((min((double)recCount,(double)recSize) /
       max((double)recCount,(double)recSize))) *
       stddev *
       ((double)regs[i].len / (double)tagPathSequence.size()));
       //(double)recCount * (double)recSize * stddev;
       */

      float positionScore = 1 - (abs(tpsCenter - regionCenter) / maxDistance);
      float sizeScore = dataRegions[i].size() / tpsSize;
      float recScore = std::min(recCount, recSize)
          / std::max(recCount, recSize);

      dataRegions[i].setScore((positionScore + sizeScore + recScore) / 3);
    }

    // k-mean clustering to identify content
    std::vector<double> ckmeansScoreInput;
    ckmeansScoreInput.push_back(0);
    for (auto i : structured)
      ckmeansScoreInput.push_back(dataRegions[i].getScore());  // cluster by score
    auto scoreResult = kmeans_1d_dp(ckmeansScoreInput, 2, 2);

    auto j = ++(scoreResult.cluster.begin());
    for (auto i : structured) {
      dataRegions[i].setContent((((*j) == 2) || (scoreResult.nClusters < 2)));

      // restore the original region's tps
      dataRegions[i].setTps(
          tagPathSequence.substr(dataRegions[i].getStartPos(),
                                 dataRegions[i].size()));
      ++j;
    }

    /*ckmeansScoreInput.clear();
     ckmeansScoreInput.push_back(0);
     for (auto i:structured)
     ckmeansScoreInput.push_back(regs[i].stddev); // cluster by stddev
     scoreResult = kmeans_1d_dp(ckmeansScoreInput,2,2);

     j=++(scoreResult.cluster.begin());
     for (auto i:structured) {
     regs[i].content |= (((*j) == 2) || (scoreResult.nClusters < 2));
     ++j;
     }*/
  }

  auto drEnd = std::remove_if(
      dataRegions.begin(), dataRegions.end(), [](const DSREDataRegion &a)
      {
        return !a.isStructured() || a.recordCount() < 2 || a.recordSize() < 2;
      });
  dataRegions.erase(drEnd, dataRegions.end());

  sort(dataRegions.begin(), dataRegions.end(),
       [](const DSREDataRegion &a, const DSREDataRegion &b)
       {
         if (a.isContent() == b.isContent())
         return (a.getScore() > b.getScore());
         else
         {
           if (a.isContent())
           return true;
           else
           return false;
         }
       });
}

void DSRE::extractRecords(std::vector<std::wstring> &m,
                          std::set<size_t> &recpos, DSREDataRegion &region) {
  if ((m.size() == 0) || (recpos.size() == 0))
    return;

  int rows = m.size(), cols = m[0].size();

  auto rp = recpos.begin();
  for (int i = 0; i < rows; i++, ++rp) {
    Record rec;

    std::cerr << std::endl;
    for (int j = 0, k = 0; j < cols; j++) {
      if (m[i][j] != 0) {
        auto node = region.getNodeSequence()[(*rp) + k];
        rec.emplace_back(node);

        auto tagName = node->getTagName();
        std::cerr << "[" << node->toString() << "]; ";

        k++;
      } else
        rec.emplace_back(nullptr);
    }
    region.addRecord(rec);
  }
  region.cleanup();
  std::cerr << std::endl;
}

double DFTPeriodEstimator::estimate(std::vector<double> signal) {
  size_t N = (signal.size() + (signal.size() % 2));

  if (signal.size() != N) {  // repeat last sample when signal size is odd
    signal.resize(N);
    signal[N - 1] = signal[N - 2];
  }

  auto spectrum = fft(signal);

  double freq = 1;
  double power = spectrum[1];
  for (size_t i = 1; i < spectrum.size() / 2; ++i) {
    if (spectrum[i] > power) {
      freq = (double) (i);
      power = spectrum[i];
    }
  }
  return (double) (N) / (double) (freq);
}

double DCTPeriodEstimator::estimate(std::vector<double> signal) {
  size_t N = (signal.size() + (signal.size() % 2));

  if (signal.size() != N) {
    signal.resize(N);
    signal[N - 1] = signal[N - 2];
  }

  auto spectrum = fct(signal);

  double freq = 1;
  double power = spectrum[1];
  for (size_t i = 1; i < spectrum.size() / 4; ++i) {
    if (spectrum[i] > power) {
      freq = (double) (i) / 2.0;
      power = spectrum[i];
    }
  }
  return (double) (N) / (double) (freq);
}

double ModifiedDCTPeriodEstimator::estimate(std::vector<double> signal) {
  size_t N = (signal.size() + (signal.size() % 2));

  if (signal.size() != N) {  // repeat last sample when signal size is odd
    signal.resize(N);
    signal[N - 1] = signal[N - 2];
  }

  auto spectrum = fct(signal);

  for (size_t i = 0; i < spectrum.size() - 3; ++i)
    spectrum[i] = abs(spectrum[i] - spectrum[i + 2]);

  spectrum.resize(signal.size() - 2);

  double freq = 1;
  double power = spectrum[1];
  for (size_t i = 1; i < spectrum.size() / 4; ++i) {
    if (spectrum[i] > power) {
      freq = (double) (i) / 2.0;
      power = spectrum[i];
    }
  }
  return (double) (N) / (double) (freq);
}

double StdDev3PeriodEstimator::estimate(std::vector<double> signal) {
  size_t N = (signal.size() + (signal.size() % 2));

  if (signal.size() != N) {  // repeat last sample when signal size is odd
    signal.resize(N);
    signal[N - 1] = signal[N - 2];
  }

  auto spectrum = fct(signal);
  auto std3 = stddev(spectrum) * 4;

  double freq = 0;
  for (size_t i = 1; i < spectrum.size() / 4; ++i) {
    if (abs(spectrum[i]) > std3) {
      freq = (double) (i) / 2.0;
      break;
    }
  }
  if (freq != 0)
    return (double) (N) / (double) (freq);
  else
    return 0;
}
