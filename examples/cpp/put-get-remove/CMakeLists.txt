cmake_minimum_required(VERSION 3.5)
project(put-get-remove)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake)
set(CMAKE_CXX_STANDARD 11)

find_package(geode-native REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    apache.geode::cpp
)
