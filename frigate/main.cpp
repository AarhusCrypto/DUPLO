#include "exprtest.hh"
#include "traverse.h"
#include <iostream>

#include "ast.h"
#include "defines.h"
#include "includes.h"
#include "typegenerate.h"
#include "circuitoutput.h"
#include "interpreter.h"
#include "circuit.h"
using namespace std;




/*
 arguments:
 -i - run interpreter
 -i_io - see interpreter io
 -i_fg - see interpreter all gates
 -i_nostat - no stats on the interpreter
 -i_header - print header info
 -i_output  - outputs the circuit file into an inlined plain text format
 -i_validation  - checks that each output value is a "1" for party 1
 -pgc - print gate counts
 -no_ctimer - print compile time
 -sco - see output (for debugging purposes)
 -nowarn  - do not show warnings
 -notypes - do not output the type file
 */


#include <sys/time.h>


bool printCompileTime=true;
bool printGates = false;
bool runInterpreter = false;
bool printWarnings = true;
bool printInterpreterIO = false;
bool printInterpreterGates = false;
bool printInterpreterStats = true;
bool printInterpreterHeader = false;
bool useInterpreterValidationOption = false;
bool printFullGateList = false;
bool useTinyInstructions = false;

bool isBirstolDuplo = false;
bool is_AES = false;
bool is_random = false;
bool isBristolFile=false;
bool isPrintDuploFile = false;
string gatelistfilename;

//duplo
bool dpIO = false;
void processArgs(int argc, char *argv[])
{
    for(int i=2;i<argc;i++)
    {
        string s = argv[i];
        
        if(s == "-nowarn")
        {
            printWarnings = false;
            setNoWarn();
        }
        else if(s == "-i")
        {
            runInterpreter = true;
        }
        else if(s == "-i_io")
        {
            printInterpreterIO = true;
        }
        else if(s == "-i_fg")
        {
            printInterpreterGates = true;
        }
        else if(s == "-i_nostats")
        {
            printInterpreterStats = false;
        }
        else if(s == "-i_header")
        {
            printInterpreterHeader = true;
        }
        else if(s == "-i_validation")
        {
            useInterpreterValidationOption = true;
        }
        else if(s == "-i_output")
        {
            i++;
            
            if(i >= argc)
            {
                cerr << "i_output must have another argument\nExiting...\n";
                exit(1);
            }
            
            printFullGateList = true;
            
            gatelistfilename = argv[i];
        }
        else if(s == "-pgc")
        {
            printGates = true;
        }
        else if(s == "-no_ctimer")
        {
            printCompileTime = false;
        }
        else if(s == "-sco")
        {
            //see output
            setSeeOutput(true);
        }
        else if(s == "-notypes")
        {
            //do not print output type file
            setPrintIOTypes(false);
        }
        else if(s == "-tiny")
        {
            setTinyFiles(true);
        }
	    //Duplo
	    else if(s == "-dpIO")
	    {
		    runInterpreter = true;
		    printInterpreterIO = true;  
		    dpIO = true;     
	    }
	    else if(s == "-dp")
	    {
	        //see output
		    printDuploGC(true);
		    isPrintDuploFile = true;
	    } else if(s == "-bdp")
	    {
		    isBirstolDuplo=true;
	    } else if(s == "-b")
	    {
	        //see output
		    isBristolFile = true;
	    }
	    else if(s == "-aes")
	    {
	        //see output
		    is_AES = true;
	    }
	    else if(s == "-rand")
	    {
	        //see output
		    is_random = true;
	    }
        else
        {
            cout << "Undefined Arguement \""<< s <<"\"\n";
        }
    }
}

#include "wirepool.h"

int main(int argc, char *argv[])
{
    
	
    ///process args
	processArgs(argc, argv);
    
	struct timeval t0, t1;
	gettimeofday(&t0, 0);
    
	string file = argv[1];

	Node * topnode = generateAst(file);
    
	if (topnode == 0)
	{
		std::cerr << "Could not open file: \"" << file << "\"" << std::endl;
		exit(0);
	}

	includeIncludes(topnode, file);
	expandDefines(topnode);
    
    
    
	generateTypes(topnode);
    
	//cout << "starting output\n";
    
	outputCircuit(isProgramListNode(topnode), file);
    
    
    
	if (hasWarning() && printWarnings)
	{
		printErrors(std::cout);
	}
	else if (hasWarning())
	{
		cout << "\nWarnings hidden by -nowarn.\n\n";
	}
    
	if (printGates)
	{
		cout << "\n\n\t program and gatecount map: \n\n";
		topnode->print(std::cout, 0);
	}
    
	delete topnode;
    
    
	gettimeofday(&t1, 0);
	long long elapsed = (t1.tv_sec - t0.tv_sec) * 1000000LL + t1.tv_usec - t0.tv_usec;
	if (printCompileTime) cout << "compiler:\ttime(s): " << (elapsed * 1.0) / 1000000 << "\n";
	
	
	//ofstream fDuploGC;
	//read_text_sBoxYale();
	if (isPrintDuploFile)
	{		
	
	string fileGC = file + ".GC";
	char * S = new char[fileGC.length() + 1];
	strcpy(S, fileGC.c_str());
		 frigate_read_text_circuit(S, isBirstolDuplo, isBristolFile, is_AES,is_random);
		
}

    if(runInterpreter)
    {
        Interpreter interpret(printInterpreterGates,printInterpreterIO,printInterpreterHeader,printInterpreterStats,useInterpreterValidationOption,printFullGateList,gatelistfilename,dpIO);
        interpret.readyProgram(file);
        interpret.runprogram();
    }
}




