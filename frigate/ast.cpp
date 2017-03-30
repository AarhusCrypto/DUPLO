 

#include "ast.h"
#include "types.h"
#include <string>
#include "variable.h"
#include "circuitoutput.h"

//if is node

//nodeName


#define RDTSC ({unsigned long long res;  unsigned hi, lo;   __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi)); res =  ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );res;})
unsigned long startTime, endTime;
unsigned long startTimeb, endTimeb;


using namespace std;
bool isFunctionCall;

Type * binaryTypeCheckNoStructNoArray(Node *left,Node *right,VariableContext * vc, TypeMap * tm, string op)
{
    Type * leftt = left->typeCheck(vc,tm);
    Type * rightt = right->typeCheck(vc,tm);
    
    int res = compareTypes(leftt,rightt);
    

    
    if(isStructType(leftt) || isArrayType(leftt))
    {
        std::stringstream streaml;
        leftt->print(streaml,0,1);
        
        std::stringstream streamr;
        left->print(streamr,0);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"\""+streamr.str()+"\" of type \""+streaml.str()+"\" cannot be used with operator \""+op+"\".fhti");
    }
    if(isStructType(rightt) || isArrayType(rightt))
    {
        std::stringstream streaml;
        rightt->print(streaml,0,1);
        
        std::stringstream streamr;
        right->print(streamr,0);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"\""+streamr.str()+"\" of type \""+streaml.str()+"\" cannot be used for operator \""+op+"\".");
    }


    
    if(res == 1)
    {
        
    }
    else if( res == 2)
    {
        std::stringstream streaml,streamr;
        
        left->print(streaml,0);
        right->print(streamr,0);
        
        std::stringstream streamlt,streamrt;
        
        leftt->print(streamlt,0,1);
        rightt->print(streamrt,0,1);
        
        addWarning(left->nodeLocation->fname,left->nodeLocation->position,"Left side \""+streaml.str()+"\" and right side \""+ streamr.str()+"\" are not the same type (\""+streamlt.str()+"\" vs  \""+ streamrt.str()+"\") in \""+op+"\". Casting...");
    }
    else
    {
        std::stringstream streaml,streamr;
        
        left->print(streaml,0);
        right->print(streamr,0);
        
        std::stringstream streamlt,streamrt;
        
        leftt->print(streamlt,0,1);
        rightt->print(streamrt,0,1);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"Left side \""+streaml.str()+"\" type: \""+streamlt.str()+"\" and right side \""+streamr.str()+"\" type: \""+streamrt.str()+"\" are not the same type in \""+op+"\".");
    }
    
    
    
    if(op == "<" || op == ">" || op == "<=" || op == ">=")
    {
        
        if(getTypeMap()["-+generated_cond_1_int_type+-"] == 0)
        {
            getTypeMap()["-+generated_cond_1_int_type+-"] = new UIntType("-+generated_cond_1_int_type+-",1);
        }
        
        return getTypeMap()["-+generated_cond_1_int_type+-"];
    }
    
    if(isConstType(leftt))
        return rightt;
    
    
    if(isIntType(leftt) && isUIntType(rightt))
    {
        return rightt;
    }
    if(isUIntType(leftt) && isIntType(rightt))
    {
        return leftt;
    }
    
    return leftt;
}


Type * binaryTypeCheckNoStructNoArrayExtend(Node *left,Node *right,VariableContext * vc, TypeMap * tm, string op, Node * parent)
{
    Type * leftt = left->typeCheck(vc,tm);
    Type * rightt = right->typeCheck(vc,tm);
    
    int res = compareTypes(leftt,rightt);
    
    
    if(isIntType(leftt) && isIntType(rightt))
    {
        parent->isSigned = true;
    }
    
    
    if(isStructType(leftt) || isArrayType(leftt))
    {
        std::stringstream streaml;
        leftt->print(streaml,0,1);
        
        std::stringstream streamr;
        left->print(streamr,0);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"\""+streamr.str()+"\" of type \""+streaml.str()+"\" cannot be used with operator \""+op+"\".");
    }
    if(isStructType(rightt) || isArrayType(rightt))
    {
        std::stringstream streaml;
        rightt->print(streaml,0,1);
        
        std::stringstream streamr;
        right->print(streamr,0);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"\""+streamr.str()+"\" of type \""+streaml.str()+"\" cannot be used for operator \""+op+"\".");
    }
    
    
    
    if(res == 1)
    {
        
    }
    else if( res == 2)
    {
        std::stringstream streaml,streamr;
        
        left->print(streaml,0);
        right->print(streamr,0);
        
        std::stringstream streamlt,streamrt;
        
        leftt->print(streamlt,0,1);
        rightt->print(streamrt,0,1);
        
        addWarning(left->nodeLocation->fname,left->nodeLocation->position,"Left side \""+streaml.str()+"\" and right side \""+ streamr.str()+"\" are not the same type (\""+streamlt.str()+"\" vs  \""+ streamrt.str()+"\") in \""+op+"\". Casting...");
    }
    else
    {
        std::stringstream streaml,streamr;
        
        left->print(streaml,0);
        right->print(streamr,0);
        
        std::stringstream streamlt,streamrt;
        
        leftt->print(streamlt,0,1);
        rightt->print(streamrt,0,1);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"Left side \""+streaml.str()+"\" type: \""+streamlt.str()+"\" and right side \""+streamr.str()+"\" type: \""+streamrt.str()+"\" are not the same type in \""+op+"\".");
    }
    
    if(isConstType(leftt) || isConstType(rightt))
    {
        addError(left->nodeLocation->fname,left->nodeLocation->position,"Extending \""+op+"\" cannot be used when either type is a constant (i.e. define the bit length first using # or ##)");
        return getNoType();
    }
    

    Type * typebase = 0;
    
    if(isConstType(leftt))
        typebase = rightt;
    
    
    if(isIntType(leftt) && isUIntType(rightt))
    {
        typebase = rightt;
    }
    if(isUIntType(leftt) && isIntType(rightt))
    {
        typebase = leftt;
    }
    
    if(typebase == 0)
        typebase = leftt;
    
    int size = 0;
    
    
    if(typebase == getNoType())
    {
        return typebase;
    }
    
    parent->operandTypeOfNode = typebase;
    
    if(isIntType(typebase))
    {
        parent->isSigned = true;
        size = isIntType(typebase)->size;
        size*=2;
        
        if(size == 0) // for safety of error checker (i.e. don't crash) purposes (if this occurs it will have been an error)
        {
            size = 2;
        }
        
        parent->nodeSpecVariable = size;
        
        if(getTypeMap()["-+generated_E_"+to_string(size)+"_int_type+-"] == 0)
        {
            getTypeMap()["-+generated_E_"+to_string(size)+"_int_type+-"] = new IntType("-+generated_E_"+to_string(size)+"_int_type+-",size);
        }
        
        return getTypeMap()["-+generated_E_"+to_string(size)+"_int_type+-"];
    }
    else /*if(isUIntType(typebase))*/ //default is unsigned.
    {
        
        /*cout << "type:";
        typebase->print(cout,0);
        cout << "\n";*/
        
        parent->isSigned = false;
        size = isUIntType(typebase)->size;
        size*=2;
        
        if(size == 0) // for safety of error checker (i.e. don't crash) purposes (if this occurs it will have been an error)
        {
            size = 2;
        }
        
        parent->nodeSpecVariable = size;
        
        if(getTypeMap()["-+generated_E_"+to_string(size)+"_uint_type+-"] == 0)
        {
            getTypeMap()["-+generated_E_"+to_string(size)+"_uint_type+-"] = new UIntType("-+generated_E_"+to_string(size)+"_uint_type+-",size);
        }
        
        return getTypeMap()["-+generated_E_"+to_string(size)+"_uint_type+-"];
    }

}

Type * binaryTypeCheckNoStructNoArrayReduce(Node *left,Node *right,VariableContext * vc, TypeMap * tm, string op, Node * parent)
{
    Type * leftt = left->typeCheck(vc,tm);
    Type * rightt = right->typeCheck(vc,tm);
    
    //int res = compareTypes(leftt,rightt);
    
    
    //cout << "pass1\n";
    
    
    if(isIntType(leftt) && isIntType(rightt))
    {
        parent->isSigned = true;
    }
    
    
    if(isStructType(leftt) || isArrayType(leftt))
    {
        std::stringstream streaml;
        leftt->print(streaml,0,1);
        
        std::stringstream streamr;
        left->print(streamr,0);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"\""+streamr.str()+"\" of type \""+streaml.str()+"\" cannot be used with operator \""+op+"\".");
        return getNoType();
    }
    if(isStructType(rightt) || isArrayType(rightt))
    {
        std::stringstream streaml;
        rightt->print(streaml,0,1);
        
        std::stringstream streamr;
        right->print(streamr,0);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"\""+streamr.str()+"\" of type \""+streaml.str()+"\" cannot be used for operator \""+op+"\".");
        
        return getNoType();
    }
    
    //cout << "pass2\n";
    
    int res = 0;
    
    int lefts =0;
    int rights = 0;
    
    if(isIntType(leftt))
    {
        lefts = isIntType(leftt)->size;
    }
    else if(isUIntType(leftt))
    {
        lefts = isUIntType(leftt)->size;
    }
    
    //cout << "pass3\n";
 
    if(isIntType(rightt))
    {
        rights = isIntType(rightt)->size;
    }
    else if(isUIntType(rightt))
    {
        rights = isUIntType(rightt)->size;
    }
    
    parent->operandTypeOfNode = leftt;
    parent->operand2TypeOfNode = rightt;
    
    res = 0;
    if((lefts+1)/2 == rights)
    {
        res = 1;
    }
    
    //cout << "pass4\n";
    
    if(res == 1)
    {
        
    }
    else if( res == 2)
    {
        std::stringstream streaml,streamr;
        
        left->print(streaml,0);
        right->print(streamr,0);
        
        std::stringstream streamlt,streamrt;
        
        leftt->print(streamlt,0,1);
        rightt->print(streamrt,0,1);
        
        addWarning(left->nodeLocation->fname,left->nodeLocation->position,"Left side \""+streaml.str()+"\" and right side \""+ streamr.str()+"\" are not the same type (\""+streamlt.str()+"\" vs  \""+ streamrt.str()+"\") in \""+op+"\". Casting...");
    }
    else
    {
        std::stringstream streaml,streamr;
        
        left->print(streaml,0);
        right->print(streamr,0);
        
        std::stringstream streamlt,streamrt;
        
        leftt->print(streamlt,0,1);
        rightt->print(streamrt,0,1);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"Left side \""+streaml.str()+"\" type: \""+streamlt.str()+"\" and right side \""+streamr.str()+"\" type: \""+streamrt.str()+"\" must complete the formula: (leftsize+1)/2 == rightsize) when used in operation \""+op+"\".");
    }
    
    if(isConstType(leftt) || isConstType(rightt))
    {
        addError(left->nodeLocation->fname,left->nodeLocation->position,"Extending \""+op+"\" cannot be used when either type is a constant (i.e. define the bit length first using # or ##)");
        return getNoType();
    }
    
    //cout << "pass5\n";
    
    Type * typebase = 0;
    
    if(isConstType(leftt))
        typebase = rightt;
    
    
    if(isIntType(leftt) && isUIntType(rightt))
    {
        typebase = rightt;
    }
    if(isUIntType(leftt) && isIntType(rightt))
    {
        typebase = leftt;
    }
    
    if(typebase == 0)
        typebase = leftt;
    
    int size = 0;
    size = (lefts+1)/2; //size of result
    
    //cout << "size: "<<size<<"\n";
    
    if(typebase == getNoType())
    {
        return typebase;
    }
    
    if(isIntType(typebase))
    {
        parent->isSigned = true;
        /*size = isIntType(typebase)->size;
        
        if(size%2 == 1)
        {
            size++;
        }
        size = size /2;
        
        if(size == 0) // for safety of error checker (i.e. don't crash) purposes (if this occurs it will have been an error)
        {
            size = 2;
        }*/
        
        parent->nodeSpecVariable = size;
        
        if(getTypeMap()["-+generated_E_"+to_string(size)+"_int_type+-"] == 0)
        {
            getTypeMap()["-+generated_E_"+to_string(size)+"_int_type+-"] = new IntType("-+generated_E_"+to_string(size)+"_int_type+-",size);
        }
        
        return getTypeMap()["-+generated_E_"+to_string(size)+"_int_type+-"];
    }
    else /*if(isUIntType(typebase))*/ //default is unsigned.
    {
        parent->isSigned = false;
        /*size = isUIntType(typebase)->size;
        
        if(size%2 == 1)
        {
            size++;
        }
        size = size /2;
        
        if(size == 0) // for safety of error checker (i.e. don't crash) purposes (if this occurs it will have been an error)
        {
            size = 2;
        }*/
        
        parent->nodeSpecVariable = size;
        
        if(getTypeMap()["-+generated_E_"+to_string(size)+"_uint_type+-"] == 0)
        {
            getTypeMap()["-+generated_E_"+to_string(size)+"_uint_type+-"] = new UIntType("-+generated_E_"+to_string(size)+"_uint_type+-",size);
        }
        
        return getTypeMap()["-+generated_E_"+to_string(size)+"_uint_type+-"];
    }
    
}






Type * binaryTypeCheckNoStructNoArray(Node *left,Node *right,VariableContext * vc, TypeMap * tm, string op, Node * parent)
{
    Type * leftt = left->typeCheck(vc,tm);
    Type * rightt = right->typeCheck(vc,tm);
    
    int res = compareTypes(leftt,rightt);
    
    
    
    if(isStructType(leftt) || isArrayType(leftt))
    {
        std::stringstream streaml;
        leftt->print(streaml,0,1);
        
        std::stringstream streamr;
        left->print(streamr,0);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"\""+streamr.str()+"\" of type \""+streaml.str()+"\" cannot be used with operator \""+op+"\".");
    }
    if(isStructType(rightt) || isArrayType(rightt))
    {
        std::stringstream streaml;
        rightt->print(streaml,0,1);
        
        std::stringstream streamr;
        right->print(streamr,0);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"\""+streamr.str()+"\" of type \""+streaml.str()+"\" cannot be used for operator \""+op+"\".");
    }
    
    
    
    if(res == 1)
    {
        
    }
    else if( res == 2)
    {
        std::stringstream streaml,streamr;
        
        left->print(streaml,0);
        right->print(streamr,0);
        
        std::stringstream streamlt,streamrt;
        
        leftt->print(streamlt,0,1);
        rightt->print(streamrt,0,1);
        
        addWarning(left->nodeLocation->fname,left->nodeLocation->position,"Left side \""+streaml.str()+"\" and right side \""+ streamr.str()+"\" are not the same type (\""+streamlt.str()+"\" vs  \""+ streamrt.str()+"\") in \""+op+"\". Casting...");
    }
    else
    {
        std::stringstream streaml,streamr;
        
        left->print(streaml,0);
        right->print(streamr,0);
        
        std::stringstream streamlt,streamrt;
        
        leftt->print(streamlt,0,1);
        rightt->print(streamrt,0,1);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"Left side \""+streaml.str()+"\" type: \""+streamlt.str()+"\" and right side \""+streamr.str()+"\" type: \""+streamrt.str()+"\" are not the same type in \""+op+"\".");
    }
    
    
    
    if(op == "<" || op == ">" || op == "<=" || op == ">=")
    {
        
        if(isConstType(leftt))
            parent->operandTypeOfNode = rightt;
        else
            parent->operandTypeOfNode = leftt;
        
        
        if(isIntType(leftt) && isIntType(rightt))
        {
            parent->isSigned = true;
        }
        
        if(isIntType(leftt) && isConstType(rightt))
        {
            parent->isSigned = true;
        }
        
        if(isConstType(leftt) && isIntType(rightt))
        {
            parent->isSigned = true;
        }
        
        if(getTypeMap()["-+generated_cond_1_int_type+-"] == 0)
        {
            getTypeMap()["-+generated_cond_1_int_type+-"] = new UIntType("-+generated_cond_1_int_type+-",1);
        }
        
        return getTypeMap()["-+generated_cond_1_int_type+-"];
    }
    
    if(isConstType(leftt))
        return rightt;
    
    
    if(isIntType(leftt) && isUIntType(rightt))
    {
        return rightt;
    }
    if(isUIntType(leftt) && isIntType(rightt))
    {
        return leftt;
    }
    
    return leftt;
}

Type * binaryTypeCheckStructOKArrayOK(Node *left,Node *right,VariableContext * vc, TypeMap * tm, string op)
{
    Type * leftt = left->typeCheck(vc,tm);
    Type * rightt = right->typeCheck(vc,tm);
    
    int res = compareTypes(leftt,rightt);
    

    
    if(res == 1)
    {
        
    }
    else if( res == 2)
    {
        std::stringstream streaml,streamr;
        
        left->print(streaml,0);
        right->print(streamr,0);
        
        
        std::stringstream streamlt,streamrt;
        
        leftt->print(streamlt,0,1);
        rightt->print(streamrt,0,1);
        
        addWarning(left->nodeLocation->fname,left->nodeLocation->position,"Left side \""+streaml.str()+"\" and right side \""+ streamr.str()+"\" are not the same type (\""+streamlt.str()+"\" vs  \""+ streamrt.str()+"\") in \""+op+"\". Casting...");
    }
    else
    {
        std::stringstream streaml,streamr;
        
        left->print(streaml,0);
        right->print(streamr,0);
        
        std::stringstream streamlt,streamrt;
        
        leftt->print(streamlt,0,1);
        rightt->print(streamrt,0,1);

        addError(left->nodeLocation->fname,left->nodeLocation->position,"Left side \""+streaml.str()+"\" type: \""+streamlt.str()+"\" and right side \""+streamr.str()+"\" type: \""+streamrt.str()+"\" are not the same type in \""+op+"\".");
    }
    if(isConstType(leftt))
        return rightt;
    
    if(isIntType(leftt) && isUIntType(rightt))
    {
        return rightt;
    }
    if(isUIntType(leftt) && isIntType(rightt))
    {
        return leftt;
    }
    
    return leftt;
}




