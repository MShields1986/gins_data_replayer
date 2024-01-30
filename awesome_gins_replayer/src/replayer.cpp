#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <ros/ros.h>
#include <ros/package.h>
#include <sensor_msgs/Imu.h>

using namespace std;

void readIn(string path, vector<vector<double>> &v)
{
    fstream in(path);
    string line;

    int i = 0;

    while (getline(in, line))
    {
        double value;
        stringstream ss(line);

        v.push_back(vector<double>());

        while (ss >> value)
        {
            v[i].push_back(value);
        }
        ++i;
    }
}


int main(int argc, char **argv)
{
    ros::init(argc, argv, "awesome_gins_data_replayer");
    ros::NodeHandle nh;
    ros::Publisher pub = nh.advertise<sensor_msgs::Imu>("/awesome_gins_data_replayer/imu", 200, true);

    // TODO: Paramserver these and the published topic name
    string file_path = "/home/matthew/Desktop/kf_stuff/catkin_ws/src/awesome-gins-datasets/ADIS16465/ADIS16465.txt";
    string frame_id = "imu_link";

    vector<vector<double>> data;
    float dt_step;
    float dt_sof;
    float gnss_start;
    ros::Duration dt_step_dur;
    ros::Duration dt_sof_dur;
    ros::Time start;

    sensor_msgs::Imu msg;

    ROS_INFO_STREAM(ros::this_node::getName() << " running...");

    readIn(file_path, data);

    // Initialise times
    dt_step = 0.0;
    gnss_start = data[0][0];
    start = ros::Time::now();

    while (ros::ok())
    {
        for (int i = 0; i < data.size(); i++) {
            ROS_INFO_THROTTLE(60, "Outputting data...");

            if (i > 0) {
                dt_step = data[i][0] - data[i-1][0];
                dt_sof = data[i][0] - gnss_start;
                //ROS_INFO_STREAM("dt_step: " << setprecision(16) << dt_step);
                //ROS_INFO_STREAM("dt_sof: " << setprecision(16) << dt_sof);
            }

            dt_step_dur = ros::Duration(0, dt_step * 1e+9);
            dt_sof_dur = ros::Duration((int)dt_sof, (dt_sof-(int)dt_sof) * 1e+9);

            msg.header.stamp = start + dt_sof_dur;
            msg.header.frame_id = frame_id;

            msg.angular_velocity.x = data[i][1];
            msg.angular_velocity.y = data[i][2];
            msg.angular_velocity.z = data[i][3];

            msg.linear_acceleration.x = data[i][4];
            msg.linear_acceleration.y = data[i][5];
            msg.linear_acceleration.z = data[i][6];

            pub.publish(msg);

            // TODO: Currently hitting ~191 Hz
            //       Could subtract the time taken for the operations in this loop from dt_step_dur
            dt_step_dur.sleep();
            ros::spinOnce();
        }
    }

  return 0;
}
