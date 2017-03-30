#include "wirepool.h"
#include "innerfunction.h"
#include <string>
#include "wirepool.h"


bool useTinyOutput = false;

bool isMainFunc = false;


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

int currentbasewire = 0;
int maxWireValue;
int functionWireStart;


WirePool * getPool()
{
	return &pool;
}

int co_depth = 0;

int InnerFunction::getDepth()
{
	return co_depth;
}
void InnerFunction::increaseDepth()
{
	co_depth++;
}
void InnerFunction::decreaseDepth()
{
	co_depth--;
}


//this function fills any extra space with 0s -> this should really only be needed for constants as type checking will force it to be correct otherwise.
void InnerFunction::ensureSameSize(vector<Wire *> & w1, vector<Wire *> & w2)
{
	while (w1.size() < w2.size())
	{
		w1.push_back(get_ZERO_WIRE());
	}
	while (w1.size() > w2.size())
	{
		w2.push_back(get_ZERO_WIRE());
	}
}

void InnerFunction::ensureTypedSize(vector<Wire *> & w1, vector<Wire *> & w2, Type * t)
{
	int l;
    
	//for untypechecked nodes - uncommenting this hides some possible errors
	/*if(t == 0)
	{
	    ensureSameSize(w1,w2);
	    return;
	}
	else*/ if (isIntType(t))
	{
		l = isIntType(t)->size;
	}
	else if (isUIntType(t))
	{
		l = isUIntType(t)->size;
	}
	else if (isConstType(t))
	{
		ensureSameSize(w1, w2);
		return;
	}
	else
	{
		ensureSameSize(w1, w2);
		return;
	}
    
	ensureSize(w1, l);
	ensureSize(w2, l);
}

void InnerFunction::ensureTypedSize(vector<Wire *> & w1, Type * t)
{
	int l;
    
	//for untypechecked nodes - uncommenting this hides some possible errors
	/*if(t == 0)
	 {
	 ensureSameSize(w1,w2);
	 return;
	 }
	 else*/ if (isIntType(t))
	{
		l = isIntType(t)->size;
	}
	else if (isUIntType(t))
	{
		l = isUIntType(t)->size;
	}
	else
	{
		cout << "ensureTypedSize should only be used with int and uint types! (but works with constant types)";
		cout << "\nused with: ";
		t->print(cout, 0);
		cout << "\n";
		exit(1);
	}
    
	ensureSize(w1, l);
}

void InnerFunction::ensureSize(vector<Wire *> & w1, int length)
{
	if (w1.size() == length)
	{
		return;
	}
	while (w1.size() < length)
	{
		w1.push_back(get_ZERO_WIRE());
	}
	if (w1.size() > length)
	{
		for (int i = w1.size() - 1; i >= length; i--)
		{
			w1[i]->locked = false;
		}
		w1.resize(length);
	}
}

void InnerFunction::ensureIntVariableToVec(CORV & c)
{
	if (c.var == 0)
	{
		return;
	}

	if (!(c.var->v_enum == Intv))
	{
		cout << "strange: variable is not integer\n";
		return;
	}
    
	IntVariable * v = (IntVariable *) c.var;
    
	c.vec.resize(v->wires.size());
    
	for (int i = 0; i < v->wires.size(); i++)
	{
		c.vec[i] = v->wires[i];
	}
}

void InnerFunction::ensureAnyVariableToVec(CORV & c)
{
	if (c.var == 0)
	{
		return;
	}
    
	if (c.var->v_enum == Intv)
	{
		ensureIntVariableToVec(c);
		return;
	}
    
	Variable * v = (Variable *) c.var;
    
	c.vec.resize(v->size());
    
	int startplace = v->wv->startwirenum;
    
	for (int i = startplace; i < v->size() + startplace; i++)
	{
		c.vec[i - startplace] = getWire(i, v->wv);
	}
}

void InnerFunction::messyUnlock(Variable * cvar)
{
	if (cvar->v_enum == Intv)
	{
		IntVariable * intv = (IntVariable *) cvar;
        
		for (int i = 0; i < intv->wires.size(); i++)
		{
			intv->wires[i]->locked = false;
		}
	}
	else if (cvar->v_enum == Arrayv)
	{
		ArrayVariable * arrayv = (ArrayVariable *) cvar;
        
		for (int i = 0; i < arrayv->av.size(); i++)
		{
			messyUnlock(arrayv->av[i]);
		}
	}
}

void InnerFunction::messyAssignAndCopy(Variable * cvar, Variable * pattern)
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
				assignWire(dest->wires[i], intv->wires[i]);
				makeWireContainValueNoONEZEROcopy(dest->wires[i]);
			}
			else
			{
				assignWire(dest->wires[i], get_ZERO_WIRE());
			}
		}
	}
	else if (cvar->v_enum == Arrayv)
	{
		ArrayVariable * arrayv = (ArrayVariable *) cvar;
		ArrayVariable * dest = (ArrayVariable *) pattern;
        
		for (int i = 0; i < dest->av.size(); i++)
		{
			messyAssignAndCopy(arrayv->av[i], dest->av[i]);
		}
	}
	else if (cvar->v_enum == Structv)
	{
		StructVariable * structv = (StructVariable *) cvar;
		StructVariable * dest = (StructVariable *) pattern;
        
		for (std::unordered_map<string, Variable *>::iterator it = structv->map.begin(); it != structv->map.end(); ++it)
		{
			Variable * v =  (Variable *)(it->second);
			messyAssignAndCopy(v, dest->map[(string)(it->first)]);
		}
	}
	else
	{
		cout << "undefined type in messy messyAssignAndCopy\n";
	}
}


//messyCopy assumes the wv in c.var are not correct and does not use them.
//also assumes pattern is correctly constructed and c.var is patterned after pattern correctly BUT the individual variables might be incorrect lengths (i.e. are from constants)
void InnerFunction::messyAssignAndCopy(CORV & c, Variable * pattern)
{
	if (c.var == 0)
	{
	    //IntVariable * intv = (IntVariable *) c.var;
		IntVariable * dest = (IntVariable *) pattern;
        
		int intvsize = c.vec.size();
        
		for (int i = 0; i < dest->wires.size(); i++)
		{
			if (i < intvsize)
			{
				assignWire(dest->wires[i], c.vec[i]);
				makeWireContainValueNoONEZEROcopy(dest->wires[i]);
			}
			else
			{
				assignWire(dest->wires[i], get_ZERO_WIRE());
			}
		}
		return;
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
				assignWire(dest->wires[i], intv->wires[i]);
				makeWireContainValueNoONEZEROcopy(dest->wires[i]);
			}
			else
			{
				assignWire(dest->wires[i], get_ZERO_WIRE());
			}
		}
	}
	else if (c.var->v_enum == Arrayv)
	{
		ArrayVariable * arrayv = (ArrayVariable *) c.var;
		ArrayVariable * dest = (ArrayVariable *) pattern;
        
		for (int i = 0; i < dest->av.size(); i++)
		{
			messyAssignAndCopy(arrayv->av[i], dest->av[i]);
		}
	}
	else if (c.var->v_enum == Structv)
	{
		StructVariable * structv = (StructVariable *) c.var;
		StructVariable * dest = (StructVariable *) pattern;
        
		for (std::unordered_map<string, Variable *>::iterator it = structv->map.begin(); it != structv->map.end(); ++it)
		{
			Variable * v =  (Variable *)(it->second);
			messyAssignAndCopy(v, dest->map[(string)(it->first)]);
		}
	}
	else
	{
		cout << "undefined type in messy messyAssignAndCopy\n";
	}
}

void InnerFunction::putVariableToVector(CORV & c)
{
	if (c.var == 0)
	{
		return;
	}
    
	c.vec.resize(c.var->size());
    
	int startplace = c.var->wv->startwirenum;
    
	for (int i = 0; i < c.var->size(); i++)
	{
		c.vec[i] = getWire(i + startplace, c.var->wv);
	}
}


void InnerFunction::printwirevec(vector<Wire *> & v)
{
	for (int i = v.size() - 1; i >= 0; i--)
	{
		if (v[i]->state == ONE)
		{
			cout << "1";
		}
		else if (v[i]->state == ZERO)
		{
			cout << "0";
		}
		else
		{
			cout << "-";
		}
	}
}



Wire * InnerFunction::addGate(short table, Wire * a, Wire * b, Wire * dest)
{
    
	int awirenum = a->wireNumber;
	int bwirenum = b->wireNumber;
    
	if (a->state == UNKNOWN_INVERT_OTHER_WIRE)
	{
		awirenum = a->other->wireNumber;
		table = invertTable(false, table);
	}
	else if (a->state == UNKNOWN_OTHER_WIRE)
	{
		awirenum = a->other->wireNumber;
	}
	else if (a->state == UNKNOWN_INVERT)
	{
		table = invertTable(false, table);
	}
    
	if (b->state == UNKNOWN_INVERT_OTHER_WIRE)
	{
		bwirenum = b->other->wireNumber;
		table = invertTable(true, table);
	}
	else if (b->state == UNKNOWN_OTHER_WIRE)
	{
		bwirenum = b->other->wireNumber;
	}
	else if (b->state == UNKNOWN_INVERT)
	{
		table = invertTable(true, table);
	}
    
	if (a->state == ONE)
	{
		awirenum = one_wire_l;
	}
	else if (a->state == ZERO)
	{
		awirenum = zero_wire_l;
	}
    
	if (b->state == ONE)
	{
		bwirenum = one_wire_l;
	}
	else if (b->state == ZERO)
	{
		bwirenum = zero_wire_l;
	}
    
	//if this the program gets here then the value must be unknown. If it was known in any way (or a reference to another wire) it would have been done in the short circuit function
	dest->state = UNKNOWN;
	writeGate(table, dest->wireNumber, awirenum, bwirenum);
	return dest;
}


Wire * InnerFunction::invertWire(Wire * w2)
{
	Wire * w1 = pool.getWire();
    
    
	if (w2->state == ONE)
	{
		w1->state = ZERO;
	}
	else if (w2->state == ZERO)
	{
		w1->state = ONE;
	}
	else if (w2->state == UNKNOWN)
	{
		w1->state = UNKNOWN_INVERT_OTHER_WIRE;
		w1->other = w2;
		w1->other->refs++; w1->other->addRef(w1);
	}
	else if (w2->state == UNKNOWN_OTHER_WIRE)
	{
		w1->state = UNKNOWN_INVERT_OTHER_WIRE;
		w1->other = w2->other;
		w1->other->refs++; w1->other->addRef(w1);
	}
	else if (w2->state == UNKNOWN_INVERT)
	{
		w1->state = UNKNOWN_OTHER_WIRE;
		w1->other = w2;
		w1->other->refs++; w1->other->addRef(w1);
	}
	else if (w2->state == UNKNOWN_INVERT_OTHER_WIRE)
	{
		w1->state = UNKNOWN_OTHER_WIRE;
		w1->other = w2->other;
		w1->other->refs++; w1->other->addRef(w1);
	}
    
	return w1;
}

Wire * InnerFunction::invertWireNoInvertOutput(Wire * w2)
{
	Wire * w1 = pool.getWire();
    
    
	if (w2->state == ONE)
	{
		w1->state = ZERO;
	}
	else if (w2->state == ZERO)
	{
		w1->state = ONE;
	}
	else if (w2->state == UNKNOWN)
	{
		addGate(6, w2, ONE_WIRE, w1);
	}
	else if (w2->state == UNKNOWN_OTHER_WIRE)
	{
		addGate(6, w2->other, ONE_WIRE, w1);
	}
	else if (w2->state == UNKNOWN_INVERT)
	{
		w1->state = UNKNOWN_OTHER_WIRE;
		w1->other = w2;
		w1->other->refs++; w1->other->addRef(w1);
	}
	else if (w2->state == UNKNOWN_INVERT_OTHER_WIRE)
	{
		w1->state = UNKNOWN_OTHER_WIRE;
		w1->other = w2->other;
		w1->other->refs++; w1->other->addRef(w1);
	}
    
	return w1;
}

