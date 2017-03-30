//
//  typegenerate.h
//  
//
//  Created by Benjamin Mood on 11/19/14.
//
//

#ifndef _typegenerate_h
#define _typegenerate_h

#include "ast.h"
#include "types.h"
#include "traverse.h"
#include <string.h>
#include "error.h"
#include "variable.h"

#include <unordered_map>


class type_context
{
public:
    bool sawminus;
    
    type_context()
    {
        sawminus=false;
    }
};


class TermWarningTraverse : public TopDownTraverse <type_context>
{
public:
    TermWarningTraverse()
    {}
    
    type_context visit(Node * n, type_context tc)
    {

        if(isUnaryPreMinusMinusNode(n))
        {
            tc.sawminus=true;
        }
        if(isUnaryPostMinusMinusNode(n))
        {
            tc.sawminus=true;
        }
        if(isUnaryMinusNode(n))
        {
            tc.sawminus=true;
        }
        if(isArithMinusNode(n))
        {
            tc.sawminus=true;
        }
        
        n->hasMinus = tc.sawminus;
        return tc;
    }
};



//function def vars
//for loops
//var defines
//global defines
//function def names

void generateTypes(Node * topnode);



#endif
