#!/bin/bash

for ex in $(find . -maxdepth 1 -type d ! -path . ! -path ./.git); do
  cd "$ex"

  echo -e "\e[1m=== Building: $ex\e[0m"

  make

  if [ $? -ne 0 ]; then
    echo -e "\e[31m==> Build failed\e[39m\n" >&2
    exit 1
  else
    echo -e "\e[32m==> Build successful\e[39m\n"
  fi

  cd ..
done
