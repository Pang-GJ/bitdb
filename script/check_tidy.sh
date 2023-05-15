#!bin/bash

find . -name \*.h -or -name \*.cpp | grep -vE "^./build/" | xargs -n12 -P8 \
  clang-tidy --config-file=./.clang-tidy
