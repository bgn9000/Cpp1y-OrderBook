ifneq   "$(gccversion)" ""
    GCC=$(gccversion)
else 
    GCC=g++-5
endif

.PHONY: all clean test

TARGET:=FeedHandler
LIB:=lib$(TARGET).a
OBJ_DIR=obj

INCS=-I. -I../lib

CFLAGS=-O2 -m64 -fPIC -g -ggdb
CFLAGS += -std=c++1y -D_REENTRANT -DLinux -pedantic-errors -Wconversion -Wno-unused-but-set-variable -Wno-unused-variable

ifneq "" "$(findstring $(sanitize), 1 on On ON)"
    CFLAGS += -fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=signed-integer-overflow -fsanitize=shift -fsanitize=integer-divide-by-zero -fsanitize=null 
endif

LIBS = $(LIB) -L ../lib -lTools -latomic -lz -lnsl -lpthread -ldl -lrt

test: all
all: $(TARGET).out

$(TARGET).out: $(OBJ_DIR)/main.o $(LIB) ../lib/libTools.a
	g++ -Wall -o $@ $(OBJ_DIR)/main.o $(CFLAGS) $(LIBS)

clean:
	rm -rf $(OBJ_DIR)/*.o
	rm -f $(TARGET).out
	rm -f lib$(TARGET).a
	
$(LIB): $(OBJ_DIR)/$(TARGET).o
	ar -r $@ $<

$(OBJ_DIR)/%.o : %.cpp %.h
	@mkdir -p $(OBJ_DIR)
	$(GCC) -fbuiltin -Wall -Werror -c -o $@ $< $(INCS) $(CFLAGS)

