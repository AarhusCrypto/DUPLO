#include "ast.h"
#include "traverse.h"
#include "defines.h"
#include <string.h>

#include <unordered_map>


std::unordered_map<string,Node *> define_map;

std::unordered_map<string,Node *> & getDefineMap(){return define_map;}

void expandDefines(Node * headNode)
{
    DefinesTraverse dt;
    dt.traverse(headNode);
    
    if(hasError())
    {
        printErrors(std::cout);
        std::cout <<"Errors, exiting... \n";
        exit(1);
    }
}



