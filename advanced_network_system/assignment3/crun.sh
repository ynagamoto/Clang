#!/bin/bash

if [[ -n $3 ]]; then
  args=${3//&/ }
fi

./bin/$1 $2 $args

