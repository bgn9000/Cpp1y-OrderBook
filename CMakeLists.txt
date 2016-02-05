cmake_minimum_required(VERSION 3.1)

project(Cpp14-OrderBook)

# External project
add_subdirectory(ext/rapidcheck)

# Optimization flag
# Can be change, Example: cmake -DGCC_MAXSPEED_OPTIMIZATION=-Ofast
set(GCC_MAXSPEED_OPTIMIZATION -O2) 
add_compile_options(${GCC_MAXSPEED_OPTIMIZATION})

# Sanity check
# Can be enable, Example: cmake -Dsanitize=ON
option(sanitize "Sanity check" OFF)
if(sanitize)
add_compile_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=signed-integer-overflow -fsanitize=shift -fsanitize=integer-divide-by-zero -fsanitize=null)
endif()

# Warnings
add_compile_options(-pedantic-errors -Wconversion -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function) #-Wpadded 

# Tests
enable_testing()

# Source code
add_subdirectory(utils)
add_subdirectory(main)


