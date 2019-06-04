# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.4)

project ("tinytangle")

SET(CMAKE_C_FLAGS "-std=c11")
SET(CMAKE_CXX_FLAGS "-std=c++14 ${CMAKE_CXX_FLAGS}")

SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC")

# --------------- Definitions ------------------
SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")
SET(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")

# --------------- REQUIREMENTS ---------------------
FILE(MAKE_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}")
FILE(MAKE_DIRECTORY "${LIBRARY_OUTPUT_PATH}")
set(Boost_USE_STATIC_LIBS   ON)
set(BOOST_COMPONENTS date_time chrono filesystem iostreams program_options regex system thread unit_test_framework)
find_package(Boost REQUIRED COMPONENTS ${BOOST_COMPONENTS})
    #system program_options)
INCLUDE_DIRECTORIES("${Boost_INCLUDE_DIRS}")


# ------------------ includes --------------------
SET(sqlite_LIBRARY sqlite_static)
SET(json_LIBRARY json_static)
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/json_static")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/sqlite_static")



# 包含子项目。
add_subdirectory ("src")
add_subdirectory ("sqlite_static")
add_subdirectory ("json_static")
