This source was written on OSX 10.9.5, it uses some C++ 11 features.Compiler Dependencies:Bison (errors with version >=3, I suggest 2.7.1)Flex (there may be a compatibility issue on some Macs that have installed a second version of Flex, see note below)I used the default mac versions for command line utilities that came with Xcode 6.1 during development (bison  2.7.12-4996, flex  2.5.37)getting bison 2.7.1 in ubuntu (from http://askubuntu.com/questions/444982/install-bison-2-7-in-ubuntu-14-04)wget http://launchpadlibrarian.net/140087283/libbison-dev_2.7.1.dfsg-1_amd64.debwget http://launchpadlibrarian.net/140087282/bison_2.7.1.dfsg-1_amd64.debdpkg -i libbison-dev_2.7.1.dfsg-1_amd64.debdpkg -i bison_2.7.1.dfsg-1_amd64.deb
Compatibility Problems: We have successfully tested Frigate on Linux (Ubuntu) and mac 10.9 but ran into a compatibility problem on mac 10.10 with flex, this problem appeared to be related to a wrong version of flex called at compile time: see https://github.com/jonathan-beard/simple_wc_example/issues/7Minimal instructions to create a VM for Frigate64 bit ubuntu vm created from ubuntu-14.04.03-desktop-amd64.iso
[I used virtual box with a hard drive size of 32 GB]
[I select download updates while installing / install 3rd party things]


sudo apt-get install git
git clone [GIT PATH]
cd frigaterelease
cd src
sudo apt-get install flex
wget http://launchpadlibrarian.net/140087283/libbison-dev_2.7.1.dfsg-1_amd64.debwget http://launchpadlibrarian.net/140087282/bison_2.7.1.dfsg-1_amd64.debsudo dpkg -i libbison-dev_2.7.1.dfsg-1_amd64.debsudo dpkg -i bison_2.7.1.dfsg-1_amd64.deb
sudo apt-get install g++
make
./frigate ./tests/rsa128.wir -i 
Interpreter Dependencies:No packages, but uses memory mappingBuild:make  (by default “make” also runs the validation test program)Run:frigate <program> flag1 … flag ncompiler output:a set of mfrig and ffrig files and a single an otype.mfrig file that labels what the input and output wires correspond to (only useful if structs or arrays are used as input)Interpreter: (the interpreter is a part of Frigate but is also provided separately for demonstration purposes) [make interp]battleship <program>   flag1 … flag nThe program file name must be the *program*.wir filename. However, this file is not actually called, i.e., only the  mfrig and ffrig are read in by the interpreter.

for instance:

./battleship ./tests/validationtest.wirExample of full circuit output (ASCII):make circuits flags: -i           run interpreter after compilation -i_io        see interpreter input and output (requires -i) -i_fg        see interpreter execute each gates (requires -i) -i_nostat    do not print stats on the interpreter (requires -i) -i_header    print header info (requires -i) -i_output [file]	prints out gates and input output (copies are replaced with XORs with 0 as second operand) file (requires -i) -i_validation  runs the validation routine in the interpreter (i.e. checks the output is 1 for party 1) (requires -i) -pgc         print gate counts inlined into program printout  -no_ctimer   do not print compile time -sco         see output of the compiler (warning, this can be difficult to parse and understand, recommended only for debugging purposes) -nowarn      issue no warnings -notypes     do not print output type fileexperimental/incomplete flags:-tiny    shrinks the program size [only works for addition / subtraction and a few other operators]example uses of Frigate:
./frigate ./tests/temp.wir -i->>>> this compiles temp.wir, runs it with the interpreter

./frigate ./tests/temp.wir -i_output out -i
->>>> this compiles temp.wir, runs it with the interpreter and outputs the circuit to file “out”

./frigate ./tests/temp.wir -i -i_io->>>> this compiles temp.wir, runs it with the interpreter and prints out the output
examples of flag output:
flag: -i
output:
“
interpreter:	gates: nonxor: 8  free ops: 49   allgates: 32
		time(s): 0.000635
“
in this case, the program has 8 non xor gates, 49 total operations, and 32 total gates. The inter peter took 0.000635 seconds to generate the circuit from the frigate output files




flag: -i -i_io        [-i is required with -i_io]
output:
“
input 1 0 1
input 0 1 1
input 0 2 1
input 1 3 1
input 1 8 2
input 1 9 2
input 0 10 2
input 0 11 2
output 0 4 1
output 0 5 1
output 1 6 1
output 1 7 1
output 0 12 2
output 0 13 2
output 1 14 2
output 1 15 2
“

input 1 0 1    
<input> <real_bit_value> <wire that we are setting to the real_bit_value> <which party’s input>

in the case of “input 1 0 1”, we are setting wire 0 to value 1 for party 1
in the case of “input 1 9 2”, we are setting wire 9 to value 1 for party 2

NOTE: INPUTS CAN BE CHANGED IN “inputs.txt” -> each line is a party’s input 





flag: -i_output out -i     [-i is required with -i_output]

output: (cat’d from file “out”)
“
IN 0 1
IN 1 1
IN 2 1
IN 3 1
IN 8 2
IN 9 2
IN 10 2
IN 11 2
15 16 0 0
0 17 0 0
6 19 8 0
8 21 19 0
6 18 0 21
6 26 9 1
6 23 26 18
6 20 18 1
8 21 26 20
6 18 1 21
6 26 10 2
6 24 26 18
6 20 18 2
8 21 26 20
6 18 2 21
6 26 11 3
6 25 26 18
copy(6) 4 19 17
copy(6) 5 23 17
copy(6) 6 24 17
copy(6) 7 25 17
OUT 4 1
OUT 5 1
OUT 6 1
OUT 7 1
“



IN 3 1 -> input next bit from party 1 to wire 3

8 21 19 0     or    15 16 0 0 -> gate commands, in the case of 8 21 19 0, take inputs (from wires) 19 and 0, use truth table 8, and output to wire 21. “truth table 8” refers to the truth table output values represented as a integer (8). 8 = output_00 | output_01 < 2 | output_10 < 3 | output_11 << 3. In other words 8 is an AND gate, 6 is an XOR gate, 14 is an OR gate, 15 always returns 1 no matter the inputs, and 0 always returns 0 no matter the inputs.

copy(6) 4 19 17 -> copies whats on wire 19 to wire 4. (for the garbled circuit implementation I was using there was no copy command, so i used XOR gates (hence the (6)) such that wire 4 = wire 19 ^ wire 17. In this case wire 17 is known to be 0.

OUT 4 1 -> output whats on wire 4 to party 1

limitations:
-hard limit of 32 bit addresses in the interpreter (i.e. max wires for input/output). No reason it cannot be extended, but this would require changing some types / output sizes. what it means: if the total wires input + output + used in functions) exceeds 32 bits, bad things happen.  If anyone wants this extended, let me (first author) know.
—————brief description of language—————operators/typing:+ - * / % have both signed and unsigned capabilities** // %% are extending and reducing versions of the * / and % operators. ** takes two N bit inputs and produces a 2*N bit output. // and %% take in a 2*N (dividend) and N (divisor)-bit numbers and outputs an N-bit number. Note that in the case of a divisor of  say 5 bits in length, both 10-bit and 11-bit are possible dividend types (i.e. must satisfy (dividend +1)/2 == divisor. KEEP IN MIND // and %% ONLY WORK IF THE RESULT CAN FIT INSIDE OF THE QUOTIENT.  They are not used in any of our test or example programs. << >> <<> require numbers (and right side must not be input dependent) but rotate must have a fixed length (otherwise, how do you know how to rotate the value?)< > >= <= require signed numbers (but in the case of single bit operations will function as unsigned operators){} and [] require numbers with known values (i.e. not input dependent)== = != can be used with any typefunctions:function return_type foo(paramtype1 name1, paramtype2 name2, paramtype3[5] name3){	return blah;}return can ONLY be the last statement in non-void functions it is also necessary for non-void functions.loops: for(type i =0;i<max;i++){}(Note, a declaration, condition, and expression is expected in a for loop, e.g., cannot skip the type declaration with for(;i<max;i++){} or the like)defines:#define NAME expression (unlike C++ or C where define can replace almost anything, in Frigate only terms [variables] can be replaced) includes:#include “filename”defining number of parties:#parties NUMBERdefining input and output types#input PARTY typeor#output PARTY typewire operator:variable{N} gets the Nth wire where the wires are 0 to LEN-1variable{N:Q} gets Q wires starting at wire N.Typedefs:integer:typedef int_t bitlength type_nameunsigned integer:typedef uint_t bitlength type_namestruct:typedef struct_t newstruct{	int x;structtype var[5];}The exact grammar can be gleaned by looking at the rules in parser.yy, I recommend using parenthesis with complex expressions, i.e., (expression) == (expression)special operator behavior:Odd can happen when untyped constants are used.Shift left and shift right technically extends the value (or reduce the size) of the output -> this behavior is needed for untyped constants——Modifying Frigate————Code Layout——There are several classes, most notable the AST, and also many functions outside of the various classes.main.cpp : the main file for the compiler	ast.cpp and ast.h : contain the classes and functions for the AST (including the node typing rules)traverse.h and traverse.cpp : classes for AST traversalsdefines.cpp and defines.h : functions for dealing with #define statementsincludes.cpp and inputs.h : functions for dealing with #include statementsparser.yy : the bison file to generate the parserscanner.ll : the flex file to generate the scannertypegenerate.cpp and typegenerate.h : functions for creating the types for the programtypes.cpp and types.h : classes and functions for type classes (notice that types.cpp and types.h contain functions for the type classes but the typegenerate files contain the code for using those types)errors.h and errors.cpp : contain functions for dealing with errorswire.h and wire.cpp : the wire class and related functionswirepool.h and wirepool.cpp : the wire pool class (i.e., wire garble collector) and related functions circuitoutput.h and circuitoutput.cpp : functions for dealing with the output of the circuit.  Note: there is a high degree of connectivity between wire.h, wire.cpp, circuitoutput.h, and circuitoutput.cppvariable.cpp and variable.h : classes and functions for variablesinterpreter.h interpreter.cpp : files for the interpreter, there should not no dependencies on the rest of the compiler. scanner.h, parser.hh, FlexLexer.h, exprtest.hh, exprtest.cpp, parse_driver.cc, and parse_driver.h : original example files I built frigate of. ——Adding new operations——To add in new operations the following tasks must be completed: 1. add in symbols to scanner.ll2. add in parser rules to parser.yy3. create the AST node in ast.h4. create the typing rule for the AST node ast.h/ast.cpp5. create how the node will be output in ast.cpp /circuitoutput.cpp/.h5.1 I highly recommend following a current node template (one example is ArithMinusNode::circuitOutput in ast.cpp). This involves getting output from the children nodes and dumping that result to the vector part of the CORV(“circuit output return value” - think of it as a struct that represents either a variable or a vector (or both!)) called “preparation”, outputting the new circuit, and cleanup (unlocking the previous wires so they can be garbled collected, locking the new wires so it won’t be garbage collected, and calling the garbage collector).  If you are adding a complex operation, you may need to understand the dependencies of how the wire object works in all its details.Note: there are many “messages” in the code that can be triggered if the output is not performed correct. If you see a strange phrase, I recommend finding out where it occurs (via grep) and look why. These should not be possible to trigger in the standard built of Frigate. If you see a list of wires at the conclusion of a function’s compilation, it means some of the wires are still “locked” i.e. they could not be garbage collected. Technically this will not create an erroneous program, but it might slow it down depending upon cache efficiencies and the number of extra wires allocated. At the conclusion of a function all wires are deallocated after this warning occurs.