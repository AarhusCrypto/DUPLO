//
//  types.h
//  
//
//  Created by Benjamin Mood on 11/17/14.
//
//


#ifndef _types_h
#define _types_h

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "error.h"


using namespace std;


enum Type_Enum {Type_t, IntType_t, StructType_t, NoType_t, FunctionType_t,VoidType_t, ConstType_t, ArrayType_t,UIntType_t};


class Node;

class Type
{
    
public:
    string typeName;
    string nodeType;
    
    Type_Enum typeType;
    
    Node * wheredefined;
    
    virtual ~Type()
    {
    }
    
    explicit Type()
    {
        nodeType = "Type";
        typeName = "Type";
        typeType = Type_t;
    }
    
    virtual void print(std::ostream &os, unsigned int depth=0, int xoptions=0)  = 0;
    
    
    static inline std::string indent(int d)
    {
        if(d < 0)
        {
            d = 0;
        }
        return std::string(d * 2, ' ');
    }
};

class IntType : public Type
{
public:
    
    long size;
    
    virtual ~IntType()
    {
        
    }
    
    explicit IntType(string _name, long _size)
    {
        typeName = _name;
        size = _size;
        nodeType = "IntType";
        
        typeType = IntType_t;
    }
    
    virtual void print(std::ostream &os, unsigned int depth, int xoptions)
    {
        if(xoptions == 2 || xoptions == 1)
        {
            if(typeName.find("-+generate_wire_type+-") !=std::string::npos)
            {
                os <<indent(depth)<< "<"<<nodeType <<"> size="<<size;
            }
            else
            {
                os <<indent(depth)<< "<"<<nodeType <<"> "<< typeName;
            }
        }
        else
        {
            os <<indent(depth)<< "\""<<typeName <<"\"  of size "<< size;
        }
    }
    
};

class UIntType : public Type
{
public:
    
    long size;
    
    virtual ~UIntType()
    {
        
    }
    
    explicit UIntType(string _name, long _size)
    {
        typeName = _name;
        size = _size;
        nodeType = "UIntType";
        
        typeType = UIntType_t;
    }
    
    virtual void print(std::ostream &os, unsigned int depth, int xoptions)
    {
        
        /*if(xoptions == 1)
        {
            os <<indent(depth)<< ""<<nodeType <<" size "<< size;
        }
        else */
        if(xoptions == 2 || xoptions == 1)
        {
            if(typeName.find("-+generate_wire_type+-") !=std::string::npos)
            {
                os <<indent(depth)<< "<"<<nodeType <<"> size="<<size;
            }
            else
            {
                os <<indent(depth)<< "<"<<nodeType <<"> "<< typeName;
            }
        }
        else
        {
            os <<indent(depth)<< "\""<<typeName <<"\"  of size "<< size;
        }
    }
    
};

class ArrayType : public Type
{
public:
    
    vector<long> sizes;
    Type * type;
    
    virtual ~ArrayType()
    {
        
    }
    
    explicit ArrayType(string _name)
    {
        typeName = _name;
        nodeType = "ArrayType";
        typeType = ArrayType_t;
    }
    
    virtual void print(std::ostream &os, unsigned int depth, int xoptions)
    {
        if(xoptions == 1)
        {
            os << type->typeName;
            for(int i=0;i<sizes.size();i++)
            {
                os<<"["+to_string(sizes[i])+"]";
            }
            return;
        }
        
        os <<indent(depth)<< typeName;
        
        for(int i=0;i<sizes.size();i++)
        {
            os<<"["+to_string(sizes[i])+"]";
        }
    }
    
};

class ConstType : public Type
{
public:

    virtual ~ConstType()
    {
        
    }
    
    explicit ConstType()
    {
        typeName = "Constant";
        nodeType = "ConstType";
        
        typeType = ConstType_t;
    }
    
    virtual void print(std::ostream &os, unsigned int depth, int xoptions)
    {
        os <<indent(depth)<< "Constant";
    }
    
};


class StructItem
{
public:
    StructItem(){}
    
    string name;
    //vector<long> arraySizes;
    Type * type;
};

class StructType : public Type
{
public:
    
    unordered_map<string,StructItem *> si;
    
