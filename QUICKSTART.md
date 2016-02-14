Clone, build and test
---------------------

Quick way

    git clone git@github.com:bgn9000/Cpp1y-OrderBook.git --recursive --depth 1
    cd Cpp14-OrderBook
    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j8
    make test

Clonning the Git repository use option `--recursive` because of submodules. But you can ommit the option `--depth 1`.

    git clone git@github.com:bgn9000/Cpp1y-OrderBook.git --recursive

You can also build in Debug mode.

    cmake .. -DCMAKE_BUILD_TYPE=Debug

Option `sanitize=ON` let you run the static code analysis.

    cmake .. -Dsanitize=ON

Instead of building `all` just build the final executable and its dependencies.

    make FeedHandler.out

Use `VERBOSE=1` to display the full command lines during build.

    make VERBOSE=1

If [`ninja`](https://github.com/ninja-build/ninja) is available, you can use it instead of `gmake`.

    cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja
    ninja
    ninja test

Use `ninja -v` to display the full command lines during build.

Use `cmake --build .` and `ctest` as an abstraction of the specific build tool (`ninja` or `make`)

    cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja
    cmake --build .
    ctest

    cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja
    cmake --build . --target FeedHandler.out

You can also select another compiler.

    CC=clang CXX=clang++ cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build .
    ctest

    export CC=clang
    export CXX=clang++
    cmake .. -Dsanitize=ON -G Ninja
    cmake --build .

 
Dependencies
------------

This project uses two external tools present in submodules
(the submodules are located in sub-directory `ext`)

1. [**RapidCheck**](https://github.com/furuholm/rapidcheck) to run unit-tests, written by [emil-e](https://github.com/emil-e) and recently improved by [Tobias Furuholm](https://github.com/furuholm)
  Recommended reading: [Generating test cases so you don’t have to](https://labs.spotify.com/2015/06/25/rapid-check)
  (RapidCheck also depends on another tool, [Catch](https://github.com/philsquared/Catch), present in another submodule within RapidCheck's sub-directory `ext`)

2. [**Python OrderBook**](https://github.com/dyn4mik3/OrderBook) to generate test cases, written by [dyn4mik3](https://github.com/dyn4mik3) with recent help from [Philippe Bourgeon](https://github.com/bgn9000)
  depends on `bintrees` (requiring cython to be installed before)

        sudo apt install python-pip cython
        pip install bintrees

  or

        sudo apt install python-pip
        pip install cython
        pip install bintrees

Test cases
----------

Use `genOrders.py` to generate messages.

    tools/genOrders.py 2>test.txt | tee test.log

* Result is written to stderr
* On-going information is written to stdout


Executable output
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
    Overall run perfs: 0 sec 410 usec (building OB: 0 sec 115 usec)

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
    Overall run perfs: 0 sec 1307 usec (building OB: 0 sec 815 usec)

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

