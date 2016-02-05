ifneq   "$(gccversion)" ""
    GCC=$(gccversion)
else 
    GCC=g++-5
endif

.PHONY: all clean

TARGET:=libTools.a

SOURCES:=$(shell ls *.cpp)
OBJ_DIR=obj
OBJECTS:=$(addprefix $(OBJ_DIR)/, $(SOURCES:.cpp=.o))

INCS=-I.

ifneq "" "$(GCC_MAXSPEED_OPTIMIZATION)"
    CFLAGS_OPTIM := $(GCC_MAXSPEED_OPTIMIZATION)
else
    CFLAGS_OPTIM := -O2
endif
CFLAGS=$(CFLAGS_OPTIM) -m64 -fPIC -g -ggdb
CFLAGS += -std=c++14 -D_REENTRANT -DLinux -pedantic-errors -Wconversion -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function

ifneq "" "$(findstring $(sanitize), 1 on On ON)"
    CFLAGS += -fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=signed-integer-overflow -fsanitize=shift -fsanitize=integer-divide-by-zero -fsanitize=null 
endif

all: $(TARGET)

$(TARGET):$(OBJECTS)
	ar -r $@ $(OBJECTS)

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(TARGET)

$(OBJ_DIR)/%.o : %.cpp %.h Common.h
	@mkdir -p $(OBJ_DIR)
	$(GCC) -fbuiltin -Wall -Werror -c -o $@ $< $(INCS) $(CFLAGS)

