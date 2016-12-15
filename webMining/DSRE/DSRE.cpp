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
		  "getMinZScore",&DSRE::getMinZScore,
		  "setMinZScore",&DSRE::setMinZScore
		  );

  DSREDataRegion::luaBinding(lua);
}

DSRE::DSRE()
    : TPSExtractor<DSREDataRegion>(), minZScore(3.0) {
}

DSRE::~DSRE() {
}

void DSRE::extract(pDOM dom) {
  cleanup();
  std::cerr << std::endl << "Processing " << dom->getURI() << " ..."
            << std::endl;

  buildTagPath("", dom->body(), true);
  segment();
  correctBoundaries();
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
  std::list<std::pair<size_t, size_t> > ret;
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

void DSRE::correctBoundaries() {
  for (auto &r : dataRegions) {
    std::set<int> alpha;

    symbolFrequency(r.getTps(), alpha);

    /*auto minTPC = *alpha.begin();
     auto maxTPC = *alpha.rbegin();*/

    // pad left
    //while (r.getStartPos() > 0 && tagPathSequence[r.getStartPos() - 1] >= minTPC && tagPathSequence[r.getStartPos() - 1] <= maxTPC) {
    while (r.getStartPos() > 0
        && alpha.count(tagPathSequence[r.getStartPos() - 1]) > 0)
      r.shiftStartPos(-1);

    // pad right
    //while (r.getEndPos() < tagPathSequence.size()-1 && tagPathSequence[r.getEndPos()+1] >= minTPC && tagPathSequence[r.getEndPos()+1] <= maxTPC) {
    while ((r.getEndPos() < tagPathSequence.size() - 1)
        && alpha.count(tagPathSequence[r.getEndPos() + 1]) > 0)
      r.shiftEndPos(+1);

    r.refreshTps();
  }
}

void DSRE::segment() {
  auto s = contour(tagPathSequence);
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
    DSREDataRegion r(tagPathSequence, nodeSequence);

    r.setStartPos(seg.first);
    r.setEndPos(seg.second);
    r.refreshTps();
    dataRegions.emplace_back(r);
  }
}

#define ANGLE 4.5
std::vector<size_t> DSRE::detectStructure() {
  float angCoeffThreshold = ANGLE*M_PI/180.0;  // convert angle to radians
  auto sizeThreshold = (tagPathSequence.size() * 3) / 100;  // % page size
  std::vector<size_t> structured;

  for (size_t i = 0; i < dataRegions.size(); ++i) {
    auto &r = dataRegions[i];
    r.detectStructure();
    auto lr = r.getLinearRegression();

    std::cerr << "size: " << r.size() << " ang.coeff.: " << lr.a << std::endl;

    if ((std::abs(lr.a) < angCoeffThreshold) &&  // test for structure
        (r.size() >= sizeThreshold)  // test for size
        ) {
      structured.emplace_back(i);
      r.setStructured(true);
    }
  }
  return structured;
}

#define PADDING 2.0
#define INTERVAL 3.0

