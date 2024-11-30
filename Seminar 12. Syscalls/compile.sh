#! /bin/bash

gcc $1 -o $1.dynamyc
gcc -static $1 -o $1.static
