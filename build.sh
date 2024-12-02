#!/bin/bash



files=$(find ./src/ -iregex '.*\.c')

demotarget="$1"

if ! [ -f $demotarget ]; then
    echo "$demotarget not found from ./demos directory"
    exit 1
fi

name="out"
compiler_flag="-ggdb"

files="$files $demotarget"

if [[ $2 == "o" ]]; then
    compiler_flag="-O3"
fi


if gcc $files $compiler_flag \
    -Wall -Wextra \
    -lglfw -lGL -lGLEW -lm \
    -o $name; then
 
    echo -en "\033[32m"
    ls -lh $name
    echo -en "\033[0m"

    if [[ $2 == "r" ]]; then
        ./$name
    fi

fi
