#!/bin/bash

# Make sure we are in the top-level llvm repository.
if [ ! -d "../.git" ]; then
    echo "Script $0 must be called from llvm source directory"
    exit 1
fi

if [ ! -r "./TPC.build" ]; then
    echo "Script $0 must be called from llvm source directory"
    exit 1
fi

# Make sure the delivered repositories are clean.
MODIFIED=`git status --untracked-files=no 2>&1 | grep 'modified:'`
if [ -n "$MODIFIED" ]; then
    echo "Working directory contains uncommitted changes"
    exit 1
fi

# Increment commit number
BUILD_NUM=`cat ./TPC.build`
BUILD_NUM=$(($BUILD_NUM+1))
echo "New build number is: $BUILD_NUM"
echo $BUILD_NUM > ./TPC.build

# Put tags in the delivered repositories.
git add ./TPC.build && \
git commit -m "Incremented build number" && \
git tag -a build-${BUILD_NUM} -m "Drop #${BUILD_NUM}" && \

if [ $? != "0" ]; then
    echo "ERROR"
    exit 1
fi

# Push the commit that increments build number as well as the new tags in llvm and clang.
# NB: We assume the remote repository is named as 'origin'.
echo -n "Do you want to push the changes? (Y/N)"
read reply
if [ "$reply" = "Y" -o "$reply" = "y" ]; then
    git push origin && \
    git push origin build-${BUILD_NUM} && \
    if [ "$?" != "0" ]; then
        echo "ERROR"
        exit 1
    else
        echo "Changes have been sent to origin"
    fi
else
    echo "Changes were not pushed"
fi
