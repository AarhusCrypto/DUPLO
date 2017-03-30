//
//  variable.h
//  
//
//  Created by Benjamin Mood on 11/23/14.
//
//

#ifndef _variable_h
#define _variable_h


#include <vector>
#include <string>
#include "types.h"
#include "wire.h"
#include "wirepool.h"


class Node;

using namespace std;

enum Variable_Enum {Intv, Vv, Arrayv, Structv, Functionp};


class Variable;

//circuit output return value
class CORV
{
public:
    vector<Wire *> vec;
    Variable * var;
    CORV(){var = 0;}
};

class WireHolderNode
{
public:
    vector<Wire *> * vec;
    WireHolderNode * node;
    WireHolderNode * next;
    Variable * coresponding_var;
    int startwirenum;
    int endwirenum;
    
    
    
    vector<WireHolderNode *> * vec_whn;
    
    WireHolderNode(Variable * n_in)
    {
        vec = 0;
        node = 0;
        next = 0;
        coresponding_var = n_in;
        vec_whn = 0;
    }
    
    ~WireHolderNode()
    {
        if(node != 0)
        {
            delete node;
        }
        if(vec != 0)
        {
            delete vec;
        }
        if(next != 0)
        {
            delete next;
        }
    }
};

void setWire(int i, WireHolderNode * n, Wire * w);
Wire * getWire(int i, WireHolderNode * n);
long setSizes(WireHolderNode * n, int count);

class Variable
{
protected:
    int depth;
public:
    //vector<long> arrays;
    bool istemp;
    bool isconst;
    bool isfunctioncallvar;
    string name;
    Type * type;
    Node * nodeOfName;
    int wirebase; //wirebase relative to the permanant wires
    int relativewirebase; //wirebase relative to the size of the variable
    
    int getVarDepth()
    {
        return depth;
    }
    
    WireSet * wireset;
    
    
    long csize;
    
    WireHolderNode * wv;
    
    Variable(){v_enum = Vv; csize=-1; wv = 0; istemp=false; isconst =false; depth=-1; isfunctioncallvar=false; wireset=0;}
    
    Variable_Enum v_enum;
    
    void print(std::ostream &os)
    {
        os <<type->typeName << " "<<name<<"";
        /*for(int j=0;j<arrays.size();j++)
        {
            os <<"[" <<arrays[j]<<"]";
        }*/
    }
    
    virtual long size()
    {
        return 0;
    }
    
    virtual void FillInType(bool usePool)
    {
        
    }
    
    virtual long FillInWires(WireSet * ws, long wirestarplace, int idx)
    {
        return 0;
    }
    
    virtual long assignPermWires(long l)
    {
        return l;
    }
    
    virtual WireHolderNode * getWires()
    {
        
        return wv;
    }
    
    virtual void deleteTemps()
    {
        
    }
    
    virtual void setFunctionCallVar()
    {
        
    }
    
    virtual void FillInDepth(int depth_in)
    {
        depth = depth_in;
    }
    
    virtual void printWithWires(std::ostream &os, string prefix, int start)
    {
    }
};

class IntVariable : public Variable
{
public:
    IntVariable(){v_enum = Intv; csize=-1; istemp=false; isconst =false; depth=-1; isfunctioncallvar=false; wireset=0;}
    
    vector<Wire *> wires;
    

    long size()
    {
        IntType * t = isIntType(type);
        
        if(t == 0)
        {
            return isUIntType(type)->size;
        }
        
        return t->size;
    }
    
    virtual void FillInType(bool usePool);
	virtual long FillInWires(WireSet * ws, long wirestarplace, int idx);
    
    long assignPermWires(long l);
    
    WireHolderNode * getWires()
    {
        return wv;
    }
    
    virtual void FillInDepth(int depth_in)
    {
        for(int i=0;i<wires.size();i++)
        {
            wires[i]->thisvar = this;
        }
        depth = depth_in;
    }
    
    virtual void printWithWires(std::ostream &os, string prefix, int start)
    {
        os <<prefix<<name<<" "<<(wirebase-start)<<" to "<<((wirebase+size())-start-1)<<"\n";
    }
    
    virtual void setFunctionCallVar()
    {
        isfunctioncallvar = true;
    }
};

class ArrayVariable : public Variable
{
public:
    ArrayVariable(){v_enum = Arrayv; istemp=false; isconst =false; depth=-1; isfunctioncallvar=false; wireset=0;}
    
    vector<Variable *> av;

    long size()
    {
        if(csize != -1)
        {
            return csize;
        }
        
        long sizer = 0;
        
        for(int i=0;i<av.size();i++)
        {
            sizer+=av[i]->size();
        }
        
        csize = sizer;
        return sizer;
    }
    
    virtual void FillInType(bool usePool);
    
	virtual long FillInWires(WireSet * ws, long wirestarplace, int idx);
    
    virtual ~ArrayVariable()
    {
        if(istemp)
            return;
        
        for(int i=0;i<av.size();i++)
        {
            if(av[i])
            {
                delete av[i];
            }
        }
    }
    long assignPermWires(long l);
    
    WireHolderNode * getWires()
    {
        return wv;
    }
    
    void deleteTemps()
    {
        for(int i=0;i<av.size();i++)
        {
            if(av[i]->istemp)
            {
                
                av[i]->deleteTemps();
                delete av[i];
                av[i] = 0;
                
            }
        }
    }
    