std::set<size_t> DSRE::locateRecords(DSREDataRegion &region) {
  std::wstring s = region.getTps();
  std::set<size_t> result;
  std::set<int> symbols(s.begin(), s.end());

  for (auto symbol : symbols) {
    size_t pos = 0;
    std::vector<size_t> recpos;
    while ((pos = s.find(symbol, pos)) != std::string::npos)
      recpos.emplace_back(pos++);

    if (recpos.size() > 2) {
      auto recsize = difference(recpos);
      auto m = mean(recsize);
      auto sd = stddev(recsize);
      auto cv = sd / m;
      auto firstRec = recpos.begin();
      auto lastRec = recpos.rbegin();
      auto regionEnd = std::min((size_t) (*lastRec + m), s.size() - 1);
      auto regionSize = regionEnd - *firstRec + 1;
      double regionCoverage = (double) regionSize / (double) s.size();
      std::cerr << "sym: " << symbol << "#" << recpos.size() << ", " << "CV: "
                << cv << ", RC: " << regionCoverage << "(" << *firstRec << ";"
                << (*lastRec) + m << ")";
      if (m > 2 && cv < 0.3 && regionCoverage > 0.80) {
        auto firstPos = s.begin();
        auto lastPos = s.begin();

        std::advance(firstPos, *firstRec);
        std::advance(lastPos, regionEnd);
        std::vector<double> signal(firstPos, lastPos);
        auto originalSize = signal.size();

        auto dc = mean(signal);

        for (auto &i : signal) // remove DC/mean
          i -= dc;


        hannWindow(signal); // smooth signal
        signal.insert(signal.end(), signal.begin(), signal.end()); // 2x
        signal.insert(signal.end(), signal.begin(), signal.end()); // 4x
        signal.insert(signal.end(), signal.begin(), signal.end()); // 8x
        signal.resize(signal.size() * PADDING, 0);  // zero pad

        auto psd = fft(signal); // transform
        auto psdSD = stddev(psd);
        auto psdMean = mean(psd);
        double transformScale = (double) signal.size()
            / (double) originalSize;
        for (auto &i : psd)  // convert to Z Score
          i = (i - psdMean) / psdSD;

        double peakFreq = -1;
        auto firstFreq = psd.begin();
        auto lastFreq = psd.begin();
        std::advance(
            firstFreq,
			//(std::max((double)recpos.size(), INTERVAL) - INTERVAL) * transformScale
            std::max(recpos.size() * transformScale, INTERVAL) - INTERVAL
			) ;
        std::advance(
        		lastFreq,
				//((recpos.size() + INTERVAL) * transformScale) + 1
				(recpos.size() * transformScale) + INTERVAL+1.0
				);

        for (auto p = firstFreq; p != lastFreq; ++p) {
          if (std::abs(*p) > minZScore) {
            peakFreq = *p;
            break;
          }
        }
        if (peakFreq > minZScore) {
          std::cerr << ", peak = " << peakFreq << std::endl;

          // adjust region to (recpos[0]; recpos[n-1])
          region.shiftStartPos(recpos[0]);
          region.shiftEndPos(-(s.size() - 1 - regionEnd));
          region.refreshTps();
          region.setTransform(psd);

          // correct recpos
          std::for_each(recpos.begin(), recpos.end(), [recpos](auto &a) {
            a -= recpos[0];
          });

          result.insert(recpos.begin(), recpos.end());
          break;
        }
      }
      std::cerr << std::endl;
    }
  }
  return result;
}

void DSRE::pruneRecords(DSREDataRegion &region, std::set<size_t> &recpos) {
  // consider only leaf nodes when performing record alignment
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
  std::unordered_map<size_t, size_t> newRecPos;
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
  std::vector<std::wstring> m;

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

      float positionScore = 1
          - (std::abs(tpsCenter - regionCenter) / maxDistance);
      float sizeScore = dataRegions[i].size() / tpsSize;
      float recScore = std::min(recCount, recSize)
          / std::max(recCount, recSize);

      dataRegions[i].setScore((positionScore + sizeScore + recScore) / 3);
    }

    // k-means clustering to identify content
    std::vector<double> ckmeansScoreInput;
    ckmeansScoreInput.push_back(0);
    for (auto i : structured)
      ckmeansScoreInput.push_back(dataRegions[i].getScore());  // cluster by score
    auto scoreResult = kmeans_1d_dp(ckmeansScoreInput, 2, 2);

    auto j = ++(scoreResult.cluster.begin());
    for (auto i : structured) {
      dataRegions[i].setContent((((*j) == 2) || (scoreResult.nClusters < 2)));

      // restore the original region's tps
      dataRegions[i].refreshTps();
      ++j;
    }
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

double DSRE::getMinZScore() const {
  return minZScore;
}

void DSRE::setMinZScore(double minZScore) {
  this->minZScore = minZScore;
}
