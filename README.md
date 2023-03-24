# 操作方法

### 构建

在SDK/cpp_project下 

~~~shell
mkdir build
cd build
cmake ..
~~~

### 运行

~~~shell
bash a.sh [地图]
~~~

## 3.18日更新

- 参数配置调整到macro.h
- 调参脚本为根目录下的tiaocan.py
- 现在直接运行根目录下的grade.py可以直接计算出四张图总分

## 3.21 更新

- 每张地图分别设置参数
- 参数以及地图哈希值在maps.h
- 调参时需要保留#define TIAOCAN（位于tools.cpp）
- 正常运行时注释掉#define TIAOCAN

## 3.24更新

- 配置信息调整至config.h，可以在里面on/off 各项功能

- 更新了碰撞避免

- “碰撞避免”是在行驶阶段，检测到即将发生的碰撞并且临时改变路线

- ”碰撞预测“是在决策阶段，尽量不安排会发生碰撞的任务，但精确性较低

  