    virtual ~StructType()
    {
        for ( std::unordered_map<string,StructItem *>::iterator it = si.begin(); it!= si.end(); ++it )
        {
            delete it->second;
        }
    }
    
    void addType(StructItem * _si, string lineno, string file)
    {
        if(si[_si->name] != 0)
        {
            addError(file,lineno,"Redefinition of variable \"" + _si->name + "\" in struct type \""+typeName+"\"");
        }
        
        si[_si->name] = _si;
    }
    
    explicit StructType(string _name)
    {
        typeName = _name;
        nodeType = "StructType";
        
        typeType = StructType_t;
    }
    
    virtual void print(std::ostream &os, unsigned int depth, int xoptions)
    {
        
        if(xoptions == 1)
        {
            os<< "struct_t "<<typeName;
            return;
        }
        
        os <<"\n{\n";
        for ( std::unordered_map<string,StructItem *>::iterator it = si.begin(); it!= si.end(); ++it )
        {
            StructItem * sItem =  (StructItem *)(it->second);
            
            //if(sItem->arraySizes.size() == 0)
            {
                sItem->type->print(os,1);
                os << " "<<sItem->name;
                os <<"\n";
            }
           /* else
            {
                
                sItem->type->print(os,1);
                os << " "<<sItem->name<<" ";
                for(int j=0;j<sItem->arraySizes.size();j++)
                {
                    os <<"[" <<sItem->arraySizes[j]<<"]";
                }
                os <<"\n";
            }*/
            
        }
        os <<"}\n";
    }
};

class NoType : public Type
{
public:
    virtual ~NoType(){}
    explicit NoType()
    {
        typeName = "NOTYPE";
        nodeType = "NoType";
        typeType = NoType_t;
    }
    
    virtual void print(std::ostream &os, unsigned int depth, int xoptions) {os<<"NoType (ErrorType)";}
};

class VoidType : public Type
{
public:
    virtual ~VoidType(){}
    explicit VoidType()
    {
        typeName = "void";
        nodeType = "VoidType";
        typeType = VoidType_t;
    }
    
    virtual void print(std::ostream &os, unsigned int depth, int xoptions) {os<<"VOIDTYPE";}
};


ArrayType * isArrayType(Type * t);

class FunctionType : public Type
{
public:
    
    Type * type;
    
    vector<Type *> param;
    
    virtual ~FunctionType()
    {
        
    }
    
    explicit FunctionType(string _name, Type * _type)
    {
        typeName = _name;
        type = _type;
        nodeType = "FunctionType";
        
        typeType = FunctionType_t;
    }
    
    void addParam(Type * t)
    {
        param.push_back(t);
    }
    
    virtual void print(std::ostream &os, unsigned int depth, int xoptions)
    {
        os <<indent(depth)<< typeName <<"(";
        for(int i=0; i<param.size(); i++)
        {
            
            if(i!= 0)
            {
                os<<", ";
            }
            os<<param[i]->typeName;
            
            if(isArrayType(param[i]))
            {
                for(int j=0; j<((ArrayType *) param[i])->sizes.size();j++)
                {
                    os<<"["+to_string(((ArrayType *) param[i])->sizes[j])+"]";
                }
            }
            
        }
        
        os <<") -> "<<type->typeName;
    }
    
};



#include <unordered_map>

Type * isTypeType(Type * t);

IntType * isIntType(Type * t);

UIntType * isUIntType(Type * t);

StructType * isStructType(Type * t);

NoType * isNoType(Type * t);

FunctionType * isFunctionType(Type * t);
VoidType * isVoidType(Type * t);

ConstType * isConstType(Type * t);

int compareTypes(Type * t1, Type * t2);

Type * compareNumTypes(Type * t1, Type * t2);

class Variable;

typedef std::unordered_map<string,Variable *> VariableContext;
typedef std::unordered_map<string,Type *> TypeMap;

TypeMap & getTypeMap();

Type * getNoType();
Type * getVoidType();
Type * getConstType();

int compareTypeAndVar(Variable *v, Type * t);
int compareTypeVectors(vector<Type *> & vec1, vector<Type *> & vec2);
void printTypes();

void CheckForRecTypes();
void checkRecursiveStructure(Type * t, vector<Type * >  vec);

#endif



