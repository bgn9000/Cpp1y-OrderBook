#! /usr/bin/python
from __future__ import print_function
from random import *
import sys
sys.path.append('ext/OrderBook/orderbook')
from orderbook import OrderBook

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

def treat_trades(trades, side):
    if not trades:
        return
    print("TRADES", trades)
    for j in range(len(trades)):
        trade = trades[j]
        print('T', trade['quantity'], trade['price'], sep=',', file=sys.stderr)
    if side == 'B':
        for j in range(len(trades)):
            trade = trades[j]
            assert (trade['party1'][1] == 'ask'), 'party1 should be ask'
            new_quantity = None
            orderId = trade['party1'][2]
            assert (orderId != None), 'party1 should have order_id'
            new_quantity = trade['party1'][3]
            order = sells[orderId]
            if new_quantity == None:
                del sells[orderId]
                print('X', order['order_id'], 'S', order['quantity'], order['price'], sep=',', file=sys.stderr)
            else:
                order['quantity'] = new_quantity
                print('M', order['order_id'], 'S', order['quantity'], order['price'], sep=',', file=sys.stderr)
    else:
        for j in range(len(trades)):
            trade = trades[j]
            assert (trade['party1'][1] == 'bid'), 'party1 should be bid'
            new_quantity = None
            orderId = trade['party1'][2]
            assert (orderId != None), 'party1 should have order_id'
            new_quantity = trade['party1'][3]
            order = buys[orderId]
            if new_quantity == None:
                del buys[orderId]
                print('X', order['order_id'], 'B', order['quantity'], order['price'], sep=',', file=sys.stderr)
            else:
                order['quantity'] = new_quantity
                print('M', order['order_id'], 'B', order['quantity'], order['price'], sep=',', file=sys.stderr)

def prefill(nb_orders_prefilled, verbose = False):
    for trade_id in range(nb_orders_prefilled):
        trades,neworder = order_book.process_order(generate_new_buy(trade_id), False, verbose)
        assert (trades == []), 'No trade at this stage'
        assert (neworder != []), 'Expect new order'
        buys[neworder['order_id']]=neworder
        print('A', neworder['order_id'], 'B', neworder['quantity'], neworder['price'], sep=',', file=sys.stderr)
        trades,neworder = order_book.process_order(generate_new_sell(trade_id), False, verbose)
        assert (trades == []), 'No trade at this stage'
        assert (neworder != []), 'Expect new order'
        sells[neworder['order_id']]=neworder
        print('A', neworder['order_id'], 'S', neworder['quantity'], neworder['price'], sep=',', file=sys.stderr)

def testCases(trade_id, action, side, verbose = False):
    if action == 'A':
        if side == 'B':
            trades,neworder = order_book.process_order(generate_new_buy(trade_id), False, verbose)
            treat_trades(trades, side)
            if neworder:
                buys[neworder['order_id']]=neworder
                print('A', neworder['order_id'], 'B', neworder['quantity'], neworder['price'], sep=',', file=sys.stderr)
        else:
            trades,neworder = order_book.process_order(generate_new_sell(trade_id), False, verbose)
            treat_trades(trades, side)
            if neworder:
                sells[neworder['order_id']]=neworder
                print('A', neworder['order_id'], 'S', neworder['quantity'], neworder['price'], sep=',', file=sys.stderr)
    elif action == 'M':
        if side == 'B':
            if len(buys):
                key = choice(buys.keys())
                order = buys[key]
                order['quantity'] = randint(1,1000)
                order_book.modify_order(key, order)
                print('M', order['order_id'], 'B', order['quantity'], order['price'], sep=',', file=sys.stderr)
        else:
            if len(sells):
                key = choice(sells.keys())
                order = sells[key]
                order['quantity'] = randint(1,1000)
                order_book.modify_order(key, order)
                print('M', order['order_id'], 'S', order['quantity'], order['price'], sep=',', file=sys.stderr)
    elif action == 'X':
        if side == 'B':
            if len(buys):
                key = choice(buys.keys())
                order = buys[key]
                del buys[key]
                order_book.cancel_order('bid', key)
                print('X', order['order_id'], 'B', order['quantity'], order['price'], sep=',', file=sys.stderr)
        else:
            if len(sells):
                key = choice(sells.keys())
                order = sells[key]
                del sells[key]
                order_book.cancel_order('ask', key)
                print('X', order['order_id'], 'S', order['quantity'], order['price'], sep=',', file=sys.stderr)
    elif action == 'C': 
        if side == 'B':
            trades,neworder = order_book.process_order(generate_cross_buy(trade_id), False, verbose)
            treat_trades(trades, side)
            if neworder:
                buys[neworder['order_id']]=neworder
                print('A', neworder['order_id'], 'B', neworder['quantity'], neworder['price'], sep=',', file=sys.stderr)
        else:
            trades,neworder = order_book.process_order(generate_cross_sell(trade_id), False, verbose)
            treat_trades(trades, side)
            if neworder:
                sells[neworder['order_id']]=neworder
                print('A', neworder['order_id'], 'S', neworder['quantity'], neworder['price'], sep=',', file=sys.stderr)


buys = {}
sells = {}
order_book = OrderBook()

# nb buys and sells to prefill orderbook
prefill(1000)

#print(order_book)


#usage: 
# 1/ trade_id
# 2/ choice('AMXC') : A (new), M (modify), X (cancel), C (cross)
# 3/ choice('BS') : B (buy), S (sell)
for trade_id in xrange(1000, 500000):
    testCases(trade_id, choice('AXC'), choice('BS'), False)

print(order_book)

# quit()


