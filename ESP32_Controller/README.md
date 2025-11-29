# 手机遥控器

本项目使用 ESP32 提供网页服务，连接到ESP32的热点后可以使用浏览器打开控制页面

## 如何使用

1. 将 ESP32 连接到C板遥控器接口
2. 连接到 ESP32 的热点：`Mobile_RC_AP`
3. 打开浏览器，输入网址：`192.168.4.1/control`
4. 点击按键即可控制啦

## 修改代码

1. 遥控器类型
   > 本项目中的控制信号皆为遥控器信号，打算直接接在 DBUS 接口上当遥控器使用。

   `main.cpp` 中 `setupUART(RC_ET08A);` 的参数 `RC_ET08A` 改为想要模拟的遥控器

2. 控制映射
   > 目前只提供了几个按钮，可以根据需要更改运动命令对应的遥控器通道值。

    `mobile_rc_server.cpp` 中 `handleControlSolvePage` 函数负责处理控制命令部分的内容，在这里修改就可以了。

3. 引脚
   > 目前只在企鹅自己买的一块 ESP32 开发板上试过，如果需要适配其他开发板的话自行配置引脚

   `communicate.cpp` 中 `setupUART` 函数中的 `UartSerial.begin(100000, SERIAL_8E1, 12, 13);` 函数中的 `12` `13` 参数为使用的 RX 和 TX 引脚id，可以根据需要改为自己的引脚

## 备注

- 关于模拟遥控器摇杆页面 `192.168.4.1/rc`

   实践表明，通过html页面完全模拟摇杆对ESP32的算力要求比较高，延迟极大，不建议玩（而且现在模拟摇杆页面还有一些BUG）。

- 关于报错 `fatal error: hwcrypto/sha.h: No such file or directory`
  ESP32新版本中不使用`<hwcrypto/sha.h>`，改为使用`<sha/sha_parallel_engine.h>`