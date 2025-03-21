#ifndef AUTOCOMPLETEGRAPH_VERTEXSE2PRIOR_10112017
#define AUTOCOMPLETEGRAPH_VERTEXSE2PRIOR_10112017

#include "EdgeInterfaceMalcolm.hpp"
#include "EdgeSE2Prior.hpp"
#include "VertexSE2ACG.hpp"
#include "auto_complete_graph/PriorLoaderInterface.hpp"

namespace g2o {

class VertexSE2Prior : public g2o::VertexSE2ACG {
   protected:
   public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    VertexSE2Prior() : g2o::VertexSE2ACG() {}
    AASS::acg::PriorAttr priorattr;

    std::vector<std::pair<double, double>> getAnglesAndOrientations() const {
        std::vector<std::pair<double, double>> out;
        auto edges = this->edges();
        std::vector<g2o::EdgeSE2Prior_malcolm*> edges_prior;
        // Get only prior edges
        if (edges.size() > 1) {
            for (auto ite = edges.begin(); ite != edges.end(); ++ite) {
                g2o::EdgeSE2Prior_malcolm* ptr =
                    dynamic_cast<g2o::EdgeSE2Prior_malcolm*>(*ite);
                if (ptr != NULL) {
                    // Make sure not pushed twice
                    for (auto ite2 = edges_prior.begin();
                         ite2 != edges_prior.end(); ++ite2) {
                        assert(ptr != *ite2);
                    }
                    edges_prior.push_back(ptr);
                }
            }

            if (edges_prior.size() > 1) {
                auto comp = [this](g2o::EdgeSE2Prior_malcolm* a,
                                   g2o::EdgeSE2Prior_malcolm* b) {
                    auto from_vec2d = a->getOrientation2D(*this);
                    auto to_vec2d = b->getOrientation2D(*this);
                    // Rotate

                    double angle_from =
                        atan2(from_vec2d(1), from_vec2d(0)) - atan2(0, 1);
                    if (angle_from < 0)
                        angle_from += 2 * M_PI;
                    double angle_to =
                        atan2(to_vec2d(1), to_vec2d(0)) - atan2(0, 1);
                    if (angle_to < 0)
                        angle_to += 2 * M_PI;

                    return angle_from < angle_to;
                };

                std::sort(edges_prior.begin(), edges_prior.end(), comp);

                auto ite = edges_prior.begin();
                auto ite_end = edges_prior.back();
                out.push_back(angle((*ite_end), *(*ite)));
                assert(out.back().first >= 0.08);
                assert(out.back().second >= 0);
                assert(out.back().first <= 2 * M_PI);
                assert(out.back().second <= 2 * M_PI);

                for (auto ite = edges_prior.begin();
                     ite != edges_prior.end() - 1; ++ite) {
                    out.push_back(angle(**ite, **(ite + 1)));
                    assert(out.back().first >= 0.08);
                    assert(out.back().second >= 0);
                    assert(out.back().first <= 2 * M_PI);
                    assert(out.back().second <= 2 * M_PI);
                }
            }
        }
        return out;
    }

   private:
    std::pair<double, double> angle(const g2o::EdgeSE2Prior_malcolm& from,
                                    const g2o::EdgeSE2Prior_malcolm& to) const {
        auto from_vec2d = from.getOrientation2D(*this);
        auto to_vec2d = to.getOrientation2D(*this);
        double angle_between = atan2(to_vec2d(1), to_vec2d(0)) -
                               atan2(from_vec2d(1), from_vec2d(0));
        if (angle_between < 0)
            angle_between += 2 * M_PI;

        double angle_from = atan2(from_vec2d(1), from_vec2d(0)) - atan2(0, 1);
        if (angle_from < 0)
            angle_from += 2 * M_PI;

        double angle_to = atan2(to_vec2d(1), to_vec2d(0)) - atan2(0, 1);
        if (angle_to < 0)
            angle_to += 2 * M_PI;

        double direction = (angle_to + angle_from) / 2;
        assert(direction <= 2 * M_PI);

        if (angle_from > angle_to) {
            direction = direction + M_PI;
        }
        while (direction >= 2 * M_PI) {
            direction = direction - (2 * M_PI);
        }

        double width = std::abs(angle_to - angle_from);
        assert(angle_between >= 0.08);
        assert(direction >= 0);
        assert(angle_between >= 0);
        assert(direction <= 2 * M_PI);
        assert(angle_between <= 2 * M_PI);

        return std::pair<double, double>(angle_between, direction);
    }
};

}  // namespace g2o
#endif