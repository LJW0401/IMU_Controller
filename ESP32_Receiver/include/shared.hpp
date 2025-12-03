#ifndef SHARED_HPP
#define SHARED_HPP

#include "protocol_wifi.hpp"

// task_host.cpp
extern protocol_wifi::imu_u head_imu_data;
extern protocol_wifi::imu_u pose_imu_data;
extern bool head_tracker_connected;
extern bool pose_tracker_connected;

#endif  // SHARED_HPP