#!bin/bash

find . -name \*.h -or -name \*.cpp | grep -vE "^./build/" | xargs -n12 -P8 \
cpplint --counting=detailed --linelength=120 \
  --filter=-legal/copyright,-build/header_guard,-runtime/references,-build/c++11
