Quick Start
-----------

    git clone git@github.com:bgn9000/Cpp1y-OrderBook.git --recursive --depth 1
    cd Cpp1y-OrderBook
    mkdir build
    cd build
    CC=gcc-5         # Optional if your default compiler
    CXX=g++-5        # does not support C++14
    cmake ..
    make -j8
    make test

Clone
-----

Cloning the Git repository use option `--recursive` because of submodules. But you can ommit the option `--depth 1`.

    git clone git@github.com:bgn9000/Cpp1y-OrderBook.git --recursive

To upgrade cmake (needs upper than 3.1 whereas ubuntu 14.04 provides only cmake 2.8)
    
    git clone https://cmake.org/cmake.git
    cd cmake
    git checkout v3.4.3
    cmake .
    make
    
You can use checkinstall as this creates a DEB package.

    sudo apt-get install checkinstall
    sudo checkinstall
    
or

    make install

Directory
---------

Most of the time CMake generates files for the build tool in directory `build` and the build tool will produce its temporary and finals files within the same directory. But you can customize this directory:

    mkdir /my/build/path
    ( cd  /my/build/path && cmake $OLDPWD )
    make -C /my/build/path all
    make -C /my/build/path test

Target
------

Instead of building `all` just build the final executable and its dependencies.

    make FeedHandler.out

Build Types
-----------

By default, build type is Release (`CMAKE_BUILD_TYPE=Release`) but you can also build in Debug mode:

    cmake .. -DCMAKE_BUILD_TYPE=Debug

Or in Coverage mode:

    cmake .. -DCMAKE_BUILD_TYPE=Coverage

Compiler Cache
--------------

`ccache` can speed up compilation and link. If you clean and rebuild often, then you should install `ccache`. This project is configured to use it when available.

    sudo apt install ccache
    cd build
    cmake ..
    time make  # First build: ccache caches all compiler output
    make clean
    time make  # ccache detects same input and bypasses the compiler

Options
-------

Option `CMAKE_EXPORT_COMPILE_COMMANDS` is enabled and will produce the file `compile_commands.json` during the compilation. This file can be then used by static code analysis tools as `clang-check`:

    awk -F: '/"file"/{print $2 }' build/compile_commands.json | xargs clang-check -fixit -p build

Option `SANITIZE=ON` let you run the run-time code analysis.

    cmake .. -DSANITIZE=ON

Option `MARCH` let you control the CFLAG `-march`. By default `MARCH=corei7` (`-march=corei7`). If `MARCH=native` CMake will request `gcc` the real *cpu-type* used in order to keep a reproductible build on another machine. Unset flag `-march` using empty option `MARCH=`.   

    cmake .. -DMARCH=native  # Request gcc to provide the corresponding cpu-type
    cmake .. -DMARCH=        # Disable flag -march

Option `OPTIM` let you control the flags `-O0 -Og -O1 -Os -O2 -O3 -Ofast`.

    cmake .. -DOPTIM=-Ofast

By default this is disable and will used the `CMAKE_BUILD_TYPE` default flag:

* `-O2` for Release
* unset for Debug and Coverage

Build Tool
----------

CMake does not build the project (i.e. CMake is not a compilation or a link tool).
CMake generates files for a build tool as [gmake, nmake, ninja, Visual Studio...](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html).

By default on CMake will generates Makefiles on Unix-like plateforms. To enable verbose mode use `VERBOSE=1` to display the full command lines during build.

    make VERBOSE=1

