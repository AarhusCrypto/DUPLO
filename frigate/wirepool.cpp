//
//  WirePool.cpp
//  
//
//
//

#include "wirepool.h"
#include "wire.h"
#include <iostream>

using namespace std;



void WirePool::freeAll()
{
    //cout << "startfreeall\n";
    
    long length;
    
    for (std::unordered_map<long,WirePoolLLHeadNode *>::iterator it = WireSetMap.begin(); it!= WireSetMap.end(); ++it )
    {
        WirePoolLLHeadNode * hn =  ((WirePoolLLHeadNode *)(it->second));
        
        length = it->first;
    
        WirePoolLLNode * ptr = hn->headUsed;
        WirePoolLLNode * tmp;
        for(int i=0;i<hn->usedSize;i++)
        {
            tmp = hn->headUsed->next;
            hn->headUsed->next = hn->headFree;
            hn->headFree = hn->headUsed;
            hn->headUsed= tmp;
            
            for(int j=0;j<length;j++)
            {
                hn->headFree->setData.wires[j]->type = FREE_wt;
                hn->headFree->setData.wires[j]->refs = 0;
                hn->headFree->setData.wires[j]->locked = false;
                hn->headFree->setData.wires[j]->varlocked = false;
                hn->headFree->setData.wires[j]->other = 0;
            }
            hn->freeSize++;
        }
        
        hn->headUsed = 0;
        hn->usedSize=0;
            
        
    }
        
    largestsize=0;
    
    //cout << "endfreeall\n";
}

void WirePool::freeIfNoRefs()
{
    
    long length;
    //int totalcheck=0;
    
    for (std::unordered_map<long,WirePoolLLHeadNode *>::iterator it = WireSetMap.begin(); it!= WireSetMap.end(); ++it )
    {
        WirePoolLLHeadNode * hn =  ((WirePoolLLHeadNode *)(it->second));
        
        length = it->first;
        
        //cout << "length "<<length <<" "<<hn->usedSize<<"\n";
        
        if(hn->usedSize == 0)
            continue;
        
        
        
    
    
        WirePoolLLNode * ptr = hn->headUsed;
        WirePoolLLNode * tmp, * lastptr=0;
        
        while(ptr != 0 )
        {
            
            int j=0;
            for(;j<length;j++)
            {
                if(ptr->setData.wires[j]->refs == 0 && ptr->setData.wires[j]->locked == false && ptr->setData.wires[j]->varlocked == false)
                {
                    //totalcheck++;
                }
                else
                {
                    break;
                }
            }
            
            if(j==length)
            {
                for(j=0;j<length;j++)
                {
                    if(ptr->setData.wires[j]->other != 0)
                    {
                        ptr->setData.wires[j]->other->refs--; ptr->setData.wires[j]->other->removeRef(ptr->setData.wires[j]);
                    }
                    
                    ptr->setData.wires[j]->other = 0;
                    ptr->setData.wires[j]->type = FREE_wt;
                }
                tmp = ptr;
                ptr = ptr->next;
                
                if(hn->headUsed == tmp)
                {
                    hn->headUsed = tmp->next;
                }
                else
                {
                    lastptr->next = tmp->next;
                }
                
                
                tmp->next = hn->headFree;
                hn->headFree = tmp;
                
                //lastptr = tmp;
                hn->usedSize--;
                hn->freeSize++;
                
                //cout <<"remove\n";
            }
            else
            {
                //cout <<"pass"<<ptr<<"\n";
                
                lastptr = ptr;
                ptr = ptr->next;
            }
        }
    }
    
    //cout <<"totalcheck : "<<totalcheck<<"\n";
    //cout << "endiffree\n";
}

WireSet * WirePool::getWires(long length)
{
    WirePoolLLHeadNode * hn = WireSetMap[length];
    
    if(hn == 0)
    {
        WireSetMap[length] = new WirePoolLLHeadNode();
        hn = WireSetMap[length];
    }
    
    if(hn->headFree == 0)
    {
        hn->headFree = new WirePoolLLNode();
        hn->headFree->setData.wires = new Wire*[length];
        for(int j=0;j<length;j++)
        {
            hn->headFree->setData.wires[j] = new Wire();
            hn->headFree->setData.wires[j]->wireNumber = wireNumberValue++;
            hn->headFree->setData.wires[j]->state =ZERO;
        }
        hn->freeSize++;
    }
    
    WirePoolLLNode * tmp;
    tmp = hn->headFree;
    
    for(int j=0;j<length;j++)
    {
        tmp->setData.wires[j]->refs = 0;
        tmp->setData.wires[j]->state = ZERO;
        if(largestsize <tmp->setData.wires[j]->wireNumber)
        {
            largestsize = tmp->setData.wires[j]->wireNumber;
        }
    }
    
    hn->headFree = hn->headFree->next;
    hn->freeSize--;
    hn->usedSize++;
    tmp->next = hn->headUsed;
    hn->headUsed = tmp;
    
    return &(tmp->setData);
}



