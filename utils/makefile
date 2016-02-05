DIRS:=lib bin

.PHONY: all clean test

all:
	@for dir in $(DIRS); do  \
	    make -C $$dir || exit 1;    \
	done

clean:
	@for dir in $(DIRS); do  \
	    make -C $$dir clean || exit 1;    \
	done

test: clean all
	make -C test test || exit 1;
