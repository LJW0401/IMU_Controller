#include "shared.hpp"

#include "protocol_wifi.hpp"

protocol_wifi::imu_u head_imu_data;
protocol_wifi::imu_u pose_imu_data;

bool head_tracker_connected = false;
bool pose_tracker_connected = false;