Type * binaryTypeCheckStructOKArrayOKEQ(Node *left,Node *right,VariableContext * vc, TypeMap * tm, string op)
{
    Type * leftt = left->typeCheck(vc,tm);
    Type * rightt = right->typeCheck(vc,tm);
    
    int res = compareTypes(leftt,rightt);
    
    if(isConstType(leftt))
    {
        std::stringstream streaml;
        
        left->print(streaml,0);
        
        std::stringstream streamlt;
        
        leftt->print(streamlt,0,1);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"Left side \""+streaml.str()+"\" type: \""+streamlt.str()+"\" cannot have a value assigned to it.");
    }
    
    if(res == 1)
    {
        
    }
    else if( res == 2)
    {
        std::stringstream streaml,streamr;
        
        left->print(streaml,0);
        right->print(streamr,0);
        
        
        std::stringstream streamlt,streamrt;
        
        leftt->print(streamlt,0,1);
        rightt->print(streamrt,0,1);
        
        addWarning(left->nodeLocation->fname,left->nodeLocation->position,"Left side \""+streaml.str()+"\" and right side \""+ streamr.str()+"\" are not the same type (\""+streamlt.str()+"\" vs  \""+ streamrt.str()+"\") in \""+op+"\". Casting...");
    }
    else
    {
        std::stringstream streaml,streamr;
        
        left->print(streaml,0);
        right->print(streamr,0);
        
        std::stringstream streamlt,streamrt;
        
        leftt->print(streamlt,0,1);
        rightt->print(streamrt,0,1);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"Left side \""+streaml.str()+"\" type: \""+streamlt.str()+"\" and right side \""+streamr.str()+"\" type: \""+streamrt.str()+"\" are not the same type in \""+op+"\".");
    }
    if(isConstType(leftt))
        return rightt;
    
    if(isIntType(leftt) && isUIntType(rightt))
    {
        return rightt;
    }
    if(isUIntType(leftt) && isIntType(rightt))
    {
        return leftt;
    }
    
    return leftt;
}





Type * binaryTypeCheckCondDifferentSizesOK(Node *left,Node *right,VariableContext * vc, TypeMap * tm, string op)
{
    Type * leftt = left->typeCheck(vc,tm);
    Type * rightt = right->typeCheck(vc,tm);
    
    if(isStructType(leftt) || isArrayType(leftt))
    {
        std::stringstream streaml;
        leftt->print(streaml,0,1);
        
        std::stringstream streamr;
        left->print(streamr,0);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"\""+streamr.str()+"\" of type \""+streaml.str()+"\" cannot be used with operator \""+op+"\".");
    }
    if(isStructType(rightt) || isArrayType(rightt))
    {
        std::stringstream streaml;
        rightt->print(streaml,0,1);
        
        std::stringstream streamr;
        right->print(streamr,0);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"\""+streamr.str()+"\" of type \""+streaml.str()+"\" cannot be used for operator \""+op+"\".");
    }
    

    if(getTypeMap()["-+generated_cond_1_int_type+-"] == 0)
    {
        getTypeMap()["-+generated_cond_1_int_type+-"] = new UIntType("-+generated_cond_1_int_type+-",1);
    }
    
    return getTypeMap()["-+generated_cond_1_int_type+-"];
}

Type * binaryTypeCheckCondStructOK(Node *left,Node *right,VariableContext * vc, TypeMap * tm, string op, Node * parent)
{
    Type * leftt = left->typeCheck(vc,tm);
    Type * rightt = right->typeCheck(vc,tm);
    
    int res = compareTypes(leftt,rightt);
    
    
    
    if(res == 1)
    {
        parent->operandTypeOfNode = leftt;
        if(isConstType(leftt))
        {
            parent->operandTypeOfNode = rightt;
        }
    }
    else if( res == 2)
    {
        parent->operandTypeOfNode = leftt;
        if(isConstType(leftt))
        {
            parent->operandTypeOfNode = rightt;
        }
        
        std::stringstream streaml,streamr;
        
        left->print(streaml,0);
        right->print(streamr,0);
        
        
        std::stringstream streamlt,streamrt;
        
        leftt->print(streamlt,0,1);
        rightt->print(streamrt,0,1);
        
        addWarning(left->nodeLocation->fname,left->nodeLocation->position,"Left side \""+streaml.str()+"\" and right side \""+ streamr.str()+"\" are not the same type (\""+streamlt.str()+"\" vs  \""+ streamrt.str()+"\") in \""+op+"\". Casting...");
    }
    else
    {
        std::stringstream streaml,streamr;
        
        left->print(streaml,0);
        right->print(streamr,0);
        
        std::stringstream streamlt,streamrt;
        
        leftt->print(streamlt,0,1);
        rightt->print(streamrt,0,1);
        
        addError(left->nodeLocation->fname,left->nodeLocation->position,"Left side \""+streaml.str()+"\" type: \""+streamlt.str()+"\" and right side \""+streamr.str()+"\" type: \""+streamrt.str()+"\" are not the same type in \""+op+"\".");
    }
    
    if(getTypeMap()["-+generated_cond_1_int_type+-"] == 0)
    {
        getTypeMap()["-+generated_cond_1_int_type+-"] = new UIntType("-+generated_cond_1_int_type+-",1);
    }
    
    return getTypeMap()["-+generated_cond_1_int_type+-"];
}

Type * binaryTypeCheckShift(Node *left,Node *right,VariableContext * vc, TypeMap * tm, string op)
{
    Type * t = right->typeCheck(vc,tm);
    Type * t2 = left->typeCheck(vc,tm);
    
    if(!(isIntType(t) || isConstType(t) || isUIntType(t) ))
    {
        std::stringstream streaml, streamr;
        right->print(streaml,0);
        t->print(streamr,0,1);
        addError(left->nodeLocation->fname,left->nodeLocation->position,"Operator \""+op+"\" cannot be applied to \""+streaml.str()+"\", type must be number, recieved: \""+streamr.str()+"\".");
    }
    
    if(!(isIntType(t2) || isConstType(t2) || isUIntType(t2) ))
    {
        std::stringstream streaml,streamr;
        left->print(streaml,0);
        t2->print(streamr,0,1);
        addError(left->nodeLocation->fname,left->nodeLocation->position,"Operator \""+op+"\" cannot be applied to \""+streaml.str()+"\", type must be number, recieved: \""+streamr.str()+"\".");
    }
    
    return t;
}

Type * unaryTypeCheckNoCheck(Node *operand,VariableContext * vc, TypeMap * tm, string op)
{
    
    return operand->typeCheck(vc,tm);
}

Type * unaryTypeCheckNumberRequiredNoConst(Node *operand,VariableContext * vc, TypeMap * tm, string op)
{
    Type * t = operand->typeCheck(vc,tm);
    
    if(isStructType(t) || isArrayType(t) || isFunctionType(t))
    {
        std::stringstream streaml;
        
        operand->print(streaml,0);
        
        addError(operand->nodeLocation->fname,operand->nodeLocation->position,"Operator \""+op+"\" cannot be applied to  \""+streaml.str()+"\", type base must be int_t.");
        
        return getNoType();
    }
    
    if(isConstType(t))
    {
        std::stringstream streaml;
        
        operand->print(streaml,0);
        
        addError(operand->nodeLocation->fname,operand->nodeLocation->position,"Operator \""+op+"\" cannot be applied to constant \""+streaml.str()+"\", only int_t types.");
        
        return t;
    }
    
    return t;
}

Type * unaryTypeCheckTerm(Node *term,VariableContext * vc, TypeMap *, string op)
{
    if(isTermNode(term)->isNum)
    {
        
        if(isTermNode(term)->bitwidthnode != 0)
        {
            isTermNode(term)->bitwidth = isTermNode(term)->bitwidthnode->getLongResultNoVar();
            
            if(isTermNode(term)->bitwidth == 0)
            {
                addError(term->nodeLocation->fname,term->nodeLocation->position,"Constants with a defined bitlength must have a size > than 0.");
                
            }
            
            /*cout << isTermNode(term)->isSigned;
            cout <<" ";
            term->print(cout,0);
            cout <<" ";
            isTermNode(term)->bitwidthnode->print(cout,0);
            cout <<"\n";*/
            
            if(isTermNode(term)->isSigned)
            {
                if(getTypeMap()["-+generate_const_type+-"+to_string(isTermNode(term)->bitwidth)] == 0)
                {
                    getTypeMap()["-+generate_const_type+-"+to_string(isTermNode(term)->bitwidth)] = new IntType("-+generate_const_type+-"+to_string(isTermNode(term)->bitwidth),isTermNode(term)->bitwidth);
                }
                
                return getTypeMap()["-+generate_const_type+-"+to_string(isTermNode(term)->bitwidth)];
            }
            else
            {
                if(getTypeMap()["-+generate_uconst_type+-"+to_string(isTermNode(term)->bitwidth)] == 0)
                {
                    getTypeMap()["-+generate_uconst_type+-"+to_string(isTermNode(term)->bitwidth)] = new UIntType("-+generate_uconst_type+-"+to_string(isTermNode(term)->bitwidth),isTermNode(term)->bitwidth);
                }
                
                return getTypeMap()["-+generate_uconst_type+-"+to_string(isTermNode(term)->bitwidth)];
            }
        }

        if(term->hasMinus)
        {
            addWarning(term->nodeLocation->fname,term->nodeLocation->position,"Constant, \""+isTermNode(term)->num+"\", does not have a set length using # operator. Using subtract (or negative numbers) may result in incorrect results.");
        }
        
        return getConstType();
    }
    else
    {
        if((*vc)[isTermNode(term)->var]==0)
        {
            addError(term->nodeLocation->fname,term->nodeLocation->position,"Variable \""+isTermNode(term)->var+"\" is used before it is defined.");
        }
        else
        {
            return (*vc)[isTermNode(term)->var]->type;
        }
    }
    
    return getNoType();
}

Type * unaryTypeCheckReturn(Node *returnn,VariableContext * vc, TypeMap * tm, string op)
{
    
    Type * t = (*vc)["-+RETURNTYPE+-"]->type;
    
    
    if(isVoidType(t))
    {
        if(isReturnNode(returnn)->operand == 0)
        {
            return getNoType();
        }
        else
        {
            std::stringstream streaml;
            
            Type * t = ((ReturnNode *)returnn)->operand->typeCheck(vc,tm);
            
            t->print(streaml,0,1);
            
            addError(returnn->nodeLocation->fname,returnn->nodeLocation->position,"Return does not return correct type, \""+ streaml.str()+"\" is not of type \"void\".");
            
            return getNoType();
        }
    }
    
    if(isReturnNode(returnn)->operand == 0)
    {
        std::stringstream streaml;
        t->print(streaml,0,1);
        
        addError(returnn->nodeLocation->fname,returnn->nodeLocation->position,"Return does not return correct type, \"void\" is not of type \""+streaml.str()+"\".");
        
        return getNoType();
    }
    
    Type * t2 = ((ReturnNode *)returnn)->operand->typeCheck(vc,tm);
    
    int res = compareTypes(t,t2);
    
    if(res == 0)
    {
        std::stringstream streaml,streamr;
        
        t->print(streaml,0,1);
        t2->print(streamr,0,1);
        
        addError(returnn->nodeLocation->fname,returnn->nodeLocation->position,"Return does not return correct type, \""+ streamr.str()+"\" is not compatible with expected type \""+streaml.str()+"\".");
    }
    else if (res == 2)
    {
        std::stringstream streaml,streamr;
  
        if(isArrayType(t) || isStructType(t))
        {
            t->print(streaml,0,1);
        }
        else
        {
            t->print(streaml,0);
        }
        
        if(isArrayType(t2) || isStructType(t2))
        {
            t2->print(streamr,0,1);
        }
        else
        {
            t2->print(streamr,0);
        }
        
        addWarning(returnn->nodeLocation->fname,returnn->nodeLocation->position,"Return does not return correct type, \""+ streamr.str()+"\", instead of expected type \""+streaml.str()+"\". Casting...");
    }
    
    
    return getNoType();
}

Type * unaryTypeCheckFunctionCall(Node *func,VariableContext * vc, TypeMap * tm, string op)
{
    FunctionCallNode * fcn = isFunctionCallNode(func);
    
    
    if((*tm)[isTermNode(fcn->function_name)->var] == 0)
    {
        addError(func->nodeLocation->fname,func->nodeLocation->position,"Function \""+isTermNode(fcn->function_name)->var+"\" is called but not defined.");
        return getNoType();
    }
    
    FunctionType * functiontype = isFunctionType((*tm)[isTermNode(fcn->function_name)->var]);
    

    
    vector<Type *> functiondecparam = functiontype->param;
    vector<Type *> functioncallparam;
    
    if(fcn->args != 0)
    {
        FunctionArgListNode * faln = isFunctionArgListNode(fcn->args);
        for(int i=0;i<faln->nodeList.size();i++)
        {
            functioncallparam.push_back(faln->nodeList[i]->typeCheck(vc,tm));
        }
    }
    
    int res = compareTypeVectors(functiondecparam,functioncallparam);
    
    if(res == 0)
    {
        std::stringstream streaml,streamr;
        
        streaml<<"(";
        for(int i=0;i<functiondecparam.size();i++)
        {
            if(i != 0)
            {
                streaml <<", ";
            }
            functiondecparam[i]->print(streaml,0,1);
            
        }
        streaml<<")";
        
        streamr<<"(";
        for(int i=0;i<functioncallparam.size();i++)
        {
            if(i != 0)
            {
                streamr <<", ";
            }
            functioncallparam[i]->print(streamr,0,1);
            
        }
        streamr<<")";
        
        addError(func->nodeLocation->fname,func->nodeLocation->position,"Function, \""+isTermNode(fcn->function_name)->var+"\", call's parameter types do not match, expecting: \""+ streaml.str()+"\"   recieved:  \""+streamr.str()+"\".");
    }
    else if(res == 2)
    {
        std::stringstream streaml,streamr;
        
        streaml<<"(";
        for(int i=0;i<functiondecparam.size();i++)
        {
            if(i != 0)
            {
                streaml <<", ";
            }
            functiondecparam[i]->print(streaml,0,1);
            
        }
        streaml<<")";
        
        streamr<<"(";
        for(int i=0;i<functioncallparam.size();i++)
        {
            if(i != 0)
            {
                streaml <<", ";
            }
            functioncallparam[i]->print(streamr,0,1);
            
        }
        streamr<<")";
        
        addWarning(func->nodeLocation->fname,func->nodeLocation->position,"Function, \""+isTermNode(fcn->function_name)->var+"\", call's parameter types do not match without casting, expecting: \""+ streaml.str()+"\"   recieved:  \""+streamr.str()+"\".");
        
    }

    /*cout << "warningcheck ";
    functiontype->type->print(cout,0);
    cout << "\n";*/
    
    return functiontype->type;
}

Type * unaryTypeCheckDeclarationVar(Node *dec,VariableContext * vc, TypeMap * tm, string op)
{
    VarDeclarationNode * vdn = isVarDeclarationNode(dec);
    
    
    if((*tm)[isTermNode(vdn->type)->var]==0)
    {
        addError(dec->nodeLocation->fname,dec->nodeLocation->position,"Type \""+isTermNode(vdn->type)->var +"\" is used on line "+to_string(vdn->type->nodeLocation->linenobegin) +" but not defined.");
        
        return getNoType();
    }
    
    
    Type * vartype = (*tm)[isTermNode(vdn->type)->var];
    DeclarationCommaListNode * vars = isDeclarationCommaListNode(vdn->each_var);
    
    
    //if(vars != 0)
    {
        for(int i=0;i < vars->nodeList.size();i++)
        {
            VarDeclarationPrimNode * dn = isVarDeclarationPrimNode(vars->nodeList[i]);
            vartype = (*tm)[isTermNode(vdn->type)->var];
            
            
            
            Variable * v = new Variable();
            
            v->name = isTermNode(dn->name)->var;
            
            string suffix="";
            if(dn->array_dec != 0)
            {
                vector<long> sizes;
                
                sizes.resize(isArrayDeclarationListNode(dn->array_dec)->nodeList.size());
                
                for(int j=0;j<isArrayDeclarationListNode(dn->array_dec)->nodeList.size();j++)
                {
                    sizes[j] = isArrayDeclarationListNode(dn->array_dec)->nodeList[j]->getLongResultNoVar();
                    
                    if(sizes[j] == 0)
                    {
                        addError(dec->nodeLocation->fname,dec->nodeLocation->position,"Variable \""+v->name +"\" has an array size declaration of 0. Variables must have array declarations of size greater than 0.");
                    }
                    
                    suffix +="["+to_string(sizes[j]);
                }
                
                Type * newt = (*tm)[isTermNode(vdn->type)->var+suffix];
                if(newt == 0)
                {
                    newt = new ArrayType(vartype->typeName);
                    for(int L=0;L<sizes.size();L++)
                    {
                        ((ArrayType *) (newt))->sizes.push_back(sizes[L]);
                    }
                    ((ArrayType *) (newt))->type = (*tm)[isTermNode(vdn->type)->var];
                }
                vartype = newt;
            }
        
            v->type = vartype;
            v->nodeOfName = dn->name;
            
            if((*vc)[v->name]!=0)
            {
                
                addError(dec->nodeLocation->fname,dec->nodeLocation->position,"Variable \""+v->name +"\" is redefined, defined previously at: "+locationToString((*vc)[v->name]->nodeOfName->nodeLocation->fname,(*vc)[v->name]->nodeOfName->nodeLocation->position) +".");
            }
            
            (*vc)[v->name] = v;
            
            if(dn->init_expression != 0 )
            {
                Type * inittype = dn->init_expression->typeCheck(vc,tm);
                
                //cout << dn->init_expression->nodeName<<"\n";
                
                int res = compareTypeAndVar(v,inittype);
                
                if(res == 0)
                {
                    std::stringstream streaml,streamr;
                    
                    v->type->print(streaml,0,1);
                    inittype->print(streamr,0,1);
                    
                    addError(dec->nodeLocation->fname,dec->nodeLocation->position,"Type mismatch in variable initialization for variable \""+v->name+"\", left side: \""+streaml.str()+"\" and right side: \""+streamr.str()+"\".");
                }
                else if(res == 2)
                {
                    std::stringstream streaml,streamr;
                    
                    v->type->print(streaml,0);
                    
                    if(isArrayType(inittype))
                    {
                        inittype->print(streamr,0,1);
                    }
                    else
                    {
                        inittype->print(streamr,0);
                    }
                    
                    addWarning(dec->nodeLocation->fname,dec->nodeLocation->position,"Type mismatch in variable initialization for variable \""+v->name+"\", left side: \""+streaml.str()+"\" and right side: \""+streamr.str()+"\", casting... .");
                }
            }
        }
    }
    
    
    return vartype;
}
Type * arrayInitTypeCheck(Node *dec,VariableContext * vc, TypeMap *tm, string op)
{
    ArrayInitListNode * ailn = isArrayInitListNode(dec);
    
    Type * lasttype = ailn->nodeList[0]->typeCheck(vc,tm);
    
    //check all types, verify compabible
    for(int i=1;i<ailn->nodeList.size();i++)
    {
        lasttype = compareNumTypes(lasttype,ailn->nodeList[i]->typeCheck(vc,tm));
        
        if(lasttype == 0)
        {
            return getNoType();
        }
    }
    
    //return array type with arrays of size right
    string suffix="";
    if(isArrayType(lasttype))
    {
        for(int i=0;i<isArrayType(lasttype)->sizes.size();i++)
        {
            suffix+="["+to_string(isArrayType(lasttype)->sizes[i]);
        }
    }
    
    suffix="["+to_string(ailn->nodeList.size())+suffix;
    
    string primtypename=lasttype->typeName;
    
    if(isArrayType(lasttype))
    {
        primtypename=isArrayType(lasttype)->type->typeName;
    }
    
    Type * newt = (*tm)[primtypename+"-generated+"+suffix];
    if(newt == 0)
    {
        newt = new ArrayType(primtypename+"-generated+"+suffix);
        
        isArrayType(newt)->sizes.push_back(ailn->nodeList.size());
        
        if(isArrayType(lasttype))
        {
            for(int i=0;i<isArrayType(lasttype)->sizes.size();i++)
            {
                isArrayType(newt)->sizes.push_back( isArrayType(lasttype)->sizes[i]);
            }
        }
     
        //build array of array type
        if(isArrayType(lasttype))
        {
            isArrayType(newt)->type = isArrayType(lasttype)->type;
        }
        else
        {
            isArrayType(newt)->type = lasttype;
        }
    }
    lasttype = newt;
    
    
    return lasttype;
    
    //return getNoType();
}

Type * unaryTypeCheckFunctionDeclarationVar(Node *dec,VariableContext * vc, TypeMap *tm, string op)
{
    
    FunctionVarDeclarationNode * fvdn = isFunctionVarDeclarationNode(dec);
    
    
    if((*tm)[isTermNode(fvdn->type)->var]==0)
    {
        addError(dec->nodeLocation->fname,dec->nodeLocation->position,"Type \""+isTermNode(fvdn->type)->var +"\" is used but not defined.");
        
        return getNoType();
    }
    Type * vartype = (*tm)[isTermNode(fvdn->type)->var];

    Variable * v = new Variable();
    
    v->name = isTermNode(fvdn->varname)->var;
    
    string suffix="";
    if(fvdn->arrays != 0)
    {
        vector<long> sizes;
        
        sizes.resize(isArrayDeclarationListNode(fvdn->arrays)->nodeList.size());
        
        for(int j=0;j<isArrayDeclarationListNode(fvdn->arrays)->nodeList.size();j++)
        {
            sizes[j] = isArrayDeclarationListNode(fvdn->arrays)->nodeList[j]->getLongResultNoVar();
            suffix +="["+to_string(sizes[j]);
        }
        
        Type * newt = (*tm)[isTermNode(fvdn->type)->var+suffix];
        if(newt == 0)
        {
            newt = new ArrayType(vartype->typeName);
            for(int L=0;L<sizes.size();L++)
            {
                ((ArrayType *) (newt))->sizes.push_back(sizes[L]);
            }
        }
        ((ArrayType *) (newt))->type = (*tm)[isTermNode(fvdn->type)->var];
        vartype = newt;
    }
    
    v->type = vartype;
    v->nodeOfName = fvdn->varname;
    (*vc)[v->name] = v;


    return vartype;
}
Type * unaryTypeCheckFunctionDeclaration(Node *dec,VariableContext * vc , TypeMap * tm, string op)
{
    FunctionDeclarationNode * fdn = isFunctionDeclarationNode(dec);
    VariableContext * vct = new VariableContext();
    
    for ( std::unordered_map<string,Variable *>::iterator it = vc->begin(); it!= vc->end(); ++it )
    {
        (*vct)[it->first] = it->second;
    }
    
    
    if(fdn->param != 0)
    {
        DeclarationArgCommaListNode * dacl = isDeclarationArgCommaListNode(fdn->param);
        for(int k=0;k<dacl->nodeList.size();k++)
        {
            FunctionVarDeclarationNode * var = isFunctionVarDeclarationNode(dacl->nodeList[k]);
            
            var->typeCheck(vct,tm);
        }
    }
    
    Variable * v = new Variable();
    v->name = "-+RETURNTYPE+-";
    v->type = isFunctionType((*tm)[isTermNode(fdn->name)->var])->type;
    (*vct)[v->name] = v;
    
    
    if(fdn->stmts != 0)
    {
        fdn->stmts->typeCheck(vct,tm);
        

        CompoundStatementNode * cs = (CompoundStatementNode *)(fdn->stmts);
        
        if(cs->stmt != 0)
        {
        
            StatementListNode * sl = (StatementListNode*) (cs->stmt);
            
            
            if(!isReturnNode(sl->nodeList[sl->nodeList.size()-1]) && v->type != getVoidType())
            {
                addError(fdn->nodeLocation->fname,fdn->nodeLocation->position,"Missing return statement on non-Void function \""+isTermNode(fdn->name)->var+"\".");
            }
        }
    }
    

    delete vct;
    return getNoType();
}
Type * ifTypeCheck(Node *ifnode,VariableContext * vc, TypeMap * tm, string op)
{
    IfNode * ifn = isIfNode(ifnode);
    
    Type * cond = ifn->cond->typeCheck(vc,tm);
    
    if(!(isConstType(cond) || (isUIntType(cond) && isUIntType(cond)->size == 1) || (isIntType(cond) && isIntType(cond)->size == 1)  ))
    {
        std::stringstream streaml,streamr;
        ifn->cond->print(streaml,0);
        
        cond->print(streamr,0,1);
        
        addError(ifnode->nodeLocation->fname,ifnode->nodeLocation->position,"If statement conditional \""+streaml.str()+"\" is of type \""+streamr.str()+"\", expecting a single wire (bool) type.");
    }
    
    ifn->stmts->typeCheck(vc,tm);
    
    if(ifn->elsestmts != 0)
    {
        ifn->elsestmts->typeCheck(vc,tm);
    }
    
    return getNoType();
}

Type * forTypeCheck(Node *fornode,VariableContext * vc, TypeMap * tm, string op)
{
    
    
    ForNode * fn = isForNode(fornode);

    VariableContext * vct = new VariableContext();
    
    for ( std::unordered_map<string,Variable *>::iterator it = vc->begin(); it!= vc->end(); ++it )
    {
        (*vct)[it->first] = it->second;
    }
    
    VarDeclarationForNode * vdfn = isVarDeclarationForNode(fn->declaration);
    
    VarDeclarationNode * temp = new VarDeclarationNode(vdfn->type,vdfn->each_var);
    temp->nodeLocation = AstLocation::Duplicate(vdfn->nodeLocation);
    
    temp->typeCheck(vct,tm);
    
    Type * shouldbebool = fn->condition->typeCheck(vct,tm);
    
    if(!(isConstType(shouldbebool) || (isUIntType(shouldbebool) && isUIntType(shouldbebool)->size == 1)|| (isIntType(shouldbebool) && isIntType(shouldbebool)->size == 1)  )  )
    {
        std::stringstream streaml,streamr;
        fn->condition->print(streaml,0);
        
        shouldbebool->print(streamr,0,1);
        
        addError(fornode->nodeLocation->fname,fornode->nodeLocation->position,"For statement conditional \""+streaml.str()+"\" is of type \""+streamr.str()+"\", expecting a single wire (bool) type.");
    }
    
    fn->increment->typeCheck(vct,tm);
    
    fn->stmts->typeCheck(vct,tm);
    
    
    temp->type = 0;
    temp->each_var = 0;
    delete temp;
    
    delete vct;
    return getNoType();
}

Type * dotTypeCheck(Node *dotnode,VariableContext * vc, TypeMap * tm, string op)
{
    DotOperatorNode * don = isDotOperatorNode(dotnode);
    
    Type * leftsidetype = don->left->typeCheck(vc,tm);
    
    if(!isStructType(leftsidetype))
    {
        std::stringstream streaml;
        leftsidetype->print(streaml,0,1);
        addError(dotnode->nodeLocation->fname,dotnode->nodeLocation->position,"Dot operator \".\" recieved type \""+streaml.str()+"\" but requires struct_t, on line: <todoline>.");
        return getNoType();
    }
    
    StructType * st = isStructType(leftsidetype);
    
    StructItem * si = st->si[isTermNode(don->right)->var];
    
    if(si == 0)
    {
        std::stringstream streaml;
        leftsidetype->print(streaml,0,1);
        
        addError(dotnode->nodeLocation->fname,dotnode->nodeLocation->position,"In dot operator \".\", right side \""+isTermNode(don->right)->var+"\" is not a part of type \""+streaml.str()+"\".");
        return getNoType();
    }
    else
    {
        return si->type;
    }
    
    return getNoType();
}
Type * unaryTypeCheckArray(Node *operand, VariableContext * vc, TypeMap * tm, string op)
{

    
    ArrayAccessNode * aan = isArrayAccessNode(operand);
    
    Type * lefttype = aan->left->typeCheck(vc,tm);
    Type * arrayexprtype = aan->arrayexpr->typeCheck(vc,tm);
    
    if(!(isUIntType(arrayexprtype)||isConstType(arrayexprtype)))
    {
        if(isIntType(arrayexprtype))
        {
            std::stringstream streaml;
            arrayexprtype->print(streaml,0);
            addWarning(operand->nodeLocation->fname,operand->nodeLocation->position,"Array operator \""+op+"\" recieved type \""+streaml.str()+"\" (int_t) for an indexing type instead of an uint_t or constant type. Int types will be treated as UInts");
        }
        else
        {
            std::stringstream streaml;
            arrayexprtype->print(streaml,0);
            addError(operand->nodeLocation->fname,operand->nodeLocation->position,"Array operator \""+op+"\" recieved type \""+streaml.str()+"\" for an indexing type instead of an uint_t or constant type.");
        }
    }
    
    if(!isArrayType(lefttype))
    {
        std::stringstream streaml;
        lefttype->print(streaml,0);
        addError(operand->nodeLocation->fname,operand->nodeLocation->position,"Array operator \""+op+"\" cannot be applied to type \""+streaml.str()+"\".");
        return getNoType();
    }
    
    if(isArrayType(lefttype)->sizes.size()==1)
    {
        return isArrayType(lefttype)->type;
    }
    else
    {
        string prefix = isArrayType(lefttype)->type->typeName;
        string suffix = "";
        
        vector<long> sizes;
        for(int i=1;i<isArrayType(lefttype)->sizes.size();i++)
        {
            suffix+="["+to_string(isArrayType(lefttype)->sizes[i]);
            sizes.push_back(isArrayType(lefttype)->sizes[i]);
        }
        
        Type * newt = (*tm)[prefix+suffix];
        if(newt == 0)
        {
            newt = new ArrayType(prefix);
            for(int L=0;L<sizes.size();L++)
            {
                ((ArrayType *) (newt))->sizes.push_back(sizes[L]);
            }
            ((ArrayType *) (newt))->type = (*tm)[prefix];
        }
        return newt;
    }
    
    //cout <<"add missing parts in unaryTypeCheckArray\n";
    return getNoType();
}
Type * unaryTypeCheckWire(Node *operand, Node *wirebase, Node * wiremount,VariableContext * vc, TypeMap * tm, string op)
{
    Type * t = operand->typeCheck(vc,tm);
    
    if(!(isIntType(t) || isConstType(t) || isUIntType(t)))
    {
        
        std::stringstream streaml;
        
        t->print(streaml,0,1);
        
        std::stringstream streamr;
        
        operand->print(streamr,0);
        
        addError(operand->nodeLocation->fname,operand->nodeLocation->position,"Wire operator operand, \""+streamr.str()+"\" must be a number type, not \""+streaml.str()+"\"");
        return getNoType();
    }
    
    t = wirebase->typeCheck(vc,tm);
    
    if(!(isIntType(t) || isConstType(t) || isUIntType(t)))
    {
        
        std::stringstream streaml;
        
        t->print(streaml,0,1);
        
        std::stringstream streamr;
        
        wirebase->print(streamr,0);
        
        addError(wirebase->nodeLocation->fname,wirebase->nodeLocation->position,"Wire operator wirebase, \""+streamr.str()+"\" must be a number type, not \""+streaml.str()+"\"");
        return getNoType();
    }
    
    
    //long temp = wirebase->getLongResultNoVar();
    
    if(wiremount == 0)
    {
        if(getTypeMap()["-+generated_cond_1_int_type+-"] == 0)
        {
            getTypeMap()["-+generated_cond_1_int_type+-"] = new UIntType("-+generated_cond_1_int_type+-",1);
        }
        
        return getTypeMap()["-+generated_cond_1_int_type+-"];
    }
    else
    {
        
        long length = wiremount->getLongResultNoVar();
        
        wiremount->typeCheck(vc,tm);
        
        if(length == 0)
        {
            
            std::stringstream streamr;
            
            wiremount->print(streamr,0);
            
            addError(operand->nodeLocation->fname,operand->nodeLocation->position,"Wire operator wiremount must be greater than 0");
            return getNoType();
        }
        
        if(getTypeMap()["-+generate_wire_type+-"+to_string(length)] == 0)
        {
            getTypeMap()["-+generate_wire_type+-"+to_string(length)] = new UIntType("-+generate_wire_type+-"+to_string(length),length);
        }
        
        return getTypeMap()["-+generate_wire_type+-"+to_string(length)];
    }
    
    
    //cout <<"add missing typecheck unaryTypeCheckWire\n";
    return getNoType();
}
Type * unaryTypeCheckNumberRequired(Node *operand,VariableContext * vc, TypeMap * tm, string op)
{
    Type * t = operand->typeCheck(vc,tm);
    
    if(isStructType(t) || isArrayType(t) || isFunctionType(t))
    {
        std::stringstream streaml;
        
        operand->print(streaml,0);
        
        addError(operand->nodeLocation->fname,operand->nodeLocation->position,"Operator \""+op+"\" cannot be applied to  \""+streaml.str()+"\", type base must be int_t.");
        
        return getNoType();
    }
    
    return t;
}

Type * InputOutputTypeCheck(Node *n,VariableContext * vc, TypeMap * tm)
{
    //n->print(cout,0);
    
    if(isInputNode(n))
    {
        InputNode * node = isInputNode(n);
        if((*tm)[isTermNode(node->type)->var]==0)
        {
            string nums = isTermNode(node->type)->num;
            
            if(nums != "")
            {
                addError(isTermNode(node->type)->nodeLocation->fname,isTermNode(node->type)->nodeLocation->position,"Type cannot be a number! (via define)\""+isTermNode(node->type)->num +"\"");
            }
            else
            {
                addError(isTermNode(node->type)->nodeLocation->fname,isTermNode(node->type)->nodeLocation->position,"Type \""+isTermNode(node->type)->var +"\" is used but not defined.");
            }
            return getNoType();
        }
    }
    
    if(isOutputNode(n))
    {
        OutputNode * node = isOutputNode(n);
        if((*tm)[isTermNode(node->type)->var]==0)
        {
            string nums = isTermNode(node->type)->num;
            
            if(nums != "")
            {
                addError(isTermNode(node->type)->nodeLocation->fname,isTermNode(node->type)->nodeLocation->position,"Type cannot be a number! (via define)\""+isTermNode(node->type)->num +"\"");
            }
            else
            {
                addError(isTermNode(node->type)->nodeLocation->fname,isTermNode(node->type)->nodeLocation->position,"Type \""+isTermNode(node->type)->var +"\" is used but not defined.");
            }
            return getNoType();
        }
    }
    
    return getNoType();
}

#include "circuitoutput.h"
#include "wirepool.h"






CORV  DefineNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV  OutputNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}

CORV IntTypedefNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV UIntTypedefNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV  IncludeNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV PartiesNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV  ProgramListNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV  StructTypedefNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV  InputNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV  VarDeclarationCommaNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "usednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV  ArrayDeclarationListNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV  DeclarationNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV  DeclarationCommaListNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV  ArrayDecNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV ArithModuloNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    /*preparation*/
    CORV leftcorv = left->circuitOutput(vc,tm, idxF);
    CORV rightcorv = right->circuitOutput(vc,tm, idxF);
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv;
    
    
    {
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        ensureAnyVariableToVec(rightcorv);
        rightv = &(rightcorv.vec);
        //ensureSameSize(*leftv,*rightv);
        ensureTypedSize(*leftv,*rightv,returnTypeOfNode,idxF);
    }
    
    
    CORV destv;
    destv.vec.resize(leftv->size());
    
    
    /*outputting the circuit*/
    if(isSigned)
    {
	    outputDivideSigned(leftv, rightv, destv.vec, true, idxF);
        //cout << "signed div\n";
    }
    else
    {
	    outputDivideUnsigned(leftv, rightv, destv.vec, true, idxF);
        //cout << "unsigned div\n";
    }
    
    /*cleanup*/
    int size =leftv->size();
    for(int i=0;i<size;i++)
    {
        leftv->operator[](i)->locked = false;
        rightv->operator[](i)->locked = false;
    }
    size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    
    return destv;

}
CORV  ArithDivNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    /*preparation*/
    CORV leftcorv = left->circuitOutput(vc,tm, idxF);
    CORV rightcorv = right->circuitOutput(vc,tm, idxF);
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv;
    
    
    {
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        ensureAnyVariableToVec(rightcorv);
        rightv = &(rightcorv.vec);
        //ensureSameSize(*leftv,*rightv);
	    ensureTypedSize(*leftv, *rightv, returnTypeOfNode, idxF);
    }
    
    
    CORV destv;
    destv.vec.resize(leftv->size());
    
    
    /*outputting the circuit*/
    if(isSigned)
    {
	    outputDivideSigned(leftv, rightv, destv.vec, false, idxF);
    }
    else
    {
	    outputDivideUnsigned(leftv, rightv, destv.vec, false, idxF);
    }
    
    /*cleanup*/
    int size =leftv->size();
    for(int i=0;i<size;i++)
    {
        leftv->operator[](i)->locked = false;
        rightv->operator[](i)->locked = false;
    }
    size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    
    return destv;

}

CORV ArithReModuloNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    /*preparation*/
    CORV leftcorv = left->circuitOutput(vc,tm, idxF);
    CORV rightcorv = right->circuitOutput(vc,tm, idxF);
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv;
    
    
    {
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        ensureAnyVariableToVec(rightcorv);
        rightv = &(rightcorv.vec);
        

        //cout << "origsize: "<<leftv->size()<<" "<<rightv->size()<<" "<<isSigned<<"\n";
	    ensureTypedSize(*leftv, operandTypeOfNode, idxF);
	    ensureTypedSize(*rightv, operand2TypeOfNode, idxF);
        //ensureSameSize(*leftv,*rightv);
        //cout << "postchangesize: "<<leftv->size()<<" "<<rightv->size()<<"\n";
    }
    
    
    CORV destv;
    destv.vec.resize(nodeSpecVariable);
    
    
    /*outputting the circuit*/
    if(isSigned)
    {
        outputReDivideSigned(leftv,rightv,destv.vec,true,nodeSpecVariable,idxF);
    }
    else
    {
	    outputReDivideUnsigned(leftv, rightv, destv.vec, true, nodeSpecVariable, idxF);
    }
    
    /*cleanup*/
    int size =leftv->size();
    for(int i=0;i<size;i++)
    {
        leftv->operator[](i)->locked = false;
    }
    
    size =rightv->size();
    for(int i=0;i<size;i++)
    {
        rightv->operator[](i)->locked = false;
    }
    
    size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    
    return destv;
    
}

