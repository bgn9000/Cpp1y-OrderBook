add_library(FeedHandler src/FeedHandler.cpp src/FeedHandler.h src/Reporter.cpp src/Reporter.h)

add_executable(FeedHandler.out src/main.cpp)

find_package(Threads)
target_link_libraries(FeedHandler Utils Threads::Threads)

target_link_libraries(FeedHandler.out FeedHandler)

# Include directory for unit-tests
target_include_directories(FeedHandler INTERFACE src)

# Unit-Tests

add_executable(test_FeedHandler tests/unit/test_FeedHandler.cpp)
target_link_libraries(test_FeedHandler FeedHandler rapidcheck)
add_test(FeedHandler test_FeedHandler)


