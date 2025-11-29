# 手机遥控器


## 如何使用

1. 将 ESP32 连接到C板遥控器接口
2. 连接到 ESP32 的热点：`Mobile_RC_AP`
3. 打开浏览器，输入网址：`192.168.4.1/control`
4. 点击按键即可控制啦

## 修改代码


## 备注

- 关于报错 `fatal error: hwcrypto/sha.h: No such file or directory`
  ESP32新版本中不使用`<hwcrypto/sha.h>`，改为使用`<sha/sha_parallel_engine.h>`