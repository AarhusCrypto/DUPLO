//
//  WirePool.h
//  
//
//

#ifndef ____WirePool__
#define ____WirePool__

#include <stdio.h>

#include "wire.h"
#include <vector>
#include <unordered_map>

class WirePool;


class WireSet
{
public:
    Wire ** wires;
    //int numlocked;
    WireSet()
    {
        //numlocked = 0;
        wires=0;
        
    }
};

class WirePoolLLNode
{
public:
    WirePoolLLNode()
    {
        //data = 0;
        next = 0;
        //setData = 0;
    }
    
    //Wire * data;
    WireSet setData;
    WirePoolLLNode * next;
};

class WirePoolLLHeadNode
{
public:
    WirePoolLLHeadNode()
    {
        headFree = 0;
        headUsed = 0;
        freeSize=0;
        usedSize=0;
    }
    WirePoolLLNode * headFree;
    int freeSize;
    
    WirePoolLLNode * headUsed; //should add in reverse,
    int usedSize;

};


class WirePool
{
public:
    WirePool()
    {
        //headFree = 0;
        //headUsed = 0;
        wireNumberValue=0;
        largestsize=0;
        WireSetMap[1] = new WirePoolLLHeadNode();
    }
    
    //temp ONLY for returning from getwires, do not use any other place.
    WireSet ReturnWireSet;
    
    std::unordered_map<long,WirePoolLLHeadNode *> WireSetMap;
    
    //free should point to the lowest free node at first,
    //WirePoolLLNode * headFree;
    //int freeSize;
    
    //WirePoolLLNode * headUsed; //should add in reverse,
    //int usedSize;
    
    void freeAll();
    void freeIfNoRefs();
    
    
    Wire * getWire();
    WireSet * getWires(long size);
    
    void assignWireNumbers(long base);
    int wireNumberValue;
    int largestsize;
    
    void printUsedPoolState();
    void printFreePoolState();
    void sortFreeWires();
};






#endif /* defined(____WirePool__) */
