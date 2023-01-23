# @file main/CMakeLists.txt
# @author HinsShum hinsshum@qq.com
# @date 2023/01/10 21:21:22
# @encoding utf-8
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.20)

# list all source file directories
set(COMPONENTS_SRC_VPATH app/tasks/daemon)
list(APPEND COMPONENTS_SRC_VPATH app/tasks/cmdline)
list(APPEND COMPONENTS_SRC_VPATH app/utils)
list(APPEND COMPONENTS_SRC_VPATH app/resources)
list(APPEND COMPONENTS_SRC_VPATH app/data_proc)
list(APPEND COMPONENTS_SRC_VPATH app/storage)
list(APPEND COMPONENTS_SRC_VPATH common/utils/xlog)
list(APPEND COMPONENTS_SRC_VPATH common/utils/data_center)
list(APPEND COMPONENTS_SRC_VPATH common/utils/pingpong_buffer)
list(APPEND COMPONENTS_SRC_VPATH common/utils/resource_manager)

# list all source head file directories
set(COMPONENTS_INC_VPATH .)
list(APPEND COMPONENTS_INC_VPATH common/inc)
list(APPEND COMPONENTS_INC_VPATH app/tasks/inc)
list(APPEND COMPONENTS_INC_VPATH app/utils/inc)
list(APPEND COMPONENTS_INC_VPATH app/resources/inc)
list(APPEND COMPONENTS_INC_VPATH app/data_proc/inc)
list(APPEND COMPONENTS_INC_VPATH app/storage/inc)
list(APPEND COMPONENTS_INC_VPATH common/utils/xlog/inc)
list(APPEND COMPONENTS_INC_VPATH common/utils/data_center/inc)
list(APPEND COMPONENTS_INC_VPATH common/utils/pingpong_buffer/inc)
list(APPEND COMPONENTS_INC_VPATH common/utils/resource_manager/inc)

idf_component_register(SRC_DIRS ${COMPONENTS_SRC_VPATH}
        INCLUDE_DIRS ${COMPONENTS_INC_VPATH})

# global macros definition
target_compile_definitions(${COMPONENT_LIB} PUBLIC "CONFIG_OPTIONS_FILE=<config/options.h>")
target_compile_definitions(${COMPONENT_LIB} PUBLIC "CONFIG_USE_XLOG" "CONFIG_XLOG_BUF_SHIFT=12")
