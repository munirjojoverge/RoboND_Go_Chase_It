#include <sensor_msgs/Image.h>
#include "ball_chaser/DriveToTarget.h"
#include "ros/ros.h"

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the
// specified direction
void drive_robot(float lin_x, float ang_z) {
  // Request a service and pass the velocities to it to drive the robot
  ball_chaser::DriveToTarget srv;
  srv.request.linear_x = lin_x;
  srv.request.angular_z = ang_z;

  client.call(srv);
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img) {
  int white_pixel = 255;

  // Loop through each pixel in the image and check if there's a bright white
  // one Then, identify if this pixel falls in the left, mid, or right side of
  // the image Depending on the white ball position, call the drive_bot function
  // and pass velocities to it Request a stop when there's no white ball seen by
  // the camera

  // Number of white pixels in the left, middle, and right part of the image,
  // respectively
  int left_pixel_count = 0;
  int middle_pixel_count = 0;
  int right_pixel_count = 0;

  for (int h = 0; h < img.height; h++) {
    // Data  from sensor_msgs/image is a 1D vector where 1 pixel is given by 3
    // bytes the RED, BLUE, and GREEN color information. So, in each iteration,
    // I'm checking three consecutive pixels/bytes
    for (int s = 0; s < img.step; s += 3) {
      if ((img.data[img.step * h + s] == white_pixel) &&
          (img.data[img.step * h + s + 1] == white_pixel) &&
          (img.data[img.step * h + s + 2] == white_pixel)) {
        // Left part of an image
        if (s < 0.35 * img.step) {
          left_pixel_count++;
        }
        // Middle part of an image
        else if (s >= 0.35 * img.step && s < 0.65 * img.step) {
          middle_pixel_count++;
        }
        // Right part of an image
        else {
          right_pixel_count++;
        }
      }
    }
  }

  // checking if there is a white ball in the image or otherwise the robot will
  // stop

  if ((left_pixel_count == 0) && (middle_pixel_count == 0) &&
      (right_pixel_count == 0)) {
    drive_robot(0.0, 0.0);
  }
  // If ball is visible, in which part of the image
  else {
    // Turn right, if the ball is in the right part of the image
    if ((left_pixel_count > right_pixel_count) &&
        (left_pixel_count > middle_pixel_count)) {
      drive_robot(0.35, 0.35);
    }

    // Go straight if the ball is in the middle part of the image
    else if ((middle_pixel_count > left_pixel_count) &&
             (middle_pixel_count > right_pixel_count)) {
      drive_robot(0.35, 0.0);
    }

    // Turn left if the ball is in the left part of the image
    else {
      drive_robot(0.35, -0.35);
    }
  }
}

int main(int argc, char** argv) {
  // Initialize the process_image node and create a handle to it
  ros::init(argc, argv, "process_image");
  ros::NodeHandle n;

  // Define a client service capable of requesting services from command_robot
  client =
      n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

  // Subscribe to /camera/rgb/image_raw topic to read the image data inside the
  // process_image_callback function
  ros::Subscriber sub1 =
      n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

  // Handle ROS communication events
  ros::spin();

  return 0;
}
