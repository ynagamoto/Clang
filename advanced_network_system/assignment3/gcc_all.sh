#!/bin/bash

mkdir -p bin

dir=documentroot
i=0

for file in $dir/*.c; do
  temp=(${file//\// })
  temp=(${temp[1]//./ })
  files[i]=${temp[0]}
  gcc $file -o ./bin/${files[i]}
  let i++
done

# for file in ${files[*]}; do
#   echo $file
# done
