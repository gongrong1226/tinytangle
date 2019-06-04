# CMakeList.txt: DAG_cmake 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.4)

IF(NOT WIN32)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
ENDIF()

#源文件
FILE(GLOB_RECURSE jsoncpp_SOURCES "*.cpp")

#增加动态库
ADD_LIBRARY(json_static STATIC ${jsoncpp_SOURCES})

TARGET_LINK_LIBRARIES(json_static)
INSTALL(TARGETS json_static DESTINATION lib)


#设置头文件搜索目录
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/json
)

# 将源代码添加到此项目的可执行文件。
#add_executable (sqlite_static "sqlite.c" "sqlite.h" "sqliteext.h")

# TODO: 如有需要，请添加测试并安装目标。
