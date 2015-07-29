#!/bin/bash

gcc firehelix.c heavy/c/*.c tinyosc/*.c -std=gnu99 -Werror -O3 -lm -lrt -o ixhelfire
