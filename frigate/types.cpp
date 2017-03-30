//
//  typefunctions.cpp
//  
//
//  Created by Benjamin Mood on 11/19/14.
//
//

#include <stdio.h>
#include "types.h"
#include <string.h>
#include <vector>
#include "error.h"
#include <algorithm>

using namespace std;

Type * isTypeType(Type * t)
{
    if(t->typeType == Type_t)
    {
        return t;
    }
    
    return 0;
}

IntType * isIntType(Type * t)
{
    if(t->typeType == IntType_t)
    {
        return (IntType *)t;
    }
    
    return 0;
}

UIntType * isUIntType(Type * t)
{
    if(t->typeType == UIntType_t)
    {
        return (UIntType *)t;
    }
    
    return 0;
}

StructType * isStructType(Type * t)
{
    if(t->typeType == StructType_t)
    {
        return (StructType *)t;
    }
    
    return 0;
}

NoType * isNoType(Type * t)
{
    if(t->typeType == NoType_t)
    {
        return (NoType *)t;
    }
    
    return 0;
}

FunctionType * isFunctionType(Type * t)
{
    if(t->typeType == FunctionType_t)
    {
        return (FunctionType *)t;
    }
    
    return 0;
}

ConstType * isConstType(Type * t)
{
    if(t->typeType == ConstType_t)
    {
        return (ConstType *)t;
    }
    
    return 0;
}

ArrayType * isArrayType(Type * t)
{
    if(t->typeType == ArrayType_t)
    {
        return (ArrayType *)t;
    }
    
    return 0;
}

VoidType * isVoidType(Type * t)
{
    if(t->typeType == VoidType_t)
    {
        return (VoidType *)t;
    }
    
    return 0;
}

int compareTypeVectors(vector<Type *> & vec1, vector<Type *> & vec2)
{
    
    
    if(vec1.size() != vec2.size())
    {
        return 0;
    }

    
    for(int i=0;i<vec1.size();i++)
    {
        if(!compareTypes(vec1[i],vec2[i]))
        {
            return 0;
        }
    }
    

    
    return 1;
}

