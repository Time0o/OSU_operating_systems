#!/bin/bash

./matrix dims matrix1.txt > /dev/null
[ $? -ne 0 ] && exit 1

./matrix transpose matrix1.txt > /dev/null
[ $? -ne 0 ] && exit 1

./matrix mean matrix1.txt > /dev/null
[ $? -ne 0 ] && exit 1

./matrix add matrix1.txt matrix2.txt > /dev/null
[ $? -ne 0 ] && exit 1

./matrix multiply matrix1.txt matrix2.txt > /dev/null
[ $? -ne 0 ] && exit 1

exit 0
