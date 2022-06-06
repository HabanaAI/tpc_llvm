#!/usr/bin/python

import sys
import string

CharsPerLine = 20

assert len(sys.argv) == 3, "expected two artguments"
destination_file = sys.argv[1]
source_file = sys.argv[2]
destination = open(destination_file, "w")
try:
    source = open(source_file, "rb")
    num_chars = 0
    while True:
        ch = source.read(1)
        if not ch:
            break
        destination.write("'\\x%02x'" % ord(ch))
        destination.write(',')
        num_chars += 1
        if num_chars >= CharsPerLine:
            destination.write('\n')
            num_chars = 0
except IOError:
    pass

destination.write('\n')
