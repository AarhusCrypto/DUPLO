//
//  wire.h
//  
//
//
//

#ifndef ____wire__
#define ____wire__

#include <stdio.h>
#include <iostream>

using namespace std;

enum State_Enum {UNKNOWN, ZERO,ONE, UNKNOWN_INVERT, UNKNOWN_INVERT_OTHER_WIRE, UNKNOWN_OTHER_WIRE};
enum Wire_Type_Enum {UNKNOWN_wt, PERM_wt, VAR_wt, TEMP_wt, FREE_wt};

class Variable;

class Wire
{
public:
    Wire(){state = ZERO; type = UNKNOWN_wt; other = 0; refs=0; locked = false; varlocked = false; thisvar =0; refsToMeSize=0; refsToMe = new Wire*[5]; maxRefsSize=5;}
    State_Enum state;
    Wire_Type_Enum type;
    long wireNumber;
	long prevWireNumber[2] = { -1, -1 };
    Wire * other;
    int otherRefArrayNumber;
    int refs;
    bool locked;
    bool varlocked;
    
    Variable * thisvar;
    
    void freeRefs(); //call if done with a wire that is not in the pool. Do not call if this wire is in the pool
    
    
    Wire ** refsToMe;
    int maxRefsSize;
    int refsToMeSize;
    
    void addRef(Wire * other)
    {
        refsToMe[refsToMeSize]=other;
        other->otherRefArrayNumber = refsToMeSize;
    
        refsToMeSize++;
        
        if(refsToMeSize == maxRefsSize)
        {
            Wire ** wires = new Wire*[maxRefsSize+5];
            for(int i=0;i<maxRefsSize;i++)
            {
                wires[i] = refsToMe[i];
            }
            delete [] refsToMe;
            refsToMe = wires;
            maxRefsSize+=5;
        }
    }
    void removeRef(Wire * other)
    {
        if(other->otherRefArrayNumber >= maxRefsSize)
        {
            cout << "other ref is too large\n";
        }
        if(other->otherRefArrayNumber < 0)
        {
            cout << "other ref is less than 0\n";
        }
        if(other->other != this)
        {
            cout <<"other other is not this\n";
        }
        if(refsToMe[other->otherRefArrayNumber] != other)
        {
            cout <<"refs to me other is not other\n";
        }
        
        
        Wire * t = refsToMe[refsToMeSize-1];
        refsToMe[other->otherRefArrayNumber] = t;
        t->otherRefArrayNumber = other->otherRefArrayNumber;
        refsToMeSize--;
    }
};


//duplo
string toStrGate(short table);


void printWire(Wire * a);

bool shortCut(Wire * a, Wire * b, short table, Wire * dest);
bool shortCutNoInvertOutput(Wire * a, Wire * b, short table, Wire * dest);

State_Enum evaluateWire(State_Enum a, State_Enum b);

State_Enum intToState(int i);

#include <string>

std::string WireStateToString(State_Enum e);

short invertTable(bool aOrb, short table);

#endif /* defined(____wire__) */
