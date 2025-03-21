
#ifndef AUTOCOMPLETEGRAPH_ACG_18042018
#define AUTOCOMPLETEGRAPH_ACG_18042018

#include "ACGBase.hpp"
#include "VertexAndEdge/EdgeLinkXY.hpp"

namespace AASS {

namespace acg {

// Simple template specialization to not brake all base code for now...
class AutoCompleteGraph
    : public AutoCompleteGraphBase<AutoCompleteGraphPriorSE2,
                                   g2o::VertexSE2Prior,
                                   g2o::EdgeSE2Prior_malcolm> {
   protected:
    ///@brief Minimum distance from a prior corner to a NDT corner. THe distance
    /// is given in meter and is fixed at 2m in the constuctor
    double _min_distance_for_link_in_meter;
    double _max_distance_for_link_in_meter;
    bool _use_links_prior;
    Eigen::Vector2d _linkNoise;
    ///@brief vector storing all linking edges
    std::vector<g2o::EdgeLinkXY_malcolm*> _edge_link;
    // TODO: hack because fuck g2o node they don't work
    std::vector<EdgeInterface> _edge_interface_of_links;

   public:
    AutoCompleteGraph(const g2o::SE2& sensoffset,
                      const Eigen::Vector2d& tn,
                      double rn,
                      const Eigen::Vector2d& ln,
                      const Eigen::Vector2d& pn,
                      double rp,
                      const Eigen::Vector2d& linkn,
                      ndt_feature::NDTFeatureGraph* ndt_graph)
        : _min_distance_for_link_in_meter(1),
          _max_distance_for_link_in_meter(2),
          _use_links_prior(true),
          _linkNoise(linkn),
          AutoCompleteGraphBase<AutoCompleteGraphPriorSE2,
                                g2o::VertexSE2Prior,
                                g2o::EdgeSE2Prior_malcolm>(sensoffset,
                                                           tn,
                                                           rn,
                                                           ln,
                                                           pn,
                                                           rp,
                                                           ndt_graph) {}

    AutoCompleteGraph(const g2o::SE2& sensoffset,
                      const Eigen::Vector2d& tn,
                      double rn,
                      const Eigen::Vector2d& ln,
                      const Eigen::Vector2d& pn,
                      double rp,
                      const Eigen::Vector2d& linkn)
        : _min_distance_for_link_in_meter(1),
          _max_distance_for_link_in_meter(2),
          _use_links_prior(true),
          _linkNoise(linkn),
          AutoCompleteGraphBase<AutoCompleteGraphPriorSE2,
                                g2o::VertexSE2Prior,
                                g2o::EdgeSE2Prior_malcolm>(sensoffset,
                                                           tn,
                                                           rn,
                                                           ln,
                                                           pn,
                                                           rp) {}

    AutoCompleteGraph(const g2o::SE2& sensoffset, const std::string& load_file)
        : AutoCompleteGraphBase<AutoCompleteGraphPriorSE2,
                                g2o::VertexSE2Prior,
                                g2o::EdgeSE2Prior_malcolm>(sensoffset,
                                                           load_file) {
        std::ifstream infile(load_file);

        double a, b, c;
        infile >> a >> b;
        _transNoise << a, b;
        std::cout << _transNoise << std::endl;
        infile >> a;
        _rotNoise = DEG2RAD(a);
        infile >> a >> b;
        _landmarkNoise << a, b;
        std::cout << _landmarkNoise << std::endl;
        infile >> a >> b;
        _prior->setPriorNoise(a, b);
        infile >> a;
        _prior->setPriorRot(DEG2RAD(a));
        infile >> a >> b;
        _linkNoise << a, b;
        std::cout << _linkNoise << std::endl;

        infile >> _age_start_value;
        std::cout << _age_start_value << std::endl;
        infile >> _age_step;
        std::cout << _age_step << std::endl;
        infile >> _max_age;
        infile >> _min_age;
        infile >> _min_distance_for_link_in_meter;
        std::cout << _min_distance_for_link_in_meter << std::endl;
        infile >> _max_distance_for_link_in_meter;
        std::cout << _max_distance_for_link_in_meter << std::endl;

        infile >> _flag_use_robust_kernel;
        infile >> _use_user_robot_pose_cov;

        assert(_age_start_value >= 0);
        assert(_max_age >= 0);

        _sensorOffset = new g2o::ParameterSE2Offset;
        _sensorOffset->setOffset(_sensorOffsetTransf);
        _sensorOffset->setId(0);
        _ndt_graph = NULL;

        if (infile.eof() == true) {
            throw std::runtime_error("NOT ENOUGH PARAMETERS IN FILE");
        }
    }

    virtual void print() const {
        std::cout << "Print ACG " << std::endl;

        std::cout << "Number of Links " << getLinkEdges().size() << std::endl;
        std::cout << "Number of robot poses " << _nodes_ndt.size() << std::endl;

        ///@brief Minimum distance from a prior corner to a NDT corner. THe
        /// distance is given in meter and is fixed at 2m in the constuctor
        std::cout << "min distance for link in meters: "
                  << _min_distance_for_link_in_meter << std::endl;
        std::cout << "max distance for link in meters: "
                  << _max_distance_for_link_in_meter << std::endl;

        std::cout << "use link prior: " << _use_links_prior << std::endl;
    }

    ///@brief vector storing all linking edges
    std::vector<g2o::EdgeLinkXY_malcolm*>& getLinkEdges() { return _edge_link; }
    const std::vector<g2o::EdgeLinkXY_malcolm*>& getLinkEdges() const {
        return _edge_link;
    }

    void setMinDistanceForLinksInMeters(double inpu) {
        _min_distance_for_link_in_meter = inpu;
    }
    double getMinDistanceForLinksInMeters() const {
        return _min_distance_for_link_in_meter;
    }

    void setMaxDistanceForLinksInMeters(double inpu) {
        _max_distance_for_link_in_meter = inpu;
    }
    double getMaxDistanceForLinksInMeters() const {
        return _max_distance_for_link_in_meter;
    }

    void linkToPrior(bool setter) { _use_links_prior = setter; }
    bool isUsingLinksToPrior() { return _use_links_prior; }

    virtual g2o::EdgeLinkXY_malcolm* addLinkBetweenMaps(
        const g2o::Vector2& pos,
        g2o::VertexSE2Prior* v2,
        g2o::VertexLandmarkNDT* v1);

    std::vector<g2o::EdgeLinkXY_malcolm*>::iterator removeLinkBetweenMaps(
        g2o::EdgeLinkXY_malcolm* v1);

    void clearLinks();

    // Todo move in private
    bool linkAlreadyExist(g2o::VertexLandmarkNDT* v_pt,
                          g2o::VertexSE2Prior* v_prior,
                          std::vector<g2o::EdgeLinkXY_malcolm*>::iterator& it);
    bool linkAlreadyExist(g2o::VertexLandmarkNDT* v_pt,
                          g2o::VertexSE2Prior* v_prior);
    bool noDoubleLinks();

    virtual void updateNDTGraph(ndt_feature::NDTFeatureGraph& ndt_graph,
                                bool noise_flag = false,
                                double deviation = 0.5);

    virtual void overCheckLinks() { checkLinkNotTooBig(); }

    virtual void testInfoNonNul(const std::string& before = "no data") const;
    virtual std::pair<int, int> optimize(int max_iter = 100);
    virtual void clearPrior();
    virtual void updateLinks();
    virtual void setKernelSizeDependingOnAge(g2o::OptimizableGraph::Edge* e,
                                             bool step);

    int createNewLinks() {
        if (this->_use_links_prior) {
            int count = 0;

            // Update ALL links
            auto it = this->_nodes_landmark.begin();
            for (it; it != this->_nodes_landmark.end(); it++) {
                Eigen::Vector2d pose_landmark = (*it)->estimate();
                auto it_prior = this->_prior->getNodes().begin();
                for (it_prior; it_prior != this->_prior->getNodes().end();
                     ++it_prior) {
                    // Don't add the same link twice
                    if (this->linkAlreadyExist(*it, *it_prior) == false) {
                        if (this->_flag_use_corner_orientation == false ||
                            (this->_flag_use_corner_orientation == true &&
                             (*it)->sameOrientation(
                                 (*it_prior)->getAnglesAndOrientations()))) {
                            Eigen::Vector3d pose_tmp =
                                (*it_prior)->estimate().toVector();
                            Eigen::Vector2d pose_prior;
                            pose_prior << pose_tmp(0), pose_tmp(1);
                            double norm_tmp =
                                (pose_prior - pose_landmark).norm();
                            // Update the link
                            if (norm_tmp <= _min_distance_for_link_in_meter) {
                                g2o::Vector2 vec;
                                vec << 0, 0;
                                addLinkBetweenMaps(vec, *it_prior, *it);

                                ++count;
                            }
                        } else {
                            std::cout << "Orientation check failed"
                                      << std::endl;
                        }
                    } else {
                        std::cout << "Already exist" << std::endl;
                    }
                }
            }
            return count;
        } else {
            return 0;
        }
    }

    void checkLinkNotTooBig() {
        std::cout << "check no big links" << std::endl;
        // Check if the link are not too big
        for (auto it_old_links = _edge_link.begin();
             it_old_links != _edge_link.end(); it_old_links++) {
            std::vector<Eigen::Vector3d> vertex_out;

            assert((*it_old_links)->vertices().size() == 2);

            g2o::VertexSE2Prior* ptr = dynamic_cast<g2o::VertexSE2Prior*>(
                (*it_old_links)->vertices()[0]);
            if (ptr == NULL) {
                std::cout << ptr << " and " << (*it_old_links)->vertices()[0]
                          << std::endl;
                throw std::runtime_error(
                    "Links do not have the good vertex type. Prior");
            }
            auto vertex = ptr->estimate().toVector();
            vertex_out.push_back(vertex);

            g2o::VertexLandmarkNDT* ptr2 =
                dynamic_cast<g2o::VertexLandmarkNDT*>(
                    (*it_old_links)->vertices()[1]);
            if (ptr2 == NULL) {
                throw std::runtime_error(
                    "Links do not have the good vertex type. Landmark");
            }
            auto vertex2 = ptr2->estimate();
            Eigen::Vector3d pose_prior;
            pose_prior << vertex2(0), vertex2(1), 0;
            vertex_out.push_back(pose_prior);

            assert(vertex_out.size() == 2);
            double norm = (vertex_out[0] - vertex_out[1]).norm();
            // Attention magic number
            if (norm > _max_distance_for_link_in_meter) {
                if (linkAlreadyExist(ptr2, ptr) == false) {
                    std::cout << "NORM" << norm << "min dist "
                              << _min_distance_for_link_in_meter << " and max "
                              << _min_distance_for_link_in_meter << std::endl;
                    throw std::runtime_error("Big link still present :O");
                }
            }
        }
    }

    void removeBadLinks() {
        // Remove links that went too far away from the points and restor the
        // edges to original state when possible:
        int count = 0;
        size_t siz = _edge_link.size();
        auto it_old_links = _edge_link.begin();
        for (it_old_links; it_old_links != _edge_link.end();) {
            std::vector<Eigen::Vector3d> vertex_out;

            assert((*it_old_links)->vertices().size() == 2);

            g2o::VertexSE2Prior* ptr = dynamic_cast<g2o::VertexSE2Prior*>(
                (*it_old_links)->vertices()[0]);
            if (ptr == NULL) {
                std::cout << ptr << " and " << (*it_old_links)->vertices()[0]
                          << std::endl;
                throw std::runtime_error(
                    "Links do not have the good vertex type. Prior");
            }
            auto vertex = ptr->estimate().toVector();
            vertex_out.push_back(vertex);

            g2o::VertexLandmarkNDT* ptr2 =
                dynamic_cast<g2o::VertexLandmarkNDT*>(
                    (*it_old_links)->vertices()[1]);
            if (ptr2 == NULL) {
                throw std::runtime_error(
                    "Links do not have the good vertex type. Landmark");
            }
            auto vertex2 = ptr2->estimate();
            Eigen::Vector3d pose_prior;
            pose_prior << vertex2(0), vertex2(1), 0;
            vertex_out.push_back(pose_prior);

            assert(vertex_out.size() == 2);
            double norm = (vertex_out[0] - vertex_out[1]).norm();

            if (norm > _max_distance_for_link_in_meter) {
                std::cout << "Removing a link" << std::endl;
                std::cout << "NORM " << norm << "min dist "
                          << _max_distance_for_link_in_meter << std::endl;
                it_old_links = removeLinkBetweenMaps(*it_old_links);
            } else {
                it_old_links++;
            }
            assert(count <= siz);
            count++;
        }
    }

    int countLinkToMake() {
        int count = 0;
        auto it = _nodes_landmark.begin();
        for (it; it != _nodes_landmark.end(); it++) {
            Eigen::Vector2d pose_landmark = (*it)->estimate();
            auto it_prior = _prior->getNodes().begin();
            for (it_prior; it_prior != _prior->getNodes().end(); ++it_prior) {
                Eigen::Vector3d pose_tmp = (*it_prior)->estimate().toVector();
                Eigen::Vector2d pose_prior;
                pose_prior << pose_tmp(0), pose_tmp(1);
                double norm_tmp = (pose_prior - pose_landmark).norm();

                // Update the link
                if (norm_tmp <= _min_distance_for_link_in_meter) {
                    if (linkAlreadyExist(*it, *it_prior) == false) {
                        count++;
                    }
                }
            }
        }

        return count;
    }
};

}  // namespace acg
}  // namespace AASS
#endif