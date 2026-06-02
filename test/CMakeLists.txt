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
