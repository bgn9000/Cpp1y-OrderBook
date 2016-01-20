ifneq   "$(gccversion)" ""
    GCC=$(gccversion)
else 
    GCC=g++-5
endif

.PHONY: all clean

TARGET:=FeedHandler
LIB:=lib$(TARGET).a
OBJ_DIR=obj

INCS=-I. -I../lib

ifneq "" "$(GCC_MAXSPEED_OPTIMIZATION)"
    CFLAGS_OPTIM := $(GCC_MAXSPEED_OPTIMIZATION)
else
    CFLAGS_OPTIM := -O2
endif
CFLAGS=$(CFLAGS_OPTIM) -m64 -fPIC -g -ggdb
CFLAGS += -std=c++1y -D_REENTRANT -DLinux -pedantic-errors -Wconversion -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function

ifneq "" "$(findstring $(sanitize), 1 on On ON)"
    CFLAGS += -fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=signed-integer-overflow -fsanitize=shift -fsanitize=integer-divide-by-zero -fsanitize=null 
endif

LIBS = $(LIB) -L ../lib -lTools -latomic -lz -lnsl -lpthread -ldl -lrt

all: $(TARGET).out

$(TARGET).out: ../lib/libTools.a $(LIB) $(OBJ_DIR)/main.o
	g++ -Wall -o $@ $(OBJ_DIR)/main.o $(CFLAGS) $(LIBS)

clean:
	rm -rf $(OBJ_DIR)/*.o
	rm -f $(TARGET).out
	rm -f lib$(TARGET).a
	
$(LIB): $(OBJ_DIR)/FeedHandler.o $(OBJ_DIR)/Reporter.o
	ar -r $@ $(OBJ_DIR)/FeedHandler.o $(OBJ_DIR)/Reporter.o

$(OBJ_DIR)/%.o : %.cpp
	@mkdir -p $(OBJ_DIR)
	$(GCC) -fbuiltin -Wall -Werror -c -o $@ $< $(INCS) $(CFLAGS)

