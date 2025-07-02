export CMAKE_PREFIX_PATH=$PREFIX
export CMAKE_INCLUDE_PATH=$PREFIX/include
export PKG_CONFIG_PATH=$CONDA_PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH
cmake -B build -DCMAKE_INSTALL_PREFIX:PATH=$PREFIX .
make -C build -j 2
make -C build install
