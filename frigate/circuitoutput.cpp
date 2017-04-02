//
//  CircuitOutput.cpp
//  
//
//

#include "wirepool.h"
#include "circuitoutput.h"
#include <string>

using namespace std;

bool useTinyOutput = false;

//duplo
bool isPrintDuploGC = false;
string strDuploGC;
vector<string> strDuploFn;
ofstream fDuploGC;
vector<string> strDuploZeroOne;
bool isMainFunc = false;
uint32_t functions_call_in_main = 0;

void printDuploGC(bool value)
{
	isPrintDuploGC = value;
}

void appendDuploGC(string value, bool cond)
{
	if(isPrintDuploGC && cond)
		strDuploGC.append(value);
	
}

bool isMainFunction()
{
	return isMainFunc;
}
void setTinyFiles(bool in)
{
    useTinyOutput = in;
}

bool getIsTiny()
{
    return useTinyOutput;
}

vector<int>currentbasewire;
int currentbasewiremain;
int maxWireValue;
int functionWireStart;

#include "traverse.h"

unordered_map<string, Node *> functionNameToNode;

vector < Wire *> ONE_WIRE;
vector < Wire *> ZERO_WIRE;

vector<unsigned long> one_wire_l;
vector<unsigned long> zero_wire_l;

class gatherFunctionCallTraverse : public BottomUpTraverse <vector<Node *> >
{
public:
    gatherFunctionCallTraverse()
    {
    }
    
    vector<Node *> visit(Node * n, vector<vector<Node *> > functioncalls_in)
    {
        FunctionCallNode * fcn = isFunctionCallNode(n);
        
        
        vector<Node *> functioncallsout;
        
        for(int i=0;i<functioncalls_in.size();i++)
        {
            for(int j=0;j<functioncalls_in[i].size();j++)
            {
                functioncallsout.push_back(functioncalls_in[i][j]);
            }
        }
        
        if(fcn != 0)
        {
            functioncallsout.push_back(functionNameToNode[isTermNode(fcn->function_name)->var]);
        }
        
        return functioncallsout;
    }
};

class FindIfContainsProcs : public BottomUpTraverse <bool>
{
public:
    FindIfContainsProcs()
    {
    }
    
    bool visit(Node * n, vector<bool > bool_in)
    {
        
        bool cvalue = false;
        
        for(int i=0;i<bool_in.size();i++)
        {
            cvalue = cvalue | bool_in[i];
        }
        
        if(isForNode(n))
        {
            isForNode(n)->has_procs = cvalue;
        }
        if(isCompoundStatementNode(n))
        {
            if(isCompoundStatementNode(n)->isproc)
                return true;
        }
        
        n->has_procs = cvalue;
        
        return cvalue;
    }
};




vector<WirePool> pool;

WirePool * getPool(int idxF)
{
	return &pool[idxF];
}

int co_depth = 0;

int getDepth()
{
    return co_depth;
}
void increaseDepth()
{
    co_depth++;
}
void decreaseDepth()
{
    co_depth--;
}


string removeFilePrefix(string fname)
{
    int pos = fname.find_last_of("/\\");
    
    if(pos == -1)
    {
        return fname;
    }
    
    return fname.substr(pos+1,fname.length()-pos+1);
}

//this function fills any extra space with 0s -> this should really only be needed for constants as type checking will force it to be correct otherwise.
void ensureSameSize(vector<Wire *> & w1, vector<Wire *> & w2, int idxF)
{
    while(w1.size() < w2.size())
    {
        w1.push_back(get_ZERO_WIRE(idxF));
    }
    while(w1.size() > w2.size())
    {
        w2.push_back(get_ZERO_WIRE(idxF));
    }
}

void ensureTypedSize(vector<Wire *> & w1, vector<Wire *> & w2, Type * t, int idxF)
{
    int l;
    
    //for untypechecked nodes - uncommenting this hides some possible errors
    /*if(t == 0)
    {
        ensureSameSize(w1,w2);
        return;
    }
    else*/ if(isIntType(t))
    {
        l = isIntType(t)->size;
    }
    else if(isUIntType(t))
    {
        l = isUIntType(t)->size;
    }
    else if(isConstType(t))
    {
	    ensureSameSize(w1, w2, idxF);
        return;
    }
    else
    {
	    ensureSameSize(w1, w2, idxF);
        return;
    }
    
	ensureSize(w1, l, idxF);
	ensureSize(w2, l, idxF);
}

void ensureTypedSize(vector<Wire *> & w1, Type * t, int idxF)
{
    int l;
    
    //for untypechecked nodes - uncommenting this hides some possible errors
    /*if(t == 0)
     {
     ensureSameSize(w1,w2);
     return;
     }
     else*/ if(isIntType(t))
     {
         l = isIntType(t)->size;
     }
     else if(isUIntType(t))
     {
         l = isUIntType(t)->size;
     }
     else
     {
         cout << "ensureTypedSize should only be used with int and uint types! (but works with constant types)";
         cout << "\nused with: ";
         t->print(cout,0);
         cout <<"\n";
         exit(1);
     }
    
	ensureSize(w1, l, idxF);
}

void ensureSize(vector<Wire *> & w1,int length, int idxF)
{
    if(w1.size() == length)
    {
        return;
    }
    while(w1.size() < length)
    {
        w1.push_back(get_ZERO_WIRE(idxF));
    }
    if(w1.size() > length)
    {
        for(int i=w1.size()-1;i>=length;i--)
        {
            w1[i]->locked = false;
        }
        w1.resize(length);
    }
}

void ensureIntVariableToVec(CORV & c)
{
    if(c.var == 0)
    {
        return;
    }

    if(!(c.var->v_enum == Intv))
    {
        cout << "strange: variable is not integer\n";
        return;
    }
    
    IntVariable * v = (IntVariable *) c.var;
    
    c.vec.resize(v->wires.size());
    
    for(int i=0;i<v->wires.size();i++)
    {
        c.vec[i] = v->wires[i];
    }
}

void ensureAnyVariableToVec(CORV & c)
{
    if(c.var == 0)
    {
        return;
    }
    
    if(c.var->v_enum == Intv)
    {
        ensureIntVariableToVec(c);
        return;
    }
    
    Variable * v = (Variable *) c.var;
    
    c.vec.resize(v->size());
    
    int startplace = v->wv->startwirenum;
    
    for(int i=startplace;i<v->size()+startplace;i++)
    {
        c.vec[i-startplace] = getWire(i,v->wv);
    }
}

void messyUnlock(Variable * cvar)
{
    if(cvar->v_enum == Intv)
    {
        IntVariable * intv = (IntVariable *) cvar;
        
        for(int i=0;i<intv->wires.size();i++)
        {
            intv->wires[i]->locked = false;
        }
    }
    else if(cvar->v_enum == Arrayv)
    {
        ArrayVariable * arrayv = (ArrayVariable *) cvar;
        
        for(int i=0;i<arrayv->av.size();i++)
        {
            messyUnlock(arrayv->av[i]);
        }
    }
}

void messyAssignAndCopy(Variable * cvar, Variable * pattern , int idxF)
{
    if(cvar->v_enum == Intv)
    {
        IntVariable * intv = (IntVariable *) cvar;
        IntVariable * dest = (IntVariable *) pattern;
        
        int intvsize=intv->wires.size();
        
        for(int i=0;i<dest->wires.size();i++)
        {
            if(i < intvsize)
            {
	            assignWire(dest->wires[i], intv->wires[i],  idxF);
	            makeWireContainValueNoONEZEROcopy(dest->wires[i], idxF);
            }
            else
            {
	            assignWire(dest->wires[i], get_ZERO_WIRE(idxF), idxF);
            }
        }
    }
    else if(cvar->v_enum == Arrayv)
    {
        ArrayVariable * arrayv = (ArrayVariable *) cvar;
        ArrayVariable * dest = (ArrayVariable *) pattern;
        
        for(int i=0;i<dest->av.size();i++)
        {
	        messyAssignAndCopy(arrayv->av[i], dest->av[i], idxF);
        }
    }
    else if(cvar->v_enum == Structv)
    {
        StructVariable * structv = (StructVariable *) cvar;
        StructVariable * dest = (StructVariable *) pattern;
        
        for (std::unordered_map<string,Variable *>::iterator it = structv->map.begin(); it!= structv->map.end(); ++it )
        {
            Variable * v =  (Variable *)(it->second);
	        messyAssignAndCopy(v, dest->map[(string)(it->first)], idxF);
        }
    }
    else
    {
        cout << "undefined type in messy messyAssignAndCopy\n";
    }
}

void messyAssignAndCopyForFn(Variable * cvar, Variable * pattern, int idxF)
{
	if (cvar->v_enum == Intv)
	{
		IntVariable * intv = (IntVariable *) cvar;
		IntVariable * dest = (IntVariable *) pattern;
        
		int intvsize = intv->wires.size();
        
		for (int i = 0; i < dest->wires.size(); i++)
		{
			if (i < intvsize)
			{
				assignWire(dest->wires[i], intv->wires[i], idxF);
				makeWireContainValueNoONEZEROcopyForFn(dest->wires[i], idxF);
			}
			else
			{
				assignWire(dest->wires[i], get_ZERO_WIRE(idxF), idxF);
			}
		}
	}
	else if (cvar->v_enum == Arrayv)
	{
		ArrayVariable * arrayv = (ArrayVariable *) cvar;
		ArrayVariable * dest = (ArrayVariable *) pattern;
        
		for (int i = 0; i < dest->av.size(); i++)
		{
			messyAssignAndCopyForFn(arrayv->av[i], dest->av[i], idxF);
		}
	}
	else if (cvar->v_enum == Structv)
	{
		StructVariable * structv = (StructVariable *) cvar;
		StructVariable * dest = (StructVariable *) pattern;
        
		for (std::unordered_map<string, Variable *>::iterator it = structv->map.begin(); it != structv->map.end(); ++it)
		{
			Variable * v =  (Variable *)(it->second);
			messyAssignAndCopyForFn(v, dest->map[(string)(it->first)], idxF);
		}
	}
	else
	{
		cout << "undefined type in messy messyAssignAndCopy\n";
	}
}

//messyCopy assumes the wv in c.var are not correct and does not use them.
//also assumes pattern is correctly constructed and c.var is patterned after pattern correctly BUT the individual variables might be incorrect lengths (i.e. are from constants)
void messyAssignAndCopy(CORV & c, Variable * pattern, int idxF)
{
    if (c.var == 0)
    {
        //IntVariable * intv = (IntVariable *) c.var;
        IntVariable * dest = (IntVariable *) pattern;
        
        int intvsize=c.vec.size();
        
        for(int i=0;i<dest->wires.size();i++)
        {
            if(i < intvsize)
            {
	            assignWire(dest->wires[i], c.vec[i], idxF);
	            makeWireContainValueNoONEZEROcopy(dest->wires[i], idxF);
            }
            else
            {
	            assignWire(dest->wires[i], get_ZERO_WIRE(idxF), idxF);
            }
        }
        return;
    }
    
    if(c.var->v_enum == Intv)
    {
        IntVariable * intv = (IntVariable *) c.var;
        IntVariable * dest = (IntVariable *) pattern;
        
        int intvsize=intv->wires.size();
        
        for(int i=0;i<dest->wires.size();i++)
        {
            if(i < intvsize)
            {
	            assignWire(dest->wires[i], intv->wires[i], idxF);
	            makeWireContainValueNoONEZEROcopy(dest->wires[i], idxF);
            }
            else
            {
	            assignWire(dest->wires[i], get_ZERO_WIRE(idxF), idxF);
            }
        }
    }
    else if(c.var->v_enum == Arrayv)
    {
        ArrayVariable * arrayv = (ArrayVariable *) c.var;
        ArrayVariable * dest = (ArrayVariable *) pattern;
        
        for(int i=0;i<dest->av.size();i++)
        {
	        messyAssignAndCopy(arrayv->av[i], dest->av[i], idxF);
        }
    }
    else if(c.var->v_enum == Structv)
    {
        StructVariable * structv = (StructVariable *) c.var;
        StructVariable * dest = (StructVariable *) pattern;
        
        for (std::unordered_map<string,Variable *>::iterator it = structv->map.begin(); it!= structv->map.end(); ++it )
        {
            Variable * v =  (Variable *)(it->second);
	        messyAssignAndCopy(v, dest->map[(string)(it->first)], idxF);
        }
    }
    else
    {
        cout << "undefined type in messy messyAssignAndCopy\n";
    }
}
string messyAssignAndCopyForFn(CORV & c, Variable * pattern, int idxF)
{
	string rs;
	if (c.var == 0)
	{
	    //IntVariable * intv = (IntVariable *) c.var;
		IntVariable * dest = (IntVariable *) pattern;
        
		int intvsize = c.vec.size();
        
		for (int i = 0; i < dest->wires.size(); i++)
		{
			if (i < intvsize)
			{
				assignWire(dest->wires[i], c.vec[i], idxF);
				makeWireContainValueNoONEZEROcopyForFn(dest->wires[i], idxF);
				rs.append(to_string(dest->wires[i]->wireNumber) + " ");
			}
			else
			{
				assignWire(dest->wires[i], get_ZERO_WIRE(idxF), idxF);
			}
		}
		return rs;
	}
    
	if (c.var->v_enum == Intv)
	{
		IntVariable * intv = (IntVariable *) c.var;
		IntVariable * dest = (IntVariable *) pattern;
        
		int intvsize = intv->wires.size();
        
		for (int i = 0; i < dest->wires.size(); i++)
		{
			if (i < intvsize)
			{
				assignWire(dest->wires[i], intv->wires[i], idxF);
				makeWireContainValueNoONEZEROcopyForFn(dest->wires[i], idxF);
				rs.append(to_string(dest->wires[i]->wireNumber) + " ");
			}
			else
			{
				assignWire(dest->wires[i], get_ZERO_WIRE(idxF), idxF);
			}
		}
	}
	else if (c.var->v_enum == Arrayv)
	{
		ArrayVariable * arrayv = (ArrayVariable *) c.var;
		ArrayVariable * dest = (ArrayVariable *) pattern;
        
		for (int i = 0; i < dest->av.size(); i++)
		{
			messyAssignAndCopyForFn(arrayv->av[i], dest->av[i], idxF);
		}
	}
	else if (c.var->v_enum == Structv)
	{
		StructVariable * structv = (StructVariable *) c.var;
		StructVariable * dest = (StructVariable *) pattern;
        
		for (std::unordered_map<string, Variable *>::iterator it = structv->map.begin(); it != structv->map.end(); ++it)
		{
			Variable * v =  (Variable *)(it->second);
			messyAssignAndCopyForFn(v, dest->map[(string)(it->first)], idxF);
		}
	}
	else
	{
		cout << "undefined type in messy messyAssignAndCopy\n";
	}
	return rs;
}

void putVariableToVector(CORV & c)
{
    if(c.var == 0)
    {
        return;
    }
    
    c.vec.resize(c.var->size());
    
    int startplace = c.var->wv->startwirenum;
    
    for(int i=0;i<c.var->size();i++)
    {
        c.vec[i] = getWire(i+startplace,c.var->wv);
    }
}


void printwirevec(vector<Wire *> & v)
{
    for(int i=v.size()-1;i>=0;i--)
    {
        if(v[i]->state == ONE)
        {
            cout << "1";
        }
        else if(v[i]->state == ZERO)
        {
            cout << "0";
        }
        else
        {
            cout << "-";
        }
    }
}

void printwirevecValue(vector<Wire *> & v)
{
    long val=0;
    for(int i=v.size()-1;i>=0;i--)
    {
        val = val<< 1;
        if(v[i]->state == ONE)
        {
            //cout << "1";
            val = val | 0x1;
        }
        else if(v[i]->state == ZERO)
        {
            //cout << "0";
        }
        else
        {
            //cout << "-";
        }
        
    }
    cout << val;
}


Wire * addGate(short table, Wire * a, Wire * b, Wire * dest, int idxF)
{
    
    int awirenum = a->wireNumber;
    int bwirenum = b->wireNumber;
    
    if(a->state == UNKNOWN_INVERT_OTHER_WIRE)
    {
        awirenum = a->other->wireNumber;
        table = invertTable(false,table);
    }
    else if (a->state == UNKNOWN_OTHER_WIRE)
    {
        awirenum = a->other->wireNumber;
    }
    else if (a->state == UNKNOWN_INVERT)
    {
        table = invertTable(false,table);
    }
    
    if(b->state == UNKNOWN_INVERT_OTHER_WIRE)
    {
        bwirenum = b->other->wireNumber;
        table = invertTable(true,table);
    }
    else if (b->state == UNKNOWN_OTHER_WIRE)
    {
        bwirenum = b->other->wireNumber;
    }
    else if (b->state == UNKNOWN_INVERT)
    {
        table = invertTable(true,table);
    }
    
    if(a->state == ONE)
    {
	    awirenum = one_wire_l[idxF];
    }
    else if(a->state == ZERO)
    {
	    awirenum = zero_wire_l[idxF];
    }
    
    if(b->state == ONE)
    {
	    bwirenum = one_wire_l[idxF];
    }
    else if(b->state == ZERO)
    {
	    bwirenum = zero_wire_l[idxF];
    }
    
    //if this the program gets here then the value must be unknown. If it was known in any way (or a reference to another wire) it would have been done in the short circuit function
    dest->state = UNKNOWN;
	writeGate(table, dest->wireNumber, awirenum, bwirenum, idxF);
    return dest;
}


Wire * invertWire(Wire * w2, int idxF)
{
	Wire * w1 = pool[idxF].getWire();
    
    
    if(w2->state == ONE)
    {
        w1->state = ZERO;
    }
    else if(w2->state == ZERO)
    {
        w1->state = ONE;
    }
    else if(w2->state == UNKNOWN)
    {
        w1->state = UNKNOWN_INVERT_OTHER_WIRE;
        w1->other = w2;
        w1->other->refs++; w1->other->addRef(w1);
    }
    else if(w2->state == UNKNOWN_OTHER_WIRE)
    {
        w1->state = UNKNOWN_INVERT_OTHER_WIRE;
        w1->other = w2->other;
        w1->other->refs++; w1->other->addRef(w1);
    }
    else if(w2->state == UNKNOWN_INVERT)
    {
        w1->state = UNKNOWN_OTHER_WIRE;
        w1->other = w2;
        w1->other->refs++; w1->other->addRef(w1);
    }
    else if(w2->state == UNKNOWN_INVERT_OTHER_WIRE)
    {
        w1->state = UNKNOWN_OTHER_WIRE;
        w1->other = w2->other;
        w1->other->refs++; w1->other->addRef(w1);
    }
    
    return w1;
}

Wire * invertWireNoInvertOutput(Wire * w2, int idxF)
{
	Wire * w1 = pool[idxF].getWire();
    
    
    if(w2->state == ONE)
    {
        w1->state = ZERO;
    }
    else if(w2->state == ZERO)
    {
        w1->state = ONE;
    }
    else if(w2->state == UNKNOWN)
    {
	    addGate(6, w2, ONE_WIRE[idxF], w1, idxF);
    }
    else if(w2->state == UNKNOWN_OTHER_WIRE)
    {
	    addGate(6, w2->other, ONE_WIRE[idxF], w1, idxF);
    }
    else if(w2->state == UNKNOWN_INVERT)
    {
        w1->state = UNKNOWN_OTHER_WIRE;
        w1->other = w2;
        w1->other->refs++; w1->other->addRef(w1);
    }
    else if(w2->state == UNKNOWN_INVERT_OTHER_WIRE)
    {
        w1->state = UNKNOWN_OTHER_WIRE;
        w1->other = w2->other;
        w1->other->refs++; w1->other->addRef(w1);
    }
    
    return w1;
}