Wire * WirePool::getWire()
{
    //cout << "stargetwire\n";
    
    if(WireSetMap[1]->headFree == 0)
    {
        WireSetMap[1]->headFree = new WirePoolLLNode();
        WireSetMap[1]->headFree->setData.wires = new Wire*[1];
        WireSetMap[1]->headFree->setData.wires[0] = new Wire();
        WireSetMap[1]->headFree->setData.wires[0]->wireNumber = wireNumberValue++;
        WireSetMap[1]->headFree->setData.wires[0]->state =ZERO;
        WireSetMap[1]->freeSize++;
    }
    
    WirePoolLLNode * tmp;
    tmp = WireSetMap[1]->headFree;
    tmp->setData.wires[0]->refs = 0;

    WireSetMap[1]->headFree = WireSetMap[1]->headFree->next;
    WireSetMap[1]->freeSize--;
    WireSetMap[1]->usedSize++;
    if(largestsize <tmp->setData.wires[0]->wireNumber)
    {
        largestsize = tmp->setData.wires[0]->wireNumber;
    }
    
    tmp->next = WireSetMap[1]->headUsed;
    WireSetMap[1]->headUsed = tmp;
    
    tmp->setData.wires[0]->state = ZERO;
    
    
    
    //cout << "endgetwire\n";
    
    return tmp->setData.wires[0];
}

void WirePool::assignWireNumbers(long base)
{
    
    long length;
    
    for (std::unordered_map<long,WirePoolLLHeadNode *>::iterator it = WireSetMap.begin(); it!= WireSetMap.end(); ++it )
    {
        WirePoolLLHeadNode * hn =  ((WirePoolLLHeadNode *)(it->second));
        
        length = it->first;
    
        if(hn->usedSize != 0)
        {
            std::cout <<"error, assigning wireNumbers with usedSize != 0\n";
            exit(1);
        }
        
        //cout << "base start is: "<<base<<"\n";
        
        largestsize = base-1;
        WirePoolLLNode * tmp = hn->headFree;
        for(int i=0;i<hn->freeSize;i++)
        {
            for(int j=0;j<length;j++)
            {
                tmp->setData.wires[j]->wireNumber = base++;
            }
            tmp = tmp->next;
        }
    
    }
    
    wireNumberValue = base;
    //largestsize=0;
}

#include "variable.h"

void WirePool::printUsedPoolState()
{
    
    long length;
    
    for (std::unordered_map<long,WirePoolLLHeadNode *>::iterator it = WireSetMap.begin(); it!= WireSetMap.end(); ++it )
    {
        WirePoolLLHeadNode * hn =  ((WirePoolLLHeadNode *)(it->second));
        
        WirePoolLLNode * tmp = hn->headUsed;
        
        length = it->first;
        
        if(hn->usedSize <=0)
        {
            continue;
        }
        
        cout << "\n\nWire set size: "<<length<<"\n";
        for(int i=0;i<hn->usedSize;i++)
        {
            for(int j=0;j<length;j++)
            {
                cout << "Wire: " <<tmp->setData.wires[j]->wireNumber<<"\n";
                cout << "\t state: "<< WireStateToString(tmp->setData.wires[j]->state);
                if(tmp->setData.wires[j]->other != 0)
                {
                    cout <<" -> "<<tmp->setData.wires[j]->other->wireNumber<<"\n";
                }
                else
                {
                    cout <<"\n";
                }
                
                cout << "\t wireNumber: "<< tmp->setData.wires[j]->wireNumber<<"\n";
                cout << "\t refs: "<< tmp->setData.wires[j]->refs<<"\n";
                cout << "\t locked: "<< tmp->setData.wires[j]->locked<<"\n";
                cout << "\t varlocked: "<< tmp->setData.wires[j]->varlocked;
                
                if(tmp->setData.wires[j]->thisvar != 0 && tmp->setData.wires[j]->varlocked)
                {
                    cout <<" -> "<<tmp->setData.wires[j]->thisvar->name<<"\n";
                }
                else
                {
                    cout <<"\n";
                }
            }
            tmp=tmp->next;
        }
    }
}

