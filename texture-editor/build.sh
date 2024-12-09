#!/bin/bash



files=$(find ./src/ -iregex '.*\.c')
sandbox_src_files=$(find ../src/ -iregex '.*\.c')

files="$files $sandbox_src_files"

echo $files

name="texedit"
compiler_flag="-ggdb"

if gcc $files $compiler_flag \
    -Wall -Wextra \
    -lglfw -lGL -lGLEW -lm \
    -o $name; then
 
    echo -en "\033[32m"
    ls -lh $name
    echo -en "\033[0m"

    if [[ $1 == "r" ]]; then
        ./$name asd.tex
    fi

fi
