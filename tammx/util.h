// Copyright 2016 Pacific Northwest National Laboratory

#ifndef TAMMX_UTIL_H__
#define TAMMX_UTIL_H__

#include <cassert>
#include <iosfwd>
#include <cmath>
#include <map>
#include <string>
#include "tammx/strong_int.h"
#include "tammx/boundvec.h"
#include "tammx/types.h"

namespace tammx {

template<typename T>
TensorVec<T>
flatten(const TensorVec<TensorVec<T>> &vec) {
  TensorVec<T> ret;
  for(auto &v : vec) {
    ret.insert_back(v.begin(), v.end());
  }
  return ret;
}

/**
 * @todo @bug Check if this works when a label already presented is
 * updated
 */
template<typename T>
class LabelMap {
 public:
  LabelMap& update(const TensorLabel& labels,
                   const TensorVec<T>& ids) {
    // Expects(labels.size() + labels_.size()  <= labels_.max_size());
    // labels_.insert_back(labels.begin(), labels.end());
    // ids_.insert_back(ids.begin(), ids.end());
    Expects(labels.size() == ids.size());
    for(int i=0; i<labels.size(); i++) {
      lmap_[labels[i]] = ids[i];
    }
    return *this;
  }

  TensorVec<T> get_blockid(const TensorLabel& labels) const {
    TensorVec<T> ret;
    for(auto l: labels) {
      auto itr = lmap_.find(l);
      Expects(itr != lmap_.end());
      ret.push_back(itr->second);
    }
    return ret;
    // TensorVec<T> ret(labels.size());
    // using size_type = TensorLabel::size_type;
    // for(size_type i=0; i<labels.size(); i++) {
    //   auto itr = std::find(begin(labels_), end(labels_), labels[i]);
    //   Expects(itr != end(labels_));
    //   ret[i] = ids_[itr - begin(labels_)];
    // }
    // return ret;
  }

