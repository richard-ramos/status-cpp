add_library(wallet
    token-model.cpp
    wallet-model.cpp
    wallet.cpp
    price-watcher.cpp
    balance-watcher.cpp
)

target_include_directories(wallet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(wallet
    PRIVATE
        status
        core
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        Qt5::Quick
        Qt5::Concurrent
)
