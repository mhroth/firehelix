#!/bin/bash

gcc firehelix.c heavy/*.c -std=gnu99 -Werror -Os -lm -lrt -o ixhelfire