CORV  ArithReDivNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    /*preparation*/
    CORV leftcorv = left->circuitOutput(vc,tm, idxF);
    CORV rightcorv = right->circuitOutput(vc,tm, idxF);
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv;
    
    
    {
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        ensureAnyVariableToVec(rightcorv);
        rightv = &(rightcorv.vec);
        //ensureSameSize(*leftv,*rightv);
	    ensureTypedSize(*leftv, operandTypeOfNode, idxF);
	    ensureTypedSize(*rightv, operand2TypeOfNode, idxF);
    }
    
    
    CORV destv;
    destv.vec.resize(nodeSpecVariable);
    
    
    /*outputting the circuit*/
    if(isSigned)
    {
	    outputReDivideSigned(leftv, rightv, destv.vec, false, nodeSpecVariable, idxF);
    }
    else
    {
	    outputReDivideUnsigned(leftv, rightv, destv.vec, false, nodeSpecVariable, idxF);
    }
    
    /*cleanup*/
    int size =leftv->size();
    for(int i=0;i<size;i++)
    {
        leftv->operator[](i)->locked = false;
        //rightv->operator[](i)->locked = false;
    }
    
    size =rightv->size();
    for(int i=0;i<size;i++)
    {
        rightv->operator[](i)->locked = false;
    }
    
    size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    
    return destv;
}


CORV  ArithExMultNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    //cout << "start ArithExMultNode\n";
    
    /*preparation*/
    CORV leftcorv = left->circuitOutput(vc,tm, idxF);
    CORV rightcorv = right->circuitOutput(vc,tm, idxF);
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv;
    
    
    {
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        ensureAnyVariableToVec(rightcorv);
        rightv = &(rightcorv.vec);
        //ensureSameSize(*leftv,*rightv);
	    ensureTypedSize(*leftv, *rightv, operandTypeOfNode, idxF);
    }
    
    //cout << "spec: "<<nodeSpecVariable<<"\n";
    
    CORV destv;
    destv.vec.resize(nodeSpecVariable);
    
    //cout << "spec: "<<nodeSpecVariable<<"\n";
    
    /*outputting the circuit*/
    if(isSigned)
    {
	    outputExMultSigned(leftv, rightv, destv.vec, idxF);
    }
    else
    {
	    outputExMultUnsigned(leftv, rightv, destv.vec, idxF);
    }
    
    
    /*cleanup*/
    int size =leftv->size();
    for(int i=0;i<size;i++)
    {
        leftv->operator[](i)->locked = false;
        rightv->operator[](i)->locked = false;
    }
    size = destv.vec.size();
    for(int i=0;i<nodeSpecVariable;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    
    //cout << "end ArithExMultNode\n";
    
    return destv;
    
}



CORV  FunctionReturnTypeNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV  VarDeclarationListNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV  VarDeclarationPrimNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV  FunctionVarDeclarationNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}
CORV DeclarationArgCommaListNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}

CORV  VarDeclarationForNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}


long wiresToLong(CORV * wireset, string errorMessageIfUnknownsFound, Node * errorNode)
{
    int size=0;
    int startplace = 0;
    if(wireset->var == 0)
    {
        size = wireset->vec.size();
    }
    else
    {
        size = wireset->var->size();
        startplace = wireset->var->wv->startwirenum;
    }
    
    long l=0;
    
    bool isint = wireset->var != 0 &&  wireset->var->v_enum == Intv;
    vector<Wire *> * intvec;
    
    if(isint)
    {
        intvec = &(((IntVariable *) (wireset->var))->wires);
    }
    
    for(int i=0;i<size && i < 64;i++)
    {
        if(wireset->var == 0)
        {
            if(wireset->vec[i]->state == ONE)
            {
                l = l | (1 << i);
            }
            else if(wireset->vec[i]->state == ZERO)
            {
                
            }
            else
            {
                addError(errorNode->nodeLocation->fname,errorNode->nodeLocation->position,errorMessageIfUnknownsFound);
                if(hasError())
                {
                    printErrors(std::cout);
                    std::cout <<"Errors, exiting... \n";
                    exit(1);
                }
            }
        }
        else
        {
            Wire * w;
            

            w = getWire(i+startplace,wireset->var->wv);
            
            
            if(w->state == ONE)
            {
                l = l | (1 << i);
            }
            else if(w->state  == ZERO)
            {
                
            }
            else
            {
                addError(errorNode->nodeLocation->fname,errorNode->nodeLocation->position,errorMessageIfUnknownsFound);
                if(hasError())
                {
                    printErrors(std::cout);
                    std::cout <<"Errors, exiting... \n";
                    exit(1);
                }
            }
        }
    }
    
    if(l < 0)
    {
        addError(errorNode->nodeLocation->fname,errorNode->nodeLocation->position,errorMessageIfUnknownsFound);
        if(hasError())
        {
            printErrors(std::cout);
            std::cout <<"Errors, exiting... \n";
            exit(1);
        }
    }
    
    return l;
}



void unlockCORV(CORV & c)
{

    int size1 = c.vec.size();
    if(c.var != 0)
    {
        size1 = c.var->size();
    }
    
    int startplace = 0;
    if(c.var != 0)
    {
        startplace = c.var->wv->startwirenum;
        
        if(c.var->isconst || (!c.var->isfunctioncallvar && !c.var->isconst))
        {
            return;
        }
    }
    
    bool isInt = false;
    if(c.var != 0 && c.var->v_enum == Intv)
    {
        isInt = true;
    }
    
    for(int i=startplace;i<size1+startplace;i++)
    {
        if(c.var == 0)
        {
            c.vec[i]->locked = false;
        }
        else
        {
            if(isInt)
            {
                ((IntVariable *)c.var)->wires[i - startplace]->locked = false;
            }
            else
            {
                getWire(i,c.var->wv)->locked = false;
            }
        }
    }
}

void lockCORVFuncCall(CORV & c)
{
    
    int size1 = c.vec.size();
    if(c.var != 0)
    {
        size1 = c.var->size();
    }
    
    int startplace = 0;
    if(c.var != 0)
    {
        startplace = c.var->wv->startwirenum;
    }
    
    bool isInt = false;
    if(c.var != 0 && c.var->v_enum == Intv)
    {
        isInt = true;
    }
    
    for(int i=startplace;i<size1+startplace;i++)
    {
        if(c.var == 0)
        {
            c.vec[i]->locked = true;
        }
        else
        {
            if(isInt)
            {
                ((IntVariable *)c.var)->wires[i - startplace]->locked = true;
            }
            else
            {
                getWire(i,c.var->wv)->locked = true;
            }
        }
    }
}

void lockCORV(CORV & c)
{
    
    int size1 = c.vec.size();
    if(c.var != 0)
    {
        size1 = c.var->size();
    }
    
    int startplace = 0;
    if(c.var != 0)
    {
        startplace = c.var->wv->startwirenum;
        
        if(getWire(startplace,c.var->wv)->varlocked || c.var->isconst || (!c.var->isfunctioncallvar && !c.var->isconst)) //var locked, no reason to continue;
        {
            return;
        }
    }
    
    bool isInt = false;
    if(c.var != 0 && c.var->v_enum == Intv)
    {
        isInt = true;
    }
    
    for(int i=startplace;i<size1+startplace;i++)
    {
        if(c.var == 0)
        {
            c.vec[i]->locked = true;
        }
        else
        {
            if(isInt)
            {
                ((IntVariable *)c.var)->wires[i - startplace]->locked = true;
            }
            else
            {
                getWire(i,c.var->wv)->locked = true;
            }
        }
    }
}

void lockCORVvec(CORV & c)
{
    for(int i=0;i<c.vec.size();i++)
    {
        c.vec[i]->locked = true;
    }
}



CORV  AssignNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
	CORV leftcorv = left->circuitOutput(vc, tm, idxF);
	CORV rightcorv = expr->circuitOutput(vc, tm, idxF);

	

	
	string strDPGlobalOut;
    
    Variable * var_x = (*vc)["-+IFCOND+-"];
    
    putVariableToVector(rightcorv);
    
    int size;
    
   if(leftcorv.var != 0)
    {
        size = leftcorv.var->size();
    }
    else
    {
        size = leftcorv.vec.size();
    }
    
    int startplace=0;
    
    if(leftcorv.var != 0)
    {
        startplace = leftcorv.var->wv->startwirenum;
    }
    
	ensureSize(rightcorv.vec, size, idxF);
    
    Variable * leftvar = leftcorv.var;
    
    if(leftvar == 0)
    {
        leftvar = leftcorv.vec[0]->thisvar;
    }
    
    if(leftvar == 0)
    {
        cout << "Wire variable is 0. This should not happen (assignnode)\n";
        this->print(cout,0);cout <<"\n";
    }
    
    
    if(var_x == 0 || (var_x->getVarDepth() < leftvar->getVarDepth()))
    {
        for(int i=startplace;i<size+startplace;i++)
        {
            Wire * w1;
            
            if(leftcorv.var != 0)
            {
                w1 = getWire(i,leftcorv.var->wv);
            }
            else
            {
                w1 = leftcorv.vec[i];
            }
            
            Wire * w2 = rightcorv.vec[i-startplace];
            

            if(w1->refs > 0 && (!(w2->other ==w1 && w1->refs ==1) ))
            {
	            if (expr->nodeName == "FunctionCallNode")
		            clearReffedWireForFn(w1, idxF);
	            else 
		            clearReffedWire(w1, idxF);

            }
            
	        assignWire(w1, w2, idxF);
            
	        if (!getIsTiny())
	        {
		        if (expr->nodeName == "FunctionCallNode")
		        {
			        makeWireContainValueNoONEZEROcopyForFn(w1, idxF) ;
		        }
		        else
			        makeWireContainValueNoONEZEROcopy(w1, idxF);
	        }
            else
            {

	            makeWireContainValueNoONEZEROcopyTiny(w1, idxF);
            }
	        if (expr->nodeName == "FunctionCallNode")
	        {
		        strDPGlobalOut.append(to_string(w1->wireNumber) + " ");
	        }
        }
        if(getIsTiny())
        {
            makeWireContainValueNoONEZEROcopyTinyEnd();
        }
    }
    else
    {
        Wire * cond = ((IntVariable *)var_x)->wires[0];
        
        for(int i=startplace;i<size+startplace;i++)
        {
            
            Wire * w1;
            
            if(leftcorv.var != 0)
            {
                w1 = getWire(i,leftcorv.var->wv);
            }
            else
            {
                w1 = leftcorv.vec[i];
            }
            
            Wire * w2 = rightcorv.vec[i-startplace];           
     
            
	        //assignWireCond(w1, w2, cond, idxF);
            
            /*if(!getIsTiny())
            {
                makeWireContainValueNoONEZEROcopy(w1);
            }
            else
            {
                makeWireContainValueNoONEZEROcopyTiny(w1);
            }*/
            
	        if (expr->nodeName == "FunctionCallNode")
	        {
		        if (w1->refs > 0)
		        {
			        clearReffedWireForFn(w1, idxF);
		        }
		        assignWireCondForFn(w1, w2, cond, idxF);
		        makeWireContainValueNoONEZEROcopyForFn(w1, idxF);
		        strDPGlobalOut.append(to_string(w1->wireNumber) + " ");
	        }
	        else
	        {
		        if (w1->refs > 0)
		        {
			        clearReffedWire(w1, idxF);
		        }
		        assignWireCond(w1, w2, cond, idxF);
		        makeWireContainValueNoONEZEROcopy(w1, idxF);
	        }
		       
		}
        /*if(getIsTiny())
        {
            makeWireContainValueNoONEZEROcopyTinyEnd();
        }*/
    }

    
    
    unlockCORV(rightcorv);
    
    getPool(idxF)->freeIfNoRefs();
	if (isMainFunction())
		appendDuploGC(strDPGlobalOut + "\n\n", expr->nodeName == "FunctionCallNode"); //"Global OUT: " +     	
	else
		appendDuploGC("++ "+ strDPGlobalOut + "\n", expr->nodeName == "FunctionCallNode"); //"Global OUT: " + 
	
    return leftcorv;
}


CORV  UnaryNOTNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
	CORV leftcorv = operand->circuitOutput(vc, tm, idxF);
    
    ensureIntVariableToVec(leftcorv);
    vector<Wire *> * leftv = &(leftcorv.vec);

    CORV v;
    v.vec.resize(leftv->size());
    
    WireSet * ws = getPool(idxF)->getWires(leftv->size());
    
    for(int i=0;i<leftv->size();i++)
    {
        v.vec[i] = ws->wires[i];
	    Wire * d = invertWire(leftv->operator[](i), idxF);
	    assignWire(v.vec[i], d, idxF);
    }
    
    for(int i=0;i<leftv->size();i++)
    {
        leftv->operator[](i)->locked = false;
    }
    for(int i=0;i<v.vec.size();i++)
    {
        v.vec[i]->locked = true;
    }
    getPool(idxF)->freeIfNoRefs();
    return v;
}



CORV  ArithPlusNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    /*preparation*/
	CORV leftcorv = left->circuitOutput(vc, tm, idxF);
	CORV rightcorv = right->circuitOutput(vc, tm, idxF);
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv;
    
    
    
    {
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        ensureAnyVariableToVec(rightcorv);
        rightv = &(rightcorv.vec);
        //ensureSameSize(*leftv,*rightv);
	    ensureTypedSize(*leftv, *rightv, returnTypeOfNode, idxF);
    }

    
    CORV destv;
    destv.vec.resize(leftv->size());
    

    /*outputting the circuit*/
	outputAddition(leftv, rightv, destv.vec, idxF);

    
    /*cleanup*/
    int size =leftv->size();
    for(int i=0;i<size;i++)
    {
        leftv->operator[](i)->locked = false;
        rightv->operator[](i)->locked = false;
    }
    size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    
    //cout << "size of cleanup "<<size<<"\n";
    
    return destv;

}

CORV  ArithMultNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    /*preparation*/
	CORV leftcorv = left->circuitOutput(vc, tm, idxF);
	CORV rightcorv = right->circuitOutput(vc, tm, idxF);
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv;
    

    {
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        ensureAnyVariableToVec(rightcorv);
        rightv = &(rightcorv.vec);
        //ensureSameSize(*leftv,*rightv);
	    ensureTypedSize(*leftv, *rightv, returnTypeOfNode, idxF);
    }
    
    
    CORV destv;
    destv.vec.resize(leftv->size());
    

    /*outputting the circuit*/
    if(isSigned)
    {
	    outputMultSigned(leftv, rightv, destv.vec, idxF);
    }
    else
    {
	    outputMultUnsigned(leftv, rightv, destv.vec, idxF);
    }
    
    
    /*cleanup*/
    int size =leftv->size();
    for(int i=0;i<size;i++)
    {
        leftv->operator[](i)->locked = false;
        rightv->operator[](i)->locked = false;
    }
    size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    
    return destv;

}


CORV  UnaryPrePlusPlusNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
	CORV leftcorv = operand->circuitOutput(vc, tm, idxF);
    
    vector<Wire *> onevec;
	onevec.push_back(get_ONE_WIRE(idxF));
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv = &onevec;
    
    if(leftcorv.var != 0)
    {
        ensureIntVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        //leftv  = &(((IntVariable *)leftcorv.var)->wires);
	    ensureSize(onevec, ((IntVariable *)leftcorv.var)->size(), idxF);
    }
    else
    {
        /*this should not be necessary as variable is required, but just in case -> better to be safe than sorry*/
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
	    ensureSameSize(*leftv, *rightv, idxF);
    }
    
    CORV destv;
    destv.vec.resize(leftv->size());
    
    /*outputting the circuit*/
	outputAddition(leftv, rightv, destv.vec, idxF);
    
    
    //force leftcorv to be in vec
    ensureIntVariableToVec(leftcorv);
    
    IntVariable * destvariable = (IntVariable *) (leftcorv.var);
    leftcorv.vec.resize(0);
    
    
    for(int i=0;i<destv.vec.size();i++)
    {
        destv.vec[i]->locked = true;
    }
    
    /*intermedate cleanup to remove unnecessary refs that would break the assignWire function*/
    
	getPool(idxF)->freeIfNoRefs();
    
    
    Variable * var_x = (*vc)["-+IFCOND+-"];
    
    Variable * leftvar = destvariable;
    
    
    if(var_x == 0 || (var_x->getVarDepth() < leftvar->getVarDepth()))
    {
        //assign destv to variable (i.e. NOT THE LEFT CORV)
        for(int i=0;i<destv.vec.size();i++)
        {
            
	        assignWire(destvariable->wires[i], destv.vec[i], idxF);
            destv.vec[i]->locked = false;
	        makeWireContainValueNoONEZEROcopy(destvariable->wires[i], idxF);
        }
    }
    else
    {
        Wire * cond = ((IntVariable *)var_x)->wires[0];
        
        //assign destv to variable (i.e. NOT THE LEFT CORV)
        for(int i=0;i<destv.vec.size();i++)
        {
            
	        assignWireCond(destvariable->wires[i], destv.vec[i], cond, idxF);
            destv.vec[i]->locked = false;
	        makeWireContainValueNoONEZEROcopy(destvariable->wires[i], idxF);
        }
    }
    
    /*cleanup*/
    getPool(idxF)->freeIfNoRefs();
    
    return leftcorv;
}
CORV ConditionalLessNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    /*preparation*/
	CORV leftcorv = left->circuitOutput(vc, tm, idxF);
	CORV rightcorv = right->circuitOutput(vc, tm, idxF);
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv;
    
    {
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        ensureAnyVariableToVec(rightcorv);
        rightv = &(rightcorv.vec);
	    ensureSameSize(*leftv, *rightv, idxF);
        //ensureTypedSize(*leftv,*rightv,typeOfNode);
	    ensureTypedSize(*leftv, *rightv, operandTypeOfNode, idxF);
    }
    
    CORV destv;
    destv.vec.resize(1);
    
    //ensureTypedSize(*leftv,*rightv,typeOfNode);
    
    //ensureSize(*leftv,16);
    //ensureSize(*rightv,16);
    
    //leftv->resize(1);
    //rightv->resize(1);
    
    /*outputting the circuit*/
    if(isSigned)
    {
	    outputLessThanSigned(leftv, rightv, destv.vec, idxF);
    }
    else
    {
	    outputLessThanUnsigned(leftv, rightv, destv.vec, idxF);
    }

    
    /*cleanup*/
    int size =leftv->size();
    for(int i=0;i<size;i++)
    {
        leftv->operator[](i)->locked = false;
        //rightv->operator[](i)->locked = false;
    }
    size =rightv->size();
    for(int i=0;i<size;i++)
    {
        //leftv->operator[](i)->locked = false;
        rightv->operator[](i)->locked = false;
    }
    
    size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    //leftv.resize(1);
    //rightv.resize(1);
    
    
    
    getPool(idxF)->freeIfNoRefs();
    return destv;
}


