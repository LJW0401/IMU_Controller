# IMU_Controller

（整活系列）用IMU实现的一个简单的遥控器。

## 设备拓扑关系

![连线拓扑关系图](images/connect.svg)

本控制器主要由3部分构成：

1. 机器人信号接收器（Receiver），其WIFI为AP模式，作为主机。
2. 手柄控制器（Hand Controller），通过WIFI连接至Receiver实现并网。
3. 头部追踪器（Head Tracker），通过WIFI连接至Receiver实现并网。

当手柄控制器/头部追踪器都没有连接至Receiver时，不向机器人发送控制指令。
