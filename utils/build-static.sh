#!/bin/bash

BINARY="nnl_ndim"

# Move to root dir
pushd ..

# Clean
make clean

# Build
./configure CONFIG_SITE="./utils/sites/config.site-all-static"
make LDFLAGS="-all-static"
cp ./src/${BINARY} ./utils/${BINARY}
strip ./utils/${BINARY}

# Clean
make clean

# Back to working dir
popd
echo "The static binary is: ./${BINARY}"

exit

