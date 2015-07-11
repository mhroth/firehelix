#!/bin/bash

gcc firehelix.c heavy/*.c -std=c99 -Werror -Os -o firehelix
