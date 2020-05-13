#include <sensor_msgs/Image.h>
#include "ball_chaser/DriveToTarget.h"
#include "ros/ros.h"

#include <algorithm>
#include <tuple>

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
  // TODO: Loop through each pixel in the image and check if there's a bright
  // white one Then, identify if this pixel falls in the left, mid, or right
  // side of the image Depending on the white ball position, call the drive_bot
  // function and pass velocities to it Request a stop when there's no white
  // ball seen by the camera

  std::tuple<int, int, int> white_threshold_low =
      std::make_tuple(240, 240, 240);
  std::tuple<int, int, int> white_threshold_high =
      std::make_tuple(255, 255, 255);

  // To steer the robot, what I'll do is to steer directly where the ball is at.
  // Here's how I'll do it. First we know from "my_robot. gazebo" that the
  // horizontal FOV of the camera is 1.3962634 rad. If we divide this FOV by the
  // number of columns on the image, we will get our steering angle "step size".
  // Now we need to find the centroid or simply the average column of "ball"
  // pixels and drive from the ceter of the image (where the robot is) to that
  // column.
  double turning_step = 1.3962634 / img.width;
  int center_col = img.width / 2;
  int center_row = img.height / 2;
  int ball_pixel_col_avg;
  int ball_pixel_row_avg;
  int ball_pixel_col_std;
  int ball_pixel_row_std;
  int ball_pix_count = 0;
  std::vector<int> ball_pixels_col;
  std::vector<int> ball_pixels_row;

  // Let's create a threshold for the number of pixels that will be considered a
  // "ball" and not just a false positive.
  int ball_pixel_count_threshold = 30;

  // We are going to search only below (or at) the horizon. There is no need to
  // search the entire image. A huge problem I had is that I tried to install
  // OpenCV on the VM provided and when I updated and upgraded, things did not
  // work anymore. The idea was trying to find the centroid of a cluster of
  // pixels that fell below the color threshold. So I decided to keep it simple
  // for now and forget about finding a cluster of "white-ish" pixels on the
  // screen, which is the ideal thing to do. NOTE: According to
  // http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html
  // "sensor_msgs::Image" message contains an uncompressed image
  // (0, 0) is at top-left corner of image
  int horizon_height_pixel = (int)0.45 * img.height * img.step;
  int R, G, B;
  for (int p = horizon_height_pixel; p < img.height * img.step; p += 3) {
    R = img.data[p + 0];
    G = img.data[p + 1];
    B = img.data[p + 2];
    auto RGB = std::make_tuple(R, G, B);

    if ((RGB >= white_threshold_low) && (RGB <= white_threshold_high)) {
      int cur_col = center_col - (p % img.step);  // from robot perspective
      ball_pix_count++;
      ball_pixel_col_avg += (cur_col - ball_pixel_col_avg) / ball_pix_count;
    }
  }

  // I didn't want to get out of the for loop prematurely by looking at a few
  // pixels while scaning a row.
  if (ball_pix_count >= ball_pixel_count_threshold) {
    double steering_ang = ball_pixel_col_avg * turning_step;
    drive_robot(0.4, steering_ang);
  } else {
    drive_robot(0.0, 0.0);
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
      n.subscribe("/camera/rgb/image_raw", 1, process_image_callback);

  // Handle ROS communication events
  ros::spin();

  return 0;
}