Wire * invertWireNoAllocUnlessNecessary(Wire * w2, int idxF)
{
    if(w2->refs > 0)
    {
	    Wire * w1 = invertWire(w2, idxF);
        return w1;
    }
    
    if(w2->state == ONE)
    {
        w2->state = ZERO;
    }
    else if(w2->state == ZERO)
    {
        w2->state = ONE;
    }
    else if(w2->state == UNKNOWN)
    {
        w2->state = UNKNOWN_INVERT;
    }
    else if(w2->state == UNKNOWN_OTHER_WIRE)
    {
        w2->state = UNKNOWN_INVERT_OTHER_WIRE;
    }
    else if(w2->state == UNKNOWN_INVERT)
    {
        w2->state = UNKNOWN;
    }
    else if(w2->state == UNKNOWN_INVERT_OTHER_WIRE)
    {
        w2->state = UNKNOWN_OTHER_WIRE;
    }

    return w2;
}

void clearReffedWire(Wire * w, int idxF)
{
    if(w->refs == 0)
        return;
    
	Wire * newwire = pool[idxF].getWire();
	writeCopy(newwire->wireNumber, w->wireNumber, idxF);
	
	if (isPrintDuploGC)
	{
		newwire->prevWireNumber[0] = w->wireNumber;
		newwire->prevWireNumber[1] = w->prevWireNumber[0];
		
	}
    
    for(int i=w->refsToMeSize-1;i>=0;i--)
    {
        Wire * temp = w->refsToMe[i];
        w->removeRef(temp);
        w->refs--;
        newwire->addRef(temp);
        temp->other = newwire;
        newwire->refs++;
    }
    newwire->state = w->state;
    
    if(w->refs > 0)
    {
        cout << "refs still > than 0\n";
    }
}

void clearReffedWireForFn(Wire * w, int idxF)
{
	if (w->refs == 0)
		return;
    
	Wire * newwire = pool[idxF].getWire();
	//writeCopy(newwire->wireNumber, w->wireNumber, idxF);
	
	if (isPrintDuploGC)
	{
		newwire->prevWireNumber[0] = w->wireNumber;
		newwire->prevWireNumber[1] = w->prevWireNumber[0];
		
	}
    
	for (int i = w->refsToMeSize - 1; i >= 0; i--)
	{
		Wire * temp = w->refsToMe[i];
		w->removeRef(temp);
		w->refs--;
		newwire->addRef(temp);
		temp->other = newwire;
		newwire->refs++;
	}
	newwire->state = w->state;
    
	if (w->refs > 0)
	{
		cout << "refs still > than 0\n";
	}
}

/*
might be broken
void clearOtherdWire(Wire * w1)
{
    if(w1->state == UNKNOWN_OTHER_WIRE || w1->state == UNKNOWN_INVERT_OTHER_WIRE )
    {
        w1->other->refs--; w1->other->removeRef(w1);
        w1->other = 0;
    }
    

}
*/

//assigns wires from w2 to w1 and deals with other references
void assignWire(Wire *  w1, Wire * w2, int idxF)
{
    if(w1 == w2)
    {
        return;
    }
    
    
    if(w1->refs == 1 && w2->other == w1)
    {
        if(w2->state == UNKNOWN_OTHER_WIRE)
        {
            w1->state = UNKNOWN;
            w1->refs--; w1->removeRef(w2);
            w2->other = 0;
            
        }
        else if(w2->state == UNKNOWN_INVERT_OTHER_WIRE)
        {
            w1->state = UNKNOWN_INVERT;
            w1->refs--; w1->removeRef(w2);
            w2->other = 0;
            
        }
        else
        {
            cout <<"error in assign wire with refs, strange\n";
        }
        return;
    }
    else if(w1->refs > 0)
    {
	    clearReffedWire(w1, idxF);
    }
    
    //clear w1
    if(w1->state == UNKNOWN_OTHER_WIRE || w1->state == UNKNOWN_INVERT_OTHER_WIRE )
    {
        w1->other->refs--; w1->other->removeRef(w1);
        w1->other = 0;
    }
    
    
    if(w2->state == ONE)
    {
        w1->state = ONE;
    }
    else if(w2->state == ZERO)
    {
        w1->state = ZERO;
    }
    else if(w2->state == UNKNOWN)
    {
        w1->state = UNKNOWN_OTHER_WIRE;
        w1->other = w2;
        w1->other->refs++; w1->other->addRef(w1);
    }
    else if(w2->state == UNKNOWN_OTHER_WIRE)
    {
        w1->state = UNKNOWN_OTHER_WIRE;
        w1->other = w2->other;
        w1->other->refs++; w1->other->addRef(w1);
    }
    else if(w2->state == UNKNOWN_INVERT)
    {
        w1->state = UNKNOWN_INVERT_OTHER_WIRE;
        w1->other = w2;
        w1->other->refs++; w1->other->addRef(w1);
    }
    else if(w2->state == UNKNOWN_INVERT_OTHER_WIRE)
    {
        w1->state = UNKNOWN_INVERT_OTHER_WIRE;
        w1->other = w2->other;
        w1->other->refs++; w1->other->addRef(w1);
    }
}

//assigns w2 to w1 if w3 is true
void assignWireCond(Wire *  w1, Wire * w2, Wire * w3, int idxF)
{
    if(w1 == w2)
    {
        return;
    }
    


	Wire * xor1o = outputGate(6, w2, w1, idxF);
	Wire * and1o = outputGate(8, xor1o, w3, idxF);
    
    if(w1->refs > 0)
    {
	    clearReffedWire(w1, idxF);
    }
    else if(w1->other != 0)
    {
        //cout << "other in assign cond is not 0\n";
	    makeWireContainValue(w1, idxF);
        /*clearOtherdWire(w1);
        w1->other = 0;
        cout << "other in assign cond is not 0\n";*/
    }
    
	outputGate(6, w1, and1o, w1, idxF);
}

//assigns w2 to w1 if w3 is true
void assignWireCondForFn(Wire *  w1, Wire * w2, Wire * w3, int idxF)
{
	if (w1 == w2)
	{
		return;
	}
    


	Wire * xor1o = outputGate(6, w2, w1, idxF);
	Wire * and1o = outputGate(8, xor1o, w3, idxF);
    
	if (w1->refs > 0)
	{
		clearReffedWire(w1, idxF);
	}
	else if (w1->other != 0)
	{
	    //cout << "other in assign cond is not 0\n";
		makeWireContainValueForFn(w1, idxF);
		/*clearOtherdWire(w1);
		w1->other = 0;
		cout << "other in assign cond is not 0\n";*/
	}
    
	outputGate(6, w1, and1o, w1, idxF);
}


ofstream * os;



Wire * get_ONE_WIRE(int idxF)
{
	return ONE_WIRE[idxF];
}
Wire * get_ZERO_WIRE(int idxF)
{
	return ZERO_WIRE[idxF];
}





/*
 
    Circuit templates
 
 */


//precondiction - all vectors are of proper size( |leftv| == |rightv| and |destv| == 1)
void outputEquals(vector<Wire *> * leftv,vector<Wire *> * rightv, vector<Wire *> & destv, int idxF)
{
    Wire * outputwire;
    Wire * lastxor;
    Wire * lastlastxor;
    Wire * lastand;
    Wire * currentxor;
    
    Wire * t;
    
    for(int i=0;i<leftv->size();i++)
    {
	    t = invertWireNoInvertOutput(leftv->operator[](i), idxF);
        
	    currentxor = outputGateNoInvertOutput(6, t, rightv->operator[](i), idxF);
        
        if(i > 1)
        {
	        outputwire = outputGate(8, currentxor, lastand, idxF);
        }
        else if(i ==1)
        {
	        outputwire = outputGate(8, currentxor, lastxor, idxF);
        }
        else if(i == 0)
        {
            outputwire = currentxor;
        }
        
        lastand = outputwire;
        lastlastxor = lastxor;
        lastxor = currentxor;
    }
    
    destv[0] = outputwire;
}


//precondiction - all vectors are of proper size( |leftv| == |rightv| and |destv| >= 1 (should be == 1 but > will suffice))
//subtracts leftv from rightv
void outputLessThanSigned(vector<Wire *> * leftv,vector<Wire *> * rightv, vector<Wire *> & destv, int idxF)
{
    int length = leftv->size();
    
    Wire * outputwire;
    
    int inlength = leftv->size();

    
    switch(length)
    {
        case 1:
	    destv[0] = outputGate(4, rightv->operator[](0), leftv->operator[](0), idxF);
            outputwire = destv[0];
            break;
        default:
            
            Wire * carry = getPool(idxF)->getWire();
            Wire * xorab = getPool(idxF)->getWire();
            Wire * xorac = getPool(idxF)->getWire();
            Wire * and1 = getPool(idxF)->getWire();
            
            Wire * na = getPool(idxF)->getWire();
            
            carry->state = ZERO;
        
        
            length++;
        
            leftv->resize(length);
            leftv->operator[](length-1) = leftv->operator[](length-2);
            rightv->resize(length);
            rightv->operator[](length-1) = rightv->operator[](length-2);
        
            if(!useTinyOutput)
            {
                for(int i=0;i<length;i++)
                {
	                na = invertWireNoInvertOutput(leftv->operator[](i), idxF);
                    
	                xorab = clearWireForReuse(xorab, idxF);
	                outputGate(6, rightv->operator[](i), na, xorab, idxF);
                    
                    if(i < length-1)
                    {
                        
	                    xorac = clearWireForReuse(xorac, idxF);
	                    outputGate(6, carry, na, xorac, idxF);
                        
	                    and1 = clearWireForReuse(and1, idxF);
	                    outputGateNoInvertOutput(8, xorab, xorac, and1, idxF);
                        
	                    carry = clearWireForReuse(carry, idxF);
	                    outputGateNoInvertOutput(6, na, and1, carry, idxF);
                    }
                    else
                    {
	                    Wire * t = invertWireNoInvertOutput(xorab, idxF);
                        
	                    Wire * d = outputGateNoInvertOutput(6, t, carry, idxF);
                        //Wire * d = outputGate(9,xorab,carry);
                        outputwire = d;
                        destv[0] = outputwire;
                    }
                    
                }
            }
            else
            {
                Wire * d = getPool(idxF)->getWire();
                destv[0] = d;
                
                for(int i=0;i<length;i++)
                {
                    //cout << i<<"\n";
                    
	                na = invertWireNoInvertOutput(leftv->operator[](i), idxF);
                    
	                xorab = clearWireForReuse(xorab, idxF);
	                outputGate(6, rightv->operator[](i), na, xorab, idxF);
                    
                    if(i < length-1)
                    {
                        
	                    xorac = clearWireForReuse(xorac, idxF);
	                    outputGate(6, carry, na, xorac, idxF);
                        
	                    and1 = clearWireForReuse(and1, idxF);
	                    outputGateNoInvertOutput(8, xorab, xorac, and1, idxF);
                        
	                    carry = clearWireForReuse(carry, idxF);
	                    outputGateNoInvertOutput(6, na, and1, carry, idxF);
                    }
                    else
                    {
	                    Wire * t = invertWireNoInvertOutput(xorab, idxF);
                        
	                    outputGateNoInvertOutput(6, t, carry, d, idxF);
                        outputwire = d;
                    }
                    
                    if(carry->state == UNKNOWN)
                    {
                        int L = 0;
                        for(L=i+1;L<length;L++)
                        {
                            if(leftv->operator[](L)->state != UNKNOWN || rightv->operator[](L)->state != UNKNOWN || (leftv->operator[](L-1)->wireNumber+1 != leftv->operator[](L)->wireNumber) || (rightv->operator[](L-1)->wireNumber+1 != rightv->operator[](L)->wireNumber) )
                            {
                                break;
                            }
                        }
                        
                        int size = L-(i+1);
                        
                        //cout << "startout\n";
                        
                        if(size > 0)
                        {
                            //cout <<"outlarg";
                            
	                        addComplexOpSingleDestBit(2, size, i + 1, i + 1, 0, (*leftv), (*rightv), destv, carry, (i + size) == length - 1, idxF);
                            
                            i+=size;
                        }
                        //cout << "endout\n";
                    }
                    
                }
               
            }
        

        
            length--;
        leftv->resize(length);
        rightv->resize(length);

        
            break;
    }
    
    //destv[0] = outputwire;
    
    //just being careful aftering changing the input vectors length. its a bad idea if done carelessly but it solves many problems. (i.e. needed the extra bit)
    if(inlength != leftv->size() || inlength != rightv->size())
    {
        cerr << "Error in leftv length in lessthan\n";
    }

}


//precondiction - all vectors are of proper size( |leftv| == |rightv| and |destv| >= 1 (should be == 1 but > will suffice))
//subtracts leftv from rightv
void outputLessThanUnsigned(vector<Wire *> * leftv,vector<Wire *> * rightv, vector<Wire *> & destv, int idxF)
{
    int length = leftv->size();
    
    Wire * outputwire;
    
    int inlength = leftv->size();
    
    
    switch(length)
    {
        case 1:
	    destv[0] = outputGate(4, rightv->operator[](0), leftv->operator[](0), idxF);
            outputwire = destv[0];
            break;
        default:
            
            Wire * carry = getPool(idxF)->getWire();
            Wire * xorab = getPool(idxF)->getWire();
            Wire * xorac = getPool(idxF)->getWire();
            Wire * and1 = getPool(idxF)->getWire();
            
            Wire * na = getPool(idxF)->getWire();
            
            carry->state = ZERO;
            
            
            length++;
            
            leftv->resize(length);
            leftv->operator[](length-1) = ZERO_WIRE[idxF];
            rightv->resize(length);
            rightv->operator[](length-1) = ZERO_WIRE[idxF];
            
            if(!useTinyOutput)
            {
                for(int i=0;i<length;i++)
                {
	                na = invertWireNoInvertOutput(leftv->operator[](i), idxF);
                    
	                xorab = clearWireForReuse(xorab, idxF);
	                outputGate(6, rightv->operator[](i), na, xorab, idxF);
                    
                    if(i < length-1)
                    {
                        
	                    xorac = clearWireForReuse(xorac, idxF);
	                    outputGate(6, carry, na, xorac, idxF);
                        
	                    and1 = clearWireForReuse(and1, idxF);
	                    outputGateNoInvertOutput(8, xorab, xorac, and1, idxF);
                        
	                    carry = clearWireForReuse(carry, idxF);
	                    outputGateNoInvertOutput(6, na, and1, carry, idxF);
                    }
                    else
                    {
	                    Wire * t = invertWireNoInvertOutput(xorab,idxF);
                        
	                    Wire * d = outputGateNoInvertOutput(6, t, carry, idxF);
                        //Wire * d = outputGate(9,xorab,carry);
                        outputwire = d;
                        destv[0] = outputwire;
                    }
                    
                }
            }
            else
            {
                Wire * d = getPool(idxF)->getWire();
                destv[0] = d;
                
                for(int i=0;i<length;i++)
                {
                    //cout << i<<"\n";
                    
                    na = invertWireNoInvertOutput(leftv->operator[](i), idxF);
                    
                    xorab = clearWireForReuse(xorab, idxF);
	                outputGate(6, rightv->operator[](i), na, xorab, idxF);
                    
                    if(i < length-1)
                    {
                        
	                    xorac = clearWireForReuse(xorac, idxF);
	                    outputGate(6, carry, na, xorac, idxF);
                        
	                    and1 = clearWireForReuse(and1, idxF);
	                    outputGateNoInvertOutput(8, xorab, xorac, and1, idxF);
                        
	                    carry = clearWireForReuse(carry, idxF);
	                    outputGateNoInvertOutput(6, na, and1, carry, idxF);
                    }
                    else
                    {
	                    Wire * t = invertWireNoInvertOutput(xorab, idxF);
                        
	                    outputGateNoInvertOutput(6, t, carry, d, idxF);
                        outputwire = d;
                    }
                    
                    if(carry->state == UNKNOWN)
                    {
                        int L = 0;
                        for(L=i+1;L<length;L++)
                        {
                            if(leftv->operator[](L)->state != UNKNOWN || rightv->operator[](L)->state != UNKNOWN || (leftv->operator[](L-1)->wireNumber+1 != leftv->operator[](L)->wireNumber) || (rightv->operator[](L-1)->wireNumber+1 != rightv->operator[](L)->wireNumber) )
                            {
                                break;
                            }
                        }
                        
                        int size = L-(i+1);
                        
                        //cout << "startout\n";
                        
                        if(size > 0)
                        {
	                        addComplexOpSingleDestBit(2, size, i + 1, i + 1, 0, (*leftv), (*rightv), destv, carry, (i + size) == length - 1, idxF);
                            
                            i+=size;
                        }
                        //cout << "endout\n";
                    }
                    
                }
            }
            //printWire(carry);
            
            
            
            length--;
            leftv->resize(length);
            rightv->resize(length);
            
            
            break;
    }
    
    
    
    //just being careful aftering changing the input vectors length. its a bad idea if done carelessly but it solves many problems. (i.e. needed the extra bit)
    if(inlength != leftv->size() || inlength != rightv->size())
    {
        cerr << "Error in leftv length in lessthan\n";
    }
    
}

