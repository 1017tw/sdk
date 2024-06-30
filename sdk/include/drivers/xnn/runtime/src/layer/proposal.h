#ifndef LAYER_PROPOSAL_H
#define LAYER_PROPOSAL_H

#include "layer.h"

namespace xnn {

class Proposal : public Layer
{
public:
    Proposal();

    virtual int load_param(const ParamDict& pd);

    virtual int forward(const std::vector<Mat>& bottom_blobs, std::vector<Mat>& top_blobs, const Option& opt) const;

public:
    // param
    int feat_stride;
    int base_size;
    int pre_nms_topN;
    int after_nms_topN;
    float nms_thresh;
    int min_size;

    Mat ratios;
    Mat scales;

    Mat anchors;
};

} // namespace xnn

#endif // LAYER_PROPOSAL_H
