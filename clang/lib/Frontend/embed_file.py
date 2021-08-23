#!/usr/bin/python

import sys
import string

assert len(sys.argv) == 3, "expected two artguments"
destination = open(sys.argv[1], "w")
source = open(sys.argv[2], "r")

lines = source.readlines()
at_start = True
at_eof = False
for line in lines:
    for ch in line:
        if not at_start:
            destination.write(',')
        if at_eof:
            destination.write('\n')
            at_eof = False
        if ch == '\n':
            destination.write("'\\n'")
        elif ch == '\r':
            destination.write("'\\r'")
        elif ch == '\t':
            destination.write("'\\t'")
        elif ch == '\'':
            destination.write("'\\\''")
        elif ch == '\\':
            destination.write("'\\\\'")
        elif ch in string.printable:
            destination.write("'" + ch + "'")
        else:
            destination.write("'\\x" + hex(ord(ch)) + "'")
        at_start = False
    at_eof = True
if not at_start:
    destination.write(',')
destination.write("'\\0'")
destination.write('\n')
