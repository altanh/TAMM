// Copyright 2016 Pacific Northwest National Laboratory

#ifndef TAMMY_TENSOR_BASE_H_
#define TAMMY_TENSOR_BASE_H_

#include <vector>
#include <numeric>

#include "index_space.h"
#include "types.h"
#include "errors.h"

namespace tammy {

class TensorBase {
 public:
  TensorVec<IndexRange> dim_ranges_;
  TensorVec<IndexPosition> ipmask_;
  Irrep irrep_;
  Spin spin_total_;
  bool has_spatial_symmetry_;
  bool has_spin_symmetry_;
  TensorVec<SpinMask> spin_mask_;

  TensorBase(const TensorVec<IndexRange>& dim_ranges,
             const TensorVec<IndexPosition>& ipmask,
             Irrep irrep = Irrep{0},
             Spin spin_total = Spin{0})
      : dim_ranges_{dim_ranges},
        ipmask_{ipmask},
        irrep_{irrep},
        spin_total_{spin_total},
        has_spatial_symmetry_{true},
        has_spin_symmetry_{false} {
    init_spatial_check();
    init_spin_check();
  }

  TensorBase(const std::tuple<
             TensorVec<IndexRange>,
             TensorVec<IndexRange>>& ranges,
             Irrep irrep = Irrep{0},
             Spin spin_total = Spin{0})
      : TensorBase{TensorBase::compute_index_range(ranges),
        compute_ipmask(ranges),
        irrep,
        spin_total} { }

  TensorBase(const std::tuple<
             TensorVec<IndexRange>,
             TensorVec<IndexRange>,
             TensorVec<IndexRange>>& ranges,
             Irrep irrep = Irrep{0},
             Spin spin_total = Spin{0})
      : TensorBase{compute_index_range(ranges),
        compute_ipmask(ranges),
        irrep,
        spin_total} { }

  bool spatial_nonzero(const BlockDimVec& bdv) const {
    if(!has_spatial_symmetry_)
      return true;

    Irrep rhs{0};
    for(size_t i=0; i<bdv.size(); i++) {
      if(ipmask_[i] == IndexPosition::upper
         || ipmask_[i] == IndexPosition::lower) {
        rhs ^= dim_ranges_[i].is().spatial(bdv[i]);
      }
    }
    return rhs == irrep_;
  }

  bool spin_nonzero(const BlockDimVec& bdv) const {
    if(!has_spin_symmetry_)
      return true;

    Spin spin_upper{0}, spin_lower{0};
    for(size_t i=0; i<bdv.size(); i++) {
      switch(spin_mask_[i]) {
        case SpinMask::upper:
          spin_upper += dim_ranges_[i].is().spin(bdv[i]);
          break;
        case SpinMask::lower:
          spin_lower += dim_ranges_[i].is().spin(bdv[i]);
          break;
        case SpinMask::ignore:
          //no-op
          break;
        default:
          break;
      }
    }
    return spin_upper - spin_lower == spin_total_;
  }

 private:
  void init_spatial_check() {
    for(size_t i=0; i<dim_ranges_.size(); i++) {
      if(ipmask_[i] == IndexPosition::upper
         || ipmask_[i] == IndexPosition::lower) {
        if(!dim_ranges_[i].is().has_spatial()) {
          has_spatial_symmetry_ = false;
        }
      }
    }
  }

  void init_spin_check() {
    spin_mask_.clear();
    for(const auto ip : ipmask_) {
      switch(ip) {
        case IndexPosition::upper:
          spin_mask_.push_back(SpinMask::upper);
          break;
        case IndexPosition::lower:
          spin_mask_.push_back(SpinMask::lower);
          break;
        case IndexPosition::neither:
          spin_mask_.push_back(SpinMask::ignore);
          break;
        default:
          break;
      }
    }
    auto u = std::find(ipmask_.begin(), ipmask_.end(), IndexPosition::upper);
    auto l = std::find(ipmask_.begin(), ipmask_.end(), IndexPosition::lower);
    while(u != ipmask_.end() && l != ipmask_.end()) {
      auto upos = u - ipmask_.begin();
      auto lpos = l - ipmask_.begin();
      if(!dim_ranges_[upos].is().has_spin() ||
         !dim_ranges_[lpos].is().has_spin()) {
        spin_mask_[upos] = SpinMask::ignore;
        spin_mask_[lpos] = SpinMask::ignore;
      }
      u = std::find(u+1, ipmask_.end(), IndexPosition::upper);
      l = std::find(l+1, ipmask_.end(), IndexPosition::lower);
    }
  }

  static TensorVec<IndexRange>
  compute_index_range(const std::tuple<
                      TensorVec<IndexRange>,
                      TensorVec<IndexRange>,
                      TensorVec<IndexRange>>& ranges) {
    TensorVec<IndexRange> ret;
    std::copy(std::get<0>(ranges).begin(),
              std::get<0>(ranges).end(),
              std::back_inserter(ret));
    std::copy(std::get<1>(ranges).begin(),
              std::get<1>(ranges).end(),
              std::back_inserter(ret));
    std::copy(std::get<2>(ranges).begin(),
              std::get<2>(ranges).end(),
              std::back_inserter(ret));
    return ret;
  }

  static TensorVec<IndexRange>
  compute_index_range(const std::tuple<
                      TensorVec<IndexRange>,
                      TensorVec<IndexRange>>& ranges) {
    return compute_index_range(std::make_tuple(
        TensorVec<IndexRange>{},
        std::get<0>(ranges),
        std::get<1>(ranges)));
  }

  static TensorVec<IndexPosition>
  compute_ipmask(const std::tuple<
                 TensorVec<IndexRange>,
                 TensorVec<IndexRange>,
                 TensorVec<IndexRange>>& ranges) {
    TensorVec<IndexPosition> ret;
    std::fill_n(std::back_inserter(ret),
                std::get<0>(ranges).size(),
                IndexPosition::neither);
    std::fill_n(std::back_inserter(ret),
                std::get<1>(ranges).size(),
                IndexPosition::upper);
    std::fill_n(std::back_inserter(ret),
                std::get<2>(ranges).size(),
                IndexPosition::lower);
    return ret;
  }

  static TensorVec<IndexPosition>
  compute_ipmask(const std::tuple<
                 TensorVec<IndexRange>,
                 TensorVec<IndexRange>>& ranges) {
    return compute_ipmask(std::make_tuple(
        TensorVec<IndexRange>{},
        std::get<0>(ranges),
        std::get<1>(ranges)));
  }
};  // TensorBase


}  // namespace tammy

#endif  // TAMMY_TENSOR_BASE_H_
