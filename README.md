C++14 OrderBook
===============

Clone, build and test
---------------------

Quick way

    git clone git@github.com:olibre/Cpp14-OrderBook.git --recursive --depth 1
    cd Cpp14-OrderBook
    mkdir build
    cd    build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j8
    make test

Clonning the Git repository use option `--recursive` because of submodules. But you can ommit the option `--depth 1`.

    git clone git@github.com:olibre/Cpp14-OrderBook.git --recursive

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
These allow verifying the repporter module.

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

List of reported CRITICAL and other errors  
(see functions `nbCriticalErrors()` and `nbErrors()` in `Common.h`)

- MODIFY LIMIT QUANTITY TOO LOW
- MODIFY LIMIT NOT FOUND
- CANCEL LIMIT QUANTITY TOO LOW
- CANCEL LIMIT NOT FOUND

- corrupted messages
- incomplete messages
- wrong actions
- wrong sides
- negative order ID
- negative quantitie
- negative price
- missing action
- missing order ID
- missing side
- missing quantity
- missing price
- zero Order ID
- zero quantity
- zero price
- out of bounds Order ID
- out of bounds quantity
- out of bounds price
- duplicate Order ID
- modify with unknown Order ID
- modify not matched price
- cancel with unknown Order ID
- cancel not matched quantity or price
- best bid equal or greater than best ask


Mechanism
----------

An exchange feed conveys messages describing two distinct objects: *Orders* and *Trades*.

*Orders* are offers by market participants to buy or sell up to a given quantity at a specified price.
An order book represents the current orders: generally there are orders to buy at low prices, and orders to sell at high prices.
Orders can be created, modified and cancelled during the trading day.

*Trade* is produced when a buy order price is equal or greater than a sell order price, the two orders are 'matched' with each other.
The trade quantity corresponds to the smaller of the two order quantities.

If there are multiple orders that could be matched against each other, the orders are matched first to the best possible price (i.e., the lowest-price sell or the highest-price buy order). In the (very common) event of a tie, orders are matched in time priority (first order to arrive will be filled first). For example, if the standing orders are:

```
price orders (oldest to newest, B = buy, S = sell)
1100
1075 S 1 // one order to sell up to 1 at 1075
1050 S 10 // one order to sell up to 10 at 1050
1025 S 2 S 5 // one order to sell up to 2 at 1025,
// and second newer order to sell up to 5

1000 B 9 B 1 // buy up to 9 at 1000, second to buy up to 1
975 B 30 // buy up to 30 at 975
950
```

The best buy order is at a price of `1000`, and the best sell order is at a price of `1025`. Since no seller is willing to sell low enough to match a buyer, and no buyer is willing to buy high enough to match a seller, there is no match between any of the existing orders.

- If a new buy order arrives for a quantity of `3` at a price of `1050`, there will be a match.
- The only sells that are willing to sell at or below a price of `1050` are the `S10`, the `S2`, and the `S5`.
- Since the `S2` and `S5` are at a better price, the new order will match first against those.
- Since the `S2` arrived first, the new order will match against the full `S2` and produce a trade of `2`.
- However, the `1` remaining quantity will still match the `S5`, so it matches and produces a trade of `1`, and the `S5` becomes an `S4`.
- Two trade messages will be generated when this happens, indicating a trade of size `2` at price `1025`, and a trade of size `1` at price `1025`.
- Two order-related messages will also be generated: one to remove the `S2`, and one to note the modification of the `S5` down to an `S4`.

The new set of standing orders will be:
```
price orders
1100
1075 S 1
1050 S 10
1025 S 4

1000 B 9 B 1
975 B 30
950
```

Note that if a new sell order arrives at a price of `1025`, it will be placed to the right of the `S4` (i.e., behind it in the queue). Also, although there are only a few price levels shown here, you should bear in mind that buys and sells can arrive at any price level.


Messages
--------

The types of messages that can arrive on the exchange feed are as follows:

```
Order: action,orderid,side,quantity,price (e.g., A,123,B,9,1000)
action = A (add), X (remove), M (modify)
orderid = unique positive integer to identify each order;
used to reference existing orders for remove/modify
side = B (buy), S (sell)
quantity = positive integer indicating maximum quantity to buy/sell
price = double indicating max price at which to buy/min price to sell

Trade: action,quantity,price (e.g., T,2,1025)
action = T (trade)
quantity = amount that traded
price = price at which the trade happened
```


Example
-------

The following set of messages builds the initial book from the example in the `Background` section above. Note that order `100004` is canceled, so it doesn't appear in the book above.

```
A,100000,S,1,1075
A,100001,B,9,1000
A,100002,B,30,975
A,100003,S,10,1050
A,100004,B,10,950
A,100005,S,2,1025
A,100006,B,1,1000
X,100004,B,10,950 // cancel
A,100007,S,5,1025
```

The following sequence of additional messages represents the new order and subsequent trades, removes, and modifies that happens in the second part of the `Background` example above:

```
A,100008,B,3,1050
T,2,1025
T,1,1025
X,100008,B,3,1050
X,100005,S,2,1025
M,100007,S,4,1025 // order is modified down to reflect quantity traded
```

The state of the book after these messages is shown at the end of the `Background` section.


Problem
-------

(1) Given a sequence of messages, as defined above, construct an **in-memory** representation of the current state of the order book.
You will need to generate your own dataset to test your code.

(2) Write out a human-readable representation of the book every 10th message.

(3) Write out a machine-readable sequence of 'midquotes' (average of best buy and best sell order prices) on every message. For example, the correct sequence of midquotes for the messages given in the first part of the Example above is

```
NAN // no buys yet, so midquote is undefined
1037.5 // = (1075. + 1000.)/2.0
1037.5 // new buy at 975 is lower than best, so midquote doesn't change
1025 // = (1050. + 1000.)/2.0 (new sell is at a better price than old)
1025 // new buy at 950 is lower than best, so midquote doesn't change
1012.5 // = (1025. + 1000.)/2.0 (new sell is again lower than best)
1012.5 // new buy at 1000 changes quantity, but not midquote
1012.5 // cancel of old buy at 950 (less than best) doesn't change midquote
1012.5 // new sell at 1025 adds to quantity, but again doesn't change best
```

(4) Write out the total quantity traded at the most recent trade price on every trade message.
For example:

```
message => output
T,2,1025 => 2@1025
T,1,1025 => 3@1025
T,1,1000 => 1@1000 // quantity resets when new trade price is seen
T,1,1025 => 1@1025 // doesn't remember the previous trades @1025
```

(5) **Your code should be clean, easy to understand, efficient, and robust**. Specifically, your program should not crash if it gets:

- a. corrupted messages
- b. duplicated order ids (duplicate adds)
- c. trades with no corresponding order
- d. removes with no corresponding order
- e. best sell order price at or below best buy order price, but no trades occur
- f. negative, missing, or out-of-bounds prices, quantities, order ids

You should note how often each of these cases occurs and print a summary when the program finishes processing all messages.

(6) The following is a stub C++ program to get you started. Feel free to modify it as you see fit. You may use STL.


```C++
class FeedHandler
{
public:
    FeedHandler();

    void processMessage(const std::string &line);
    void printCurrentOrderBook(std::ostream &os) const;
};

int main(int argc, char **argv)
{
    FeedHandler feed;
    std::string line;
    const std::string filename(argv[1]);
    std::ifstream infile(filename.c_str(), ios::in);
    int counter = 0;
    while (std::getline(infile, line)) {
        feed.processMessage(line);
        if (++counter % 10 == 0) {
            feed.printCurrentOrderBook(std::cerr);
        }
    }
    feed.printCurrentOrderBook(std::cout);
    return 0;
}
```


Solution
--------


### Dev environment

All benchmarks below were done onto a `CPU (4 cores): Intel(R) Core(TM) i5-5200U CPU @ 2.20GHz` machine.
With:
```
Linux: 3.19.0-43-generic #49~14.04.1-Ubuntu)
Complier: gcc version 5.3.0 20151204 (Ubuntu 5.3.0-3ubuntu1~14.04)
Python: Python 2.7.6 (default, Jun 22 2015, 17:58:13)
```


### Main development choices