//precondiction - all vectors are of proper size( |leftv| == |rightv| == |destv| ))
void outputSubtract(vector<Wire *> * leftv,vector<Wire *> * rightv, vector<Wire *> & destv, int idxF)
{
    int length = leftv->size();
    
    /*printwirevec(*leftv);
    cout <<"\n";
    printwirevec(*rightv);
    cout <<"\n";*/
    
    switch(length)
    {
        case 1:
	    destv[0] = outputGate(6, rightv->operator[](0), leftv->operator[](0), idxF);
            break;
        default:
            
            
            Wire * carry = getPool(idxF)->getWire();
            Wire * xorab = getPool(idxF)->getWire();
            Wire * xorac = getPool(idxF)->getWire();
            Wire * and1 = getPool(idxF)->getWire();
            
            Wire * na = getPool(idxF)->getWire();
            
            carry->state = ZERO;
            
            WireSet * ws = getPool(idxF)->getWires(length);
            
            for(int i=0;i<length;i++)
            {
                destv[i] = ws->wires[i];
            }
            
            if(!useTinyOutput)
            {
                for(int i=0;i<length;i++)
                {
                    na = invertWireNoInvertOutput(leftv->operator[](i),idxF);
                    
	                xorab = clearWireForReuse(xorab, idxF);
	                outputGate(6, rightv->operator[](i), na, xorab, idxF);
                    
	                Wire * t = invertWireNoInvertOutput(xorab, idxF);
                    
	                outputGateNoInvertOutput(6, t, carry, destv[i], idxF);
                    
                    //outputGate(9,xorab,carry, destv[i]);
                    //destv[i] = d;
                    
                    if(i < length-1)
                    {
                        
	                    xorac = clearWireForReuse(xorac, idxF);
	                    outputGate(6, carry, na, xorac, idxF);
                        
	                    and1 = clearWireForReuse(and1, idxF);
	                    outputGateNoInvertOutput(8, xorab, xorac, and1, idxF);
                        
	                    carry = clearWireForReuse(carry, idxF);
	                    outputGateNoInvertOutput(6, na, and1, carry, idxF);
                    }
                    
                }
            }
            else
            {
                for(int i=0;i<length;i++)
                {
	                na = invertWireNoInvertOutput(leftv->operator[](i), idxF);
                    
	                xorab = clearWireForReuse(xorab, idxF);
	                outputGate(6, rightv->operator[](i), na, xorab, idxF);
                    
	                Wire * t = invertWireNoInvertOutput(xorab, idxF);
                    
	                outputGateNoInvertOutput(6, t, carry, destv[i], idxF);
                    
                    //outputGate(9,xorab,carry, destv[i]);
                    //destv[i] = d;
                    
                    if(i < length-1)
                    {
                        
	                    xorac = clearWireForReuse(xorac, idxF);
	                    outputGate(6, carry, na, xorac, idxF);
                        
	                    and1 = clearWireForReuse(and1, idxF);
	                    outputGateNoInvertOutput(8, xorab, xorac, and1, idxF);
                        
	                    carry = clearWireForReuse(carry, idxF);
	                    outputGateNoInvertOutput(6, na, and1, carry, idxF);
                    }
                    
                    if(carry->state == UNKNOWN)
                    {
                        int L = 0;
                        for(L=i+1;L<length;L++)
                        {
                            if(leftv->operator[](L)->state != UNKNOWN || rightv->operator[](L)->state != UNKNOWN || (leftv->operator[](L-1)->wireNumber+1 != leftv->operator[](L)->wireNumber) || (rightv->operator[](L-1)->wireNumber+1 != rightv->operator[](L)->wireNumber) )
                            {
                                break;
                            }
                        }
                        
                        int size = L-(i+1);
                        
                        if(size > 0)
                        {
	                        addComplexOp(1, size, i + 1, i + 1, i + 1, (*leftv), (*rightv), destv, carry, (i + size) == length - 1, idxF);
                            
                            i+=size;
                        }
                    }
                    
                }
                
            }
            break;
            


    }
    
    /*printwirevec(destv);
    cout << " = ";
    
    printwirevec(*leftv);
    cout << " - ";
    printwirevec(*rightv);
    cout <<"\n\n";*/
    
    /*printwirevec(destv);
    cout <<"\n\n";*/
}

//precondiction - all vectors are of proper size( |leftv| == |rightv| == |destv| ))
void outputAddition(vector<Wire *> * leftv,vector<Wire *> * rightv, vector<Wire *> & destv, int idxF)
{
    int length = leftv->size();
    
    switch(length)
    {
        case 1:
	    destv[0] = outputGate(6, rightv->operator[](0), leftv->operator[](0), idxF);
            break;
        default:
            //new adder
            Wire * carry = getPool(idxF)->getWire();
            Wire * xorab = getPool(idxF)->getWire();
            Wire * xorac = getPool(idxF)->getWire();
            //Wire * xorabxc = getPool(idxF)->getWire();
            Wire * and1 = getPool(idxF)->getWire();
            
            carry->state = ZERO;
            
            WireSet * ws = getPool(idxF)->getWires(length);
            
            for(int i=0;i<length;i++)
            {
                destv[i] = ws->wires[i];
            }
            
            
            if(!useTinyOutput)
            {
                for(int i=0;i<length;i++)
                {
	                xorab = clearWireForReuse(xorab, idxF);
	                outputGateNoInvertOutput(6, rightv->operator[](i), leftv->operator[](i), xorab, idxF);
                    
                    /*Wire * d = outputGate(6,xorab,carry);
                    destv[i] = d;*/
	                outputGate(6, xorab, carry, destv[i], idxF);
                    
                    if(i < length-1)
                    {
                        
	                    xorac = clearWireForReuse(xorac, idxF);
	                    outputGate(6, carry, leftv->operator[](i), xorac, idxF);
                        
	                    and1 = clearWireForReuse(and1, idxF);
	                    outputGateNoInvertOutput(8, xorab, xorac, and1, idxF);
                        
	                    carry = clearWireForReuse(carry, idxF);
	                    outputGateNoInvertOutput(6, leftv->operator[](i), and1, carry, idxF);
                    }
                    
                }
            }
            else
            {

                
                for(int i=0;i<length;i++)
                {
	                xorab = clearWireForReuse(xorab, idxF);
	                outputGateNoInvertOutput(6, rightv->operator[](i), leftv->operator[](i), xorab, idxF);
                    
	                outputGate(6, xorab, carry, destv[i], idxF);
                    
                    if(i < length-1)
                    {
                        
	                    xorac = clearWireForReuse(xorac, idxF);
	                    outputGate(6, carry, leftv->operator[](i), xorac, idxF);
                        
	                    and1 = clearWireForReuse(and1, idxF);
	                    outputGateNoInvertOutput(8, xorab, xorac, and1, idxF);
                        
	                    carry = clearWireForReuse(carry, idxF);
                        outputGateNoInvertOutput(6,leftv->operator[](i),and1,carry,idxF);
                    }
                    
                    if(carry->state == UNKNOWN)
                    {
                        int L = 0;
                        for(L=i+1;L<length;L++)
                        {
                            if(leftv->operator[](L)->state != UNKNOWN || rightv->operator[](L)->state != UNKNOWN || (leftv->operator[](L-1)->wireNumber+1 != leftv->operator[](L)->wireNumber) || (rightv->operator[](L-1)->wireNumber+1 != rightv->operator[](L)->wireNumber) )
                            {
                                break;
                            }
                        }
                        
                        int size = L-(i+1);
                        
                        if(size > 0)
                        {
	                        addComplexOp(0, size, i + 1, i + 1, i + 1, (*leftv), (*rightv), destv, carry, (i + size) == length - 1, idxF);
                            
                            i+=size;
                        }
                    }
                    
                }
            }
            break;
    }
}


//only does right side of multiplication trapazoid (i.e. if your mult is 32 bits by 32 bits we don't need the result of the left 32 bits since it goes back into a 32bit int, not a 64 bit int).
//precondiction - all vectors are of proper size( |leftv| == |rightv| == |destv| ))
//left is x input
//right is y input
//mult algorithm from MIT slides from course 6.111, fall 2012, lecture 8/9, slide 33
void outputMultSigned(vector<Wire *> * leftv,vector<Wire *> * rightv, vector<Wire *> & destv, int idxF)
{
    int length = leftv->size();
    
    
    switch(length)
    {
        case 1:
	    destv[0] = outputGate(8, leftv->operator[](0), rightv->operator[](0), idxF);
            break;
        default:
            
            vector<Wire *> rowoutputs;
            rowoutputs.resize(length);
            vector<Wire *> rowinputsleft;
            rowinputsleft.resize(length);
            vector<Wire *> rowinputsright;
            rowinputsright.resize(length);
            
            Wire * carry = getPool(idxF)->getWire();
            Wire * xor2 = getPool(idxF)->getWire();
            Wire * xorab = getPool(idxF)->getWire();
            Wire * andn1 = getPool(idxF)->getWire();
            Wire * andn2 = getPool(idxF)->getWire();
            
            WireSet * ws = getPool(idxF)->getWires(length);
            
            for(int i=0;i<length;i++)
            {
                destv[i] = ws->wires[i];
            }
            
            
            //length--;
            
            //number of rows
            for(int i=0;i<length-1;i++)
            {
                //cout << "headerstart\n";
                
                //create inputs to each adder
                if(i == 0)
                {

                    for(int k=0;k<length-i;k++)
                    {
                        //cout << "> gate\n";
	                    rowinputsleft[k] = outputGate(8, leftv->operator[](k), rightv->operator[](0), idxF);
                    }
                    //only on first row do we do this
	                rowinputsleft[length - 1] = invertWireNoAllocUnlessNecessary(rowinputsleft[length - 1], idxF);
                    
                    for(int k=0;k<length-i-1;k++)
                    {
                       // cout << "> gate\n";
	                    rowinputsright[k] = outputGate(8, leftv->operator[](k), rightv->operator[](1), idxF);
                    }
                    
                    
                    //destv[0] = getPool(idxF)->getWire();
                    
	                assignWire(destv[0], rowinputsleft[0], idxF);
                    
                    //shift down
                    for(int k=0;k<length-1;k++)
                    {
	                    assignWire(rowinputsleft[k], rowinputsleft[k + 1], idxF);
                    }
                    
                    if(i == length-2)
                    {
	                    rowinputsright[0] = invertWireNoAllocUnlessNecessary(rowinputsright[0], idxF);
                    }
                }
                else
                {

                    
                    for(int k=0;k<length-1-i;k++)
                    {
                        //cout << "> gate\n";
	                    rowinputsright[k] = clearWireForReuse(rowinputsright[k], idxF);
	                    outputGate(8, leftv->operator[](k), rightv->operator[](i + 1), rowinputsright[k], idxF);
                    }
                    
                    //last row
                    if(i == length-2)
                    {
	                    rowinputsright[0] = invertWireNoAllocUnlessNecessary(rowinputsright[0], idxF);
                    }
                    
                }

                //cout << "rowstart\n";
                //create each adder
                for(int j=0;j<length-i-1;j++)
                {
                    //performs the HA or FA
                    

                    
                    //output half adder
                    if(j == 0)
                    {
	                    xorab = clearWireForReuse(xorab, idxF);
	                    outputGate(6, rowinputsright[0], rowinputsleft[0], xorab, idxF);

                        //destv[i+1] = pool[idxF].getWire();
	                    assignWire(destv[i + 1], xorab, idxF);
                       // cout << "> gate\n";
	                    carry = clearWireForReuse(carry, idxF);
                        
                        if(i != length-2)
                        {
	                        outputGate(8, rowinputsright[0], rowinputsleft[0], carry, idxF);
                        }

                    }
                    else //output full adder
                    {
                        //cout << "start FA\n";
                        
	                    xorab = clearWireForReuse(xorab, idxF);
	                    outputGate(6, rowinputsright[j], rowinputsleft[j], xorab, idxF);
                        
	                    rowinputsleft[j - 1] = clearWireForReuse(rowinputsleft[j - 1], idxF);
	                    outputGate(6, xorab, carry, rowinputsleft[j - 1], idxF);
                        
                        if(j < length-1-i-1)
                        {
                            //cout << "full adder\n";
                            
	                        andn2 = clearWireForReuse(andn2, idxF);
	                        outputGate(6, carry, rowinputsleft[j], andn2, idxF);
                            //cout << "> gate\n";
	                        andn1 = clearWireForReuse(andn1, idxF);
	                        outputGate(8, xorab, andn2, andn1, idxF);
                            
	                        carry = clearWireForReuse(carry, idxF);
	                        outputGate(6, rowinputsleft[j], andn1, carry, idxF);
                        }
                    }
                    
	                if (andn2->refs == 0) clearWireForReuse(andn2, idxF);
	                if (andn1->refs == 0) clearWireForReuse(andn1, idxF);
	                if (xorab->refs == 0) clearWireForReuse(xorab, idxF);
	                if (andn2->refs == 0) clearWireForReuse(andn2, idxF);
	                if (andn1->refs == 0) clearWireForReuse(andn1, idxF);
	                if (xorab->refs == 0) clearWireForReuse(xorab, idxF);
	                if (andn2->refs == 0) clearWireForReuse(andn2, idxF);
	                if (andn1->refs == 0) clearWireForReuse(andn1, idxF);
	                if (xorab->refs == 0) clearWireForReuse(xorab, idxF);
                }
            }
            break;
    }
    
    //cout << destv.size()<<"\n";
    /*cout << rightv->operator[](destv.size()-1)->state <<  leftv->operator[](destv.size()-1)->state << destv[destv.size()-1]->state <<"\n";
    
    printwirevec(*rightv);cout <<"\n";
    printwirevec(*leftv); cout <<"\n";
    printwirevec(destv); cout <<"\n";*/
    
    //destv[leftv->size()-1] = pool[idxF].getWire();
    //outputGate(6,leftv->operator[](leftv->size()-1),rightv->operator[](leftv->size()-1),destv[leftv->size()-1]);
    
}


void outputMultUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, int idxF)
{
    int length = leftv->size();
    
    
    switch(length)
    {
        case 1:
            destv[0] = outputGate(8,leftv->operator[](0),rightv->operator[](0), idxF);
            break;
        default:
            
            vector<Wire *> rowoutputs;
            rowoutputs.resize(length);
            vector<Wire *> rowinputsleft;
            rowinputsleft.resize(length);
            vector<Wire *> rowinputsright;
            rowinputsright.resize(length);
            
            Wire * carry = getPool(idxF)->getWire();
            Wire * xor2 = getPool(idxF)->getWire();
            Wire * xorab = getPool(idxF)->getWire();
            Wire * andn1 = getPool(idxF)->getWire();
            Wire * andn2 = getPool(idxF)->getWire();
            
            
            WireSet * ws = getPool(idxF)->getWires(length);
            
            for(int i=0;i<length;i++)
            {
                destv[i] = ws->wires[i];
            }
            
            
            //number of rows
            for(int i=0;i<length-1;i++)
            {
                //create inputs to each adder
                if(i == 0)
                {
                    
                    for(int k=0;k<length-i;k++)
                    {
	                    rowinputsleft[k] = outputGate(8, leftv->operator[](k), rightv->operator[](0), idxF);
                    }
                    //only on first row do we do this
                    //rowinputsleft[length-1] = invertWireNoAllocUnlessNecessary(rowinputsleft[length-1]);
                    
                    for(int k=0;k<length-i-1;k++)
                    {
	                    rowinputsright[k] = outputGate(8, leftv->operator[](k), rightv->operator[](1), idxF);
                    }
                    
                    
                    //destv[0] = getPool(idxF)->getWire();
                    
                    assignWire(destv[0],rowinputsleft[0], idxF);
                    
                    //shift down
                    for(int k=0;k<length-1;k++)
                    {
	                    assignWire(rowinputsleft[k], rowinputsleft[k + 1], idxF);
                    }
                    
                    /*if(i == length-2)
                    {
                        rowinputsright[0] = invertWireNoAllocUnlessNecessary(rowinputsright[0]);
                    }*/
                }
                else
                {
                    
                    
                    for(int k=0;k<length-1-i;k++)
                    {
	                    rowinputsright[k] = clearWireForReuse(rowinputsright[k], idxF);
	                    outputGate(8, leftv->operator[](k), rightv->operator[](i + 1), rowinputsright[k], idxF);
                    }
                    
                    //last row
                    if(i == length-2)
                    {
                        //rowinputsright[0] = invertWireNoAllocUnlessNecessary(rowinputsright[0]);
                    }
                    
                }
                
                //create each adder
                for(int j=0;j<length-i-1;j++)
                {
                    //performs the HA or FA
                    
                    
                    
                    //output half adder
                    if(j == 0)
                    {
	                    xorab = clearWireForReuse(xorab, idxF);
	                    outputGate(6, rowinputsright[0], rowinputsleft[0], xorab, idxF);
                        
                        //destv[i+1] = pool[idxF].getWire();
	                    assignWire(destv[i + 1], xorab, idxF);
                        
	                    carry = clearWireForReuse(carry, idxF);
                        
                        if(i != length-2)
                        {
	                        outputGate(8, rowinputsright[0], rowinputsleft[0], carry, idxF);
                        }
                        
                    }
                    else //output full adder
                    {
	                    xorab = clearWireForReuse(xorab, idxF);
	                    outputGate(6, rowinputsright[j], rowinputsleft[j], xorab, idxF);
                        
	                    rowinputsleft[j - 1] = clearWireForReuse(rowinputsleft[j - 1], idxF);
	                    outputGate(6, xorab, carry, rowinputsleft[j - 1], idxF);
                        
                        if(j < length-1-i-1)
                        {
	                        andn2 = clearWireForReuse(andn2, idxF);
	                        outputGate(6, carry, rowinputsleft[j], andn2, idxF);
                            
	                        andn1 = clearWireForReuse(andn1, idxF);
	                        outputGate(8, xorab, andn2, andn1, idxF);
                            
	                        carry = clearWireForReuse(carry, idxF);
	                        outputGate(6, rowinputsleft[j], andn1, carry, idxF);
                        }
                    }
                    
	                if (andn2->refs == 0) clearWireForReuse(andn2, idxF);
	                if (andn1->refs == 0) clearWireForReuse(andn1, idxF);
	                if (xorab->refs == 0) clearWireForReuse(xorab, idxF);
	                if (andn2->refs == 0) clearWireForReuse(andn2, idxF);
	                if (andn1->refs == 0) clearWireForReuse(andn1, idxF);
	                if (xorab->refs == 0) clearWireForReuse(xorab, idxF);
	                if (andn2->refs == 0) clearWireForReuse(andn2, idxF);
	                if (andn1->refs == 0) clearWireForReuse(andn1, idxF);
	                if (xorab->refs == 0) clearWireForReuse(xorab, idxF);
                }
            }
            break;
    }
}


vector<Wire *> xWires;
int xwirefront;
int xwireback;





