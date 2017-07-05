/******************************************************************************
* Copyright (C) 2017, Divideon. All rights reserved.
* No part of this code may be reproduced in any form
* without the written permission of the copyright holder.
******************************************************************************/

#ifndef XVC_COMMON_LIB_INTRA_PREDICTION_H_
#define XVC_COMMON_LIB_INTRA_PREDICTION_H_

#include <array>

#include "xvc_common_lib/coding_unit.h"
#include "xvc_common_lib/common.h"

namespace xvc {

class IntraPredictorLuma :
  public std::array<IntraMode, constants::kNumIntraMpm> {
public:
  int num_neighbor_modes = 0;
};
typedef std::array<IntraChromaMode,
  constants::kNumIntraChromaModes> IntraPredictorChroma;

class IntraPrediction {
public:
  static const ptrdiff_t kRefSampleStride_ = constants::kMaxBlockSize * 2 + 1;
  struct State {
    std::array<Sample, kRefSampleStride_ * 2> ref_samples = {};
    std::array<Sample, kRefSampleStride_ * 2> ref_filtered = {};
  };

  explicit IntraPrediction(int bitdepth) : bitdepth_(bitdepth) {}
  void Predict(IntraMode intra_mode, const CodingUnit &cu, YuvComponent comp,
               const Sample *input_pic, ptrdiff_t input_stride,
               Sample *output_buffer, ptrdiff_t output_stride);
  void Predict(IntraMode intra_mode, const CodingUnit &cu, YuvComponent comp,
               const State &ref_state, Sample *output_buffer,
               ptrdiff_t output_stride);
  State ComputeReferenceState(const CodingUnit &cu, YuvComponent comp,
                              const Sample *input_pic, ptrdiff_t input_stride);
  IntraPredictorLuma GetPredictorLuma(const CodingUnit &cu) const;
  IntraPredictorChroma GetPredictorsChroma(IntraMode luma_mode) const;

private:
  struct NeighborState {
    bool has_any() const {
      return has_above_left || has_above || has_left ||
        has_above_right > 0 || has_below_left > 0;
    }
    bool has_all(int width, int height) const {
      return has_above_left && has_above && has_left &&
        has_below_left == width && has_above_right == height;
    }
    bool has_above_left = false;
    bool has_above = false;
    int has_above_right = 0;
    bool has_left = false;
    int has_below_left = 0;
  };
  static const int8_t kAngleTable_[17];
  static const int16_t kInvAngleTable_[8];

  bool UseFilteredRefSamples(const CodingUnit &cu, IntraMode intra_mode);
  void PredIntraDC(int width, int height, bool dc_filter,
                   const Sample *ref_samples, ptrdiff_t ref_stride,
                   Sample *output_buffer, ptrdiff_t output_stride);
  void PlanarPred(int width, int height,
                  const Sample *ref_samples, ptrdiff_t ref_stride,
                  Sample *output_buffer, ptrdiff_t output_stride);
  void AngularPred(int width, int height, IntraMode mode, bool filter,
                  const Sample *ref_samples, ptrdiff_t ref_stride,
                  Sample *output_buffer, ptrdiff_t output_stride);
  NeighborState DetermineNeighbors(const CodingUnit &cu, YuvComponent comp);
  void ComputeRefSamples(int width, int height,
                         const NeighborState &neighbors,
                         const Sample *input, ptrdiff_t input_stride,
                         Sample *output, ptrdiff_t output_stride);
  void FilterRefSamples(int width, int height, const Sample *src_ref,
                        Sample *dst_ref, ptrdiff_t stride);

  int bitdepth_;
};

}   // namespace xvc

#endif  // XVC_COMMON_LIB_INTRA_PREDICTION_H_
