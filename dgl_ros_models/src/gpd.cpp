#include <rclcpp/rclcpp.hpp>
#include <dgl_ros/actor.hpp>
#include <dgl_ros/observer.hpp>
#include <dgl_ros/agent.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <dgl_ros_interfaces/action/sample_grasp_poses.hpp>
#include <Eigen/Dense>
#include <pcl_conversions/pcl_conversions.h>
#include <dgl_ros/util/geometry.hpp>
#include <dgl_ros/util/cloud.hpp>
#include <dgl_ros_models/gpd.hpp>
#include <gpd/grasp_detector.h>

using std::placeholders::_1;
using sensor_msgs::msg::PointCloud2;
using dgl_ros_interfaces::action::SampleGraspPoses;

typedef pcl::PointCloud<pcl::PointXYZRGB> PointCloudRGB;
typedef pcl::PointCloud<pcl::PointXYZRGBA> PointCloudRGBA;
namespace dgl_models
{
  
Gpd::Gpd(rclcpp::NodeOptions& options)
  : dgl::Agent<PointCloud2, SampleGraspPoses, PointCloud2>(
        options.parameter_overrides({{ "publish", true }, { "action_topic", "sample_grasp_poses"}}), { "rgbd_camera/points" })
{
  const Eigen::Isometry3d trans_base_cam = dgl::util::isometryFromXYZRPY({ 0.084, 0.017, 0.522, 0, 0.8, 0 });
  const Eigen::Isometry3d transform_cam_opt =  dgl::util::isometryFromXYZRPY({ 0, 0, 0, 0, 0, 0 });
  transform_base_opt_ =  trans_base_cam * transform_cam_opt;
  gpd_grasp_detector_ = std::make_unique<gpd::GraspDetector>("/simply_ws/src/dgl_ros/dgl_ros_models/config/gpd_config.yaml");
}

SampleGraspPoses::Feedback::SharedPtr
Gpd::actionFromObs(std::shared_ptr<dgl::Observer<PointCloud2, PointCloud2>> observer)
{
  auto [id, msg] = observer->observe();
  // Convert to PCL.
  PointCloudRGB cloud;
  pcl::moveFromROSMsg(*msg, cloud);

  // Convert to GPD.
  auto grasp_cloud = std::make_shared<PointCloudRGBA>();
  pcl::copyPointCloud(cloud, *grasp_cloud);
  Eigen::Matrix3Xd camera_view_point(3, 1);
  gpd::util::Cloud gpd_cloud(grasp_cloud, 0, camera_view_point);
  gpd_grasp_detector_->preprocessPointCloud(gpd_cloud, transform_base_opt_);

  std::vector<std::unique_ptr<gpd::candidate::Hand>> grasps;  // detect grasp poses                   // detect grasps
                                                              // in the point cloud
  grasps = gpd_grasp_detector_->detectGrasps(gpd_cloud);      // detect grasp poses
  std::vector<unsigned int> grasp_ids;
  for (unsigned int i = 0; i < grasps.size(); i++)
  {
    grasp_ids.push_back(i);
  }
  auto feedback = std::make_shared<SampleGraspPoses::Feedback>();
  for (auto id : grasp_ids)
  {
    // transform grasp from camera optical link into frame_id
    const Eigen::Isometry3d transform_opt_grasp =
        Eigen::Translation3d(grasps.at(id)->getPosition()) * Eigen::Quaterniond(grasps.at(id)->getOrientation());

    const Eigen::Isometry3d transform_base_grasp = transform_base_opt_ * transform_opt_grasp;
    const Eigen::Vector3d trans = transform_base_grasp.translation();
    const Eigen::Quaterniond rot(transform_base_grasp.rotation());

    // convert back to PoseStamped
    geometry_msgs::msg::PoseStamped grasp_pose;
    grasp_pose.header.frame_id = "world";
    grasp_pose.pose.position.x = trans.x();
    grasp_pose.pose.position.y = trans.y();
    grasp_pose.pose.position.z = trans.z();

    grasp_pose.pose.orientation.w = rot.w();
    grasp_pose.pose.orientation.x = rot.x();
    grasp_pose.pose.orientation.y = rot.y();
    grasp_pose.pose.orientation.z = rot.z();

    feedback->grasp_candidates.emplace_back(grasp_pose);

    // Grasp is selected based on cost not score
    // Invert score to represent grasp with lowest cost
    feedback->costs.emplace_back(static_cast<double>(1.0 / grasps.at(id)->getScore()));
  }

  return feedback;
}

std::unique_ptr<PointCloud2> Gpd::obsFromSrcs(std::shared_ptr<PointCloud2> msg)
{
  // Convert to PCL
  auto cloud = std::make_shared<PointCloudRGB>();
  pcl::fromROSMsg(*msg, *cloud);
  // Segementation works best with XYXRGB
  dgl::util::cloud::removeTable(cloud);

  // Return the final observation
  std::unique_ptr<PointCloud2> cloud_msg = std::make_unique<PointCloud2>();
  pcl::toROSMsg(*cloud, *cloud_msg);
  return cloud_msg;
}
// template class Observer<PointCloud2, PointCloud2>::Observer;
// template class Agent<PointCloud2, SampleGraspPoses, PointCloud2>::Agent;
}  // namespace dgl

int main(int argc, char** argv)
{
  
  rclcpp::init(argc, argv);
  rclcpp::NodeOptions options;
  options.allow_undeclared_parameters(true);
  dgl_models::Gpd server(options);
  server.run();
  rclcpp::shutdown();
  return 0;
}