#include "ast.h"
#include "exprtest.hh"

#include <unordered_map>

#ifndef _includes_h
#define _includes_h

using namespace std;

string getFilePrefix(string fname);
void includeIncludes(Node * topnode,string originalfilename);

#endif
