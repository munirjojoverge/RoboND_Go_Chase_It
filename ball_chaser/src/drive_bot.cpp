#include "geometry_msgs/Twist.h"
#include "ros/ros.h"
// TODO: Include the ball_chaser "DriveToTarget" header file
#include "ball_chaser/DriveToTarget.h"

#include <tuple>

// ROS::Publisher motor commands;
ros::Publisher motor_command_publisher;

// This function checks and clamps the joint angles to a safe zone
std::vector<float> clamp_at_boundaries(float req_linear_x,
                                       float req_angular_z) {
  // Define clamped joint angles and assign them to the requested ones
  float clamped_linear_x = req_linear_x;
  float clamped_angular_z = req_angular_z;

  // Get min and max joint parameters, and assigning them to their respective
  // variables
  float min_linear_x, max_linear_x, min_angular_z, max_angular_z;

  // // Assign a new node handle since we have no access to the main one
  // ros::NodeHandle n2;
  // // Get node name
  // std::string node_name = ros::this_node::getName();
  // // Get joints min and max parameters
  // n2.getParam(node_name + "/min_linear_x_speed", min_linear_x);
  // n2.getParam(node_name + "/max_linear_x_speed", max_linear_x);
  // n2.getParam(node_name + "/min_angular_z_speed", min_angular_z);
  // n2.getParam(node_name + "/max_angular_z_speed", max_angular_z);
  min_linear_x = 0.0;
  max_linear_x = 10.0;
  min_angular_z = 0.0;
  max_angular_z = 1.57;
  // Check if joint 1 falls in the safe zone, otherwise clamp it
  if (req_linear_x < min_linear_x || req_linear_x > max_linear_x) {
    clamped_linear_x =
        std::min(std::max(req_linear_x, min_linear_x), max_linear_x);
    ROS_WARN(
        "linear_x is out of bounds, valid range (%1.2f,%1.2f), clamping to: "
        "%1.2f",
        min_linear_x, max_linear_x, clamped_linear_x);
  }
  // Check if joint 2 falls in the safe zone, otherwise clamp it
  if (req_angular_z < min_angular_z || req_angular_z > max_angular_z) {
    clamped_angular_z =
        std::min(std::max(req_angular_z, min_angular_z), max_angular_z);
    ROS_WARN(
        "angular_z is out of bounds, valid range (%1.2f,%1.2f), clamping to: "
        "%1.2f",
        min_angular_z, max_angular_z, clamped_angular_z);
  }

  // Store clamped joint angles in a clamped_data vector
  std::vector<float> clamped_data = {clamped_linear_x, clamped_angular_z};

  return clamped_data;
}

// TODO: Create a handle_drive_request callback function that executes whenever
// a drive_bot service is requested This function should publish the requested
// linear x and angular velocities to the robot wheel joints After publishing
// the requested velocities, a message feedback should be returned with the
// requested wheel velocities
bool handle_drive_request(ball_chaser::DriveToTarget::Request& req,
                          ball_chaser::DriveToTarget::Response& res) {
  ROS_INFO("DriveToTarget Request received - linear_x:%1.2f, angular_z:%1.2f",
           (float)req.linear_x, (float)req.angular_z);

  // Check if requested linear and angular speeds are in the safe zone,
  // otherwise clamp them
  std::vector<float> wheel_velocities =
      clamp_at_boundaries(req.linear_x, req.angular_z);

  // Create a motor_command object of type geometry_msgs::Twist
  geometry_msgs::Twist motor_command;

  // Set wheel velocities
  motor_command.linear.x = wheel_velocities[0];
  motor_command.angular.z = wheel_velocities[1];

  // Publish angles to drive the robot
  motor_command_publisher.publish(motor_command);

  // Wait 1 seconds
  // ros::Duration(1).sleep();

  // Return a response message
  res.msg_feedback =
      "Wheels speed set - linear_x: " + std::to_string(wheel_velocities[0]) +
      " , angular_z: " + std::to_string(wheel_velocities[1]);
  ROS_INFO_STREAM(res.msg_feedback);

  return true;
}

int main(int argc, char** argv) {
  // Initialize a ROS node
  ros::init(argc, argv, "drive_bot");

  // Create a ROS NodeHandle object
  ros::NodeHandle n;

  // Inform ROS master that we will be publishing a message of type
  // geometry_msgs::Twist on the robot actuation topic with a publishing queue
  // size of 10
  motor_command_publisher = n.advertise<geometry_msgs::Twist>("/cmd_vel", 10);

  // TODO: Define a drive /ball_chaser/command_robot service with a
  // handle_drive_request callback function
  ros::ServiceServer service =
      n.advertiseService("/ball_chaser/command_robot", handle_drive_request);
  ROS_INFO("DriveToTarget Service Server is ready to receive requests");

  // TODO: Delete the loop, move the code to the inside of the callback function
  // and make the necessary changes to publish the requested velocities instead
  // of constant values

  // TODO: Handle ROS communication events
  ros::spin();

  return 0;
}
