# 灯条匹配作业1

### 阅读armor.hpp中 Light 类和 Armor 类具有的变量，完成detect.cpp文件“TODO”部分更准确地实现灯条匹配。具体涉及到的方法参考 notion 教程



一些开始指引：（可用Tab补全） \n
在/homework1依次输入指令： \n
mkdir build \n
cd build \n
cmake .. \n
make \n
./lightBarMatching01 \n
即可运行初始版本

一些远程连接指引：（待完善）
在vscode中左边栏选择 ”远程资源管理器“ -> 远程（SSH） -> + 输入 ssh phx@10.108.4.125 -> 确认连接后输入密码：一个空格 -> cd 到/home/phx/tutorial/

可能的问题：
显示.so文件不存在或无法链接：可能是复制/clone时出错导致.so文件损坏
尝试使用摄像头进行试验时显示无设备：检查是否成功接上海康相机
