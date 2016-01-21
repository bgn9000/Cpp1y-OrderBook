ifneq   "$(gccversion)" ""
    GCC=$(gccversion)
else 
    GCC=g++-5
endif

.PHONY: all clean test

SOURCES:=$(shell ls *.cpp)
TARGETS:=$(SOURCES:.cpp=.out)

INCS=-I../lib -I../bin -I../../tools/rapidcheck-stable_20160119/include

ifneq "" "$(GCC_MAXSPEED_OPTIMIZATION)"
    CFLAGS_OPTIM := $(GCC_MAXSPEED_OPTIMIZATION)
else
    CFLAGS_OPTIM := -O2
endif
CFLAGS=$(CFLAGS_OPTIM) -m64 -fPIC -g -ggdb
CFLAGS += -std=c++1y -D_REENTRANT -DLinux -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function

ifneq "" "$(findstring $(sanitize), 1 on On ON)"
    CFLAGS += -fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=signed-integer-overflow -fsanitize=shift -fsanitize=integer-divide-by-zero -fsanitize=null 
endif

LIBS = -L ../bin -lFeedHandler -L ../lib -lTools -L ../../tools/rapidcheck-stable_20160119/build -lrapidcheck -latomic -lz -lnsl -lpthread -ldl -lrt

all: $(TARGETS)

clean:
	rm -f *.out

test: clean all
	@for target in $(TARGETS); do  \
	    ./$$target;    \
	done 

%.out : %.cpp
	$(GCC) -fbuiltin -Wall -Werror -o $@ $< $(INCS) $(CFLAGS) $(LIBS)

