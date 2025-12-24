cmake_minimum_required(VERSION 3.16)
project(EyeCalibrationDemo LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(EyeCalibration
    src/main.cpp
    src/DemoWindow.cpp
    src/CalibrationModel.cpp
    src/DemoWindow.h
    src/CalibrationModel.h
)

# 开启 C++17，便于使用现代语法
set_target_properties(EyeCalibration PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(EyeCalibration PRIVATE Qt6::Widgets)