 private:
  // TensorRank rank_{};
  // TensorLabel labels_;
  // TensorVec<T> ids_;
  std::map<IndexLabel, T> lmap_;
};

inline TensorPerm
perm_compute(const TensorLabel& from, const TensorLabel& to) {
  TensorPerm layout;

  std::cerr<<__FUNCTION__<<" from="<<from<<std::endl;
  std::cerr<<__FUNCTION__<<" to="<<to<<std::endl;

  Expects(from.size() == to.size());
  for(auto p : to) {
    auto itr = std::find(from.begin(), from.end(), p);
    Expects(itr != from.end());
    layout.push_back(itr - from.begin());
  }
  return layout;
}

inline int
perm_count_inversions(const TensorPerm& perm) {
  int num_inversions = 0;
  TensorPerm perm_sort{perm};
#if 0
  std::sort(perm_sort.begin(), perm_sort.end());
  Expects(std::adjacent_find(perm_sort.begin(), perm_sort.end()) == perm_sort.end());
  using size_type = TensorPerm::size_type;
  for(size_type i=0; i<perm.size(); i++) {
    auto itr = std::find(perm_sort.begin(), perm_sort.end(), perm[i]);
    Expects(itr != perm.end());
    num_inversions += std::abs((itr - perm.begin()) - i);
  }
#else
  std::sort(perm_sort.begin(), perm_sort.end());
  Expects(std::adjacent_find(perm_sort.begin(), perm_sort.end()) == perm_sort.end());
  //using size_type = TensorPerm::size_type;
  for(int i=0; i<perm.size(); i++) {
    auto itr = std::find(perm.begin(), perm.end(), i);
    Expects(itr != perm.end());
    num_inversions += std::abs((itr - perm.begin()) - i);
  }
#endif
  return num_inversions / 2;
}

template<typename T>
inline TensorVec<T>
perm_apply(const TensorVec<T>& label, const TensorPerm& perm) {
  TensorVec<T> ret;
  std::cerr<<__FUNCTION__<<":"<<__LINE__<<": label="<<label<<std::endl;
  std::cerr<<__FUNCTION__<<":"<<__LINE__<<": perm="<<perm<<std::endl;
  Expects(label.size() == perm.size());
  using size_type = TensorPerm::size_type;
  for(size_type i=0; i<label.size(); i++) {
    ret.push_back(label[perm[i]]);
  }
  return ret;
}

inline TensorPerm
perm_compose(const TensorPerm& p1, const TensorPerm& p2) {
  TensorPerm ret(p1.size());
  Expects(p1.size() == p2.size());
  for(unsigned i=0; i<p1.size(); i++) {
    ret[i] = p1[p2[i]];
  }
  return ret;
}

inline bool
is_permutation(TensorPerm perm) {
  std::sort(perm.begin(), perm.end());
  // return std::adjacent_find(perm.begin(), perm.end()) == perm.end();
  for(int i=0 ;i<perm.size(); i++) {
    if(perm[i] != i)
      return false;
  }
  return true;
}

inline TensorPerm
perm_invert(const TensorPerm& perm) {
  TensorPerm ret(perm.size());
  Expects(is_permutation(perm));
  for(unsigned i=0; i<perm.size(); i++) {
    auto itr = std::find(perm.begin(), perm.end(), i);
    Expects(itr != perm.end());
    ret[i] = itr - perm.begin();
  }
  return ret;
}

template<typename Container>
auto intersect(const Container &ctr1, const Container &ctr2) {
#if 0
  Container ret;
  for (auto &x : ctr1) {
    for (auto &y : ctr2) {
      if (x == y) {
        ret.push_back(x);
      }
    }
  }
  return ret;
#else
  Container ret;
  Container c1 = ctr1;
  Container c2 = ctr2;
  std::sort(c1.begin(), c1.end());
  std::sort(c2.begin(), c2.end());
  std::set_intersection(c1.begin(), c1.end(),
                        c2.begin(), c2.end(),
                        std::back_inserter(ret));
  return ret;
#endif
}

inline TensorVec<TensorLabel>
group_labels(const TensorVec<SymmGroup>& groups, const TensorLabel& labels) {
  // std::accumulate(groups.begin(), groups.end(), 0,
  //                 [] (const SymmGroup& sg, int sz) {
  //                   return sg.size() + sz;
  //                 });
  unsigned sz = 0;
  for(auto v : groups) {
    sz += v.size();
  }
  Expects(sz == labels.size());

  int pos = 0;
  TensorVec<TensorLabel> ret;
  for(auto sg : groups) {
    ret.push_back(TensorLabel{labels.begin()+pos, labels.begin()+pos+sg.size()});
    pos += sg.size();
  }
  return ret;
}



inline TensorVec<TensorVec<TensorLabel>>
group_partition(const TensorVec<TensorLabel>& label_groups_1,
                const TensorVec<TensorLabel>& label_groups_2) {
  TensorVec<TensorVec<TensorLabel>> ret_labels;
  for (auto &lg1 : label_groups_1) {
    TensorVec<TensorLabel> ret_group;
    for (auto &lg2 : label_groups_2) {
      auto lbls = intersect(lg1, lg2);
      if (lbls.size() > 0) {
        ret_group.push_back(lbls);
      }
    }
    ret_labels.push_back(ret_group);
  }
  return ret_labels;
}

inline TensorVec<TensorVec<TensorLabel>>
group_partition(const TensorVec<SymmGroup>& indices1,
                const TensorLabel& label1,
                const TensorVec<SymmGroup>& indices2,
                const TensorLabel& label2) {
  auto label_groups_1 = group_labels(indices1, label1);
  auto label_groups_2 = group_labels(indices2, label2);
  return group_partition(label_groups_1, label_groups_2);
}

template<typename T>
inline std::string
to_string(T val) {
  return std::to_string(val);
}

inline std::string
to_string(const IndexLabel& lbl) {
  return to_string(lbl.dt) + to_string(lbl.label);
}

template<typename T, int maxsize>
std::string to_string(const BoundVec<T,maxsize> &vec, const std::string& sep = ",") {
  std::string ret;
  for(int i=0; i<vec.size()-1; i++) {
    ret += to_string(vec[i]) + sep;
  }
  if(vec.size()>0) {
    ret += to_string(vec.back());
  }
  return ret;
}


}; //namespace tammx


#endif  // TAMMX_UTIL_H__