//leftv - dividend, rightv - divisor
void outputDivideUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv, int idxF)
{
    int origlength = leftv->size();
    int length = leftv->size()+1;
    
    Wire * carry = getPool(idxF)->getWire();
    Wire * xorab = getPool(idxF)->getWire();
    Wire * xorac = getPool(idxF)->getWire();
    Wire * and1 = getPool(idxF)->getWire();
    
    Wire * xortout = getPool(idxF)->getWire();
    
	Wire * t = get_ONE_WIRE(idxF);
    
    vector<Wire *> inputx, inputy,remainw;
    vector<Wire *> lleft, lright,ldest;
    
    lleft.resize(origlength);
    lright.resize(origlength);
    ldest.resize(length);
    
    int i_out=0;
    for(;i_out<origlength;i_out++)
    {
        lleft[i_out] = leftv->operator[](i_out);
        lright[i_out] = rightv->operator[](i_out);
    }
    
    /*extend extra bit for correctness purposes*/
    lleft.resize(length);
    lright.resize(length);

    for(;i_out<length;i_out++)
    {
        lleft[i_out] = get_ZERO_WIRE(idxF);
        lright[i_out] = get_ZERO_WIRE(idxF);
    }

    inputx.resize(length);
    inputy.resize(length);
    remainw.resize(length);
    
    //divisor
    for(int i=0;i<length;i++)
    {
        //printWire(lleft[i]);
        //printWire(lright[i]);
        
        inputx[i] = lright[i];
        inputy[i] = getPool(idxF)->getWire();
    }
    
    vector<Wire *> keepwiresA;
    vector<Wire *> keepwiresB;
    
    
    for(int i=0;i<length;i++)
    {
        /* setting each input row*/
        if(i==0)
        {
            //remain in / dividend
            inputy[0] = lleft[length-1];
            
            for(int j=1;j<length;j++)
            {
	            assignWire(inputy[j], get_ZERO_WIRE(idxF), idxF);
            }
        }
        else
        {
            inputy[0] = lleft[length-1-i];
            for(int j=1;j<length;j++)
            {
	            assignWire(inputy[j], remainw[j - 1] , idxF);
            }
        }

        //carry in
        if(i == 0)
        {
            carry->state = ONE;
        }
        else
        {
	        assignWire(carry, t, idxF);
        }
        
        
        /*controlled add / subtract*/
        for(int j=0;j<length;j++)
        {
	        xortout = clearWireForReuse(xortout, idxF);
	        outputGateNoInvertOutput(6, t, inputx[j], xortout, idxF);
            
	        xorab = clearWireForReuse(xorab, idxF);
	        outputGateNoInvertOutput(6, inputy[j], xortout, xorab, idxF);
            
            //full adder part
            if(remainw[j] != 0)
            {
                
                //save wires from layer i so they can be used at layer i+1 (otherwise each remainw wire requires a new wire, very inefficient)
                if(remainw[j]->refs > 0)
                {
                    if(i%2 == 0)
                    {
                        keepwiresA.push_back(remainw[j]);
                        if(keepwiresB.size() > 0)
                        {
                            remainw[j] = keepwiresB.back();
                            keepwiresB.pop_back();
                        }
                        else
                        {
                            remainw[j] = getPool(idxF)->getWire();
                        }
                    }
                    else
                    {
                        keepwiresB.push_back(remainw[j]);
                        if(keepwiresA.size() > 0)
                        {
                            remainw[j] = keepwiresA.back();
                            keepwiresA.pop_back();
                        }
                        else
                        {
                            remainw[j] = getPool(idxF)->getWire();
                        }
                    }
                    if(remainw[j] == 0) remainw[j] = getPool(idxF)->getWire();
                }
                
    
                
	            remainw[j] = clearWireForReuse(remainw[j], idxF);
	            outputGate(6, xorab, carry, remainw[j], idxF);
            }
            else
            {
	            remainw[j] = outputGate(6, xorab, carry, idxF);
            }
            
            
            if(j < length-1)
            {
	            xorac = clearWireForReuse(xorac, idxF);
	            outputGate(6, carry, xortout, xorac, idxF);
                
	            and1 = clearWireForReuse(and1, idxF);
	            outputGateNoInvertOutput(8, xorab, xorac, and1, idxF);
                
                carry = clearWireForReuse(carry, idxF);
	            outputGateNoInvertOutput(6, xortout, and1, carry, idxF);
            }
            
	        if (xortout->refs == 0) clearWireForReuse(xortout, idxF);
	        if (xorab->refs == 0) clearWireForReuse(xorab, idxF);
	        if (xorac->refs == 0) clearWireForReuse(xorac, idxF);
	        if (and1->refs == 0) clearWireForReuse(and1, idxF);
        }
        
	    t = invertWireNoInvertOutput(remainw[length - 1], idxF);

        
        if(!IsModDiv)
        {
            ldest[(length-1)-i] = t;
        }
    }
    
    /*get modulus*/
    if(IsModDiv)
    {
        
        for(int i=0;i<length;i++)
        {
            ldest[i] = remainw[i];
        }
        
        vector<Wire *> addDest;
        addDest.resize(ldest.size());
	    outputAddition(&ldest, &lright, addDest, idxF);
        
        //cout << "before fail\n";
        for(int i=0;i<ldest.size();i++)
        {
	        assignWireCond(ldest[i], addDest[i], ldest[destv.size()], idxF);
        }
    }
    
    WireSet * ws = getPool(idxF)->getWires(origlength);
    
    for(int i=0;i<origlength;i++)
    {
        destv[i] = ws->wires[i];
        //cout << destv[i]->wireNumber<<"\n";
    }
    
    
    /*reduce to original length*/
    for(int i=0;i<origlength;i++)
    {
	    assignWire(destv[i], ldest[i], idxF);
    }
}



//leftv - dividend, rightv - divisor
void outputDivideSigned(vector<Wire *> * leftv,vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv, int idxF)
{
    //cout << "outputDivideSigned not complete\n";
    
    int origlength = leftv->size();
    int length = leftv->size()+1;
    
    Wire * carry = getPool(idxF)->getWire();
    Wire * xorab = getPool(idxF)->getWire();
    Wire * xorac = getPool(idxF)->getWire();
    Wire * and1 = getPool(idxF)->getWire();
    
    Wire * xortout = getPool(idxF)->getWire();
    
	Wire * t = get_ONE_WIRE(idxF);
    
    vector<Wire *> inputx, inputy,remainw;
    vector<Wire *> lleft, lright,ldest, subtractedleft;
    
    lleft.resize(origlength);
    lright.resize(origlength);
    ldest.resize(length);
    
    int i_out=0;
    for(;i_out<origlength;i_out++)
    {
        lleft[i_out] = getPool(idxF)->getWire();
	    assignWire(lleft[i_out], leftv->operator[](i_out), idxF);
        //makeWireContainValue(lleft[i_out]);
        
        lright[i_out] = getPool(idxF)->getWire();
        //lright[i_out] = rightv->operator[](i_out);
	    assignWire(lright[i_out], rightv->operator[](i_out), idxF);
        //makeWireContainValue(lright[i_out]);
    }
    
    
    Wire * ifsubtractl = getPool(idxF)->getWire();
    Wire * ifsubtractr = getPool(idxF)->getWire();
    
	assignWire(ifsubtractl, lleft[origlength - 1], idxF);
	assignWire(ifsubtractr, lright[origlength - 1], idxF);
    
    vector<Wire *> zeros;
    vector<Wire *> subDestr,subDestl;
    subDestr.resize(origlength);
    subDestl.resize(origlength);
    zeros.resize(origlength);
    
    for(int i=0;i<origlength;i++)
    {
        zeros[i] = ZERO_WIRE[idxF];
    }
    
	outputSubtract(&zeros, leftv, subDestl, idxF);
	outputSubtract(&zeros, rightv, subDestr, idxF);
    
    
    for(i_out=0;i_out<origlength;i_out++)
    {
	    assignWireCond(lleft[i_out], subDestl[i_out], ifsubtractl, idxF);
	    assignWireCond(lright[i_out], subDestr[i_out], ifsubtractr, idxF);
    }
    
    
    
    /*extend extra bit for correctness purposes*/
    lleft.resize(length);
    lright.resize(length);
    
    for(;i_out<length;i_out++)
    {
        lleft[i_out] = get_ZERO_WIRE(idxF);
        lright[i_out] = get_ZERO_WIRE(idxF);
    }
    
    inputx.resize(length);
    inputy.resize(length);
    remainw.resize(length);
    
    //divisor
    for(int i=0;i<length;i++)
    {
        //printWire(lleft[i]);
        //printWire(lright[i]);
        
        inputx[i] = lright[i];
        inputy[i] = getPool(idxF)->getWire();
    }
    
    vector<Wire *> keepwiresA;
    vector<Wire *> keepwiresB;
    
    
    for(int i=0;i<length;i++)
    {
        /* setting each input row*/
        if(i==0)
        {
            //remain in / dividend
            inputy[0] = lleft[length-1];
            
            for(int j=1;j<length;j++)
            {
	            assignWire(inputy[j], get_ZERO_WIRE(idxF), idxF);
            }
        }
        else
        {
            inputy[0] = lleft[length-1-i];
            for(int j=1;j<length;j++)
            {
	            assignWire(inputy[j], remainw[j - 1], idxF);
            }
        }
        
        //carry in
        if(i == 0)
        {
            carry->state = ONE;
        }
        else
        {
	        assignWire(carry, t, idxF);
        }
        
        
        /*controlled add / subtract*/
        for(int j=0;j<length;j++)
        {
	        xortout = clearWireForReuse(xortout, idxF);
	        outputGateNoInvertOutput(6, t, inputx[j], xortout, idxF);
            
	        xorab = clearWireForReuse(xorab, idxF);
	        outputGateNoInvertOutput(6, inputy[j], xortout, xorab, idxF);
            
            //full adder part
            if(remainw[j] != 0)
            {
                
                //save wires from layer i so they can be used at layer i+1 (otherwise each remainw wire requires a new wire, very inefficient)
                if(remainw[j]->refs > 0)
                {
                    if(i%2 == 0)
                    {
                        keepwiresA.push_back(remainw[j]);
                        if(keepwiresB.size() > 0)
                        {
                            remainw[j] = keepwiresB.back();
                            keepwiresB.pop_back();
                        }
                        else
                        {
                            remainw[j] = getPool(idxF)->getWire();
                        }
                    }
                    else
                    {
                        keepwiresB.push_back(remainw[j]);
                        if(keepwiresA.size() > 0)
                        {
                            remainw[j] = keepwiresA.back();
                            keepwiresA.pop_back();
                        }
                        else
                        {
                            remainw[j] = getPool(idxF)->getWire();
                        }
                    }
                    if(remainw[j] == 0) remainw[j] = getPool(idxF)->getWire();
                }
                
                
                
	            remainw[j] = clearWireForReuse(remainw[j], idxF);
	            outputGate(6, xorab, carry, remainw[j], idxF);
            }
            else
            {
	            remainw[j] = outputGate(6, xorab, carry, idxF);
            }
            
            
            if(j < length-1)
            {
	            xorac = clearWireForReuse(xorac, idxF);
	            outputGate(6, carry, xortout, xorac, idxF);
                
	            and1 = clearWireForReuse(and1, idxF);
	            outputGateNoInvertOutput(8, xorab, xorac, and1, idxF);
                
	            carry = clearWireForReuse(carry, idxF);
	            outputGateNoInvertOutput(6, xortout, and1, carry, idxF);
            }
            
	        if (xortout->refs == 0) clearWireForReuse(xortout, idxF);
	        if (xorab->refs == 0) clearWireForReuse(xorab, idxF);
	        if (xorac->refs == 0) clearWireForReuse(xorac, idxF);
	        if (and1->refs == 0) clearWireForReuse(and1, idxF);
        }
        
	    t = invertWireNoInvertOutput(remainw[length - 1], idxF);
        
        
        if(!IsModDiv)
        {
            ldest[(length-1)-i] = t;
        }
    }
    
    /*get modulus*/
    if(IsModDiv)
    {
        
        for(int i=0;i<length;i++)
        {
            ldest[i] = remainw[i];
        }
        
        vector<Wire *> addDest;
        addDest.resize(ldest.size());
	    outputAddition(&ldest, &lright, addDest, idxF);
        
        //cout << "before fail\n";
        for(int i=0;i<ldest.size();i++)
        {
	        assignWireCond(ldest[i], addDest[i], ldest[destv.size()], idxF);
        }
        
        
        
        //signed portion of modulus below:
        vector<Wire *> resultsubDest;
        resultsubDest.resize(origlength+1);
        zeros.resize(origlength+1);
        
        //expand 0 array
	    zeros[origlength] = ZERO_WIRE[ idxF ];
        
	    outputSubtract(&zeros, &ldest, resultsubDest, idxF);
        
        for(i_out=0;i_out<origlength+1;i_out++)
        {
	        assignWireCond(ldest[i_out], resultsubDest[i_out], ifsubtractl, idxF);
        }
    }
    else
    {
        vector<Wire *> resultsubDest;
        resultsubDest.resize(origlength+1);
        zeros.resize(origlength+1);
        
        //expand 0 array
	    zeros[origlength] = ZERO_WIRE[idxF] ;

	    outputSubtract(&zeros, &ldest, resultsubDest, idxF);
        
	    Wire * result = outputGateNoInvertOutput(6, ifsubtractl, ifsubtractr, idxF);
        
        for(i_out=0;i_out<origlength+1;i_out++)
        {
	        assignWireCond(ldest[i_out], resultsubDest[i_out], result, idxF);
        }
        
    }
    
    WireSet * ws = getPool(idxF)->getWires(origlength);
    
    for(int i=0;i<origlength;i++)
    {
        destv[i] = ws->wires[i];
    }
    
    
    /*reduce to original length*/
    for(int i=0;i<origlength;i++)
    {
	    assignWire(destv[i], ldest[i], idxF);
    }
}





//only does right side of multiplication trapazoid (i.e. if your mult is 32 bits by 32 bits we don't need the result of the left 32 bits since it goes back into a 32bit int, not a 64 bit int).
//precondiction - all vectors are of proper size( |leftv| == |rightv| == |destv| ))
//left is x input
//right is y input
//mult algorithm from MIT slides from course 6.111, fall 2012, lecture 8/9, slide 33
void outputExMultSigned(vector<Wire *> * leftv,vector<Wire *> * rightv, vector<Wire *> & destv, int idxF)
{
    
    
    
    int length = leftv->size();
    

    
    //cout << "start outputExMultUnsigned out\n";
    WireSet * ws;
    
    switch(length)
    {
        case 1:
            ws = getPool(idxF)->getWires(2);
            
            for(int i=0;i<2;i++)
            {
                destv[i] = ws->wires[i];
            }
            
        
            
	    assignWire(destv[0], outputGate(8, leftv->operator[](0), rightv->operator[](0), idxF), idxF);
	    assignWire(destv[1], outputGate(6, ZERO_WIRE[idxF], ZERO_WIRE[idxF], idxF), idxF);
            break;
        default:
            
            vector<Wire *> rowoutputs;
            rowoutputs.resize(length);
            vector<Wire *> rowinputsleft;
            rowinputsleft.resize(length);
            vector<Wire *> rowinputsright;
            rowinputsright.resize(length);
            
            vector<Wire *> tdest;
            tdest.resize(destv.size());
            
            Wire * carry = getPool(idxF)->getWire();
            Wire * xor2 = getPool(idxF)->getWire();
            Wire * xorab = getPool(idxF)->getWire();
            Wire * andn1 = getPool(idxF)->getWire();
            Wire * andn2 = getPool(idxF)->getWire();
            
            
            //number of rows
            for(int i=0;i<length-1;i++)
            {
                //cout << "new header\n";
                
                if(i != 0)
                {
	                rowinputsleft[length - 1] = clearWireForReuse(rowinputsleft[length - 1], idxF);
	                assignWire(rowinputsleft[length - 1], carry, idxF);
                }
                
                //create inputs to each adder
                if(i == 0)
                {
                    
                    for(int k=0;k<length;k++)
                    {
	                    rowinputsleft[k] = outputGate(8, leftv->operator[](k), rightv->operator[](0), idxF);
                    }
                    //only on first row do we do this
                    
                    for(int k=0;k<length;k++)
                    {
	                    rowinputsright[k] = outputGate(8, leftv->operator[](k), rightv->operator[](1), idxF);
                    }
                    
                    
                    tdest[0] = getPool(idxF)->getWire();
	                assignWire(tdest[0], rowinputsleft[0], idxF);
                    
                    //shift down
                    for(int k=0;k<length-1;k++)
                    {
	                    assignWire(rowinputsleft[k], rowinputsleft[k + 1], idxF);
                    }
	                rowinputsleft[length - 1] = clearWireForReuse(rowinputsleft[length - 1], idxF);
	                assignWire(rowinputsleft[length - 1], ONE_WIRE[idxF], idxF);
                    
	                rowinputsleft[length - 2] = invertWireNoAllocUnlessNecessary(rowinputsleft[length - 2], idxF);
                    
	                rowinputsright[length - 1] = invertWireNoAllocUnlessNecessary(rowinputsright[length - 1], idxF);
                    
                }
                else
                {
                    for(int k=0;k<length;k++)
                    {
	                    rowinputsright[k] = clearWireForReuse(rowinputsright[k], idxF);
	                    outputGate(8, leftv->operator[](k), rightv->operator[](i + 1), rowinputsright[k], idxF);
                    }
                    
                    if(i != length-2)
                    {
	                    rowinputsright[length - 1] = invertWireNoAllocUnlessNecessary(rowinputsright[length - 1], idxF);
                    }
                    else
                    {
                        for(int k=0;k<length-1;k++)
                        {
	                        rowinputsright[k] = invertWireNoAllocUnlessNecessary(rowinputsright[k], idxF);
                        }
                    }
                }
                
                //cout << "new row\n";
                
                //create each adder
                for(int j=0;j<length;j++)
                {
                    
                    
                    //output half adder
                    if(j == 0)
                    {
	                    xorab = clearWireForReuse(xorab, idxF);
	                    outputGate(6, rowinputsright[0], rowinputsleft[0], xorab, idxF);
                        
                        tdest[i+1] = pool[idxF].getWire();
	                    assignWire(tdest[i + 1], xorab, idxF);
                        
	                    carry = clearWireForReuse(carry, idxF);
                        
                        
	                    outputGate(8, rowinputsright[0], rowinputsleft[0], carry, idxF);
                        
                        
                    }
                    else //output full adder
                    {
	                    xorab = clearWireForReuse(xorab, idxF);
	                    outputGate(6, rowinputsright[j], rowinputsleft[j], xorab, idxF);
                        
	                    rowinputsleft[j - 1] = clearWireForReuse(rowinputsleft[j - 1], idxF);
	                    outputGate(6, xorab, carry, rowinputsleft[j - 1], idxF);
                        
                        //if(j < length-1)
                        {
	                        andn2 = clearWireForReuse(andn2, idxF);
	                        outputGate(6, carry, rowinputsleft[j], andn2, idxF);
                            
	                        andn1 = clearWireForReuse(andn1, idxF);
	                        outputGate(8, xorab, andn2, andn1, idxF);
                            
	                        carry = clearWireForReuse(carry, idxF);
	                        outputGate(6, rowinputsleft[j], andn1, carry, idxF);
                        }
                    }
                    
	                if (andn2->refs == 0) clearWireForReuse(andn2, idxF);
	                if (andn1->refs == 0) clearWireForReuse(andn1, idxF);
	                if (xorab->refs == 0) clearWireForReuse(xorab, idxF);
                }
                
                
                
            }
            
            for(int i=0;i<length-1;i++)
            {
                tdest[i+length] =  rowinputsleft[i];
            }
            
            //cout << "beingx\n";
            tdest[length+length-1] = carry;
            
            
            
            Wire * temp = getPool(idxF)->getWire();
	    assignWire(temp, tdest[length + length - 1], idxF);
            
	    makeWireContainValue(temp, idxF);
	    assignWire(tdest[length + length - 1], ZERO_WIRE[idxF], idxF);
            
	    outputGateNoInvertOutput(6, temp, ONE_WIRE[idxF], tdest[length + length - 1], idxF);
            
            ws = getPool(idxF)->getWires(destv.size());
            
            for(int i=0;i<destv.size();i++)
            {
                destv[i] = ws->wires[i];
	            assignWire(destv[i], tdest[i], idxF);
            }
            
            
            break;
    }
    
    
    
    //cout << "end outputExMultUnsigned out\n";

}