If [`ninja`](https://github.com/ninja-build/ninja) is available, you can use it instead of `gmake`.

    cmake .. -G Ninja
    ninja
    ninja test

Use `ninja -v` for verbose mode (to display the full command lines during build).

    cmake .. -DCMAKE_BUILD_TYPE=Debug -G Ninja
    ninja -v

Use `cmake --build your-build-path` to abstract the build tool.

    cmake .. -G "${MY_CMAKE_GENERATOR}"
    cmake --build .

Compiler
--------

You can also select another compiler.

    CC=clang CXX=clang++ cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build .
    ctest

    CC=clang
    CXX=clang++
    cmake .. -DSANITIZE=ON -G Ninja
    cmake --build .

Test
----

Use `cmake --build .` and `ctest` as an abstraction of the specific build tool (`ninja` or `make`)

    cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja
    cmake --build .
    ctest

    cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja
    cmake --build . --target FeedHandler.out


 
Dependencies
------------

This project uses two external tools present in submodules
(the submodules are located in sub-directory `ext`)

1. [**RapidCheck**](https://github.com/furuholm/rapidcheck) to run unit-tests, written by [emil-e](https://github.com/emil-e) and recently improved by [Tobias Furuholm](https://github.com/furuholm)
  Recommended reading: [Generating test cases so you don’t have to](https://labs.spotify.com/2015/06/25/rapid-check)
  (RapidCheck also depends on another tool, [Catch](https://github.com/philsquared/Catch), present in another submodule within RapidCheck's sub-directory `ext`)

2. [**Python OrderBook**](https://github.com/dyn4mik3/OrderBook) to generate test cases, written by [dyn4mik3](https://github.com/dyn4mik3) with recent help from [bgn9000](https://github.com/bgn9000)
  depends on `bintrees` (requiring cython to be installed before)

        sudo apt install python-pip cython
        pip install bintrees

  or

        sudo apt install python-pip
        pip install cython
        pip install bintrees

Test Cases
----------

Use `genOrders.py` to generate messages.

    tools/genOrders.py 2>test.txt | tee test.log

* Result is written to stderr
* On-going information is written to stdout

Executable Output
-----------------

Use your own `test.txt` or use one of the available test cases in `main/tests/perf/test*.txt`

The executable `FeedHandler.out` ends by writting a summary of errors found.

The test case `test1.txt` contains commented/blank lines and a cross not followed by a trade.
These allow verifying the reporter module.

    $ build/main/FeedHandler.out main/tests/perf/test1.txt 2>result1.txt
    Verbose is 0 : default is 0, param '-v 1 or higher' to activate it
    Full Bids/Asks:
    0     : 15 @ 1000.000000                18 @ 1000.000000
    1     : 30 @ 975.000000                 4 @ 1025.000000
    2     : empty                           10 @ 1050.000000
    3     : empty                           1 @ 1075.000000
    4     : empty                           empty
    Summary:
     [4] commented lines
     [5] blank lines
    Found 1 error:
     [1] best bid equal or upper than best ask
    No critical error found
    Overall run perfs: 0 sec 13026 usec (building OB: 0 sec 52 usec)

The last line `Overall` provide some benchmarks.

The test case `test2.txt` is cleaner.

    $ build/main/FeedHandler.out main/tests/perf/test2.txt 2>result2.txt
    Verbose is 0 : default is 0, param '-v 1 or higher' to activate it
    Full Bids/Asks:
    0     : 476 @ 909.000000                368 @ 960.000000
    1     : 154 @ 902.000000                446 @ 972.000000
    2     : 365 @ 900.000000                679 @ 992.000000
    3     : empty                           429 @ 998.000000
    4     : empty                           340 @ 1013.000000
    5     : empty                           337 @ 1035.000000
    6     : empty                           386 @ 1151.000000
    7     : empty                           583 @ 1178.000000
    8     : empty                           500 @ 1181.000000
    9     : empty                           699 @ 1194.000000
    10    : empty                           empty
    Summary:
    No error found
    No critical error found
    Overall run perfs: 0 sec 895 usec (building OB: 0 sec 229 usec)

Here is the list of reported errors (in Common.h):
```C++
        unsigned long long nbErrors()
        {
            return  corruptedMessages +
                    IncompleteMessages +
                    wrongActions +
                    wrongSides +
                    negativeOrderIds +
                    negativeQuantities +
                    negativePrices +
                    missingActions +
                    missingOrderIds +
                    missingSides +
                    missingQuantities +
                    missingPrices +
                    zeroOrderIds +
                    zeroQuantities +
                    zeroPrices +
                    outOfBoundsOrderIds +
                    outOfBoundsQuantities +
                    outOfBoundsPrices +
                    duplicateOrderIds +
                    modifiesWithUnknownOrderId +
                    modifiesNotMatchedPrice +
                    cancelsWithUnknownOrderId +
                    cancelsNotMatchedQtyOrPrice +
                    bestBidEqualOrUpperThanBestAsk;
        }
        
        unsigned long long nbCriticalErrors()
        {
            return  modifiesLimitQtyTooLow +
                    modifiesLimitNotFound +
                    cancelsLimitQtyTooLow +
                    cancelsLimitNotFound;
        }
```
