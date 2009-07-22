#!/bin/bash

# Define installation directory
install_dir="/usr/local"

# Move to root dir
pushd ..

# Clean
make clean

# Build
./configure CONFIG_SITE="./utils/sites/config.site-pentium4" --prefix=$install_dir
make
make install

# Clean
make clean

# Back to working dir
popd
echo "All done."

exit
