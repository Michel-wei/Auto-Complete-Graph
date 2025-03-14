#ifndef AUTOCOMPLETEGRAPH_GOODMATCHINGSLOCALIZATION_18042018
#define AUTOCOMPLETEGRAPH_GOODMATCHINGSLOCALIZATION_18042018

#include <geometry_msgs/PointStamped.h>

#include "RvizPointsLocalization.hpp"
#include "opencv2/opencv.hpp"

namespace AASS {

namespace acg {

/*
 * Get Points from Rviz. THose point are used to marked good links and the class
 * can use them to calculate a number of outliers
 */
class GoodMatchingsLocalization : public RvizPointsLocalization {
   public:
    GoodMatchingsLocalization(ros::NodeHandle nh,
                              AutoCompleteGraphLocalization* acg)
        : RvizPointsLocalization(nh, acg) {}

    /**
     * @brief register a point from either the prior or the robot map. Start
     * with the prior
     */
    virtual void clicked(const geometry_msgs::PointStamped::ConstPtr& msg) {
        std::cout << "Clicked !!! " << _flag_go << std::endl;
        cv::Point2f point(msg->point.x, msg->point.y);

        if (_flag_go == false) {
            _tmp_point = point;
        } else {
            std::cout << "add point" << std::endl;
            MatchLocalization match(_tmp_point, point);
            match.getNodes(*_acg);

            if (checkMatchExist(match) == false) {
                _points.push_back(match);
            }
        }

        if (_flag_go == true) {
            _flag_go = false;
        } else {
            _flag_go = true;
        }

        publishAll();
    }

    void publishAll() {
        std::cout << "Publishing " << _points.size() << std::endl;

        _link_markers.points.clear();
        _link_markers.header.stamp = ros::Time::now();

        for (auto it = _points.begin(); it != _points.end(); ++it) {
            geometry_msgs::Point p;

            auto vertex = it->getPriorNode()->estimate();
            p.x = vertex(0);
            p.y = vertex(1);
            p.z = 0;

            std::cout << "first point " << p.x << " " << p.y << std::endl;

            _link_markers.points.push_back(p);

            auto vertex2 = it->getLandmarkNode()->estimate();
            p.x = vertex2(0);
            p.y = vertex2(1);
            p.z = 0;

            std::cout << "second point " << p.x << " " << p.y << std::endl;

            _link_markers.points.push_back(p);
        }

        _link_pub.publish(_link_markers);
    }

   private:
    bool checkMatchExist(MatchLocalization& match) {
        for (auto it = _points.begin(); it != _points.end(); ++it) {
            if (match.getPriorNode() == it->getPriorNode() &&
                match.getLandmarkNode() == it->getLandmarkNode()) {
                return true;
            }
        }

        return false;
    }
};
}  // namespace acg

}  // namespace AASS

#endif