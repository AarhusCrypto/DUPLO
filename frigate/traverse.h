#include "ast.h"

#ifndef TRAVERSE_H
#define TRAVERSE_H

//class Node;

using namespace std;

template <class T>
class BottomUpTraverse
{
protected:
    Node_Enum nodetype;
    
public:
    virtual ~BottomUpTraverse(){}
    
    BottomUpTraverse(){}

    T traverse(Node * topnode)
    {
        //T t;
        
        T t= bottomUpTraversal(topnode);
        return t;
    }
    
    T bottomUpTraversal(Node * current)
    {
        int num = current->getNumChildren();
        
        vector<T> accu;
        
        for(int i=0;i<num;i++)
        {
            if(current->getChild(i) != 0)
            {
                accu.push_back(bottomUpTraversal(current->getChild(i)));
            }
        }
        if(current != 0)
        {
            T temp = visit(current,accu);
            
            return temp;
        }
        
        //cannot ever reach here
        T tx;
        return tx;
        
        //return attribute::attribute();

    }
    
    virtual T visit(Node * current, vector<T> vec)=0;
};


template <class T>
class TopDownTraverse
{
protected:
    Node_Enum nodetype;
    
public:
    
    virtual ~TopDownTraverse(){}
    
    TopDownTraverse(){}
    
    void traverse(Node * topnode)
    {
        T t;
        topDownTraversal(topnode,t);
        //return t;
    }

    void topDownTraversal(Node * current, T down)
    {
        int num = current->getNumChildren();
        
        T temp;
        
        if(current != 0)
        {
            temp = visit(current, down);
        }
        
        for(int i=0;i<num;i++)
        {
            if(current->getChild(i)!=0)
            {
                topDownTraversal(current->getChild(i), temp);
            }
        }
    }
    
    virtual T visit(Node * current, T t)=0;
};

#endif
