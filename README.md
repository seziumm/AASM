# AASM

The AASM structure is provided below:

```
.
├── bin/                                # Folder for built application binary
├── include/                            # Dependencies
│   └── *.h                             # Source code of the application
├── src/                                # Folder for all source files
│   └── *.c                             # Source code of the application
└── CMakeLists.txt                      # Main compile script
```

## Dependencies

This project was built and tested on NixOS and Hyprland with the
following dependencies:

* CMake  >= 4.1
* Bear   >= 3.1.6 (optional)
* gcc    >= C99

## Installation

Clone this repo with

```
git clone https://github.com/seziumm/AASM.git
```

If you're using nixos and a wayland compositor, you can 
run the following command:

```
nix develop
```

It will put in a shell with all the library needed.


## Instructions

Do as you would any CMake project to compile:

```
mkdir build
cd build
cmake ..
make
```

Run:

```
cd bin
./AASM
```
# AASM
