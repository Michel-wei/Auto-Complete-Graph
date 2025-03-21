#ifndef AUTOCOMPLETEGRAPH_CONVERSION_22072016
#define AUTOCOMPLETEGRAPH_CONVERSION_22072016

#include "g2o/types/slam2d/edge_se2.h"
#include "g2o/types/slam2d/se2.h"
#include "g2o/types/slam2d/vertex_se2.h"

#include "auto_complete_graph/ACGMaps.h"
#include "graph_map/graph_map_navigator.h"

namespace AASS {
namespace acg {

inline double getRobustYawFromAffine3d(const Eigen::Affine3d& a) {
    // To simply get the yaw from the euler angles is super sensitive to
    // numerical errors which will cause roll and pitch to have angles very
    // close to PI...
    Eigen::Vector3d v1(1, 0, 0);
    Eigen::Vector3d v2 = a.rotation() * v1;
    double dot = v1(0) * v2(0) +
                 v1(1) * v2(1);  // Only compute the rotation in xy plane...
    double angle = acos(dot);
    // Need to find the sign
    if (v1(0) * v2(1) - v1(1) * v2(0) > 0)
        return angle;
    return -angle;
}

inline Eigen::Affine2d eigenAffine3dTo2d(const Eigen::Affine3d& a3d) {
    return Eigen::Translation2d(a3d.translation().topRows<2>()) *
           Eigen::Rotation2D<double>(getRobustYawFromAffine3d(
               a3d));  // a3d.linear().topLeftCorner<2,2>();
}

inline Eigen::Isometry2d Affine3d2Isometry2d(const Eigen::Affine3d& affine) {
    Eigen::Affine2d affine2d = eigenAffine3dTo2d(affine);
    Eigen::Isometry2d isometry2d;
    isometry2d.translation() = affine2d.translation();
    isometry2d.linear() = affine2d.rotation();
    return isometry2d;
}

inline Eigen::Affine3d createRotationMatrix(double ax, double ay, double az) {
    Eigen::Affine3d rx =
        Eigen::Affine3d(Eigen::AngleAxisd(ax, Eigen::Vector3d(1, 0, 0)));
    Eigen::Affine3d ry =
        Eigen::Affine3d(Eigen::AngleAxisd(ay, Eigen::Vector3d(0, 1, 0)));
    Eigen::Affine3d rz =
        Eigen::Affine3d(Eigen::AngleAxisd(az, Eigen::Vector3d(0, 0, 1)));
    return rz * ry * rx;
}

inline Eigen::Affine3d se2ToAffine3d(const g2o::SE2& vec, int z = 0) {
    Eigen::Affine3d r = createRotationMatrix(0, 0, vec.rotation().angle());
    Eigen::Affine3d t(Eigen::Translation3d(
        Eigen::Vector3d(vec.translation()[0], vec.translation()[1], z)));

    Eigen::Matrix4d m = (t * r).matrix();  // Option 1
    Eigen::Affine3d b;
    b.matrix() = m;

    return b;
}

inline void updateGraphMap(
    const auto_complete_graph::ACGMaps::ConstPtr& acg_maps,
    boost::shared_ptr<perception_oru::graph_map::GraphMapNavigator>&
        graph_map) {
    int count = 0;
    for (auto pose : acg_maps->ndt_maps.poses) {
        auto node_ptr_gm = graph_map->GetNode(count);
        Eigen::Affine3d pose_affine;
        tf::poseMsgToEigen(pose, pose_affine);
        node_ptr_gm->SetPose(pose_affine);
        count++;
    }

    count = 0;
    for (auto transf : acg_maps->ndt_maps.transformations) {
        auto node_odom_ptr = graph_map->GetFactors()[count];
        Eigen::Affine3d transf_affine;
        tf::transformMsgToEigen(transf, transf_affine);
        node_odom_ptr->UpdateFactor(transf_affine);
        count++;
    }
}

}  // namespace acg
}  // namespace AASS

#endif