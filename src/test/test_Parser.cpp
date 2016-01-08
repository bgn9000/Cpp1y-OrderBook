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
    std::cout << "Verbose is " << verbose << " : default is 0, param '-v 1 or higher' to activate it" << std::endl;
    
    rc::check("nbChar for OrderId, Qty and Price", []() 
    {
        RC_ASSERT(nbCharOfOrderId == 9);
        RC_ASSERT(nbCharOfOrderQty == 6);
        RC_ASSERT(nbCharOfOrderPrice == 9);
    });
    
    auto spaces = [](int n) -> std::string
    {
        return std::string(*rc::gen::inRange(0, n), ' ');
    };
    
    auto time_span1 = 0ULL, time_span2 = 0ULL;
    auto nbTests = 0U;
    rc::check("Skip comment lines", [&](std::string comment) 
    {
        std::string line;
        {
            line = spaces(10) + "//" + spaces(10) + comment;
            Errors errors;
            high_resolution_clock::time_point start = high_resolution_clock::now();
            Parser parser;
            auto ret = parser.parse(line.c_str(), line.length(), errors, verbose);
            high_resolution_clock::time_point end = high_resolution_clock::now();
            time_span1 += duration_cast<nanoseconds>(end - start).count();
            
            ++nbTests;
            
            RC_LOG() << "line [" << line << ']' << std::endl;
            RC_ASSERT(false == ret);
            RC_ASSERT(1ULL == errors.commentedLines);
            RC_ASSERT(errors.nbErrors() == 0ULL);
        }
        {
            line = spaces(10) + "/ " + spaces(10) + comment;
            Errors errors;
            Parser parser;
            auto ret = parser.parse(line.c_str(), line.length(), errors, verbose);
            RC_LOG() << "line [" << line << ']' << std::endl;
            RC_ASSERT(false == ret);
            RC_ASSERT(1ULL == errors.corruptedMessages);
            RC_ASSERT(0ULL == errors.commentedLines);
            RC_ASSERT(errors.nbErrors() == 1ULL);
        }
    });
    if (nbTests)
    {
        std::cout << "Skip comment lines perfs  [" << time_span1/nbTests << "] (in ns)" << std::endl;
    }

    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Parse good order lines", [&](std::string comment) 
    {
        const auto action = *rc::gen::element('A', 'X', 'M');
        const auto orderId = *rc::gen::inRange<OrderId>(1, maxOrderId);
        char orderIdStr[64] = {};
        auto len = Decoder::convert_unsigned_integer<OrderId>(orderId, orderIdStr);
        const auto side = *rc::gen::element('B', 'S');
        const auto qty = *rc::gen::inRange<Quantity>(1, maxOrderQty);
        char qtyStr[64] = {};
        len = Decoder::convert_unsigned_integer<Quantity>(qty, qtyStr);
        /*Not working well to provide 100 tests case :
        const auto price = *rc::gen::suchThat<Price>([](Price price) 
        {
            return (price > 0 && price <= maxOrderPrice);
        });*/
        const auto price = static_cast<Price>(*rc::gen::inRange(1, 99)) / *rc::gen::inRange(100, 10'000) + *rc::gen::inRange(0, maxOrderPrice);
        char priceStr[64] = {};
        len = Decoder::convert_unsigned_float<Price>(priceStr, price, std::numeric_limits<Price>::digits10);
        
        std::string line =  spaces(10) + action + spaces(10) + ',' +
                            spaces(10) + orderIdStr + spaces(10) + ',' +
                            spaces(10) + side + spaces(10) + ',' +
                            spaces(10) + qtyStr + spaces(10) + ',' +
                            spaces(10) + priceStr + spaces(10);
        RC_LOG() << "line [" << line << ']' << std::endl;
        
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
            RC_ASSERT(price - _price < std::pow(10, -nbCharOfPricePrecision+1));
        };
        
        auto test_parse = [&]() 
        {
            Errors errors;
            Parser parser;
            high_resolution_clock::time_point start = high_resolution_clock::now();
            bool ret = parser.parse(line.c_str(), line.length(), errors, verbose);
            high_resolution_clock::time_point end = high_resolution_clock::now();
            time_span1 += duration_cast<nanoseconds>(end - start).count();
            
            RC_ASSERT(parser.getAction() == action);
            RC_ASSERT(parser.getOrderId() == orderId);
            RC_ASSERT(parser.getSide() == side);
            RC_ASSERT(parser.getQty() == qty);
            RC_ASSERT(price - parser.getPrice() < std::pow(10, -nbCharOfPricePrecision+1));
            RC_ASSERT(true == ret);
            RC_ASSERT(errors.nbErrors() == 0ULL);
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
    
    rc::check("Parse order lines with 0 in orderId or quantity or price", [&](std::string comment) 
    {
        const auto action = *rc::gen::element('A', 'X', 'M');
        const auto orderId = *rc::gen::inRange<OrderId>(1, maxOrderId);
        char orderIdStr[64] = {};
        auto len = Decoder::convert_unsigned_integer<OrderId>(orderId, orderIdStr);
        const auto side = *rc::gen::element('B', 'S');
        const auto qty = *rc::gen::inRange<Quantity>(1, maxOrderQty);
        char qtyStr[64] = {};
        len = Decoder::convert_unsigned_integer<Quantity>(qty, qtyStr);
        const auto price = static_cast<Price>(*rc::gen::inRange(1, 99)) / *rc::gen::inRange(100, 10'000) + *rc::gen::inRange(0, maxOrderPrice);
        char priceStr[64] = {};
        len = Decoder::convert_unsigned_float<Price>(priceStr, price, nbCharOfPricePrecision);
        
        std::string line =  spaces(10) + action + spaces(10) + ',' +
                            spaces(10) + '0' + spaces(10) + ',' +
                            spaces(10) + side + spaces(10) + ',' +
                            spaces(10) + qtyStr + spaces(10) + ',' +
                            spaces(10) + priceStr + spaces(10) + "//" + comment;
        {
            Errors errors;
            Parser parser;
            bool ret = parser.parse(line.c_str(), line.length(), errors, verbose);
            RC_LOG() << "line [" << line << ']' << std::endl;
            RC_ASSERT(0U == parser.getOrderId());
            RC_ASSERT(false == ret);
            RC_ASSERT(errors.nbErrors() == 1ULL);
            RC_ASSERT(1ULL == errors.zeroOrderIds);
        }
        
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + '0' + spaces(10) + ',' +
                spaces(10) + priceStr + spaces(10) + "//" + comment;
        {
            Errors errors;
            Parser parser;
            bool ret = parser.parse(line.c_str(), line.length(), errors, verbose);
            RC_LOG() << "line [" << line << ']' << std::endl;                     
            RC_ASSERT(orderId == parser.getOrderId());
            RC_ASSERT(side == parser.getSide());
            RC_ASSERT(0U == parser.getQty());
            RC_ASSERT(false == ret);
            RC_ASSERT(errors.nbErrors() == 1ULL);
            RC_ASSERT(1ULL == errors.zeroQuantities);
        }
        
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',' +
                spaces(10) + '0' + spaces(10) + "//" + comment;
        {
            Errors errors;
            Parser parser;
            bool ret = parser.parse(line.c_str(), line.length(), errors, verbose);
            RC_LOG() << "line [" << line << ']' << std::endl;
            RC_ASSERT(orderId == parser.getOrderId());
            RC_ASSERT(side == parser.getSide());
            RC_ASSERT(qty == parser.getQty());
            RC_ASSERT(0.0 == parser.getPrice());
            RC_ASSERT(false == ret);
            RC_ASSERT(errors.nbErrors() == 1ULL);
            RC_ASSERT(1ULL == errors.zeroPrices);
        }
    });
    
    rc::check("Parse order lines with negative orderId or quantity or price", [&](std::string comment) 
    {
        const auto action = *rc::gen::element('A', 'X', 'M');
        const auto orderId = *rc::gen::inRange<OrderId>(1, maxOrderId);
        char orderIdStr[64] = {};
        auto len = Decoder::convert_unsigned_integer<OrderId>(orderId, orderIdStr);
        const auto side = *rc::gen::element('B', 'S');
        const auto qty = *rc::gen::inRange<Quantity>(1, maxOrderQty);
        char qtyStr[64] = {};
        len = Decoder::convert_unsigned_integer<Quantity>(qty, qtyStr);
        const auto price = static_cast<Price>(*rc::gen::inRange(1, 99)) / *rc::gen::inRange(100, 10'000) + *rc::gen::inRange(0, maxOrderPrice);
        char priceStr[64] = {};
        len = Decoder::convert_unsigned_float<Price>(priceStr, price, nbCharOfPricePrecision);
        
        std::string line =  spaces(10) + action + spaces(10) + ',' +
                            spaces(10) + '-' + orderIdStr + spaces(10) + ',' +
                            spaces(10) + side + spaces(10) + ',' +
                            spaces(10) + qtyStr + spaces(10) + ',' +
                            spaces(10) + priceStr + spaces(10) + "//" + comment;
        {
            Errors errors;
            Parser parser;
            bool ret = parser.parse(line.c_str(), line.length(), errors, verbose);
            RC_LOG() << "line [" << line << ']' << std::endl;
            RC_ASSERT(0U == parser.getOrderId());
            RC_ASSERT(false == ret);
            RC_ASSERT(errors.nbErrors() == 1ULL);
            RC_ASSERT(1ULL == errors.negativeOrderIds);
        }
        
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + '-' + qtyStr + spaces(10) + ',' +
                spaces(10) + priceStr + spaces(10) + "//" + comment;
        {
            Errors errors;
            Parser parser;
            bool ret = parser.parse(line.c_str(), line.length(), errors, verbose);
            RC_LOG() << "line [" << line << ']' << std::endl;                     
            RC_ASSERT(orderId == parser.getOrderId());
            RC_ASSERT(side == parser.getSide());
            RC_ASSERT(0U == parser.getQty());
            RC_ASSERT(false == ret);
            RC_ASSERT(errors.nbErrors() == 1ULL);
            RC_ASSERT(1ULL == errors.negativeQuantities);
        }
        
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',' +
                spaces(10) + '-' + priceStr + spaces(10) + "//" + comment;
        {
            Errors errors;
            Parser parser;
            bool ret = parser.parse(line.c_str(), line.length(), errors, verbose);
            RC_LOG() << "line [" << line << ']' << std::endl;
            RC_ASSERT(orderId == parser.getOrderId());
            RC_ASSERT(side == parser.getSide());
            RC_ASSERT(qty == parser.getQty());
            RC_ASSERT(0.0 == parser.getPrice());
            RC_ASSERT(false == ret);
            RC_ASSERT(errors.nbErrors() == 1ULL);
            RC_ASSERT(1ULL == errors.negativePrices);
        }
    });
    
    rc::check("Parse order lines with out of bounds orderId or quanity or price", [&](std::string comment) 
    {
        const auto action = *rc::gen::element('A', 'X', 'M');
        
        const auto orderId = *rc::gen::inRange<Quantity>(1, maxOrderId);
        char orderIdStr[64] = {};
        auto len = Decoder::convert_unsigned_integer<OrderId>(orderId, orderIdStr);
        
        const auto orderId_over = *rc::gen::suchThat<OrderId>([](OrderId id) 
        {
            return (id > maxOrderId);
        });
        char orderId_overStr[64] = {};
        len = Decoder::convert_unsigned_integer<OrderId>(orderId_over, orderId_overStr);
        
        const auto side = *rc::gen::element('B', 'S');
        
        const auto qty = *rc::gen::inRange<Quantity>(1, maxOrderQty);
        char qtyStr[64] = {};
        len = Decoder::convert_unsigned_integer<Quantity>(qty, qtyStr);
        char qty_overStr[64] = {};
        const auto qty_over = *rc::gen::suchThat<Quantity>([](Quantity qty) 
        {
            return (qty > maxOrderQty);
        });
        len = Decoder::convert_unsigned_integer<Quantity>(qty_over, qty_overStr);
        
        const auto price = static_cast<Price>(*rc::gen::inRange(1, 99)) / *rc::gen::inRange(100, 10'000) + *rc::gen::inRange(0, maxOrderPrice);
        char priceStr[64] = {};
        len = Decoder::convert_unsigned_float<Price>(priceStr, price, nbCharOfPricePrecision);
        
        char price_overStr[64] = {};
        const auto price_over = *rc::gen::suchThat<Price>([](Price price) 
        {
            return (price > maxOrderPrice);
        });
        len = Decoder::convert_unsigned_float<Price>(price_overStr, price_over, nbCharOfPricePrecision);
        
        std::string line =  spaces(10) + action + spaces(10) + ',' +
                            spaces(10) + orderId_overStr + spaces(10) + ',' +
                            spaces(10) + side + spaces(10) + ',' +
                            spaces(10) + qtyStr + spaces(10) + ',' +
                            spaces(10) + priceStr + spaces(10) + "//" + comment;
        {
            Errors errors;
            Parser parser;
            bool ret = parser.parse(line.c_str(), line.length(), errors, verbose);
            RC_LOG() << "line [" << line << ']' << std::endl;
            RC_ASSERT(0U == parser.getOrderId());
            RC_ASSERT(false == ret);
            RC_ASSERT(errors.nbErrors() == 1ULL);
            RC_ASSERT(1ULL == errors.outOfBoundsOrderIds);
        }
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qty_overStr + spaces(10) + ',' +
                spaces(10) + priceStr + spaces(10) + "//" + comment;
        {
            Errors errors;
            Parser parser;
            bool ret = parser.parse(line.c_str(), line.length(), errors, verbose);
            RC_LOG() << "line [" << line << ']' << std::endl;
            RC_ASSERT(orderId == parser.getOrderId());
            RC_ASSERT(0U == parser.getQty());
            RC_ASSERT(false == ret);
            RC_ASSERT(errors.nbErrors() == 1ULL);
            RC_ASSERT(1ULL == errors.outOfBoundsQuantities);
        }
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',' +
                spaces(10) + price_overStr + spaces(10) + "//" + comment;
        {
            Errors errors;
            Parser parser;
            bool ret = parser.parse(line.c_str(), line.length(), errors, verbose);
            RC_LOG() << "line [" << line << ']' << std::endl;
            RC_ASSERT(orderId == parser.getOrderId());
            RC_ASSERT(qty == parser.getQty());
            RC_ASSERT(0.0 == parser.getPrice());
            RC_ASSERT(false == ret);
            RC_ASSERT(errors.nbErrors() == 1ULL);
            RC_ASSERT(1ULL == errors.outOfBoundsPrices);
        }
    });
    
    rc::check("Parse partial order lines", [&](std::string comment) 
    {
        const auto action = *rc::gen::element('A', 'X', 'M');
        const auto orderId = *rc::gen::inRange<OrderId>(1, maxOrderId);
        char orderIdStr[64] = {};
        auto len = Decoder::convert_unsigned_integer<OrderId>(orderId, orderIdStr);
        const auto side = *rc::gen::element('B', 'S');
        const auto qty = *rc::gen::inRange<Quantity>(1, maxOrderQty);
        char qtyStr[64] = {};
        len = Decoder::convert_unsigned_integer<Quantity>(qty, qtyStr);
        const auto price = static_cast<Price>(*rc::gen::inRange(1, 99)) / *rc::gen::inRange(100, 10'000) + *rc::gen::inRange(0, maxOrderPrice);
        char priceStr[64] = {};
        len = Decoder::convert_unsigned_float<Price>(priceStr, price, nbCharOfPricePrecision);

        std::string line;
        
        auto test_parse = [&](Errors& errors, bool isError = true) 
        {
            Parser parser;
            bool ret = parser.parse(line.c_str(), line.length(), errors, verbose);
            RC_LOG() << "line [" << line << ']' << std::endl;
            RC_ASSERT(false == ret);
            if (isError) RC_ASSERT(errors.nbErrors() == 1ULL);
            else RC_ASSERT(errors.nbErrors() == 0ULL);
        };
        
        Errors errors;
        test_parse(errors, false);
        RC_ASSERT(1ULL == errors.blankLines);
        
        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + ',';
        test_parse(errors);
        RC_ASSERT(1ULL == errors.missingActions);
        
        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + ',' + spaces(10) + ',';
        test_parse(errors);
        RC_ASSERT(1ULL == errors.missingActions);
        
        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10);
        test_parse(errors);
        RC_ASSERT(1ULL == errors.IncompleteMessages);

        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10) + ',';
        test_parse(errors);
        RC_ASSERT(1ULL == errors.IncompleteMessages);

        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + ',';
        test_parse(errors);
        RC_ASSERT(1ULL == errors.missingOrderIds);

        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10) + ',' + 
                spaces(10) + orderIdStr + spaces(10);
        test_parse(errors);
        RC_ASSERT(1ULL == errors.IncompleteMessages);

        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',';
        test_parse(errors);
        RC_ASSERT(1ULL == errors.IncompleteMessages);

        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + ',';
        test_parse(errors);
        RC_ASSERT(1ULL == errors.missingSides);

        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10);
        test_parse(errors);
        RC_ASSERT(1ULL == errors.IncompleteMessages);

        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',';
        test_parse(errors);
        RC_ASSERT(1ULL == errors.IncompleteMessages);

        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + ',';
        test_parse(errors);
        RC_ASSERT(1ULL == errors.missingQuantities);

        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10);
        test_parse(errors);
        RC_ASSERT(1ULL == errors.IncompleteMessages);

        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',';
        test_parse(errors);
        RC_ASSERT(1ULL == errors.missingPrices);

        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10) + ',' +
              spaces(10) + orderIdStr + spaces(10) + ',' +
              spaces(10) + side + spaces(10) + ',' +
              spaces(10) + qtyStr + spaces(10) + ',' +
              spaces(10);
        test_parse(errors);
        RC_ASSERT(1ULL == errors.missingPrices);

        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',' +
                spaces(10) + ',';
        test_parse(errors);
        RC_ASSERT(1ULL == errors.missingPrices);
    });
    
    rc::check("Parse wrong order lines", [&](std::string comment) 
    {
        const auto action = *rc::gen::element('A', 'X', 'M');
        const auto orderId = *rc::gen::inRange<OrderId>(1, maxOrderId);
        char orderIdStr[64] = {};
        auto len = Decoder::convert_unsigned_integer<OrderId>(orderId, orderIdStr);
        const auto side = *rc::gen::element('B', 'S');
        const auto qty = *rc::gen::inRange<Quantity>(1, maxOrderQty);
        char qtyStr[64] = {};
        len = Decoder::convert_unsigned_integer<Quantity>(qty, qtyStr);
        const auto price = static_cast<Price>(*rc::gen::inRange(1, 99)) / *rc::gen::inRange(100, 10'000) + *rc::gen::inRange(0, maxOrderPrice);
        char priceStr[64] = {};
        len = Decoder::convert_unsigned_float<Price>(priceStr, price, nbCharOfPricePrecision);

        std::string line;
        
        auto test_parse = [&](Errors& errors) 
        {
            Parser parser;
            bool ret = parser.parse(line.c_str(), line.length(), errors, verbose);
            RC_LOG() << "line [" << line << ']' << std::endl;
            RC_ASSERT(errors.nbErrors() == 1ULL);
            RC_ASSERT(false == ret);
        };
        
        auto bad_character = *rc::gen::arbitrary<char>();
        if ('A' == bad_character || ',' == bad_character || 
            '/' == bad_character || 'S' == bad_character)
            bad_character += 2;
        if ('X' == bad_character || 'M' == bad_character || 
            'T' == bad_character || 'B' == bad_character ||
            ' ' == bad_character || '-' == bad_character ||
            '0' == bad_character || 0 == bad_character) 
            ++bad_character;
        Errors errors;
        line =  spaces(10) + bad_character + spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',' +
                spaces(10) + priceStr;
        test_parse(errors);
        RC_ASSERT(1ULL == errors.wrongActions);
        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10) + bad_character + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',' +
                spaces(10) + priceStr;
        test_parse(errors);
        RC_ASSERT(1ULL == errors.corruptedMessages);
        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + bad_character + spaces(10) + side + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',' +
                spaces(10) + priceStr;
        test_parse(errors);
        RC_ASSERT(1ULL == errors.wrongSides);
        memset((void*)&errors, 0, sizeof(Errors));
        line =  spaces(10) + action + spaces(10) + ',' +
                spaces(10) + orderIdStr + spaces(10) + ',' +
                spaces(10) + side + spaces(10) + bad_character + spaces(10) + ',' +
                spaces(10) + qtyStr + spaces(10) + ',' +
                spaces(10) + priceStr;
        test_parse(errors);
        RC_ASSERT(1ULL == errors.corruptedMessages);
        if (!std::isdigit(bad_character))
        {
            memset((void*)&errors, 0, sizeof(Errors));
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + bad_character + spaces(10) + orderIdStr + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + qtyStr + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse(errors);
            RC_ASSERT(1ULL == errors.corruptedMessages);
            memset((void*)&errors, 0, sizeof(Errors));
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + orderIdStr + spaces(10) + bad_character + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + qtyStr + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse(errors);
            RC_ASSERT(1ULL == errors.corruptedMessages);
            memset((void*)&errors, 0, sizeof(Errors));
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + orderIdStr + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + bad_character + spaces(10) + qtyStr + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse(errors);
            RC_ASSERT(1ULL == errors.corruptedMessages);
            memset((void*)&errors, 0, sizeof(Errors));
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + orderIdStr + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + qtyStr + spaces(10) + bad_character + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse(errors);
            RC_ASSERT(1ULL == errors.corruptedMessages);
            memset((void*)&errors, 0, sizeof(Errors));
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + orderIdStr + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + qtyStr + spaces(10) + ',' +
                    spaces(10) + bad_character + spaces(10) + priceStr;
            test_parse(errors);
            RC_ASSERT(1ULL == errors.corruptedMessages);
        }
        else
        {
            memset((void*)&errors, 0, sizeof(Errors));
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + bad_character + ' ' + spaces(10) + orderIdStr + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + qtyStr + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse(errors);
            RC_ASSERT(1ULL == errors.corruptedMessages);
            memset((void*)&errors, 0, sizeof(Errors));
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + orderIdStr + spaces(10) + ' ' + bad_character + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + qtyStr + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse(errors);
            RC_ASSERT(1ULL == errors.corruptedMessages);
            memset((void*)&errors, 0, sizeof(Errors));
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + orderIdStr + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + bad_character + ' ' + spaces(10) + qtyStr + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse(errors);
            RC_ASSERT(1ULL == errors.corruptedMessages);
            memset((void*)&errors, 0, sizeof(Errors));
            line =  spaces(10) + action + spaces(10) + ',' +
                    spaces(10) + orderIdStr + spaces(10) + ',' +
                    spaces(10) + side + spaces(10) + ',' +
                    spaces(10) + qtyStr + spaces(10) + ' ' + bad_character + spaces(10) + ',' +
                    spaces(10) + priceStr;
            test_parse(errors);
            RC_ASSERT(1ULL == errors.corruptedMessages);
        }
    });

    time_span1 = time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Parse good trade lines", [&](std::string comment) 
    {
        const auto action = 'T';
        const auto qty = *rc::gen::inRange<Quantity>(1, maxOrderQty);
        char qtyStr[64] = {};
        auto len = Decoder::convert_unsigned_integer<Quantity>(qty, qtyStr);
        const auto price = static_cast<Price>(*rc::gen::inRange(1, 99)) / *rc::gen::inRange(100, 10'000) + *rc::gen::inRange(0, maxOrderPrice);
        char priceStr[64] = {};
        len = Decoder::convert_unsigned_float<Price>(priceStr, price, nbCharOfPricePrecision);
        
        std::string line =  spaces(10) + action + spaces(10) + ',' +
                            spaces(10) + qtyStr + spaces(10) + ',' +
                            spaces(10) + priceStr + spaces(10);
        
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
            RC_LOG() << "line [" << line << ']' << std::endl;
            RC_ASSERT(action == _action);
            RC_ASSERT(qty == _qty);
            RC_ASSERT(price - _price < std::pow(10, -nbCharOfPricePrecision+1));
        };
        
        auto test_parse = [&]() 
        {
            Errors errors;
            Parser parser;
            high_resolution_clock::time_point start = high_resolution_clock::now();
            bool ret = parser.parse(line.c_str(), line.length(), errors, verbose);
            high_resolution_clock::time_point end = high_resolution_clock::now();
            time_span1 += duration_cast<nanoseconds>(end - start).count();
            RC_LOG() << "line [" << line << ']' << std::endl;
            RC_ASSERT(action == parser.getAction());
            RC_ASSERT(qty == parser.getQty());
            RC_ASSERT(price - parser.getPrice() < std::pow(10, -nbCharOfPricePrecision+1));
            RC_ASSERT(true == ret);
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

