cmake_minimum_required(VERSION 3.16)
project(go2_tests CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "")

find_package(unitree_sdk2 REQUIRED)
find_package(OpenCV REQUIRED)

function(add_test_exe name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE unitree_sdk2)
endfunction()

add_test_exe(t_vy_test)
add_test_exe(recover_joystick)
add_test_exe(t2_basic_motion)
add_test_exe(t3_jump)
add_test_exe(t4_obstacle_avoid)
add_test_exe(t5_climb_stairs)
add_test_exe(t6_actions)
add_test_exe(t7_vui_light)
add_test_exe(t8_camera)

target_link_libraries(t8_camera PRIVATE unitree_sdk2 ${OpenCV_LIBS})
target_include_directories(t8_camera PRIVATE ${OpenCV_INCLUDE_DIRS})

set(HIK_LIBS /opt/MVS/lib/64/libMvCameraControl.so)
set(HIK_INC /opt/MVS/include)

# t9: 纯视觉调试，不连机器人，无需 sudo
add_executable(t9_line_follow t9_line_follow.cpp)
target_include_directories(t9_line_follow PRIVATE ${OpenCV_INCLUDE_DIRS} ../src ${HIK_INC})
target_link_libraries(t9_line_follow PRIVATE ${OpenCV_LIBS} ${HIK_LIBS})

# t10: 连机器人实际运行，需要 sudo
add_executable(t10_line_follow_robot t10_line_follow_robot.cpp)
target_include_directories(t10_line_follow_robot PRIVATE ${OpenCV_INCLUDE_DIRS} ../src ${HIK_INC})
target_link_libraries(t10_line_follow_robot PRIVATE unitree_sdk2 ${OpenCV_LIBS} ${HIK_LIBS})

# t11: ArUco Tag 检测（GO2 前置摄像头）
add_executable(t11_aruco_tag t11_aruco_tag.cpp)
target_include_directories(t11_aruco_tag PRIVATE ${OpenCV_INCLUDE_DIRS} ../src)
target_link_libraries(t11_aruco_tag PRIVATE unitree_sdk2 ${OpenCV_LIBS})

# t12: 警示标志识别（GO2 前置摄像头）
add_executable(t12_sign_detect t12_sign_detect.cpp)
target_include_directories(t12_sign_detect PRIVATE ${OpenCV_INCLUDE_DIRS} ../src)
target_link_libraries(t12_sign_detect PRIVATE unitree_sdk2 ${OpenCV_LIBS})

# t13: YOLO11 警示标志识别（GO2 前置摄像头，需 ONNX Runtime）
add_executable(t13_yolo_sign t13_yolo_sign.cpp)
target_include_directories(t13_yolo_sign PRIVATE ${OpenCV_INCLUDE_DIRS} ../src /usr/local/include)
target_link_libraries(t13_yolo_sign PRIVATE unitree_sdk2 ${OpenCV_LIBS}
    /usr/local/lib/libonnxruntime.so)

# t14: 障碍物检测调试（海康相机，无需连机器人）
add_executable(t14_obstacle_detect t14_obstacle_detect.cpp)
target_include_directories(t14_obstacle_detect PRIVATE ${OpenCV_INCLUDE_DIRS} ../src ${HIK_INC})
target_link_libraries(t14_obstacle_detect PRIVATE ${OpenCV_LIBS} ${HIK_LIBS})

# t15: 障碍物检测 + 跳跃（连机器人）
add_executable(t15_obstacle_jump t15_obstacle_jump.cpp)
target_include_directories(t15_obstacle_jump PRIVATE ${OpenCV_INCLUDE_DIRS} ../src ${HIK_INC})
target_link_libraries(t15_obstacle_jump PRIVATE unitree_sdk2 ${OpenCV_LIBS} ${HIK_LIBS})

# t16: 地面红色圆形检测（无需机器人）
add_executable(t16_red_circle_detect t16_red_circle_detect.cpp)
target_include_directories(t16_red_circle_detect PRIVATE ${OpenCV_INCLUDE_DIRS} ../src ${HIK_INC})
target_link_libraries(t16_red_circle_detect PRIVATE ${OpenCV_LIBS} ${HIK_LIBS})

# t17: 雷达侧距调试（连机器人）
add_executable(t17_lidar_range t17_lidar_range.cpp)
target_include_directories(t17_lidar_range PRIVATE ../src)
target_link_libraries(t17_lidar_range PRIVATE unitree_sdk2)

# t18: 台阶检测 + 爬台阶（连机器人+台阶）
add_executable(t18_climb_stairs t18_climb_stairs.cpp)
target_include_directories(t18_climb_stairs PRIVATE ${OpenCV_INCLUDE_DIRS} ../src)
target_link_libraries(t18_climb_stairs PRIVATE unitree_sdk2 ${OpenCV_LIBS})

# t19: 检测平台完整流程（连机器人）
add_executable(t19_detection_point t19_detection_point.cpp)
target_include_directories(t19_detection_point PRIVATE ${OpenCV_INCLUDE_DIRS} ../src /usr/local/include)
target_link_libraries(t19_detection_point PRIVATE unitree_sdk2 ${OpenCV_LIBS} /usr/local/lib/libonnxruntime.so)

# t20: 直角弯检测+计数（连机器人+赛道）
add_executable(t20_corner_count t20_corner_count.cpp)
target_include_directories(t20_corner_count PRIVATE ${OpenCV_INCLUDE_DIRS} ../src ${HIK_INC})
target_link_libraries(t20_corner_count PRIVATE unitree_sdk2 ${OpenCV_LIBS} ${HIK_LIBS})
