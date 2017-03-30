#include "exprtest.hh"
#include "traverse.h"
#include <iostream>

#include "ast.h"
#include "defines.h"
#include "includes.h"
#include "typegenerate.h"
#include "circuitoutput.h"
#include "interpreter.h"

using namespace std;




/*
 arguments:
 -i_io - see interpreter io
 -i_fg - see interpreter all gates
 -i_nostat - no stats on the interpreter
 -i_header - print header info
 -i_output  - outputs the circuit file into an inlined plain text format
 -i_validation  - checks that each output value is a "1" for party 1
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
string gatelistfilename;
bool dpIO = false;

void processArgs(int argc, char *argv[])
{
    for(int i=2;i<argc;i++)
    {
        string s = argv[i];
        
        /*if(s == "-nowarn")
        {
            printWarnings = false;
        }
        else if(s == "-i")
        {
            runInterpreter = true;
        }*/
        if(s == "-i_io")
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
	    //Duplo
	    else if(s == "-dpIO")
	    {
		    runInterpreter = true;
		    printInterpreterIO = true;               
		    dpIO = true;     
	    }
        else
        {
            cout << "Undefined Arguement \""<< s <<"\"\n";
        }
    }
}

int main(int argc, char *argv[])
{
    ///process args
    processArgs(argc,argv);

    
    string file = argv[1];
    
    
    {
        Interpreter interpret(printInterpreterGates,printInterpreterIO,printInterpreterHeader,printInterpreterStats,useInterpreterValidationOption,printFullGateList,gatelistfilename,dpIO);
        interpret.readyProgram(file);
        interpret.runprogram();
    }
    
    
}
