# @file components/wifi_sta_manager/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2023/01/14 08:35:40
# @encoding utf-8
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.20)

idf_component_register(SRCS wifi_sta_manager.c
        INCLUDE_DIRS inc
        PRIV_REQUIRES main)