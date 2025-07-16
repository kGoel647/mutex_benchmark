#! /bin/bash

echo "This command should be run in a VM simulating a CXL environment"

# Make sure emucxl is cloned
if [ ! -d "emucxl "] ; then
    git clone "https://github.com/cloudarxiv/emucxl"
fi

# Make kernel module
cd ./emucxl/src
make clean > /dev/null
make > /dev/null 2>/dev/null

# Insert kernel module for CXL
sudo insmod emucxl_kernel.ko

# Run custom tests
gcc ../../test.c emucxl_lib.b emucxl_lib.h ../../bakery_static_mutex.c \
    && sudo ./a.out