void outputExMultUnsigned(vector<Wire *> * leftv,vector<Wire *> * rightv, vector<Wire *> & destv, int idxF)
{
    int length = leftv->size();
    
    
    //cout << "start outputExMultUnsigned out\n";
    
    WireSet * ws;
    
    switch(length)
    {
        case 1:
            ws = getPool(idxF)->getWires(2);
            
            for(int i=0;i<2;i++)
            {
                destv[i] = ws->wires[i];
            }
            
            
            
	    assignWire(destv[0], outputGate(8, leftv->operator[](0), rightv->operator[](0), idxF), idxF);
	    assignWire(destv[1], outputGate(6, ZERO_WIRE[idxF], ZERO_WIRE[idxF], idxF), idxF);
            break;
        default:
            
            vector<Wire *> rowoutputs;
            rowoutputs.resize(length);
            vector<Wire *> rowinputsleft;
            rowinputsleft.resize(length);
            vector<Wire *> rowinputsright;
            rowinputsright.resize(length);
            
            vector<Wire *> tdest;
            tdest.resize(destv.size());
            
            Wire * carry = getPool(idxF)->getWire();
            Wire * xor2 = getPool(idxF)->getWire();
            Wire * xorab = getPool(idxF)->getWire();
            Wire * andn1 = getPool(idxF)->getWire();
            Wire * andn2 = getPool(idxF)->getWire();
            
            //number of rows
            for(int i=0;i<length-1;i++)
            {
                //cout << "new header\n";
                
                if(i != 0)
                {
	                rowinputsleft[length - 1] = clearWireForReuse(rowinputsleft[length - 1], idxF);
	                assignWire(rowinputsleft[length - 1], carry, idxF);
                }
                
                //create inputs to each adder
                if(i == 0)
                {
                    
                    for(int k=0;k<length;k++)
                    {
	                    rowinputsleft[k] = outputGate(8, leftv->operator[](k), rightv->operator[](0), idxF);
                    }
                    //only on first row do we do this
                    
                    for(int k=0;k<length;k++)
                    {
	                    rowinputsright[k] = outputGate(8, leftv->operator[](k), rightv->operator[](1), idxF);
                    }
                    
                    
                    tdest[0] = getPool(idxF)->getWire();
	                assignWire(tdest[0], rowinputsleft[0], idxF);
                    
                    //shift down
                    for(int k=0;k<length-1;k++)
                    {
	                    assignWire(rowinputsleft[k], rowinputsleft[k + 1], idxF);
                    }
	                rowinputsleft[length - 1] = clearWireForReuse(rowinputsleft[length - 1], idxF);
	                assignWire(rowinputsleft[length - 1], ZERO_WIRE[idxF], idxF);

                }
                else
                {
                    for(int k=0;k<length;k++)
                    {
	                    rowinputsright[k] = clearWireForReuse(rowinputsright[k], idxF);
	                    outputGate(8, leftv->operator[](k), rightv->operator[](i + 1), rowinputsright[k], idxF);
                    }

                    
                }
                
                //cout << "new row\n";
                
                //create each adder
                for(int j=0;j<length;j++)
                {
                    
                    
                    //output half adder
                    if(j == 0)
                    {
	                    xorab = clearWireForReuse(xorab, idxF);
	                    outputGate(6, rowinputsright[0], rowinputsleft[0], xorab, idxF);
                        
                        tdest[i+1] = pool[idxF].getWire();
	                    assignWire(tdest[i + 1], xorab, idxF);
                        
	                    carry = clearWireForReuse(carry, idxF);
                        

	                    outputGate(8, rowinputsright[0], rowinputsleft[0], carry, idxF);
                        
                        
                    }
                    else //output full adder
                    {
	                    xorab = clearWireForReuse(xorab, idxF);
	                    outputGate(6, rowinputsright[j], rowinputsleft[j], xorab, idxF);
                        
	                    rowinputsleft[j - 1] = clearWireForReuse(rowinputsleft[j - 1], idxF);
	                    outputGate(6, xorab, carry, rowinputsleft[j - 1], idxF);
                        
                        //if(j < length-1)
                        {
	                        andn2 = clearWireForReuse(andn2, idxF);
	                        outputGate(6, carry, rowinputsleft[j], andn2, idxF);
                            
	                        andn1 = clearWireForReuse(andn1, idxF);
	                        outputGate(8, xorab, andn2, andn1, idxF);
                            
	                        carry = clearWireForReuse(carry, idxF);
	                        outputGate(6, rowinputsleft[j], andn1, carry, idxF);
                        }
                    }
                    
	                if (andn2->refs == 0) clearWireForReuse(andn2, idxF);
	                if (andn1->refs == 0) clearWireForReuse(andn1, idxF);
	                if (xorab->refs == 0) clearWireForReuse(xorab, idxF);
                }
                

                
            }
            
            //cout << "lengths: "<<destv.size()<<" "<<(length+length-1)<<"\n";
            
            for(int i=0;i<length-1;i++)
            {
                tdest[i+length] =  rowinputsleft[i];
            }
            tdest[length+length-1] = carry;
            
            ws = getPool(idxF)->getWires(destv.size());
            
            for(int i=0;i<destv.size();i++)
            {
                destv[i] = ws->wires[i];
	            assignWire(destv[i], tdest[i], idxF);
            }
            
            break;
    }
    
    //cout << "end outputExMultUnsigned out\n";
}

//leftv - dividend, rightv - divisor
void outputReDivideUnsigned(vector<Wire *> * leftv,vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv, int length_in, int idxF)
{
    
    vector<Wire *> R_left;
    vector<Wire *> R_right;
    
    vector<Wire *> quotient;
    vector<Wire *> remainder;
    
    
    int lengthOfOp = length_in+1;
    int lengthLeft = lengthOfOp*2-1;
    
    
    R_left.resize(lengthLeft);
    R_right.resize(lengthOfOp);
    quotient.resize(lengthOfOp);
    remainder.resize(lengthOfOp);
    
    int lssize = leftv->size();
    
    
    WireSet * ws = getPool(idxF)->getWires(lssize);
    
    for(int i=0;i<lssize;i++)
    {
        R_left[i] = ws->wires[i];
    }
    
    int j;
    for(j=0;j<lssize;j++)
    {
	    assignWire(R_left[j], leftv->operator[](j), idxF);
    }
    for(;j<R_left.size();j++)
    {
        R_left[j] = ZERO_WIRE[idxF];
    }
    
    lssize = rightv->size();
    
    
    ws = getPool(idxF)->getWires(lssize);
    
    for(int i=0;i<lssize && i < R_right.size();i++)
    {
        R_right[i] = ws->wires[i];
    }
    
    for(j=0;j<lssize && j < R_right.size();j++)
    {
	    assignWire(R_right[j], rightv->operator[](j), idxF);
    }
    for(;j<R_right.size();j++)
    {
        R_right[j] = ZERO_WIRE[idxF];
    }
    
    
    vector<Wire *> rowinputsright;
    vector<Wire *> rowinputsleft;
    
    rowinputsright.resize(lengthOfOp);
    rowinputsleft.resize(lengthOfOp);
    
    
    
    vector<Wire *> heldresults;
    
    heldresults.resize(lengthOfOp);
    for(int i=0;i<heldresults.size();i++)
    {
        heldresults[i] =getPool(idxF)->getWire();
    }

        
    
    Wire * CASxor = getPool(idxF)->getWire();
    Wire * carry = getPool(idxF)->getWire();
    Wire * xor2 = getPool(idxF)->getWire();
    Wire * xorab = getPool(idxF)->getWire();
    Wire * andn1 = getPool(idxF)->getWire();
    Wire * andn2 = getPool(idxF)->getWire();
    
    
	Wire * T = ONE_WIRE[idxF];
    
    
    for(int i=0;i<lengthOfOp;i++)
    {
        rowinputsright[i] = R_right[i];
    }
    
    
    int diff = R_left.size() - rowinputsright.size();
    for(int i=0;i<lengthOfOp;i++)
    {
        rowinputsleft[i] =getPool(idxF)->getWire();
	    assignWire(rowinputsleft[i], R_left[i + diff], idxF);
    }
    
    
    /*printwirevec(R_right);
    cout <<"\n";
    printwirevec(R_left);*/
    
    
    //cout << "begin gates\n";
    
    //cout << "loo: "<<lengthOfOp<<"\nLL: "<<lengthLeft<<"\n";
    
    for(int row=0;row<lengthOfOp;row++)
    {
        //cout << "row: "<<row<<"\n";
        
	    assignWire(rowinputsleft[0], R_left[(lengthOfOp - 1) - row], idxF);
        //cout << "RL0 = R_L-"<<(lengthOfOp -1)-row<<"\n";
        
	    assignWire(carry, T, idxF);
        for(int col=0;col<lengthOfOp;col++)
        {
         //    cout << "col: "<<col<<"\n";
            
	        CASxor = clearWireForReuse(CASxor, idxF);
	        outputGateNoInvertOutput(6, T, rowinputsright[col], CASxor, idxF);
            
            
	        xorab = clearWireForReuse(xorab, idxF);
	        outputGateNoInvertOutput(6, CASxor, rowinputsleft[col], xorab, idxF);
            
	        heldresults[col] = clearWireForReuse(heldresults[col], idxF);
	        outputGateNoInvertOutput(6, xorab, carry, heldresults[col], idxF);

	        andn2 = clearWireForReuse(andn2, idxF);
	        outputGateNoInvertOutput(6, carry, CASxor, andn2, idxF);
            
	        andn1 = clearWireForReuse(andn1, idxF);
	        outputGateNoInvertOutput(8, xorab, andn2, andn1, idxF);
            
	        carry = clearWireForReuse(carry, idxF);
	        outputGateNoInvertOutput(6, CASxor, andn1, carry, idxF);
            
            
            
            
            
	        if (andn2->refs == 0) clearWireForReuse(andn2, idxF);
	        if (andn1->refs == 0) clearWireForReuse(andn1, idxF);
	        if (xorab->refs == 0) clearWireForReuse(xorab, idxF);
        }
        
        
        
        for(int i=0;i<lengthOfOp-1;i++)
        {
	        assignWire(rowinputsleft[i + 1], heldresults[i], idxF);
           // cout << "RL"<<i+1<<" = held-"<<i+1<<"\n";
        }
        
        
        
        T = heldresults[lengthOfOp-1];
	    T = invertWireNoInvertOutput(T, idxF);
        quotient[lengthOfOp-row-1] = T;
      //  cout << "outing: "<< lengthOfOp-row-1<<"\n";
    }

    
    
    //cout << "begin copy output\n";
    
    //cout << "IsModDiv: "<<IsModDiv<<"\n";
    
    if(!IsModDiv)
    {
        
        ws = getPool(idxF)->getWires(destv.size());
        
        for(int i=0;i<destv.size();i++)
        {
            destv[i] = ws->wires[i];
	        assignWire(destv[i], quotient[i], idxF);
        }
        
        /*for(int i=0;i<destv.size();i++)
        {
            destv[i] = quotient[i];
        }*/
    }
    else
    {
        vector<Wire *> addDest;
        addDest.resize(heldresults.size());
        
        //cout << "sizes: "<<heldresults.size()<<" "<<R_right.size()<<" "<<addDest.size()<<"\n";
        
	    outputAddition(&heldresults, &R_right, addDest, idxF);
        
        for(int i=0;i<heldresults.size();i++)
        {
	        assignWireCond(heldresults[i], addDest[i], heldresults[destv.size()], idxF);
        }
        
        
        /*for(int i=0;i<destv.size();i++)
        {
            destv[i] = heldresults[i]; //remainder[i];
        }*/
        
        ws = getPool(idxF)->getWires(destv.size());
        
        for(int i=0;i<destv.size();i++)
        {
            destv[i] = ws->wires[i];
	        assignWire(destv[i], heldresults[i], idxF);
        }

    }
    
    //cout << "end outputReDivideUnsigned\n";
}



//leftv - dividend, rightv - divisor
void outputReDivideSigned(vector<Wire *> * leftv,vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv, int length_in, int idxF)
{

    vector<Wire *> R_left;
    vector<Wire *> R_right;
    
    vector<Wire *> quotient;
    vector<Wire *> remainder;
    
    
    int lengthOfOp = length_in+1;
    int lengthLeft = lengthOfOp*2-1;
    
    
    R_left.resize(lengthLeft);
    R_right.resize(lengthOfOp);
    quotient.resize(lengthOfOp);
    remainder.resize(lengthOfOp);
    
    int lssize = leftv->size();
    
    
    
    WireSet * ws = getPool(idxF)->getWires(lssize);
    
    for(int i=0;i<lssize;i++)
    {
        R_left[i] = ws->wires[i];
    }
    
    int j;
    for(j=0;j<lssize;j++)
    {
	    assignWire(R_left[j], leftv->operator[](j), idxF);
    }
    for(;j<R_left.size();j++)
    {
        R_left[j] = ZERO_WIRE[idxF];
    }
    
    lssize = rightv->size();
    
    ws = getPool(idxF)->getWires(lssize);
    
    for(int i=0;i<lssize && i < R_right.size();i++)
    {
        R_right[i] = ws->wires[i];
    }
    
    for(j=0;j<lssize && j < R_right.size();j++)
    {
	    assignWire(R_right[j], rightv->operator[](j), idxF);
    }
    for(;j<R_right.size();j++)
    {
        R_right[j] = ZERO_WIRE[idxF];
    }
    
    
    /*cout <<"first\n";
    printwirevec(R_right);
    cout <<"\n";
    printwirevec(R_left);
    cout <<"\n";*/
 
    
    
    
    int origlength = leftv->size();
    int origlengthx = rightv->size();
    
    
    //cout << origlength<< "\n" << origlengthx<<"\n"<< R_left.size()<<"\n"<<R_right.size()<<"\n";
    
    
    Wire * ifsubtractl = getPool(idxF)->getWire();
    Wire * ifsubtractr = getPool(idxF)->getWire();
    
    //cout << "second\n";
    
	assignWire(ifsubtractl, R_left[origlength - 1], idxF);
	assignWire(ifsubtractr, R_right[origlengthx - 1], idxF);
    
    //cout << "third\n";
    
    //printWire(ifsubtractl);
    
    vector<Wire *> zerosleft, zerosright;
    vector<Wire *> subDestr,subDestl;
    subDestr.resize(origlengthx);
    subDestl.resize(origlength);
    zerosleft.resize(origlength);
    zerosright.resize(origlengthx);
    
    //cout << "forth\n";
    
    for(int i=0;i<origlength;i++)
    {
        zerosleft[i] = ZERO_WIRE[idxF];
    }
    for(int i=0;i<origlengthx;i++)
    {
        zerosright[i] = ZERO_WIRE[idxF];
    }
    
    //cout << "fifth\n";
    
	outputSubtract(&zerosleft, leftv, subDestl, idxF);
	outputSubtract(&zerosright, rightv, subDestr, idxF);
    
    
    /*cout << R_left.size()<<"\n";
    cout << subDestl.size()<<"\n";
    cout << origlength<<"\n";
    
    cout << R_right.size()<<"\n";
    cout << subDestr.size()<<"\n";
    cout << origlengthx<<"\n";

    cout << length_in<<"\n";*/
    
    for(int i_out=0;i_out<origlength;i_out++)
    {
	    assignWireCond(R_left[i_out], subDestl[i_out], ifsubtractl, idxF);
    }
    for(int i_out=0;i_out<origlengthx;i_out++)
    {
	    assignWireCond(R_right[i_out], subDestr[i_out], ifsubtractr, idxF);
    }

    
    /*cout <<"mid\n";
    printwirevec(R_right);
    cout <<"\n";
    printwirevec(R_left);
    cout <<"\n";
    printwirevec(subDestl);
    cout <<"\n";*/
    
    
    vector<Wire *> rowinputsright;
    vector<Wire *> rowinputsleft;
    
    rowinputsright.resize(lengthOfOp);
    rowinputsleft.resize(lengthOfOp);
    
    
    
    vector<Wire *> heldresults;
    
    heldresults.resize(lengthOfOp);
    for(int i=0;i<heldresults.size();i++)
    {
        heldresults[i] =getPool(idxF)->getWire();
    }
    
    
    
    Wire * CASxor = getPool(idxF)->getWire();
    Wire * carry = getPool(idxF)->getWire();
    Wire * xor2 = getPool(idxF)->getWire();
    Wire * xorab = getPool(idxF)->getWire();
    Wire * andn1 = getPool(idxF)->getWire();
    Wire * andn2 = getPool(idxF)->getWire();
    
    
	Wire * T = ONE_WIRE[idxF];
    
    
    
    for(int i=0;i<lengthOfOp;i++)
    {
        rowinputsright[i] = R_right[i];
    }
    
    
    int diff = R_left.size() - rowinputsright.size();
    for(int i=0;i<lengthOfOp;i++)
    {
        rowinputsleft[i] =getPool(idxF)->getWire();
	    assignWire(rowinputsleft[i], R_left[i + diff], idxF);
    }
    
    /*cout <<"last\n";
    printwirevec(R_right);
     cout <<"\n";
     printwirevec(R_left);
    cout <<"\n";
    */
    
    //cout << "begin gates\n";
    
    //cout << "loo: "<<lengthOfOp<<"\nLL: "<<lengthLeft<<"\n";
    
    for(int row=0;row<lengthOfOp;row++)
    {
        //cout << "row: "<<row<<"\n";
        
	    assignWire(rowinputsleft[0], R_left[(lengthOfOp - 1) - row], idxF);
        //cout << "RL0 = R_L-"<<(lengthOfOp -1)-row<<"\n";
        
	    assignWire(carry, T, idxF);
        for(int col=0;col<lengthOfOp;col++)
        {
            //    cout << "col: "<<col<<"\n";
            
	        CASxor = clearWireForReuse(CASxor, idxF);
	        outputGateNoInvertOutput(6, T, rowinputsright[col], CASxor, idxF);
            
            
	        xorab = clearWireForReuse(xorab, idxF);
	        outputGateNoInvertOutput(6, CASxor, rowinputsleft[col], xorab, idxF);
            
	        heldresults[col] = clearWireForReuse(heldresults[col], idxF);
	        outputGateNoInvertOutput(6, xorab, carry, heldresults[col], idxF);
            
	        andn2 = clearWireForReuse(andn2, idxF);
	        outputGateNoInvertOutput(6, carry, CASxor, andn2, idxF);
            
	        andn1 = clearWireForReuse(andn1, idxF);
	        outputGateNoInvertOutput(8, xorab, andn2, andn1, idxF);
            
            carry = clearWireForReuse(carry, idxF);
	        outputGateNoInvertOutput(6, CASxor, andn1, carry, idxF);
            
            
            
            
            
	        if (andn2->refs == 0) clearWireForReuse(andn2, idxF);
	        if (andn1->refs == 0) clearWireForReuse(andn1, idxF);
	        if (xorab->refs == 0) clearWireForReuse(xorab, idxF);
        }
        
        
        
        for(int i=0;i<lengthOfOp-1;i++)
        {
	        assignWire(rowinputsleft[i + 1], heldresults[i], idxF);
            // cout << "RL"<<i+1<<" = held-"<<i+1<<"\n";
        }
        
        
        
        T = heldresults[lengthOfOp-1];
	    T = invertWireNoInvertOutput(T, idxF);
        quotient[lengthOfOp-row-1] = T;
        //  cout << "outing: "<< lengthOfOp-row-1<<"\n";
    }
    
    
    
    //cout << "begin copy output\n";
    
    //cout << "IsModDiv: "<<IsModDiv<<"\n";
    
    if(!IsModDiv)
    {
        
        
        vector<Wire *> resultsubDest;
        resultsubDest.resize(origlengthx+1);
        zerosright.resize(origlengthx+1);
        
        //expand 0 array
        zerosright[origlengthx] = ZERO_WIRE[idxF];
        
        //cout << "fsize: "<<zerosright.size()<<" "<<quotient.size()<<" "<<resultsubDest.size()<<"\n";
        
	    outputSubtract(&zerosright, &quotient, resultsubDest, idxF);
        
	    Wire * result = outputGateNoInvertOutput(6, ifsubtractl, ifsubtractr, idxF);
        
        for(int i_out=0;i_out<origlengthx+1;i_out++)
        {
	        assignWireCond(quotient[i_out], resultsubDest[i_out], result, idxF);
        }
        
       
        
        /*for(int i=0;i<destv.size();i++)
        {
            destv[i] = quotient[i];
        }*/
        
        ws = getPool(idxF)->getWires(destv.size());
        
        for(int i=0;i<destv.size();i++)
        {
            destv[i] = ws->wires[i];
	        assignWire(destv[i], quotient[i], idxF);
        }
        
    }
    else
    {
        
        vector<Wire *> addDest;
        addDest.resize(heldresults.size());
        
        //cout << "sizes: "<<heldresults.size()<<" "<<R_right.size()<<" "<<addDest.size()<<"\n";
        
	    outputAddition(&heldresults, &R_right, addDest, idxF);
        
        for(int i=0;i<heldresults.size();i++)
        {
	        assignWireCond(heldresults[i], addDest[i], heldresults[destv.size()], idxF);
        }
        
        
        //signed portion of modulus below:
        vector<Wire *> resultsubDest;
        resultsubDest.resize(origlengthx+1);
        zerosright.resize(origlengthx+1);
        
        //expand 0 array
        zerosright[origlengthx] = ZERO_WIRE[idxF];
        
	    outputSubtract(&zerosright, &heldresults, resultsubDest, idxF);
        
        for(int i_out=0;i_out<origlengthx+1;i_out++)
        {
	        assignWireCond(heldresults[i_out], resultsubDest[i_out], ifsubtractl, idxF);
        }
        
        /*for(int i=0;i<destv.size();i++)
        {
            destv[i] = heldresults[i]; //remainder[i];
        }*/
        
        ws = getPool(idxF)->getWires(destv.size());
        
        for(int i=0;i<destv.size();i++)
        {
            destv[i] = ws->wires[i];
	        assignWire(destv[i], heldresults[i], idxF);
        }
     
        
    }
    
    
    //cout << "end outputReDivideUnsigned\n";
}






