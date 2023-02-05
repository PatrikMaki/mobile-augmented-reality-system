#!/bin/bash
# Run the mars server in a loop in case it would crash.

cd build
while :
do
  ./mars
  sleep 1
done
