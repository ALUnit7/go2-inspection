cmake_minimum_required(VERSION 3.16)
project(go2_inspection CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release CACHE STRING "")

# ── 依赖 ──────────────────────────────────────────────────────────────────────
find_package(unitree_sdk2 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(yaml-cpp REQUIRED)

# ONNX Runtime（可选，视觉模块需要）
find_library(ONNXRUNTIME_LIB onnxruntime HINTS /usr/local/lib /opt/onnxruntime/lib)
find_path(ONNXRUNTIME_INCLUDE onnxruntime_cxx_api.h
    HINTS /usr/local/include/onnxruntime /opt/onnxruntime/include)

# ── 源文件（排除只在 test 中使用的感知模块）────────────────────────────────
file(GLOB_RECURSE SOURCES src/*.cpp)
list(FILTER SOURCES EXCLUDE REGEX ".*HikCamera\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*ArucoDetector\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*YoloDetector\\.cpp$")

add_executable(go2_inspection ${SOURCES})

target_include_directories(go2_inspection PRIVATE
    src
    ${OpenCV_INCLUDE_DIRS}
)

if(ONNXRUNTIME_LIB AND ONNXRUNTIME_INCLUDE)
    target_include_directories(go2_inspection PRIVATE ${ONNXRUNTIME_INCLUDE})
endif()

target_link_libraries(go2_inspection PRIVATE
    unitree_sdk2
    ${OpenCV_LIBS}
    yaml-cpp
)

if(ONNXRUNTIME_LIB)
    target_link_libraries(go2_inspection PRIVATE ${ONNXRUNTIME_LIB})
    target_compile_definitions(go2_inspection PRIVATE WITH_ONNXRUNTIME)
endif()

install(TARGETS go2_inspection DESTINATION bin)

add_subdirectory(test)