CORV  ConditionalEqualNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    /*preparation*/
	CORV leftcorv = left->circuitOutput(vc, tm, idxF);
    
	CORV rightcorv = right->circuitOutput(vc, tm, idxF);
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv;
    
    {
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        ensureAnyVariableToVec(rightcorv);
        rightv = &(rightcorv.vec);
        //ensureSameSize(*leftv,*rightv);
	    ensureTypedSize(*leftv, *rightv, operandTypeOfNode, idxF);
    }
    
    CORV destv;
    destv.vec.resize(1);
    
    /*outputting the circuit*/
	outputEquals(leftv, rightv, destv.vec, idxF);
    
    
    /*cleanup*/
    int size =leftv->size();
    for(int i=0;i<size;i++)
    {
        leftv->operator[](i)->locked = false;
        rightv->operator[](i)->locked = false;
    }
    size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    return destv;
}
CORV  ConditionalGreaterNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    /*preparation*/
	CORV leftcorv = left->circuitOutput(vc, tm, idxF);
	CORV rightcorv = right->circuitOutput(vc, tm, idxF);
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv;
    
    {
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        ensureAnyVariableToVec(rightcorv);
        rightv = &(rightcorv.vec);
        //ensureSameSize(*leftv,*rightv);
	    ensureTypedSize(*leftv, *rightv, operandTypeOfNode, idxF);
    }
    
    CORV destv;
    destv.vec.resize(1);
    
    /*outputting the circuit*/
    /*notice the parameter reversal*/
    if(isSigned)
    {
	    outputLessThanSigned(rightv, leftv, destv.vec, idxF);
    }
    else
    {
	    outputLessThanUnsigned(rightv, leftv, destv.vec, idxF);
    }
    
    /*cleanup*/
    int size =leftv->size();
    for(int i=0;i<size;i++)
    {
        leftv->operator[](i)->locked = false;
        rightv->operator[](i)->locked = false;
    }
    size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    return destv;

}

CORV  BitwiseORNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    CORV leftcorv = left->circuitOutput(vc,tm, idxF);
    CORV rightcorv = right->circuitOutput(vc,tm, idxF);
    
    ensureIntVariableToVec(leftcorv);
    ensureIntVariableToVec(rightcorv);
    
    vector<Wire *> * leftv = &(leftcorv.vec);
    vector<Wire *> * rightv = &(rightcorv.vec);
    
    //ensureSameSize(*leftv,*rightv);
	ensureTypedSize(*leftv, *rightv, returnTypeOfNode, idxF);

    
    CORV v;
    v.vec.resize(leftv->size());
    
    WireSet * ws = getPool(idxF)->getWires(leftv->size());
    
    for(int i=0;i<leftv->size();i++)
    {
        v.vec[i] = ws->wires[i];
    }
    
    for(int i=0;i<leftv->size();i++)
    {
	    Wire * d = outputGate(14, leftv->operator[](i), rightv->operator[](i), idxF);
	    assignWire(v.vec[i], d, idxF);
    }
    
    for(int i=0;i<leftv->size();i++)
    {
        leftv->operator[](i)->locked = false;
        rightv->operator[](i)->locked = false;
    }
    for(int i=0;i<v.vec.size();i++)
    {
        v.vec[i]->locked = true;
    }
    getPool(idxF)->freeIfNoRefs();
    
    return v;
}

CORV  ShiftLeftNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    
    
    /*preparation*/
	CORV leftcorv = left->circuitOutput(vc, tm, idxF);
	CORV rightcorv = right->circuitOutput(vc, tm, idxF);
    

    long shiftamt;
    shiftamt = wiresToLong(&rightcorv,"right side of shift must not be dependent on input and must be > 0.",this);
    
    CORV destv;
    
    int lsize = leftcorv.vec.size();
    if(leftcorv.var != 0)
    {
        lsize = leftcorv.var->size();
    }

    

    
    /*shifting*/
    if(leftcorv.var ==0)
    {
        destv.vec.resize(lsize+shiftamt);
        
        WireSet * ws = getPool(idxF)->getWires(lsize+shiftamt);
        
        for(int i=0;i<lsize+shiftamt;i++)
        {
            destv.vec[i] = ws->wires[i];
        }

        for(int i=0;i<shiftamt;i++)
        {
	        assignWire(destv.vec[i], get_ZERO_WIRE(idxF), idxF);
        }
        for(int i=0;i<leftcorv.vec.size();i++)
        {
            //destv.vec[i+shiftamt] = getPool(idxF)->getWire();
	        assignWire(destv.vec[i + shiftamt], leftcorv.vec[i], idxF);
	        makeWireContainValueNoONEZEROcopy(destv.vec[i + shiftamt], idxF);
        }
    }
    else
    {
        
        IntVariable * iv = (IntVariable *) leftcorv.var;
        
        if(leftcorv.var->isconst)
        {
            destv.vec.resize(lsize+shiftamt);
            
            WireSet * ws = getPool(idxF)->getWires(lsize+shiftamt);
            
            for(int i=0;i<lsize+shiftamt;i++)
            {
                destv.vec[i] = ws->wires[i];
            }
            
            //destv.vec.resize(lsize+shiftamt);
            
            for(int i=0;i<shiftamt;i++)
            {
                //destv.vec[i] = get_ZERO_WIRE(idxF);
                assignWire(destv.vec[i],get_ZERO_WIRE(idxF), idxF);
            }
            for(int i=0;i<leftcorv.var->size();i++)
            {
	            assignWire(destv.vec[i + shiftamt], iv->wires[i], idxF);
	            makeWireContainValueNoONEZEROcopy(destv.vec[i + shiftamt], idxF);
            }
        }
        else
        {
            destv.vec.resize(lsize);
            
            WireSet * ws = getPool(idxF)->getWires(lsize);
            
            for(int i=0;i<lsize;i++)
            {
                destv.vec[i] = ws->wires[i];
            }
            
            //destv.vec.resize(lsize);
            for(int i=0;i<shiftamt && i < destv.vec.size();i++)
            {
                //destv.vec[i] = get_ZERO_WIRE(idxF);
	            assignWire(destv.vec[i], get_ZERO_WIRE(idxF), idxF);
            }
            for(int i=0;i<lsize && (i+shiftamt) < destv.vec.size();i++)
            {
                //destv.vec[i+shiftamt] = getPool(idxF)->getWire();
	            assignWire(destv.vec[i + shiftamt], iv->wires[i], idxF);
	            makeWireContainValueNoONEZEROcopy(destv.vec[i + shiftamt], idxF);
            }
            
        }
    }
    
    /*cleanup*/
    unlockCORV(leftcorv);
    unlockCORV(rightcorv);

    int size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    
    
    getPool(idxF)->freeIfNoRefs();
    return destv;

}
CORV  ArithMinusNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    /*preparation*/
    CORV leftcorv = left->circuitOutput(vc,tm, idxF);
    CORV rightcorv = right->circuitOutput(vc,tm, idxF);
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv;
    
    
    ensureAnyVariableToVec(leftcorv);
    leftv = &(leftcorv.vec);
    ensureAnyVariableToVec(rightcorv);
    rightv = &(rightcorv.vec);
	ensureTypedSize(*leftv, *rightv, returnTypeOfNode, idxF);
    

    
    CORV destv;
    destv.vec.resize(leftv->size());
    
    /*outputting the circuit*/
	outputSubtract(leftv, rightv, destv.vec, idxF);
    
    /*cleanup*/
    int size =leftv->size();
    for(int i=0;i<size;i++)
    {
        leftv->operator[](i)->locked = false;
    }
    size =rightv->size();
    for(int i=0;i<size;i++)
    {
        rightv->operator[](i)->locked = false;
    }
    size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    return destv;
}
CORV  BitwiseANDNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    CORV leftcorv = left->circuitOutput(vc,tm, idxF);
    CORV rightcorv = right->circuitOutput(vc,tm, idxF);
    
    ensureIntVariableToVec(leftcorv);
    ensureIntVariableToVec(rightcorv);
    
    vector<Wire *> * leftv = &(leftcorv.vec);
    vector<Wire *> * rightv = &(rightcorv.vec);
    
    //ensureSameSize(*leftv,*rightv);
	ensureTypedSize(*leftv, *rightv, returnTypeOfNode, idxF);
    
    
    CORV v;
    v.vec.resize(leftv->size());
    
    WireSet * ws = getPool(idxF)->getWires(leftv->size());
    
    for(int i=0;i<leftv->size();i++)
    {
        v.vec[i] = ws->wires[i];
    }
    
    for(int i=0;i<leftv->size();i++)
    {
	    Wire * d = outputGate(8, leftv->operator[](i), rightv->operator[](i), idxF);
	    assignWire(v.vec[i], d, idxF);
    }
    
    for(int i=0;i<leftv->size();i++)
    {
        leftv->operator[](i)->locked = false;
        rightv->operator[](i)->locked = false;
    }
    for(int i=0;i<v.vec.size();i++)
    {
        v.vec[i]->locked = true;
    }
    getPool(idxF)->freeIfNoRefs();
    return v;
}
CORV  BitwiseXORNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    CORV leftcorv = left->circuitOutput(vc,tm, idxF);
    CORV rightcorv = right->circuitOutput(vc,tm, idxF);
    
    ensureIntVariableToVec(leftcorv);
    ensureIntVariableToVec(rightcorv);
    
    vector<Wire *> * leftv = &(leftcorv.vec);
    vector<Wire *> * rightv = &(rightcorv.vec);
    
    //ensureSameSize(*leftv,*rightv);
	ensureTypedSize(*leftv, *rightv, returnTypeOfNode, idxF);
    
    CORV v;
    v.vec.resize(leftv->size());
    
    WireSet * ws = getPool(idxF)->getWires(leftv->size());
    
    for(int i=0;i<leftv->size();i++)
    {
        v.vec[i] = ws->wires[i];
    }
    
    for(int i=0;i<leftv->size();i++)
    {
	    Wire * d = outputGate(6, leftv->operator[](i), rightv->operator[](i), idxF);
	    assignWire(v.vec[i], d, idxF);
    }
    
    for(int i=0;i<leftv->size();i++)
    {
        leftv->operator[](i)->locked = false;
        rightv->operator[](i)->locked = false;
    }
    for(int i=0;i<v.vec.size();i++)
    {
        v.vec[i]->locked = true;
    }
    getPool(idxF)->freeIfNoRefs();
    return v;
}

CORV  RotateLeftNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    /*preparation*/
	CORV leftcorv = left->circuitOutput(vc, tm, idxF);
	CORV rightcorv = right->circuitOutput(vc, tm, idxF);
    
    
    long shiftamt;
    shiftamt = wiresToLong(&rightcorv,"right side of shift must not be dependent on input and must be > 0.",this);
    
    CORV destv;
    
    int lsize = leftcorv.vec.size();
    if(leftcorv.var != 0)
    {
        lsize = leftcorv.var->size();
    }
    
    shiftamt = shiftamt % lsize;
    

    destv.vec.resize(lsize);
    
    WireSet * ws = getPool(idxF)->getWires(lsize);
    
    for(int i=0;i<lsize;i++)
    {
        destv.vec[i] = ws->wires[i];
    }
    
    
    /*rotating*/
    if(leftcorv.var ==0)
    {
        //destv.vec.resize(lsize);

        for(int i=0;i<leftcorv.vec.size();i++)
        {
            //destv.vec[(i+shiftamt)%lsize] = getPool(idxF)->getWire();
	        assignWire(destv.vec[(i + shiftamt) % lsize], leftcorv.vec[i], idxF);
	        makeWireContainValueNoONEZEROcopy(destv.vec[(i + shiftamt) % lsize], idxF);
        }
    }
    else
    {
        
        IntVariable * iv = (IntVariable *) leftcorv.var;
        
        if(leftcorv.var->isconst)
        {
            //destv.vec.resize(lsize);

            for(int i=0;i<lsize;i++)
            {
	            assignWire(destv.vec[(i + shiftamt) % lsize], iv->wires[i],idxF);
            }
        }
        else
        {
            //destv.vec.resize(lsize);

            for(int i=0;i<lsize;i++)
            {
                //destv.vec[(i+shiftamt)%lsize] = getPool(idxF)->getWire();
	            assignWire(destv.vec[(i + shiftamt) % lsize], iv->wires[i], idxF);
	            makeWireContainValueNoONEZEROcopy(destv.vec[(i + shiftamt) % lsize], idxF);
            }
            
        }
    }
    
    /*cleanup*/
    unlockCORV(leftcorv);
    unlockCORV(rightcorv);
    
    int size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    return destv;
}
CORV  ShiftRightNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    /*preparation*/
	CORV leftcorv = left->circuitOutput(vc, tm, idxF);
	CORV rightcorv = right->circuitOutput(vc, tm, idxF);
    
    
    long shiftamt;
    shiftamt = wiresToLong(&rightcorv,"right side of shift must not be dependent on input and must be > 0.",this);
    
    CORV destv;
    
    int lsize = leftcorv.vec.size();
    if(leftcorv.var != 0)
    {
        lsize = leftcorv.var->size();
    }
    
    

    
    
    /*shifting*/
    if(leftcorv.var ==0)
    {
        int newsize = lsize-shiftamt;
        
        if(newsize <= 0)
            newsize = 1;
        
        destv.vec.resize(newsize);
        
        WireSet * ws = getPool(idxF)->getWires(newsize);
        
        for(int i=0;i<newsize;i++)
        {
            destv.vec[i] = ws->wires[i];
        }
    
        int i;
        for(i=0;i<lsize - shiftamt;i++)
        {
            //destv.vec[i] = getPool(idxF)->getWire();
	        assignWire(destv.vec[i], leftcorv.vec[i + shiftamt], idxF);
	        makeWireContainValue(destv.vec[i], idxF);
        }
        for(;i<newsize;i++)
        {
	        destv.vec[i] = get_ZERO_WIRE(idxF);
        }
    }
    else
    {
        IntVariable * iv = (IntVariable *) leftcorv.var;
        
        if(leftcorv.var->isconst)
        {
            int newsize = lsize-shiftamt;
            
            if(newsize <= 0)
                newsize = 1;
            
            destv.vec.resize(newsize);
            
            WireSet * ws = getPool(idxF)->getWires(newsize);
            
            for(int i=0;i<newsize;i++)
            {
                destv.vec[i] = ws->wires[i];
            }
            
            int i;
            for(i=0;i<lsize - shiftamt;i++)
            {
                //destv.vec[i] = getPool(idxF)->getWire();
	            assignWire(destv.vec[i], iv->wires[i + shiftamt], idxF);
	            makeWireContainValue(destv.vec[i], idxF);
            }
            for(;i<newsize;i++)
            {
	            assignWire(destv.vec[i], get_ZERO_WIRE(idxF), idxF);
            }
        }
        else
        {
            destv.vec.resize(lsize);
            
            WireSet * ws = getPool(idxF)->getWires(lsize);
            
            for(int i=0;i<lsize;i++)
            {
                destv.vec[i] = ws->wires[i];
            }
            
            //destv.vec.resize(lsize);
            int i;
            for(i=0;i<iv->size() - shiftamt;i++)
            {
                //destv.vec[i] = getPool(idxF)->getWire();
	            assignWire(destv.vec[i], iv->wires[i + shiftamt], idxF);
	            makeWireContainValue(destv.vec[i], idxF);
            }
            for(;i<iv->size();i++)
            {
                //this would need to be modified for signextend
	            assignWire(destv.vec[i], get_ZERO_WIRE(idxF), idxF);
            }
        }
    }
    
    /*cleanup*/
    unlockCORV(leftcorv);
    unlockCORV(rightcorv);
    
    int size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    return destv;
}
CORV  UnaryMinusNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    
    /*preparation*/
    CORV leftcorv = operand->circuitOutput(vc,tm, idxF);
    
    
    vector<Wire *> rightrealv;
    rightrealv.resize(1);
    rightrealv[0] = get_ZERO_WIRE(idxF);
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv = &rightrealv;
    
    /*converts CORV to a state to be used */
    if(leftcorv.var != 0)
    {
        ensureIntVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        //leftv  = &(((IntVariable *)leftcorv.var)->wires);
	    ensureSize(rightrealv, ((IntVariable *)leftcorv.var)->size(), idxF);
    }
    else
    {
        /*this should not be necessary as variable is required, but just in case -> better to be safe than sorry*/
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
	    ensureSameSize(*leftv, *rightv, idxF);
    }
    
    CORV destv;
    destv.vec.resize(leftv->size());
    
    /*outputting the circuit*/
	outputSubtract(rightv, leftv, destv.vec, idxF);
    
    /*cleanup*/
    int size =leftv->size();
    for(int i=0;i<size;i++)
    {
        leftv->operator[](i)->locked = false;
    }
    size =rightv->size();
    for(int i=0;i<size;i++)
    {
        rightv->operator[](i)->locked = false;
    }
    size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    return destv;

}

int counta=0;
int countb=0;

CORV  ReturnNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    if(operand != 0)
    {
        CORV rcorv = operand->circuitOutput(vc,tm, idxF);
        
        CORV * c = &(rcorv);
        Variable * paramvar = (*vc)["-+RETURNTYPE+-"];
        
        if(c->var == 0)
        {
            
            int j=0;
            for(j=0;j<c->vec.size() && j < paramvar->size();j++)
            {
                Wire * w = ((IntVariable *)paramvar)->wires[j];
                
	            assignWire(w, c->vec[j], idxF);
	            makeWireContainValue(w, idxF);
            }
            for(;j<paramvar->size();j++)
            {
                Wire * w = ((IntVariable *)paramvar)->wires[j];
	            assignWire(w, get_ZERO_WIRE(idxF), idxF);
	            makeWireContainValue(w, idxF);
            }
        }
        else
        {
            if(c->var->v_enum == Intv)
            {
                IntVariable * ivar = (IntVariable *)(c->var);
                
                int j=0;
                for(j=0;j<ivar->size() && j < paramvar->size();j++)
                {
                    Wire * w = ((IntVariable *)paramvar)->wires[j];
                    
	                assignWire(w, ivar->wires[j], idxF);
	                makeWireContainValue(w, idxF);
                }
                for(;j<paramvar->size();j++)
                {
                    Wire * w = ((IntVariable *)paramvar)->wires[j];
	                assignWire(w, get_ZERO_WIRE(idxF), idxF);
	                makeWireContainValue(w, idxF);
                }
            }
            else
            {
                Variable * anyvar = c->var;
                
                int startanyvar = anyvar->wv->startwirenum;
                int startparam = paramvar->wv->startwirenum;
                
                for(int j=0;j<anyvar->size();j++)
                {
                    Wire * w = getWire(j+startparam,paramvar->wv);
                    
	                assignWire(w, getWire(j + startanyvar, anyvar->wv), idxF);
	                makeWireContainValue(w, idxF);
                }
            }
        }
        
        unlockCORV(*c);

    }
    
    
    CORV v;
    return v;
}
CORV  ParenExprNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    
    if(operand == 0)
    {
        CORV v;
        return v;
    }
    else
    {
        CORV v = operand->circuitOutput(vc,tm, idxF);
        return v;
    }
}

