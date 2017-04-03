# DUPLO
A C++ implementation of the DUPLO cryptographic protocol

## Installation
The code has been tested to work on MacOS (10.12.1) and Ubuntu 16.04.

### Requirements
* C++ compiler with C++14 support. The code has been successfully built with GCC 5.3.1, GCC 6.1 and CLANG 3.8. For building the main DUPLO runtime code the following tools are required
* make
* cmake
* wget

In addition, for the TableProducer executable [GMP](https://gmplib.org) for arbitrary-precision arithmetic is required. Finally for the Frigate extension compiler, the following external libraries are required:
* [Flex](https://github.com/westes/flex) (tested with Flex 2.5.35, 2.5.37 and 2.6)
* [Bison](https://www.gnu.org/software/bison/) (version 2.7, seems to be issues on systems with bison 3.x)

NOTE: If these libraries are not detected by the build system, the TableProducer and/or Frigate executables will be skipped during compilation.

#### Installing GMP and Bison 2.7 on MacOS (10.12) and updating PATH
Using homebrew:
* brew install gmp
* brew install gmp@4
* brew install bison@2.7
* Add 'export PATH="/usr/local/opt/bison@2.7/bin:$PATH"' to end of ~/.bash_profile

#### Linux Notes
Links to the required packages for the Frigate compiler:
  * [`bison  2.7.12-4996`](http://launchpadlibrarian.net/140087283/libbison-dev_2.7.1.dfsg-1_amd64.deb)
  * [`flex  2.5.37`](http://launchpadlibrarian.net/140087282/bison_2.7.1.dfsg-1_amd64.deb)
  
Please see [`Frigate Readme`](https://bitbucket.org/bmood/frigaterelease) or [`Local Frigate Readme`](https://github.com/AarhusCrypto/DUPLO/blob/master/frigate/README.txt) for more details on how to install the required libraries.

### Building the Project
To clone, build and test the code:
* git clone --recursive https://github.com/AarhusCrypto/DUPLO
* cd DUPLO
* ./cmake-release.sh
* ./build/release/TestDUPLO

## Frigate Extension Compiler
The [`Frigate`](https://bitbucket.org/bmood/frigaterelease) compiler transforms a C-style program into a bolean circuit. We here provide an extension of this work that in addition can output a set of boolean circuits along with an appropriate soldering description that can be fed to the DUPLO system for secure computation of the original source program.

### Running the compiler
	
#### Flags:
	-dp		output circuit file in DUPLO format
	-aes 		embed the [`S-Box`](http://www.cs.yale.edu/homes/peralta/CircuitStuff/CMT.html) circuit to generate AES circuits efficiently
	-rand		generate a random component subcircuit to finding "sweet spot" for DUPLO
##### Example:
	./Frigate ./circuits/test.wir -dp
##### Generate AES 
	./Frigate ./circuits/aes/p1_aes.wir -dp -aes
NOTE: In order to build a program using the -aes flag, the $curr_working_dir must the the root of the project, i.e. (DUPLO), else our compiler cannot find the custom s-box circuit that we provide.

##### Generate random circuits
	./Frigate ./circuits/randomcCircuits/p12_1_test.wir -dp -rand
	
#### DUPLO circuit format: 
Each file .GC_Duplo consists of:
1. A first line defining the number of functions and then the number of components in the circuit.
2. A second line defining the length (in bits) of constructor's input, evaluator's input, the size of total output, the size of construtor's output, and the size of evaluator's output
3. Each function starts at "FN" and ends at "--end FN #id--". The first line of the function shows the function index, the length of input, the length of output, the number of wires, the number of non-xor gate, and the number of gates.
4. Gate operation: 
	input_wire_1 input_wire_2 output_wire gate_operation

gate_operation legend:
* "0000"	: constant 0 //Never generated explicitly, supported through other means (XOR).
* "0001"	: NOR
* "0010"	: (NOT left) AND right
* "0011"	: NOT left-wire
* "0100"	: left AND (NOT right)
* "0101"	: NOT right-wire
* "0110"	: XOR
* "0111" 	: NAND
* "1000" 	: AND
* "1001" 	: NXOR
* "1010" 	: right identity
* "1011" 	: (NOT left) OR right
* "1100" 	: left identity
* "1101" 	: left OR (NOT right)
* "1110" 	: OR
* "1111" 	: constant 1 //Never generated explicitly, supported through other means (XOR + NOT).
