# Batched Oblivious PRF
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
	-dp			output circuit file in DUPLO format
	-b			output circuit file in [`Bristol`](https://www.cs.bris.ac.uk/Research/CryptographySecurity/MPC/) format
	-aes 		embed the [`S-Box`](http://www.cs.yale.edu/homes/peralta/CircuitStuff/CMT.html) circuit to generate AES circuits efficiently
	-rand		generate a random component subcircuit to finding "sweet spot" for DUPLO
#### Example:
	./frigate ./circuits/test.wir -dp
##### Generate AES 
	./frigate ./circuits/aes/p1_aes.wir -dp -aes

##### Generate random circuits
	./frigate ./circuits/randomcCircuits/p12_1_test.wir -dp -rand
	
### DUPLO circuit format: 
Each file consists of:
