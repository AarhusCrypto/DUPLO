

#include <iostream>
#include <fstream>

#include "parse_driver.h"
#include "ast.h"
#include "traverse.h"

#include "exprtest.hh"

Node * generateAst(string filename)
{
    ProgramContext prog;
    compiler::Driver driver(prog);
    bool readfile = false;

    std::fstream infile(filename);
    if (!infile.good())
    {
        //std::cerr << "Could not open file: {" << filename << "}"<<std::endl;
        //exit(0);
        return 0;
    }
    
    setEFileName(filename);

    //calc.clearNodes();
    bool result = driver.parse_stream(infile, filename);
    
    infile.close();
    
    if (result)
    {
        //std::cout << "Program:" << std::endl;
        for (unsigned int ei = 0; ei < prog.topLevelNodes.size(); ++ei)
        {
            //std::cout << "[" << ei << "]:" << std::endl;
            //std::cout << "tree:" << std::endl;
            if(prog.topLevelNodes[ei] == 0)
            {
                cout <<"top level node is null\n";
            }
           return  prog.topLevelNodes[ei]/*->print(std::cout)*/;
	}
    }

    return 0; 
}