int compareTypes(Type * t1, Type * t2)
{
    if(t1 == t2)
    {
        return 1;
    }
    
    if((isConstType(t1) && isIntType(t2)) || (isIntType(t1) && isConstType(t2)))
    {
        return 1;
    }
    
    if((isConstType(t1) && isUIntType(t2)) || (isUIntType(t1) && isConstType(t2)))
    {
        return 1;
    }
    
    
    if(isIntType(t1) && isIntType(t2))
    {
        if(isIntType(t1)->size == isIntType(t2)->size)
        {
            if(t1->typeName.find("-+generated_E") !=std::string::npos || t2->typeName.find("-+generated_E") !=std::string::npos ||t1->typeName.find("-+generate_const_type+-") !=std::string::npos || t2->typeName.find("-+generate_const_type+-") !=std::string::npos || t1->typeName.find("-+generate_uconst_type+-") !=std::string::npos || t2->typeName.find("-+generate_uconst_type+-") !=std::string::npos || t1->typeName.find("-+generate_wire_type+-") !=std::string::npos || t2->typeName.find("-+generate_wire_type+-") !=std::string::npos || t1->typeName.find("-+generated_cond_1_int_type+-") !=std::string::npos| t2->typeName.find("-+generated_cond_1_int_type+-") !=std::string::npos)
            {
                return 1;
            }
            return 2; //warning about type casting
        }
        return 0;
    }
    
    
    if(isUIntType(t1) && isUIntType(t2))
    {
        if(isUIntType(t1)->size == isUIntType(t2)->size)
        {
            if(t1->typeName.find("-+generated_E") !=std::string::npos || t2->typeName.find("-+generated_E") !=std::string::npos ||t1->typeName.find("-+generate_const_type+-") !=std::string::npos || t2->typeName.find("-+generate_const_type+-") !=std::string::npos || t1->typeName.find("-+generate_uconst_type+-") !=std::string::npos || t2->typeName.find("-+generate_uconst_type+-") !=std::string::npos  || t1->typeName.find("-+generate_wire_type+-") !=std::string::npos || t2->typeName.find("-+generate_wire_type+-") !=std::string::npos || t1->typeName.find("-+generated_cond_1_int_type+-") !=std::string::npos| t2->typeName.find("-+generated_cond_1_int_type+-") !=std::string::npos)
            {
                return 1;
            }
            return 2; //warning about type casting
        }
        return 0;
    }
    
    if(isIntType(t1) && isUIntType(t2))
    {
        if(isIntType(t1)->size == isUIntType(t2)->size)
        {
            if(  t1->typeName.find("-+generated_cond_1_int_type+-") !=std::string::npos| t2->typeName.find("-+generated_cond_1_int_type+-") !=std::string::npos)
            {
                return 1;
            }

            return 2; //warning about type casting
        }
        return 0;
    }
    
    if(isUIntType(t1) && isIntType(t2))
    {
        if(isUIntType(t1)->size == isIntType(t2)->size)
        {
            if( t1->typeName.find("-+generated_cond_1_int_type+-") !=std::string::npos| t2->typeName.find("-+generated_cond_1_int_type+-") !=std::string::npos)
            {
                return 1;
            }

            return 2; //warning about type casting
        }
        return 0;
    }
    
    
    if(isArrayType(t1) && isArrayType(t2))
    {
        
        if(isArrayType(t1)->sizes != isArrayType(t2)->sizes)
        {
            return 0;
        }
        
        
        //check base types are equiv;
        if(isArrayType(isArrayType(t1)->type) || isArrayType(isArrayType(t2)->type) )
        {
            cout <<" array type's base type must be non-array type, exiting...";
            exit(1);
        }
        
        if(compareNumTypes(isArrayType(t1)->type,isArrayType(t2)->type))
        {
            if(isIntType(isArrayType(t1)->type) && isIntType(isArrayType(t2)->type))
            {
                if(isIntType(isArrayType(t1)->type)->size == isIntType(isArrayType(t2)->type)->size && isArrayType(t1)->type != isArrayType(t2)->type)
                {
                    return 2;
                }
            }
            if(isUIntType(isArrayType(t1)->type) && isUIntType(isArrayType(t2)->type))
            {
                if(isUIntType(isArrayType(t1)->type)->size == isUIntType(isArrayType(t2)->type)->size && isArrayType(t1)->type != isArrayType(t2)->type)
                {
                    return 2;
                }
            }
            if(isIntType(isArrayType(t1)->type) && isUIntType(isArrayType(t2)->type))
            {
                if(isIntType(isArrayType(t1)->type)->size == isUIntType(isArrayType(t2)->type)->size && isArrayType(t1)->type != isArrayType(t2)->type)
                {
                    return 2;
                }
            }
            if(isUIntType(isArrayType(t1)->type) && isIntType(isArrayType(t2)->type))
            {
                if(isUIntType(isArrayType(t1)->type)->size == isIntType(isArrayType(t2)->type)->size && isArrayType(t1)->type != isArrayType(t2)->type)
                {
                    return 2;
                }
            }
            
            return 1;
        }
        return 0;
    }

    
    return 0;
}

Type * compareNumTypes(Type * t1, Type * t2)
{
    if(t1 == 0 || t2 == 0)
    {
        return 0;
    }
    
    if(t1 == t2)
    {
        return t1;
    }
    
    if(isArrayType(t1) && isArrayType(t2))
    {
        
        if(isArrayType(t1)->sizes != isArrayType(t2)->sizes)
        {
            return 0;
        }
        

        //check base types are equiv;
        if(isArrayType(isArrayType(t1)->type) || isArrayType(isArrayType(t2)->type) )
        {
            cout <<" array type's base type must be non-array type, exiting...";
            exit(1);
        }
        
        if(compareNumTypes(isArrayType(t1)->type,isArrayType(t2)->type))
        {
            return t1;
        }
        return 0;
    }

    if((isConstType(t1) && isIntType(t2)))
    {
        return isIntType(t2);
    }

    if( (isIntType(t1) && isConstType(t2)))
    {
        return isIntType(t1);
    }
    
    if((isConstType(t1) && isUIntType(t2)))
    {
        return isUIntType(t2);
    }
    
    if( (isUIntType(t1) && isConstType(t2)))
    {
        return isUIntType(t1);
    }
    
    if(isIntType(t1) && isIntType(t2))
    {
        if(isIntType(t1)->size == isIntType(t2)->size)
        {
            return t1;
        }
        return 0;
    }
    
    if(isUIntType(t1) && isUIntType(t2))
    {
        if(isUIntType(t1)->size == isUIntType(t2)->size)
        {
            return t1;
        }
        return 0;
    }
    
    if(isIntType(t1) && isUIntType(t2))
    {
        if(isIntType(t1)->size == isUIntType(t2)->size)
        {
            return t1;
        }
        return 0;
    }
    
    if(isUIntType(t1) && isIntType(t2))
    {
        if(isUIntType(t1)->size == isIntType(t2)->size)
        {
            return t1;
        }
        return 0;
    }
    
    return 0;
}

