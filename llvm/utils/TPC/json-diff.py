#!/usr/bin/python

import sys
import json

eps = 0.0000001


def is_equal(val1, val2):
    if type(val1) is float or type(val2) is float:
        return abs(val1 - val2) < eps
    return val1 == val2


def compareArrays(orig,new):
    differences = []
    diffOrig = []
    diffNew = []
    for i, value1 in enumerate(orig):
        if len(new) <= i:
            diffOrig.append(value1)
            continue
        value2 = new[i]
        if not is_equal(value1, value2):
            diffOrig.append(value1)
            diffNew.append(value2)
        elif diffOrig != [] or diffNew != []:
            differences.append((diffOrig,diffNew))
            diffOrig = []
            diffNew = []
    if len(new) > len(orig):
        diffNew = diffNew + new[len(orig):]
    if diffOrig != [] or diffNew != []:
        differences.append((diffOrig, diffNew))
    if differences != []:
        return differences
    else:
        return False

def checkDifference(orig, new):
    diff = {}
    if type(orig) != type(new):
        # print "Type difference"
        return True
    if type(orig) is dict and type(new) is dict:
        # print "Types are both dicts"
        ##	Check each of these dicts from the key level
        diffTest = False
        for key in orig:
            if key not in new:
                diff[key] = ("KeyNotFound", orig[key])
                print("Key not found: " + key)
                diffTest = True
            else:
                result = checkDifference(orig[key], new[key])
                if result != False:  ## Means a difference was found and returned
                    diffTest = True
                    print("key/Values different in: " + str(key))
                    if type(result) is list:
                        for diffGroup in result:
                            for value in diffGroup[0]:
                                print(value)
                            print("---")
                            for value in diffGroup[1]:
                                print(value)
                            print("===")
                    elif type(result[0]) is str:
                        print(result[0])
                        print("---")
                        print(result[1])
                    diff[key] = result
        ##	And check for keys in second dataset that aren't in first
        for key in new:
            if key not in orig:
                diff[key] = ("KeyNotFound", new[key])
                print("Key not found: " + key)
                diffTest = True
        if diffTest:
            return diff
        else:
            return False
    elif type(orig) is list and type(new) is list:
        #print "Types were not dicts, likely strings"
        return compareArrays(orig,new)
    else:
        return False if is_equal(orig, new) else (orig, new)
    return diff

def main(argv):
    if "-eps" in argv:
        epsIndex = argv.index("-eps")
        global eps
        eps = float(argv[epsIndex+1])
        del argv[epsIndex+1]
        argv.remove("-eps")
    if len(argv) < 2:
        print("Usage: python json-diff.py <file1>.json <file2>.json [-eps <precision>]")
        return
    with open(argv[0]) as f:
        json1 = json.load(f)
    with open(argv[1]) as f:
        json2 = json.load(f)
    if checkDifference(json1, json2):
        exit(1)


if __name__ == "__main__":
    main(sys.argv[1:])
