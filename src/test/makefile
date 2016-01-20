ifneq   "$(gccversion)" ""
    GCC=$(gccversion)
else 
    GCC=g++-5
endif

.PHONY: all clean test

SOURCES:=$(shell ls *.cpp)
TARGETS:=$(SOURCES:.cpp=.out)

INCS=-I../lib -I../bin -I../../tools/rapidcheck/include

CFLAGS=-O2 -m64 -fPIC -g -ggdb -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function
CFLAGS += -std=c++1y -D_REENTRANT -DLinux 

ifneq "" "$(findstring $(sanitize), 1 on On ON)"
    CFLAGS += -fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=signed-integer-overflow -fsanitize=shift -fsanitize=integer-divide-by-zero -fsanitize=null 
endif

LIBS = -L ../bin -lFeedHandler -L ../lib -lTools -L ../../tools/rapidcheck/build -lrapidcheck -latomic -lz -lnsl -lpthread -ldl -lrt

all: $(TARGETS)

clean:
	rm -f *.out

test: clean all
	@for target in $(TARGETS); do  \
	    ./$$target;    \
	done 

%.out : %.cpp
	$(GCC) -fbuiltin -Wall -Werror -o $@ $< $(INCS) $(CFLAGS) $(LIBS)

