cmake_minimum_required(VERSION 3.5)

project (EventProcessor CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/conan.cmake)
conan_cmake_configure(REQUIRES gtest/cci.20210126
                      GENERATORS cmake_find_package)

conan_cmake_autodetect(settings)
conan_cmake_install(PATH_OR_REFERENCE .
                    BUILD missing
                    REMOTE conancenter
                    SETTINGS ${settings})

add_library(SuperQueueLib INTERFACE)
target_include_directories(SuperQueueLib INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/superqueue>
    $<INSTALL_INTERFACE:superqueue>)

add_library(SuperFactoryLib INTERFACE)
target_include_directories(SuperFactoryLib INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/superfactory>
    $<INSTALL_INTERFACE:superfactory>)

add_library(EventBusLib INTERFACE)
target_include_directories(EventBusLib INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

find_package(GTest)
find_package(Threads REQUIRED)

add_executable(EventProcessorTest tests/AllTests.cpp)
target_link_libraries(EventProcessorTest
    SuperQueueLib
    SuperFactoryLib
    EventBusLib
    Threads::Threads
    GTest::gtest
    GTest::gtest_main
)

add_executable(BenchMark benchmarks/Benchmarks.cpp)
target_link_libraries(BenchMark
    SuperQueueLib
    SuperFactoryLib
    EventBusLib
    Threads::Threads
)

enable_testing()
add_test(test_all EventProcessorTest)
