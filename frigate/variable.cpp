//
//  variable.cpp
//  
//
//  Created by Benjamin Mood on 12/15/14.
//
//

#include "variable.h"
#include "wirepool.h"
#include "circuitoutput.h"
#include "wire.h"

Wire * getWire(int i, WireHolderNode * n)
{
    //cout << "gw0\n";
    
    if(n->vec!= 0)
    {
        /*cout << "gw1\n";
        cout << i << " "<<n->startwirenum<<"\n";
        cout <<"return wire from getwire: "<<((*(n->vec))[i-n->startwirenum])<<"\n";*/
        return ((*(n->vec))[i-n->startwirenum]);
    }
    else
    {
        if(n->vec_whn != 0)
        {
            //cout << "gw2\n";
            int nsize = n->coresponding_var->size();
            int index = (i-n->startwirenum)/nsize;

            if(index != 0)
            {
              //  cout << "gw3\n";
                return getWire(i,n->vec_whn->at(index)->node);
            }
            else
            {
              //  cout << "gw4\n";
                return getWire(i,n->node);
            }
        }
        
        
        while(n != 0 && (!( n->startwirenum <= i && i <= n->endwirenum)) )
        {
            n = n->next;
        }
        
        //cout << "gw5\n";
        
        if(n == 0)
        {
            cout << "error: getwire returns 0\n";
            return 0;
        }
        else
        {
           // cout << "gw6\n";
            return getWire(i,n->node);
        }
    }
}

void setWire(int i, WireHolderNode * n, Wire * w)
{
    if(n->vec!= 0)
    {
        (n->vec)->operator[](i-n->startwirenum) = w;
        return;
    }
    else
    {
        if(n->vec_whn != 0)
        {
            int nsize = n->coresponding_var->size();
            int index = (i-n->startwirenum)/nsize;
            
            if(index != 0)
            {
                setWire(i,n->vec_whn->at(index)->node,w);
                return;
            }
            else
            {
                setWire(i,n->node,w);
                return;
            }
        }
        
        
        while(n != 0 && (!( n->startwirenum <= i && i <= n->endwirenum)) )
        {
            n = n->next;
        }
        
        if(n == 0)
        {
            cout << "error: getwire returns 0\n";
            return;
        }
        else
        {
            setWire(i,n->node,w);
            return;
        }
    }
}

long setSizes(WireHolderNode * n, int count)
{
    int startsize=count;
    while(n != 0)
    {
        n->startwirenum = startsize;

        if(n->coresponding_var != 0)
        {
            n->endwirenum = startsize+n->coresponding_var->size()-1;
        }
        else
        {
            n->endwirenum = startsize+n->vec->size()-1;
        }
        //cout <<"pass1\n";
        
        if(n->node != 0)
        {
            setSizes(n->node,n->startwirenum);
        }
        
        //cout << "start2\n";
        
        if(n->coresponding_var != 0)
        {
            startsize = startsize+n->coresponding_var->size();
        }
        else
        {
            startsize = startsize+n->vec->size();
        }
        //cout <<"pass2\n";
        
        n = n->next;
    }
    return startsize;
}

