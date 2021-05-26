# CS214-Algorithm-Final-Project

#### 目录

`./ToyData`: 样例数据

`./tests`: 单元测试

`./includes`: 所有的模块，包括`common.hpp`,`DAG`,`scheduler`,`simulator`……

`./scripts`: 脚本文件，用于生成数据并测试

`main_*.cpp`: 主程序



#### 运行方法

##### 单独运行

1. 编译`main_*`

2. 保证4个`json`数据文件和`exe`在同一个目录
3. 运行

##### 使用脚本运行（推荐）

1. 进入`scripts`目录
2. 将`link.json`拷贝至该目录（不会生成）
3. 运行`compile_O3.bat`，编译所有`main_*`
4. 运行`gen_test.bat`，每次生成一组新的数据并测试所有`main_*`