- Since 3.5 years, I am very focused on C++ new features.
  Many parts of boost are becoming less interresting since C++11 and C++14 new features (keywords and STL).
  Therefore this project does not use Boost but rely on C++14 which is cleaner and easier to read.

- **Only order quantity can be modified** (neither price nor order id, in that case I expect cancel and add new order).

- First version was a monothreaded program (from reading input file, parsing messages, orderbook management then print results).
=> Make it simple and make it work then consider optimization.
=> Good design (KISS) helps to simplify later refactoring (like usage of `auto`).

- Focus on parsing messages and printing results latency.

- Tests of different ways to read input files (`getline` => `mmap`).

- Unit tests (50% of overall lines of code) from the beginning with random generators to cover much larger possibilities of failure.

- Second version with two threads to separate critical path (parsing messages and orderbook management) versus print results.


### Benchmark results

#### Unit tests

- Parse Quantity: `std::stoul [105] Decoder::retreive_unsigned_integer [45] (in ns)`
- Parse Price: `std::stod [531] Decoder::retreive_float [259] (in ns)`
- Convert Quantity to string: `std::to_string [262] Decoder::convert_unsigned_integer [65] (in ns)`
- Convert Price to string: `sprintf [1716] Decoder::convert_float [172] (in ns)`
- Parse good order lines: `internal parser [610] naive stl impl [1723] (in ns)`
- Parse good trade lines: `internal parser [317] naive stl impl [811] (in ns)`
- Read file (test6.txt): `line by line [19'254'867] std::getline [34'078'561] with mmap [16'158'367] (in ns)`
- FeedHandler and Reporter tests (see below in `Design benchmarks comparison` section)

#### Test cases files

- test1.txt: `new (12), modify (2), cancel (6), trade (4)`
```
Overall processing time for monothreaded version is 217 usec
Overall processing time for dual threads version is 221 usec sec but for parsing messages and orderbook management is 31 usec
```

- test5.txt: `new (50376), modify (49721), cancel (50052), trade (99773)`
```
Overall processing time for monothreaded version is 0.487432 sec
Overall processing time for dual threads version is 0.490797 sec but for parsing messages and orderbook management is 0.075556 sec
```

- test6.txt: `new (239893), modify (94891), cancel (239888), trade (200891)`
```
Overall processing time for monothreaded version is 1.474538 sec
Overall processing time for dual threads version is 1.499489 sec but for parsing messages and orderbook management is 0.238893 sec
```

- External allocators (`gperftools, lockless`) with test6.txt:
```
with gperftools 2.4:
Overall processing time for monothreaded version is 1.467972 sec
Overall processing time for dual threads version is 1.455903 sec but for parsing messages and orderbook management is 0.243840 sec
with lockless:
Overall processing time for monothreaded version is 1.47099 sec
Overall processing time for dual threads version is 1.448604 sec but for parsing messages and orderbook management is 0.242458
```

- Compiler flags for optimization (default is -O2): -O3 or -Ofast: nothing noticeable for those tests.


### Discussion

#### Data structures

In my past experience, I did several benchmarks with different data structures in order to come up with the most efficient in terms of access (insertion, deletion and searching) and user friendly in terms of readability (further maintenance and team adoption).
Of course, **there is no obvious perfect solution and it depends on situation and experience we have with the software**.

Usually I prefer using straightforward `stl containers` like `map` (for orders) and `vector` (for limits) but I chose here `unordered_map` and `deque` to bring more performance for middle size orderbooks (around `10k to 100k` live orders which is high even for liquid instruments).
Beyond a `map` would be a better choice in terms of memory. Less a `vector` would be a bit faster when looping over limit prints each 10 messages.

Only continuous tests and measurments will allow us to keep the best choice.

#### Fast Code after code fast (and clean)

Once we wrote a working clean and efficient code, it is time to consider optimizations when our target is low latency software.
To address this, there are several topics to consider:

##### Design benchmarks comparison

Here is some numbers to show the cost of a dual threads design (due to the intermediate queue and duplication of orderbook management) compared to monothreaded design but also the expected benefit for the `critical path`:

- In monothread design (only FeedHandler managing its orderbook):

> Prefill (100000 buy and sell orders) and empty orderbooks performance comparisons (in ns)