//tables
// 0 - 0000 (0)
// 1 - 0001 (nor)
// 2 - 0010
// 3 - 0011 (invert passthrough a)
// 4 - 0100
// 5 - 0101 (invert passthorugh b)
// 6 - 0110 (xor)
// 7 - 0111 (nand)
// 8 - 1000 (and)
// 9 - 1001 (reverse xor)
// 10 - 1010 (passthrough b)
// 11 - 1011
// 12 - 1100 (passthrough a)
// 13 - 1101
// 14 - 1110 (or)
// 15 - 1111 (1)

bool seeoutput=false;


void makeONEandZERO(ostream & mos, int idxF, bool isMain)
{
    
	one_wire_l[idxF] = currentbasewire[idxF];
	writeGate(15, currentbasewire[idxF], 0, 0, &mos, idxF);
	if(!isMain)
		currentbasewire[idxF]++;
    
	zero_wire_l[idxF] = currentbasewire[idxF];
	writeGate(0, currentbasewire[idxF], 0, 0, &mos, idxF);
	if (!isMain)
		currentbasewire[idxF]++;
    
	ONE_WIRE[idxF] = new Wire();
	ONE_WIRE[idxF]->state = ONE;
	ONE_WIRE[idxF]->wireNumber = one_wire_l[idxF];
    
	ZERO_WIRE[idxF] = new Wire();
	ZERO_WIRE[idxF]->state = ZERO;
	ZERO_WIRE[idxF]->wireNumber = zero_wire_l[idxF];
}