Wire * InnerFunction::invertWireNoAllocUnlessNecessary(Wire * w2)
{
	if (w2->refs > 0)
	{
		Wire * w1 = invertWire(w2);
		return w1;
	}
    
	if (w2->state == ONE)
	{
		w2->state = ZERO;
	}
	else if (w2->state == ZERO)
	{
		w2->state = ONE;
	}
	else if (w2->state == UNKNOWN)
	{
		w2->state = UNKNOWN_INVERT;
	}
	else if (w2->state == UNKNOWN_OTHER_WIRE)
	{
		w2->state = UNKNOWN_INVERT_OTHER_WIRE;
	}
	else if (w2->state == UNKNOWN_INVERT)
	{
		w2->state = UNKNOWN;
	}
	else if (w2->state == UNKNOWN_INVERT_OTHER_WIRE)
	{
		w2->state = UNKNOWN_OTHER_WIRE;
	}

	return w2;
}

void InnerFunction::clearReffedWire(Wire * w)
{
	if (w->refs == 0)
		return;
    
	Wire * newwire = pool.getWire();
	writeCopy(newwire->wireNumber, w->wireNumber);

    
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
void InnerFunction::assignWire(Wire *  w1, Wire * w2)
{
	if (w1 == w2)
	{
		return;
	}
    
    
	if (w1->refs == 1 && w2->other == w1)
	{
		if (w2->state == UNKNOWN_OTHER_WIRE)
		{
			w1->state = UNKNOWN;
			w1->refs--; w1->removeRef(w2);
			w2->other = 0;
            
		}
		else if (w2->state == UNKNOWN_INVERT_OTHER_WIRE)
		{
			w1->state = UNKNOWN_INVERT;
			w1->refs--; w1->removeRef(w2);
			w2->other = 0;
            
		}
		else
		{
			cout << "error in assign wire with refs, strange\n";
		}
		return;
	}
	else if (w1->refs > 0)
	{
		clearReffedWire(w1);
	}
    
	//clear w1
	if (w1->state == UNKNOWN_OTHER_WIRE || w1->state == UNKNOWN_INVERT_OTHER_WIRE)
	{
		w1->other->refs--; w1->other->removeRef(w1);
		w1->other = 0;
	}
    
    
	if (w2->state == ONE)
	{
		w1->state = ONE;
	}
	else if (w2->state == ZERO)
	{
		w1->state = ZERO;
	}
	else if (w2->state == UNKNOWN)
	{
		w1->state = UNKNOWN_OTHER_WIRE;
		w1->other = w2;
		w1->other->refs++; w1->other->addRef(w1);
	}
	else if (w2->state == UNKNOWN_OTHER_WIRE)
	{
		w1->state = UNKNOWN_OTHER_WIRE;
		w1->other = w2->other;
		w1->other->refs++; w1->other->addRef(w1);
	}
	else if (w2->state == UNKNOWN_INVERT)
	{
		w1->state = UNKNOWN_INVERT_OTHER_WIRE;
		w1->other = w2;
		w1->other->refs++; w1->other->addRef(w1);
	}
	else if (w2->state == UNKNOWN_INVERT_OTHER_WIRE)
	{
		w1->state = UNKNOWN_INVERT_OTHER_WIRE;
		w1->other = w2->other;
		w1->other->refs++; w1->other->addRef(w1);
	}
}

//assigns w2 to w1 if w3 is true
void InnerFunction::assignWireCond(Wire *  w1, Wire * w2, Wire * w3)
{
	if (w1 == w2)
	{
		return;
	}
    


	Wire * xor1o = outputGate(6, w2, w1);
	Wire * and1o = outputGate(8, xor1o, w3);
    
	if (w1->refs > 0)
	{
		clearReffedWire(w1);
	}
	else if (w1->other != 0)
	{
	    //cout << "other in assign cond is not 0\n";
		makeWireContainValue(w1);
		/*clearOtherdWire(w1);
		w1->other = 0;
		cout << "other in assign cond is not 0\n";*/
	}
    
	outputGate(6, w1, and1o, w1);
}


ofstream * os;



Wire * InnerFunction::get_ONE_WIRE()
{
	return ONE_WIRE;
}
Wire * InnerFunction::get_ZERO_WIRE()
{
	return ZERO_WIRE;
}





/*
 
    Circuit templates
 
 */


//precondiction - all vectors are of proper size( |leftv| == |rightv| and |destv| == 1)
void InnerFunction::outputEquals(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv)
{
	Wire * outputwire;
	Wire * lastxor;
	Wire * lastlastxor;
	Wire * lastand;
	Wire * currentxor;
    
	Wire * t;
    
	for (int i = 0; i < leftv->size(); i++)
	{
		t = invertWireNoInvertOutput(leftv->operator[](i));
        
		currentxor = outputGateNoInvertOutput(6, t, rightv->operator[](i));
        
		if (i > 1)
		{
			outputwire = outputGate(8, currentxor, lastand);
		}
		else if (i == 1)
		{
			outputwire = outputGate(8, currentxor, lastxor);
		}
		else if (i == 0)
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
void InnerFunction::outputLessThanSigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv)
{
	int length = leftv->size();
    
	Wire * outputwire;
    
	int inlength = leftv->size();

    
	switch (length)
	{
	case 1:
		destv[0] = outputGate(4, rightv->operator[](0), leftv->operator[](0));
		outputwire = destv[0];
		break;
	default:
            
		Wire * carry = getPool()->getWire();
		Wire * xorab = getPool()->getWire();
		Wire * xorac = getPool()->getWire();
		Wire * and1 = getPool()->getWire();
            
		Wire * na = getPool()->getWire();
            
		carry->state = ZERO;
        
        
		length++;
        
		leftv->resize(length);
		leftv->operator[](length - 1) = leftv->operator[](length - 2);
		rightv->resize(length);
		rightv->operator[](length - 1) = rightv->operator[](length - 2);
        
		if (!useTinyOutput)
		{
			for (int i = 0; i < length; i++)
			{
				na = invertWireNoInvertOutput(leftv->operator[](i));
                    
				xorab = clearWireForReuse(xorab);
				outputGate(6, rightv->operator[](i), na, xorab);
                    
				if (i < length - 1)
				{
                        
					xorac = clearWireForReuse(xorac);
					outputGate(6, carry, na, xorac);
                        
					and1 = clearWireForReuse(and1);
					outputGateNoInvertOutput(8, xorab, xorac, and1);
                        
					carry = clearWireForReuse(carry);
					outputGateNoInvertOutput(6, na, and1, carry);
				}
				else
				{
					Wire * t = invertWireNoInvertOutput(xorab);
                        
					Wire * d = outputGateNoInvertOutput(6, t, carry);
					//Wire * d = outputGate(9,xorab,carry);
					outputwire = d;
					destv[0] = outputwire;
				}
                    
			}
		}
		else
		{
			Wire * d = getPool()->getWire();
			destv[0] = d;
                
			for (int i = 0; i < length; i++)
			{
			    //cout << i<<"\n";
                    
				na = invertWireNoInvertOutput(leftv->operator[](i));
                    
				xorab = clearWireForReuse(xorab);
				outputGate(6, rightv->operator[](i), na, xorab);
                    
				if (i < length - 1)
				{
                        
					xorac = clearWireForReuse(xorac);
					outputGate(6, carry, na, xorac);
                        
					and1 = clearWireForReuse(and1);
					outputGateNoInvertOutput(8, xorab, xorac, and1);
                        
					carry = clearWireForReuse(carry);
					outputGateNoInvertOutput(6, na, and1, carry);
				}
				else
				{
					Wire * t = invertWireNoInvertOutput(xorab);
                        
					outputGateNoInvertOutput(6, t, carry, d);
					outputwire = d;
				}
                    
				if (carry->state == UNKNOWN)
				{
					int L = 0;
					for (L = i + 1; L < length; L++)
					{
						if (leftv->operator[](L)->state != UNKNOWN || rightv->operator[](L)->state != UNKNOWN || (leftv->operator[](L - 1)->wireNumber + 1 != leftv->operator[](L)->wireNumber) || (rightv->operator[](L - 1)->wireNumber + 1 != rightv->operator[](L)->wireNumber))
						{
							break;
						}
					}
                        
					int size = L - (i + 1);
                        
					//cout << "startout\n";
                        
					if (size > 0)
					{
					    //cout <<"outlarg";
                            
						addComplexOpSingleDestBit(2, size, i + 1, i + 1, 0, (*leftv), (*rightv), destv, carry, (i + size) == length - 1);
                            
						i += size;
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
	if (inlength != leftv->size() || inlength != rightv->size())
	{
		cerr << "Error in leftv length in lessthan\n";
	}

}


//precondiction - all vectors are of proper size( |leftv| == |rightv| and |destv| >= 1 (should be == 1 but > will suffice))
//subtracts leftv from rightv
void InnerFunction::outputLessThanUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv)
{
	int length = leftv->size();
    
	Wire * outputwire;
    
	int inlength = leftv->size();
    
    
	switch (length)
	{
	case 1:
		destv[0] = outputGate(4, rightv->operator[](0), leftv->operator[](0));
		outputwire = destv[0];
		break;
	default:
            
		Wire * carry = getPool()->getWire();
		Wire * xorab = getPool()->getWire();
		Wire * xorac = getPool()->getWire();
		Wire * and1 = getPool()->getWire();
            
		Wire * na = getPool()->getWire();
            
		carry->state = ZERO;
            
            
		length++;
            
		leftv->resize(length);
		leftv->operator[](length - 1) = ZERO_WIRE;
		rightv->resize(length);
		rightv->operator[](length - 1) = ZERO_WIRE;
            
		if (!useTinyOutput)
		{
			for (int i = 0; i < length; i++)
			{
				na = invertWireNoInvertOutput(leftv->operator[](i));
                    
				xorab = clearWireForReuse(xorab);
				outputGate(6, rightv->operator[](i), na, xorab);
                    
				if (i < length - 1)
				{
                        
					xorac = clearWireForReuse(xorac);
					outputGate(6, carry, na, xorac);
                        
					and1 = clearWireForReuse(and1);
					outputGateNoInvertOutput(8, xorab, xorac, and1);
                        
					carry = clearWireForReuse(carry);
					outputGateNoInvertOutput(6, na, and1, carry);
				}
				else
				{
					Wire * t = invertWireNoInvertOutput(xorab);
                        
					Wire * d = outputGateNoInvertOutput(6, t, carry);
					//Wire * d = outputGate(9,xorab,carry);
					outputwire = d;
					destv[0] = outputwire;
				}
                    
			}
		}
		else
		{
			Wire * d = getPool()->getWire();
			destv[0] = d;
                
			for (int i = 0; i < length; i++)
			{
			    //cout << i<<"\n";
                    
				na = invertWireNoInvertOutput(leftv->operator[](i));
                    
				xorab = clearWireForReuse(xorab);
				outputGate(6, rightv->operator[](i), na, xorab);
                    
				if (i < length - 1)
				{
                        
					xorac = clearWireForReuse(xorac);
					outputGate(6, carry, na, xorac);
                        
					and1 = clearWireForReuse(and1);
					outputGateNoInvertOutput(8, xorab, xorac, and1);
                        
					carry = clearWireForReuse(carry);
					outputGateNoInvertOutput(6, na, and1, carry);
				}
				else
				{
					Wire * t = invertWireNoInvertOutput(xorab);
                        
					outputGateNoInvertOutput(6, t, carry, d);
					outputwire = d;
				}
                    
				if (carry->state == UNKNOWN)
				{
					int L = 0;
					for (L = i + 1; L < length; L++)
					{
						if (leftv->operator[](L)->state != UNKNOWN || rightv->operator[](L)->state != UNKNOWN || (leftv->operator[](L - 1)->wireNumber + 1 != leftv->operator[](L)->wireNumber) || (rightv->operator[](L - 1)->wireNumber + 1 != rightv->operator[](L)->wireNumber))
						{
							break;
						}
					}
                        
					int size = L - (i + 1);
                        
					//cout << "startout\n";
                        
					if (size > 0)
					{
						addComplexOpSingleDestBit(2, size, i + 1, i + 1, 0, (*leftv), (*rightv), destv, carry, (i + size) == length - 1);
                            
						i += size;
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
	if (inlength != leftv->size() || inlength != rightv->size())
	{
		cerr << "Error in leftv length in lessthan\n";
	}
    
}

//precondiction - all vectors are of proper size( |leftv| == |rightv| == |destv| ))
void InnerFunction::outputSubtract(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv)
{
	int length = leftv->size();
    
	/*printwirevec(*leftv);
	cout <<"\n";
	printwirevec(*rightv);
	cout <<"\n";*/
    
	switch (length)
	{
	case 1:
		destv[0] = outputGate(6, rightv->operator[](0), leftv->operator[](0));
		break;
	default:
            
            
		Wire * carry = getPool()->getWire();
		Wire * xorab = getPool()->getWire();
		Wire * xorac = getPool()->getWire();
		Wire * and1 = getPool()->getWire();
            
		Wire * na = getPool()->getWire();
            
		carry->state = ZERO;
            
		WireSet * ws = getPool()->getWires(length);
            
		for (int i = 0; i < length; i++)
		{
			destv[i] = ws->wires[i];
		}
            
		if (!useTinyOutput)
		{
			for (int i = 0; i < length; i++)
			{
				na = invertWireNoInvertOutput(leftv->operator[](i));
                    
				xorab = clearWireForReuse(xorab);
				outputGate(6, rightv->operator[](i), na, xorab);
                    
				Wire * t = invertWireNoInvertOutput(xorab);
                    
				outputGateNoInvertOutput(6, t, carry, destv[i]);
                    
				//outputGate(9,xorab,carry, destv[i]);
				//destv[i] = d;
                    
				if (i < length - 1)
				{
                        
					xorac = clearWireForReuse(xorac);
					outputGate(6, carry, na, xorac);
                        
					and1 = clearWireForReuse(and1);
					outputGateNoInvertOutput(8, xorab, xorac, and1);
                        
					carry = clearWireForReuse(carry);
					outputGateNoInvertOutput(6, na, and1, carry);
				}
                    
			}
		}
		else
		{
			for (int i = 0; i < length; i++)
			{
				na = invertWireNoInvertOutput(leftv->operator[](i));
                    
				xorab = clearWireForReuse(xorab);
				outputGate(6, rightv->operator[](i), na, xorab);
                    
				Wire * t = invertWireNoInvertOutput(xorab);
                    
				outputGateNoInvertOutput(6, t, carry, destv[i]);
                    
				//outputGate(9,xorab,carry, destv[i]);
				//destv[i] = d;
                    
				if (i < length - 1)
				{
                        
					xorac = clearWireForReuse(xorac);
					outputGate(6, carry, na, xorac);
                        
					and1 = clearWireForReuse(and1);
					outputGateNoInvertOutput(8, xorab, xorac, and1);
                        
					carry = clearWireForReuse(carry);
					outputGateNoInvertOutput(6, na, and1, carry);
				}
                    
				if (carry->state == UNKNOWN)
				{
					int L = 0;
					for (L = i + 1; L < length; L++)
					{
						if (leftv->operator[](L)->state != UNKNOWN || rightv->operator[](L)->state != UNKNOWN || (leftv->operator[](L - 1)->wireNumber + 1 != leftv->operator[](L)->wireNumber) || (rightv->operator[](L - 1)->wireNumber + 1 != rightv->operator[](L)->wireNumber))
						{
							break;
						}
					}
                        
					int size = L - (i + 1);
                        
					if (size > 0)
					{
						addComplexOp(1, size, i + 1, i + 1, i + 1, (*leftv), (*rightv), destv, carry, (i + size) == length - 1);
                            
						i += size;
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
void InnerFunction::outputAddition(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv)
{
	int length = leftv->size();
    
	switch (length)
	{
	case 1:
		destv[0] = outputGate(6, rightv->operator[](0), leftv->operator[](0));
		break;
	default:
	    //new adder
		Wire * carry = getPool()->getWire();
		Wire * xorab = getPool()->getWire();
		Wire * xorac = getPool()->getWire();
		//Wire * xorabxc = getPool()->getWire();
		Wire * and1 = getPool()->getWire();
            
		carry->state = ZERO;
            
		WireSet * ws = getPool()->getWires(length);
            
		for (int i = 0; i < length; i++)
		{
			destv[i] = ws->wires[i];
		}
            
            
		if (!useTinyOutput)
		{
			for (int i = 0; i < length; i++)
			{
				xorab = clearWireForReuse(xorab);
				outputGateNoInvertOutput(6, rightv->operator[](i), leftv->operator[](i), xorab);
                    
				/*Wire * d = outputGate(6,xorab,carry);
				destv[i] = d;*/
				outputGate(6, xorab, carry, destv[i]);
                    
				if (i < length - 1)
				{
                        
					xorac = clearWireForReuse(xorac);
					outputGate(6, carry, leftv->operator[](i), xorac);
                        
					and1 = clearWireForReuse(and1);
					outputGateNoInvertOutput(8, xorab, xorac, and1);
                        
					carry = clearWireForReuse(carry);
					outputGateNoInvertOutput(6, leftv->operator[](i), and1, carry);
				}
                    
			}
		}
		else
		{

                
			for (int i = 0; i < length; i++)
			{
				xorab = clearWireForReuse(xorab);
				outputGateNoInvertOutput(6, rightv->operator[](i), leftv->operator[](i), xorab);
                    
				outputGate(6, xorab, carry, destv[i]);
                    
				if (i < length - 1)
				{
                        
					xorac = clearWireForReuse(xorac);
					outputGate(6, carry, leftv->operator[](i), xorac);
                        
					and1 = clearWireForReuse(and1);
					outputGateNoInvertOutput(8, xorab, xorac, and1);
                        
					carry = clearWireForReuse(carry);
					outputGateNoInvertOutput(6, leftv->operator[](i), and1, carry);
				}
                    
				if (carry->state == UNKNOWN)
				{
					int L = 0;
					for (L = i + 1; L < length; L++)
					{
						if (leftv->operator[](L)->state != UNKNOWN || rightv->operator[](L)->state != UNKNOWN || (leftv->operator[](L - 1)->wireNumber + 1 != leftv->operator[](L)->wireNumber) || (rightv->operator[](L - 1)->wireNumber + 1 != rightv->operator[](L)->wireNumber))
						{
							break;
						}
					}
                        
					int size = L - (i + 1);
                        
					if (size > 0)
					{
						addComplexOp(0, size, i + 1, i + 1, i + 1, (*leftv), (*rightv), destv, carry, (i + size) == length - 1);
                            
						i += size;
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
void InnerFunction::outputMultSigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv)
{
	int length = leftv->size();
    
    
	switch (length)
	{
	case 1:
		destv[0] = outputGate(8, leftv->operator[](0), rightv->operator[](0));
		break;
	default:
            
		vector<Wire *> rowoutputs;
		rowoutputs.resize(length);
		vector<Wire *> rowinputsleft;
		rowinputsleft.resize(length);
		vector<Wire *> rowinputsright;
		rowinputsright.resize(length);
            
		Wire * carry = getPool()->getWire();
		Wire * xor2 = getPool()->getWire();
		Wire * xorab = getPool()->getWire();
		Wire * andn1 = getPool()->getWire();
		Wire * andn2 = getPool()->getWire();
            
		WireSet * ws = getPool()->getWires(length);
            
		for (int i = 0; i < length; i++)
		{
			destv[i] = ws->wires[i];
		}
            
            
		//length--;
            
		//number of rows
		for (int i = 0; i < length - 1; i++)
		{
		    //cout << "headerstart\n";
                
		    //create inputs to each adder
			if (i == 0)
			{

				for (int k = 0; k < length - i; k++)
				{
				    //cout << "> gate\n";
					rowinputsleft[k] = outputGate(8, leftv->operator[](k), rightv->operator[](0));
				}
				//only on first row do we do this
				rowinputsleft[length - 1] = invertWireNoAllocUnlessNecessary(rowinputsleft[length - 1]);
                    
				for (int k = 0; k < length - i - 1; k++)
				{
				   // cout << "> gate\n";
					rowinputsright[k] = outputGate(8, leftv->operator[](k), rightv->operator[](1));
				}
                    
                    
				//destv[0] = getPool()->getWire();
                    
				assignWire(destv[0], rowinputsleft[0]);
                    
				//shift down
				for (int k = 0; k < length - 1; k++)
				{
					assignWire(rowinputsleft[k], rowinputsleft[k + 1]);
				}
                    
				if (i == length - 2)
				{
					rowinputsright[0] = invertWireNoAllocUnlessNecessary(rowinputsright[0]);
				}
			}
			else
			{

                    
				for (int k = 0; k < length - 1 - i; k++)
				{
				    //cout << "> gate\n";
					rowinputsright[k] = clearWireForReuse(rowinputsright[k]);
					outputGate(8, leftv->operator[](k), rightv->operator[](i + 1), rowinputsright[k]);
				}
                    
				//last row
				if (i == length - 2)
				{
					rowinputsright[0] = invertWireNoAllocUnlessNecessary(rowinputsright[0]);
				}
                    
			}

			                //cout << "rowstart\n";
			                //create each adder
			for (int j = 0; j < length - i - 1; j++)
			{
			    //performs the HA or FA
                    

                    
			                        //output half adder
				if (j == 0)
				{
					xorab = clearWireForReuse(xorab);
					outputGate(6, rowinputsright[0], rowinputsleft[0], xorab);

					                        //destv[i+1] = pool.getWire();
					assignWire(destv[i + 1], xorab);
				   // cout << "> gate\n";
					carry = clearWireForReuse(carry);
                        
					if (i != length - 2)
					{
						outputGate(8, rowinputsright[0], rowinputsleft[0], carry);
					}

				}
				else //output full adder
				{
				    //cout << "start FA\n";
                        
					xorab = clearWireForReuse(xorab);
					outputGate(6, rowinputsright[j], rowinputsleft[j], xorab);
                        
					rowinputsleft[j - 1] = clearWireForReuse(rowinputsleft[j - 1]);
					outputGate(6, xorab, carry, rowinputsleft[j - 1]);
                        
					if (j < length - 1 - i - 1)
					{
					    //cout << "full adder\n";
                            
						andn2 = clearWireForReuse(andn2);
						outputGate(6, carry, rowinputsleft[j], andn2);
						//cout << "> gate\n";
						andn1 = clearWireForReuse(andn1);
						outputGate(8, xorab, andn2, andn1);
                            
						carry = clearWireForReuse(carry);
						outputGate(6, rowinputsleft[j], andn1, carry);
					}
				}
                    
				if (andn2->refs == 0) clearWireForReuse(andn2);
				if (andn1->refs == 0) clearWireForReuse(andn1);
				if (xorab->refs == 0) clearWireForReuse(xorab);
				if (andn2->refs == 0) clearWireForReuse(andn2);
				if (andn1->refs == 0) clearWireForReuse(andn1);
				if (xorab->refs == 0) clearWireForReuse(xorab);
				if (andn2->refs == 0) clearWireForReuse(andn2);
				if (andn1->refs == 0) clearWireForReuse(andn1);
				if (xorab->refs == 0) clearWireForReuse(xorab);
			}
		}
		break;
	}
    
	//cout << destv.size()<<"\n";
	/*cout << rightv->operator[](destv.size()-1)->state <<  leftv->operator[](destv.size()-1)->state << destv[destv.size()-1]->state <<"\n";
    
	printwirevec(*rightv);cout <<"\n";
	printwirevec(*leftv); cout <<"\n";
	printwirevec(destv); cout <<"\n";*/
    
	//destv[leftv->size()-1] = pool.getWire();
	//outputGate(6,leftv->operator[](leftv->size()-1),rightv->operator[](leftv->size()-1),destv[leftv->size()-1]);
    
}


void InnerFunction::outputMultUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv)
{
	int length = leftv->size();
    
    
	switch (length)
	{
	case 1:
		destv[0] = outputGate(8, leftv->operator[](0), rightv->operator[](0));
		break;
	default:
            
		vector<Wire *> rowoutputs;
		rowoutputs.resize(length);
		vector<Wire *> rowinputsleft;
		rowinputsleft.resize(length);
		vector<Wire *> rowinputsright;
		rowinputsright.resize(length);
            
		Wire * carry = getPool()->getWire();
		Wire * xor2 = getPool()->getWire();
		Wire * xorab = getPool()->getWire();
		Wire * andn1 = getPool()->getWire();
		Wire * andn2 = getPool()->getWire();
            
            
		WireSet * ws = getPool()->getWires(length);
            
		for (int i = 0; i < length; i++)
		{
			destv[i] = ws->wires[i];
		}
            
            
		//number of rows
		for (int i = 0; i < length - 1; i++)
		{
		    //create inputs to each adder
			if (i == 0)
			{
                    
				for (int k = 0; k < length - i; k++)
				{
					rowinputsleft[k] = outputGate(8, leftv->operator[](k), rightv->operator[](0));
				}
				//only on first row do we do this
				//rowinputsleft[length-1] = invertWireNoAllocUnlessNecessary(rowinputsleft[length-1]);
                    
				for (int k = 0; k < length - i - 1; k++)
				{
					rowinputsright[k] = outputGate(8, leftv->operator[](k), rightv->operator[](1));
				}
                    
                    
				//destv[0] = getPool()->getWire();
                    
				assignWire(destv[0], rowinputsleft[0]);
                    
				//shift down
				for (int k = 0; k < length - 1; k++)
				{
					assignWire(rowinputsleft[k], rowinputsleft[k + 1]);
				}
                    
				/*if(i == length-2)
				{
				    rowinputsright[0] = invertWireNoAllocUnlessNecessary(rowinputsright[0]);
				}*/
			}
			else
			{
                    
                    
				for (int k = 0; k < length - 1 - i; k++)
				{
					rowinputsright[k] = clearWireForReuse(rowinputsright[k]);
					outputGate(8, leftv->operator[](k), rightv->operator[](i + 1), rowinputsright[k]);
				}
                    
				//last row
				if (i == length - 2)
				{
				    //rowinputsright[0] = invertWireNoAllocUnlessNecessary(rowinputsright[0]);
				}
                    
			}
                
			//create each adder
			for (int j = 0; j < length - i - 1; j++)
			{
			    //performs the HA or FA
                    
                    
                    
			    //output half adder
				if (j == 0)
				{
					xorab = clearWireForReuse(xorab);
					outputGate(6, rowinputsright[0], rowinputsleft[0], xorab);
                        
					//destv[i+1] = pool.getWire();
					assignWire(destv[i + 1], xorab);
                        
					carry = clearWireForReuse(carry);
                        
					if (i != length - 2)
					{
						outputGate(8, rowinputsright[0], rowinputsleft[0], carry);
					}
                        
				}
				else //output full adder
				{
					xorab = clearWireForReuse(xorab);
					outputGate(6, rowinputsright[j], rowinputsleft[j], xorab);
                        
					rowinputsleft[j - 1] = clearWireForReuse(rowinputsleft[j - 1]);
					outputGate(6, xorab, carry, rowinputsleft[j - 1]);
                        
					if (j < length - 1 - i - 1)
					{
						andn2 = clearWireForReuse(andn2);
						outputGate(6, carry, rowinputsleft[j], andn2);
                            
						andn1 = clearWireForReuse(andn1);
						outputGate(8, xorab, andn2, andn1);
                            
						carry = clearWireForReuse(carry);
						outputGate(6, rowinputsleft[j], andn1, carry);
					}
				}
                    
				if (andn2->refs == 0) clearWireForReuse(andn2);
				if (andn1->refs == 0) clearWireForReuse(andn1);
				if (xorab->refs == 0) clearWireForReuse(xorab);
				if (andn2->refs == 0) clearWireForReuse(andn2);
				if (andn1->refs == 0) clearWireForReuse(andn1);
				if (xorab->refs == 0) clearWireForReuse(xorab);
				if (andn2->refs == 0) clearWireForReuse(andn2);
				if (andn1->refs == 0) clearWireForReuse(andn1);
				if (xorab->refs == 0) clearWireForReuse(xorab);
			}
		}
		break;
	}
}


vector<Wire *> xWires;
int xwirefront;
int xwireback;





//leftv - dividend, rightv - divisor
void InnerFunction::outputDivideUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv)
{
	int origlength = leftv->size();
	int length = leftv->size() + 1;
    
	Wire * carry = getPool()->getWire();
	Wire * xorab = getPool()->getWire();
	Wire * xorac = getPool()->getWire();
	Wire * and1 = getPool()->getWire();
    
	Wire * xortout = getPool()->getWire();
    
	Wire * t = get_ONE_WIRE();
    
	vector<Wire *> inputx, inputy, remainw;
	vector<Wire *> lleft, lright, ldest;
    
	lleft.resize(origlength);
	lright.resize(origlength);
	ldest.resize(length);
    
	int i_out = 0;
	for (; i_out < origlength; i_out++)
	{
		lleft[i_out] = leftv->operator[](i_out);
		lright[i_out] = rightv->operator[](i_out);
	}
    
	/*extend extra bit for correctness purposes*/
	lleft.resize(length);
	lright.resize(length);

	for (; i_out < length; i_out++)
	{
		lleft[i_out] = get_ZERO_WIRE();
		lright[i_out] = get_ZERO_WIRE();
	}

	inputx.resize(length);
	inputy.resize(length);
	remainw.resize(length);
    
	//divisor
	for (int i = 0; i < length; i++)
	{
	    //printWire(lleft[i]);
	    //printWire(lright[i]);
        
		inputx[i] = lright[i];
		inputy[i] = getPool()->getWire();
	}
    
	vector<Wire *> keepwiresA;
	vector<Wire *> keepwiresB;
    
    
	for (int i = 0; i < length; i++)
	{
	    /* setting each input row*/
		if (i == 0)
		{
		    //remain in / dividend
			inputy[0] = lleft[length - 1];
            
			for (int j = 1; j < length; j++)
			{
				assignWire(inputy[j], get_ZERO_WIRE());
			}
		}
		else
		{
			inputy[0] = lleft[length - 1 - i];
			for (int j = 1; j < length; j++)
			{
				assignWire(inputy[j], remainw[j - 1]);
			}
		}

		        //carry in
		if (i == 0)
		{
			carry->state = ONE;
		}
		else
		{
			assignWire(carry, t);
		}
        
        
		/*controlled add / subtract*/
		for (int j = 0; j < length; j++)
		{
			xortout = clearWireForReuse(xortout);
			outputGateNoInvertOutput(6, t, inputx[j], xortout);
            
			xorab = clearWireForReuse(xorab);
			outputGateNoInvertOutput(6, inputy[j], xortout, xorab);
            
			//full adder part
			if (remainw[j] != 0)
			{
                
			    //save wires from layer i so they can be used at layer i+1 (otherwise each remainw wire requires a new wire, very inefficient)
				if (remainw[j]->refs > 0)
				{
					if (i % 2 == 0)
					{
						keepwiresA.push_back(remainw[j]);
						if (keepwiresB.size() > 0)
						{
							remainw[j] = keepwiresB.back();
							keepwiresB.pop_back();
						}
						else
						{
							remainw[j] = getPool()->getWire();
						}
					}
					else
					{
						keepwiresB.push_back(remainw[j]);
						if (keepwiresA.size() > 0)
						{
							remainw[j] = keepwiresA.back();
							keepwiresA.pop_back();
						}
						else
						{
							remainw[j] = getPool()->getWire();
						}
					}
					if (remainw[j] == 0) remainw[j] = getPool()->getWire();
				}
                
    
                
				remainw[j] = clearWireForReuse(remainw[j]);
				outputGate(6, xorab, carry, remainw[j]);
			}
			else
			{
				remainw[j] = outputGate(6, xorab, carry);
			}
            
            
			if (j < length - 1)
			{
				xorac = clearWireForReuse(xorac);
				outputGate(6, carry, xortout, xorac);
                
				and1 = clearWireForReuse(and1);
				outputGateNoInvertOutput(8, xorab, xorac, and1);
                
				carry = clearWireForReuse(carry);
				outputGateNoInvertOutput(6, xortout, and1, carry);
			}
            
			if (xortout->refs == 0) clearWireForReuse(xortout);
			if (xorab->refs == 0) clearWireForReuse(xorab);
			if (xorac->refs == 0) clearWireForReuse(xorac);
			if (and1->refs == 0) clearWireForReuse(and1);
		}
        
		t = invertWireNoInvertOutput(remainw[length - 1]);

        
		if (!IsModDiv)
		{
			ldest[(length - 1) - i] = t;
		}
	}
    
	/*get modulus*/
	if (IsModDiv)
	{
        
		for (int i = 0; i < length; i++)
		{
			ldest[i] = remainw[i];
		}
        
		vector<Wire *> addDest;
		addDest.resize(ldest.size());
		outputAddition(&ldest, &lright, addDest);
        
		//cout << "before fail\n";
		for (int i = 0; i < ldest.size(); i++)
		{
			assignWireCond(ldest[i], addDest[i], ldest[destv.size()]);
		}
	}
    
	WireSet * ws = getPool()->getWires(origlength);
    
	for (int i = 0; i < origlength; i++)
	{
		destv[i] = ws->wires[i];
		//cout << destv[i]->wireNumber<<"\n";
	}
    
    
	/*reduce to original length*/
	for (int i = 0; i < origlength; i++)
	{
		assignWire(destv[i], ldest[i]);
	}
}



//leftv - dividend, rightv - divisor
void InnerFunction::outputDivideSigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv)
{
    //cout << "outputDivideSigned not complete\n";
    
	int origlength = leftv->size();
	int length = leftv->size() + 1;
    
	Wire * carry = getPool()->getWire();
	Wire * xorab = getPool()->getWire();
	Wire * xorac = getPool()->getWire();
	Wire * and1 = getPool()->getWire();
    
	Wire * xortout = getPool()->getWire();
    
	Wire * t = get_ONE_WIRE();
    
	vector<Wire *> inputx, inputy, remainw;
	vector<Wire *> lleft, lright, ldest, subtractedleft;
    
	lleft.resize(origlength);
	lright.resize(origlength);
	ldest.resize(length);
    
	int i_out = 0;
	for (; i_out < origlength; i_out++)
	{
		lleft[i_out] = getPool()->getWire();
		assignWire(lleft[i_out], leftv->operator[](i_out));
		//makeWireContainValue(lleft[i_out]);
        
		lright[i_out] = getPool()->getWire();
		//lright[i_out] = rightv->operator[](i_out);
		assignWire(lright[i_out], rightv->operator[](i_out));
		//makeWireContainValue(lright[i_out]);
	}
    
    
	Wire * ifsubtractl = getPool()->getWire();
	Wire * ifsubtractr = getPool()->getWire();
    
	assignWire(ifsubtractl, lleft[origlength - 1]);
	assignWire(ifsubtractr, lright[origlength - 1]);
    
	vector<Wire *> zeros;
	vector<Wire *> subDestr, subDestl;
	subDestr.resize(origlength);
	subDestl.resize(origlength);
	zeros.resize(origlength);
    
	for (int i = 0; i < origlength; i++)
	{
		zeros[i] = ZERO_WIRE;
	}
    
	outputSubtract(&zeros, leftv, subDestl);
	outputSubtract(&zeros, rightv, subDestr);
    
    
	for (i_out = 0; i_out < origlength; i_out++)
	{
		assignWireCond(lleft[i_out], subDestl[i_out], ifsubtractl);
		assignWireCond(lright[i_out], subDestr[i_out], ifsubtractr);
	}
    
    
    
	/*extend extra bit for correctness purposes*/
	lleft.resize(length);
	lright.resize(length);
    
	for (; i_out < length; i_out++)
	{
		lleft[i_out] = get_ZERO_WIRE();
		lright[i_out] = get_ZERO_WIRE();
	}
    
	inputx.resize(length);
	inputy.resize(length);
	remainw.resize(length);
    
	//divisor
	for (int i = 0; i < length; i++)
	{
	    //printWire(lleft[i]);
	    //printWire(lright[i]);
        
		inputx[i] = lright[i];
		inputy[i] = getPool()->getWire();
	}
    
	vector<Wire *> keepwiresA;
	vector<Wire *> keepwiresB;
    
    
	for (int i = 0; i < length; i++)
	{
	    /* setting each input row*/
		if (i == 0)
		{
		    //remain in / dividend
			inputy[0] = lleft[length - 1];
            
			for (int j = 1; j < length; j++)
			{
				assignWire(inputy[j], get_ZERO_WIRE());
			}
		}
		else
		{
			inputy[0] = lleft[length - 1 - i];
			for (int j = 1; j < length; j++)
			{
				assignWire(inputy[j], remainw[j - 1]);
			}
		}
        
		//carry in
		if (i == 0)
		{
			carry->state = ONE;
		}
		else
		{
			assignWire(carry, t);
		}
        
        
		/*controlled add / subtract*/
		for (int j = 0; j < length; j++)
		{
			xortout = clearWireForReuse(xortout);
			outputGateNoInvertOutput(6, t, inputx[j], xortout);
            
			xorab = clearWireForReuse(xorab);
			outputGateNoInvertOutput(6, inputy[j], xortout, xorab);
            
			//full adder part
			if (remainw[j] != 0)
			{
                
			    //save wires from layer i so they can be used at layer i+1 (otherwise each remainw wire requires a new wire, very inefficient)
				if (remainw[j]->refs > 0)
				{
					if (i % 2 == 0)
					{
						keepwiresA.push_back(remainw[j]);
						if (keepwiresB.size() > 0)
						{
							remainw[j] = keepwiresB.back();
							keepwiresB.pop_back();
						}
						else
						{
							remainw[j] = getPool()->getWire();
						}
					}
					else
					{
						keepwiresB.push_back(remainw[j]);
						if (keepwiresA.size() > 0)
						{
							remainw[j] = keepwiresA.back();
							keepwiresA.pop_back();
						}
						else
						{
							remainw[j] = getPool()->getWire();
						}
					}
					if (remainw[j] == 0) remainw[j] = getPool()->getWire();
				}
                
                
                
				remainw[j] = clearWireForReuse(remainw[j]);
				outputGate(6, xorab, carry, remainw[j]);
			}
			else
			{
				remainw[j] = outputGate(6, xorab, carry);
			}
            
            
			if (j < length - 1)
			{
				xorac = clearWireForReuse(xorac);
				outputGate(6, carry, xortout, xorac);
                
				and1 = clearWireForReuse(and1);
				outputGateNoInvertOutput(8, xorab, xorac, and1);
                
				carry = clearWireForReuse(carry);
				outputGateNoInvertOutput(6, xortout, and1, carry);
			}
            
			if (xortout->refs == 0) clearWireForReuse(xortout);
			if (xorab->refs == 0) clearWireForReuse(xorab);
			if (xorac->refs == 0) clearWireForReuse(xorac);
			if (and1->refs == 0) clearWireForReuse(and1);
		}
        
		t = invertWireNoInvertOutput(remainw[length - 1]);
        
        
		if (!IsModDiv)
		{
			ldest[(length - 1) - i] = t;
		}
	}
    
	/*get modulus*/
	if (IsModDiv)
	{
        
		for (int i = 0; i < length; i++)
		{
			ldest[i] = remainw[i];
		}
        
		vector<Wire *> addDest;
		addDest.resize(ldest.size());
		outputAddition(&ldest, &lright, addDest);
        
		//cout << "before fail\n";
		for (int i = 0; i < ldest.size(); i++)
		{
			assignWireCond(ldest[i], addDest[i], ldest[destv.size()]);
		}
        
        
        
		//signed portion of modulus below:
		vector<Wire *> resultsubDest;
		resultsubDest.resize(origlength + 1);
		zeros.resize(origlength + 1);
        
		//expand 0 array
		zeros[origlength] = ZERO_WIRE;
        
		outputSubtract(&zeros, &ldest, resultsubDest);
        
		for (i_out = 0; i_out < origlength + 1; i_out++)
		{
			assignWireCond(ldest[i_out], resultsubDest[i_out], ifsubtractl);
		}
	}
	else
	{
		vector<Wire *> resultsubDest;
		resultsubDest.resize(origlength + 1);
		zeros.resize(origlength + 1);
        
		//expand 0 array
		zeros[origlength] = ZERO_WIRE;

		outputSubtract(&zeros, &ldest, resultsubDest);
        
		Wire * result = outputGateNoInvertOutput(6, ifsubtractl, ifsubtractr);
        
		for (i_out = 0; i_out < origlength + 1; i_out++)
		{
			assignWireCond(ldest[i_out], resultsubDest[i_out], result);
		}
        
	}
    
	WireSet * ws = getPool()->getWires(origlength);
    
	for (int i = 0; i < origlength; i++)
	{
		destv[i] = ws->wires[i];
	}
    
    
	/*reduce to original length*/
	for (int i = 0; i < origlength; i++)
	{
		assignWire(destv[i], ldest[i]);
	}
}





//only does right side of multiplication trapazoid (i.e. if your mult is 32 bits by 32 bits we don't need the result of the left 32 bits since it goes back into a 32bit int, not a 64 bit int).
//precondiction - all vectors are of proper size( |leftv| == |rightv| == |destv| ))
//left is x input
//right is y input
//mult algorithm from MIT slides from course 6.111, fall 2012, lecture 8/9, slide 33
void InnerFunction::outputExMultSigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv)
{
    
    
    
	int length = leftv->size();
    

    
	    //cout << "start outputExMultUnsigned out\n";
	WireSet * ws;
    
	switch (length)
	{
	case 1:
		ws = getPool()->getWires(2);
            
		for (int i = 0; i < 2; i++)
		{
			destv[i] = ws->wires[i];
		}
            
        
            
		assignWire(destv[0], outputGate(8, leftv->operator[](0), rightv->operator[](0)));
		assignWire(destv[1], outputGate(6, ZERO_WIRE, ZERO_WIRE));
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
            
		Wire * carry = getPool()->getWire();
		Wire * xor2 = getPool()->getWire();
		Wire * xorab = getPool()->getWire();
		Wire * andn1 = getPool()->getWire();
		Wire * andn2 = getPool()->getWire();
            
            
		//number of rows
		for (int i = 0; i < length - 1; i++)
		{
		    //cout << "new header\n";
                
			if (i != 0)
			{
				rowinputsleft[length - 1] = clearWireForReuse(rowinputsleft[length - 1]);
				assignWire(rowinputsleft[length - 1], carry);
			}
                
			//create inputs to each adder
			if (i == 0)
			{
                    
				for (int k = 0; k < length; k++)
				{
					rowinputsleft[k] = outputGate(8, leftv->operator[](k), rightv->operator[](0));
				}
				//only on first row do we do this
                    
				for (int k = 0; k < length; k++)
				{
					rowinputsright[k] = outputGate(8, leftv->operator[](k), rightv->operator[](1));
				}
                    
                    
				tdest[0] = getPool()->getWire();
				assignWire(tdest[0], rowinputsleft[0]);
                    
				//shift down
				for (int k = 0; k < length - 1; k++)
				{
					assignWire(rowinputsleft[k], rowinputsleft[k + 1]);
				}
				rowinputsleft[length - 1] = clearWireForReuse(rowinputsleft[length - 1]);
				assignWire(rowinputsleft[length - 1], ONE_WIRE);
                    
				rowinputsleft[length - 2] = invertWireNoAllocUnlessNecessary(rowinputsleft[length - 2]);
                    
				rowinputsright[length - 1] = invertWireNoAllocUnlessNecessary(rowinputsright[length - 1]);
                    
			}
			else
			{
				for (int k = 0; k < length; k++)
				{
					rowinputsright[k] = clearWireForReuse(rowinputsright[k]);
					outputGate(8, leftv->operator[](k), rightv->operator[](i + 1), rowinputsright[k]);
				}
                    
				if (i != length - 2)
				{
					rowinputsright[length - 1] = invertWireNoAllocUnlessNecessary(rowinputsright[length - 1]);
				}
				else
				{
					for (int k = 0; k < length - 1; k++)
					{
						rowinputsright[k] = invertWireNoAllocUnlessNecessary(rowinputsright[k]);
					}
				}
			}
                
			//cout << "new row\n";
                
			//create each adder
			for (int j = 0; j < length; j++)
			{
                    
                    
			    //output half adder
				if (j == 0)
				{
					xorab = clearWireForReuse(xorab);
					outputGate(6, rowinputsright[0], rowinputsleft[0], xorab);
                        
					tdest[i + 1] = pool.getWire();
					assignWire(tdest[i + 1], xorab);
                        
					carry = clearWireForReuse(carry);
                        
                        
					outputGate(8, rowinputsright[0], rowinputsleft[0], carry);
                        
                        
				}
				else //output full adder
				{
					xorab = clearWireForReuse(xorab);
					outputGate(6, rowinputsright[j], rowinputsleft[j], xorab);
                        
					rowinputsleft[j - 1] = clearWireForReuse(rowinputsleft[j - 1]);
					outputGate(6, xorab, carry, rowinputsleft[j - 1]);
                        
					//if(j < length-1)
					{
						andn2 = clearWireForReuse(andn2);
						outputGate(6, carry, rowinputsleft[j], andn2);
                            
						andn1 = clearWireForReuse(andn1);
						outputGate(8, xorab, andn2, andn1);
                            
						carry = clearWireForReuse(carry);
						outputGate(6, rowinputsleft[j], andn1, carry);
					}
				}
                    
				if (andn2->refs == 0) clearWireForReuse(andn2);
				if (andn1->refs == 0) clearWireForReuse(andn1);
				if (xorab->refs == 0) clearWireForReuse(xorab);
			}
                
                
                
		}
            
		for (int i = 0; i < length - 1; i++)
		{
			tdest[i + length] =  rowinputsleft[i];
		}
            
		//cout << "beingx\n";
		tdest[length + length - 1] = carry;
            
            
            
		Wire * temp = getPool()->getWire();
		assignWire(temp, tdest[length + length - 1]);
            
		makeWireContainValue(temp);
		assignWire(tdest[length + length - 1], ZERO_WIRE);
            
		outputGateNoInvertOutput(6, temp, ONE_WIRE, tdest[length + length - 1]);
            
		ws = getPool()->getWires(destv.size());
            
		for (int i = 0; i < destv.size(); i++)
		{
			destv[i] = ws->wires[i];
			assignWire(destv[i], tdest[i]);
		}
            
            
		break;
	}
    
    
    
	//cout << "end outputExMultUnsigned out\n";

}


void InnerFunction::outputExMultUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv)
{
	int length = leftv->size();
    
    
	//cout << "start outputExMultUnsigned out\n";
    
	WireSet * ws;
    
	switch (length)
	{
	case 1:
		ws = getPool()->getWires(2);
            
		for (int i = 0; i < 2; i++)
		{
			destv[i] = ws->wires[i];
		}
            
            
            
		assignWire(destv[0], outputGate(8, leftv->operator[](0), rightv->operator[](0)));
		assignWire(destv[1], outputGate(6, ZERO_WIRE, ZERO_WIRE));
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
            
		Wire * carry = getPool()->getWire();
		Wire * xor2 = getPool()->getWire();
		Wire * xorab = getPool()->getWire();
		Wire * andn1 = getPool()->getWire();
		Wire * andn2 = getPool()->getWire();
            
		//number of rows
		for (int i = 0; i < length - 1; i++)
		{
		    //cout << "new header\n";
                
			if (i != 0)
			{
				rowinputsleft[length - 1] = clearWireForReuse(rowinputsleft[length - 1]);
				assignWire(rowinputsleft[length - 1], carry);
			}
                
			//create inputs to each adder
			if (i == 0)
			{
                    
				for (int k = 0; k < length; k++)
				{
					rowinputsleft[k] = outputGate(8, leftv->operator[](k), rightv->operator[](0));
				}
				//only on first row do we do this
                    
				for (int k = 0; k < length; k++)
				{
					rowinputsright[k] = outputGate(8, leftv->operator[](k), rightv->operator[](1));
				}
                    
                    
				tdest[0] = getPool()->getWire();
				assignWire(tdest[0], rowinputsleft[0]);
                    
				//shift down
				for (int k = 0; k < length - 1; k++)
				{
					assignWire(rowinputsleft[k], rowinputsleft[k + 1]);
				}
				rowinputsleft[length - 1] = clearWireForReuse(rowinputsleft[length - 1]);
				assignWire(rowinputsleft[length - 1], ZERO_WIRE);

			}
			else
			{
				for (int k = 0; k < length; k++)
				{
					rowinputsright[k] = clearWireForReuse(rowinputsright[k]);
					outputGate(8, leftv->operator[](k), rightv->operator[](i + 1), rowinputsright[k]);
				}

                    
			}
                
			//cout << "new row\n";
                
			//create each adder
			for (int j = 0; j < length; j++)
			{
                    
                    
			    //output half adder
				if (j == 0)
				{
					xorab = clearWireForReuse(xorab);
					outputGate(6, rowinputsright[0], rowinputsleft[0], xorab);
                        
					tdest[i + 1] = pool.getWire();
					assignWire(tdest[i + 1], xorab);
                        
					carry = clearWireForReuse(carry);
                        

					outputGate(8, rowinputsright[0], rowinputsleft[0], carry);
                        
                        
				}
				else //output full adder
				{
					xorab = clearWireForReuse(xorab);
					outputGate(6, rowinputsright[j], rowinputsleft[j], xorab);
                        
					rowinputsleft[j - 1] = clearWireForReuse(rowinputsleft[j - 1]);
					outputGate(6, xorab, carry, rowinputsleft[j - 1]);
                        
					//if(j < length-1)
					{
						andn2 = clearWireForReuse(andn2);
						outputGate(6, carry, rowinputsleft[j], andn2);
                            
						andn1 = clearWireForReuse(andn1);
						outputGate(8, xorab, andn2, andn1);
                            
						carry = clearWireForReuse(carry);
						outputGate(6, rowinputsleft[j], andn1, carry);
					}
				}
                    
				if (andn2->refs == 0) clearWireForReuse(andn2);
				if (andn1->refs == 0) clearWireForReuse(andn1);
				if (xorab->refs == 0) clearWireForReuse(xorab);
			}
                

                
		}
            
		//cout << "lengths: "<<destv.size()<<" "<<(length+length-1)<<"\n";
            
		for (int i = 0; i < length - 1; i++)
		{
			tdest[i + length] =  rowinputsleft[i];
		}
		tdest[length + length - 1] = carry;
            
		ws = getPool()->getWires(destv.size());
            
		for (int i = 0; i < destv.size(); i++)
		{
			destv[i] = ws->wires[i];
			assignWire(destv[i], tdest[i]);
		}
            
		break;
	}
    
	//cout << "end outputExMultUnsigned out\n";
}

//leftv - dividend, rightv - divisor
void InnerFunction::outputReDivideUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv, int length_in)
{
    
	vector<Wire *> R_left;
	vector<Wire *> R_right;
    
	vector<Wire *> quotient;
	vector<Wire *> remainder;
    
    
	int lengthOfOp = length_in + 1;
	int lengthLeft = lengthOfOp * 2 - 1;
    
    
	R_left.resize(lengthLeft);
	R_right.resize(lengthOfOp);
	quotient.resize(lengthOfOp);
	remainder.resize(lengthOfOp);
    
	int lssize = leftv->size();
    
    
	WireSet * ws = getPool()->getWires(lssize);
    
	for (int i = 0; i < lssize; i++)
	{
		R_left[i] = ws->wires[i];
	}
    
	int j;
	for (j = 0; j < lssize; j++)
	{
		assignWire(R_left[j], leftv->operator[](j));
	}
	for (; j < R_left.size(); j++)
	{
		R_left[j] = ZERO_WIRE;
	}
    
	lssize = rightv->size();
    
    
	ws = getPool()->getWires(lssize);
    
	for (int i = 0; i < lssize && i < R_right.size(); i++)
	{
		R_right[i] = ws->wires[i];
	}
    
	for (j = 0; j < lssize && j < R_right.size(); j++)
	{
		assignWire(R_right[j], rightv->operator[](j));
	}
	for (; j < R_right.size(); j++)
	{
		R_right[j] = ZERO_WIRE;
	}
    
    
	vector<Wire *> rowinputsright;
	vector<Wire *> rowinputsleft;
    
	rowinputsright.resize(lengthOfOp);
	rowinputsleft.resize(lengthOfOp);
    
    
    
	vector<Wire *> heldresults;
    
	heldresults.resize(lengthOfOp);
	for (int i = 0; i < heldresults.size(); i++)
	{
		heldresults[i] = getPool()->getWire();
	}

        
    
	Wire * CASxor = getPool()->getWire();
	Wire * carry = getPool()->getWire();
	Wire * xor2 = getPool()->getWire();
	Wire * xorab = getPool()->getWire();
	Wire * andn1 = getPool()->getWire();
	Wire * andn2 = getPool()->getWire();
    
    
	Wire * T = ONE_WIRE;
    
    
	for (int i = 0; i < lengthOfOp; i++)
	{
		rowinputsright[i] = R_right[i];
	}
    
    
	int diff = R_left.size() - rowinputsright.size();
	for (int i = 0; i < lengthOfOp; i++)
	{
		rowinputsleft[i] = getPool()->getWire();
		assignWire(rowinputsleft[i], R_left[i + diff]);
	}
    
    
	/*printwirevec(R_right);
	cout <<"\n";
	printwirevec(R_left);*/
    
    
	//cout << "begin gates\n";
    
	//cout << "loo: "<<lengthOfOp<<"\nLL: "<<lengthLeft<<"\n";
    
	for (int row = 0; row < lengthOfOp; row++)
	{
	    //cout << "row: "<<row<<"\n";
        
		assignWire(rowinputsleft[0], R_left[(lengthOfOp - 1) - row]);
		//cout << "RL0 = R_L-"<<(lengthOfOp -1)-row<<"\n";
        
		assignWire(carry, T);
		for (int col = 0; col < lengthOfOp; col++)
		{
		 //    cout << "col: "<<col<<"\n";
            
			CASxor = clearWireForReuse(CASxor);
			outputGateNoInvertOutput(6, T, rowinputsright[col], CASxor);
            
            
			xorab = clearWireForReuse(xorab);
			outputGateNoInvertOutput(6, CASxor, rowinputsleft[col], xorab);
            
			heldresults[col] = clearWireForReuse(heldresults[col]);
			outputGateNoInvertOutput(6, xorab, carry, heldresults[col]);

			andn2 = clearWireForReuse(andn2);
			outputGateNoInvertOutput(6, carry, CASxor, andn2);
            
			andn1 = clearWireForReuse(andn1);
			outputGateNoInvertOutput(8, xorab, andn2, andn1);
            
			carry = clearWireForReuse(carry);
			outputGateNoInvertOutput(6, CASxor, andn1, carry);
            
            
            
            
            
			if (andn2->refs == 0) clearWireForReuse(andn2);
			if (andn1->refs == 0) clearWireForReuse(andn1);
			if (xorab->refs == 0) clearWireForReuse(xorab);
		}
        
        
        
		for (int i = 0; i < lengthOfOp - 1; i++)
		{
			assignWire(rowinputsleft[i + 1], heldresults[i]);
		   // cout << "RL"<<i+1<<" = held-"<<i+1<<"\n";
		}
        
        
        
		T = heldresults[lengthOfOp - 1];
		T = invertWireNoInvertOutput(T);
		quotient[lengthOfOp - row - 1] = T;
	  //  cout << "outing: "<< lengthOfOp-row-1<<"\n";
	}

    
    
	    //cout << "begin copy output\n";
    
	    //cout << "IsModDiv: "<<IsModDiv<<"\n";
    
	if (!IsModDiv)
	{
        
		ws = getPool()->getWires(destv.size());
        
		for (int i = 0; i < destv.size(); i++)
		{
			destv[i] = ws->wires[i];
			assignWire(destv[i], quotient[i]);
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
        
		outputAddition(&heldresults, &R_right, addDest);
        
		for (int i = 0; i < heldresults.size(); i++)
		{
			assignWireCond(heldresults[i], addDest[i], heldresults[destv.size()]);
		}
        
        
		/*for(int i=0;i<destv.size();i++)
		{
		    destv[i] = heldresults[i]; //remainder[i];
		}*/
        
		ws = getPool()->getWires(destv.size());
        
		for (int i = 0; i < destv.size(); i++)
		{
			destv[i] = ws->wires[i];
			assignWire(destv[i], heldresults[i]);
		}

	}
    
	//cout << "end outputReDivideUnsigned\n";
}



//leftv - dividend, rightv - divisor
void InnerFunction::outputReDivideSigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv, int length_in)
{

	vector<Wire *> R_left;
	vector<Wire *> R_right;
    
	vector<Wire *> quotient;
	vector<Wire *> remainder;
    
    
	int lengthOfOp = length_in + 1;
	int lengthLeft = lengthOfOp * 2 - 1;
    
    
	R_left.resize(lengthLeft);
	R_right.resize(lengthOfOp);
	quotient.resize(lengthOfOp);
	remainder.resize(lengthOfOp);
    
	int lssize = leftv->size();
    
    
    
	WireSet * ws = getPool()->getWires(lssize);
    
	for (int i = 0; i < lssize; i++)
	{
		R_left[i] = ws->wires[i];
	}
    
	int j;
	for (j = 0; j < lssize; j++)
	{
		assignWire(R_left[j], leftv->operator[](j));
	}
	for (; j < R_left.size(); j++)
	{
		R_left[j] = ZERO_WIRE;
	}
    
	lssize = rightv->size();
    
	ws = getPool()->getWires(lssize);
    
	for (int i = 0; i < lssize && i < R_right.size(); i++)
	{
		R_right[i] = ws->wires[i];
	}
    
	for (j = 0; j < lssize && j < R_right.size(); j++)
	{
		assignWire(R_right[j], rightv->operator[](j));
	}
	for (; j < R_right.size(); j++)
	{
		R_right[j] = ZERO_WIRE;
	}
    
    
	/*cout <<"first\n";
	printwirevec(R_right);
	cout <<"\n";
	printwirevec(R_left);
	cout <<"\n";*/
 
    
    
    
	int origlength = leftv->size();
	int origlengthx = rightv->size();
    
    
	//cout << origlength<< "\n" << origlengthx<<"\n"<< R_left.size()<<"\n"<<R_right.size()<<"\n";
    
    
	Wire * ifsubtractl = getPool()->getWire();
	Wire * ifsubtractr = getPool()->getWire();
    
	//cout << "second\n";
    
	assignWire(ifsubtractl, R_left[origlength - 1]);
	assignWire(ifsubtractr, R_right[origlengthx - 1]);
    
	//cout << "third\n";
    
	//printWire(ifsubtractl);
    
	vector<Wire *> zerosleft, zerosright;
	vector<Wire *> subDestr, subDestl;
	subDestr.resize(origlengthx);
	subDestl.resize(origlength);
	zerosleft.resize(origlength);
	zerosright.resize(origlengthx);
    
	//cout << "forth\n";
    
	for (int i = 0; i < origlength; i++)
	{
		zerosleft[i] = ZERO_WIRE;
	}
	for (int i = 0; i < origlengthx; i++)
	{
		zerosright[i] = ZERO_WIRE;
	}
    
	//cout << "fifth\n";
    
	outputSubtract(&zerosleft, leftv, subDestl);
	outputSubtract(&zerosright, rightv, subDestr);
    
    
	/*cout << R_left.size()<<"\n";
	cout << subDestl.size()<<"\n";
	cout << origlength<<"\n";
    
	cout << R_right.size()<<"\n";
	cout << subDestr.size()<<"\n";
	cout << origlengthx<<"\n";

	    cout << length_in<<"\n";*/
    
	for (int i_out = 0; i_out < origlength; i_out++)
	{
		assignWireCond(R_left[i_out], subDestl[i_out], ifsubtractl);
	}
	for (int i_out = 0; i_out < origlengthx; i_out++)
	{
		assignWireCond(R_right[i_out], subDestr[i_out], ifsubtractr);
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
	for (int i = 0; i < heldresults.size(); i++)
	{
		heldresults[i] = getPool()->getWire();
	}
    
    
    
	Wire * CASxor = getPool()->getWire();
	Wire * carry = getPool()->getWire();
	Wire * xor2 = getPool()->getWire();
	Wire * xorab = getPool()->getWire();
	Wire * andn1 = getPool()->getWire();
	Wire * andn2 = getPool()->getWire();
    
    
	Wire * T = ONE_WIRE;
    
    
    
	for (int i = 0; i < lengthOfOp; i++)
	{
		rowinputsright[i] = R_right[i];
	}
    
    
	int diff = R_left.size() - rowinputsright.size();
	for (int i = 0; i < lengthOfOp; i++)
	{
		rowinputsleft[i] = getPool()->getWire();
		assignWire(rowinputsleft[i], R_left[i + diff]);
	}
    
	/*cout <<"last\n";
	printwirevec(R_right);
	 cout <<"\n";
	 printwirevec(R_left);
	cout <<"\n";
	*/
    
	//cout << "begin gates\n";
    
	//cout << "loo: "<<lengthOfOp<<"\nLL: "<<lengthLeft<<"\n";
    
	for (int row = 0; row < lengthOfOp; row++)
	{
	    //cout << "row: "<<row<<"\n";
        
		assignWire(rowinputsleft[0], R_left[(lengthOfOp - 1) - row]);
		//cout << "RL0 = R_L-"<<(lengthOfOp -1)-row<<"\n";
        
		assignWire(carry, T);
		for (int col = 0; col < lengthOfOp; col++)
		{
		    //    cout << "col: "<<col<<"\n";
            
			CASxor = clearWireForReuse(CASxor);
			outputGateNoInvertOutput(6, T, rowinputsright[col], CASxor);
            
            
			xorab = clearWireForReuse(xorab);
			outputGateNoInvertOutput(6, CASxor, rowinputsleft[col], xorab);
            
			heldresults[col] = clearWireForReuse(heldresults[col]);
			outputGateNoInvertOutput(6, xorab, carry, heldresults[col]);
            
			andn2 = clearWireForReuse(andn2);
			outputGateNoInvertOutput(6, carry, CASxor, andn2);
            
			andn1 = clearWireForReuse(andn1);
			outputGateNoInvertOutput(8, xorab, andn2, andn1);
            
			carry = clearWireForReuse(carry);
			outputGateNoInvertOutput(6, CASxor, andn1, carry);
            
            
            
            
            
			if (andn2->refs == 0) clearWireForReuse(andn2);
			if (andn1->refs == 0) clearWireForReuse(andn1);
			if (xorab->refs == 0) clearWireForReuse(xorab);
		}
        
        
        
		for (int i = 0; i < lengthOfOp - 1; i++)
		{
			assignWire(rowinputsleft[i + 1], heldresults[i]);
			// cout << "RL"<<i+1<<" = held-"<<i+1<<"\n";
		}
        
        
        
		T = heldresults[lengthOfOp - 1];
		T = invertWireNoInvertOutput(T);
		quotient[lengthOfOp - row - 1] = T;
		//  cout << "outing: "<< lengthOfOp-row-1<<"\n";
	}
    
    
    
	//cout << "begin copy output\n";
    
	//cout << "IsModDiv: "<<IsModDiv<<"\n";
    
	if (!IsModDiv)
	{
        
        
		vector<Wire *> resultsubDest;
		resultsubDest.resize(origlengthx + 1);
		zerosright.resize(origlengthx + 1);
        
		//expand 0 array
		zerosright[origlengthx] = ZERO_WIRE;
        
		//cout << "fsize: "<<zerosright.size()<<" "<<quotient.size()<<" "<<resultsubDest.size()<<"\n";
        
		outputSubtract(&zerosright, &quotient, resultsubDest);
        
		Wire * result = outputGateNoInvertOutput(6, ifsubtractl, ifsubtractr);
        
		for (int i_out = 0; i_out < origlengthx + 1; i_out++)
		{
			assignWireCond(quotient[i_out], resultsubDest[i_out], result);
		}
        
       
        
	    /*for(int i=0;i<destv.size();i++)
	    {
	        destv[i] = quotient[i];
	}*/
        
		ws = getPool()->getWires(destv.size());
        
		for (int i = 0; i < destv.size(); i++)
		{
			destv[i] = ws->wires[i];
			assignWire(destv[i], quotient[i]);
		}
        
	}
	else
	{
        
		vector<Wire *> addDest;
		addDest.resize(heldresults.size());
        
		//cout << "sizes: "<<heldresults.size()<<" "<<R_right.size()<<" "<<addDest.size()<<"\n";
        
		outputAddition(&heldresults, &R_right, addDest);
        
		for (int i = 0; i < heldresults.size(); i++)
		{
			assignWireCond(heldresults[i], addDest[i], heldresults[destv.size()]);
		}
        
        
		//signed portion of modulus below:
		vector<Wire *> resultsubDest;
		resultsubDest.resize(origlengthx + 1);
		zerosright.resize(origlengthx + 1);
        
		//expand 0 array
		zerosright[origlengthx] = ZERO_WIRE;
        
		outputSubtract(&zerosright, &heldresults, resultsubDest);
        
		for (int i_out = 0; i_out < origlengthx + 1; i_out++)
		{
			assignWireCond(heldresults[i_out], resultsubDest[i_out], ifsubtractl);
		}
        
		/*for(int i=0;i<destv.size();i++)
		{
		    destv[i] = heldresults[i]; //remainder[i];
		}*/
        
		ws = getPool()->getWires(destv.size());
        
		for (int i = 0; i < destv.size(); i++)
		{
			destv[i] = ws->wires[i];
			assignWire(destv[i], heldresults[i]);
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

bool seeoutput = false;


void InnerFunction::makeONEandZERO(ostream & mos)
{
    
	one_wire_l = currentbasewire;
	writeGate(15, currentbasewire++, 0, 0, &mos);
    
	zero_wire_l = currentbasewire;
	writeGate(0, currentbasewire++, 0, 0, &mos);
    
    
	ONE_WIRE = new Wire();
	ONE_WIRE->state = ONE;
	ONE_WIRE->wireNumber = one_wire_l;
    
	ZERO_WIRE = new Wire();
	ZERO_WIRE->state = ZERO;
	ZERO_WIRE->wireNumber = zero_wire_l;
}

void InnerFunction::makeWireContainValueNoONEZEROcopy(Wire * w)
{

    
    
	if (w->other == 0 || w->state == UNKNOWN)
	{
		if (w->state == UNKNOWN_INVERT)
		{
			writeGate(6, w->wireNumber, w->wireNumber, ONE_WIRE->wireNumber);
			w->state = UNKNOWN;
		}
        
		return;
	}
    
	writeCopy(w->wireNumber, w->other->wireNumber);

    
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
		writeGate(6, w->wireNumber, w->wireNumber, ONE_WIRE->wireNumber);
		w->state = UNKNOWN;
	}
    
   
}


int MWCV_base_dest;
int MWCV_base_from;
int MWCV_amt = 0;
int MWCV_lastnum_dest;
int MWCV_lastnum_from;
bool MWCV_isFirstLine;

void InnerFunction::makeWireContainValueNoONEZEROcopyTiny(Wire * w)
{
	if (w->other == 0 || w->state == UNKNOWN)
	{
		if (w->state == UNKNOWN_INVERT)
		{

			if (MWCV_amt > 0)
			{
			    /* */
				writeComplexGate(5, MWCV_base_dest, MWCV_base_from, 0, MWCV_amt, 0, 0);
                
				//MWCV_base = w->wireNumber;
				//MWCV_amt=1;
				MWCV_amt = 0;
				MWCV_isFirstLine = true;
			}
            
			writeGate(6, w->wireNumber, w->wireNumber, ONE_WIRE->wireNumber);
			w->state = UNKNOWN;
		}
		return;
	}
    
    
	if (MWCV_isFirstLine)
	{
		MWCV_base_dest = w->wireNumber;
		MWCV_base_from = w->other->wireNumber;
		MWCV_amt = 1;
		MWCV_isFirstLine = false;
	}
	else
	{
		if ((w->wireNumber - 1 != MWCV_lastnum_dest) || (w->other->wireNumber - 1 != MWCV_lastnum_from)) // print
		{
		    /* */
			if (MWCV_amt > 0)
				writeComplexGate(5, MWCV_base_dest, MWCV_base_from, 0, MWCV_amt, 0, 0);
            
            
			MWCV_base_dest = w->wireNumber;
			MWCV_base_from = w->other->wireNumber;
			MWCV_amt = 1;
			MWCV_isFirstLine = false;
		}
		else
		{
			MWCV_amt++;
		}
	}
	MWCV_lastnum_dest  = w->wireNumber;
	MWCV_lastnum_from = w->other->wireNumber;
    
	//writeCopy(w->wireNumber,w->other->wireNumber);
    
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
	    /* */
		writeComplexGate(5, MWCV_base_dest, MWCV_base_from, 0, MWCV_amt, 0, 0);
        
		MWCV_isFirstLine = true;
		MWCV_amt = 0;
        
		writeGate(6, w->wireNumber, w->wireNumber, ONE_WIRE->wireNumber);
		w->state = UNKNOWN;
	}
}

void InnerFunction::makeWireContainValueNoONEZEROcopyTinyEnd()
{
    /* */
	if (MWCV_amt > 0)
		writeComplexGate(5, MWCV_base_dest, MWCV_base_from, 0, MWCV_amt, 0, 0);
    
	MWCV_isFirstLine = true;
	MWCV_amt = 0;
}





Wire * InnerFunction::clearWireForReuse(Wire * w)
{
	if (w->refs > 0)
	{
		return pool.getWire();
	}
    
	w->state = ZERO;
    
	if (w->other != 0)
	{
		w->other->refs--; w->other->removeRef(w);
		w->other = 0;
	}
	return w;
}

void InnerFunction::makeWireContainValue(Wire * w)
{
	if (w->state == ONE)
	{
		writeCopy(w->wireNumber, one_wire_l);
		
	}
	if (w->state == ZERO)
	{
		writeCopy(w->wireNumber, zero_wire_l);
	
	}
    
    
	if (w->other == 0 || w->state == UNKNOWN)
	{
		return;
	}
    
	writeCopy(w->wireNumber, w->other->wireNumber);
	

	
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
		writeGate(6, w->wireNumber, w->wireNumber, ONE_WIRE->wireNumber);
		w->state = UNKNOWN;
	}
    
    
}

void InnerFunction::makeWireNotOther(Wire * w)
{
	if (w->other != 0)
	{
		writeCopy(w->wireNumber, w->other->wireNumber);
	    
        
		if (w->state == UNKNOWN_INVERT_OTHER_WIRE)
		{
			w->state = UNKNOWN_INVERT;
		}
		else
		{
			w->state = UNKNOWN;
		}
	}
    
	if (w->state == UNKNOWN_INVERT)
	{
		w->state = UNKNOWN;
		writeGate(6, w->wireNumber, w->wireNumber, ONE_WIRE->wireNumber);
		return;
	}
    
}

Wire * InnerFunction::outputGate(short table, Wire * a, Wire * b)
{
    /*if(table == 9)
    {
        cout << "9 gate\n";
    }*/
    
	Wire * dest = pool.getWire();
    
	if (shortCut(a, b, table, dest))
	{
		return dest;
	}

	    /*if(table != 6)
	    {
	        cout << "gate\n";
	    }*/
    
	return addGate(table, a, b, dest);
}

void InnerFunction::outputGate(short table, Wire * a, Wire * b, Wire * dest)
{
    /*if(table == 9)
    {
           cout << "9 gate\n";
    }*/
    
	if (shortCut(a, b, table, dest))
	{
		return;
	}
    
	/*if(table != 6)
	{
	    cout << "gate\n";
	}*/

    
	addGate(table, a, b, dest);
}

Wire * InnerFunction::outputGateNoInvertOutput(short table, Wire * a, Wire * b)
{
    /*if(table == 9)
    {
        cout << "9 gate\n";
    }*/
    
	Wire * dest = pool.getWire();
    
	if (shortCutNoInvertOutput(a, b, table, dest))
	{
		return dest;
	}
    
	/*if(table != 6)
	{
	    cout << "gate\n";
	}*/
    
	return addGate(table, a, b, dest);
}

void InnerFunction::outputGateNoInvertOutput(short table, Wire * a, Wire * b, Wire * dest)
{
    /*if(table == 9)
    {
        cout << "9 gate\n";
    }*/
    
	if (shortCutNoInvertOutput(a, b, table, dest))
	{
		return;
	}
    
	/*if(table != 6)
	{
	    cout << "gate\n";
	}*/
    
	addGate(table, a, b, dest);
}


void InnerFunction::outputFunctionCall(int num)
{
	writeFunctionCall(num, os);
}



long InnerFunction::getNonXorGates()
{
	return co_nonxorgates;
}

long InnerFunction::getXorGates()
{
	return co_xorgates;
}

void InnerFunction::incrementCountsBy(long nonxor, long xorg)
{
	co_nonxorgates += nonxor;
	co_xorgates += xorg;
}



void InnerFunction::setSeeOutput(bool value)
{
	seeoutput = value;
}
void InnerFunction::setPrintIOTypes(bool value)
{
	printIOTypeWires = value;
}


void InnerFunction::addComplexOp(short op, int length, int starta, int startb, int startdest, vector<Wire *> a, vector<Wire *> b, vector<Wire *> dest, Wire * carry, int isend)
{
    //cout << "testingcomplexop\n";
    
    //cout << "op: "<<op<<"\nlength: "<<length<<"\nstarta: "<<starta<<"\nstartb: "<<startb<<"\nstartdest: "<<startdest<<"\n"<<a[0]->wireNumber<<"\n"<<b[0]->wireNumber<<"\n"<<dest[0]->wireNumber<<"\n" <<carry->wireNumber<<"\n";
    
    
	for (int i = startdest; i < startdest + length; i++)
	{
		Wire * temp = dest[i];
        
		//clear wire
		if (temp->refs > 0)
		{
			clearReffedWire(temp);
		}
		if (temp->state == UNKNOWN_OTHER_WIRE || temp->state == UNKNOWN_INVERT_OTHER_WIRE)
		{
			temp->other->refs--; temp->other->removeRef(temp);
			temp->other = 0;
		}
        
		//label as unknown
		temp->state = UNKNOWN;
	}
    
    
	int carryw = 0;
	if (carry != 0)
	{
		carryw = carry->wireNumber;
	}
    
    
	//if this the program gets here then the value must be unknown. If it was known in any way (or a reference to another wire) it would have been done in the short circuit function
	writeComplexGate(op,
		dest[startdest]->wireNumber,
		a[starta]->wireNumber,
		b[startb]->wireNumber,
		length,
		carryw,
		isend);
}

void InnerFunction::addComplexOpSingleDestBit(short op, int length, int starta, int startb, int startdest, vector<Wire *> a, vector<Wire *> b, vector<Wire *> dest, Wire * carry, int isend)
{
    //cout << "testingcomplexop\n";
    
    //cout << "op: "<<op<<"\nlength: "<<length<<"\nstarta: "<<starta<<"\nstartb: "<<startb<<"\nstartdest: "<<startdest<<"\n"<<a[0]->wireNumber<<"\n"<<b[0]->wireNumber<<"\n"<<dest[0]->wireNumber<<"\n" <<carry->wireNumber<<"\n";
    
    
    
	for (int i = 0; i < 1; i++)
	{
		Wire * temp = dest[i];
        
		//clear wire
		if (temp->refs > 0)
		{
			clearReffedWire(temp);
		}
		if (temp->state == UNKNOWN_OTHER_WIRE || temp->state == UNKNOWN_INVERT_OTHER_WIRE)
		{
			temp->other->refs--; temp->other->removeRef(temp);
			temp->other = 0;
		}
        
		//label as unknown
		temp->state = UNKNOWN;
	}
    
    
	int carryw = 0;
	if (carry != 0)
	{
		carryw = carry->wireNumber;
	}
    
    
	//if this the program gets here then the value must be unknown. If it was known in any way (or a reference to another wire) it would have been done in the short circuit function
	writeComplexGate(op,
		dest[0]->wireNumber,
		a[starta]->wireNumber,
		b[startb]->wireNumber,
		length,
		carryw,
		isend);
}

void InnerFunction::writeComplexGate(short op, int dest, int x, int y, int length, int carryadd, int isend)
{
	outbuffer[0] = dest;
	outbuffer[1] = 5 << 8;
	outbuffer[2] = x;
	outbuffer[3] = y;
	os->write((char *)(&outbuffer[0]), 16);
    
	outbuffer[0] = carryadd;
	outbuffer[1] = isend;
	outbuffer[2] = length - 1;
	outbuffer[3] = op;
	os->write((char *)(&outbuffer[0]), 16);
    
	if (seeoutput) cout << os << " Complex op: " << op << " " << dest << " " << x << " " << y << " " << length << " " << "\n";
	
	 
	if (op == 4)
	{
		co_xorgates += length;
		//co_nonxorgates++;
	}
	else if (op == 3)
	{
		co_xorgates += length;
		//co_nonxorgates++;
	}
	else if (op == 0)
	{
		co_xorgates += length * 4;
		co_nonxorgates += length;
		if (isend)
		{
			co_nonxorgates--;
			co_xorgates -= 2;
		}
	}
	else if (op == 1)
	{
		co_xorgates += length * 6;
		co_nonxorgates += length;
		if (isend)
		{
			co_nonxorgates--;
			co_xorgates -= 2;
		}
	}
	else if (op == 2)
	{
		co_xorgates += length * 4;
		co_nonxorgates += length;
		if (isend)
		{
			co_nonxorgates--;
			co_xorgates -= 1;
		}
	}
	else
	{
	    /*so it doesn't get stopped*/
		co_nonxorgates++;
		co_xorgates++;
	}
	
}

void InnerFunction::writeGate(short table, int d, int x, int y)
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
}



void InnerFunction::writeGate(short table, int d, int x, int y, ostream * os)
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
}

void InnerFunction::writeCopy(int to, int from)
{
	outbuffer[0] = 0;
	outbuffer[1] = 0x300;
	outbuffer[2] = to;
	outbuffer[3] = from;
	os->write((char *)(&outbuffer[0]), 16);
	if (seeoutput) cout << os << " CP " << to << " " << from << "\n";
	co_xorgates++;
	
}

void InnerFunction::writeFunctionCall(int function, ostream * os)
{
	outbuffer[0] = function;
	outbuffer[1] = 0x400;
	outbuffer[2] = 0;
	outbuffer[3] = 0;
	os->write((char *)(&outbuffer[0]), 16);
	if (seeoutput) cout << os << " FN " << function << "\n";
	co_xorgates++;	
}


//inOut 0 - input
//inOut 1 - output
//party is party
//void InnerFunction::outputGate(ostream & mos, long wire, bool inOut, int party)
//{
//	if (inOut == 0)
//	{
//		outbuffer[0] = wire;
//		outbuffer[1] = 0x100;
//		outbuffer[2] = party;
//		outbuffer[3] = 0;
//		mos.write((char *)(&outbuffer[0]), 16);
//	}
//	else if (inOut == 1)
//	{
//		outbuffer[0] = wire;
//		outbuffer[1] = 0x200;
//		outbuffer[2] = party;
//		outbuffer[3] = 0;
//		mos.write((char *)(&outbuffer[0]), 16);
//	}
//
//	return;
//}

string InnerFunction::toFullEntry(string s)
{
	string tr = "";
	tr += "0 0 ";
	tr += s[3];
	tr += "\n";
	tr += "0 1 ";
	tr += s[2];
	tr += "\n";
	tr += "1 0 ";
	tr += s[1];
	tr += "\n";
	tr += "1 1 ";
	tr += s[0];
	tr += "\n";
    
	return tr;
}


string InnerFunction::toTable(int i)
{
	switch (i)
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


string InnerFunction::getFunctionPrefix()
{
	return functionprefix;
}


void InnerFunction::outputCircuit(VariableContext * vc, TypeMap * tm, Node * selectedNode, string odir)
{
	
	functionprefix = odir + "_f";
	VariableContext * vct = new VariableContext();
                
	for (std::unordered_map<string, Variable *>::iterator it = (*vc).begin(); it != (*vc).end(); ++it)
	{
					if (it->second != 0 && it->second->v_enum == Functionp)
					{
						(*vct)[it->first] = it->second;
					}
				}
	            
	           selectedNode->circuitOutput(vct, tm);
				//end-duplo
                
				delete vct;
	
            
			getPool()->freeIfNoRefs();
			pool.printUsedPoolState();
            
			//prevlargest = pool.largestsize;
            
			pool.freeAll();
			//pool.assignWireNumbers(prevlargest + 1);
            
			closeOutputFile();		
}
    
	
   

void openOutputFile(string s)
{
	os->open(s);
}

void closeOutputFile()
{
	if (os->is_open())
	{
		os->close();
	}
}
