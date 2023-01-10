# @file CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2023/01/10 21:13:46
# @encoding utf-8
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.20)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(aligenie)

idf_build_set_property(COMPILE_OPTIONS "-Wformat=0" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-implicit-fallthrough" APPEND)