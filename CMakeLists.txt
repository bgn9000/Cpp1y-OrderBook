cmake_minimum_required(VERSION 3.1)

project(Cpp14-OrderBook)


## OPTIONS ##

option(SANITIZE "Sanity check" OFF)
# To enable: cmake -DSANITIZE=ON

set( MARCH "corei7"  CACHE STRING "Control flag -march" )
# Default produce -march=corei7
# To override use for example:    cmake .. -DMARCH=native (if native => convert to real cpu-type)
# To disable provide empty value: cmake .. -DMARCH=

set( OPTIM "" CACHE STRING "Control flags -Ox" )
# Example: cmake -DOPTIM=-Ofast

set( GnuOrClang $<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>> CACHE BOOL "Check if C++ compiler is GNU or Clang" )


## CMAKE_BUILD_TYPE ##

# Release  = Debug info g2 + disable assert + Optim (-O2)
# Debug    = Debug info g3 + enable  assert + Step-by-Step (-Og) + -D_GLIBCXX_DEBUG_PEDANTIC
# Coverage = Debug info g3 + disable assert + Step-by-Step (-Og) + Coverage

# Default = Release
if( NOT CMAKE_BUILD_TYPE )
    message(STATUS "** Setting build type to 'Release' as none was specified.")
    set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE )
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "Coverage") # For 'cmake -i' and 'cmake-gui'
endif()

# Depending on CMAKE_BUILD_TYPE: Set -march and --coverage (check also tools gcov/...)
if( CMAKE_BUILD_TYPE STREQUAL "Coverage" )
    if( NOT CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
        message(SEND_ERROR "Coverage only supported by GCC and Clang")
    endif()
    find_program( path_gcov    gcov    )
    find_program( path_lcov    lcov    )
    find_program( path_genhtml genhtml )
    find_program( path_gcovr   gcovr   )
    if( NOT path_gcov )
        message(SEND_ERROR "Cannot find gcov => Exit")
    endif()
    if( NOT path_lcov )
        message(SEND_ERROR "Cannot find lcov => Exit")
    endif()
    if( NOT path_gcovr )
        message(SEND_ERROR "Cannot find gcovr => Exit")
    endif()
    if( NOT path_genhtml )
        message(WARNING "Cannot find genhtml => Cannot generate HTML")
    endif()
    add_compile_options( --coverage )
    link_libraries(      --coverage )
    add_definitions( -DNDEBUG )  # Disable assert to avoid code coverage bias
endif()

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    add_definitions(-D_GLIBCXX_DEBUG_PEDANTIC)
endif()


## Compilation flag -march ##

if( MARCH STREQUAL "native" )
    if( CMAKE_COMPILER_IS_GNUCXX )
        EXECUTE_PROCESS( COMMAND ${CMAKE_CXX_COMPILER} -march=native -Q --help=target COMMAND awk "/-march=/{ printf $2}" OUTPUT_VARIABLE march_native )
        message(STATUS "** MARCH is native and compiler is GNU => Detected processor '${march_native}' => -march=${march_native}")
        add_compile_options( -march=${march_native} )
    else()
        message(STATUS "** MARCH is native and compiler is *not* GNU => -march=native")
        add_compile_options( -march=native )
    endif()
elseif( MARCH )
    message(STATUS "** MARCH is not native => -march=${MARCH}")
    add_compile_options( -march=${MARCH} )
else()
    message(STATUS "** MARCH is empty => Do not set flag -march")
endif()


## Compilation flag -O0/-Og/-O1/-O2/-O3/-Ofast ##
if( OPTIM )
    message(STATUS "** OPTIM is set to ${OPTIM} => Add it as compilation flag")
    add_compile_options( ${OPTIM} )
endif()

## Speed up build if ccache installed ##
find_program( path_ccache ccache )
if( path_ccache )
    message(STATUS "** Command 'ccache' detected => Will use 'ccache' to speed up compilation and link" )
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK    ccache)
    # TODO(olibre): Use "CMAKE_{C,CXX}_COMPILER_LAUNCHER=ccmake" with cmake-v3.4
endif()

## Speed up build using pipes (rather than temporary files) for communication between the various GCC stages
add_compile_options( -pipe )
link_libraries(      -pipe )


# External dependency for unit tests
add_subdirectory(ext/rapidcheck)
enable_testing()


## Static code analysis ##
# Generate file 'compile_commands.json' for clang-check
# Usage: awk -F: '/"file"/{print $2 }' build/compile_commands.json | xargs clang-check -fixit -p build
set(CMAKE_EXPORT_COMPILE_COMMANDS "on")


## Instrument code for run-time analysis ##
if(SANITIZE)
    add_compile_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=signed-integer-overflow -fsanitize=shift -fsanitize=integer-divide-by-zero -fsanitize=null)
endif()


# Warnings
add_compile_options(-Wall -Wextra -Wswitch-enum -Wno-ignored-qualifiers -Weffc++ -pedantic -pedantic-errors -Wconversion -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function) #-Wpadded


# Full debug info for all CMAKE_BUILD_TYPE
add_compile_options( -g3 -ggdb3 )    # -g3 -> include also the MACRO definitions


# Source code
add_subdirectory(utils)
add_subdirectory(main)