void ArrayVariable::FillInType(bool usePool)
{
    long thissize = isArrayType(type)->sizes[0];
    
    av.resize(thissize);
    
    
    
    //gentype if nec
    Type * pregen;
    if(isArrayType(type)->sizes.size() > 1)
    {
        string prefix = isArrayType(type)->type->typeName;
        string suffix = "";
        
        vector<long> sizes;
        for(int i=1;i<isArrayType(type)->sizes.size();i++)
        {
            suffix+="["+to_string(isArrayType(type)->sizes[i]);
            sizes.push_back(isArrayType(type)->sizes[i]);
        }
        
        Type * newt = getTypeMap()[prefix+suffix];
        if(newt == 0)
        {
            newt = new ArrayType(prefix);
            for(int L=0;L<sizes.size();L++)
            {
                ((ArrayType *) (newt))->sizes.push_back(sizes[L]);
            }
            ((ArrayType *) (newt))->type = getTypeMap()[prefix];
        }
        //return newt;
        
        
        pregen = newt;
    }
    
    wv = new WireHolderNode(0);
    WireHolderNode * wptr = wv,  * lptr;
    
    wv->vec_whn = new vector<WireHolderNode *>();
    
    wv->vec_whn->resize(thissize);
    
    for(int i=0;i<thissize;i++)
    {
        if(isArrayType(type)->sizes.size()==1)
        {
            if(isStructType(isArrayType(type)->type))
            {
                av[i] = new StructVariable();
                av[i]->type = isArrayType(type)->type;
            }
            else if(isIntType(isArrayType(type)->type))
            {
                av[i] = new IntVariable();
                av[i]->type = isArrayType(type)->type;
            }
            else if(isUIntType(isArrayType(type)->type))
            {
                av[i] = new IntVariable();
                av[i]->type = isArrayType(type)->type;
            }
            
            av[i]->FillInType(usePool);
        }
        else
        {
            av[i] = new ArrayVariable();
            av[i]->type = pregen;
            av[i]->FillInType(usePool);
        }
        
        wptr->node = av[i]->wv;
        wptr->coresponding_var = av[i];
        
        wv->vec_whn->operator[](i) = wptr;
        
        if(i != thissize -1)
        {
            wptr->next = new WireHolderNode(0);
            wptr = wptr->next;
        }
    }
}


void StructVariable::FillInType(bool usePool)
{
    
    wv = new WireHolderNode(0);
    WireHolderNode * wptr = wv,  * lptr;
    
    StructType * st = isStructType(type);
    for (std::unordered_map<string,StructItem *>::iterator it = st->si.begin(); it!= st->si.end(); ++it )
    {
        StructItem * sItem =  (StructItem *)(it->second);
        map[sItem->name];
        
        if(isStructType(sItem->type))
        {
            map[sItem->name] = new StructVariable();
        }
        else if(isIntType(sItem->type))
        {
            map[sItem->name] = new IntVariable();
        }
        else if(isUIntType(sItem->type))
        {
            map[sItem->name] = new IntVariable();
        }
        else if(isArrayType(sItem->type))
        {
            map[sItem->name] = new ArrayVariable();
        }
        
        map[sItem->name]->name = sItem->name;
        
        map[sItem->name]->type = sItem->type;
        map[sItem->name]->FillInType(usePool);
        
        wptr->node = map[sItem->name]->wv;
        wptr->coresponding_var = map[sItem->name];
        
        wptr->next = new WireHolderNode(0);
        lptr = wptr;
        wptr = wptr->next;
    }
    
    if(lptr->next != 0)
    {
        delete lptr->next;
        lptr->next = 0;
    }
}

void  IntVariable::FillInType(bool usePool)
{
    if(isIntType(type))
    {
        IntType * t = isIntType(type);
        wires.resize(t->size);
    }
    else if (isUIntType(type))
    {
        UIntType * t = isUIntType(type);
        wires.resize(t->size);
    }
    
    if(!usePool)
    {
        for(int i=0;i<wires.size();i++)
        {
            wires[i] = new Wire();
        }
    }
    else
    {
        /*for(int i=0;i<wires.size();i++)
        {
            wires[i] = getPool()->getWire();
        }*/
    }
    
    wv = new WireHolderNode(this);
    wv->vec = &(wires);
    wv->node = 0;
    wv->coresponding_var = this;
}