Type * noType=0;

Type * getNoType()
{
    if(noType == 0)
    {
        noType = new NoType();
    }
    
    return noType;
}


Type * voidType=0;

Type * getVoidType()
{
    if(voidType == 0)
    {
        voidType = new VoidType();
    }
    
    return voidType;
}

Type * constType=0;

Type * getConstType()
{
    if(constType == 0)
    {
        constType = new ConstType();
    }
    
    return constType;
}


TypeMap  type_map;

TypeMap & getTypeMap(){return type_map;}

void printTypes()
{
    cout <<"--- Types ---\n";
    for ( std::unordered_map<string,Type *>::iterator it = getTypeMap().begin(); it!= getTypeMap().end(); ++it )
    {
        if(it->second == 0)
        {
            continue;
        }
        
        if(isIntType((Type *)it->second))
        {
            std::cout << "IntType: ";
            ((Type *)it->second)->print(cout,0);
            cout <<"\n";
        }
        else if(isUIntType((Type *)it->second))
        {
            std::cout << "UIntType: ";
            ((Type *)it->second)->print(cout,0);
            cout <<"\n";
        }
        else if(isStructType((Type *)it->second))
        {
            std::cout << "StructType: " << it->first << ": ";
            ((Type *)it->second)->print(cout,0);
        }
        else if(isFunctionType((Type *)it->second))
        {
            std::cout << "FunctionType: ";
            ((Type *)it->second)->print(cout,0);
            cout <<"\n";
        }
        
        
    }
    cout <<"\n";
}

#include "variable.h"

int compareTypeAndVar(Variable * v, Type * t)
{
    
    if(v->type == t)
    {
        return 1; //almost every case
    }
    
    if(isConstType(t) && isIntType(v->type))
    {
        return 1;
    }
    
    if(isConstType(t) && isUIntType(v->type))
    {
        return 1;
    }
    
    if(isArrayType(t) && isArrayType(v->type))
    {
        return compareTypes(t,v->type);
        //cout <<"comparing types:";
        if(compareNumTypes(t,v->type))
        {
            return 1;
        }
        else
        {
            return 0;
        }
       
    }
    
    if(isIntType(t) && isIntType(v->type))
    {
        return compareTypes(t,v->type);
        if(compareNumTypes(t,v->type))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    
    if(isUIntType(t) && isUIntType(v->type))
    {
        return compareTypes(t,v->type);
        if(compareNumTypes(t,v->type))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    
    
    if(isIntType(t) && isUIntType(v->type))
    {
        return compareTypes(t,v->type);
        if(compareNumTypes(t,v->type))
        {
            return 2;
        }
        else
        {
            return 0;
        }
    }
    
    if(isUIntType(t) && isIntType(v->type))
    {
        return compareTypes(t,v->type);
        if(compareNumTypes(t,v->type))
        {
            return 2;
        }
        else
        {
            return 0;
        }
    }
    
    
    return 0;
}

void CheckForRecTypes()
{
    for ( std::unordered_map<string,Type *>::iterator it = getTypeMap().begin(); it!= getTypeMap().end(); ++it )
    {
        if(it->second == 0)
        {
            continue;
        }
        
        if(isStructType((Type *)it->second))
        {
            vector<Type *> v;
            checkRecursiveStructure(((Type *)it->second),v);
        }
    }
}


#include "ast.h"

void checkRecursiveStructure(Type * t, vector<Type * >  vec)
{
    if(isStructType(t) && !vec.empty() && std::find(vec.begin(), vec.end(), t) != vec.end())
    {
        addError(t->wheredefined->nodeLocation->fname,t->wheredefined->nodeLocation->position,"Recursion found in Type: "+t->typeName+". Recursion is not allowed in types.");
        return;
    }
    else
    {
        vec.push_back(t);
    }
    
    
    if(isStructType(t))
    {
        StructType * st = isStructType(t);
        
        for (std::unordered_map<string,StructItem *>::iterator it = st->si.begin(); it!= st->si.end(); ++it)
        {
            StructItem * sItem =  (StructItem *)(it->second);
            
            checkRecursiveStructure(sItem->type,vec);
        }
        return;
    }
    else
    {
        return;
    }
}



