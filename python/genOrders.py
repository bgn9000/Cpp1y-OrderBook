#! /usr/bin/python
from __future__ import print_function
from random import *
import sys
sys.path.append('../tools/OrderBook/orderbook')
from orderbook import OrderBook

#print("CROSS", cross_buy, file=sys.stderr)

def generate_new_buy(trade_id):
    return {'type' : 'limit', 
           'side' : 'bid', 
           'quantity' : randint(1,1000),
           'price' : randint(900,1050),
           'trade_id' : trade_id}
           
def generate_cross_buy(trade_id):
    return {'type' : 'limit', 
            'side' : 'bid', 
            'quantity' : randint(1,1000), 
            'price' : randint(1055,1200),
            'trade_id' : trade_id}

def generate_new_sell(trade_id):
    return {'type' : 'limit', 
            'side' : 'ask', 
            'quantity' : randint(1,1000), 
            'price' : randint(1055,1200),
            'trade_id' : trade_id}
            
def generate_cross_sell(trade_id):
    return {'type' : 'limit', 
            'side' : 'ask', 
            'quantity' : randint(1,1000),
            'price' : randint(900,1050),
            'trade_id' : trade_id}

def prefill(nb_orders_prefilled, verbose = False):
    for trade_id in range(nb_orders_prefilled):
        trades,order = order_book.process_order(generate_new_buy(trade_id), False, verbose)
        buys.append(order)
        print('A', order['order_id'], 'B', order['quantity'], order['price'], sep=',', file=sys.stderr)
        trades,order = order_book.process_order(generate_new_sell(trade_id), False, verbose)
        sells.append(order)
        print('A', order['order_id'], 'S', order['quantity'], order['price'], sep=',', file=sys.stderr)

def tests_cases(nb_tests_cases, action, side):
    for trade_id in xrange(3, 3+nb_tests_cases):
        if action == 'A':
            if side == 'B':
                trades,order = order_book.process_order(generate_new_buy(trade_id), False, False)
                buys.append(order)
                print('A', order['order_id'], 'B', order['quantity'], order['price'], sep=',', file=sys.stderr)
            else:
                trades,order = order_book.process_order(generate_new_sell(trade_id), False, False)
                sells.append(order)
                print('A', order['order_id'], 'S', order['quantity'], order['price'], sep=',', file=sys.stderr)
        elif action == 'M':
            if side == 'B':
                i = randrange(len(buys))
                order = buys[i]
                order['quantity'] = randint(1,1000)
                order_book.modify_order(order['order_id'], order)
                print('M', order['order_id'], 'B', order['quantity'], order['price'], sep=',', file=sys.stderr)
            else:
                i = randrange(len(sells))
                order = sells[i]
                order['quantity'] = randint(1,1000)
                order_book.modify_order(order['order_id'], order)
                print('M', order['order_id'], 'B', order['quantity'], order['price'], sep=',', file=sys.stderr)
        elif action == 'X':
            if side == 'B':
                i = randrange(len(buys))
                order = buys[i]
                del buys[i]
                order_book.cancel_order('bid', order['order_id'])
                print('X', order['order_id'], 'B', order['quantity'], order['price'], sep=',', file=sys.stderr)
            else:
                i = randrange(len(sells))
                order = sells[i]
                del sells[i]
                order_book.cancel_order('ask', order['order_id'])
                print('X', order['order_id'], 'S', order['quantity'], order['price'], sep=',', file=sys.stderr)
        # C is a cross order to get executions
        elif action == 'C': 
            if side == 'B':
                trades,order = order_book.process_order(generate_cross_buy(trade_id), False, True)
                for j in range(len(trades)):
                    trade = trades[j]
                    print('T', trade['quantity'], trade['price'], sep=',', file=sys.stderr)
                print("TRADES", trades, file=sys.stderr)
                print(order, file=sys.stderr)
            else:
                trades,order = order_book.process_order(generate_cross_sell(trade_id), False, True)
                for j in range(len(trades)):
                    trade = trades[j]
                    print('T', trade['quantity'], trade['price'], sep=',', file=sys.stderr)
                print("TRADES", trades, file=sys.stderr)
                print(order, file=sys.stderr)


buys = []
sells = []
order_book = OrderBook()

prefill(3)

#choice('AMXC') : A (new), M (modify), X (cancel), C (cross)
tests_cases(1, choice('C'), choice('BS'))

print(order_book)

# quit()


