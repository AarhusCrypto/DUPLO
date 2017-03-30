//
//  wire.cpp
//  
//
//
//

#include "wire.h"
#include <iostream>


using namespace std;

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

//aOrb - true is b, false is a
short invertTable(bool aOrb, short table)
{
    if(aOrb)
    {
        table = ((table << 1)&10) | ((table >> 1)&5); //((table <<2)&12) | ((table >>2)&3);
    }
    else
    {
        table = ((table <<2)&12) | ((table >>2)&3);
    }
    
    return table;
}

void Wire::freeRefs()
{
    if(other != 0)
    {
        other->refs--; other->removeRef(this);
        
    }
    other = 0;
}

string WireStateToString(State_Enum e)
{
    if(UNKNOWN == e)
    {
        return "UNKNOWN";
    }
    if(ZERO == e)
    {
        return "ZERO";
    }
    if (ONE == e)
    {
        return "ONE";
    }
    if (UNKNOWN_INVERT == e)
    {
        return "UNKNOWN_INVERT";
    }
    if (UNKNOWN_INVERT_OTHER_WIRE == e)
    {
        return "UNKNOWN_INVERT_OTHER_WIRE";
    }
    if (UNKNOWN_OTHER_WIRE)
    {
        return "UNKNOWN_OTHER_WIRE";
    }
        
    return "Not A State!!!!!";
}

State_Enum intToState(int i)
{
    switch(i)
    {
        case 0:
            return UNKNOWN;
        case 1:
            return ZERO;
        case 2:
            return ONE;
        case 3:
            return UNKNOWN_INVERT;
        case 4:
            return UNKNOWN_INVERT_OTHER_WIRE;
    




    default:
            return UNKNOWN_OTHER_WIRE;
    }
}


