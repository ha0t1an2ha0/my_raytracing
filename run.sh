#!/bin/bash
cd /mnt/c/Users/haotian.zhao/source/repos/my_raytracing/build
make clean
make
if [ $? -eq 0 ]; then
  cd ..
  bin/zrt > image1.ppm
  feh image1.ppm
else
  echo "Build failed."
fi