CORV  WireAccessNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    //startTime = RDTSC;
    //startTimeb = RDTSC;
    long size = 1;
    
	CORV leftexpr = left->circuitOutput(vc, tm, idxF);
    
	CORV base = wirebase->circuitOutput(vc, tm, idxF);
    long baseint = wiresToLong(&base,"Wire base must not be \"unknown\", i.e. dependent on input, value and must be > 0.",this);
    //long baseint = 0;
    
    int startplace = 0;
    
    CORV amt;
    if(wireamount != 0)
    {
	    amt = wireamount->circuitOutput(vc, tm, idxF);
        size = wiresToLong(&amt,"Wire amount must not be \"unknown\", i.e. dependent on input, value. and must be > 0",this);
    }
    
   

    int leftexprmax = leftexpr.vec.size();
    if(leftexpr.var != 0)
    {
        leftexprmax = leftexpr.var->size();
    }
    
    /*endTimeb = RDTSC;
    cout << "a: "<<(endTimeb - startTimeb)<<"\n";*/
    //startTimeb = RDTSC;
    
    CORV v;
    v.vec.resize(size);
    
    
    
    /*endTimeb = RDTSC;
    cout <<"b: "<<(endTimeb - startTimeb)<<"\n";
    startTimeb = RDTSC;*/
    
    
    
    if(leftexpr.var != 0)
    {
        startplace = leftexpr.var->wv->startwirenum;
    }
    
    //if too big or too small
    if(!( 0 <= baseint && (leftexprmax-1) >= (baseint+size-1)))
    {
        
        string s;
        if(baseint != baseint+size-1)
        {
            s = "wires access out of bounds. Valid ranges are within 0 to " +to_string(leftexprmax-1)+", recieved range: "+to_string(baseint)+" to "+to_string(baseint+size-1)+".";
        }
        else
        {
             s = "wires access out of bounds. Valid ranges are within 0 to " +to_string(leftexprmax-1)+", recieved bit selection: "+to_string(baseint)+".";
        }
        
        addError(this->nodeLocation->fname,this->nodeLocation->position,s);
        if(hasError())
        {
            printErrors(std::cout);
            std::cout <<"Errors, exiting... \n";
            exit(1);
        }
    }
    
    /*endTimeb = RDTSC;
    cout << "c: "<<(endTimeb - startTimeb)<<"\n";*/
    //startTimeb = RDTSC;
    
    bool isint = false;
    if(leftexpr.var != 0)
    {
        if(leftexpr.var->v_enum == Intv)
        {
            isint=true;
        }
    }
    
    for(int i=0;i<size;i++)
    {
        if(leftexpr.var == 0)
        {
            if(i < leftexpr.vec.size())
            {
                v.vec[i] = leftexpr.vec[i+baseint];
            }
            else
            {
	            v.vec[i] = get_ZERO_WIRE(idxF);
            }
        }
        else
        {
            
            if(Intv)
            {
                if(i < leftexpr.var->size())
                {
                    v.vec[i] = ((IntVariable *)leftexpr.var)->wires[i+baseint+startplace];//getWire(i+baseint+startplace,leftexpr.var->wv);
                }
                else
                {
	                v.vec[i] = get_ZERO_WIRE(idxF);
                }
            }
            else
            {
                if(i < leftexpr.var->size())
                {
                    v.vec[i] = getWire(i+baseint+startplace,leftexpr.var->wv);
                }
                else
                {
	                v.vec[i] = get_ZERO_WIRE(idxF);
                }
            }
        }
    }
    
    unlockCORV(base);
    
    if(wireamount != 0)
    {
        unlockCORV(amt);
        
    }
    
    unlockCORV(leftexpr);
    
    for(int i=0;i<size;i++)
    {
        v.vec[i]->locked = true;
    }
    getPool(idxF)->freeIfNoRefs();
    

    
    
    return v;
}



CORV  ArrayAccessNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{

	CORV leftcorv = left->circuitOutput(vc, tm, idxF);
	CORV arraycorv = arrayexpr->circuitOutput(vc, tm, idxF);
    CORV v;
    
    long place = wiresToLong(&arraycorv, "Unknown values found, indexing on arrays MUST be with known values and must be > 0.",this);
    
    ArrayVariable * avar = (ArrayVariable* )leftcorv.var;

    
    if(place < 0 || place >=  avar->av.size())
    {
        std::stringstream streaml;
        this->print(streaml,0);
        
        string s = "Array index out of range for array access \""+streaml.str()+"\". Length is "+ to_string(avar->av.size()) +" and received index: "+to_string(place)+"";
        
        addError(this->nodeLocation->fname,this->nodeLocation->position,s);
        if(hasError())
        {
            printErrors(std::cout);
            std::cout <<"Errors, exiting... \n";
            exit(1);
        }
    }
    
    Variable * pickedVar = avar->av[place];
    
    v.var = pickedVar;
    
    

   
    /*startTimeb = RDTSC;*/
    
    unlockCORV(leftcorv);
    
    /*endTimeb = RDTSC;
    cout << "a:"<<" "<< (endTimeb - startTimeb)<<"\n";
    
    startTimeb = RDTSC;*/
    
    unlockCORV(arraycorv);
    


    getPool(idxF)->freeIfNoRefs();

    
    
    /*endTimeb = RDTSC;
    cout << "b:"<<" "<< (endTimeb - startTimeb)<<"\n";*/
    
    return v;
}

CORV  DotOperatorNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    //left side must be variable.
    CORV leftcorv = left->circuitOutput(vc,tm, idxF);

    StructVariable * structvar = (StructVariable *)leftcorv.var;
    Variable *item = structvar->map[isTermNode(right)->var];//get map
    
    CORV v;
    v.var = item;
    
    return v;
}
CORV  FunctionCallNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    
	isFunctionCall = true;
    
    string name = isTermNode(function_name)->var;
    FunctionVariable * funcvar = (FunctionVariable *)((*vc)["FUNC_VAR_$$_"+name]);
    
    //|algorithm:
    //--|copy parameters to function paramter slots
    //--|call function
    //--|copy returnv to CORV v and return
    
    vector<CORV> param;
	
	bool isMainFunc = isMainFunction();
	string strDPGlobalInput, strDPLocalInput;
    
    param.resize(funcvar->argsv.size());
    
    if(args != 0)
    {
        FunctionArgListNode * faln = isFunctionArgListNode(args);
        for(int i=0;i<faln->nodeList.size();i++)
        {
	        param[i] = faln->nodeList[i]->circuitOutput(vc, tm, idxF);
        }
        
        //copy param
        for(int i=0;i<param.size();i++)
        {
            CORV * c = &(param[i]);
            Variable * paramvar = funcvar->argsv[i];
            
            
            
            if(c->var == 0)
            {
                
                int j=0;
                for(j=0;j<c->vec.size() && j < paramvar->size();j++)
                {
                    Wire * w = ((IntVariable *)paramvar)->wires[j];
	                strDPGlobalInput.append(to_string(c->vec[j]->wireNumber) + " ");
	                strDPLocalInput.append(to_string(c->vec[j]->prevWireNumber[1]) + " ");
	                assignWire(w, c->vec[j], idxF);
	                makeWireContainValueForFn(w, idxF);	               
	                
                }
                for(;j<paramvar->size();j++)
                {
                    Wire * w = ((IntVariable *)paramvar)->wires[j];
	                
	               strDPGlobalInput.append(to_string(get_ZERO_WIRE(idxF)->wireNumber) + " ");
	               strDPLocalInput.append(to_string(get_ZERO_WIRE(idxF)->prevWireNumber[1]) + " ");
	                assignWire(w, get_ZERO_WIRE(idxF), idxF);
	                makeWireContainValueForFn(w, idxF);
	                
	                
                }
            }
            else
            {
                if(c->var->v_enum == Intv)
                {
                    IntVariable * ivar = (IntVariable *)(c->var);
                    
                    int j=0;
                    for(j=0;j<ivar->size() && j < paramvar->size();j++)
                    {
                        Wire * w = ((IntVariable *)paramvar)->wires[j];
	                    strDPGlobalInput.append(to_string(ivar->wires[j]->wireNumber) + " ");
	                    strDPLocalInput.append(to_string(ivar->wires[j]->prevWireNumber[1]) + " ");
	                    assignWire(w, ivar->wires[j], idxF);
	                    makeWireContainValueForFn(w, idxF);
	                    //appendDuploGC("--" + to_string(w->wireNumber) + " " +  to_string(w->prevWireNumber[0]) + " " + to_string(w->prevWireNumber[1]) + "\n", isMainFunc);
	                
                    }
                    for(;j<paramvar->size();j++)
                    {
                        Wire * w = ((IntVariable *)paramvar)->wires[j];
	                    strDPGlobalInput.append(to_string(get_ZERO_WIRE(idxF)->wireNumber) + " ");
	                    strDPLocalInput.append(to_string(get_ZERO_WIRE(idxF)->prevWireNumber[1]) + " ");
	                    assignWire(w, get_ZERO_WIRE(idxF), idxF);
	                    makeWireContainValueForFn(w, idxF);	                   
	                
                    }
                }
                else
                {
                    Variable * anyvar = c->var;
                    
                    int startanyvar = anyvar->wv->startwirenum;
                    int startparam = paramvar->wv->startwirenum;
                    
                    for(int j=0;j<anyvar->size();j++)
                    {
                        Wire * w = getWire(j+startparam,paramvar->wv);
	                   
	                    strDPGlobalInput.append(to_string(getWire(j + startanyvar, anyvar->wv)->wireNumber) + " ");
	                    strDPLocalInput.append(to_string(getWire(j + startanyvar, anyvar->wv)->prevWireNumber[1]) + " ");
	                    
	                    assignWire(w, getWire(j + startanyvar, anyvar->wv), idxF);
	                    makeWireContainValueForFn(w, idxF);	 
	                
                    }
                }
            }
            
            unlockCORV(*c);
        }
    }

    getPool(idxF)->freeIfNoRefs();
    
    if(funcvar->functionNode->gatesFromNode != 0 || funcvar->functionNode->gatesFromNodeXor != 0)
    {
	   // appendDuploGC("--"+to_string(funcvar->getVarDepth())+"\n", true);
       outputFunctionCall(funcvar->functionNumber);
	    outputFunctionCallDP(funcvar->functionNumber, strDPLocalInput, strDPGlobalInput);
    }
    
    
    CORV v;
    
    //put results intocorv
    if(funcvar->returnv !=0)
    {
        //get or create variable and add to scope
        Variable * rvar=0;
        int counter=0;
        
        while(true)
        {
            rvar = (*vc)[to_string(counter)+"-+r+"+funcvar->name];

            //cout << "checking: "<<to_string(counter)+"-+r+"+funcvar->name<<"\n";
            
            if(rvar == 0)
            {
                //create
                
                Type * tt = funcvar->returnv->type;
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
                v->FillInType(true);
	            v->FillInWires(0, 0, idxF);
                setSizes(v->wv,0);
                v->FillInDepth(99999);
                
                v->setFunctionCallVar();
                
                for(int j=0;j<v->size();j++)
                {
                    getWire(j,v->wv)->varlocked = true;
                }

                rvar = v;

                
                (*vc)[to_string(counter)+"-+r+"+funcvar->name] = rvar;
                
                //cout << "creating var\n";
                
                break;
            }
            else if(!(getWire(0,rvar->wv)->locked))
            {
                
                break;
            }
            
            
            counter++;
        }
        
	    messyAssignAndCopyForFn(funcvar->returnv, rvar, idxF);
        
        v.var = rvar;
    }

    
    lockCORVFuncCall(v);

    
    //adding gates for this function.
    FunctionDeclarationNode * n = isFunctionDeclarationNode(funcvar->functionNode);
    if(n->stmts != 0)
    {
        incrementCountsBy(n->stmts->gatesFromNode, n->stmts->gatesFromNodeXor);
    }
    return v;
}
CORV  FunctionArgListNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    cout << "unusednode: "<<nodeName<<"\n";
    CORV v;
    return v;
}

CORV  ArrayInitListNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    
    ArrayVariable * av = new ArrayVariable();
    
    av->istemp = true;
    
    for(int i=0;i<nodeList.size();i++)
    {
        
        CORV c = nodeList[i]->circuitOutput(vc,tm, idxF);
        
        if(c.var == 0)
        {
            IntVariable * iv = new IntVariable();
            iv->istemp = true;
            iv->wires.resize(c.vec.size());
            for(int j=0;j<c.vec.size();j++)
            {
                iv->wires[j] = c.vec[j];
            }
            av->av.push_back(iv);
        }
        else
        {
            av->av.push_back(c.var);
        }
    }
    

    CORV v;
    
    v.var = av;
    
    return v;
}
CORV  ArrayInitNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
	CORV v = operand->circuitOutput(vc, tm, idxF);
    return v;
}

CORV  StatementListNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    for(int i=0;i<nodeList.size();i++)
    {
	    nodeList[i]->circuitOutput(vc, tm, idxF);
    }
    
    CORV v;
    return v;
}

CORV  VarDeclarationNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{

    VarDeclarationNode * vdn = isVarDeclarationNode(this);
    
    Type * vartype = (*tm)[isTermNode(vdn->type)->var];
    DeclarationCommaListNode * vars = isDeclarationCommaListNode(vdn->each_var);

    for(int i=0;i < vars->nodeList.size();i++)
    {
        VarDeclarationPrimNode * dn = isVarDeclarationPrimNode(vars->nodeList[i]);
        vartype = (*tm)[isTermNode(vdn->type)->var];

        Variable * v=0;
        
        if((*vc)[isTermNode(dn->name)->var] != 0)
        {
            if((*vc)[isTermNode(dn->name)->var]->type == vartype)
            {
                Variable * vinside = (*vc)[isTermNode(dn->name)->var];
                
                for(int j=0;j<vinside->size();j++)
                {
                    Wire * w = getWire(j,vinside->wv);
                    if(w->other != 0)
                    {
                        w->other->refs--; w->other->addRef(w);
                    }
                    w->state = ZERO;
                    w->other = 0;
                }
                
                v = vinside;
            }
            else
            {
                
                /*cout << (*vc)[isTermNode(dn->name)->var]->type<< " "<<vartype<<"\n";
                
                (*vc)[isTermNode(dn->name)->var]->type->print(cout);
                cout <<"\n";
                vartype->print(cout);
                cout <<"\n";
                
                Variable * vinside = (*vc)[isTermNode(dn->name)->var];
                
                for(int j=0;j<vinside->size();j++)
                {
                    Wire * w = getWire(j,vinside->wv);
                    if(w->other != 0)
                    {
                        w->other->refs--; w->other->addRef(w);
                    }
                    w->state = ZERO;
                    w->other = 0;
                }
                
                v = vinside;*/
                
                cout << "Variable is redeclared with different type and not implemented. grep in code\n";
            }
        }
        else
        {
            string suffix="";
            if(dn->array_dec != 0)
            {
                vector<long> sizes;
                
                sizes.resize(isArrayDeclarationListNode(dn->array_dec)->nodeList.size());
                
                for(int j=0;j<isArrayDeclarationListNode(dn->array_dec)->nodeList.size();j++)
                {
                    sizes[j] = isArrayDeclarationListNode(dn->array_dec)->nodeList[j]->getLongResultNoVar();
                    suffix +="["+to_string(sizes[j]);
                }
                
                Type * newt = (*tm)[isTermNode(vdn->type)->var+suffix];
                if(newt == 0)
                {
                    newt = new ArrayType(vartype->typeName);
                    for(int L=0;L<sizes.size();L++)
                    {
                        ((ArrayType *) (newt))->sizes.push_back(sizes[L]);
                    }
                    ((ArrayType *) (newt))->type = (*tm)[isTermNode(vdn->type)->var];
                }
                vartype = newt;
            }

            
            
            
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
            
            v->name = isTermNode(dn->name)->var;
            v->type = vartype;
            v->nodeOfName = dn->name;
            
            v->FillInType(true);
	        v->FillInWires(0, 0, idxF);
            
            (*vc)[v->name] = v;
            setSizes(v->wv,0);

            for(int j=0;j<v->size();j++)
            {
                getWire(j,v->wv)->varlocked = true;
                getWire(j,v->wv)->locked = false;
            }
            
            v->FillInDepth(getDepth());
        }
        
        if(dn->init_expression != 0)
        {
            
       
            
            CORV rv = dn->init_expression->circuitOutput(vc,tm, idxF);
	     
	        
	        if (dn->init_expression->nodeName == "FunctionCallNode")
	        {
		        string strDPGlobalOut= messyAssignAndCopyForFn(rv, v, idxF);
		        if (isMainFunction())
			        appendDuploGC(strDPGlobalOut + "\n\n", true); //"Global OUT: " +     	
		        else
			        appendDuploGC("++ " + strDPGlobalOut + "\n", true); //"Global OUT: " +
	        }
	        else
	        {
		        messyAssignAndCopy(rv, v, idxF);
	        }

            if(rv.var != 0)
            {
                messyUnlock(rv.var);
            }
            else
            {
                for(int k=0;k<rv.vec.size();k++)
                {
                    rv.vec[k]->locked = false;
                }
            }
            
            if(rv.var != 0 && rv.var->istemp==true)
            {
                rv.var->deleteTemps();
                delete rv.var;
                rv.var = 0;
            }
        }
    }

    CORV v;
    return v;
}



CORV  CompoundStatementNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    increaseDepth();
    
    if(isproc && timesgenerated == 1)
    {
        procnumber = getNumberOfFunctions();
        increaseFunctions();
        
        //add output file from stack
        pushOutputFile(getFunctionPrefix()+to_string(procnumber)+".ffrig");
        procxorgates = getXorGates();
        procnonxorgates = getNonXorGates();
    }
    
    if(stmt != 0 && (!isproc || (timesgenerated < 2)) )
    {
        
        
        beginCircuitCount(getNonXorGates(),getXorGates());
        
        VariableContext * vct;
        if(vctt == 0)
        {
            vct = new VariableContext();
            vctt = vct;
        }
        else
        {
            vct =vctt;
        }
        
        
        int sizex=0;
        
        for ( std::unordered_map<string,Variable *>::iterator it = vc->begin(); it!= vc->end(); ++it )
        {
            sizex++;
            
            if((*vc)[it->first] != 0 && !((*vc)[it->first]->isconst))
                (*vct)[it->first] = it->second;
        }
        

        
	    stmt->circuitOutput(vct, tm, idxF);
        
        

        
        
        
        /* unlock variables that only exist in the deleted scope */
        for ( std::unordered_map<string,Variable *>::iterator it = vc->begin(); it!= vc->end(); ++it )
        {
            if( (*vct)[it->first] == it->second )
            {
                (*vct)[it->first] = 0;
            }
        }
        for ( std::unordered_map<string,Variable *>::iterator it = vct->begin(); it!= vct->end(); ++it )
        {
            if((*vct)[it->first] != 0)
            {
                Variable * v = (*vct)[it->first];
                WireSet * ws = v->wireset;
                
                if(ws != 0)
                {
                    Wire ** wires = (ws->wires);
                    int size = v->size();
                    for(int i=0;i<size;i++)
                    {
                        wires[i]->varlocked = false;
                        wires[i]->locked = false;
                    }
                }
                else
                {
                    for(int i=0;i<v->size();i++)
                    {
                        Wire * w = getWire(i,v->wv);
                        w->varlocked = false; //note, if the refs still point to this wire, it won't be removed because refs > 0
                        w->locked = false; //shouldn't be locked, but just in case
                    }
                }
                
                (*vct)[it->first] = 0;
            }
        }
        
        
        getPool(idxF)->freeIfNoRefs();
        

        
        //delete vct;
        
        if(!(timesgenerated > 0 && isproc))
        {
            endCircuitCount(getNonXorGates(),getXorGates());
           // cout << "addc1\n";
        }

        
        
    }
    else
    {
        //so if this is not output
        if(timesgenerated == 0)
        {
            gatesFromNodeXor+=procxorgates;
            gatesFromNode+=procnonxorgates;
            incrementCountsBy(procnonxorgates,procxorgates);
        }
        
    }
    
    if(isproc && timesgenerated == 1)
    {
        //remove output file from stack
        popOutputFile();
        procxorgates = getXorGates() - procxorgates;
        procnonxorgates = getNonXorGates() - procnonxorgates;
    }
    
    if(isproc && timesgenerated > 0 && (procxorgates != 0 || procnonxorgates != 0))
    {
        //add function call
        outputFunctionCall(procnumber);
        if(timesgenerated > 1)
            incrementCountsBy(procnonxorgates,procxorgates);
    }
    
    
    decreaseDepth();
    timesgenerated++;
    
    CORV v;
    return v;
}

CORV  UnaryPostPlusPlusNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{

	CORV leftcorv = operand->circuitOutput(vc, tm, idxF);
    
    vector<Wire *> onevec;
	onevec.push_back(get_ONE_WIRE(idxF));
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv = &onevec;
    
    if(leftcorv.var != 0)
    {
        ensureIntVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        //leftv  = &(((IntVariable *)leftcorv.var)->wires);
	    ensureSize(onevec, ((IntVariable *)leftcorv.var)->size(), idxF);
    }
    else
    {
        /*this should not be necessary as variable is required, but just in case -> better to be safe than sorry*/
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
	    ensureSameSize(*leftv, *rightv, idxF);
    }
    
    CORV destv;
    destv.vec.resize(leftv->size());
    
    /*outputting the circuit*/
	outputAddition(leftv, rightv, destv.vec, idxF);
    
    
    //force leftcorv to be in vec
    ensureIntVariableToVec(leftcorv);
    
    IntVariable * destvariable = (IntVariable *) (leftcorv.var);
    leftcorv.var = 0; // i.e. only the vector is passed up, not the variable.
    
    
    WireSet * ws = getPool(idxF)->getWires(leftcorv.vec.size());
    
    
    //have to assign since the vec wires hold references to the variable wires
    for(int i=0;i<leftcorv.vec.size();i++)
    {
        Wire * w = ws->wires[i];
        
	    assignWire(w, leftcorv.vec[i], idxF);
        leftcorv.vec[i] = w;
        w->locked = true;
	    makeWireContainValueNoONEZEROcopy(leftcorv.vec[i], idxF);
        
    }
    
    for(int i=0;i<destv.vec.size();i++)
    {
        destv.vec[i]->locked = true;
    }

    /*intermedate cleanup to remove unnecessary refs that would break the assignWire function*/
    
    getPool(idxF)->freeIfNoRefs();
    
    Variable * var_x = (*vc)["-+IFCOND+-"];
    
    Variable * leftvar = destvariable;

    
    if(var_x == 0 || (var_x->getVarDepth() < leftvar->getVarDepth()))
    {
        //assign destv to variable (i.e. NOT THE LEFT CORV)
        for(int i=0;i<destv.vec.size();i++)
        {
            
	        assignWire(destvariable->wires[i], destv.vec[i], idxF);
            destv.vec[i]->locked = false;
	        makeWireContainValueNoONEZEROcopy(destvariable->wires[i], idxF);
        }
    }
    else
    {
        Wire * cond = ((IntVariable *)var_x)->wires[0];
        
        //assign destv to variable (i.e. NOT THE LEFT CORV)
        for(int i=0;i<destv.vec.size();i++)
        {
            
	        assignWireCond(destvariable->wires[i], destv.vec[i], cond, idxF);
            destv.vec[i]->locked = false;
	        makeWireContainValueNoONEZEROcopy(destvariable->wires[i], idxF);
        }
    }
    
    /*cleanup*/
    getPool(idxF)->freeIfNoRefs();
    
    return leftcorv;
}



CORV  UnaryPreMinusMinusNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
	CORV leftcorv = operand->circuitOutput(vc, tm, idxF);
    
    vector<Wire *> onevec;
	onevec.push_back(get_ONE_WIRE(idxF));
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv = &onevec;
    
    if(leftcorv.var != 0)
    {
        ensureIntVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        //leftv  = &(((IntVariable *)leftcorv.var)->wires);
	    ensureSize(onevec, ((IntVariable *)leftcorv.var)->size(), idxF);
    }
    else
    {
        /*this should not be necessary as variable is required, but just in case -> better to be safe than sorry*/
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
	    ensureSameSize(*leftv, *rightv, idxF);
    }
    
    CORV destv;
    destv.vec.resize(leftv->size());
    
    /*outputting the circuit*/
	outputSubtract(leftv, rightv, destv.vec, idxF);
    
    
    //force leftcorv to be in vec
    ensureIntVariableToVec(leftcorv);
    
    IntVariable * destvariable = (IntVariable *) (leftcorv.var);
    leftcorv.vec.resize(0);
    
    
    for(int i=0;i<destv.vec.size();i++)
    {
        destv.vec[i]->locked = true;
    }
    
    /*intermedate cleanup to remove unnecessary refs that would break the assignWire function*/
    
    getPool(idxF)->freeIfNoRefs();
    
    Variable * var_x = (*vc)["-+IFCOND+-"];
    
    Variable * leftvar = destvariable;
    
    
    if(var_x == 0 || (var_x->getVarDepth() < leftvar->getVarDepth()))
    {
        //assign destv to variable (i.e. NOT THE LEFT CORV)
        for(int i=0;i<destv.vec.size();i++)
        {
            
	        assignWire(destvariable->wires[i], destv.vec[i], idxF);
            destv.vec[i]->locked = false;
	        makeWireContainValueNoONEZEROcopy(destvariable->wires[i], idxF);
        }
    }
    else
    {
        Wire * cond = ((IntVariable *)var_x)->wires[0];
        
        //assign destv to variable (i.e. NOT THE LEFT CORV)
        for(int i=0;i<destv.vec.size();i++)
        {
            
	        assignWireCond(destvariable->wires[i], destv.vec[i], cond, idxF);
            destv.vec[i]->locked = false;
	        makeWireContainValueNoONEZEROcopy(destvariable->wires[i], idxF);
        }
    }
    
    /*cleanup*/
    getPool(idxF)->freeIfNoRefs();
    
    return leftcorv;
}

CORV  ConditionalNotEqualNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    /*preparation*/
    CORV leftcorv = left->circuitOutput(vc,tm, idxF);
    CORV rightcorv = right->circuitOutput(vc,tm, idxF);
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv;
    
    {
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        ensureAnyVariableToVec(rightcorv);
        rightv = &(rightcorv.vec);
        //ensureSameSize(*leftv,*rightv);
	    ensureTypedSize(*leftv, *rightv, operandTypeOfNode, idxF);
    }
    
    CORV destv;
    destv.vec.resize(1);
    
    /*outputting the circuit*/
	outputEquals(leftv, rightv, destv.vec, idxF);
	destv.vec[0] = invertWire(destv.vec[0], idxF);
    
    /*cleanup*/
    
    int size =leftv->size();
    for(int i=0;i<size;i++)
    {
        leftv->operator[](i)->locked = false;
        rightv->operator[](i)->locked = false;
    }
    size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    return destv;
}
CORV  ExpressionStatementNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    if(operand != 0)
    {
        CORV t = operand->circuitOutput(vc,tm, idxF);
        
        if(t.var == 0)
        {
            vector<Wire *> * leftv = &(t.vec);
            
            for(int i=0;i<leftv->size();i++)
            {
                leftv->operator[](i)->locked = false;
            }
        }
        else
        {
            int startplace = t.var->wv->startwirenum;
            
            for(int i=startplace;i<t.var->size()+startplace;i++)
            {
                getWire(i,t.var->wv)->locked = false;
            }
        }

        getPool(idxF)->freeIfNoRefs();
    }
    CORV v;
    return v;
}
CORV  FunctionDeclarationNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{

    increaseDepth();
    
    beginCircuitCount(getNonXorGates(),getXorGates());
    
    FunctionDeclarationNode * fdn = isFunctionDeclarationNode(this);
    VariableContext * vct = new VariableContext();
    
    FunctionVariable * funcvar = (FunctionVariable *) ((*vc)["FUNC_VAR_$$_"+isTermNode(fdn->name)->var]);

    for ( std::unordered_map<string,Variable *>::iterator it = vc->begin(); it!= vc->end(); ++it )
    {
        (*vct)[it->first] = it->second;
    }

    if(fdn->param != 0)
    {
        DeclarationArgCommaListNode * dacl = isDeclarationArgCommaListNode(fdn->param);
        for(int k=0;k<dacl->nodeList.size();k++)
        {
            FunctionVarDeclarationNode * fvdn = isFunctionVarDeclarationNode(dacl->nodeList[k]);
            
            Variable * v_ = funcvar->argsv[k];
            v_->nodeOfName = fvdn->varname;
            (*vct)[isTermNode(fvdn->varname)->var] = v_;

            int startplace  = v_->wv->startwirenum;
            v_->FillInDepth(getDepth());
        }
    }
    

    Type * thisfunctype = isFunctionType((*tm)[isTermNode(fdn->name)->var])->type;
    
    if(funcvar->returnv != 0)
    {
        Variable * var = funcvar->returnv;
        var->name = "-+RETURNTYPE+-";
        (*vct)[var->name] = var;
    }
    else
    {
        Variable * var = new Variable();
        var->name = "-+RETURNTYPE+-";
        var->type = isFunctionType((*tm)[isTermNode(fdn->name)->var])->type;
        (*vct)[var->name] = var;
    }
    

    if(fdn->stmts != 0)
    {
	    fdn->stmts->circuitOutput(vct, tm, idxF);
    }

    
    if(funcvar->returnv != 0)
    {
        Variable * v_ = funcvar->returnv;
        
        for(int i=0; i <v_->size();i++)
        {
	        makeWireNotOther(getWire(i, funcvar->return_var), idxF);
        }
    }
    
    //free other variables
    for ( std::unordered_map<string,Variable *>::iterator it = vc->begin(); it!= vc->end(); ++it )
    {
        if( (*vct)[it->first] == it->second )
        {
            (*vct)[it->first] = 0;
        }
    }
    for ( std::unordered_map<string,Variable *>::iterator it = vct->begin(); it!= vct->end(); ++it )
    {
        if((*vct)[it->first] != 0)
        {
            Variable * v = (*vct)[it->first];
            
            if(v->v_enum == Functionp)
                continue;
            
            int start=0;
            if(v->wv != 0)
            {
                start = v->wv->startwirenum;
            }
            for(int i=start;i<v->size();i++)
            {
                getWire(i,v->wv)->varlocked = false;
                getWire(i,v->wv)->locked = false;
            }
        }
    }
	getPool(idxF)->freeIfNoRefs();
    delete vct;
    
    endCircuitCount(getNonXorGates(),getXorGates());
    
    decreaseDepth();

    CORV v;
    return v;
}
CORV  UnaryPostMinusMinusNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    CORV leftcorv = operand->circuitOutput(vc,tm, idxF);
    
    vector<Wire *> onevec;
	onevec.push_back(get_ONE_WIRE(idxF));
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv = &onevec;
    
    if(leftcorv.var != 0)
    {
        ensureIntVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        //leftv  = &(((IntVariable *)leftcorv.var)->wires);
	    ensureSize(onevec, ((IntVariable *)leftcorv.var)->size(), idxF);
    }
    else
    {
        /*this should not be necessary as variable is required, but just in case -> better to be safe than sorry*/
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
	    ensureSameSize(*leftv, *rightv, idxF);
    }
    
    CORV destv;
    destv.vec.resize(leftv->size());
    
    /*outputting the circuit*/
	outputSubtract(leftv, rightv, destv.vec, idxF);
    
    
    //force leftcorv to be in vec
    ensureIntVariableToVec(leftcorv);
    
    IntVariable * destvariable = (IntVariable *) (leftcorv.var);
    leftcorv.var = 0; // i.e. only the vector is passed up, not the variable.
    
    WireSet * ws = getPool(idxF)->getWires(leftcorv.vec.size());
    
    //have to assign since the vec wires hold references to the variable wires
    for(int i=0;i<leftcorv.vec.size();i++)
    {
        Wire * w = ws->wires[i];
        
	    assignWire(w, leftcorv.vec[i], idxF);
        leftcorv.vec[i] = w;
        w->locked = true;
	    makeWireContainValueNoONEZEROcopy(leftcorv.vec[i], idxF);
        
    }
    
    for(int i=0;i<destv.vec.size();i++)
    {
        destv.vec[i]->locked = true;
    }
    
    /*intermedate cleanup to remove unnecessary refs that would break the assignWire function*/
    
    getPool(idxF)->freeIfNoRefs();
    
    Variable * var_x = (*vc)["-+IFCOND+-"];
    
    Variable * leftvar = destvariable;
    
    
    if(var_x == 0 || (var_x->getVarDepth() < leftvar->getVarDepth()))
    {
        //assign destv to variable (i.e. NOT THE LEFT CORV)
        for(int i=0;i<destv.vec.size();i++)
        {
            
	        assignWire(destvariable->wires[i], destv.vec[i], idxF);
            destv.vec[i]->locked = false;
	        makeWireContainValueNoONEZEROcopy(destvariable->wires[i], idxF);
        }
    }
    else
    {
        Wire * cond = ((IntVariable *)var_x)->wires[0];
        
        //assign destv to variable (i.e. NOT THE LEFT CORV)
        for(int i=0;i<destv.vec.size();i++)
        {
            
	        assignWireCond(destvariable->wires[i], destv.vec[i], cond, idxF);
            destv.vec[i]->locked = false;
	        makeWireContainValueNoONEZEROcopy(destvariable->wires[i], idxF);
        }
    }
    
    /*cleanup*/
    getPool(idxF)->freeIfNoRefs();
    
    return leftcorv;

}

CORV ConditionalLessEqualNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    /*preparation*/
    CORV leftcorv = left->circuitOutput(vc,tm, idxF);
    CORV rightcorv = right->circuitOutput(vc,tm, idxF);
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv;
    
    {
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        ensureAnyVariableToVec(rightcorv);
        rightv = &(rightcorv.vec);
        //ensureSameSize(*leftv,*rightv);
	    ensureTypedSize(*leftv, *rightv, operandTypeOfNode, idxF);
    }
    
    CORV destv;
    destv.vec.resize(1);
    
    /*outputting the circuit*/
    /*notice the parameter reversal for a > operation*/
    if(isSigned)
    {
	    outputLessThanSigned(rightv, leftv, destv.vec, idxF);
    }
    else
    {
	    outputLessThanUnsigned(rightv, leftv, destv.vec, idxF);
    }
    
    
    /*then not the greater than result*/
	destv.vec[0] = invertWire(destv.vec[0], idxF);
    
    /*cleanup*/
    int size =leftv->size();
    for(int i=0;i<size;i++)
    {
        leftv->operator[](i)->locked = false;
        rightv->operator[](i)->locked = false;
    }
    size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    return destv;

}


CORV  ConditionalGreaterEqualNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    /*preparation*/
    CORV leftcorv = left->circuitOutput(vc,tm, idxF);
    CORV rightcorv = right->circuitOutput(vc,tm, idxF);
    
    vector<Wire *> * leftv;
    vector<Wire *> * rightv;
    
    {
        ensureAnyVariableToVec(leftcorv);
        leftv = &(leftcorv.vec);
        ensureAnyVariableToVec(rightcorv);
        rightv = &(rightcorv.vec);
        //ensureSameSize(*leftv,*rightv);
	    ensureTypedSize(*leftv, *rightv, operandTypeOfNode, idxF);
    }
    
    CORV destv;
    destv.vec.resize(1);
    
    /*outputting the circuit*/
    if(isSigned)
    {
	    outputLessThanSigned(leftv, rightv, destv.vec, idxF);
    }
    else
    {
	    outputLessThanUnsigned(leftv, rightv, destv.vec, idxF);
    }
    /*then not the less than result*/
	destv.vec[0] = invertWire(destv.vec[0], idxF);
    
    /*cleanup*/
    int size =leftv->size();
    for(int i=0;i<size;i++)
    {
        leftv->operator[](i)->locked = false;
        rightv->operator[](i)->locked = false;
    }
    size = destv.vec.size();
    for(int i=0;i<size;i++)
    {
        destv.vec[i]->locked = true;
    }
    
    getPool(idxF)->freeIfNoRefs();
    return destv;
}