bool shortCut(Wire * a, Wire * b, short table, Wire * dest)
{
    if(dest->other != 0)
    {
        cout << "shoft circuit other != 0\n";
        cout << "is "<<dest->other<<"\n";
    }
    if(dest->refs != 0)
    {
        cout << "shoft circuit refs != 0\n";
        cout << "is "<<dest->refs<<"\n";
    }
    
    //cout << "enter short circuit\n";
    
    /*dest->refs = 0;
    dest->other = 0;*/
    
    //if gate is constant
    if( (a->state == ONE || a->state == ZERO) &&  (b->state == ONE || b->state == ZERO))
    {
        //evaluate
        short wa =0;
        if(a->state == ONE)
        {
            wa = 1;
        }
        
        short wb = 0;
        if(b->state == ONE)
        {
            wb = 1;
        }
        
        short entry = (wa << 1) | wb;
        
        //cout << "entry: "<<entry<<"\n"<<wa<<" "<<wb<<"\n";
        
        int res = (table >> entry) & 1;
        
        dest->state = ZERO;
        if(res == 1)
        {
            dest->state = ONE;
        }
        
        //cout << "test evaluate extensively, i.e. all 4*16 cases\n";
        
        return true;
    }
    

    
    //trivial short circuits
    if(table == 0)
    {
        dest->state = ZERO;
        return true;
    }
    
    if(table == 15)
    {
        dest->state = ONE;
        return true;
    }
    
    //std::cout << "finish cases for unknown invert";
    //std::cout << "test short circuit";
    
    if (a->state == UNKNOWN_INVERT)
    {
        table = invertTable(false,table);
    }
    
    if (b->state == UNKNOWN_INVERT)
    {
        table = invertTable(true,table);
    }
    

    
    //ip a
    if(table == 3)
    {
        if(a->state == ONE)
        {
            dest->state = ZERO;
            return true;
        }
            
        if(a->state == ZERO)
        {
            dest->state = ONE;
            return true;
        }
        
        if(dest == a)
        {
            if(dest->state == UNKNOWN)
            {
                dest->state = UNKNOWN_INVERT;
            }
            else if(dest->state == UNKNOWN_INVERT)
            {
                //cout << "check entry\n";
                dest->state = UNKNOWN_INVERT;
            }
            else if(dest->state == UNKNOWN_OTHER_WIRE)
            {
                dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            }
            else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            return true;
        }
        
        dest->other = a;
        dest->state = UNKNOWN_INVERT_OTHER_WIRE;
        
        if(a->state == UNKNOWN_OTHER_WIRE)
        {
            dest->other = a->other;
        }
        
        if(a->state == UNKNOWN_INVERT_OTHER_WIRE)
        {
            dest->other = a->other;
            
            dest->state = UNKNOWN_OTHER_WIRE;
        }
        dest->other->refs++; dest->other->addRef(dest);
        return true;
    }
    
    //ip b
    if(table == 5)
    {
        if(b->state == ONE)
        {
            dest->state = ZERO;
            return true;
        }
        
        if(b->state == ZERO)
        {
            dest->state = ONE;
            return true;
        }
        
        if(dest == b)
        {
            if(dest->state == UNKNOWN)
            {
                dest->state = UNKNOWN_INVERT;
            }
            else if(dest->state == UNKNOWN_INVERT)
            {
                //cout << "check entry\n";
                dest->state = UNKNOWN_INVERT;
            }
            else if(dest->state == UNKNOWN_OTHER_WIRE)
            {
                dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            }
            else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            return true;
        }
        
        dest->other = b;
        dest->state = UNKNOWN_INVERT_OTHER_WIRE;
        
        if(b->state == UNKNOWN_OTHER_WIRE)
        {
            dest->other = b->other;
        }
        
        if(b->state == UNKNOWN_INVERT_OTHER_WIRE)
        {
            dest->other = b->other;
            
            dest->state = UNKNOWN_OTHER_WIRE;
        }
        
        dest->other->refs++; dest->other->addRef(dest);
        return true;
    }
    
    //p b
    if(table == 10)
    {
        if(b->state == ONE)
        {
            dest->state = ONE;
            return true;
        }
        
        if(b->state == ZERO)
        {
            dest->state = ZERO;
            return true;
        }
        
        if(dest == b)
        {
            if(dest->state == UNKNOWN)
            {
                dest->state = UNKNOWN;
            }
            else if(dest->state == UNKNOWN_INVERT)
            {
                //cout << "check entry-2\n";
                dest->state = UNKNOWN;
            }
            else if(dest->state == UNKNOWN_OTHER_WIRE)
            {
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            }
            return true;
        }
        
        dest->other = b;
        dest->state = UNKNOWN_OTHER_WIRE;
        
        if(b->state == UNKNOWN_OTHER_WIRE)
        {
            dest->other = b->other;
        }

        if(b->state == UNKNOWN_INVERT_OTHER_WIRE)
        {
            dest->other = b->other;
            
            dest->state = UNKNOWN_INVERT_OTHER_WIRE;
        }
        dest->other->refs++; dest->other->addRef(dest);
        return true;
    }
    
    //p a
    if(table == 12)
    {
        if(a->state == ONE)
        {
            dest->state = ONE;
            return true;
        }
        
        if(a->state == ZERO)
        {
            dest->state = ZERO;
            return true;
        }
        
        
        if(dest == a)
        {
            if(dest->state == UNKNOWN)
            {
                dest->state = UNKNOWN;
            }
            else if(dest->state == UNKNOWN_INVERT)
            {
                dest->state = UNKNOWN;
            }
            else if(dest->state == UNKNOWN_OTHER_WIRE)
            {
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            }
            return true;
        }
        
        
        dest->other = a;
        dest->state = UNKNOWN_OTHER_WIRE;
        
        if(a->state == UNKNOWN_OTHER_WIRE)
        {
            dest->other = a->other;
        }
        
        if(a->state == UNKNOWN_INVERT_OTHER_WIRE)
        {
            dest->other = a->other;
            
            dest->state = UNKNOWN_INVERT_OTHER_WIRE;
        }
        dest->other->refs++; dest->other->addRef(dest);
        return true;
    }
    
    
    //slightly less trivial short circuits | if one OR value is one
    if(table == 14 && (a->state == ONE || b->state == ONE))
    {
        dest->state = ONE;
        return true;
    }
    
    // if one AND value is zero
    if(table == 8 && (a->state == ZERO || b->state == ZERO))
    {
        dest->state = ZERO;
        return true;
    }
    
    //one NOR value is one
    if(table == 1 && (a->state == ONE || b->state == ONE))
    {
        dest->state = ZERO;
        return true;
    }
    
    //one NAND value is zero
    if(table == 7 && (a->state == ZERO || b->state == ZERO))
    {
        dest->state = ONE;
        return true;
    }


    if(a->state == ONE)
    {
        int option0 = (table >> 2)&1;
        int option1 = (table >> 3)&1;
        
        if(option0 == 1 && option1 == 1)
        {
            dest->state = ONE;
            return true;
        }
        
        if(option0 == 0 && option1 == 0)
        {
            dest->state = ZERO;
            return true;
        }
        
        if(option0 == 0 && option1 == 1)
        {
            
            if(dest == b)
            {
                if(dest->state == UNKNOWN)
                {
                    dest->state = UNKNOWN;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry1\n";
                    dest->state = UNKNOWN;
                }
                /*else if(dest->state == UNKNOWN_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_OTHER_WIRE;
                }
                else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                }*/
                return true;
            }
            
            
            dest->other = b;
            dest->state = UNKNOWN_OTHER_WIRE;
            
            if(b->state == UNKNOWN_OTHER_WIRE)
            {
                dest->other = b->other;
            }
            
            if(b->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->other = b->other;
                dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            }
            
            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
        else if(option0 == 1 && option1 == 0)
        {

            if(dest == b)
            {
                if(dest->state == UNKNOWN)
                {
                    dest->state = UNKNOWN_INVERT;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry2\n";
                    dest->state = UNKNOWN_INVERT;
                }
                else if(dest->state == UNKNOWN_OTHER_WIRE)
                 {
                     dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                 }
                 else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                 {
                     dest->state = UNKNOWN_OTHER_WIRE;
                 }
                return true;
            }
            
            dest->other = b;
            dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            
            if(b->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->other = b->other;
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            
            if(b->state == UNKNOWN_OTHER_WIRE)
            {
                dest->other = b->other;
                dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            }
            
            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
    }
    else if(a->state == ZERO)
    {
        int option0 = (table >> 0)&1;
        int option1 = (table >> 1)&1;
        
        if(option0 == 1 && option1 == 1)
        {
            dest->state = ONE;
            return true;
        }
        if(option0 == 0 && option1 == 0)
        {
            dest->state = ZERO;
            return true;
        }
        
        if(option0 == 0 && option1 == 1)
        {
            
            if(dest == b)
            {
                if(dest->state == UNKNOWN)
                {
                    dest->state = UNKNOWN;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry3\n";
                    dest->state = UNKNOWN;
                }
                /*else if(dest->state == UNKNOWN_OTHER_WIRE)
                 {
                 dest->state = UNKNOWN_OTHER_WIRE;
                 }
                 else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                 {
                 dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                 }*/
                return true;
            }
            
            
            dest->other = b;
            dest->state = UNKNOWN_OTHER_WIRE;
            
            if(b->state == UNKNOWN_OTHER_WIRE)
            {
                dest->other = b->other;
            }
            
            if(b->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->other = b->other;
                dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            }
            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
        else if(option0 == 1 && option1 == 0)
        {
            
            
            if(dest == b)
            {
                if(dest->state == UNKNOWN)
                {
                    dest->state = UNKNOWN_INVERT;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry4\n";
                    dest->state = UNKNOWN_INVERT;
                }
                else if(dest->state == UNKNOWN_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                }
                else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_OTHER_WIRE;
                }
                return true;
            }
            
            
            dest->other = b;
            dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            
            if(b->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->other = b->other;
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            
            if(b->state == UNKNOWN_OTHER_WIRE)
            {
                dest->other = b->other;
                dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            }
            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
    }
    else if(b->state == ONE)
    {
        int option0 = (table >> 1)&1;
        int option1 = (table >> 3)&1;
        
        if(option0 == 1 && option1 == 1)
        {
            dest->state = ONE;
            return true;
        }
        if(option0 == 0 && option1 == 0)
        {
            dest->state = ZERO;
            return true;
        }
        
        
        if(option0 == 0 && option1 == 1)
        {

            if(dest == a)
            {
                if(dest->state == UNKNOWN)
                {
                    dest->state = UNKNOWN;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry5\n";
                    dest->state = UNKNOWN;
                }
                /*else if(dest->state == UNKNOWN_OTHER_WIRE)
                 {
                 dest->state = UNKNOWN_OTHER_WIRE;
                 }
                 else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                 {
                 dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                 }*/
                return true;
            }
            
            
            dest->other = a;
            dest->state = UNKNOWN_OTHER_WIRE;
            
            if(a->state == UNKNOWN_OTHER_WIRE)
            {
                dest->other = a->other;
            }
            
            if(a->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->other = a->other;
                
                dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            }
            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
        else if(option0 == 1 && option1 == 0)
        {
            
            if(dest == a)
            {
                if(dest->state == UNKNOWN)
                {
                    dest->state = UNKNOWN_INVERT;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry6\n";
                    dest->state = UNKNOWN_INVERT;
                }
                else if(dest->state == UNKNOWN_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                }
                else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_OTHER_WIRE;
                }
                return true;
            }
            
            dest->other = a;
            dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            
            if(a->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->other = a->other;
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            
            if(a->state == UNKNOWN_OTHER_WIRE)
            {
                dest->other = a->other;
                dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            }
            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
        
    }
    else if( b->state == ZERO)
    {
        int option0 = (table >> 0)&1;
        int option1 = (table >> 2)&1;
        
        if(option0 == 1 && option1 == 1)
        {
            dest->state = ONE;
            return true;
        }
        if(option0 == 0 && option1 == 0)
        {
            dest->state = ZERO;
            return true;
        }
        
        if(option0 == 0 && option1 == 1)
        {
            
            if(dest == a)
            {
                if(dest->state == UNKNOWN)
                {
                    dest->state = UNKNOWN;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry7\n";
                    dest->state = UNKNOWN;
                }
                /*else if(dest->state == UNKNOWN_OTHER_WIRE)
                 {
                 dest->state = UNKNOWN_OTHER_WIRE;
                 }
                 else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                 {
                 dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                 }*/
                return true;
            }
            
            
            dest->other = a;
            dest->state = UNKNOWN_OTHER_WIRE;
            
            if(a->state == UNKNOWN_OTHER_WIRE)
            {
                dest->other = a->other;
            }
            
            if(a->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->other = a->other;
                
                dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            }
            
            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
        else if(option0 == 1 && option1 == 0)
        {
            
            if(dest == a)
            {
                if(dest->state == UNKNOWN)
                {
                    dest->state = UNKNOWN_INVERT;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry8\n";
                    dest->state = UNKNOWN_INVERT;
                }
                else if(dest->state == UNKNOWN_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                }
                else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_OTHER_WIRE;
                }
                return true;
            }
            
            dest->other = a;
            dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            
            if(a->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->other = a->other;
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            
            if(a->state == UNKNOWN_OTHER_WIRE)
            {
                dest->other = a->other;
                dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            }
            
            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
    }
    
    
    //tables 0,3,5,10,12,15 already done at this point in time)
    //2,4,11,13 cannot be optimized in general (i think)
    //leaving 1 6 7 8 9 14
    
    
    if(a == b)
    {
        if(table == 6)
        {
            //put 0
            dest->state = ZERO;
        }
        else if (table == 9)
        {
            //put 1
            dest->state = ONE;
        }
        else if(table == 11 || table == 13)
        {
            dest->state = ONE;
        }
        else if(table == 2 || table == 4)
        {
            dest->state = ZERO;
        }
        else if(table == 14 || table == 8 )
        {
            //put a
            
            if(dest == a)
            {
                if(dest->state == UNKNOWN)
                {
                    dest->state = UNKNOWN;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    dest->state = UNKNOWN;
                }
                else if(dest->state == UNKNOWN_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_OTHER_WIRE;
                }
                else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                }
                return true;
            }

            if(a->state == UNKNOWN)
            {
                dest->state = UNKNOWN_OTHER_WIRE;
                dest->other = a;
            }
            else if(a->state == UNKNOWN_INVERT)
            {
                dest->state = UNKNOWN_OTHER_WIRE; //might be wrong
                dest->other = a;
            }
            else
            {
                dest->state = a->state;
                dest->other = a->other;
            }
            
            
            dest->other->refs++; dest->other->addRef(dest);
        }
        else if(table == 1 || table == 7)
        {
             //put invert a
            
            
            if(dest == a)
            {
                if(dest->state == UNKNOWN)
                {
                    dest->state = UNKNOWN_INVERT;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    dest->state = UNKNOWN_INVERT;
                }
                else if(dest->state == UNKNOWN_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                }
                else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_OTHER_WIRE;
                }
                return true;
            }
            
            
            dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            
            if(a->state == UNKNOWN)
            {
                dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                dest->other = a;
            }
            else if(a->state == UNKNOWN_INVERT)
            {
                dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                dest->other = a;
            }
            else if (a->state == UNKNOWN_OTHER_WIRE)
            {
                dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                dest->other = a->other;
            }
            else if (a->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->state = UNKNOWN_OTHER_WIRE;
                dest->other = a->other;
            }
            
            
            dest->other->refs++; dest->other->addRef(dest);
        }
        return true;
    }

    //cout << "leave short circuit\n";
    
    return false;
}

bool shortCutNoInvertOutput(Wire * a, Wire * b, short table, Wire * dest)
{
    if(dest->other != 0)
    {
        cout << "shoft circuitnio other != 0\n";
    }
    if(dest->refs != 0)
    {
        cout << "shoft circuitnio refs != 0\n";
    }
    
    //cout << "enter short circuit\n";
    
    /*dest->refs = 0;
    dest->other = 0;*/
    
    //if gate is constant
    if( (a->state == ONE || a->state == ZERO) &&  (b->state == ONE || b->state == ZERO))
    {
        //evaluate
        short wa =0;
        if(a->state == ONE)
        {
            wa = 1;
        }
        
        short wb = 0;
        if(b->state == ONE)
        {
            wb = 1;
        }
        
        short entry = (wa << 1) | wb;
        
        //cout << "entry: "<<entry<<"\n"<<wa<<" "<<wb<<"\n";
        
        int res = (table >> entry) & 1;
        
        dest->state = ZERO;
        if(res == 1)
        {
            dest->state = ONE;
        }
        
        //cout << "test evaluate extensively, i.e. all 4*16 cases\n";
        
        return true;
    }
    
    
    
    //trivial short circuits
    if(table == 0)
    {
        dest->state = ZERO;
        return true;
    }
    
    if(table == 15)
    {
        dest->state = ONE;
        return true;
    }
    
    //std::cout << "finish cases for unknown invert";
    //std::cout << "test short circuit";
    
    if (a->state == UNKNOWN_INVERT)
    {
        table = invertTable(false,table);
    }
    
    if (b->state == UNKNOWN_INVERT)
    {
        table = invertTable(true,table);
    }
    
    
    
    //ip a
    if(table == 3)
    {
        if(a->state == ONE)
        {
            dest->state = ZERO;
            return true;
        }
        
        if(a->state == ZERO)
        {
            dest->state = ONE;
            return true;
        }
        

        if(dest == a)
        {
            if(dest->state == UNKNOWN)
            {
                return false;
                //dest->state = UNKNOWN_INVERT;
            }
            else if(dest->state == UNKNOWN_INVERT)
            {
                //cout << "check entry\n";
                //dest->state = UNKNOWN_INVERT;
                return false;
            }
            else if(dest->state == UNKNOWN_OTHER_WIRE)
            {
                //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                return false;
            }
            else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            return true;
        }
        
        
        if(a->state == UNKNOWN_OTHER_WIRE)
        {
            //dest->other = a->other;
            return false;
        }
        
        dest->other = a;
        dest->state = UNKNOWN_INVERT_OTHER_WIRE;
        if(a->state == UNKNOWN_INVERT_OTHER_WIRE)
        {
            dest->other = a->other;
            
            dest->state = UNKNOWN_OTHER_WIRE;
        }
        dest->other->refs++; dest->other->addRef(dest);
        return true;
    }
    
    //ip b
    if(table == 5)
    {
        if(b->state == ONE)
        {
            dest->state = ZERO;
            return true;
        }
        
        if(b->state == ZERO)
        {
            dest->state = ONE;
            return true;
        }
        

        if(dest == b)
        {
            if(dest->state == UNKNOWN)
            {
                //dest->state = UNKNOWN_INVERT;
                return false;
            }
            else if(dest->state == UNKNOWN_INVERT)
            {
                //cout << "check entry\n";
                //dest->state = UNKNOWN_INVERT;
                return false;
            }
            else if(dest->state == UNKNOWN_OTHER_WIRE)
            {
                //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                return false;
            }
            else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            return true;
        }
        
        
        if(b->state == UNKNOWN_OTHER_WIRE)
        {
            //dest->other = b->other;
            return false;
        }
        
        dest->other = b;
        dest->state = UNKNOWN_INVERT_OTHER_WIRE;
        if(b->state == UNKNOWN_INVERT_OTHER_WIRE)
        {
            dest->other = b->other;
            
            dest->state = UNKNOWN_OTHER_WIRE;
        }
        
        dest->other->refs++; dest->other->addRef(dest);
        return true;
    }
    
    //p b
    if(table == 10)
    {
        if(b->state == ONE)
        {
            dest->state = ONE;
            return true;
        }
        
        if(b->state == ZERO)
        {
            dest->state = ZERO;
            return true;
        }
        
        if(dest == b)
        {
            if(dest->state == UNKNOWN)
            {
                dest->state = UNKNOWN;
            }
            else if(dest->state == UNKNOWN_INVERT)
            {
                //cout << "check entry-2\n";
                dest->state = UNKNOWN;
            }
            else if(dest->state == UNKNOWN_OTHER_WIRE)
            {
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                return false;
            }
            return true;
        }
        
        if(b->state == UNKNOWN_INVERT_OTHER_WIRE)
        {
            /*dest->other = b->other;
            dest->state = UNKNOWN_INVERT_OTHER_WIRE;*/
            return false;
        }
        
        
        dest->other = b;
        dest->state = UNKNOWN_OTHER_WIRE;
        if(b->state == UNKNOWN_OTHER_WIRE)
        {
            dest->other = b->other;
        }
        

        dest->other->refs++; dest->other->addRef(dest);
        return true;
    }
    
    //p a
    if(table == 12)
    {
        if(a->state == ONE)
        {
            dest->state = ONE;
            return true;
        }
        
        if(a->state == ZERO)
        {
            dest->state = ZERO;
            return true;
        }
        
        
        if(dest == a)
        {
            if(dest->state == UNKNOWN)
            {
                dest->state = UNKNOWN;
            }
            else if(dest->state == UNKNOWN_INVERT)
            {
                dest->state = UNKNOWN;
            }
            else if(dest->state == UNKNOWN_OTHER_WIRE)
            {
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                return false;
            }
            return true;
        }
        
        
        
        if(a->state == UNKNOWN_INVERT_OTHER_WIRE)
        {
            return false;
            //dest->other = a->other;
            //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
        }
        
        dest->other = a;
        dest->state = UNKNOWN_OTHER_WIRE;
        
        if(a->state == UNKNOWN_OTHER_WIRE)
        {
            dest->other = a->other;
        }
        

        dest->other->refs++; dest->other->addRef(dest);
        return true;
    }
    
    
    //slightly less trivial short circuits | if one OR value is one
    if(table == 14 && (a->state == ONE || b->state == ONE))
    {
        dest->state = ONE;
        return true;
    }
    
    // if one AND value is zero
    if(table == 8 && (a->state == ZERO || b->state == ZERO))
    {
        dest->state = ZERO;
        return true;
    }
    
    //one NOR value is one
    if(table == 1 && (a->state == ONE || b->state == ONE))
    {
        dest->state = ZERO;
        return true;
    }
    
    //one NAND value is zero
    if(table == 7 && (a->state == ZERO || b->state == ZERO))
    {
        dest->state = ONE;
        return true;
    }
    
    
    if(a->state == ONE)
    {
        int option0 = (table >> 2)&1;
        int option1 = (table >> 3)&1;
        
        if(option0 == 1 && option1 == 1)
        {
            dest->state = ONE;
            return true;
        }
        
        if(option0 == 0 && option1 == 0)
        {
            dest->state = ZERO;
            return true;
        }
        
        if(option0 == 0 && option1 == 1)
        {
            
            if(dest == b)
            {
                if(dest->state == UNKNOWN)
                {
                    dest->state = UNKNOWN;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry1\n";
                    dest->state = UNKNOWN;
                }
                /*else if(dest->state == UNKNOWN_OTHER_WIRE)
                 {
                 dest->state = UNKNOWN_OTHER_WIRE;
                 }
                 else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                 {
                 dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                 }*/
                return true;
            }
            
            
            
            if(b->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                //dest->other = b->other;
                //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                return false;
            }
            
            dest->other = b;
            dest->state = UNKNOWN_OTHER_WIRE;
            
            if(b->state == UNKNOWN_OTHER_WIRE)
            {
                dest->other = b->other;
            }
            
            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
        else if(option0 == 1 && option1 == 0)
        {
            
            if(dest == b)
            {
                if(dest->state == UNKNOWN)
                {
                    //dest->state = UNKNOWN_INVERT;
                    return false;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry2\n";
                    //dest->state = UNKNOWN_INVERT;
                    return false;
                }
                else if(dest->state == UNKNOWN_OTHER_WIRE)
                {
                    //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                    return false;
                }
                else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_OTHER_WIRE;
                }
                return true;
            }
            
            
            
            if(b->state == UNKNOWN_OTHER_WIRE)
            {
                //dest->other = b->other;
                //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                return false;
            }
            
            dest->other = b;
            dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            
            if(b->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->other = b->other;
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            
            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
    }
    else if(a->state == ZERO)
    {
        int option0 = (table >> 0)&1;
        int option1 = (table >> 1)&1;
        
        if(option0 == 1 && option1 == 1)
        {
            dest->state = ONE;
            return true;
        }
        if(option0 == 0 && option1 == 0)
        {
            dest->state = ZERO;
            return true;
        }
        
        if(option0 == 0 && option1 == 1)
        {
            
            if(dest == b)
            {
                if(dest->state == UNKNOWN)
                {
                    dest->state = UNKNOWN;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry3\n";
                    dest->state = UNKNOWN;
                }
                /*else if(dest->state == UNKNOWN_OTHER_WIRE)
                 {
                 dest->state = UNKNOWN_OTHER_WIRE;
                 }
                 else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                 {
                 dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                 }*/
                return true;
            }
            
            
            if(b->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                //dest->other = b->other;
                //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                return false;
            }
            
            dest->other = b;
            dest->state = UNKNOWN_OTHER_WIRE;
            
            if(b->state == UNKNOWN_OTHER_WIRE)
            {
                dest->other = b->other;
            }
            

            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
        else if(option0 == 1 && option1 == 0)
        {
            
            
            if(dest == b)
            {
                if(dest->state == UNKNOWN)
                {
                    //dest->state = UNKNOWN_INVERT;
                    return false;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry4\n";
                   // dest->state = UNKNOWN_INVERT;
                    return false;
                }
                else if(dest->state == UNKNOWN_OTHER_WIRE)
                {
                    //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                    return false;
                }
                else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_OTHER_WIRE;
                }
                return true;
            }
            
            
            
            if(b->state == UNKNOWN_OTHER_WIRE)
            {
                //dest->other = b->other;
                //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                return false;
            }
            
            dest->other = b;
            dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            
            if(b->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->other = b->other;
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            

            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
    }
    else if(b->state == ONE)
    {
        int option0 = (table >> 1)&1;
        int option1 = (table >> 3)&1;
        
        if(option0 == 1 && option1 == 1)
        {
            dest->state = ONE;
            return true;
        }
        if(option0 == 0 && option1 == 0)
        {
            dest->state = ZERO;
            return true;
        }
        
        
        if(option0 == 0 && option1 == 1)
        {
            
            
            if(dest == a)
            {
                if(dest->state == UNKNOWN)
                {
                    dest->state = UNKNOWN;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry5\n";
                    dest->state = UNKNOWN;
                }
                /*else if(dest->state == UNKNOWN_OTHER_WIRE)
                 {
                 dest->state = UNKNOWN_OTHER_WIRE;
                 }
                 else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                 {
                 dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                 }*/
                return true;
            }
            
            
            if(a->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                //dest->other = a->other;
                
                //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                return false;
            }
            
            dest->other = a;
            dest->state = UNKNOWN_OTHER_WIRE;
            
            if(a->state == UNKNOWN_OTHER_WIRE)
            {
                dest->other = a->other;
            }
            

            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
        else if(option0 == 1 && option1 == 0)
        {
            
            
            if(dest == a)
            {
                if(dest->state == UNKNOWN)
                {
                    //dest->state = UNKNOWN_INVERT;
                    return false;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry6\n";
                    //dest->state = UNKNOWN_INVERT;
                    return false;
                }
                else if(dest->state == UNKNOWN_OTHER_WIRE)
                {
                    //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                    return false;
                }
                else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_OTHER_WIRE;
                }
                return true;
            }
            
            
            if(a->state == UNKNOWN_OTHER_WIRE)
            {
                //dest->other = a->other;
                //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                return false;
            }
            
            dest->other = a;
            dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            
            if(a->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->other = a->other;
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            

            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
        
    }
    else if( b->state == ZERO)
    {
        int option0 = (table >> 0)&1;
        int option1 = (table >> 2)&1;
        
        if(option0 == 1 && option1 == 1)
        {
            dest->state = ONE;
            return true;
        }
        if(option0 == 0 && option1 == 0)
        {
            dest->state = ZERO;
            return true;
        }
        
        if(option0 == 0 && option1 == 1)
        {
            
            if(dest == a)
            {
                if(dest->state == UNKNOWN)
                {
                    dest->state = UNKNOWN;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry7\n";
                    dest->state = UNKNOWN;
                }
                /*else if(dest->state == UNKNOWN_OTHER_WIRE)
                 {
                 dest->state = UNKNOWN_OTHER_WIRE;
                 }
                 else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                 {
                 dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                 }*/
                return true;
            }
            
            
            if(a->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                //dest->other = a->other;
                //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                return false;
            }
            
            dest->other = a;
            dest->state = UNKNOWN_OTHER_WIRE;
            
            if(a->state == UNKNOWN_OTHER_WIRE)
            {
                dest->other = a->other;
            }
            

            
            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
        else if(option0 == 1 && option1 == 0)
        {
            
            
            if(dest == a)
            {
                if(dest->state == UNKNOWN)
                {
                    //dest->state = UNKNOWN_INVERT;
                    return false;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    //cout << "check entry8\n";
                    //dest->state = UNKNOWN_INVERT;
                    return false;
                }
                else if(dest->state == UNKNOWN_OTHER_WIRE)
                {
                    //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                    return false;
                }
                else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_OTHER_WIRE;
                }
                return true;
            }
            
            
            if(a->state == UNKNOWN_OTHER_WIRE)
            {
                //dest->other = a->other;
                //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                return false;
            }
            
            dest->other = a;
            dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            
            if(a->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->other = a->other;
                dest->state = UNKNOWN_OTHER_WIRE;
            }
            

            
            dest->other->refs++; dest->other->addRef(dest);
            return true;
        }
    }
    
    
    //tables 0,3,5,10,12,15 already done at this point in time)
    //2,4,11,13 cannot be optimized (i think)
    //leaving 1 6 7 8 9 14
    if(a == b)
    {
        if(table == 6)
        {
            //put 0
            dest->state = ZERO;
        }
        else if (table == 9)
        {
            //put 1
            dest->state = ONE;
        }
        else if(table == 11 || table == 13)
        {
            dest->state = ONE;
        }
        else if(table == 2 || table == 4)
        {
            dest->state = ZERO;
        }
        else if(table == 14 || table == 8 )
        {
            //put a
            
            if(dest == a)
            {
                if(dest->state == UNKNOWN)
                {
                    dest->state = UNKNOWN;
                }
                else if(dest->state == UNKNOWN_INVERT)
                {
                    dest->state = UNKNOWN;
                }
                else if(dest->state == UNKNOWN_OTHER_WIRE)
                {
                    dest->state = UNKNOWN_OTHER_WIRE;
                }
                else if(dest->state == UNKNOWN_INVERT_OTHER_WIRE)
                {
                    //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                    return false;
                }
                return true;
            }
            
            if(a->state == UNKNOWN)
            {
                dest->state = UNKNOWN_OTHER_WIRE;
                dest->other = a;
            }
            else if(a->state == UNKNOWN_INVERT)
            {
                dest->state = UNKNOWN_OTHER_WIRE;
                dest->other = a;
            }
            else
            {
                dest->state = a->state;
                dest->other = a->other;
            }
            
            
            dest->other->refs++; dest->other->addRef(dest);
        }
        else if(table == 1 || table == 7)
        {
            if(a->state == UNKNOWN || a->state == UNKNOWN_OTHER_WIRE)
            {
                return false;
            }
            
            //put invert a
            dest->state = UNKNOWN_INVERT_OTHER_WIRE;
            
            if(a->state == UNKNOWN)
            {
                //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                //dest->other = a;
                return false;
            }
            else if(a->state == UNKNOWN_INVERT)
            {
                //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                //dest->other = a;
                return false;
            }
            else if (a->state == UNKNOWN_OTHER_WIRE)
            {
                //dest->state = UNKNOWN_INVERT_OTHER_WIRE;
                //dest->other = a->other;
                return false;
            }
            else if (a->state == UNKNOWN_INVERT_OTHER_WIRE)
            {
                dest->state = UNKNOWN_OTHER_WIRE;
                dest->other = a->other;
            }
            
            
            dest->other->refs++; dest->other->addRef(dest);
        }
        return true;
    }
    
    //cout << "leave short circuit\n";
    
    return false;
}


//duplo
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
string toStrGate(short table)
{
	switch (table)
	{
	case 0:
		return "0000"; //represent 0
	case 1:
		return "0001";
	case 2:
		return "0010";
	case 3:
		return "0011"; 
	case 4:
		return "0100"; 
	case 5:
		return "0101"; 
	case 6:
		return "0110";
	case 7:
		return "0111";
	case 8:
		return "1000";
	case 9:
		return "1001";
	case 10:
		return "1010";
	case 11:
		return "1011";
	case 12:
		return "1100";
	case 13:
		return "1101";	
	case 14:
		return "1110";
	case 15:
		return "1111"; //represent 1
	}
	return "Error " + to_string(table);
}

void printWire(Wire * a)
{
    cout << "Wire: " <<a->wireNumber<<"\n";
    cout << "\t state: "<< WireStateToString(a->state);
    if(a->other != 0)
    {
        cout <<" -> "<<a->other->wireNumber<<"\n";
    }
    else
    {
        cout <<"\n";
    }
    cout << "\t wireNumber: "<< a->wireNumber<<"\n";
    cout << "\t refs: "<< a->refs<<"\n";
    cout << "\t locked: "<< a->locked<<"\n";
    cout << "\t varlocked: "<< a->varlocked<<"\n";
    cout << "\t refs to me: "<<a->refsToMeSize<<"\n";
}
