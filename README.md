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