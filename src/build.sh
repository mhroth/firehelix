#!/bin/bash

# -DNDEBUG disables all assert checking
gcc firehelix.c heavy/c/*.c tinyosc/*.c -std=gnu99 -Werror -O3 -DNDEBUG -lm -lrt -o ixhelfire
