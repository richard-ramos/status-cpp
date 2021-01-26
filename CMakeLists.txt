cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(status-cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
# If dependencies are managed with conan, we set the CMake module path (where
# CMake looks for dependencies) to the current build folder.
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
endif()

set(Qt5_USE_STATIC_LIBS ON)    
set(Qt5_USE_STATIC_RUNTIME ON)

# required modules for our task
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckIncludeFiles)
include(ExternalProject)


# Add the current CMake modules folder to the CMake search path.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(Filesystem REQUIRED)
find_package(OpenSSL REQUIRED)



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
# End: status-go
################################################################################


################################################################################
# Begin: Qt
find_package(Qt5 COMPONENTS Core Gui Widgets Quick Qml Concurrent Network DBus PrintSupport REQUIRED)
find_package(Qt5QuickCompiler)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DUSE_INSTALL_TARGET)

# End: Qt
################################################################################

set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/status.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/settings.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/onboarding-model.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/login-model.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp"
#    "${CMAKE_CURRENT_SOURCE_DIR}/qml/resources.qrc"
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


add_executable(status-cpp ${SOURCES})

target_compile_features(status-cpp
    PRIVATE
        cxx_std_17
)

target_include_directories(status-cpp PUBLIC include)

target_link_libraries(status-cpp
    PRIVATE
        OpenSSL::OpenSSL
        std::filesystem
        status
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        Qt5::Quick
        Qt5::Qml
        Qt5::Concurrent
)

target_compile_features(status-cpp
    PRIVATE
        cxx_std_17
)
 

# Build constants
file (STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/resources/infura_key" INFURA_KEY)
target_compile_definitions(status-cpp
    PRIVATE
        INFURA_KEY="${INFURA_KEY}"
)