#
# SPDX-License-Identifier: MIT
# Copyright (c) 2022-present jsoulier
# ARSLab - Carleton University
# Copyright (c) 2022-present Román Cárdenas Rodríguez
# ARSLab - Carleton University
#

echo Downloading all the dependencies...
git submodule update --init --recursive
mkdir build
cd build || exit
cmake ..
cmake --build .
cd ..
echo Compilation done. All the examples are in the bin folder
