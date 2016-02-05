file(GLOB source_files src/*.cpp)
file(GLOB header_files include/utils/*.h)

add_library(Utils ${source_files} ${header_files})

target_include_directories(Utils PUBLIC include)

# This project requires features from C++11 and C++14
# See https://cmake.org/cmake/help/latest/prop_gbl/CMAKE_CXX_KNOWN_FEATURES.html
target_compile_features(Utils PUBLIC cxx_digit_separators)


# Unit-Tests

add_executable(test_CircularBlock tests/unit/test_CircularBlock.cpp)
find_package(Threads) # test_CircularBlock requires pthread_create
target_link_libraries(test_CircularBlock Utils rapidcheck Threads::Threads)
add_test(CircularBlock test_CircularBlock)

add_executable(test_Decoder tests/unit/test_Decoder.cpp)
target_link_libraries(test_Decoder Utils rapidcheck)
add_test(Decoder test_Decoder)

add_executable(test_Parser tests/unit/test_Parser.cpp)
target_link_libraries(test_Parser Utils rapidcheck)
add_test(Parser test_Parser)

add_executable(test_SimpleBuffer tests/unit/test_SimpleBuffer.cpp)
target_link_libraries(test_SimpleBuffer Utils rapidcheck)
add_test(SimpleBuffer test_SimpleBuffer)

add_executable(test_StrStream tests/unit/test_StrStream.cpp)
target_link_libraries(test_StrStream Utils rapidcheck)
add_test(StrStream test_StrStream)






