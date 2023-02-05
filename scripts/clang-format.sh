#! /bin/bash

for run in {1..2}
do
  git clang-format HEAD^ -- include/*.h benchmarks/*.cpp tests/*.cpp superqueue/*.h superfactory/*.h tests/*.h -f
done
