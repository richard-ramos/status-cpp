cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(status-cpp)


# Dependencies
# If dependencies are managed with conan, we set the CMake module path (where
# CMake looks for dependencies) to the current build folder.
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
endif()


# required modules for our task
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckIncludeFiles)
include(ExternalProject)


# Add the current CMake modules folder to the CMake search path.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(Filesystem REQUIRED)
find_package(OpenSSL REQUIRED)

set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)



################################################################################
# Begin: status-go
set(STATUSGO_ROOT ${CMAKE_BINARY_DIR}/status-go)
set(STATUSGO_LIB_DIR ${STATUSGO_ROOT}/src/status-go/build/bin)

ExternalProject_Add(status-go
  PREFIX ${STATUSGO_ROOT}
  GIT_REPOSITORY "git://github.com/status-im/status-go"
  GIT_TAG "develop"
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
  BUILD_IN_SOURCE 1
  BUILD_COMMAND make statusgo-shared-library V=1
  BUILD_BYPRODUCTS ${STATUSGO_LIB_DIR}/libstatus.so   # USE VARIABLES
)

ExternalProject_Get_Property(status-go SOURCE_DIR)
add_library(status STATIC IMPORTED)
set_property(TARGET status PROPERTY IMPORTED_LOCATION ${STATUSGO_LIB_DIR}/libstatus.so)
add_dependencies(status status-go)
include_directories(${STATUSGO_LIB_DIR})
# End :status-go
################################################################################



add_executable(status-cpp ${SOURCES})

target_compile_features(status-cpp
    PRIVATE
        cxx_std_17
)

target_link_libraries(status-cpp
    PRIVATE
        OpenSSL::OpenSSL
        std::filesystem
        status
)

target_compile_features(status-cpp
    PRIVATE
        cxx_std_17
)
 