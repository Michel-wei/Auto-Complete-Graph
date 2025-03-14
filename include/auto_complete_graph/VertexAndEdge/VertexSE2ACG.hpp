// g2o - General Graph Optimization
// Copyright (C) 2011 R. Kuemmerle, G. Grisetti, W. Burgard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef AUTOCOMPLETEGRAPH_VERTEXSE2ACG_12032018
#define AUTOCOMPLETEGRAPH_VERTEXSE2ACG_12032018

#include "g2o/config.h"
#include "g2o/core/base_vertex.h"
#include "g2o/core/hyper_graph_action.h"
#include "g2o/types/slam2d/se2.h"
//#include "g2o/types/slam2d/g2o_types_slam2d_api.h"

namespace g2o {

/**
 * \brief 2D pose Vertex, (x,y,theta)
 */
class VertexSE2ACG : public g2o::BaseVertex<3, SE2> {
   public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    VertexSE2ACG();

    virtual void setToOriginImpl() { _estimate = SE2(); }

    virtual void oplusImpl(const number_t* update) {
        Vector2 t = _estimate.translation();
        t += Eigen::Map<const Vector2>(update);
        number_t angle =
            normalize_theta(_estimate.rotation().angle() + update[2]);
        _estimate.setTranslation(t);
        _estimate.setRotation(Rotation2D(angle));
    }

    virtual bool setEstimateDataImpl(const number_t* est) {
        _estimate = SE2(est[0], est[1], est[2]);
        return true;
    }

    virtual bool getEstimateData(number_t* est) const {
        Eigen::Map<Vector3> v(est);
        v = _estimate.toVector();
        return true;
    }

    virtual int estimateDimension() const { return 3; }

    virtual bool setMinimalEstimateDataImpl(const number_t* est) {
        return setEstimateData(est);
    }

    virtual bool getMinimalEstimateData(number_t* est) const {
        return getEstimateData(est);
    }

    virtual int minimalEstimateDimension() const { return 3; }

    virtual bool read(std::istream& is);
    virtual bool write(std::ostream& os) const;
};

}  // namespace g2o

#endif
