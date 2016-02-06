#! /bin/bash
valgrind -v --tool=memcheck --leak-check=full --log-file=valgrind.log $*
