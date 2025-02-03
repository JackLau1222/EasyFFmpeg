#!/bin/bash
TOP_DIR=$PWD

directories=("example" "include")

for dir in "${directories[@]}"; do
    echo "format dir: $dir"
    find "$dir" -name "*.cpp" -exec clang-format -style="{BasedOnStyle: llvm, IndentWidth: 4}" -i {} \;
    find "$dir" -name "*.c" -exec clang-format -style="{BasedOnStyle: llvm, IndentWidth: 4}" -i {} \;
    find "$dir" -name "*.h" -exec clang-format -style="{BasedOnStyle: llvm, IndentWidth: 4}" -i {} \;
done