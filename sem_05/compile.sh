#! /bin/bash

gcc hello.c -o a.hello.dynamyc
gcc -static hello.c -o a.hello.static
gcc -static -nostdlib hello.s -o a.hello.pure