void makeWireContainValueNoONEZEROcopy(Wire * w, int idxF)
{   
    if(w->other == 0 || w->state == UNKNOWN)
    {
        if(w->state == UNKNOWN_INVERT)
        {
	        //writeGate(6, w->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
	        Wire * temp = pool[idxF].getWire();
	        writeGate(3, temp->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
	        writeGate(6, w->wireNumber, temp->wireNumber, ZERO_WIRE[idxF]->wireNumber, idxF);
            w->state = UNKNOWN;
        }
        
        return;
    }
    
	writeCopy(w->wireNumber, w->other->wireNumber, idxF);
	if (isPrintDuploGC)
	{
		w->prevWireNumber[0] = w->other->wireNumber;
		w->prevWireNumber[1] = w->other->prevWireNumber[0];
		
	}
    
    //if(seeoutput) cout << "CP MWCOa "<< w->wireNumber<<" "<<w->other->wireNumber<<"\n";
    
    if(w->state == UNKNOWN_INVERT_OTHER_WIRE)
    {
        w->state = UNKNOWN_INVERT;
        w->other->refs--; w->other->removeRef(w);
        w->other = 0;
    }
    else
    {
        w->state = UNKNOWN;
        w->other->refs--; w->other->removeRef(w);
        w->other = 0;
    }
    
    
    if(w->state == UNKNOWN_INVERT)
    {
	    Wire * temp = pool[idxF].getWire();
	    writeGate(3, temp->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
	    writeGate(6, w->wireNumber, temp->wireNumber, ZERO_WIRE[idxF]->wireNumber, idxF);
        w->state = UNKNOWN;
	   // pool[idxF].freeIfNoRefs();
    }
    
   
}

void makeWireContainValueNoONEZEROcopyForFn(Wire * w, int idxF)
{   
	if (w->other == 0 || w->state == UNKNOWN)
	{
		if (w->state == UNKNOWN_INVERT)
		{
			//writeGate(6, w->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
			Wire * temp = pool[idxF].getWire();
			writeGate(6, temp->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
			writeGate(6, w->wireNumber, temp->wireNumber, ZERO_WIRE[idxF]->wireNumber, idxF);
			w->state = UNKNOWN;
		}
        
		return;
	}
    
	//writeCopy(w->wireNumber, w->other->wireNumber, idxF);
	if (isPrintDuploGC)
	{
		w->prevWireNumber[0] = w->other->wireNumber;
		w->prevWireNumber[1] = w->other->prevWireNumber[0];
		
	}
    
    //if(seeoutput) cout << "CP MWCOa "<< w->wireNumber<<" "<<w->other->wireNumber<<"\n";
    
	if (w->state == UNKNOWN_INVERT_OTHER_WIRE)
	{
		w->state = UNKNOWN_INVERT;
		w->other->refs--; w->other->removeRef(w);
		w->other = 0;
	}
	else
	{
		w->state = UNKNOWN;
		w->other->refs--; w->other->removeRef(w);
		w->other = 0;
	}
    
    
	if (w->state == UNKNOWN_INVERT)
	{
		//writeGate(6, w->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
		Wire * temp = pool[idxF].getWire();
		writeGate(3, temp->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
		writeGate(6, w->wireNumber, temp->wireNumber, ZERO_WIRE[idxF]->wireNumber, idxF);
		w->state = UNKNOWN;
	}
    
}
int MWCV_base_dest;
int MWCV_base_from;
int MWCV_amt=0;
int MWCV_lastnum_dest;
int MWCV_lastnum_from;
bool MWCV_isFirstLine;

void makeWireContainValueNoONEZEROcopyTiny(Wire * w, int idxF)
{
    if(w->other == 0 || w->state == UNKNOWN)
    {
        if(w->state == UNKNOWN_INVERT)
        {

            if(MWCV_amt > 0)
            {
                /* */
                writeComplexGate(5,MWCV_base_dest,MWCV_base_from,0,MWCV_amt,0,0);
                
                //MWCV_base = w->wireNumber;
                //MWCV_amt=1;
                MWCV_amt=0;
                MWCV_isFirstLine=true;
            }
            
	       // writeGate(6, w->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
	        Wire * temp = pool[idxF].getWire();
	        writeGate(3, temp->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
	        writeGate(6, w->wireNumber, temp->wireNumber, ZERO_WIRE[idxF]->wireNumber, idxF);
            w->state = UNKNOWN;
        }
        return;
    }
    
    
    if(MWCV_isFirstLine)
    {
        MWCV_base_dest = w->wireNumber;
        MWCV_base_from = w->other->wireNumber;
        MWCV_amt=1;
        MWCV_isFirstLine=false;
    }
    else
    {
        if((w->wireNumber-1 != MWCV_lastnum_dest) || (w->other->wireNumber-1 != MWCV_lastnum_from)) // print
        {
            /* */
            if(MWCV_amt > 0)
                writeComplexGate(5,MWCV_base_dest,MWCV_base_from,0,MWCV_amt,0,0);
            
            
            MWCV_base_dest = w->wireNumber;
            MWCV_base_from = w->other->wireNumber;
            MWCV_amt=1;
            MWCV_isFirstLine=false;
        }
        else
        {
            MWCV_amt++;
        }
    }
    MWCV_lastnum_dest  = w->wireNumber;
    MWCV_lastnum_from = w->other->wireNumber;
    
    //writeCopy(w->wireNumber,w->other->wireNumber);
    
    if(w->state == UNKNOWN_INVERT_OTHER_WIRE)
    {
        w->state = UNKNOWN_INVERT;
        w->other->refs--; w->other->removeRef(w);
        w->other = 0;
    }
    else
    {
        w->state = UNKNOWN;
        w->other->refs--; w->other->removeRef(w);
        w->other = 0;
    }
    
    if(w->state == UNKNOWN_INVERT)
    {
        /* */
        writeComplexGate(5,MWCV_base_dest,MWCV_base_from,0,MWCV_amt,0,0);
        
        MWCV_isFirstLine=true;
        MWCV_amt=0;
        
	   // writeGate(6, w->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
	    Wire * temp = pool[idxF].getWire();
	    writeGate(3, temp->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
	    writeGate(6, w->wireNumber, temp->wireNumber, ZERO_WIRE[idxF]->wireNumber, idxF);
        w->state = UNKNOWN;
    }
}

void makeWireContainValueNoONEZEROcopyTinyEnd()
{
    /* */
    if(MWCV_amt > 0)
        writeComplexGate(5,MWCV_base_dest,MWCV_base_from,0,MWCV_amt,0,0);
    
    MWCV_isFirstLine = true;
    MWCV_amt=0;
}





Wire * clearWireForReuse(Wire * w, int idxF)
{
    if(w->refs > 0)
    {
	    return pool[idxF].getWire();
    }
    
    w->state = ZERO;
    
    if(w->other != 0)
    {
        w->other->refs--; w->other->removeRef(w);
        w->other = 0;
    }
    return w;
}

void makeWireContainValue(Wire * w, int idxF)
{
    if(w->state == ONE)
    {
	    writeCopy(w->wireNumber, one_wire_l[idxF], idxF);
	    if (isPrintDuploGC)
	    {
		    w->prevWireNumber[0] = one_wire_l[idxF];		
	    }
    }
    if(w->state == ZERO)
    {
	    writeCopy(w->wireNumber, zero_wire_l[idxF], idxF);
	    if (isPrintDuploGC)
	    {
		    w->prevWireNumber[0] = zero_wire_l[idxF];		
	    }
    }
    
    
    if(w->other == 0 || w->state == UNKNOWN)
    {
        return;
    }
    
	writeCopy(w->wireNumber, w->other->wireNumber, idxF);
	
	if (isPrintDuploGC)
	{
		w->prevWireNumber[0] = w->other->wireNumber;
		w->prevWireNumber[1] = w->other->prevWireNumber[0];
		
	}
	
    //if(seeoutput) cout << "CP MWCOb "<< w->wireNumber<<" "<<w->other->wireNumber<<"\n";
    
    if(w->state == UNKNOWN_INVERT_OTHER_WIRE)
    {
        w->state = UNKNOWN_INVERT;
        w->other->refs--; w->other->removeRef(w);
        w->other = 0;
    }
    else
    {
        w->state = UNKNOWN;
        w->other->refs--; w->other->removeRef(w);
        w->other = 0;
    }
    
    
    if(w->state == UNKNOWN_INVERT)
    {
	   // writeGate(6, w->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
	    Wire * temp = pool[idxF].getWire();
	    writeGate(3, temp->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
	    writeGate(6, w->wireNumber, temp->wireNumber, ZERO_WIRE[idxF]->wireNumber, idxF);
        w->state = UNKNOWN;
    }
    
    
}

void makeWireContainValueForFn(Wire * w, int idxF)
{
	if (w->state == ONE)
	{
		//writeCopy(w->wireNumber, one_wire_l[idxF], idxF);
		if (isPrintDuploGC)
		{
			w->prevWireNumber[0] = one_wire_l[idxF];		
		}
	}
	if (w->state == ZERO)
	{
		//writeCopy(w->wireNumber, zero_wire_l[idxF], idxF);
		if (isPrintDuploGC)
		{
			w->prevWireNumber[0] = zero_wire_l[idxF];		
		}
	}
    
    
	if (w->other == 0 || w->state == UNKNOWN)
	{
		return;
	}
    
	//writeCopy(w->wireNumber, w->other->wireNumber, idxF);
	
	if (isPrintDuploGC)
	{
		w->prevWireNumber[0] = w->other->wireNumber;
		w->prevWireNumber[1] = w->other->prevWireNumber[0];
		
	}
	
    //if(seeoutput) cout << "CP MWCOb "<< w->wireNumber<<" "<<w->other->wireNumber<<"\n";
    
	if (w->state == UNKNOWN_INVERT_OTHER_WIRE)
	{
		w->state = UNKNOWN_INVERT;
		w->other->refs--; w->other->removeRef(w);
		w->other = 0;
	}
	else
	{
		w->state = UNKNOWN;
		w->other->refs--; w->other->removeRef(w);
		w->other = 0;
	}
    
    
	if (w->state == UNKNOWN_INVERT)
	{
		//writeGate(6, w->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
		Wire * temp = pool[idxF].getWire();
		writeGate(3, temp->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
		writeGate(6, w->wireNumber, temp->wireNumber, ZERO_WIRE[idxF]->wireNumber, idxF);
		w->state = UNKNOWN;
	}
    
    
}

void makeWireNotOther(Wire * w, int idxF)
{
    if(w->other != 0)
    {
	    writeCopy(w->wireNumber, w->other->wireNumber, idxF);
	    
	    if (isPrintDuploGC)
	    {
		    w->prevWireNumber[0] = w->other->wireNumber;
		    w->prevWireNumber[1] = w->other->prevWireNumber[0];
		
	    }
        
        if(w->state == UNKNOWN_INVERT_OTHER_WIRE)
        {
            w->state = UNKNOWN_INVERT;
        }
        else
        {
            w->state = UNKNOWN;
        }
    }
    
    if(w->state == UNKNOWN_INVERT)
    {
        w->state = UNKNOWN;
	    //writeGate(6, w->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
	    Wire * temp = pool[idxF].getWire();
	    writeGate(3, temp->wireNumber, w->wireNumber, ONE_WIRE[idxF]->wireNumber, idxF);
	    writeGate(6, w->wireNumber, temp->wireNumber, ZERO_WIRE[idxF]->wireNumber, idxF);
        return;
    }
    
}

Wire * outputGate(short table, Wire * a, Wire * b, int idxF)
{
    /*if(table == 9)
    {
        cout << "9 gate\n";
    }*/
    
	Wire * dest = pool[idxF].getWire();
    
    if(shortCut(a,b,table,dest))
    {
        return dest;
    }

    /*if(table != 6)
    {
        cout << "gate\n";
    }*/
    
	return addGate(table, a, b, dest, idxF);
}

void outputGate(short table, Wire * a, Wire * b, Wire * dest, int idxF)
{
    /*if(table == 9)
    {
           cout << "9 gate\n";
    }*/
    
    if(shortCut(a,b,table,dest))
    {
        return;
    }
    
    /*if(table != 6)
    {
        cout << "gate\n";
    }*/

    
	addGate(table, a, b, dest, idxF);
}

Wire * outputGateNoInvertOutput(short table, Wire * a, Wire * b, int idxF)
{
    /*if(table == 9)
    {
        cout << "9 gate\n";
    }*/
    
    Wire * dest = pool[idxF].getWire();
    
    if(shortCutNoInvertOutput(a,b,table,dest))
    {
        return dest;
    }
    
    /*if(table != 6)
    {
        cout << "gate\n";
    }*/
    
	return addGate(table, a, b, dest, idxF);
}

	void outputGateNoInvertOutput(short table, Wire * a, Wire * b, Wire * dest, int idxF)
{
    /*if(table == 9)
    {
        cout << "9 gate\n";
    }*/
    
    if(shortCutNoInvertOutput(a,b,table,dest))
    {
        return;
    }
    
    /*if(table != 6)
    {
        cout << "gate\n";
    }*/
    
	addGate(table, a, b, dest, idxF);
}


void outputFunctionCall(int num)
{
    writeFunctionCall(num, os);
}

void outputFunctionCallDP(int num, string localInp, string globalInp)
{
	//duplo
	if (isPrintDuploGC && isMainFunc)
	{
		//strDuploGC.append("Local Inp: " + localInp + "\n");
		functions_call_in_main++;
		strDuploGC.append("FN " + to_string(num + 1) + "\n");	
		strDuploGC.append(globalInp + "\n");	
	}
	if (isPrintDuploGC && !isMainFunc)
	{
		//strDuploGC.append("Local Inp: " + localInp + "\n");
		//functions_call_in_main++;
		//strDuploGC.append("FN " + to_string(num + 1) + "\n");	
		//strDuploGC.append(globalInp + "\n");	
		strDuploGC.append("++ FN " + to_string(num + 1) + "\n");	
		strDuploGC.append("++ " + globalInp + "\n");
	}
	
}

long co_nonxorgates=0;
long co_xorgates=0;

long getNonXorGates()
{
    return co_nonxorgates;
}

long getXorGates()
{
    return co_xorgates;
}

void incrementCountsBy(long nonxor, long xorg)
{
    co_nonxorgates+=nonxor;
    co_xorgates+=xorg;
}

uint32_t outbuffer[16];

bool printIOTypeWires=true;

void setSeeOutput(bool value)
{
    seeoutput = value;
}
void setPrintIOTypes(bool value)
{
    printIOTypeWires = value;
}


void addComplexOp(short op, int length, int starta, int startb, int startdest, vector<Wire *> a, vector<Wire *> b, vector<Wire *> dest, Wire * carry, int isend, int idxF)
{
    //cout << "testingcomplexop\n";
    
    //cout << "op: "<<op<<"\nlength: "<<length<<"\nstarta: "<<starta<<"\nstartb: "<<startb<<"\nstartdest: "<<startdest<<"\n"<<a[0]->wireNumber<<"\n"<<b[0]->wireNumber<<"\n"<<dest[0]->wireNumber<<"\n" <<carry->wireNumber<<"\n";
    
    
    for(int i=startdest;i<startdest+length;i++)
    {
        Wire * temp = dest[i];
        
        //clear wire
        if(temp->refs > 0)
        {
	        clearReffedWire(temp, idxF);
        }
        if(temp->state == UNKNOWN_OTHER_WIRE || temp->state == UNKNOWN_INVERT_OTHER_WIRE )
        {
            temp->other->refs--; temp->other->removeRef(temp);
            temp->other = 0;
        }
        
        //label as unknown
        temp->state = UNKNOWN;
    }
    
    
    int carryw=0;
    if(carry != 0)
    {
        carryw = carry->wireNumber;
    }
    
    
    //if this the program gets here then the value must be unknown. If it was known in any way (or a reference to another wire) it would have been done in the short circuit function
    writeComplexGate(op,
                     dest[startdest]->wireNumber,
                     a[starta]->wireNumber,
                     b[startb]->wireNumber,
                     length,
                     carryw,isend);
}

void addComplexOpSingleDestBit(short op, int length, int starta, int startb, int startdest, vector<Wire *> a, vector<Wire *> b, vector<Wire *> dest, Wire * carry, int isend, int idxF)
{
    //cout << "testingcomplexop\n";
    
    //cout << "op: "<<op<<"\nlength: "<<length<<"\nstarta: "<<starta<<"\nstartb: "<<startb<<"\nstartdest: "<<startdest<<"\n"<<a[0]->wireNumber<<"\n"<<b[0]->wireNumber<<"\n"<<dest[0]->wireNumber<<"\n" <<carry->wireNumber<<"\n";
    
    
    
    for(int i=0;i<1;i++)
    {
        Wire * temp = dest[i];
        
        //clear wire
        if(temp->refs > 0)
        {
	        clearReffedWire(temp, idxF);
        }
        if(temp->state == UNKNOWN_OTHER_WIRE || temp->state == UNKNOWN_INVERT_OTHER_WIRE )
        {
            temp->other->refs--; temp->other->removeRef(temp);
            temp->other = 0;
        }
        
        //label as unknown
        temp->state = UNKNOWN;
    }
    
    
    int carryw=0;
    if(carry != 0)
    {
        carryw = carry->wireNumber;
    }
    
    
    //if this the program gets here then the value must be unknown. If it was known in any way (or a reference to another wire) it would have been done in the short circuit function
    writeComplexGate(op,
                     dest[0]->wireNumber,
                     a[starta]->wireNumber,
                     b[startb]->wireNumber,
                     length,
                     carryw,isend);
}

void writeComplexGate(ostream & mos,short op, int dest, int x, int y, int length, int carryadd, int isend)
{
    outbuffer[0] = dest;
    outbuffer[1] = 5 << 8;
    outbuffer[2] = x;
    outbuffer[3] = y;
    mos.write((char *)(&outbuffer[0]),16);
    
    outbuffer[0] = carryadd;
    outbuffer[1] = isend;
    outbuffer[2] = length-1;
    outbuffer[3] = op;
    mos.write((char *)(&outbuffer[0]),16);
    
    if(seeoutput) cout <<os<< " Complex op: "<<op <<" "<<dest<<" "<<x<<" "<<y<<" "<<length<<" "<<"\n";
	
	//duplo
	if (isPrintDuploGC) 
	{
		strDuploGC.append(" Complex op: " + to_string(x) + " " + to_string(y) + " " + to_string(dest) + " " + to_string(op) + " " + to_string(length) + "\n");		
		printf("ERROR: exist a complex gate which DUPLO-Frigate cannot handle => Suggest to write your .wir in deffirent way");
		exit(EXIT_FAILURE);	
	}  
}

void writeComplexGate(short op, int dest, int x, int y, int length, int carryadd, int isend)
{
    outbuffer[0] = dest;
    outbuffer[1] = 5 << 8;
    outbuffer[2] = x;
    outbuffer[3] = y;
    os->write((char *)(&outbuffer[0]),16);
    
    outbuffer[0] = carryadd;
    outbuffer[1] = isend;
    outbuffer[2] = length-1;
    outbuffer[3] = op;
    os->write((char *)(&outbuffer[0]),16);
    
    if(seeoutput) cout <<os<< " Complex op: "<<op <<" "<<dest<<" "<<x<<" "<<y<<" "<<length<<" "<<"\n";
	
	 
    if(op == 4)
    {
        co_xorgates+=length;
        //co_nonxorgates++;
    }
    else if(op == 3)
    {
        co_xorgates+=length;
        //co_nonxorgates++;
    }
    else if(op == 0)
    {
        co_xorgates+=length*4;
        co_nonxorgates+=length;
        if(isend)
        {
            co_nonxorgates--;
            co_xorgates-=2;
        }
    }
    else if(op == 1)
    {
        co_xorgates+=length*6;
        co_nonxorgates+=length;
        if(isend)
        {
            co_nonxorgates--;
            co_xorgates-=2;
        }
    }
    else if(op == 2)
    {
        co_xorgates+=length*4;
        co_nonxorgates+=length;
        if(isend)
        {
            co_nonxorgates--;
            co_xorgates-=1;
        }
    }
    else
    {
        /*so it doesn't get stopped*/
        co_nonxorgates++;
        co_xorgates++;
    }
	
	//duplo
	if (isPrintDuploGC) 
	{
		strDuploGC.append(" Complex op: " + to_string(x) + " " + to_string(y) + " " + to_string(dest) + " " + to_string(op) + " " + to_string(length) + "\n");	
		
		printf("ERROR: exist a complex gate which DUPLO-Frigate cannot handle => Suggest to write your .wir in deffirent way");
		exit(EXIT_FAILURE);		
	}  
}

void writeGate(short table, int d, int x, int y, int idxF)
{
    
	outbuffer[0] = d;
	outbuffer[1] = table;
	outbuffer[2] = x;
	outbuffer[3] = y;
	os->write((char *)(&outbuffer[0]), 16);
	if (seeoutput) cout << os << " " << d << " " << table << " " << x << " " << y << "\n";
	
	if (table == 6)
	{
		co_xorgates++;
	}
	else
	{
		co_nonxorgates++;
	}
	
	if (isPrintDuploGC) 
		if (table == 0 )
		{
			strDuploZeroOne[idxF].append(to_string(x) + " " + to_string(y) + " " + to_string(d) + " 0110\n");
		}	
		else if (table == 15)
		{
			strDuploZeroOne[idxF].append(to_string(x) + " " + to_string(y) + " " + to_string(d) + " 1001\n");
		}
		else
		{
			strDuploGC.append( to_string(x) + " " + to_string(y) + " " + to_string(d) + " " + toStrGate(table) + "\n");
			strDuploFn[idxF].append( to_string(x) + " " + to_string(y) + " " + to_string(d) + " " + toStrGate(table) + "\n");
		}
}



void writeGate(short table, int d, int x, int y, ostream * os, int idxF)
{
    
	outbuffer[0] = d;
	outbuffer[1] = table;
	outbuffer[2] = x;
	outbuffer[3] = y;
	os->write((char *)(&outbuffer[0]), 16);
	if (seeoutput) cout << os << " " << d << " " << table << " " << x << " " << y << "\n";
	if (table == 6)
	{
		co_xorgates++;
	}
	else
	{
		co_nonxorgates++;
	}
	
	if (isPrintDuploGC) 
		if (table == 0 )
		{
			strDuploZeroOne[idxF].append(to_string(x) + " " + to_string(y) + " " + to_string(d) + " 0110\n");
		}	
		else if (table == 15)
		{
			strDuploZeroOne[idxF].append(to_string(x) + " " + to_string(y) + " " + to_string(d) + " 1001\n");
		}	
		else
		{
			strDuploGC.append( to_string(x) + " " + to_string(y) + " " + to_string(d) + " " + toStrGate(table) + "\n");
			strDuploFn[idxF].append( to_string(x) + " " + to_string(y) + " " + to_string(d) + " " + toStrGate(table) + "\n");
		}
}

void writeCopy(int to, int from, int idxF)
{
    outbuffer[0] = 0;
    outbuffer[1] = 0x300;
    outbuffer[2] = to;
    outbuffer[3] = from;
    os->write((char *)(&outbuffer[0]),16);
    if(seeoutput) cout <<os<< " CP " <<to << " "<<from<<"\n";
    co_xorgates++;
	
	//duplo
	if (isPrintDuploGC && !isMainFunc)
	{
		strDuploGC.append( to_string(from) + " " + to_string(zero_wire_l[idxF]) + " " + to_string(to) + " 0110 \n");
		strDuploFn[idxF].append( to_string(from) + " " + to_string(zero_wire_l[idxF]) + " " + to_string(to) + " 0110 \n");
	}	
}

void writeFunctionCall(int function, ostream * os)
{
    outbuffer[0] = function;
    outbuffer[1] = 0x400;
    outbuffer[2] = 0;
    outbuffer[3] = 0;
    os->write((char *)(&outbuffer[0]),16);
    if(seeoutput) cout << os<< " FN "<<function<<"\n";
    co_xorgates++;
	
//duplo
	//if (isPrintDuploGC && !isMainFunc)
		//strDuploGC.append("FN " + to_string(function) + "\n");	
	
}


//inOut 0 - input
//inOut 1 - output
//party is party
void outputGate(ostream & mos, long wire, bool inOut, int party)
{
    if(inOut == 0)
    {
        outbuffer[0] = wire;
        outbuffer[1] = 0x100;
        outbuffer[2] = party;
        outbuffer[3] = 0;
        mos.write((char *)(&outbuffer[0]),16);
    }
    else if(inOut == 1)
    {
        outbuffer[0] = wire;
        outbuffer[1] = 0x200;
        outbuffer[2] = party;
        outbuffer[3] = 0;
        mos.write((char *)(&outbuffer[0]),16);
    }

    return;
}

string toFullEntry(string s)
{
    string tr = "";
    tr+="0 0 ";
    tr+=s[3];
    tr+="\n";
    tr+="0 1 ";
    tr+=s[2];
    tr+="\n";
    tr+="1 0 ";
    tr+=s[1];
    tr+="\n";
    tr+="1 1 ";
    tr+=s[0];
    tr+="\n";
    
    return tr;
}


string toTable(int i)
{
    switch(i)
    {
        case 0:
            return toFullEntry("0000");
        case 1:
            return toFullEntry("0001");
        case 2:
            return toFullEntry("0010");
        case 3:
            return toFullEntry("0011");
        case 4:
            return toFullEntry("0100");
        case 5:
            return toFullEntry("0101");
        case 6:
            return toFullEntry("0110");
        case 7:
            return toFullEntry("0111");
        case 8:
            return toFullEntry("1000");
        case 9:
            return toFullEntry("1001");
        case 10:
            return toFullEntry("1010");
        case 11:
            return toFullEntry("1011");
        case 12:
            return toFullEntry("1100");
        case 13:
            return toFullEntry("1101");
        case 14:
            return toFullEntry("1110");
        case 15:
            return toFullEntry("1111");
    }
    return "X";
}


#include <algorithm>
uint32_t functions = 0;


int getNumberOfFunctions()
{
    return functions;
}
void increaseFunctions()
{
    functions++;
}

string functionprefix="";
string getFunctionPrefix()
{
    return functionprefix;
}


vector<ofstream *> fileStack;
int ofstreamstackplace=0;
void pushOutputFile(string s)
{

    ofstream * newstream = new ofstream();
    newstream->open(s,std::ofstream::out |std::ofstream::binary);
    
    if(!(newstream->is_open()))
    {
        cerr << "file open failed for procedure\n"<<"\n";
    }
    
    fileStack.push_back(os);
    os = newstream;

}


void popOutputFile()
{

    if(os != 0)
    {
        os->flush();
        os->close();
        delete os;
    }
    else
    {
        cerr << "file close failed for procedure\n"<<"\n";
    }
    os = fileStack[fileStack.size()-1];
    fileStack.pop_back();

}




void outputCircuit(ProgramListNode * topNode, string outputFilePrefix)
{
    VariableContext * vc = new VariableContext();
    TypeMap * tm = &(getTypeMap());
    
    uint32_t numinputs = 0;
    uint32_t numoutputs = 0;

    ProgramListNode * pln = (ProgramListNode * )topNode;
    
    uint32_t parties;
    
    os = new ofstream();
    
    for(int i=0;i<pln->nodeList.size();i++)
    {
        
        
        
        if(isInputNode(pln->nodeList[i]))
        {
            Type * vartype = getTypeMap()[isTermNode(isInputNode(pln->nodeList[i])->type)->var];
            
            Variable * v=0;
            if(isIntType(vartype))
            {
                v = new IntVariable();
            }
            else if(isUIntType(vartype))
            {
                v = new IntVariable();
            }
            else if(isStructType(vartype))
            {
                v = new StructVariable();
            }
            else if(isArrayType(vartype))
            {
                v = new ArrayVariable();
            }
            
            v->name ="input"+isTermNode(isInputNode(pln->nodeList[i])->party)->num;
            v->type = getTypeMap()[isTermNode(isInputNode(pln->nodeList[i])->type)->var];
            
            if((*vc)["input"+(isTermNode(isInputNode(pln->nodeList[i])->party)->num)] != 0)
            {
                addError(pln->nodeList[i]->nodeLocation->fname,pln->nodeList[i]->nodeLocation->position,"Input for party "+isTermNode(isInputNode(pln->nodeList[i])->party)->num+" is already defined!");
            }
            
            (*vc)["input"+(isTermNode(isInputNode(pln->nodeList[i])->party)->num)]=v;
            
            v->FillInType(false);
            v->FillInDepth(0);
            setSizes(v->wv,0);
	        currentbasewiremain = v->assignPermWires(currentbasewiremain);
            
            for(int i=0;i<v->size();i++)
            {
                getWire(i,v->wv)->state = UNKNOWN;
            }
            numinputs++;
        }
        
        if(isOutputNode(pln->nodeList[i]))
        {
            
            Type * vartype = getTypeMap()[isTermNode(isOutputNode(pln->nodeList[i])->type)->var];
            
            Variable * v=0;
            if(isIntType(vartype))
            {
                v = new IntVariable();
            }
            else if(isUIntType(vartype))
            {
                v = new IntVariable();
            }
            else if(isStructType(vartype))
            {
                v = new StructVariable();
            }
            else if(isArrayType(vartype))
            {
                v = new ArrayVariable();
            }
            
            v->name ="output"+isTermNode(isOutputNode(pln->nodeList[i])->party)->num;
            v->type = vartype;
            
            if((*vc)["output"+(isTermNode(isOutputNode(pln->nodeList[i])->party)->num)] != 0)
            {
                 addError(pln->nodeList[i]->nodeLocation->fname,pln->nodeList[i]->nodeLocation->position,"Output for party "+isTermNode(isOutputNode(pln->nodeList[i])->party)->num+" is already defined!");
            }
            
            (*vc)["output"+(isTermNode(isOutputNode(pln->nodeList[i])->party)->num)]=v;
            
            v->FillInType(false);
            v->FillInDepth(0);
            setSizes(v->wv,0);
	        currentbasewiremain = v->assignPermWires(currentbasewiremain);
            for(int i=0;i<v->size();i++)
            {
                getWire(i,v->wv)->state = ZERO;
            }
            numoutputs++;
        }
        
        if(isPartiesNode(pln->nodeList[i]))
        {
            parties = atoi(isTermNode(isPartiesNode(pln->nodeList[i])->number)->num.c_str());
        }
    }
    
    
    for(int i=0;i<pln->nodeList.size();i++)
    {
        if(isFunctionDeclarationNode(pln->nodeList[i]))
        {
            FunctionDeclarationNode * fdn = isFunctionDeclarationNode(pln->nodeList[i]);
            
            Type * t = getTypeMap()[isTermNode(fdn->name)->var];
            
            FunctionVariable * v = new FunctionVariable();
            v->name =isTermNode(fdn->name)->var;
            v->type = t;
            v->nodeOfName = fdn->name;
            (*vc)["FUNC_VAR_$$_"+v->name] = v;
            
            v->FillInType(false);
            
            functionNameToNode[v->name] = fdn;
            
            v->functionNode = pln->nodeList[i];
            
            
            setSizes(v->return_var,0);
            setSizes(v->args,0);
            for(int j=0;j<v->argsv.size();j++)
            {
                for(int k=0;k<v->argsv[j]->size();k++)
                {
                    getWire(k+v->argsv[j]->wv->startwirenum,v->argsv[j]->wv)->state = UNKNOWN;
                }
            }
//            if(v->returnv != 0)
//            {
//                for(int l=0;l< v->returnv->size();l++)
//                {
//                    getWire(l+v->returnv->wv->startwirenum,v->returnv->wv)->state = UNKNOWN;
//                }
//            }
            
            v->functionNumber = functions++;
        }
    }

    string odir = outputFilePrefix;
	zero_wire_l.resize(functions);
	ONE_WIRE.resize(functions);
	ZERO_WIRE.resize(functions);
	one_wire_l.resize(functions);
	currentbasewire.resize(functions);
	pool.resize(functions);
	strDuploZeroOne.resize(functions);
	strDuploFn.resize(functions);
	

    //output header file
    ofstream mos;
    mos.open(odir+"_cul.mfrig", ios::out|ios::binary);
	if(isPrintDuploGC)
		fDuploGC.open(odir + ".GC");
    
    
    uint32_t ibuffer[50];
    
    //version of file
    ibuffer[0] = 1;
    mos.write((char *)(&ibuffer[0]),4);
    
    //number of parties
    ibuffer[0] = parties;
    mos.write((char *)(&ibuffer[0]),4);
    
    ibuffer[0] = numinputs;
    mos.write((char *)(&ibuffer[0]),4);
    for(int i=0;i<pln->nodeList.size();i++)
    {
        if(isInputNode(pln->nodeList[i]))
        {
            Variable * v;
            v = (*vc)["input"+(isTermNode(isInputNode(pln->nodeList[i])->party)->num)];
            ibuffer[0] = atoi(isTermNode(isInputNode(pln->nodeList[i])->party)->num.c_str());
            ibuffer[1] = v->wirebase;
            ibuffer[2] = v->size();
            mos.write((char *)(&ibuffer[0]),12);

            if(atoi(isTermNode(isInputNode(pln->nodeList[i])->party)->num.c_str()) > parties | atoi(isTermNode(isInputNode(pln->nodeList[i])->party)->num.c_str()) < 1)
            {
                addError(pln->nodeList[i]->nodeLocation->fname,pln->nodeList[i]->nodeLocation->position,"Input party is < 1 or greater than the max parties");
            }
        }
    }
    
    
    vector<int> outputwirenumvec;
    
    ibuffer[0] = numoutputs;
    mos.write((char *)(&ibuffer[0]),4);
    for(int i=0;i<pln->nodeList.size();i++)
    {
        if(isOutputNode(pln->nodeList[i]))
        {
            Variable * v;
            v = (*vc)["output"+(isTermNode(isOutputNode(pln->nodeList[i])->party)->num)];
            
            ibuffer[0] = atoi(isTermNode(isOutputNode(pln->nodeList[i])->party)->num.c_str());
            ibuffer[1] = v->wirebase;
            ibuffer[2] = v->size();
            mos.write((char *)(&ibuffer[0]),12);
            
            if(atoi(isTermNode(isOutputNode(pln->nodeList[i])->party)->num.c_str()) > parties | atoi(isTermNode(isOutputNode(pln->nodeList[i])->party)->num.c_str()) < 1)
            {
                addError(pln->nodeList[i]->nodeLocation->fname,pln->nodeList[i]->nodeLocation->position,"Output party is < 1 or greater than the max parties");
            }
        }
    }
    
    if(parties < 1)
    {
        addError("","","#parties  must be greater than 0");
    }
    
    if(hasError())
    {
        printErrors(std::cout);
        std::cout <<"Errors, exiting... \n";
        exit(1);
    }

    
    for(int sp=0;sp<parties;sp++)
    {
        string s ="output"+to_string(sp);
        Variable * v = (*vc)[s];
        if(v != 0)
        {
            for(int j=0;j<v->size();j++)
            {
                outputwirenumvec.push_back( getWire(j,v->wv)->wireNumber );
            }
        }
    }
    
    //output number of functions
    streampos functionpos = mos.tellp();
    ibuffer[0] = functions;
	

	
    mos.write((char *)(&ibuffer[0]),4);
    
    //saving space for totalneededwires
    streampos posforwirenum = mos.tellp();
    ibuffer[0] = 0;
    mos.write((char *)(&ibuffer[0]),4);
    
    
    int lastnum=-1;
    
    bool isFirstLine=true;
    int base=-1;
    int amt = -1;
    
    //mos << "in\n";
    for(int i=0;i<pln->nodeList.size();i++)
    {
        if(isInputNode(pln->nodeList[i]))
        {
            Variable * v;
            v = (*vc)["input"+(isTermNode(isInputNode(pln->nodeList[i])->party)->num)];
            //cout <<"newvar \n";
            
            isFirstLine=true;
            
            for(int j=0;j<v->size();j++)
            {
                Wire * w = getWire(j,v->wv);
                
                if(!useTinyOutput)
                {
                    outputGate(mos,w->wireNumber,0,atol(isTermNode(isInputNode(pln->nodeList[i])->party)->num.c_str()));
                }
                else
                {
                    if(isFirstLine)
                    {
                        base = w->wireNumber;
                        amt=1;
                        isFirstLine=false;
                    }
                    else
                    {
                        if(w->wireNumber-1 != lastnum) // print
                        {
                            writeComplexGate(mos,3,base,atol(isTermNode(isInputNode(pln->nodeList[i])->party)->num.c_str()),0,amt,0,0);

                            base = w->wireNumber;
                            amt=1;
                            isFirstLine=false;
                        }
                        else
                        {
                            amt++;
                        }
                    }
                    lastnum  = w->wireNumber;
                }
            }
            
            if(useTinyOutput && !isFirstLine)
            {
                writeComplexGate(mos,3,base,atol(isTermNode(isInputNode(pln->nodeList[i])->party)->num.c_str()),0,amt,0,0);
            }
        }
    }
    
    FunctionVariable * vf = (FunctionVariable *) ((*vc)["FUNC_VAR_$$_main"]);
	//don't call main function at the first time 
    writeFunctionCall(vf->functionNumber,&mos);

    for(int i=0;i<pln->nodeList.size();i++)
    {
        if(isOutputNode(pln->nodeList[i]))
        {
            Variable * v;
            v = (*vc)["output"+(isTermNode(isOutputNode(pln->nodeList[i])->party)->num)];
            
            isFirstLine=true;
            
            for(int j=0;j<v->size();j++)
            {
                Wire * w = getWire(j,v->wv);
                w->state = ZERO;
                
                if(!useTinyOutput)
                {
                    outputGate(mos,w->wireNumber,1,atol(isTermNode(isOutputNode(pln->nodeList[i])->party)->num.c_str()));
                }
                else
                {
                    if(isFirstLine)
                    {
                        base = w->wireNumber;
                        amt=1;
                        isFirstLine=false;
                    }
                    else
                    {
                        if(w->wireNumber-1 != lastnum) // print
                        {
                            writeComplexGate(mos,4,base,atol(isTermNode(isOutputNode(pln->nodeList[i])->party)->num.c_str()),0,amt,0,0);

                            base = w->wireNumber;
                            amt=1;
                            isFirstLine=false;
                        }
                        else
                        {
                            amt++;
                        }
                    }
                    lastnum  = w->wireNumber;
                }

            }
            if(useTinyOutput && !isFirstLine)
            {
                writeComplexGate(mos,4,base,atol(isTermNode(isOutputNode(pln->nodeList[i])->party)->num.c_str()),0,amt,0,0);
            }
        }
    }


	int idxFunc=0;

    for(int i=0;i<pln->nodeList.size();i++)
    {
        if(isFunctionDeclarationNode(pln->nodeList[i]))
        {
	        
	      //  idxFunc = functions - 1;
	        if (idxFunc == functions - 1)
		        currentbasewire[idxFunc] = currentbasewiremain;
	        else
		        currentbasewire[idxFunc] = 0;
		        
	        FunctionVariable * v = (FunctionVariable *) ((*vc)["FUNC_VAR_$$_"+isTermNode(isFunctionDeclarationNode(pln->nodeList[i])->name)->var]);
            if(v->args != 0)
            {
	            currentbasewire[idxFunc] = v->assignPermWires(currentbasewire[idxFunc]);
                setSizes(v->args,0);
            }

            //if(v->return_var != 0)
          //  {
	     //       currentbasewire[idxFunc] = v->assignPermWires(currentbasewire[idxFunc]);
          //      setSizes(v->return_var,0);
        //    }
	        makeONEandZERO(mos, idxFunc, idxFunc == functions - 1);
		        pool[idxFunc].assignWireNumbers(currentbasewire[idxFunc]);

			idxFunc++;
	       
        }
    }
    
    
    FindIfContainsProcs ficptraversal;
    
    ficptraversal.traverse(pln);
    
    
    
    int prevlargest=0;
    maxWireValue = 0;
	
    

    //determining recusion and
    vector<vector<Node *> > function_callers;
    function_callers.resize(functions);
    vector<FunctionDeclarationNode *> in_order_functions_matching_functionNumber;
    in_order_functions_matching_functionNumber.resize(functions);
    
    vector<FunctionDeclarationNode *> in_order_functions_matching_functionNumber_perm; //does not change
    in_order_functions_matching_functionNumber_perm.resize(functions);
    
    //sort functions by how they are called for function output purposes (needed for gate counts)
    gatherFunctionCallTraverse gfct;
    
    for(int i=0;i<pln->nodeList.size();i++)
    {
        if(isFunctionDeclarationNode(pln->nodeList[i]))
        {
            FunctionVariable * v = (FunctionVariable *) ((*vc)["FUNC_VAR_$$_"+isTermNode(isFunctionDeclarationNode(pln->nodeList[i])->name)->var]);
            function_callers[v->functionNumber] = gfct.traverse(pln->nodeList[i]);
            
            //remove duplicates from http://stackoverflow.com/questions/1041620/whats-the-most-efficient-way-to-erase-duplicates-and-sort-a-vector
            
            sort( function_callers[v->functionNumber].begin(), function_callers[v->functionNumber].end() );
            function_callers[v->functionNumber].erase( unique( function_callers[v->functionNumber].begin(), function_callers[v->functionNumber].end() ), function_callers[v->functionNumber].end() );
            
            in_order_functions_matching_functionNumber_perm[v->functionNumber] =isFunctionDeclarationNode(pln->nodeList[i]);
            in_order_functions_matching_functionNumber[v->functionNumber] =isFunctionDeclarationNode(pln->nodeList[i]);
            
            
            
            /*cout << functionNameToNode[v->name] <<"\n";
            for (int j=0;j<function_callers[v->functionNumber].size();j++)
            {
                cout << function_callers[v->functionNumber][j]<<"\n";
            }
            cout << "\n";*/
        }
    }
    
    int premfunctions = functions; //since functions changes for procs
    
    
    vector<Node *> functions_in_order;
    for(int i=0;i<premfunctions;i++)
    {
        for(int j=0;j<in_order_functions_matching_functionNumber.size();j++)
        {
            //find
            if(function_callers[j].size() == 0 && in_order_functions_matching_functionNumber[j] != 0)
            {
                Node * n = in_order_functions_matching_functionNumber[j];
                
                //cout << "removing: "<<n<<"\n";
                functions_in_order.push_back(in_order_functions_matching_functionNumber[j]);
                
                for(int k=0;k<function_callers.size();k++)
                {
                    for(int L=0;L<function_callers[k].size();L++)
                    {
                        //remove function from list when we generate it
                        if(function_callers[k][L] == n)
                        {
                            function_callers[k].erase(function_callers[k].begin()+L);
                            break;
                        }
                    }
                }
                
                //remove from in_order_functions_matching_functionNumber and function_callers
                in_order_functions_matching_functionNumber[j] = 0;
                break;
            }
            
        }
    }

    
    for(int i=0;i<function_callers.size();i++)
    {
        if(function_callers[i].size() > 0)
        {
            for(int j=0;j<function_callers[i].size();j++)
            {
                if(!isFunctionDeclarationNode(function_callers[i][j])->givenrecwarning)
                {
                    addError(function_callers[i][j]->nodeLocation->fname,function_callers[i][j]->nodeLocation->position,"Recusion detected in function \""+isTermNode(isFunctionDeclarationNode(function_callers[i][j])->name)->var+ "\"");
                    isFunctionDeclarationNode(function_callers[i][j])->givenrecwarning = true;
                }
                /*if(function_callers[i][j] == in_order_functions_matching_functionNumber_perm[i])
                {
                    addError(in_order_functions_matching_functionNumber_perm[i]->nodeLocation->fname,in_order_functions_matching_functionNumber_perm[i]->nodeLocation->position,"Recusion detected in function \""+isTermNode(in_order_functions_matching_functionNumber_perm[i]->name)->var+"\"");
                }*/
            }
        }
    }
    
    if(hasError())
    {
        printErrors(std::cout);
        std::cout <<"Errors, exiting... \n";
        exit(1);
    }
    
    functionprefix = odir+"_f";
    
	//duplo	
	Node * selectedNode;
    //output each function
	
	idxFunc = 0;
    for(int I=0;I<premfunctions;I++)
    {
       selectedNode = functions_in_order[I];
	    
	   
        
        if(isFunctionDeclarationNode(selectedNode))
        {
	      // idxFunc = functions - 1;
	        
            if(seeoutput) std::cout <<"output function... " << isTermNode(isFunctionDeclarationNode(selectedNode)->name)->var<<endl;
            
            FunctionVariable * v = (FunctionVariable *) ((*vc)["FUNC_VAR_$$_"+isTermNode(isFunctionDeclarationNode(selectedNode)->name)->var]);
 
            
            openOutputFile(odir+"_f"+to_string(v->functionNumber)+".ffrig");

            
            if(selectedNode->getNodeType() == FunctionDeclarationNode_t && isTermNode(isFunctionDeclarationNode(selectedNode)->name)->var != "main")
            {
	            
	            //duplo
	            isMainFunc = false;
	            int startInpWire = -1;
	            int startOutWire = -1;
	            if (v->sizereturn() != 0)
		            startOutWire = getWire(0, v->returnv->wv)->wireNumber;

	            if (v->sizeparam() != 0)
		            startInpWire = getWire(0, v->argsv[0]->wv)->wireNumber;
	            
		          
	            int cur = pool[idxFunc].largestsize ;
	            if (isPrintDuploGC) {
		            strDuploGC.append("\nFN " + to_string(I+1) +  " " + 
															   to_string(v->sizeparam()) + " " +
	            											to_string(v->sizereturn()) + " " );		            
	            }
	            

	            uint32_t posforNumWire = strDuploGC.length();
	            //end-duplo
	            
	        
	            
                VariableContext * vct = new VariableContext();
                
                for ( std::unordered_map<string,Variable *>::iterator it = (*vc).begin(); it!= (*vc).end(); ++it )
                {
                    if(it->second != 0 && it->second->v_enum == Functionp)
                    {
                        (*vct)[it->first] = it->second;
                    }
                }
                
	            //makeONEandZERO(mos, idxFunc);
	            if (isPrintDuploGC) {
		            strDuploGC.append(strDuploZeroOne[idxFunc]);		            
	            }
	            selectedNode->circuitOutput(vct, tm, idxFunc);
	            
	             //duplo
	            if (isPrintDuploGC) {
		            
	            
		            if (premfunctions != 0 && I == 0)
		            {
			            strDuploGC.insert(posforNumWire, to_string(pool[idxFunc].wireNumberValue) +" "+isTermNode(isFunctionDeclarationNode(selectedNode)->name)->var + "\n");
			        }
		            else
			            strDuploGC.insert(posforNumWire, to_string(pool[idxFunc].wireNumberValue)+  " "+isTermNode(isFunctionDeclarationNode(selectedNode)->name)->var +"\n");
	            
		            strDuploGC.append("--end FN " + to_string(I+1) + "--\n");
	            }
	            //end-duplo
                
                delete vct;
            }
            else
            {
	            isMainFunc = true;
	            if (isPrintDuploGC) {
		            strDuploGC.append("\nFN " + to_string(I+1) +  "\n\n");
		            if (premfunctions == 0)
		            {
			           // strDuploGC.append(strDuploZeroOne + "\n");
		            }		           
	            }
	            //makeONEandZERO(mos, idxFunc);
//	            if (isPrintDuploGC) {
//		            strDuploGC.append(strDuploZeroOne[idxFunc]);		            
//	            }
	            selectedNode->circuitOutput(vc, tm, idxFunc);
                if(seeoutput) cout << "printing output\n";
                
                Variable * v;
                int currentoutvecindex = 0;
                
                for(int sp=1;sp<parties+1;sp++)
                {
                    string s ="output"+to_string(sp);
                    
                    //cout << s<<"\n";
                    
                    v = (*vc)[s];
                    if(v != 0)
                    {
                        for(int j=0;j<v->size();j++)
                        {
                            Wire * w = getWire(j,v->wv);
	                        makeWireContainValue(w, idxFunc);
                            /*int shouldbethisoutputwirenum = outputwirenumvec[currentoutvecindex++];
                            if(w->wireNumber != shouldbethisoutputwirenum)
                            {
                                writeCopy(shouldbethisoutputwirenum, w->wireNumber);
                            }*/
                        }
                    }
                }
            }
            
	        getPool(idxFunc)->freeIfNoRefs();
	        pool[idxFunc].printUsedPoolState();
            
	        prevlargest = pool[idxFunc].largestsize;
            
	        pool[idxFunc].freeAll();
	        pool[idxFunc].assignWireNumbers(prevlargest + 1);
	        
	        idxFunc++;
            closeOutputFile();
        }
    }
    
    //write max wire
    mos.seekp(posforwirenum);
    ibuffer[0] = prevlargest;
    mos.write((char *)(&ibuffer[0]),4);
    
    mos.seekp(functionpos);
    ibuffer[0] = functions;
    mos.write((char *)(&ibuffer[0]),4);
    
    mos.close();
    
    
    if(printIOTypeWires)
    {
        
        ofstream otfile(odir+"_otype.mfrig");
        
        otfile << "printing IO type wire numbers:\n";
        
        for(int i=0;i<pln->nodeList.size();i++)
        {
            if(isInputNode(pln->nodeList[i]))
            {
                Variable * v;
                v = (*vc)["input"+(isTermNode(isInputNode(pln->nodeList[i])->party)->num)];
                v->printWithWires(otfile,"",v->wirebase);
            }
        }
        
        for(int i=0;i<pln->nodeList.size();i++)
        {
            if(isOutputNode(pln->nodeList[i]))
            {
                Variable * v;
                v = (*vc)["output"+(isTermNode(isOutputNode(pln->nodeList[i])->party)->num)];
                v->printWithWires(otfile,"",v->wirebase);
            }
        }
        otfile <<"\n";
    }
    
    
	//duplo
	if (isPrintDuploGC)
	{			
		fDuploGC << vf->functionNumber+1 << " " << functions_call_in_main << " " << functions_call_in_main << " // #numberfunction #layer  #numberComponent\n";
		for (int sp = 0; sp < parties; sp++)
		{
			string s = "input" + to_string(sp + 1);
			Variable * v = (*vc)[s];
			if (v != 0)
			{
				//cout << "v->size()in" << v->size() << "\n";
				fDuploGC << v->size() << " ";
			}
			else
				fDuploGC <<"0 ";
		}

		for (int sp = 0; sp < parties; sp++)
		{
			string s = "output" + to_string(sp + 1);
			Variable * v = (*vc)[s];
			if (v != 0)
			{
				//cout << "v->size()out" << v->size() << "\n";
				fDuploGC << v->size() << " ";
			}
			else
				fDuploGC <<"0 ";
		}
		fDuploGC << " //#input_eval #input_const #output_eval #output_const\n\n";	
		
		fDuploGC << strDuploGC;
		fDuploGC.close();
	}
    
	
    //this code is for validating shortCircuit (must be done by own eyes)
    vector<Wire> temp;
    
    Wire othera, otherb;
    othera.state = UNKNOWN;
    otherb.state = UNKNOWN;

    /*{

        {

                
            for(int i=0;i<16;i++)
            {
                
                Wire * Dest, *a= pool[idxF].getWire(), *b = pool[idxF].getWire();
                
                a->state = ONE;
                b->state =  UNKNOWN_OTHER_WIRE;
                
                a = b;
                Dest =a;//=pool[idxF].getWire();
                
                bool isshort;
                isshort = shortCut(a,b,i,Dest);

                if(isshort)
                {
                    cout << toTable(i) <<" "<< WireStateToString(a->state) << "   "<< WireStateToString(b->state) << " :  " << WireStateToString(Dest->state) <<" ";
                    if(Dest->other == a) cout<< "A"<<"\n";
                    else if(Dest->other == b) cout<< "B"<<"\n";

                    else cout<< "\n";

                }
                else
                {
                    cout << toTable(i) <<" "<< WireStateToString(a->state) << "   "<< WireStateToString(b->state) << " :  UNKNOWN" <<"\n";

                    if(Dest->other == a) cout<< "A"<<"\n";
                    if(Dest->other == b) cout<< "B"<<"\n";


                }
                
                Dest->freeRefs();

                pool[idxF].freeIfNoRefs();
                cout << "\n";
            }
        }

        
        pool[idxF].freeIfNoRefs();
    }*/
    
    
    /*for(int l=0;l<6;l++)
    {
        for(int k=0;k<6;k++)
        {
            
            cout << "\n\nTable set: "<<l*6+k<<"\n\n";
            
            for(int i=0;i<16;i++)
            {
                
                
                Wire Dest,a, *b = pool[idxF].getWire();
                
                a.state = intToState(l);
                b->state = intToState(k);
                
                
                a.other = &othera;
                b->other = &otherb;
                othera.addRef(&a);
                otherb.addRef(b);
                
                
                bool isshort;
                
                {
                    isshort = shortCut(&a,b,i,&Dest);
                }
                
                
                if(isshort)
                {
                    cout << toTable(i) <<" "<< WireStateToString(a.state) << "   "<< WireStateToString(b->state) << " :  " << WireStateToString(Dest.state) <<" ";
                    if(Dest.other == &a) cout<< "A"<<"\n";
                    else if(Dest.other == b) cout<< "B"<<"\n";
                    else if(Dest.other == b->other) cout<< "B-other"<<"\n";
                    else if(Dest.other == a.other) cout<< "A-other"<<"\n";
                    else cout<< "\n";
                    
                }
                else
                {
                    cout << toTable(i) <<" "<< WireStateToString(a.state) << "   "<< WireStateToString(b->state) << " :  UNKNOWN" <<"\n";
                    
                    if(Dest.other == &a) cout<< "A"<<"\n";
                    if(Dest.other == b) cout<< "B"<<"\n";
                    if(Dest.other == b->other) cout<< "B-other"<<"\n";
                    if(Dest.other == a.other) cout<< "A-other"<<"\n";
                    
                }
                
                Dest.freeRefs();
                
                pool[idxF].freeIfNoRefs();
                cout << "\n";
            }
        }
        
        
        
        
        
        
        
        pool[idxF].freeIfNoRefs();
    }*/
    
    
    
    /*WireSet * ws = pool[idxF].getWires(32);
    
    
    ws->wires[31]->locked = true;
    cout << "end of function\n";
    
    pool[idxF].freeIfNoRefs();
    cout << "used wires\n";
    pool[idxF].printUsedPoolState();
    cout << "free wires\n";
    pool[idxF].printFreePoolState();*/
   
  

    
    
    delete vc;
}









void openOutputFile(string s)
{
    os->open(s);
}

void closeOutputFile()
{
    if(os->is_open())
    {
        os->close();
    }
}
