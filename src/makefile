DIRS:=lib bin test

.PHONY: all clean test

all:
	@for dir in $(DIRS); do  \
	    make -C $$dir || exit 1;    \
	done

clean:
	@for dir in $(DIRS); do  \
	    make -C $$dir clean || exit 1;    \
	done

test:
	@for dir in $(DIRS); do  \
	    make -C $$dir test || exit 1;    \
	done
