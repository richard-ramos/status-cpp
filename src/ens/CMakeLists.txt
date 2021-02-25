add_library(ens
    ens-model.cpp
    ens-utils.cpp
)

target_include_directories(ens PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(ens
    PRIVATE
        status
        core
        Qt5::Core
        Qt5::Widgets
        Qt5::Quick
        Qt5::Concurrent
)
