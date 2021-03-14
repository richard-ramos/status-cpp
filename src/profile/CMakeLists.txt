add_library(profile
    custom-networks-model.cpp
    devices-model.cpp
    mailserver-model.cpp
    mailserver-cycle.cpp
)

target_include_directories(profile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(profile
    PRIVATE
        status
        core
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        Qt5::Quick
        Qt5::Concurrent
)
