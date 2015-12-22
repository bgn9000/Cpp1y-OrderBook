#include <rapidcheck.h>

#include "Parser.h"
#include "Decoder.h"

#include <cmath>
#include <cstring>

#include <chrono>
using namespace std::chrono;

int main(int argc, char **argv)
{
    auto verbose = 0;
    if (argc == 3)
    {
        if (!strcmp(argv[1], "-v")) verbose = std::stoi(argv[2]);
    }
    std::cout << "Verbose is " << verbose << " : default is false, param '-v' to activate it" << std::endl;
    
    auto spaces = [](int n) -> std::string
    {
        return std::string(*rc::gen::inRange(0, n), ' ');
    };
    
    auto time_span1 = 0ULL, time_span2 = 0ULL;
    auto nbTests = 0U;
    rc::check("Skip comment lines", [&](std::string comment) 
    {
        std::string line = spaces(10) + "//" + spaces(10) + comment;
        high_resolution_clock::time_point start = high_resolution_clock::now();
        Parser parser;
        auto ret = parser.parse(line, verbose);
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += duration_cast<nanoseconds>(end - start).count();
        
        ++nbTests;
            
        RC_ASSERT(ret == false);
    });
    if (nbTests)
    {
        std::cout << "Skip comment lines perfs  [" << time_span1/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Parse good order lines", [&](OrderId orderId, Quantity qty, Price price, std::string comment) 
    {
        const auto action = *rc::gen::element('A', 'X', 'M');
        char orderIdStr[64] = {};
        auto len = Decoder::convert_unsigned_integer<OrderId>(orderId, orderIdStr);
        const auto side = *rc::gen::element('B', 'S');
        char qtyStr[64] = {};
        len = Decoder::convert_unsigned_integer<Quantity>(qty, qtyStr);
        price = std::abs(price); // only positive prices
        char priceStr[64] = {};
        len = Decoder::convert_float<Price>(priceStr, price, std::numeric_limits<Price>::digits10);
        
        std::string line =  spaces(10) + action + spaces(10) + ',' +
                            spaces(10) + orderIdStr + spaces(10) + ',' +
                            spaces(10) + side + spaces(10) + ',' +
                            spaces(10) + qtyStr + spaces(10) + ',' +
                            spaces(10) + priceStr + spaces(10);
        RC_LOG() << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "line [" << line << ']' << std::endl;
        
        auto test_parse_stl = [&]() 
        {
            high_resolution_clock::time_point start = high_resolution_clock::now();
            auto pos = line.find_first_of("AXM");
            auto _action = line[pos];
            pos = line.find(',', pos+1);
            auto pos2 = line.find(',', ++pos);
            auto _orderId = std::stoul(line.substr(pos, pos2-pos));
            pos = line.find_first_of("BS", pos2);
            auto _side = line[pos];
            pos = line.find(',', pos+1);
            pos2 = line.find(',', ++pos);
            auto _qty = std::stoul(line.substr(pos, pos2-pos));
            pos = line.find(',', pos+1);
            pos2 = line.find(',', ++pos);
            auto _price = std::stod(line.substr(pos, pos2-pos));
            high_resolution_clock::time_point end = high_resolution_clock::now();
            time_span2 += duration_cast<nanoseconds>(end - start).count();
            
            RC_ASSERT(action == _action);
            RC_ASSERT(orderId == _orderId);
            RC_ASSERT(side == _side);
            RC_ASSERT(qty == _qty);
            RC_ASSERT(price - _price < std::pow(10, -std::numeric_limits<Price>::digits10));
        };
        
        auto test_parse = [&]() 
        {
            Parser parser;
            high_resolution_clock::time_point start = high_resolution_clock::now();
            bool ret = parser.parse(line, verbose);
            high_resolution_clock::time_point end = high_resolution_clock::now();
            time_span1 += duration_cast<nanoseconds>(end - start).count();
            RC_ASSERT(action == parser.getAction());
            RC_ASSERT(orderId == parser.getOrderId());
            RC_ASSERT(side == parser.getSide());
            RC_ASSERT(qty == parser.getQty());
            RC_ASSERT(price - parser.getPrice() < std::pow(10, -std::numeric_limits<Price>::digits10));
            RC_ASSERT(ret == true);
        };
        
        test_parse();
        test_parse_stl();
        line += ("//" + comment);
        test_parse();
        test_parse_stl();
        nbTests += 2;
    });
    if (nbTests)
    {
        std::cout << "Parse good order lines perfs  [" << time_span1/nbTests << "] naive stl impl [" 
            << time_span2/nbTests << "] (in ns)" << std::endl;
    }

    rc::check("Parse partial order lines", [&](OrderId orderId, Quantity qty, Price price, std::string comment) 
    {
        const auto action = *rc::gen::element('A', 'X', 'M');
        char orderIdStr[64] = {};
        auto len = Decoder::convert_unsigned_integer<OrderId>(orderId, orderIdStr);
        const auto side = *rc::gen::element('B', 'S');
        char qtyStr[64] = {};
        len = Decoder::convert_unsigned_integer<Quantity>(qty, qtyStr);
        price = std::abs(price); // only positive prices
        char priceStr[64] = {};
        len = Decoder::convert_float<Price>(priceStr, price, std::numeric_limits<Price>::digits10);

        std::string line;
        
        auto test_parse = [&]() 
        {
            Parser parser;
            bool ret = parser.parse(line, verbose);
            RC_ASSERT(ret == false);
        };
        
        line =  spaces(10) + ',';
        test_parse();
        line =  spaces(10) + ',' +
                spaces(10) + ',';
        test_parse();
        line =  spaces(10) + action + spaces(10);
        test_parse();
        line =  spaces(10) + action + spaces(10) + ',';
        test_parse();
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + ',';
        test_parse();
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10);
        test_parse();
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',';
        test_parse();
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + ',';
        test_parse();
        
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10);
        test_parse();
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',';
        test_parse();
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + ',';
        test_parse();
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10);
        test_parse();
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',';
        test_parse();
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',' +
                spaces(10);
        test_parse();
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',' +
                spaces(10) + ',';
        test_parse();
    });
    
    rc::check("Parse wrong order lines", [&](OrderId orderId, Quantity qty, Price price, std::string comment) 
    {
        const auto action = *rc::gen::element('A', 'X', 'M');
        char orderIdStr[64] = {};
        auto len = Decoder::convert_unsigned_integer<OrderId>(orderId, orderIdStr);
        const auto side = *rc::gen::element('B', 'S');
        char qtyStr[64] = {};
        len = Decoder::convert_unsigned_integer<Quantity>(qty, qtyStr);
        price = std::abs(price); // only positive prices
        char priceStr[64] = {};
        len = Decoder::convert_float<Price>(priceStr, price, std::numeric_limits<Price>::digits10);

        std::string line;
        
        auto test_parse = [&]() 
        {
            Parser parser;
            bool ret = parser.parse(line, verbose);
            RC_ASSERT(ret == false);
        };
        
        auto bad_character = *rc::gen::arbitrary<char>();
        if ('A' == bad_character || 'X' == bad_character || 'M' == bad_character ||
            ' ' == bad_character || 0 == bad_character ||  '/' == bad_character) 
            ++bad_character;
// bad_character = ',';
        line =  spaces(10) + bad_character + spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',' +
                spaces(10) + priceStr;
        test_parse();
        line =  spaces(10) + action + spaces(10) + bad_character + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',' +
                spaces(10) + priceStr;
        test_parse();
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + bad_character + spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',' +
                spaces(10) + priceStr;
        test_parse();
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + bad_character + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',' +
                spaces(10) + priceStr;
        test_parse();
        if (!std::isdigit(bad_character))
        {
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + bad_character + spaces(10) + orderIdStr + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + qtyStr + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse();
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + orderIdStr + spaces(10) + bad_character + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + qtyStr + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse();            
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + orderIdStr + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + bad_character + spaces(10) + qtyStr + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse();
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + orderIdStr + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + qtyStr + spaces(10) + bad_character + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse();
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + orderIdStr + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + qtyStr + spaces(10) + ',' +
                    spaces(10) + bad_character + spaces(10) + priceStr;
            test_parse();
        }
        else
        {
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + bad_character + ' ' + spaces(10) + orderIdStr + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + qtyStr + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse();
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + orderIdStr + spaces(10) + ' ' + bad_character + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + qtyStr + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse();
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + orderIdStr + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + bad_character + ' ' + spaces(10) + qtyStr + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse();
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + orderIdStr + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + qtyStr + spaces(10) + ' ' + bad_character + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse();
        }
    });

    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Parse good trade lines", [&](Quantity qty, Price price, std::string comment) 
    {
        const auto action = 'T';
        char qtyStr[64] = {};
        auto len = Decoder::convert_unsigned_integer<Quantity>(qty, qtyStr);
        price = std::abs(price); // only positive prices
        char priceStr[64] = {};
        len = Decoder::convert_float<Price>(priceStr, price, std::numeric_limits<Price>::digits10);
        
        std::string line =  spaces(10) + action + spaces(10) + ',' +
                            spaces(10) + qtyStr + spaces(10) + ',' +
                            spaces(10) + priceStr + spaces(10);
        RC_LOG() << std::fixed << std::setprecision(std::numeric_limits<Price>::digits10)
            << "line [" << line << ']' << std::endl;
        
        auto test_parse_stl = [&]() 
        {
            high_resolution_clock::time_point start = high_resolution_clock::now();
            auto pos = line.find_first_of("T");
            auto _action = line[pos];
            pos = line.find(',', pos+1);
            auto pos2 = line.find(',', ++pos);
            auto _qty = std::stoul(line.substr(pos, pos2-pos));
            pos = line.find(',', pos+1);
            pos2 = line.find(',', ++pos);
            auto _price = std::stod(line.substr(pos, pos2-pos));
            high_resolution_clock::time_point end = high_resolution_clock::now();
            time_span2 += duration_cast<nanoseconds>(end - start).count();
            
            RC_ASSERT(action == _action);
            RC_ASSERT(qty == _qty);
            RC_ASSERT(price - _price < std::pow(10, -std::numeric_limits<Price>::digits10));
        };
        
        auto test_parse = [&]() 
        {
            Parser parser;
            high_resolution_clock::time_point start = high_resolution_clock::now();
            bool ret = parser.parse(line, verbose);
            high_resolution_clock::time_point end = high_resolution_clock::now();
            time_span1 += duration_cast<nanoseconds>(end - start).count();
            RC_ASSERT(action == parser.getAction());
            RC_ASSERT(qty == parser.getQty());
            RC_ASSERT(price - parser.getPrice() < std::pow(10, -std::numeric_limits<Price>::digits10));
            RC_ASSERT(ret == true);
        };
        
        test_parse();
        test_parse_stl();
        line += ("//" + comment);
        test_parse();
        test_parse_stl();
        nbTests += 2;
    });
    if (nbTests)
    {
        std::cout << "Parse good trade lines perfs  [" << time_span1/nbTests << "] naive stl impl [" 
            << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    return 0;
}