    virtual void FillInDepth(int depth_in)
    {
        depth = depth_in;
        for(int i=0;i<av.size();i++)
        {
            if(av[i] != 0)
            {
                av[i]->FillInDepth(depth_in);
            }
        }
    }
    
    virtual void printWithWires(std::ostream &os, string prefix, int start)
    {
        for(int i=0;i<av.size();i++)
        {
            if(av[i] != 0)
            {
                av[i]->printWithWires(os,prefix+"["+to_string(i)+"]",start);
            }
        }
    }
    
    virtual void setFunctionCallVar()
    {
        isfunctioncallvar = true;
        
        for(int i=0;i<av.size();i++)
        {
            if(av[i] != 0)
            {
                av[i]->setFunctionCallVar();
            }
        }
        
    }
};

class StructVariable : public Variable
{
public:
    StructVariable(){v_enum = Structv; csize=-1; istemp=false; isconst =false; depth=-1; isfunctioncallvar=false; wireset=0;}
    
    unordered_map<string, Variable *> map;

    
    long size()
    {
        if(csize != -1)
        {
            return csize;
        }
        
        long sizer=0;
        for (std::unordered_map<string,Variable *>::iterator it = map.begin(); it!= map.end(); ++it )
        {
            Variable * v =  (Variable *)(it->second);
            sizer += v->size();
        }
        
        csize = sizer;
        return sizer;
    }
    
    virtual void FillInType(bool usePool);
	virtual long FillInWires(WireSet * ws, long wirestarplace, int idx);
    
    virtual ~StructVariable()
    {
        if(istemp)
            return;
        
        for (std::unordered_map<string,Variable *>::iterator it = map.begin(); it!= map.end(); ++it )
        {
            Variable * v =  (Variable *)(it->second);
            
            if(v != 0)
            {
                delete v;
            }
        }
        
    }
    long assignPermWires(long l);
    
    WireHolderNode * getWires()
    {
        return wv;
    }
    
    void deleteTemps()
    {
        for (std::unordered_map<string,Variable *>::iterator it = map.begin(); it!= map.end(); ++it )
        {
            if(istemp)
            {
                Variable * v =  (Variable *)(it->second);
                
                v->deleteTemps();
                delete v;
                map[it->first]=0;
            }
        }
    }
    
    virtual void FillInDepth(int depth_in)
    {
        depth = depth_in;
        
        for (std::unordered_map<string,Variable *>::iterator it = map.begin(); it!= map.end(); ++it )
        {
            Variable * v =  (Variable *)(it->second);
            
            if(v != 0)
            {
                v->FillInDepth(depth_in);
            }
        }
    }
    
    virtual void printWithWires(std::ostream &os, string prefix, int start)
    {
        prefix = prefix + name+".";
        
        for (std::unordered_map<string,Variable *>::iterator it = map.begin(); it!= map.end(); ++it )
        {
            Variable * v =  (Variable *)(it->second);
            
            if(v != 0)
            {
                v->printWithWires(os,prefix + (string)(it->first),start);
            }
        }
    }
    
    virtual void setFunctionCallVar()
    {
        isfunctioncallvar = true;
        
        for (std::unordered_map<string,Variable *>::iterator it = map.begin(); it!= map.end(); ++it )
        {
            Variable * v =  (Variable *)(it->second);
            
            if(v != 0)
            {
                v->setFunctionCallVar();
            }
        }
        
    }
};

class FunctionVariable : public Variable
{
public:
    WireHolderNode * args;
    WireHolderNode * return_var;
    
    vector<Variable *> argsv;
    Variable * returnv;
    
    long rsize;
    long psize;
    
    int functionNumber;
    
    Node * functionNode;
    
    FunctionVariable()
    {
        csize=-1;
        rsize=-1;
        psize=-1;
        v_enum = Functionp;
        return_var=0;
        args=0;
        returnv = 0;
        istemp=false;
        isconst =false;
        functionNode=0;
        isfunctioncallvar=false;
        
        wireset=0;
    }
    
    WireHolderNode * getWires()
    {
        return 0;
    }
    
    //total
    long size()
    {
        if(csize != -1)
        {
            return csize;
        }
        
        long sizer = 0;
        for(int i=0;i<argsv.size();i++)
        {
            sizer+= argsv[i]->size();
        }
        
        if(returnv != 0)
        {
            sizer+= returnv->size();
        }
        
        csize = sizer;
        
        return sizer;
    }
    
    long sizeparam()
    {
        if(psize != -1)
        {
            return psize;
        }
        
        long sizer=0;
        
        for(int i=0;i<argsv.size();i++)
        {
            sizer+= argsv[i]->size();
            
        }
        
        psize = sizer;
        return sizer;
    }
    
    long sizereturn()
    {
        if(rsize != -1)
        {
            return rsize;
        }
        
        long sizer=0;
        
        if(returnv != 0)
        {
            sizer+= returnv->size();
        }
        //cout << returnv;
        
        rsize = sizer;
        return sizer;
    }
    
    long assignPermWires(long l);
    
    virtual void FillInType(bool usePool);
	virtual long FillInWires(WireSet * ws, long wirestarplace, int idx);
    
    virtual ~FunctionVariable()
    {
        if(args!= 0)
        {
            delete args;
        }
        if(return_var != 0)
        {
            delete return_var;
        }
        if(returnv != 0)
        {
            delete returnv;
        }
        
        for(int i=0;i<argsv.size();i++)
        {
           delete argsv[i];
        }
    }
};


#endif