|Action|Prefill buy|Empty buy|Prefill sell|Empty sell|
|------------------------|:---------:|:---------:|:---------:|:---------:|
|**new order**|647|384|580|390|
|**new order same price**|560|344|549|397|
|**modify order**|518|190|501|346|
|**cancel order**|500|268|502|234|

- In dual thread design (FeedHandler and Reporter linked by the wait free queue):

> Prefill (100000 buy and sell orders) and empty orderbooks performance comparisons (in ns)

|Action|Prefill buy|Empty buy|Prefill sell|Empty sell|
|------------------------|:---------:|:---------:|:---------:|:---------:|
|**new order**|856|498|727|551|
|**new order same price**|604|546|629|499|
|**modify order**|419|363|421|324|
|**cancel order**|582|337|644|336|

- Where is the latency?
```
Print prefilled (100000) orderbook perfs                    : [26'964'652] (in ns)
Copy prefilled (100000) orderbook perfs				        : [2'786'017] (in ns)
Insert at beginning of prefilled (100000) orderbook perfs	: [968] (in ns)
```

As we can see, Print is the latency killer whereas adding at beginning is much faster.
Then the overhead introduced by the dual thread design is negligeable compared to the Print latency cost.

##### Software architecture with regards of system architecture
            
            Make the critical path only doing critical

This means we may consider multithreading if we have enough core (and optimally cores on different cpus for L3 cache):
```
C++: std::thread::hardware_concurrency.
cat /proc/cpuinfo
cat /sys/devices/system/cpu/cpu*/topology/thread_siblings_list
```

Even if the expected solution here doesn't specify a critical processing in the problem statements, I chose to separate in two threads (for simplicity of synchronisation):
- First one is parsing input messages and managing orders to allow a live limit orderbook.
- Second one to receive updates (or events) to rebuild his own copy of the limit orderbook (and keep track of errors) and do all print tasks like a logging mechanism.

My first attempt was to develop a simple ring buffer implementation but because of the different latencies between the two threads and the continuous input, the first one was spending too much time to wait the second thread.
I moved to a simple wait free queue with a `deque` and a `spinlock` as barrier which is not optimal but quicker to implement and easier to maintain.
Anyway, there are many good reading on the web like [1024cores](http://www.1024cores.net/home/lock-free-algorithms/introduction).

System architecture considerations:
- When doing this, we have to pay attention of cpu cache to avoid "false" sharing (thanks `Martin Thompson`), I added padding.
I could check in the program the cache size but I chose to put 2 padding of 64 characters that should cover most of the current situations.
```
size_t cache_line_size() {
    FILE * p = 0;
    p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
    unsigned int i = 0;
    if (p) {
        auto ret = fscanf(p, "%d", &i);
        fclose(p);
    }
    return i;
}
```
- Also, we need to pay attention of concurrent heap memory allocation (`gperftools` or other allocator can fix this otherwise an object pool could be implemented).
- I also tried to play with memory page locks (for page faults) but I didn't find a good usage here. I didn't try prefault or prefetch neither.
- For cache miss, since the beginning I stored quantity/price `tuples` into `deque` for print loops and binary searches (dichotomy) efficiency.
- I prefer to use `mmap` to relay on OS to flush the data from/to disk in asynchrone for the input file.
- Branching prediction (likely/unlikely macros). Perfers switchs than list of if/else when it was possible.

##### System tuning (not tried here but I have good experience on it)

- GCC flags (-mtune=native)
- CPU overclocking
- CPU affinity (`pthread_setaffinity_np`)
- Scheduler (FIFO, RT)
- Isolated CPU (IRQ affinity)
- Network stack (kernel-by-pass)
- SSD (but disk controllers are very efficient, so tests are mandatory).


#### Keep focus on time to market, production concerns and incremental improvments

Continuous deployment is the key of success to acheive both targets (doing the right thing, doing the thing right and deliver fast).
Unit tests with coverage is the basic thing to do at the very beginning. RapidCheck is a good tool to go beyond guessing test cases.
Simulator or test cases generator (in Python) is the second step of validation. They allow quick and high availability feedback of the forecast delivery.


