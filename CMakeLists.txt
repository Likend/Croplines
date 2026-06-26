cmake_minimum_required(VERSION 3.21)
project(Croplines)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the build type" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-s")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(wxWidgets_USE_DEBUG ON)
else()
    set(wxWidgets_USE_STATIC ON)
endif()

find_package(wxWidgets REQUIRED aui core base gl)
find_package(Eigen3 REQUIRED) # required by opencv
find_package(OpenCV REQUIRED)
find_package(OpenMP REQUIRED)
find_package(cereal REQUIRED)
include(${wxWidgets_USE_FILE})

file(GLOB_RECURSE SRCS src/*.cpp)
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
set (WIN32_RESOURCES src/resource.rc)
endif()

if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
add_executable(Croplines ${SRCS} ${WIN32_RESOURCES})
else()
add_executable(Croplines WIN32 ${SRCS} ${WIN32_RESOURCES})
endif()
target_include_directories(Croplines PRIVATE src)
target_include_directories(Croplines PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(Croplines PRIVATE wxWidgets::wxWidgets)
target_link_libraries(Croplines PRIVATE ${OpenCV_LIBS})
target_link_libraries(Croplines PRIVATE opengl32)
target_link_libraries(Croplines PRIVATE cereal::cereal)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Install.cmake")
