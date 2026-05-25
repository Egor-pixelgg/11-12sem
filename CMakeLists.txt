cmake_minimum_required(VERSION 3.20)
project(sem11__12 LANGUAGES CXX)

cmake_policy(SET CMP0167 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost 1.74 REQUIRED)
find_package(Threads REQUIRED)

add_executable(sem11__12
        main.cpp
    echo_session.cpp
    listener.cpp
    read_from.cpp
    multiplexer.cpp
        bank_account.cpp
)

target_include_directories(sem11__12 PRIVATE ${Boost_INCLUDE_DIRS})

target_compile_definitions(sem11__12 PRIVATE
    BOOST_ASIO_HAS_CO_AWAIT
)

if(MINGW)
    target_link_options(sem11__12 PRIVATE -Wl,--allow-multiple-definition)
endif()

if(WIN32)
    target_link_libraries(sem11__12 PRIVATE ws2_32 mswsock)
endif()

target_link_libraries(sem11__12 PRIVATE Threads::Threads)
