add_library(contacts
    contact.cpp
    contacts-model.cpp
)

target_include_directories(contacts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(contacts
    PRIVATE
        status
        chat
        core
        profile
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        Qt5::Qml
        Qt5::Concurrent
)
