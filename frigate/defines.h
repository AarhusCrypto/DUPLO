#include "ast.h"
#include "traverse.h"
#include <string.h>

#include <unordered_map>

#ifndef _defines_h
#define _defines_h


class defines_context
{
public:
    defines_context()
    {
    }
};

std::unordered_map<string,Node *> & getDefineMap();

class DefinesTraverse : public TopDownTraverse <defines_context>
{
public:
    DefinesTraverse()
    {
        
    }
    
    
    defines_context visit(Node * n, defines_context dc)
    {
        TermNode * tn = isTermNode(n);
        
        if(tn != 0)
        {
            
            //don't replace define left sides
            if(isDefineNode(tn->parent)&&tn->parent->getChildNumber(tn)==0)
            {
                return dc;
            }
            
            //don't replace numbers
            if(tn->isNum)
            {
                return dc;
            }


            std::unordered_map<string,Node*>::const_iterator found = getDefineMap().find(tn->var);
            
            if(isIntTypedefNode(tn->parent) && found != getDefineMap().end())
            {
                
                IntTypedefNode * tdn = isIntTypedefNode(tn->parent);
                if(tdn->name == tn)
                {
                    addError(tdn->nodeLocation->fname,tdn->nodeLocation->position,"Replacing typedef name with define: \""+tn->var+"\" at line: "+to_string(tdn->name->nodeLocation->linenobegin));
                }
            }
            
            if(isStructTypedefNode(tn->parent) && found != getDefineMap().end())
            {
                
                StructTypedefNode * tdn = isStructTypedefNode(tn->parent);
                if(tdn->name == tn)
                {
                    addError(tdn->nodeLocation->fname,tdn->nodeLocation->position,"Replacing typedef name with define: \""+tn->var+"\" at line: "+to_string(tdn->name->nodeLocation->linenobegin));
                }
            }
            
            if(isOutputNode(tn->parent) && found != getDefineMap().end())
            {
                
                OutputNode * tdn = isOutputNode(tn->parent);
                if(tdn->type == tn)
                {
                    addError(tdn->nodeLocation->fname,tdn->nodeLocation->position,"Replacing type name with define: \""+tn->var+"\" at line: "+to_string(tdn->type->nodeLocation->linenobegin));
                }
            }
            
            if(isInputNode(tn->parent) && found != getDefineMap().end())
            {
                
                InputNode * tdn = isInputNode(tn->parent);
                if(tdn->type == tn)
                {
                    addError(tdn->nodeLocation->fname,tdn->nodeLocation->position,"Replacing type name with define: \""+tn->var+"\" at line: "+to_string(tdn->type->nodeLocation->linenobegin));
                }
            }
            
            if(found != getDefineMap().end())
            {
                //std::cout << "replacing by: " << tn->var<<"\n";
                
                Node * nn = found->second->deepCopy();
                
                
                
                int tbr = tn->parent->getChildNumber(tn);
                
                
                tn->nodeLocation->position = tn->nodeLocation->position + " (by define \""+tn->var+"\")";
                nn->passDownNewLocation(tn->nodeLocation);
                
                Node * tnparent = tn->parent;
                tnparent->replaceChild(tbr, nn,false);
                
                if(isDefineNode(tnparent))
                {
                    DefineNode * ddn = isDefineNode(tnparent);
                    
                    tn = isTermNode(ddn->name);
                    
                    
                    //std::cout << "adding to define pool: "<< tn->var<<"\n";
                    
                    getDefineMap()[tn->var] = ddn->expression;
                }
            }
            
            return dc;
        }
        
        DefineNode * dn = isDefineNode(n);
        
        if(dn != 0)
        {
            
            
            tn = isTermNode(dn->name);
            
            
            //std::cout << "adding to define pool: "<< tn->var<<"\n";
            
            getDefineMap()[tn->var] = dn->expression;
        }
        
        return dc;
    }
};


void expandDefines(Node * headNode);



#endif