CORV  IfNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    
    increaseDepth();
    
    CORV condcorv = cond->circuitOutput(vc,tm, idxF);
    
    Wire * condw;
    
    if(condcorv.var == 0)
    {
        condw = condcorv.vec[0];
    }
    else
    {
        condw = ((IntVariable *)(condcorv.var))->wires[0];
    }
        
    if(condw->state == ONE)
    {
        if(stmts!= 0){ stmts->circuitOutput(vc, tm, idxF); }
    }
    else if(condw->state == ZERO)
    {
        if(elsestmts!= 0){ elsestmts->circuitOutput(vc, tm, idxF); }
    }
    else
    {
        Variable * var_x = (*vc)["-+IFCOND+-"];
        
        IntVariable var;
        Wire * ififcond=0;
        Wire * prevcond=0;
        if(var_x != 0)
        {
            prevcond = ((IntVariable *)var_x)->wires[0];
            
	        ififcond = outputGate(8, prevcond, condw, idxF);
            ififcond->locked = true;
            ififcond->varlocked = true;
            var.wires.resize(1);
            var.wires[0] = ififcond;
        }
        else
        {
            var.wires.resize(1);
            var.wires[0] = condw;
            condw->locked = true;
        }
        var.FillInDepth(getDepth());
        
        (*vc)["-+IFCOND+-"] = &var;
        
        increaseDepth();
        
        if(stmts != 0)
        {
	        stmts->circuitOutput(vc, tm, idxF);
        }
        if(elsestmts!= 0)
        {
            condw->locked = false;
	        condw = invertWire(condw, idxF);
            condw->locked = true;
            if(var_x == 0)
            {
                var.wires[0] = condw;
            }
            
            if(var_x != 0)
            {
                ififcond->locked = false;
                ififcond->varlocked = false;
	            ififcond = outputGate(8, prevcond, condw, idxF);
                ififcond->locked = true;
                ififcond->varlocked = true;
                var.wires[0] = ififcond;
            }
	        elsestmts->circuitOutput(vc, tm, idxF);
        }
        
        if(ififcond != 0)
        {
            ififcond->locked = false;
            ififcond->varlocked = false;
        }
        
        decreaseDepth();
        
        condw->locked = false;
        (*vc)["-+IFCOND+-"] = var_x;
    }
    
    unlockCORV(condcorv);
    getPool(idxF)->freeIfNoRefs();
    
    decreaseDepth();
    
    CORV v;
    return v;
}
CORV  ForNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    

    
    //add variable declaration
    
    increaseDepth();
    
    VariableContext * vct = new VariableContext();
    
    for ( std::unordered_map<string,Variable *>::iterator it = vc->begin(); it!= vc->end(); ++it )
    {
        if((*vc)[it->first] != 0)
            (*vct)[it->first] = it->second;
    }
    
    VarDeclarationForNode * vdfn = isVarDeclarationForNode(declaration);
    VarDeclarationNode * temp = new VarDeclarationNode(vdfn->type,vdfn->each_var);
    
	temp->circuitOutput(vct, tm, idxF);
    
    
    
    
	CORV condcorv = condition->circuitOutput(vct, tm, idxF);
    
    Wire * cond;
    if(condcorv.var == 0)
    {
        cond = condcorv.vec[0];
    }
    else
    {
        cond = ((IntVariable *)(condcorv.var))->wires[0];
    }
    
    if(cond->state != ZERO && cond->state != ONE)
    {
        addError(this->nodeLocation->fname,this->nodeLocation->position,"Conditional Expression in for loop cannot be based on input values when it is check!");
        
        
        
        if(hasError())
        {
            printErrors(std::cout);
            std::cout <<"Errors, exiting... \n";
            exit(1);
        }
    }
    
    
    
    while(cond->state == ONE )
    {
        if(has_procs){ getPool(idxF)->sortFreeWires(); /*cout << "sorting\n"; getPool(idxF)->printUsedPoolState();*/ }
        
	    CORV temp = stmts->circuitOutput(vct, tm, idxF);
        
	    temp = increment->circuitOutput(vct, tm, idxF);
        unlockCORV(temp);
        
        unlockCORV(condcorv);
	    condcorv = condition->circuitOutput(vct, tm, idxF);
        
        if(condcorv.var == 0)
        {
            cond = condcorv.vec[0];
        }
        else
        {
            cond = ((IntVariable *)(condcorv.var))->wires[0];
        }
        
        if(cond->state != ZERO && cond->state != ONE)
        {
            addError(this->nodeLocation->fname,this->nodeLocation->position,"Conditional Expression in for loop cannot be based on input values when it is check!");
            
            getPool(idxF)->printUsedPoolState();
            
            if(hasError())
            {
                printErrors(std::cout);
                std::cout <<"Errors, exiting... \n";
                exit(1);
            }
        }
        
    }
    unlockCORV(condcorv);
    
    
    
    
    for ( std::unordered_map<string,Variable *>::iterator it = vc->begin(); it!= vc->end(); ++it )
    {
        if( (*vct)[it->first] == it->second )
        {
            (*vct)[it->first] = 0;
        }
    }
    for ( std::unordered_map<string,Variable *>::iterator it = vct->begin(); it!= vct->end(); ++it )
    {
        if((*vct)[it->first] != 0)
        {
            Variable * v = (*vct)[it->first];
            for(int i=0;i<v->size();i++)
            {
                getWire(i,v->wv)->varlocked = false; //note, if the refs still point to this wire, it won't be removed because refs > 0
                getWire(i,v->wv)->locked = false; //shouldn't be locked, but just in case
            }
        }
    }
    getPool(idxF)->freeIfNoRefs();
    
    
    decreaseDepth();
    
    delete vct;
    
    CORV v;
    return v;
}

IntVariable * createIntOfSizeNOSETNAME(int size)
{
    
    long length = size;
    
    if(getTypeMap()["-+generate_const_type+-"+to_string(length)] == 0)
    {
        getTypeMap()["-+generate_const_type+-"+to_string(length)] = new IntType("-+generate_const_type+-"+to_string(length),length);
    }
    
    Type * t = getTypeMap()["-+generate_const_type+-"+to_string(length)];
    
    IntVariable * v = new IntVariable();
    v->type = t;
    
    v->FillInType(false);
    setSizes(v->wv,0);
    
    return v;
}

CORV  TermNode::circuitOutput(VariableContext * vc, TypeMap * tm, int idxF)
{
    if(isNum)
    {
        
        if((*vc)["NUM_VAR_$$_"+num+"_"+to_string(bitwidth)] == 0)
        {
            long number = stol(num);
            
            int bit = 63;
            while(bit > 0)
            {
                if((number >> bit)> 0)
                    break;
                
                bit--;
            }
            
            bit++;
            bit++;
            
            if(bit <= 0)
            {
                bit = 1;
            }
            
            if(bitwidth > 0)
            {
                bit = bitwidth;
            }
           
            
            
            IntVariable * vtemp = createIntOfSizeNOSETNAME(bit);
            vtemp->isconst = true;
            vtemp->name ="NUM_VAR_$$_"+num+"_"+to_string(bitwidth);
            vtemp->wires.resize(bit);
            for(int i=0;i<bit;i++)
            {
                vtemp->wires[i] = new Wire();
                if(((number >> i) & 1) == 1)
                {
                    vtemp->wires[i]->state = ONE;
                }
                else
                {
                    vtemp->wires[i]->state = ZERO;
                }
            }
            
            (*vc)["NUM_VAR_$$_"+num+"_"+to_string(bitwidth)] = vtemp;
            
            
        }

        Variable * v =  (*vc)["NUM_VAR_$$_"+num+"_"+to_string(bitwidth)];
        CORV corv;
        corv.var = v;
        return corv;
    }
    
    Variable * var_x = (*vc)[var];

    
    if(var_x == 0)
    {
        cout << var<< " var is null in termnode circuit output\n";
    }
    
    CORV v;
    v.var = var_x;
    return v;
}





Node * isNodeNode(Node *n)
{
    if(n->getNodeType()==Node_t)
    {
        return (Node *)n;
    }
    return 0;
}

BitwiseORNode * isBitwiseORNode(Node *n)
{
    if(n->getNodeType()==BitwiseORNode_t)
    {
        return (BitwiseORNode *)n;
    }
    return 0;
}

BitwiseANDNode * isBitwiseANDNode(Node *n)
{
    if(n->getNodeType()==BitwiseANDNode_t)
    {
        return (BitwiseANDNode *)n;
    }
    return 0;
}

BitwiseXORNode * isBitwiseXORNode(Node *n)
{
    if(n->getNodeType()==BitwiseXORNode_t)
    {
        return (BitwiseXORNode *)n;
    }
    return 0;
}

/*
ConditionalANDNode * isConditionalANDNode(Node *n)
{
    if(n->getNodeType()==ConditionalANDNode_t)
    {
        return (ConditionalANDNode *)n;
    }
    return 0;
}

ConditionalORNode * isConditionalORNode(Node *n)
{
    if(n->getNodeType()==ConditionalORNode_t)
    {
        return (ConditionalORNode *)n;
    }
    return 0;
}
 */

ArrayDecNode * isArrayDecNode(Node *n)
{
    if(n->getNodeType()==ArrayDecNode_t)
    {
        return (ArrayDecNode *)n;
    }
    return 0;
}

ArrayInitNode * isArrayInitNode(Node *n)
{
    if(n->getNodeType()==ArrayInitNode_t)
    {
        return (ArrayInitNode *)n;
    }
    return 0;
}

UnaryNOTNode * isUnaryNOTNode_t(Node *n)
{
    if(n->getNodeType()==UnaryNOTNode_t)
    {
        return (UnaryNOTNode *)n;
    }
    return 0;
}

UnaryMinusNode * isUnaryMinusNode(Node *n)
{
    if(n->getNodeType()==UnaryMinusNode_t)
    {
        return (UnaryMinusNode *)n;
    }
    return 0;
}

UnaryPostMinusMinusNode * isUnaryPostMinusMinusNode(Node *n)
{
    if(n->getNodeType()==UnaryPostMinusMinusNode_t)
    {
        return (UnaryPostMinusMinusNode *)n;
    }
    return 0;
}

UnaryPostPlusPlusNode * isUnaryPostPlusPlusNode(Node *n)
{
    if(n->getNodeType()==UnaryPostPlusPlusNode_t)
    {
        return (UnaryPostPlusPlusNode *)n;
    }
    return 0;
}

UnaryPrePlusPlusNode * isUnaryPrePlusPlusNode(Node *n)
{
    if(n->getNodeType()==UnaryPrePlusPlusNode_t)
    {
        return (UnaryPrePlusPlusNode *)n;
    }
    return 0;
}

UnaryPreMinusMinusNode * isUnaryPreMinusMinusNode(Node *n)
{
    if(n->getNodeType()==UnaryPreMinusMinusNode_t)
    {
        return (UnaryPreMinusMinusNode *)n;
    }
    return 0;
}

ArithModuloNode * isArithModuloNode(Node *n)
{
    if(n->getNodeType()==ArithModuloNode_t)
    {
        return (ArithModuloNode *)n;
    }
    return 0;
}

ArithPlusNode * isArithPlusNode(Node *n)
{
    if(n->getNodeType()==ArithPlusNode_t)
    {
        return (ArithPlusNode *)n;
    }
    return 0;
}

ArithMinusNode * isArithMinusNode(Node *n)
{
    if(n->getNodeType()==ArithMinusNode_t)
    {
        return (ArithMinusNode *)n;
    }
    return 0;
}

ArithDivNode * isArithDivNode(Node *n)
{
    if(n->getNodeType()==ArithDivNode_t)
    {
        return (ArithDivNode *)n;
    }
    return 0;
}

ArithMultNode * isArithMultNode(Node *n)
{
    if(n->getNodeType()==ArithMultNode_t)
    {
        return (ArithMultNode *)n;
    }
    return 0;
}

ConditionalLessNode * isConditionalLessNode(Node *n)
{
    if(n->getNodeType()==ConditionalLessNode_t)
    {
        return (ConditionalLessNode *)n;
    }
    return 0;
}

ConditionalGreaterNode * isConditionalGreaterNode(Node *n)
{
    if(n->getNodeType()==ConditionalGreaterNode_t)
    {
        return (ConditionalGreaterNode *)n;
    }
    return 0;
}

ConditionalGreaterEqualNode * isConditionalGreaterEqualNode(Node *n)
{
    if(n->getNodeType()==ConditionalGreaterEqualNode_t)
    {
        return (ConditionalGreaterEqualNode *)n;
    }
    return 0;
}

ConditionalLessEqualNode * isConditionalLessEqualNode(Node *n)
{
    if(n->getNodeType()==ConditionalLessEqualNode_t)
    {
        return (ConditionalLessEqualNode *)n;
    }
    return 0;
}

ConditionalEqualNode * isConditionalEqualNode(Node *n)
{
    if(n->getNodeType()==ConditionalEqualNode_t)
    {
        return (ConditionalEqualNode *)n;
    }
    return 0;
}

ConditionalNotEqualNode * isConditionalNotEqualNode(Node *n)
{
    if(n->getNodeType()==ConditionalNotEqualNode_t)
    {
        return (ConditionalNotEqualNode *)n;
    }
    return 0;
}

ShiftLeftNode * isShiftLeftNode(Node *n)
{
    if(n->getNodeType()==ShiftLeftNode_t)
    {
        return (ShiftLeftNode *)n;
    }
    return 0;
}

ShiftRightNode * isShiftRightNode(Node *n)
{
    if(n->getNodeType()==ShiftRightNode_t)
    {
        return (ShiftRightNode *)n;
    }
    return 0;
}

RotateLeftNode * isRotateLeftNode(Node *n)
{
    if(n->getNodeType()==RotateLeftNode_t)
    {
        return (RotateLeftNode *)n;
    }
    return 0;
}

ParenExprNode * isParenExprNode(Node *n)
{
    if(n->getNodeType()==ParenExprNode_t)
    {
        return (ParenExprNode *)n;
    }
    return 0;
}

TermNode * isTermNode(Node *n)
{
    if(n->getNodeType()==TermNode_t)
    {
        return (TermNode *)n;
    }
    return 0;
}

ArrayAccessNode * isArrayAccessNode(Node *n)
{
    if(n->getNodeType()==ArrayAccessNode_t)
    {
        return (ArrayAccessNode *)n;
    }
    return 0;
}

WireAccessNode * isWireAccessNode(Node *n)
{
    if(n->getNodeType()==WireAccessNode_t)
    {
        return (WireAccessNode *)n;
    }
    return 0;
}

AssignNode * isAssignNode(Node *n)
{
    if(n->getNodeType()==AssignNode_t)
    {
        return (AssignNode *)n;
    }
    return 0;
}

IntTypedefNode * isIntTypedefNode(Node *n)
{
    if(n->getNodeType()==IntTypedefNode_t)
    {
        return (IntTypedefNode *)n;
    }
    return 0;
}

UIntTypedefNode * isUIntTypedefNode(Node *n)
{
    if(n->getNodeType()==UIntTypedefNode_t)
    {
        return (UIntTypedefNode *)n;
    }
    return 0;
}

StructTypedefNode * isStructTypedefNode(Node *n)
{
    if(n->getNodeType()==StructTypedefNode_t)
    {
        return (StructTypedefNode *)n;
    }
    return 0;
}

DeclarationNode * isDeclarationNode(Node *n)
{
    if(n->getNodeType()==DeclarationNode_t)
    {
        return (DeclarationNode *)n;
    }
    return 0;
}

FunctionVarDeclarationNode * isFunctionVarDeclarationNode(Node *n)
{
    if(n->getNodeType()==FunctionVarDeclarationNode_t)
    {
        return (FunctionVarDeclarationNode *)n;
    }
    return 0;
}

VarDeclarationCommaNode * isVarDeclarationCommaNode(Node *n)
{
    if(n->getNodeType()==VarDeclarationCommaNode_t)
    {
        return (VarDeclarationCommaNode *)n;
    }
    return 0;
}

PartiesNode * isPartiesNode(Node *n)
{
    if(n->getNodeType()==PartiesNode_t)
    {
        return (PartiesNode *)n;
    }
    return 0;
}

InputNode * isInputNode(Node *n)
{
    if(n->getNodeType()==InputNode_t)
    {
        return (InputNode *)n;
    }
    return 0;
}

FunctionCallNode * isFunctionCallNode(Node *n)
{
    if(n->getNodeType()==FunctionCallNode_t)
    {
        return (FunctionCallNode *)n;
    }
    return 0;
}

DotOperatorNode * isDotOperatorNode(Node *n)
{
    if(n->getNodeType()==DotOperatorNode_t)
    {
        return (DotOperatorNode *)n;
    }
    return 0;
}

OutputNode * isOutputNode(Node *n)
{
    if(n->getNodeType()==OutputNode_t)
    {
        return (OutputNode *)n;
    }
    return 0;
}

DefineNode * isDefineNode(Node *n)
{
    if(n->getNodeType()==DefineNode_t)
    {
        return (DefineNode *)n;
    }
    return 0;
}

IncludeNode * isIncludeNode(Node *n)
{
    if(n->getNodeType()==IncludeNode_t)
    {
        return (IncludeNode *)n;
    }
    return 0;
}

VarDeclarationListNode * isVarDeclarationListNode(Node *n)
{
    if(n->getNodeType()==VarDeclarationListNode_t)
    {
        return (VarDeclarationListNode *)n;
    }
    return 0;
}

ArrayDeclarationListNode * isArrayDeclarationListNode(Node *n)
{
    if(n->getNodeType()==ArrayDeclarationListNode_t)
    {
        return (ArrayDeclarationListNode *)n;
    }
    return 0;
}

StatementListNode * isStatementListNode(Node *n)
{
    if(n->getNodeType()==StatementListNode_t)
    {
        return (StatementListNode *)n;
    }
    return 0;
}

DeclarationCommaListNode * isDeclarationCommaListNode(Node *n)
{
    if(n->getNodeType()==DeclarationCommaListNode_t)
    {
        return (DeclarationCommaListNode *)n;
    }
    return 0;
}


ArrayInitListNode * isArrayInitListNode(Node *n)
{
    if(n->getNodeType()==ArrayInitListNode_t)
    {
        return (ArrayInitListNode *)n;
    }
    return 0;
}

FunctionArgListNode * isFunctionArgListNode(Node *n)
{
    if(n->getNodeType()==FunctionArgListNode_t)
    {
        return (FunctionArgListNode *)n;
    }
    return 0;
}

DeclarationArgCommaListNode * isDeclarationArgCommaListNode(Node *n)
{
    if(n->getNodeType()==DeclarationArgCommaListNode_t)
    {
        return (DeclarationArgCommaListNode *)n;
    }
    return 0;
}

ProgramListNode * isProgramListNode(Node *n)
{
    if(n->getNodeType()==ProgramListNode_t)
    {
        return (ProgramListNode *)n;
    }
    return 0;
}

FunctionDeclarationNode * isFunctionDeclarationNode(Node *n)
{
    if(n->getNodeType()==FunctionDeclarationNode_t)
    {
        return (FunctionDeclarationNode *)n;
    }
    return 0;
}

ForNode * isForNode(Node *n)
{
    if(n->getNodeType()==ForNode_t)
    {
        return (ForNode *)n;
    }
    return 0;
}

IfNode * isIfNode(Node *n)
{
    if(n->getNodeType()==IfNode_t)
    {
        return (IfNode *)n;
    }
    return 0;
}

CompoundStatementNode * isCompoundStatementNode(Node *n)
{
    if(n->getNodeType()==CompoundStatementNode_t)
    {
        return (CompoundStatementNode *)n;
    }
    return 0;
}

ExpressionStatementNode * isExpressionStatementNode(Node *n)
{
    if(n->getNodeType()==ExpressionStatementNode_t)
    {
        return (ExpressionStatementNode *)n;
    }
    return 0;
}

VarDeclarationNode * isVarDeclarationNode(Node *n)
{
    if(n->getNodeType()==VarDeclarationNode_t)
    {
        return (VarDeclarationNode *)n;
    }
    return 0;
}

VarDeclarationForNode * isVarDeclarationForNode(Node *n)
{
    if(n->getNodeType()==VarDeclarationForNode_t)
    {
        return (VarDeclarationForNode *)n;
    }
    return 0;
}

VarDeclarationPrimNode * isVarDeclarationPrimNode(Node *n)
{
    if(n->getNodeType()==VarDeclarationPrimNode_t)
    {
        return (VarDeclarationPrimNode *)n;
    }
    return 0;
}

ReturnNode * isReturnNode(Node *n)
{
    if(n->getNodeType()==ReturnNode_t)
    {
        return (ReturnNode *)n;
    }
    return 0;
}

FunctionReturnTypeNode * isFunctionReturnTypeNode(Node *n)
{
    if(n->getNodeType()==FunctionReturnTypeNode_t)
    {
        return (FunctionReturnTypeNode *)n;
    }
    return 0;
}


















