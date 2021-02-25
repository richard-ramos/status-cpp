add_library(chat
    chat-type.cpp
    chat.cpp
    chats-model.cpp
    content-type.cpp
    message-format.cpp
    message.cpp
    messages-model.cpp)

target_include_directories(chat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(chat
    PRIVATE
        status
        contacts
        core
        Qt5::Core
        Qt5::Widgets
        Qt5::Quick
        Qt5::Concurrent
)

include_directories(
    ../../include
    ../../vendor/lib-qt-qml-tricks/include
    ../../vendor/bitcoin-core-base58/
)