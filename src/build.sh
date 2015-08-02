#!/bin/bash

# -DNDEBUG disables all assert checking
gcc firehelix.c heavy/*.c tinyosc/*.c -std=gnu99 -Werror -Os -DNDEBUG -lm -lrt -o ixhelfire
