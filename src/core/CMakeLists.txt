add_library(core
    constants.cpp
    mailserver-cycle.cpp
    settings.cpp
    status.cpp
    utils.cpp
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(core
    PRIVATE
        status
        Qt5::Core
        Qt5::Gui
        Qt5::Qml
        Qt5::Concurrent
)
