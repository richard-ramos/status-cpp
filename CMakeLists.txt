cmake_minimum_required(VERSION 3.17 FATAL_ERROR)


project(status-cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif() 

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

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/SortFilterProxyModel)
target_compile_options(SortFilterProxyModel PRIVATE -fPIC)

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
  BUILD_BYPRODUCTS ${STATUSGO_LIB_DIR}/libstatus.so
)

ExternalProject_Get_Property(status-go SOURCE_DIR)
add_library(status STATIC IMPORTED)
set_property(TARGET status PROPERTY IMPORTED_LOCATION ${STATUSGO_LIB_DIR}/libstatus.so)
add_dependencies(status status-go)
include_directories(${STATUSGO_LIB_DIR})
# End: status-go


################################################################################
# Begin: QR-Code-generator
set(QRCODE_ROOT ${CMAKE_BINARY_DIR}/QR-Code-generator)
set(QRCODE_LIB_DIR ${QRCODE_ROOT}/src/QR-Code-generator/cpp)

ExternalProject_Add(QR-Code-generator
  PREFIX ${QRCODE_ROOT}
  GIT_REPOSITORY "git://github.com/status-im/QR-Code-generator"
  GIT_TAG "master"
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
  BUILD_IN_SOURCE 1
  BUILD_COMMAND make -C cpp V=1
  BUILD_BYPRODUCTS ${QRCODE_LIB_DIR}/libqrcodegen.a
)

ExternalProject_Get_Property(QR-Code-generator SOURCE_DIR)
add_library(qrcodegen STATIC IMPORTED)
set_property(TARGET qrcodegen PROPERTY IMPORTED_LOCATION ${QRCODE_LIB_DIR}/libqrcodegen.a)
add_dependencies(qrcodegen QR-Code-generator)
include_directories(${QRCODE_LIB_DIR})
# End: QR-Code-generator


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


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/lib-qt-qml-tricks/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/bitcoin-core-base58/
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/core)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/contacts)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/chat)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/ens)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/login)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/profile)

set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/logs.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/bitcoin-core-base58/base58.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.qrc"
)

add_executable(status-cpp 
    ${SOURCES}
    $<TARGET_OBJECTS:SortFilterProxyModel> 
)

target_compile_features(status-cpp
    PRIVATE
        cxx_std_17
)

target_link_libraries(status-cpp
    PRIVATE
        Qt5::Core
        Qt5::Widgets
        Qt5::Quick
        chat
        contacts
        core
        ens
        login
        profile
        OpenSSL::OpenSSL
        # std::filesystem
        status
        qrcodegen
)

target_compile_features(status-cpp
    PRIVATE
        cxx_std_17
)
TARGET_COMPILE_DEFINITIONS(status-cpp
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)


qt5_add_binary_resources(static-resources "${CMAKE_CURRENT_SOURCE_DIR}/qml/resources.qrc" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/static-resources.rcc")

add_dependencies(status-cpp static-resources)


# Ninja run!
add_custom_target(run
    COMMENT
    DEPENDS static-resources
    COMMAND  "./status-cpp"
    VERBATIM
    USES_TERMINAL
)

# Ninja run debug
add_custom_target(run-debug
    COMMENT
    DEPENDS static-resources
    COMMAND  ./status-cpp -qmljsdebugger=port:1234
    VERBATIM
    USES_TERMINAL
)
