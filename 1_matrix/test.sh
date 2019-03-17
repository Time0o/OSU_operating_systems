#!/bin/bash

echo "dims..."
./matrix dims matrix1.txt > /dev/null
[ $? -ne 0 ] && exit 1

echo "transpose..."
./matrix transpose matrix1.txt > /dev/null
[ $? -ne 0 ] && exit 1

echo "mean..."
./matrix mean matrix1.txt > /dev/null
[ $? -ne 0 ] && exit 1

echo "add..."
./matrix add matrix1.txt matrix2.txt > /dev/null
[ $? -ne 0 ] && exit 1

echo "multiply..."
./matrix multiply matrix1.txt matrix2.txt > /dev/null
[ $? -ne 0 ] && exit 1

exit 0
