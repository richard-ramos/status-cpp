add_library(login
    login-model.cpp
    onboarding-model.cpp
)

target_include_directories(login PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(login
    PRIVATE
        status
        core
        Qt5::Core
        Qt5::Gui
        Qt5::Quick
        Qt5::Concurrent
)

# Build constants
file (STRINGS "../../resources/infura_key" INFURA_KEY)
target_compile_definitions(login
    PRIVATE
        INFURA_KEY="${INFURA_KEY}"
)
