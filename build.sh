#! /bin/bash
#
# SPDX-License-Identifier: MIT
# Copyright (c) 2022-present Román Cárdenas Rodríguez
# Copyright (c) 2025-present Sasisekhar
# ARSLab - Carleton University
#


# Function to add the CADMIUM variable to the environment
add_cadmium_env() {
  # Check if CADMIUM is already set in .bashrc
  if ! grep -Fxq "export CADMIUM=$(pwd)" ~/.bashrc; then
    echo "export CADMIUM=$(pwd)" >> ~/.bashrc
    echo "The CADMIUM variable has been set to $(pwd) in ~/.bashrc."
  else
    echo "The CADMIUM variable is already set in ~/.bashrc."
  fi

  # Source the updated .bashrc
  source ~/.bashrc
}

echo Downloading all the dependencies...
sudo apt install build-essential make cmake git
git pull
git submodule update --init --recursive --progress
mkdir build
cd build || exit
cmake ..
make all
cd ..
echo Compilation done. All the examples are in the bin folder

cd include
# Prompt the user for confirmation
echo "Do you want to add cadmium ($(pwd)) to the PATH? (yes/no)"
read -r response

# Check the response and take action
if [[ "$response" =~ ^[Yy][Ee][Ss]$ || "$response" =~ ^[Yy]$ ]]; then
  add_cadmium_env
elif [[ "$response" =~ ^[Nn][Oo]$ || "$response" =~ ^[Nn]$ ]]; then
  echo "Operation canceled. The $(pwd) was not added to PATH."
else
  echo "Invalid response. Please run the script again and respond with 'yes' or 'no'."
fi