void WirePool::printFreePoolState()
{

    long length;
    
    for (std::unordered_map<long,WirePoolLLHeadNode *>::iterator it = WireSetMap.begin(); it!= WireSetMap.end(); ++it )
    {
        WirePoolLLHeadNode * hn =  ((WirePoolLLHeadNode *)(it->second));
        
        WirePoolLLNode * tmp = hn->headFree;
    
        length = it->first;
        
        if(hn->freeSize <=0)
        {
            continue;
        }
    
        cout << "\n\nWire set size: "<<length<<"\n";
        for(int i=0;i<hn->freeSize;i++)
        {
            for(int j=0;j<length;j++)
            {
                cout << "Wire: " <<tmp->setData.wires[j]->wireNumber<<"\n";
                cout << "\t state: "<< WireStateToString(tmp->setData.wires[j]->state);
                if(tmp->setData.wires[j]->other != 0)
                {
                    cout <<" -> "<<tmp->setData.wires[j]->other->wireNumber<<"\n";
                }
                else
                {
                    cout <<"\n";
                }
                
                cout << "\t wireNumber: "<< tmp->setData.wires[j]->wireNumber<<"\n";
                cout << "\t refs: "<< tmp->setData.wires[j]->refs<<"\n";
                cout << "\t locked: "<< tmp->setData.wires[j]->locked<<"\n";
                cout << "\t varlocked: "<< tmp->setData.wires[j]->varlocked<<"\n";
            }
            tmp=tmp->next;
        }
    }
}

//radux sort
void WirePool::sortFreeWires()
{
    WirePoolLLNode ** buckets = new WirePoolLLNode*[10]; //base 10
    WirePoolLLNode ** tempholder = new WirePoolLLNode*[10]; //base 10
    
    
    long length;
    
    for (std::unordered_map<long,WirePoolLLHeadNode *>::iterator it = WireSetMap.begin(); it!= WireSetMap.end(); ++it )
    {
        WirePoolLLHeadNode * hn =  ((WirePoolLLHeadNode *)(it->second));
        
        length = it->first;
    
        WirePoolLLNode * head = hn->headFree;
        
        long max=1;
        
        for(int i=0;i<10;i++)
        {
            buckets[i]=0;
        }
        
        //cap of x10^19 in wire size in sorting
        for(int mult =0;mult<19;mult++)
        {
            if(mult == 0)
            {
                max = 1;
            }
            else
            {
                max = max *10;
            }
            //cout << "2\n";
            
            while(head != 0)
            {
                //cout << "1a\n";
                int bucket = (head->setData.wires[0]->wireNumber/(max))%10;

                if(buckets[bucket] == 0)
                {
                    //cout << "1c\n";
                    buckets[bucket] = head;
                    tempholder[bucket] = head;
                }
                else
                {
                    //cout << "1dn";
                    tempholder[bucket]->next = head;
                    tempholder[bucket] = tempholder[bucket]->next;
                }
                
                head = head->next;
            }
            
            bool headfound =false;
            WirePoolLLNode * last=0;
            
            //merge lists
            for(int i=0;i<10;i++)
            {
                if(headfound)
                {
                    if(buckets[i] != 0)
                    {
                        last->next = buckets[i];
                        last = tempholder[i];
                    }
                }
                else
                {
                    if(buckets[i] != 0)
                    {
                        head = buckets[i];
                        headfound=true;
                        last = tempholder[i];
                    }
                }
            }
            if(last != 0)
            {
                last->next = 0;
            }
            
            for(int i=0;i<10;i++)
            {
                buckets[i]=0;
            }
            
        }
        
        //check correct
        WirePoolLLNode * tmp = head;
        while(tmp != 0 && tmp->next != 0)
        {
            if(tmp->setData.wires[0]->wireNumber >= tmp->next->setData.wires[0]->wireNumber)
            {
                printFreePoolState();
                cout << "error out of order in sort\nsee wire: "<<tmp->setData.wires[0]->wireNumber<<"\n";
                exit(1);
            }
            
            tmp = tmp->next;
        }
        
        
        
        hn->headFree = head;
        
    }
    
    delete [] buckets;
    delete [] tempholder;
}













