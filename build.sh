echo Downloading all the dependencies...
git submodule update --init --recursive
mkdir build
cd build || exit
cmake ..
cmake --build .
cd ..
echo Compilation done. All the examples are in the bin folder
