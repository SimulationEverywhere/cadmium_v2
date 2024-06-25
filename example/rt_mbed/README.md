# Blinky model example

This is the _Hello World!_ example for embedded systems.

For a detailed explanation of this example check this repo: https://github.com/epecker/jupyter-cadmiumv2.git.

## Simulation in real-time with a general purpose operating system

To build this model to run in real-time in a PC with Linux:

```bash
cd top_model
make all
```

Then, an executable will be placed in the folder `BUILD`. To run a simulation for 10~s just type:

```bash
./Blinky
```

The results are then placed in the file `blinkyLog.csv`.

You can clean the executable and other generated files (object ang log files) with:

```bash
make clean
```

## Execution in real-time in an embedded board running Mbed-OS v5

### Install the toolchain

To build an application with Mbed-OS we will use the **C**ommand **L**ine **I**nterface version 1 (Mbed CLI 1). This is a Python-based building tool. 

Mbed CLI1 can make use of different building toolchains, but we are going to use the **GNU ARM Embedded toolchains** (named `GCC_ARM` compiler in CLI).

In what follows I'll assume we are using `gcc-arm-none-eabi-10.3-2021.10` for Linux (tested with Ubuntu 18.04 and 20.04). To download it, go to this [link](https://developer.arm.com/downloads/-/gnu-rm) and download the corresponding version for your OS. For Ubuntu:
```bash
$ wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2?rev=78196d3461ba4c9089a67b5f33edf82a&hash=D484B37FF37D6FC3597EBE2877FB666A41D5253B
```

Then, extract GCC ARM:
```bash
$ tar jxf gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2
```

And finally move the GCC ARM Compiler to the `/opt/` folder (requires root privileges):
```bash
$ sudo mv gcc-arm-none-eabi-10.3-2021.10 /opt/
```

### Installation and setup of Mbed CLI 1

A step-by-step guide to install the Mbed CLI 1 tool can be found [here](https://os.mbed.com/docs/mbed-os/v6.15/build-tools/install-and-set-up.html). Make sure that all the dependencies are satisfied, otherwise run the following:

```bash
$ sudo apt install python3 python3-pip git
```

Then, to install the mbed-cli run:

```bash
$ python3 -m pip install mbed-cli
```

Finally, to configure Mbed CLI 1 we use the command `mbed config` to make Mbed-OS know where to find the GCC ARM Compiler that we downloaded before. Given that we are using the GCC ARM Embedded Compiler, we have to set the `GCC_ARM_PATH`:
```bash
mbed config -G GCC_ARM_PATH /opt/gcc-arm-none-eabi-10.3-2021.10/bin
```
where the `-G` means that this setting is global, i.e. it applies to all the applications compiled with Mbed CLI 1.

### Donwload Mbed-OS v5

This model was tested with **Mbed-OS v5.13**. To download this version run:

```bash
mbed deploy
```

This command makes use of the link in the file `mbed-os.lib`.

## Build and run the simulation

To build this model with Mbed CLI1 to run in a embedded platform and flash the executable just type:
```bash
make embedded
```
Make sure that you have already plugged the board to the USB port, otherwise an error will came up. Once flashed, a serial terminal will start where the events and output values are printed. To close this terminal just type CTRL+C.

In case you want to modify the compilation flags for the C/C++ compiler (arm-none-eabi-gcc / arm-none-eabi-g++) and/or set the path for additional libraries, edit the profile `cadmium_logging_off.json` in the parent folder.

Finally, in order to clean the executable and other generated files just type:
```bash
make clean-embedded
```


