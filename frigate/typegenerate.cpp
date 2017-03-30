//
//  typegenerate.h
//  
//
//  Created by Benjamin Mood on 11/19/14.
//
//

#include "ast.h"
#include "types.h"
#include "traverse.h"
#include <string.h>
#include "error.h"
#include "variable.h"
#include "typegenerate.h"

#include <unordered_map>



//function def vars
//for loops
//var defines
//global defines
//function def names

void generateTypes(Node * topnode)
{
    
    bool iserror=false;
    bool partiesfound=false;
    
    ProgramListNode * pln = (ProgramListNode * )topnode;
    
    TermWarningTraverse twt;
    twt.traverse(pln);
    
    long parties=0;
    VariableContext * vc = new VariableContext();

    //build type map
    for(int i=0;i<pln->nodeList.size();i++)
    {
        
        if(isIntTypedefNode(pln->nodeList[i]))
        {
            IntTypedefNode * tdn = isIntTypedefNode(pln->nodeList[i]);
            
            Type * tttype;
            
            
            long size = tdn->sizev->getLongResultNoVar();
            tttype = new IntType(isTermNode(tdn->name)->var,size);
            
            tttype->wheredefined =tdn;
            
            
            if(getTypeMap()[isTermNode(tdn->name)->var] != 0)
            {
                Node * firstdeclocnode = getTypeMap()[isTermNode(tdn->name)->var]->wheredefined;
                
                addError(tdn->nodeLocation->fname,tdn->nodeLocation->position,"Type \""+isTermNode(tdn->name)->var +"\" is redefined on line "+to_string(tdn->name->nodeLocation->linenobegin)+", first defined at: "+locationToString(firstdeclocnode->nodeLocation->fname,firstdeclocnode->nodeLocation->position));
            }
            
            getTypeMap()[isTermNode(tdn->name)->var] = tttype;
        }
        
        if(isUIntTypedefNode(pln->nodeList[i]))
        {
            UIntTypedefNode * tdn = isUIntTypedefNode(pln->nodeList[i]);
            
            Type * tttype;
            
            
            long size = tdn->sizev->getLongResultNoVar();
            tttype = new UIntType(isTermNode(tdn->name)->var,size);
            
            tttype->wheredefined =tdn;
            
            
            if(getTypeMap()[isTermNode(tdn->name)->var] != 0)
            {
                Node * firstdeclocnode = getTypeMap()[isTermNode(tdn->name)->var]->wheredefined;
                
                addError(tdn->nodeLocation->fname,tdn->nodeLocation->position,"Type \""+isTermNode(tdn->name)->var +"\" is redefined on line "+to_string(tdn->name->nodeLocation->linenobegin)+", first defined at: "+locationToString(firstdeclocnode->nodeLocation->fname,firstdeclocnode->nodeLocation->position));
            }
            
            getTypeMap()[isTermNode(tdn->name)->var] = tttype;
        }
        
        if(isStructTypedefNode(pln->nodeList[i]))
        {
            StructTypedefNode * tdn = isStructTypedefNode(pln->nodeList[i]);
            
            Type * tttype;
            
            tttype = new StructType(isTermNode(tdn->name)->var);
            tttype->wheredefined =tdn;
            
            if(getTypeMap()[isTermNode(tdn->name)->var] != 0)
            {
                Node * firstdeclocnode = getTypeMap()[isTermNode(tdn->name)->var]->wheredefined;
                
                
                addError(tdn->nodeLocation->fname,tdn->nodeLocation->position,"Type \""+isTermNode(tdn->name)->var +"\" is redefined on line "+to_string(tdn->name->nodeLocation->linenobegin)+", first defined at: "+locationToString(firstdeclocnode->nodeLocation->fname,firstdeclocnode->nodeLocation->position));
            }
            
            getTypeMap()[isTermNode(tdn->name)->var] = tttype;
        }
        
        if(isPartiesNode(pln->nodeList[i]))
        {
            parties = stol(isTermNode(isPartiesNode(pln->nodeList[i])->number)->num);
            partiesfound=true;
        }
        
        
    }
    
    //io check
    for(int i=0;i<pln->nodeList.size();i++)
    {
        //cout << pln->nodeList[i]->nodeName<<" "<<isOutputNode(pln->nodeList[i]) << "\n";
        
        if(isInputNode(pln->nodeList[i]) || isOutputNode(pln->nodeList[i]))
        {
            
            pln->nodeList[i]->typeCheck(vc,&getTypeMap());
        }
    }
    
    if(!partiesfound)
    {
        addError("","","You must define the number of parties with a \"#parties x\" statement somewhere in the program.");
    }
    
    if(hasError())
    {
        printErrors(std::cout);
        std::cout <<"Errors, exiting... \n";
        exit(1);
    }
    
    
    bool found_main=false;
    
    
    
    //fill in struct members and function calls and input output types
    for(int i=0;i<pln->nodeList.size();i++)
    {
        if(isStructTypedefNode(pln->nodeList[i]))
        {
            StructTypedefNode * tdn = isStructTypedefNode(pln->nodeList[i]);
            
            Type * tttype;
            
            
            tttype = getTypeMap()[isTermNode(tdn->name)->var];
            
            VarDeclarationListNode * vdln = isVarDeclarationListNode(tdn->structt);
            for(int j=0;j<vdln->nodeList.size();j++)
            {
                VarDeclarationNode * vdn  = isVarDeclarationNode(vdln->nodeList[j]);
                
                Type * selectedType = getTypeMap()[isTermNode(vdn->type)->var];
                
                if(selectedType == 0)
                {
                    addError(vdn->nodeLocation->fname,vdn->nodeLocation->position,"Type \""+isTermNode(vdn->type)->var +"\" is used on line "+to_string(vdn->type->nodeLocation->linenobegin) +" but not defined.");

                    continue;
                }
                
               StructType * st = isStructType(tttype);
                
                VarDeclarationCommaNode * typeList = (VarDeclarationCommaNode *) vdn->each_var;
                
                for(int k=0;k<typeList->nodeList.size();k++)
                {
                    DeclarationNode * dn = isDeclarationNode(typeList->nodeList[k]);
                    
                    string name = isTermNode(dn->varname)->var;
                    
                    vector<long> arraySizes;
                    
                    if(dn->arrays != 0)
                    {
                        string suffix="";
                        
                        ArrayDeclarationListNode * indexList = isArrayDeclarationListNode(dn->arrays);
                        for(int L=0;L<indexList->nodeList.size();L++)
                        {
                            arraySizes.push_back(indexList->nodeList[L]->getLongResultNoVar());
                            suffix +="["+to_string(arraySizes[L]);
                        }
                        
                        //create new type
                        Type * newt = getTypeMap()[isTermNode(vdn->type)->var+suffix];
                        
                        if(newt == 0)
                        {
                            newt = new ArrayType(selectedType->typeName);
                            for(int L=0;L<arraySizes.size();L++)
                            {
                                ((ArrayType *) (newt))->sizes.push_back(arraySizes[L]);
                            }
                            ((ArrayType *) (newt))->type = getTypeMap()[isTermNode(vdn->type)->var];
                        }
                        
                        selectedType = newt;
                    }
                    
                    StructItem * si = new StructItem();
                    si->name = name;
                    //si->arraySizes = arraySizes;
                    si->type = selectedType;
    
                    st->addType(si,dn->nodeLocation->position,dn->nodeLocation->fname);
                }
            }
        }
        
        if(isFunctionDeclarationNode(pln->nodeList[i]))
        {
            
            Type * tttype;

            FunctionDeclarationNode * fdn = isFunctionDeclarationNode(pln->nodeList[i]);
            
            FunctionReturnTypeNode * frtn = isFunctionReturnTypeNode(fdn->returntype);

            if(isTermNode(frtn->type)->var != "void")
            {
                string suffix="";
                vector<long> sizes;
                if(frtn->arrays != 0)
                {
                ArrayDeclarationListNode * adln = isArrayDeclarationListNode(frtn->arrays);
                
                
                sizes.resize(adln->nodeList.size());
                
                for(int j=0;j<adln->nodeList.size();j++)
                {
                    sizes[j] = adln->nodeList[j]->getLongResultNoVar();
                    suffix +="["+to_string(sizes[j]);
                }
                }
                
                
                Type * selectedType = getTypeMap()[isTermNode(frtn->type)->var];
                if(selectedType == 0)
                {
                    addError(frtn->nodeLocation->fname,frtn->nodeLocation->position,"Type \""+isTermNode(frtn->type)->var +"\" is not defined.");
                    
                    continue;
                }
                
                selectedType = getTypeMap()[isTermNode(frtn->type)->var+suffix];
                if(selectedType == 0)
                {
                    selectedType = new ArrayType(isTermNode(frtn->type)->var+suffix);
                    ((ArrayType *)selectedType)->type = getTypeMap()[isTermNode(frtn->type)->var];
                    ((ArrayType *)selectedType)->sizes = sizes;
                }
                
                
                tttype = new FunctionType(isTermNode(fdn->name)->var,selectedType);
                tttype->wheredefined =fdn;
            }
            else
            {
                if(frtn->arrays != 0)
                {
                      addError(frtn->nodeLocation->fname,frtn->nodeLocation->position,"Void type cannot have arrays declared.");
                }
                
                tttype = new FunctionType(isTermNode(fdn->name)->var,getVoidType());
                tttype->wheredefined =fdn;
            }
            if(fdn->param != 0)
            {
                VariableContext * vct = new VariableContext();
                
                DeclarationArgCommaListNode * dacl = isDeclarationArgCommaListNode(fdn->param);
                for(int k=0;k<dacl->nodeList.size();k++)
                {
                    FunctionVarDeclarationNode * var = isFunctionVarDeclarationNode(dacl->nodeList[k]);
                    ((FunctionType *)tttype)->param.push_back(var->typeCheck(vct,&getTypeMap()));
                }
                
                delete vct;

            }
            
        
            if(getTypeMap()[isTermNode(fdn->name)->var] != 0)
            {
                Node * firstdeclocnode = getTypeMap()[isTermNode(fdn->name)->var]->wheredefined;
                
                addError(fdn->nodeLocation->fname,fdn->nodeLocation->position,"Function \""+isTermNode(fdn->name)->var +"\" is redefined on line "+to_string(fdn->name->nodeLocation->linenobegin)+", first defined at: "+locationToString(firstdeclocnode->nodeLocation->fname,firstdeclocnode->nodeLocation->position));
            }
        
            getTypeMap()[isTermNode(fdn->name)->var] = tttype;
            
            if(isTermNode(fdn->name)->var == "main")
            {
                found_main = true;
                
                if(isTermNode(frtn->type)->var  != "void")
                {
                    addError(fdn->nodeLocation->fname,fdn->nodeLocation->position,"Function \""+isTermNode(fdn->name)->var +" must have void return type. (returning output is implicit)");
                }
                
                if(fdn->param != 0)
                {
                   addError(fdn->nodeLocation->fname,fdn->nodeLocation->position,"Function \""+isTermNode(fdn->name)->var +" must have no arguments. (taking input is implicit)");
                }
            }
        }
        
        if(isInputNode(pln->nodeList[i]))
        {
            Variable * v = new Variable();
            v->name ="input"+isTermNode(isInputNode(pln->nodeList[i])->party)->num;
            v->type = getTypeMap()[isTermNode(isInputNode(pln->nodeList[i])->type)->var];
            
            (*vc)["input"+(isTermNode(isInputNode(pln->nodeList[i])->party)->num)]=v;
        }
        
        if(isOutputNode(pln->nodeList[i]))
        {
            Variable * v = new Variable();
            v->name ="output"+isTermNode(isOutputNode(pln->nodeList[i])->party)->num;
            v->type = getTypeMap()[isTermNode(isOutputNode(pln->nodeList[i])->type)->var];
            
            (*vc)["output"+(isTermNode(isOutputNode(pln->nodeList[i])->party)->num)]=v;
        }
    
        
    }
    
    

    
    TypeMap * tm = new TypeMap();
    
    for ( std::unordered_map<string,Type *>::iterator it = getTypeMap().begin(); it!= getTypeMap().end(); ++it )
    {
         (*tm)[it->first] = it->second;
    }
    
    /*
     removed global variables, this is dead code
     //add global varaibles
    for(int i=0;i<pln->nodeList.size();i++)
    {
        if(isVarDeclarationNode(pln->nodeList[i]))
        {
            pln->nodeList[i]->typeCheck(vc,tm);
            
        }
    }*/
    
    
    if(!found_main)
    {
        addError("","","No entry point, \"main\", found in program.");
    }
    
    CheckForRecTypes();

    if(hasError())
    {
        printErrors(std::cout);
        std::cout <<"Errors, exiting... \n";
        exit(1);
    }
    

    //topnode->typeCheck(vc,tm);
    
    for(int i=0;i<pln->nodeList.size();i++)
    {
        if(pln->nodeList[i]->getNodeType() == FunctionDeclarationNode_t || pln->nodeList[i]->getNodeType() == InputNode_t || pln->nodeList[i]->getNodeType() == OutputNode_t)
        {
            if(pln->nodeList[i]->getNodeType() == FunctionDeclarationNode_t && isTermNode(isFunctionDeclarationNode(pln->nodeList[i])->name)->var != "main")
            {
                VariableContext * vct = new VariableContext();
                pln->nodeList[i]->typeCheck(vct, tm);
                delete vct;
            }
            else
            {
                pln->nodeList[i]->typeCheck(vc, tm);
            }
            
            
        }
    }
    
    if(hasError())
    {
        printErrors(std::cout);
        std::cout <<"Errors, exiting... \n";
        exit(1);
    }

}
