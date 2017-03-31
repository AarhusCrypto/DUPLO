# DUPLO
A C++ implementation of the DUPLO cryptographic protocol

## Installation
The code has been tested to work on MacOS (10.12.1) and Ubuntu 16.04.

### Requirements
* C++ compiler with C++14 support. The code has been successfully built with GCC 5.3.1, GCC 6.1 and CLANG 3.8. For building the main DUPLO runtime code the following tools are required
* make
* cmake
* wget

In addition, for the TableProducer executable ([GMP](https://gmplib.org) for arbitrary-precision arithmetic) is required. Finally for the Frigate extension compiler, the following external libraries are required:
* [Flex](https://github.com/westes/flex)
* [Bison](https://www.gnu.org/software/bison/) (version 2.7)

NOTE: If these libraries are not detected by the build system, the TableProducer and/or Frigate executables will be skipped during compilation.

#### Installing GMP and Bison 2.7 on MacOS (10.12) and updating PATH
Using homebrew:
* brew install gmp/gmp@4
* brew install bison@2.7
* Add 'export PATH="/usr/local/opt/bison@2.7/bin:$PATH"' to end of ~/.bash_profile

#### Building on Linux/macOS
To clone, build and test the code:
* git clone --recursive https://github.com/AarhusCrypto/DUPLO
* cd DUPLO
* ./cmake-release.sh
* ./build/release/TestDUPLO