#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

namespace common
{
    using OrderId = unsigned int;
    using Quantity = unsigned int;
    using Price = double;
    using LimitNum = unsigned int;
}