void FunctionVariable::FillInType(bool usePool)
{
    FunctionType * t = isFunctionType(type);
    
    if(t->param.size() > 0)
    {
        args = new WireHolderNode(0);
    }
    
    WireHolderNode * wptr = args,  * lptr;
    
    for(int i=0;i<t->param.size();i++)
    {
        Type * tt = t->param[i];
        Variable * v=0;
        if(isStructType(tt))
        {
            v = new StructVariable();
        }
        else if(isIntType(tt))
        {
            v = new IntVariable();
        }
        else if(isUIntType(tt))
        {
            v = new IntVariable();
        }
        else if(isArrayType(tt))
        {
            v = new ArrayVariable();
        }
        
        v->type = tt;
        v->FillInType(usePool);
        
        argsv.push_back(v);
        
        wptr->coresponding_var = v;
        //v->wv->coresponding_var = v;
        
        if(v->wv != 0)
        {
            wptr->node = v->wv;
        }
        
        wptr->next = new WireHolderNode(0);
        lptr = wptr;
        wptr = wptr->next;
        
    }

    
    if(wptr != 0)
    {
        delete lptr->next;
        lptr->next = 0;
    }
    
    if(t->type != 0 && t->type != getVoidType())
    {
        Type * tt = t->type;
        Variable * v=0;
        if(isStructType(tt))
        {
            v = new StructVariable();
        }
        else if(isIntType(tt))
        {
            v = new IntVariable();
        }
        else if(isUIntType(tt))
        {
            v = new IntVariable();
        }
        else if(isArrayType(tt))
        {
            v = new ArrayVariable();
        }
        v->type = tt;
        v->FillInType(usePool);
        
        returnv = v;
        
        return_var = new WireHolderNode(v);
        
        if(v->wv != 0)
        {
            //cout <<"addr: "<< v->wv<<"\n";
            return_var->node = v->wv;
        }
        
        return_var->coresponding_var = v;
    }
}

long ArrayVariable::assignPermWires(long l)
{
    wirebase = l;
    for(int i=0;i<av.size();i++)
    {
        l = av[i]->assignPermWires(l);
        //cout << "a size: "<<l<<" "<<av[i]<<"\n";
    }
    return l;
}

long IntVariable::assignPermWires(long l)
{
    wirebase = l;
    for(int i=0;i<wires.size();i++)
    {
        wires[i]->wireNumber = l++;
        
        //cout << "int size: "<<l<<" "<<"\n";
    }
    return l;
}

long StructVariable::assignPermWires(long l)
{
    wirebase = l;
    StructType * st = isStructType(type);
    for (std::unordered_map<string,StructItem *>::iterator it = st->si.begin(); it!= st->si.end(); ++it )
    {
        StructItem * sItem =  (StructItem *)(it->second);
        l = map[sItem->name]->assignPermWires(l);
    }
    return l;
}

long FunctionVariable::assignPermWires(long l)
{
    
    wirebase = l;
 
    for(int i=0;i<argsv.size();i++)
    {
        l = argsv[i]->assignPermWires(l);
    }
	if (returnv != 0)
	{
		l = returnv->assignPermWires(l);
	}
    return l;
}





long ArrayVariable::FillInWires(WireSet * ws, long l, int idxF)
{
    //cout << "fit1s\n";
    
    if(ws == 0)
    {
	    ws = getPool(idxF)->getWires(size());
    }
    wireset = ws;

    for(int i=0;i<av.size();i++)
    {
	    l = av[i]->FillInWires(ws, l, idxF);
    }
    
    //cout << "fit1e\n";
    
    return l;
}

long IntVariable::FillInWires(WireSet * ws, long l, int idxF)
{
    
    if(ws == 0)
    {
	    ws = getPool(idxF)->getWires(size());
    }
    wireset = ws;

    //cout << "fit2s\n";
    for(int i=0;i<wires.size();i++)
    {
        wires[i] = ws->wires[l++];
    }
    //cout << "fit2e\n";
    return l;
}

long StructVariable::FillInWires(WireSet * ws, long l, int idxF)
{
    if(ws == 0)
    {
	    ws = getPool(idxF)->getWires(size());
    }
    wireset = ws;
    
    
    StructType * st = isStructType(type);
    for (std::unordered_map<string,StructItem *>::iterator it = st->si.begin(); it!= st->si.end(); ++it )
    {
        StructItem * sItem =  (StructItem *)(it->second);
	    l = map[sItem->name]->FillInWires(ws, l, idxF);
    }
    return l;
}

long FunctionVariable::FillInWires(WireSet * ws, long l, int idxF)
{
    
    
    if(ws == 0)
    {
	    ws = getPool(idxF)->getWires(size());
    }
    wireset = ws;
    
    cout << "calling FunctionVariable::FillInWires will break, i.e. functionvariable size is not configured right. Good luck! (I suggest using the size of return + size of all args\n";
    
    if(returnv != 0)
    {
	    l = returnv->FillInWires(ws, l, idxF);
    }
    for(int i=0;i<argsv.size();i++)
    {
	    l = argsv[i]->FillInWires(ws, l, idxF);
    }
    return l;
}











