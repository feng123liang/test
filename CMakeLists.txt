cmake_minimum_required(VERSION 3.16)

# 工程名称
project(lightBarMatching01)

# 设置C++标准为C++17
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

add_executable(lightBarMatching01 detector.cpp main.cpp)
target_link_libraries(
    lightBarMatching01 
    ${OpenCV_LIBS} 
    yaml-cpp
    Eigen3::Eigen
    ${CMAKE_CURRENT_SOURCE_DIR}/cameraOpen/64/libMvCameraControl.so
)