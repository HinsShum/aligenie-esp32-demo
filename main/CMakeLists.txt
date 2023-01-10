# @file main/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2023/01/10 21:21:22
# @encoding utf-8
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.20)

# list all source file directories
set(COMPONENTS_SRC_VPATH app/tasks/daemon)
list(APPEND COMPONENTS_SRC_VPATH common/utils/xlog)

# list all source head file directories
set(COMPONENTS_INC_VPATH .)
list(APPEND COMPONENTS_INC_VPATH common/inc)
list(APPEND COMPONENTS_INC_VPATH app/tasks/inc)
list(APPEND COMPONENTS_INC_VPATH common/utils/xlog/inc)

idf_component_register(SRC_DIRS ${COMPONENTS_SRC_VPATH}
        INCLUDE_DIRS ${COMPONENTS_INC_VPATH})

# global macros definition
target_compile_definitions(${COMPONENT_LIB} PUBLIC "CONFIG_OPTIONS_FILE=<config/options.h>")
target_compile_definitions(${COMPONENT_LIB} PUBLIC "CONFIG_USE_XLOG" "CONFIG_XLOG_BUF_SHIFT=12")
