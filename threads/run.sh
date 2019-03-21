#!/bin/bash

./buffer 1> out.txt 2> err.txt  &
PID="$(ps -ux | grep -m 1 ./buffer | cut -d ' ' -f 9)"
echo $PID
sleep 30
kill -2 $PID 

