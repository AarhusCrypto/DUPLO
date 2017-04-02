# Frigate Extension Compiler (used for Duplo)
This is the implementation of our xxx paper: **DUPLO: Unifying Cut-and-Choose for Garbled Circuits**[[ePrint](https://eprint.iacr.org/2017/xxx)].  We extend the [`Frigate`](https://bitbucket.org/bmood/frigaterelease) compiler that allows to transform a high-level C-style program into a
set of boolean circuit components that can be fed to
the DUPLO system for secure computation

## Abstract
---


## Installations
---
### Required libraries
  * [`bison  2.7.12-4996`](http://launchpadlibrarian.net/140087283/libbison-dev_2.7.1.dfsg-1_amd64.deb)
  * [`flex  2.5.37`](http://launchpadlibrarian.net/140087282/bison_2.7.1.dfsg-1_amd64.deb)
  
Please read [`Frigate`](https://bitbucket.org/bmood/frigaterelease) for more detail about how to install the required libraries
### Building the Project
After cloning project from git,

1. make
2. Run:
	frigate <program> flag1 … flag n
	
#### Additional Flags:
	-dp		output circuit file in DUPLO format
	-aes 		embed the [`S-Box`](http://www.cs.yale.edu/homes/peralta/CircuitStuff/CMT.html) circuit to generate AES circuits efficiently
	-rand		generate a random component subcircuit to finding "sweet spot" for DUPLO
#### Example:
	./frigate ./circuits/test.wir -dp
##### Generate AES 
	./frigate ./circuits/aes/p1_aes.wir -dp -aes

##### Generate random circuits
	./frigate ./circuits/randomcCircuits/p12_1_test.wir -dp -rand
	
### DUPLO circuit format: 
Each file .GC_Duplo consists of:
1. A first line defining the number of functions and then the number of components in the circuit.
2. A second line defining the length (in bits) of constructor's input, evaluator's input, the size of total output, the size of construtor's output, and the size of evaluator's output
3. Each function starts at "FN" and ends at "--end FN #id--". The first line of the function shows the function index, the length of input, the length of output, the number of wires, the number of non-xor gate, and the number of gates.
4. Gate operation: 
	input_wire_1 input_wire_2 output_wire gate_operation


