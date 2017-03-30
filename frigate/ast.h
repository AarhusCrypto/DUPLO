//This code is based off an example found online.

#ifndef AST_H
#define AST_H

enum Node_Enum {Node_t, BitwiseORNode_t,BitwiseANDNode_t,BitwiseXORNode_t,ConditionalANDNode_t, ConditionalORNode_t, ArrayDecNode_t, ArrayInitNode_t, UnaryNOTNode_t, UnaryMinusNode_t, UnaryPostMinusMinusNode_t, UnaryPostPlusPlusNode_t, UnaryPrePlusPlusNode_t, UnaryPreMinusMinusNode_t, ArithModuloNode_t, ArithReModuloNode_t, ArithPlusNode_t, ArithMinusNode_t, ArithDivNode_t, ArithReDivNode_t, ArithMultNode_t, ArithExMultNode_t,  ConditionalLessNode_t, ConditionalGreaterNode_t, ConditionalGreaterEqualNode_t, ConditionalLessEqualNode_t, ConditionalEqualNode_t, ConditionalNotEqualNode_t, ShiftLeftNode_t, ShiftRightNode_t, RotateLeftNode_t, ParenExprNode_t, TermNode_t, ArrayAccessNode_t, WireAccessNode_t, AssignNode_t, IntTypedefNode_t,UIntTypedefNode_t,  StructTypedefNode_t, DeclarationNode_t, FunctionVarDeclarationNode_t, VarDeclarationCommaNode_t, PartiesNode_t, InputNode_t, FunctionCallNode_t, DotOperatorNode_t, OutputNode_t, DefineNode_t, IncludeNode_t, VarDeclarationListNode_t, ArrayDeclarationListNode_t, StatementListNode_t, DeclarationCommaListNode_t, ArrayInitListNode_t, FunctionArgListNode_t, DeclarationArgCommaListNode_t, ProgramListNode_t, FunctionDeclarationNode_t, ForNode_t, IfNode_t, CompoundStatementNode_t, ExpressionStatementNode_t, VarDeclarationNode_t, VarDeclarationForNode_t, VarDeclarationPrimNode_t, ReturnNode_t, FunctionReturnTypeNode_t};


#include <map>
#include <vector>
#include <ostream>
#include <stdexcept>
#include <cmath>
#include <vector>
#include <string.h>
#include <iostream>


#include "types.h"
#include "error.h"
#include <sstream>


#include "wire.h"


using namespace std;

class CORV;

class AstLocation
{
public:
    int linenobegin;
    int linenoend;
    int colbegin;
    int colend;
    string fname;
    
    string position;
    
    static AstLocation * ConvertLocation(int _linebegin, int _linened, int _colbegin, int _colend)
    {
        AstLocation * loc = new AstLocation();
        loc->linenobegin = _linebegin;
        loc->linenoend = _linened;
        loc->colbegin = _colbegin;
        loc->colend = _colend;
        
        loc->fname = getEFileName();
        
        loc->position = to_string(_linebegin);
        
        return loc;
    }
    
    static AstLocation * Duplicate(AstLocation * loc_in)
    {
        AstLocation * loc = new AstLocation();
        loc->linenobegin = loc_in->linenobegin;
        loc->linenoend = loc_in->linenoend;
        loc->colbegin = loc_in->colbegin;
        loc->colend = loc_in->colend;
        
        loc->fname = loc_in->fname;
        
        loc->position = loc_in->position;
        
        return loc;
    }
};


class Node
{
protected:
    Node_Enum nodetype;
    
    
public:
    string nodeName;
    AstLocation * nodeLocation;
    
    unsigned long long gatesFromNode;
    unsigned long long  gatesFromNodeXor;
    unsigned long long  tempgatecount;
    unsigned long long  tempgatecountxor;
    
    bool has_procs;
    
    bool isSigned;
    bool hasMinus;
    
    long nodeSpecVariable; ///this variable has different uses across different nodes
    
    void beginCircuitCount(long count, long xcount)
    {
        tempgatecount = count;
        tempgatecountxor = xcount;
    }
    void endCircuitCount(long count, long xcount)
    {
        gatesFromNode += (count - tempgatecount);
        gatesFromNodeXor += (xcount - tempgatecountxor);
    }
    
    void allNodeSetup()
    {
        has_procs=true;
        tempgatecount=0;
        gatesFromNode=0;
        gatesFromNodeXor=0;
        isSigned=false;
        returnTypeOfNode=0;
        operandTypeOfNode=0;
        operand2TypeOfNode=0;
        hasMinus=false;
    }
    
    virtual ~Node()
    {
    }
    
    explicit Node()
    {
        nodeName = "Node";
        addedextradepth = false;
        parent = 0;
        nodeLocation=0;
        allNodeSetup();
        returnTypeOfNode=0;
        operandTypeOfNode=0;
    }

    virtual void print(std::ostream &os, unsigned int depth=0) const = 0;


    static inline std::string indent(int d)
    {
        if(d < 0)
        {
            d = 0;
        }
        
        return std::string(d * 2, ' ');
    }
    
    Node_Enum getNodeType()
    {
        return nodetype;
    }
    
    bool addedextradepth;
    Node* parent;
    
    virtual int getNumChildren() = 0;
    virtual Node * getChild(int i)= 0;
    
    virtual Node * deepCopy() = 0;
    
    //returns the child number n is of "this" node. if n is not a child, returns -1
    int getChildNumber(Node * n)
    {
        for(int i=0;i<getNumChildren();i++)
        {
            if(getChild(i)==n)
            {
                return i;
            }
        }
        
        return -1;
    }
    
    Type * returnTypeOfNode;
    Type * operandTypeOfNode;
    Type * operand2TypeOfNode;
    
    void checkSign(Type * t)
    {
        returnTypeOfNode = t;
        
        if(isIntType(t))
        {
            //cout << "isIntType(t): "<<isIntType(t)<<"\n";
            isSigned = true;
        }
    }
    
    //set the new child. deletes old child
    virtual int replaceChild(int i,Node * n, bool deleteOldChild)=0;
    
    virtual long getLongResultNoVar()=0;
    
    virtual Type * typeCheck(VariableContext *, TypeMap *)=0;
    
    void passDownNewLocation(AstLocation * newloc)
    {
        for(int i=0;i<getNumChildren();i++)
        {
            getChild(i)->passDownNewLocation(newloc);
        }
        
        delete nodeLocation;
        nodeLocation = AstLocation::Duplicate(newloc);
    }
    
    virtual CORV  circuitOutput(VariableContext *, TypeMap *, int )=0;

};




Type * binaryTypeCheckNoStructNoArray(Node *left,Node *right,VariableContext *, TypeMap *, string op);
Type * binaryTypeCheckNoStructNoArrayExtend(Node *left,Node *right,VariableContext *, TypeMap *, string op, Node * parent);
Type * binaryTypeCheckNoStructNoArrayReduce(Node *left,Node *right,VariableContext *, TypeMap *, string op, Node * parent);
Type * binaryTypeCheckNoStructNoArray(Node *left,Node *right,VariableContext *, TypeMap *, string op, Node * parent);
Type * binaryTypeCheckStructOKArrayOK(Node *left,Node *right,VariableContext *, TypeMap *, string op);
Type * binaryTypeCheckStructOKArrayOKEQ(Node *left,Node *right,VariableContext *, TypeMap *, string op);
Type * binaryTypeCheckShift(Node *left,Node *right,VariableContext *, TypeMap *, string op);
Type * binaryTypeCheckCondDifferentSizesOK(Node *left,Node *right,VariableContext *, TypeMap *, string op);
Type * binaryTypeCheckCondStructOK(Node *left,Node *right,VariableContext *, TypeMap *, string op,Node * parent);


Type * unaryTypeCheckNoCheck(Node *operand,VariableContext *, TypeMap *, string op);
Type * unaryTypeCheckArray(Node *operand, VariableContext *, TypeMap *, string op);
Type * unaryTypeCheckWire(Node *operand, Node *wirebase, Node * wiremount,VariableContext *, TypeMap *, string op);
Type * unaryTypeCheckNumberRequired(Node *operand,VariableContext *, TypeMap *, string op);

Type * unaryTypeCheckTerm(Node *term,VariableContext *, TypeMap *, string op);
Type * unaryTypeCheckReturn(Node *returnn,VariableContext *, TypeMap *, string op);
Type * unaryTypeCheckFunctionCall(Node *func,VariableContext *, TypeMap *, string op);
Type * unaryTypeCheckDeclarationVar(Node *dec,VariableContext *, TypeMap *, string op);
Type * unaryTypeCheckFunctionDeclarationVar(Node *dec,VariableContext *, TypeMap *, string op);
Type * unaryTypeCheckFunctionDeclaration(Node *dec,VariableContext *, TypeMap *, string op);

Type * ifTypeCheck(Node *ifnode,VariableContext *, TypeMap *, string op);
Type * forTypeCheck(Node *fornode,VariableContext *, TypeMap *, string op);
Type * dotTypeCheck(Node *dotnode,VariableContext *, TypeMap *, string op);
Type * arrayInitTypeCheck(Node *dec,VariableContext *, TypeMap *, string op);
Type * unaryTypeCheckNumberRequiredNoConst(Node *operand,VariableContext * vc, TypeMap * tm, string op);

Type * InputOutputTypeCheck(Node *n,VariableContext * vc, TypeMap * tm);

class BitwiseORNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit BitwiseORNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "BitwiseORNode";
        
        left->parent = this;
        right->parent = this;
        
        nodetype = BitwiseORNode_t;
        nodeLocation=0;
    }
    
    virtual ~BitwiseORNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os<< " | ";
        right->print(os, depth);
    }
    
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new BitwiseORNode(t_left,t_right);
        
        
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() | right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  binaryTypeCheckNoStructNoArray(left,right,vc,tm,"|");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class BitwiseANDNode  : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit BitwiseANDNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "BitwiseANDNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = BitwiseANDNode_t;
        nodeLocation=0;
    }
    
    virtual ~BitwiseANDNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os<< " & ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new BitwiseANDNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() & right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t = binaryTypeCheckNoStructNoArray(left,right,vc,tm,"&");
        checkSign(t);
        return t;
    }

    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class BitwiseXORNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit BitwiseXORNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "BitwiseXORNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = BitwiseXORNode_t;
        nodeLocation=0;
    }
    
    virtual ~BitwiseXORNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os<< " ^ ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new BitwiseXORNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() ^ right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t = binaryTypeCheckNoStructNoArray(left,right,vc,tm,"^");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

/*
class ConditionalANDNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ConditionalANDNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ConditionalANDNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ConditionalANDNode_t;
        nodeLocation=0;
    }
    
    virtual ~ConditionalANDNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os<< " && ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ConditionalANDNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() & right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t = binaryTypeCheckCondDifferentSizesOK(left,right,vc,tm,"&&");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class ConditionalORNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ConditionalORNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ConditionalORNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ConditionalORNode_t;
        nodeLocation=0;
    }
    
    virtual ~ConditionalORNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os<< " || ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ConditionalORNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() | right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  binaryTypeCheckCondDifferentSizesOK(left,right,vc,tm,"||");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};
*/

class ArrayDecNode : public Node
{
public:
    Node* 	operand;
    
public:
    explicit ArrayDecNode(Node* _left) : Node(), operand(_left)
    {
        allNodeSetup();
        nodeName = "ArrayDecNode";
        
        operand->parent = this;
        nodetype = ArrayDecNode_t;
        nodeLocation=0;
    }
    
    virtual ~ArrayDecNode()
    {
        delete operand;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        os<< "[";
        operand->print(os, depth);
        os<< "]";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return operand;
        }

        return 0;
    }
    
    int getNumChildren(){ return 1;}
    
    Node * deepCopy()
    {
        Node * t_operand=0;
        if(operand != 0)
        {
            t_operand = operand->deepCopy();
        }
        else
        {
            t_operand = 0;
        }
        Node * t_node = new ArrayDecNode(t_operand);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete operand;}
            operand=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        //cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return operand->getLongResultNoVar();
    }
 
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  unaryTypeCheckNoCheck(operand,vc,tm,"");
        checkSign(t);
        return t;
    }
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class ArrayInitNode : public Node
{
public:
    Node* 	operand;
    
public:
    explicit ArrayInitNode(Node* _left) : Node(), operand(_left)
    {
        allNodeSetup();
        nodeName = "ArrayInitNode";
        
        operand->parent = this;
        nodetype = ArrayInitNode_t;
        nodeLocation=0;
    }
    
    virtual ~ArrayInitNode()
    {
        delete operand;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        os<< "{";
        operand->print(os, depth);
        os<< "}";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return operand;
        }
        
        return 0;
    }
    
    int getNumChildren(){ return 1;}
    
    Node * deepCopy()
    {
        Node * t_operand=0;
        if(operand != 0)
        {
            t_operand = operand->deepCopy();
        }
        else
        {
            t_operand = 0;
        }
        Node * t_node = new ArrayInitNode(t_operand);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete operand;}
            operand=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  arrayInitTypeCheck(operand,vc,tm,"");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class UnaryNOTNode : public Node
{
public:
    Node* 	operand;
    
public:
    explicit UnaryNOTNode(Node* _left) : Node(), operand(_left)
    {
        allNodeSetup();
        nodeName = "UnaryNOTNode";
        
        operand->parent = this;
        nodetype = UnaryNOTNode_t;
        nodeLocation=0;
    }
    
    virtual ~UnaryNOTNode()
    {
        delete operand;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {

        os<< "~";
        operand->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return operand;
        }
        
        return 0;
    }
    
    int getNumChildren(){ return 1;}
    
    Node * deepCopy()
    {
        Node * t_operand=0;
        if(operand != 0)
        {
            t_operand = operand->deepCopy();
        }
        else
        {
            t_operand = 0;
        }
        Node * t_node = new UnaryNOTNode(t_operand);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete operand;}
            operand=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return ~(operand->getLongResultNoVar());
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  unaryTypeCheckNumberRequiredNoConst(operand,vc,tm,"~");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class ReturnNode : public Node
{
public:
    Node* 	operand;
    
public:
    explicit ReturnNode(Node* _left) : Node(), operand(_left)
    {
        allNodeSetup();
        nodeName = "ReturnNode";
        
        if(operand != 0)
        {
            operand->parent = this;
        }
        nodetype = ReturnNode_t;
        nodeLocation=0;
    }
    
    virtual ~ReturnNode()
    {
        if(operand != 0)
        {
            delete operand;
        }
    }
    
    
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        os<< indent(depth)<<"return";
        if(operand!= 0)
        {
            os <<" ";
            operand->print(os, depth);
        }
        os <<";\n";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return operand;
        }
        
        return 0;
    }
    
    int getNumChildren(){ return 1;}
    
    Node * deepCopy()
    {
        Node * t_operand=0;
        if(operand != 0)
        {
            t_operand = operand->deepCopy();
        }
        else
        {
            t_operand = 0;
        }
        Node * t_node = new ReturnNode(t_operand);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(operand != 0)
            {
                if(deleteOldChild){ delete operand;}
            }
            operand=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  unaryTypeCheckReturn(this,vc,tm,"");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};



class UnaryMinusNode : public Node
{
public:
    Node* 	operand;
    
public:
    explicit UnaryMinusNode(Node* _left) : Node(), operand(_left)
    {
        allNodeSetup();
        nodeName = "UnaryMinusNode";
        
        operand->parent = this;
        nodetype = UnaryMinusNode_t;
        nodeLocation=0;
    }
    
    virtual ~UnaryMinusNode()
    {
        delete operand;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        os<< "-";
        operand->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return operand;
        }
        
        return 0;
    }

    int getNumChildren(){ return 1;}
    
    Node * deepCopy()
    {
        Node * t_operand=0;
        if(operand != 0)
        {
            t_operand = operand->deepCopy();
        }
        else
        {
            t_operand = 0;
        }
        Node * t_node = new UnaryMinusNode(t_operand);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete operand;}
            operand=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return - operand->getLongResultNoVar() ;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        
        Type * t =  unaryTypeCheckNumberRequired(operand,vc,tm,"-");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class UnaryPostMinusMinusNode : public Node
{
public:
    Node* 	operand;
    
public:
    explicit UnaryPostMinusMinusNode(Node* _left) : Node(), operand(_left)
    {
        allNodeSetup();
        nodeName = "UnaryPostMinusMinusNode";
        
        operand->parent = this;
        nodetype = UnaryPostMinusMinusNode_t;
        nodeLocation=0;
    }
    
    virtual ~UnaryPostMinusMinusNode()
    {
        delete operand;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        operand->print(os, depth);
        os<< "--";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return operand;
        }
        
        return 0;
    }
    
    int getNumChildren(){ return 1;}
    
    Node * deepCopy()
    {
        Node * t_operand=0;
        if(operand != 0)
        {
            t_operand = operand->deepCopy();
        }
        else
        {
            t_operand = 0;
        }
        Node * t_node = new UnaryPostMinusMinusNode(t_operand);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete operand;}
            operand=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return operand->getLongResultNoVar() -1;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  unaryTypeCheckNumberRequiredNoConst(operand,vc,tm,"--");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class UnaryPostPlusPlusNode : public Node
{
public:
    Node* 	operand;
    
public:
    explicit UnaryPostPlusPlusNode(Node* _left) : Node(), operand(_left)
    {
        allNodeSetup();
        nodeName = "UnaryPostPlusPlusNode";
        
        operand->parent = this;
        nodetype = UnaryPostPlusPlusNode_t;
        nodeLocation=0;
    }
    
    virtual ~UnaryPostPlusPlusNode()
    {
        delete operand;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        operand->print(os, depth);
        os<< "++";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return operand;
        }
        
        return 0;
    }
    
    int getNumChildren(){ return 1;}
    
    Node * deepCopy()
    {
        Node * t_operand=0;
        if(operand != 0)
        {
            t_operand = operand->deepCopy();
        }
        else
        {
            t_operand = 0;
        }
        Node * t_node = new UnaryPostPlusPlusNode(t_operand);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete operand;}
            operand=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return operand->getLongResultNoVar() +1;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t = unaryTypeCheckNumberRequiredNoConst(operand,vc,tm,"++");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class UnaryPrePlusPlusNode : public Node
{
public:
    Node* 	operand;
    
public:
    explicit UnaryPrePlusPlusNode(Node* _left) : Node(), operand(_left)
    {
        allNodeSetup();
        nodeName = "UnaryPrePlusPlusNode";
        
        operand->parent = this;
        nodetype = UnaryPrePlusPlusNode_t;
        nodeLocation=0;
    }
    
    virtual ~UnaryPrePlusPlusNode()
    {
        delete operand;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        os<< "++";
        operand->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return operand;
        }
        
        return 0;
    }
    
    int getNumChildren(){ return 1;}
    
    Node * deepCopy()
    {
        Node * t_operand=0;
        if(operand != 0)
        {
            t_operand = operand->deepCopy();
        }
        else
        {
            t_operand = 0;
        }
        Node * t_node = new UnaryPrePlusPlusNode(t_operand);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete operand;}
            operand=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return operand->getLongResultNoVar() +1;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t = unaryTypeCheckNumberRequiredNoConst(operand,vc,tm,"++");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class UnaryPreMinusMinusNode : public Node
{
public:
    Node* 	operand;
    
public:
    explicit UnaryPreMinusMinusNode(Node* _left) : Node(), operand(_left)
    {
        allNodeSetup();
        nodeName = "UnaryPreMinusMinusNode";
        
        operand->parent = this;
        nodetype = UnaryPreMinusMinusNode_t;
        nodeLocation=0;
    }
    
    virtual ~UnaryPreMinusMinusNode()
    {
        delete operand;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        os<< "--";
        operand->print(os, depth);
    }
    
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return operand;
        }
        
        return 0;
    }
    
    int getNumChildren(){ return 1;}
    
    Node * deepCopy()
    {
        Node * t_operand=0;
        if(operand != 0)
        {
            t_operand = operand->deepCopy();
        }
        else
        {
            t_operand = 0;
        }
        Node * t_node = new UnaryPreMinusMinusNode(t_operand);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete operand;}
            operand=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return operand->getLongResultNoVar() -1;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  unaryTypeCheckNumberRequiredNoConst(operand,vc,tm,"--");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class ArithModuloNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ArithModuloNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ArithModuloNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ArithModuloNode_t;
        nodeLocation=0;
    }
    
    virtual ~ArithModuloNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" % ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ArithModuloNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() % right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  binaryTypeCheckNoStructNoArray(left,right,vc,tm,"%");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};



class ArithReModuloNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ArithReModuloNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ArithReModuloNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ArithReModuloNode_t;
        nodeLocation=0;
    }
    
    virtual ~ArithReModuloNode()
    {
        delete left;
        delete right;
    }
    
    
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" % ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ArithReModuloNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() % right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  binaryTypeCheckNoStructNoArrayReduce(left,right,vc,tm,"%%",this);
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};




class ArithPlusNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ArithPlusNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ArithPlusNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ArithPlusNode_t;
        nodeLocation=0;
    }
    
    virtual ~ArithPlusNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" + ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ArithPlusNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() + right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  binaryTypeCheckNoStructNoArray(left,right,vc,tm,"+");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class ArithMinusNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ArithMinusNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ArithMinusNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ArithMinusNode_t;
        nodeLocation=0;
    }
    
    virtual ~ArithMinusNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" - ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ArithMinusNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() - right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t = binaryTypeCheckNoStructNoArray(left,right,vc,tm,"-");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class ArithDivNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ArithDivNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ArithDivNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ArithDivNode_t;
        nodeLocation=0;
    }
    
    virtual ~ArithDivNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" / ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ArithDivNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() / right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  binaryTypeCheckNoStructNoArray(left,right,vc,tm,"/");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class ArithReDivNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ArithReDivNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ArithReDivNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ArithReDivNode_t;
        nodeLocation=0;
    }
    
    virtual ~ArithReDivNode()
    {
        delete left;
        delete right;
    }
    
    
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" / ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ArithReDivNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() / right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  binaryTypeCheckNoStructNoArrayReduce(left,right,vc,tm,"//",this);
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};



class ArithMultNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ArithMultNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ArithMultNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ArithMultNode_t;
        nodeLocation=0;
    }
    
    virtual ~ArithMultNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" * ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ArithMultNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() * right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t = binaryTypeCheckNoStructNoArray(left,right,vc,tm,"*");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class ArithExMultNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ArithExMultNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ArithExMultNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ArithExMultNode_t;
        nodeLocation=0;
    }
    
    virtual ~ArithExMultNode()
    {
        delete left;
        delete right;
    }
    
    
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" ** ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ArithExMultNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() * right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t = binaryTypeCheckNoStructNoArrayExtend(left,right,vc,tm,"**",this);
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};




class ConditionalLessNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ConditionalLessNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ConditionalLessNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ConditionalLessNode_t;
        nodeLocation=0;
    }
    
    virtual ~ConditionalLessNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" < ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ConditionalLessNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() < right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t = binaryTypeCheckNoStructNoArray(left,right,vc,tm,"<",this);
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class ConditionalGreaterNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ConditionalGreaterNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ConditionalGreaterNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ConditionalGreaterNode_t;
        nodeLocation=0;
    }
    
    virtual ~ConditionalGreaterNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" > ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ConditionalGreaterNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() > right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  binaryTypeCheckNoStructNoArray(left,right,vc,tm,">",this);
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class ConditionalGreaterEqualNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ConditionalGreaterEqualNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ConditionalGreaterEqualNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ConditionalGreaterEqualNode_t;
        nodeLocation=0;
    }
    
    virtual ~ConditionalGreaterEqualNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" >= ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ConditionalGreaterEqualNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() >= right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  binaryTypeCheckNoStructNoArray(left,right,vc,tm,">=",this);
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class ConditionalLessEqualNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ConditionalLessEqualNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ConditionalLessEqualNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ConditionalLessEqualNode_t;
        nodeLocation=0;
    }
    
    virtual ~ConditionalLessEqualNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" <= ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ConditionalLessEqualNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() <= right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  binaryTypeCheckNoStructNoArray(left,right,vc,tm,"<=",this);
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class ConditionalEqualNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ConditionalEqualNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ConditionalEqualNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ConditionalEqualNode_t;
        nodeLocation=0;
    }
    
    virtual ~ConditionalEqualNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" == ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ConditionalEqualNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() == right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  binaryTypeCheckCondStructOK(left,right,vc,tm,"==",this);
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class ConditionalNotEqualNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ConditionalNotEqualNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ConditionalNotEqualNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ConditionalNotEqualNode_t;
        nodeLocation=0;
    }
    
    virtual ~ConditionalNotEqualNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" != ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ConditionalNotEqualNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() != right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t = binaryTypeCheckCondStructOK(left,right,vc,tm,"!=",this);
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class ShiftLeftNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ShiftLeftNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ShiftLeftNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ShiftLeftNode_t;
        nodeLocation=0;
    }
    
    virtual ~ShiftLeftNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" << ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ShiftLeftNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() << right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  binaryTypeCheckShift(left,right,vc,tm,"<<");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class ShiftRightNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit ShiftRightNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "ShiftRightNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = ShiftRightNode_t;
        nodeLocation=0;
    }
    
    virtual ~ShiftRightNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" >> ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new ShiftRightNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return left->getLongResultNoVar() >> right->getLongResultNoVar();
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  binaryTypeCheckShift(left,right,vc,tm,">>");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};




class RotateLeftNode : public Node
{
public:
    Node* 	left;
    Node* 	right;
    
public:
    explicit RotateLeftNode(Node* _left, Node* _right) : Node(), left(_left), right(_right)
    {
        allNodeSetup();
        nodeName = "RotateLeftNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = RotateLeftNode_t;
        nodeLocation=0;
    }
    
    virtual ~RotateLeftNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os <<" <<> ";
        right->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new RotateLeftNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        long x = left->getLongResultNoVar();
        long n = right->getLongResultNoVar();
        
        return  (x<<n) | (x>>(32-n));
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  binaryTypeCheckShift(left,right,vc,tm,"<<>");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class ParenExprNode: public Node
{
public:
    Node* operand;

    
public:
    explicit ParenExprNode(Node* _node) : Node(), operand(_node)
    {
        allNodeSetup();
        nodeName = "ParenExprNode";
        
        operand->parent = this;
        nodetype = ParenExprNode_t;
        nodeLocation=0;

    }
    
    virtual ~ParenExprNode()
    {
        if(operand != 0)
        {
            delete operand;
        }
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        if(operand != 0)
        {
            os <<"(";
            operand->print(os,depth);
            os<<")";
        }
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return operand;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        if(operand == 0)
        {
            return 0;
        }
        return 1;
    }

    Node * deepCopy()
    {
        Node * t_operand=0;
        if(operand != 0)
        {
            t_operand = operand->deepCopy();
        }
        else
        {
            t_operand = 0;
        }
        Node * t_node = new ParenExprNode(t_operand);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(operand!= 0)
            {
                if(deleteOldChild){ delete operand;}
            }
            operand=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        return operand->getLongResultNoVar();
    }

    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t = unaryTypeCheckNoCheck(operand,vc,tm,"()");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};



class TermNode : public Node
{
public:
    
    string num;
    string var;
    bool isNum;
    
    int bitwidth;
    Node * bitwidthnode;
    
public:
    explicit TermNode(string _var) : Node(), var(_var)
    {
        allNodeSetup();
        nodeName = "TermNode";
        
        num ="";
        nodetype = TermNode_t;
        isNum = false;
        nodeLocation=0;
        bitwidth=-1;
        bitwidthnode=0;
    }
    
    explicit TermNode(string _num, bool isnum) : Node(), num(_num)
    {
        allNodeSetup();
        nodeName = "TermNode";
        
        var ="";
        nodetype = TermNode_t;
        isNum = true;
        nodeLocation=0;
        bitwidth=-1;
        bitwidthnode=0;
    }
    
    explicit TermNode(string _var, AstLocation * _astloc) : Node(), var(_var)
    {
        allNodeSetup();
        nodeName = "TermNode";
        
        num ="";
        nodetype = TermNode_t;
        isNum = false;
        nodeLocation = _astloc;
        bitwidth=-1;
        bitwidthnode=0;
    }
    
    explicit TermNode(string _num, bool isnum, AstLocation * _astloc) : Node(), num(_num)
    {
        allNodeSetup();
        nodeName = "TermNode";
        
        var ="";
        nodetype = TermNode_t;
        isNum = true;
        nodeLocation = _astloc;
        bitwidth=-1;
        bitwidthnode=0;
    }
    
    virtual ~TermNode()
    {
        //std::cout<<"deletingterm\n";
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        if(num != "")
        {
            os <<num;
        }
        
        if (var != "")
        {
            os <<var;
        }
    }
    

    
    Node * getChild(int i)
    {
        if(bitwidthnode != 0 && i == 0)
        {
            return bitwidthnode;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        if(bitwidthnode != 0)
        {
            return 1;
        }
        
        return 0;
    }
    
    Node * deepCopy()
    {
        Node * t_node=0;
        if(!isNum)
        {
            t_node = new TermNode(var);
        }
        else
        {
            t_node = new TermNode(num,true);
        }
        
        if(bitwidthnode != 0)
        {
            Node * bit = 0;
            bit = bitwidthnode->deepCopy();
            ((TermNode *) (t_node))->bitwidthnode = bit;
        }
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i == 0 && bitwidthnode != 0)
        {
            if(deleteOldChild){ delete bitwidthnode;}
            bitwidthnode=newnode;
            return 1;
        }
        
        return 0;
    }
    
    long getLongResultNoVar()
    {
        if(isNum)
        {
            return stol(num);
        }

        addError(nodeLocation->fname,nodeLocation->position,"Cannot have variable acess in typdef, variable, const, or wireamount declarations.  Recieved variable \""+var+"\".");
        
        return -1;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t = unaryTypeCheckTerm(this,vc,tm,"");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class ArrayAccessNode : public Node
{
public:
    Node* 	left;
    Node*   arrayexpr;
    
public:
    explicit ArrayAccessNode(Node* _left, Node* _arrayexpr) : Node(), left(_left), arrayexpr(_arrayexpr)
    {
        allNodeSetup();
        nodeName = "ArrayAccessNode";
        
        left->parent = this;
        arrayexpr->parent = this;
        nodetype = ArrayAccessNode_t;
        nodeLocation=0;
    }
    
    virtual ~ArrayAccessNode()
    {
        delete left;
        delete arrayexpr;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os<< "[";
        arrayexpr->print(os, depth);
        os<< "]";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        
        if(i == 1)
        {
            return arrayexpr;
        }
        
        return 0;
    }
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_arrayexpr = arrayexpr->deepCopy();
        Node * t_node = new ArrayAccessNode(t_left,t_arrayexpr);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete arrayexpr;}
            arrayexpr=newnode;
            return 1;
        }
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    long getLongResultNoVar()
    {
        addError(nodeLocation->fname,nodeLocation->position,"Cannot have array acess in typdef, variable, or wireamount declarations.");
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t = unaryTypeCheckArray(this,vc,tm,"[]");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class WireAccessNode : public Node
{
public:
    Node* 	left;
    Node*   wirebase;
    Node*   wireamount;
    
public:
    explicit WireAccessNode(Node* _left, Node* _arrayexpr) : Node(), left(_left), wirebase(_arrayexpr), wireamount(0)
    {
        allNodeSetup();
        nodeName = "WireAccessNode";
        
        left->parent = this;
        wirebase->parent = this;
        nodetype = WireAccessNode_t;
        nodeLocation=0;
    }
    
    explicit WireAccessNode(Node* _left, Node* _arrayexpr, Node* _arrayexpr2) : Node(), left(_left), wirebase(_arrayexpr), wireamount(_arrayexpr2)
    {
        allNodeSetup();
        nodeName = "WireAccessNode";
        
        left->parent = this;
        wirebase->parent = this;
        wireamount->parent = this;
        nodetype = WireAccessNode_t;
        nodeLocation=0;
    }
    
    virtual ~WireAccessNode()
    {
        delete left;
        delete wirebase;
        if(wireamount!= 0)
        {
            delete wireamount;
        }
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os<< "{";
        wirebase->print(os, depth);
        if(wireamount!= 0)
        {
            os <<":";
            wireamount->print(os, depth);
        }
        os<< "}";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        
        if(i == 1)
        {
            return wirebase;
        }
        
        if(i == 2)
        {
            return wireamount;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        if(wireamount == 0)
        {
            return 2;
        }
        return 3;
    }
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_wirebase = wirebase->deepCopy();
        Node * t_wireamount=0;
        
        Node * t_node=0;
        
        if(wireamount != 0)
        {
            t_wireamount = wireamount->deepCopy();
            t_node =new WireAccessNode(t_left,t_wirebase, t_wireamount);
        }
        else
        {
             t_node = new WireAccessNode(t_left,t_wirebase);
        }
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete wirebase;}
            wirebase=newnode;
            return 1;
        }
        if(i == 2)
        {
            if(deleteOldChild){ delete wireamount;}
            wireamount=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        addError(nodeLocation->fname,nodeLocation->position,"Cannot have wire acess in typdef, variable, or wireamount declarations.");
        
        //long res =
        
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  unaryTypeCheckWire(left,wirebase,wireamount,vc,tm,"");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class AssignNode : public Node
{
public:
    Node* 	left;
    Node*   expr;
    
public:
    explicit AssignNode(Node* _left, Node* _arrayexpr) : Node(), left(_left), expr(_arrayexpr)
    {
        allNodeSetup();
        nodeName = "AssignNode";
        
        left->parent = this;
        expr->parent = this;
        nodetype = AssignNode_t;
        nodeLocation=0;
    }
    
    virtual ~AssignNode()
    {
        delete left;
        delete expr;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        left->print(os, depth);
        os<< " = ";
        expr->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        
        if(i == 1)
        {
            return expr;
        }

        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_expr = expr->deepCopy();
        Node * t_node = new AssignNode(t_left,t_expr);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete expr;}
            expr=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        addError(nodeLocation->fname,nodeLocation->position,"Cannot have assign in typdef, variable, or wireamount declarations.");
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t = binaryTypeCheckStructOKArrayOKEQ(left,expr,vc,tm,"=");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};



class IntTypedefNode : public Node
{
public:
    Node* 	name;
    Node*   sizev;
    
public:
    explicit IntTypedefNode(Node* _name, Node* _sizet) : Node(), name(_name), sizev(_sizet)
    {
        allNodeSetup();
        nodeName = "IntTypedefNode";
        
        name->parent = this;
        sizev->parent = this;
        nodetype = IntTypedefNode_t;
        nodeLocation=0;
        
    }
    
    virtual ~IntTypedefNode()
    {
        delete name;
        if(sizev != 0)
        {
            delete sizev;
        }

    }
    
    
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        os <<"typedef ";
        if(sizev != 0)
        {
            os <<"int ";
            sizev->print(os, depth);
            os <<" ";
            name->print(os, depth);
            os <<"\n";
        }

    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(sizev != 0)
        {
            if(i == 0)
            {
                return sizev;
            }
            
            if(i == 1)
            {
                return name;
            }
        }

        
        
        
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_name = name->deepCopy();
        
        
        Node * t_sizev=0;
        if(sizev!= 0)
        {
            t_sizev = sizev->deepCopy();
        }
        
        Node * t_node=0;
        
        if(t_sizev!= 0)
        {
            t_node = new IntTypedefNode(t_name,t_sizev);
        }
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(sizev != 0)
        {
            if(i == 0)
            {
                if(deleteOldChild){ delete sizev;}
                sizev=newnode;
                return 1;
            }
            
            if(i == 1)
            {
                if(deleteOldChild){ delete name;}
                name=newnode;
                return 1;
            }
        }

        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return getNoType();
        
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class UIntTypedefNode : public Node
{
public:
    Node* 	name;
    Node*   sizev;
    
public:
    explicit UIntTypedefNode(Node* _name, Node* _sizet) : Node(), name(_name), sizev(_sizet)
    {
        allNodeSetup();
        nodeName = "UIntTypedefNode";
        
        name->parent = this;
        sizev->parent = this;
        nodetype = UIntTypedefNode_t;
        nodeLocation=0;
        
    }
    
    virtual ~UIntTypedefNode()
    {
        delete name;
        if(sizev != 0)
        {
            delete sizev;
        }
        
    }
    
    
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        os <<"typedef ";
        if(sizev != 0)
        {
            os <<"uint ";
            sizev->print(os, depth);
            os <<" ";
            name->print(os, depth);
            os <<"\n";
        }
        
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(sizev != 0)
        {
            if(i == 0)
            {
                return sizev;
            }
            
            if(i == 1)
            {
                return name;
            }
        }
        
        
        
        
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_name = name->deepCopy();
        
        
        Node * t_sizev=0;
        if(sizev!= 0)
        {
            t_sizev = sizev->deepCopy();
        }
        
        Node * t_node=0;
        
        if(t_sizev!= 0)
        {
            t_node = new UIntTypedefNode(t_name,t_sizev);
        }
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(sizev != 0)
        {
            if(i == 0)
            {
                if(deleteOldChild){ delete sizev;}
                sizev=newnode;
                return 1;
            }
            
            if(i == 1)
            {
                if(deleteOldChild){ delete name;}
                name=newnode;
                return 1;
            }
        }
        
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return getNoType();
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};




class StructTypedefNode : public Node
{
public:
    Node* 	name;

    Node*   structt;
    
public:

    explicit StructTypedefNode(Node* _name, Node* _structt) : Node(), name(_name), structt(_structt)
    {
        allNodeSetup();
        nodeName = "StructTypedefNode";
        
        name->parent = this;
        structt->parent = this;
        nodetype = StructTypedefNode_t;
        nodeLocation=0;
    }
    
    virtual ~StructTypedefNode()
    {
        delete name;
        if(structt != 0)
        {
            delete structt;
        }
    }
    
    
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        os <<"typedef ";

        if(structt != 0)
        {
            os <<"struct ";
            name->print(os, depth);
            os <<"\n{\n";
            structt->print(os, depth+1);
            os << "}\n";
        }
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }

        
        if(structt != 0)
        {
            if(i == 1)
            {
                return structt;
            }
            
            if(i == 0)
            {
                return name;
            }
        }
        
        
        
        return 0;
    }
    
    int getNumChildren(){ return 2;}
    
    Node * deepCopy()
    {
        Node * t_name = name->deepCopy();
        
        Node * t_structt=0;
        if(structt!= 0)
        {
            t_structt = structt->deepCopy();
        }
        
        
        Node * t_node=0;

        
        if(t_structt != 0)
        {
            t_node = new StructTypedefNode(t_name,t_structt);
        }
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }

        if(structt != 0)
        {
            if(i == 1)
            {
                if(deleteOldChild){ delete structt;}
                structt=newnode;
                return 1;
            }
            
            if(i == 0)
            {
                if(deleteOldChild){ delete name;}
                name=newnode;
                return 1;
            }
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return getNoType();
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};



class DeclarationNode : public Node
{
public:
    Node* 	varname;
    Node*   arrays;

    
public:
    explicit DeclarationNode(Node* _varname, Node* _arraysize) : Node(), varname(_varname), arrays(_arraysize)
    {
        allNodeSetup();
        nodeName = "DeclarationNode";
        
        varname->parent = this;
        arrays->parent = this;
        nodetype = DeclarationNode_t;
        nodeLocation=0;

    }
    
    explicit DeclarationNode(Node* _varname) : Node(), varname(_varname), arrays(0)
    {
        allNodeSetup();
        nodeName = "DeclarationNode";
        
        varname->parent = this;
        nodetype = DeclarationNode_t;
    }
    

    
    virtual ~DeclarationNode()
    {
        delete varname;
        
        if(arrays != 0)
        {
            delete arrays;
        }
    }
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        varname->print(os, depth);
        if(arrays != 0)
        {
            arrays->print(os, depth);
        }
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return varname;
        }
        
        if(i == 1)
        {
            return arrays;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        if(arrays == 0)
        {
            return 1;
        }
        return 2;
    }
    
    Node * deepCopy()
    {
        Node * t_varname = varname->deepCopy();
        
        Node * t_arrays=0;
        if(arrays!= 0)
        {
            t_arrays = arrays->deepCopy();
        }
        
        Node * t_node=0;
        
        if(t_arrays== 0)
        {
            t_node = new DeclarationNode(t_varname);
        }
        
        if(t_arrays != 0)
        {
            t_node = new DeclarationNode(t_varname,t_arrays);
        }
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
    
        if(i == 0)
        {
            if(deleteOldChild){ delete varname;}
            varname=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete arrays;}
            arrays=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return getNoType();
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class FunctionVarDeclarationNode : public Node
{
public:
    Node* 	varname;
    Node*   arrays;
    Node*   type;
    bool    passByReference;
    
public:
    explicit FunctionVarDeclarationNode(Node* _type, Node* _varname, Node* _arraysize) : Node(), varname(_varname), arrays(_arraysize), type(_type)
    {
        allNodeSetup();
        nodeName = "FunctionVarDeclarationNode";
        
        varname->parent = this;
        type->parent = this;
        arrays->parent = this;
        nodetype = FunctionVarDeclarationNode_t;
        passByReference = false;
        nodeLocation=0;
    }
    
    explicit FunctionVarDeclarationNode(Node* _type, Node* _varname) : Node(), varname(_varname), type(_type), arrays(0)
    {
        allNodeSetup();
        nodeName = "FunctionVarDeclarationNode";
        
        varname->parent = this;
        type->parent = this;
        passByReference = false;
        nodetype = FunctionVarDeclarationNode_t;
        nodeLocation=0;
    }
    
    
    
    virtual ~FunctionVarDeclarationNode()
    {
        delete varname;
        delete type;
        
        if(arrays != 0)
        {
            delete arrays;
        }
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        type->print(os,depth);
        os<<" ";
        
        if(passByReference)
        {
            os<<"& ";
        }
        
        varname->print(os, depth);
        if(arrays != 0)
        {
            arrays->print(os, depth);
        }
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return type;
        }
        
        if(i == 1)
        {
            return varname;
        }
        
        if(i == 2)
        {
            return arrays;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        if(arrays == 0)
        {
            return 2;
        }
        return 3;
    }
    
    Node * deepCopy()
    {
        Node * t_type = type->deepCopy();
        Node * t_varname = varname->deepCopy();
        
        FunctionVarDeclarationNode * t_node=0;
        Node * t_arrays=0;
        if(arrays!= 0)
        {
            t_arrays = arrays->deepCopy();
            t_node = new FunctionVarDeclarationNode(t_type,t_varname,t_arrays);
        }
        else
        {
            t_node = new FunctionVarDeclarationNode(t_type,t_varname);
        }

        t_node->passByReference = this->passByReference;
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 1)
        {
            if(deleteOldChild){ delete varname;}
            varname=newnode;
            return 1;
        }
        if(i == 2)
        {
            if(deleteOldChild){ delete arrays;}
            arrays=newnode;
            return 1;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete type;}
            type=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t = unaryTypeCheckFunctionDeclarationVar(this,vc,tm,"");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};



class VarDeclarationCommaNode : public Node
{
public:
    vector<Node *> nodeList;
    
public:
    explicit VarDeclarationCommaNode() : Node()
    {
        allNodeSetup();
        nodeName = "VarDeclarationCommaNode";
        
        nodetype = VarDeclarationCommaNode_t;
        nodeLocation=0;
    }
    
    virtual ~VarDeclarationCommaNode()
    {
        for(int i=0;i<nodeList.size();i++)
        {
            delete nodeList[i];
        }
    }
    void addNode(Node * node)
    {
        node->parent = this;
        nodeList.push_back(node);
    }
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        for(int i=0;i<nodeList.size();i++)
        {
            nodeList[i]->print(os,depth);
            
            if(i != nodeList.size() - 1)
            {
                os <<", ";
            }
        }
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        return nodeList[i];
    }
    
    int getNumChildren()
    {
        return nodeList.size();
    }
    
    Node * deepCopy()
    {
        VarDeclarationCommaNode * t_node = new VarDeclarationCommaNode();
        t_node->nodeList.resize(nodeList.size());
        for(int i=0;i<nodeList.size();i++)
        {
            t_node->nodeList[i] = nodeList[i]->deepCopy();
        }
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(deleteOldChild){ delete nodeList[i];}
        nodeList[i]=newnode;
        return 1;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        for(int i=0;i<nodeList.size();i++)
        {
            nodeList[i]->typeCheck(vc, tm);
        }
        return getNoType();
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};



class PartiesNode : public Node
{
public:
    Node* 	number;
    
public:
    explicit PartiesNode(Node* _left) : Node(), number(_left)
    {
        allNodeSetup();
        nodeName = "PartiesNode";
        
        number->parent = this;
        nodetype = PartiesNode_t;
        nodeLocation=0;
    }
    
    virtual ~PartiesNode()
    {
        delete number;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        os<< "#parties ";
        number->print(os, depth);
        os<< "\n";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return number;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        return 1;
    }
    
    Node * deepCopy()
    {
        Node * t_number = number->deepCopy();
        
        PartiesNode * t_node= new PartiesNode(t_number);

        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete number;}
            number=newnode;
            return 1;
        }

        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return InputOutputTypeCheck(this,vc,tm);
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class InputNode : public Node
{
public:
    Node* 	party;
    Node*   type;
    
public:
    explicit InputNode(Node* _left, Node* _arrayexpr) : Node(), party(_left), type(_arrayexpr)
    {
        allNodeSetup();
        nodeName = "InputNode";
        
        party->parent = this;
        type->parent = this;
        nodetype = InputNode_t;
        nodeLocation=0;
    }
    
    virtual ~InputNode()
    {
        delete party;
        delete type;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        os<< "#input ";
        party->print(os, depth);
        os <<" ";
        type->print(os, depth);
        os<< "\n";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return party;
        }
        if(i == 1)
        {
            return type;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        return 2;
    }
    
    Node * deepCopy()
    {
        Node * t_party = party->deepCopy();
        Node * t_type = type->deepCopy();
        
        InputNode * t_node= new InputNode(t_party,t_type);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete party;}
            party=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete type;}
            type=newnode;
            return 1;
        }
        
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return InputOutputTypeCheck(this,vc,tm);
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class FunctionCallNode : public Node
{
public:
    Node* 	function_name;
    Node*   args;
    
public:
    explicit FunctionCallNode(Node* _left, Node* _arrayexpr) : Node(), function_name(_left), args(_arrayexpr)
    {
        allNodeSetup();
        nodeName = "FunctionCallNode";
        
        function_name->parent = this;
        if(args != 0)
        {
            args->parent = this;
        }
        nodetype = FunctionCallNode_t;
        nodeLocation=0;
    }
    
    virtual ~FunctionCallNode()
    {
        delete function_name;
        
        if(args != 0)
        {
            delete args;
        }
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        
        function_name->print(os, depth);
        os <<"(";
        if(args != 0)
        {
            args->print(os, depth);
        }
        os<< ")";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return function_name;
        }
        if(i == 1)
        {
            return args;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        if(args == 0)
        {
            return 1;
        }
        return 2;
    }
    
    Node * deepCopy()
    {
        Node * t_function_name = function_name->deepCopy();
        
        Node * t_args=0;
        if(args != 0)
        {
           t_args = args->deepCopy();
        }
        
        FunctionCallNode * t_node= new FunctionCallNode(t_function_name,t_args);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete function_name;}
            function_name=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete args;}
            args=newnode;
            return 1;
        }
        
        return 0;
    }
    
    long getLongResultNoVar()
    {
        addError(nodeLocation->fname,nodeLocation->position,"Cannot have function calls in typdef, variable, or wireamount declarations.");
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t  =  unaryTypeCheckFunctionCall(this,vc,tm,"");
        checkSign(t);
        return t;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class DotOperatorNode : public Node
{
public:
    Node* 	left;
    Node*   right;
    
public:
    explicit DotOperatorNode(Node* _left, Node* _arrayexpr) : Node(), left(_left), right(_arrayexpr)
    {
        allNodeSetup();
        nodeName = "DotOperatorNode";
        
        left->parent = this;
        right->parent = this;
        nodetype = DotOperatorNode_t;
        nodeLocation=0;
    }
    
    virtual ~DotOperatorNode()
    {
        delete left;
        delete right;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        
        left->print(os, depth);
        os <<".";
        right->print(os, depth);

    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return left;
        }
        if(i == 1)
        {
            return right;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        return 2;
    }
    
    Node * deepCopy()
    {
        Node * t_left = left->deepCopy();
        Node * t_right = right->deepCopy();
        Node * t_node = new DotOperatorNode(t_left,t_right);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete left;}
            left=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete right;}
            right=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        addError(nodeLocation->fname,nodeLocation->position,"Cannot use \".\" in typdef, variable, or wireamount declarations.");
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return dotTypeCheck(this,vc,tm,"");
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class OutputNode : public Node
{
public:
    Node* 	party;
    Node*   type;
    
public:
    explicit OutputNode(Node* _left, Node* _arrayexpr) : Node(), party(_left), type(_arrayexpr)
    {
        allNodeSetup();
        nodeName = "OutputNode";
        
        party->parent = this;
        type->parent = this;
        nodetype = OutputNode_t;
        nodeLocation=0;
    }
    
    virtual ~OutputNode()
    {
        delete party;
        delete type;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        os<< "#output ";
        party->print(os, depth);
        os <<" ";
        type->print(os, depth);
        os<< "\n";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return party;
        }
        if(i == 1)
        {
            return type;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        return 2;
    }
    
    Node * deepCopy()
    {
        Node * t_party = party->deepCopy();
        Node * t_type = type->deepCopy();
        
        OutputNode * t_node= new OutputNode(t_party,t_type);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete party;}
            party=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete type;}
            type=newnode;
            return 1;
        }
        
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return InputOutputTypeCheck(this,vc,tm);
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class DefineNode : public Node
{
public:
    Node* 	name;
    Node*   expression;
    
public:
    explicit DefineNode(Node* _left, Node* _arrayexpr) : Node(), name(_left), expression(_arrayexpr)
    {
        allNodeSetup();
        nodeName = "DefineNode";
        
        name->parent = this;
        expression->parent = this;
        nodetype = DefineNode_t;
        nodeLocation=0;
    }
    
    virtual ~DefineNode()
    {
        delete name;
        delete expression;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        
        os<< "#define ";
        name->print(os, depth);
        os <<" ";
        expression->print(os, depth);
        os<< "\n";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return name;
        }
        if(i == 1)
        {
            return expression;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        return 2;
    }
    
    Node * deepCopy()
    {
        Node * t_name = name->deepCopy();
        Node * t_expression = expression->deepCopy();
        
        DefineNode * t_node= new DefineNode(t_name,t_expression);
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete name;}
            name=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete expression;}
            expression=newnode;
            return 1;
        }
        
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return getNoType();
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class IncludeNode : public Node
{
public:
    Node* 	filename;
    
public:
    explicit IncludeNode(Node* _left) : Node(), filename(_left)
    {
        allNodeSetup();
        nodeName = "IncludeNode";
        
        filename->parent = this;
        nodetype = IncludeNode_t;
        nodeLocation=0;
    }
    
    virtual ~IncludeNode()
    {
        delete filename;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        os<< "#include ";
        filename->print(os, depth);
        os<< "\n";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return filename;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        return 1;
    }
    
    Node * deepCopy()
    {
        Node * t_filename = filename->deepCopy();
        
        IncludeNode * t_node= new IncludeNode(t_filename);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete filename;}
            filename=newnode;
            return 1;
        }

        
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return getNoType();
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class VarDeclarationListNode : public Node
{
public:
    vector<Node *> nodeList;
    
public:
    explicit VarDeclarationListNode() : Node()
    {
        allNodeSetup();
        nodeName = "VarDeclarationListNode";
        
        nodetype = VarDeclarationListNode_t;
        nodeLocation=0;
    }
    
    virtual ~VarDeclarationListNode()
    {
        for(int i=0;i<nodeList.size();i++)
        {
            delete nodeList[i];
        }
    }
    void addNode(Node * node)
    {
        node->parent = this;
        nodeList.push_back(node);
    }
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        for(int i=0;i<nodeList.size();i++)
        {
            nodeList[i]->print(os,depth);
        }
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        return nodeList[i];
    }
    
    int getNumChildren()
    {
        return nodeList.size();
    }
    
    Node * deepCopy()
    {
        VarDeclarationListNode * t_node = new VarDeclarationListNode();
        t_node->nodeList.resize(nodeList.size());
        for(int i=0;i<nodeList.size();i++)
        {
            t_node->nodeList[i] = nodeList[i]->deepCopy();
        }
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(deleteOldChild){ delete nodeList[i];}
        nodeList[i]=newnode;
        return 1;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        for(int i=0;i<nodeList.size();i++)
        {
            nodeList[i]->typeCheck(vc, tm);
        }
        return getNoType();
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class ArrayDeclarationListNode : public Node
{
public:
    vector<Node *> nodeList;
    
public:
    explicit ArrayDeclarationListNode() : Node()
    {
        allNodeSetup();
        nodeName = "ArrayDeclarationListNode";

        nodetype = ArrayDeclarationListNode_t;
        nodeLocation=0;
    }
    
    virtual ~ArrayDeclarationListNode()
    {
        for(int i=0;i<nodeList.size();i++)
        {
            delete nodeList[i];
        }
    }
    void addNode(Node * node)
    {
        node->parent = this;
        nodeList.push_back(node);
    }
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        for(int i=0;i<nodeList.size();i++)
        {
            nodeList[i]->print(os,depth);
        }
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        return nodeList[i];
    }
    
    int getNumChildren()
    {
        return nodeList.size();
    }
    
    Node * deepCopy()
    {
        ArrayDeclarationListNode * t_node = new ArrayDeclarationListNode();
        t_node->nodeList.resize(nodeList.size());
        for(int i=0;i<nodeList.size();i++)
        {
            t_node->nodeList[i] = nodeList[i]->deepCopy();
        }
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(deleteOldChild){ delete nodeList[i];}
        nodeList[i]=newnode;
        return 1;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        for(int i=0;i<nodeList.size();i++)
        {
            Type * t =  nodeList[i]->typeCheck(vc, tm);
            
        }
        return getNoType();
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class StatementListNode : public Node
{
public:
    vector<Node *> nodeList;
    
public:
    explicit StatementListNode() : Node()
    {
        allNodeSetup();
        nodeName = "StatementListNode";
        
        nodetype = StatementListNode_t;
        nodeLocation=0;
    }
    
    virtual ~StatementListNode()
    {
        for(int i=0;i<nodeList.size();i++)
        {
            delete nodeList[i];
        }
    }
    
    void addNode(Node * node)
    {
        node->parent = this;
        nodeList.push_back(node);
    }
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        for(int i=0;i<nodeList.size();i++)
        {
            nodeList[i]->print(os,depth);
        }
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        return nodeList[i];
    }
    
    int getNumChildren()
    {
        return nodeList.size();
    }
    
    Node * deepCopy()
    {
        StatementListNode * t_node = new StatementListNode();
        t_node->nodeList.resize(nodeList.size());
        for(int i=0;i<nodeList.size();i++)
        {
            t_node->nodeList[i] = nodeList[i]->deepCopy();
        }
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(deleteOldChild){ delete nodeList[i];}
        nodeList[i]=newnode;
        return 1;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        for(int i=0;i<nodeList.size();i++)
        {
            nodeList[i]->typeCheck(vc, tm);
        }
        return getNoType();
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class DeclarationCommaListNode : public Node
{
public:
    vector<Node *> nodeList;
    
public:
    explicit DeclarationCommaListNode() : Node()
    {
        allNodeSetup();
        nodeName = "DeclarationCommaListNode";
        
        nodetype = DeclarationCommaListNode_t;
        nodeLocation=0;
    }
    
    virtual ~DeclarationCommaListNode()
    {
        for(int i=0;i<nodeList.size();i++)
        {
            delete nodeList[i];
        }
    }
    
    void addNode(Node * node)
    {
        node->parent = this;
        nodeList.push_back(node);
    }
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        for(int i=0;i<nodeList.size();i++)
        {
            nodeList[i]->print(os,depth);
            
            if(i != nodeList.size() - 1)
            {
                os <<", ";
            }
        }
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        return nodeList[i];
    }
    
    int getNumChildren()
    {
        return nodeList.size();
    }
    
    Node * deepCopy()
    {
        DeclarationCommaListNode * t_node = new DeclarationCommaListNode();
        t_node->nodeList.resize(nodeList.size());
        for(int i=0;i<nodeList.size();i++)
        {
            t_node->nodeList[i] = nodeList[i]->deepCopy();
        }
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(deleteOldChild){ delete nodeList[i];}
        nodeList[i]=newnode;
        return 1;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        for(int i=0;i<nodeList.size();i++)
        {
            nodeList[i]->typeCheck(vc, tm);
        }
        return getNoType();
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class ArrayInitListNode : public Node
{
public:
    vector<Node *> nodeList;
    
public:
    explicit ArrayInitListNode() : Node()
    {
        allNodeSetup();
        nodeName = "ArrayInitListNode";

        nodetype = ArrayInitListNode_t;
        nodeLocation=0;
    }
    
    virtual ~ArrayInitListNode()
    {
        for(int i=0;i<nodeList.size();i++)
        {
            delete nodeList[i];
        }
    }
    
    void addNode(Node * node)
    {
        node->parent = this;
        nodeList.push_back(node);
    }
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        for(int i=0;i<nodeList.size();i++)
        {
            nodeList[i]->print(os,depth);
            
            if(i != nodeList.size() - 1)
            {
                os <<", ";
            }
        }
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        return nodeList[i];
    }
    
    int getNumChildren()
    {
        return nodeList.size();
    }
    
    Node * deepCopy()
    {
        ArrayInitListNode * t_node = new ArrayInitListNode();
        t_node->nodeList.resize(nodeList.size());
        for(int i=0;i<nodeList.size();i++)
        {
            t_node->nodeList[i] = nodeList[i]->deepCopy();
        }
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(deleteOldChild){ delete nodeList[i];}
        nodeList[i]=newnode;
        return 1;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
 
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * first= nodeList[0]->typeCheck(vc, tm);
        
        for(int i=1;i<nodeList.size();i++)
        {
            Type * second = nodeList[i]->typeCheck(vc, tm);
            
            int res = compareTypes(first,second);
            
            if(res == 0)
            {
                std::stringstream streaml,streamr;
                
                first->print(streaml,0);
                second->print(streamr,0);
                
                addError(nodeLocation->fname,nodeLocation->position,"Array Initialization is not all the same, recieved: \""+streaml.str()+"\" and \""+streamr.str()+"\"");
            }
        }
        
        return first;
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class FunctionArgListNode : public Node
{
public:
    vector<Node *> nodeList;
    
public:
    explicit FunctionArgListNode() : Node()
    {
        allNodeSetup();
        nodeName = "FunctionArgListNode";
        
        nodetype = FunctionArgListNode_t;
        nodeLocation=0;
    }
    
    virtual ~FunctionArgListNode()
    {
        for(int i=0;i<nodeList.size();i++)
        {
            delete nodeList[i];
        }
    }
    
 
    void addNode(Node * node)
    {
        node->parent = this;
        nodeList.push_back(node);
    }
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        for(int i=0;i<nodeList.size();i++)
        {
            nodeList[i]->print(os,depth);
            
            if(i != nodeList.size() - 1)
            {
                os <<", ";
            }
        }
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        return nodeList[i];
    }
    
    int getNumChildren()
    {
        return nodeList.size();
    }
    
    Node * deepCopy()
    {
        FunctionArgListNode * t_node = new FunctionArgListNode();
        t_node->nodeList.resize(nodeList.size());
        for(int i=0;i<nodeList.size();i++)
        {
            t_node->nodeList[i] = nodeList[i]->deepCopy();
        }
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(deleteOldChild){ delete nodeList[i];}
        nodeList[i]=newnode;
        return 1;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        for(int i=0;i<nodeList.size();i++)
        {
            nodeList[i]->typeCheck(vc, tm);
        }
        return getNoType();
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class DeclarationArgCommaListNode : public Node
{
public:
    vector<Node *> nodeList;
    
public:
    explicit DeclarationArgCommaListNode() : Node()
    {
        allNodeSetup();
        nodeName = "DeclarationArgCommaListNode";
        nodetype = DeclarationArgCommaListNode_t;
        nodeLocation=0;
    }
    
    virtual ~DeclarationArgCommaListNode()
    {
        for(int i=0;i<nodeList.size();i++)
        {
            delete nodeList[i];
        }
    }
    
 
    
    void addNode(Node * node)
    {
        node->parent = this;
        nodeList.push_back(node);
    }
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        for(int i=0;i<nodeList.size();i++)
        {
            nodeList[i]->print(os,depth);
            
            if(i != nodeList.size() - 1)
            {
                os <<", ";
            }
        }
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        return nodeList[i];
    }
    
    int getNumChildren()
    {
        return nodeList.size();
    }
    
    Node * deepCopy()
    {
        DeclarationArgCommaListNode * t_node = new DeclarationArgCommaListNode();
        t_node->nodeList.resize(nodeList.size());
        for(int i=0;i<nodeList.size();i++)
        {
            t_node->nodeList[i] = nodeList[i]->deepCopy();
        }
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(deleteOldChild){ delete nodeList[i];}
        nodeList[i]=newnode;
        return 1;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        for(int i=0;i<nodeList.size();i++)
        {
            nodeList[i]->typeCheck(vc, tm);
        }
        return getNoType();
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class ProgramListNode : public Node
{
public:
    vector<Node *> nodeList;
    
public:
    explicit ProgramListNode() : Node()
    {
        allNodeSetup();
        nodeName = "ProgramListNode";

        nodetype = ProgramListNode_t;
        nodeLocation=0;
    }
    

    
    virtual ~ProgramListNode()
    {

        for(int i=0;i<nodeList.size();i++)
        {
            delete nodeList[i];
        }
    }
    
    void addNode(Node * node)
    {
        node->parent = this;
        nodeList.push_back(node);
    }

    virtual void print(std::ostream &os, unsigned int depth) const
    {
        
        for(int i=0;i<nodeList.size();i++)
        {
            nodeList[i]->print(os,depth);
        }
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        return nodeList[i];
    }
    
    int getNumChildren()
    {
        return nodeList.size();
    }
    
    Node * deepCopy()
    {
        ProgramListNode * t_node = new ProgramListNode();
        t_node->nodeList.resize(nodeList.size());
        for(int i=0;i<nodeList.size();i++)
        {
            t_node->nodeList[i] = nodeList[i]->deepCopy();
        }
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(deleteOldChild){ delete nodeList[i];}
        nodeList[i]=newnode;
        return 1;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        for(int i=0;i<nodeList.size();i++)
        {
            if(nodeList[i]->getNodeType() == FunctionDeclarationNode_t || nodeList[i]->getNodeType() == InputNode_t || nodeList[i]->getNodeType() == OutputNode_t)
            {
                //cout << nodeList[i]->nodeName<<"\n";
                //if(nodeList[i]->getNodeType() == InputNode_t || nodeList[i]->getNodeType() == OutputNode_t) nodeList[i]->print(cout,0);
                nodeList[i]->typeCheck(vc, tm);
            }
        }
        return getNoType();
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class FunctionReturnTypeNode : public Node
{
public:
    Node* 	type;
    Node* 	arrays;
    
public:
    explicit FunctionReturnTypeNode(Node* _left, Node* _right) : Node(), type(_left), arrays(_right)
    {
        allNodeSetup();
        nodeName = "FunctionReturnTypeNode";
        
        type->parent = this;
        
        if(arrays!= 0)
        {
            arrays->parent = this;
        }
        
        nodetype = FunctionReturnTypeNode_t;
        nodeLocation=0;
    }
    
    virtual ~FunctionReturnTypeNode()
    {
        delete type;
        
        if(arrays != 0)
        {
            delete arrays;
        }
    }
    
    
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        type->print(os, depth);
        if(arrays != 0)
        {
            os<< " ";
            arrays->print(os, depth);
        }
    }
    
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return type;
        }
        if(i == 1)
        {
            return arrays;
        }
        return 0;
    }
    
    int getNumChildren(){ if(arrays == 0 ) { return 1;} return 2;}
    
    Node * deepCopy()
    {
        Node * t_type = type->deepCopy();
        Node * t_arrays=0;
        
        if(arrays != 0)
        {
           t_arrays = arrays->deepCopy();
        }
        
        Node * t_node = new FunctionReturnTypeNode(t_type,t_arrays);
        
        
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete type;}
            type=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete arrays;}
            arrays=newnode;
            return 1;
        }
        return 0;
    }
    
    
    long getLongResultNoVar()
    {
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return getNoType();
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class FunctionDeclarationNode : public Node
{
public:
    Node*   name;
    Node* 	param;
    Node*   stmts;
    Node*   returntype;
    
public:
    explicit FunctionDeclarationNode(Node* _name, Node* _param, Node* _stmts,Node * _returntype) : Node(), name(_name), param(_param), stmts(_stmts), returntype(_returntype)
    {
        allNodeSetup();
        nodeName = "FunctionDeclarationNode";
        
        name->parent = this;
        if(param != 0)
        {
            param->parent = this;
        }
        stmts->parent = this;
        returntype->parent = this;
        
        nodetype = FunctionDeclarationNode_t;
        nodeLocation=0;
        givenrecwarning = false;
    }
    
    virtual ~FunctionDeclarationNode()
    {
        delete name;
        if(param != 0)
        {
            delete param;
        }
        delete stmts;
        delete returntype;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        os <<"\nfunction ";
        returntype->print(os,depth);
        os<<" ";
        name->print(os,depth);
        os <<"(";
        if(param != 0)
        {
            param->print(os,depth);
        }
        os <<")\n";
        stmts->print(os, depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return returntype;
        }
        
        if(i == 1)
        {
            return name;
        }
        
        if(i == 2)
        {
            if(param == 0)
            {
                return stmts;
            }
            else
            {
                return param;
            }
        }
        
        if(i == 3)
        {
            return stmts;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        if(param == 0)
        {
            return 3;
        }
        return 4;
    }
    
    Node * deepCopy()
    {
        Node * t_returntype = returntype->deepCopy();
        Node * t_name = name->deepCopy();
        Node * t_param=0;
        
        if(param != 0)
        {
            t_param = param->deepCopy();
        }
        Node * t_stmts = stmts->deepCopy();
        
        
        FunctionDeclarationNode * t_node= new FunctionDeclarationNode(t_name,t_param,t_stmts,t_returntype);
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }

    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete returntype;}
            returntype=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete name;}
            name=newnode;
            return 1;
        }
        
        if(i == 2)
        {
            if(param == 0)
            {
                if(deleteOldChild){ delete stmts;}
                stmts=newnode;
                return 1;
            }
            else
            {
                if(deleteOldChild){ delete param;}
                param=newnode;
                return 1;
            }
        }
        
        if(i == 3)
        {
            if(deleteOldChild){ delete stmts;}
            stmts=newnode;
            return 1;
        }
        
        
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return unaryTypeCheckFunctionDeclaration(this,vc,tm,"");
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
    bool givenrecwarning;
};


class ForNode : public Node
{
public:
    Node*   declaration;
    Node* 	condition;
    Node*   increment;
    Node*   stmts;
    
    
    
public:
    explicit ForNode(Node* _left, Node* _center, Node* _right,Node* _stmts) : Node(), declaration(_left), condition(_center), increment(_right), stmts(_stmts)
    {
        allNodeSetup();
        nodeName = "ForNode";
        
        declaration->parent = this;
        condition->parent = this;
        stmts->parent = this;
        increment->parent = this;
        
        nodetype = ForNode_t;
        nodeLocation=0;
        
        has_procs=false;
    }
    
    virtual ~ForNode()
    {
        delete declaration;
        delete condition;
        delete increment;
        delete stmts;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        os <<indent(depth)<<"for(";
        declaration->print(os,depth);
        os <<"; ";
        condition->print(os,depth);
        os <<"; ";
        increment->print(os,depth);
        os<<")\n";
        stmts->addedextradepth = true;
        stmts->print(os,depth+1);

    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return declaration;
        }
        
        if(i == 1)
        {
            return condition;
        }
        
        if(i == 2)
        {
            return increment;
        }
        
        if(i == 3)
        {
            return stmts;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {

        return 4;
    }
    
    Node * deepCopy()
    {
        Node * t_declaration = declaration->deepCopy();
        Node * t_condition = condition->deepCopy();
        Node * t_increment= increment->deepCopy();
        Node * t_stmts = stmts->deepCopy();
        
        ForNode * t_node= new ForNode(t_declaration,t_condition,t_increment,t_stmts);
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete declaration;}
            declaration=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete condition;}
            condition=newnode;
            return 1;
        }
        if(i == 2)
        {
            if(deleteOldChild){ delete increment;}
            increment=newnode;
            return 1;
        }
        if(i == 3)
        {
            if(deleteOldChild){ delete stmts;}
            stmts=newnode;
            return 1;
        }
        
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return forTypeCheck(this,vc,tm,"");
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class IfNode : public Node
{
public:
    Node*   cond;
    Node* 	stmts;
    Node*   elsestmts;
    
public:
    explicit IfNode(Node* _cond, Node* _stmts, Node* _elsestmtss) : Node(), cond(_cond), stmts(_stmts), elsestmts(_elsestmtss)
    {
        allNodeSetup();
        nodeName = "IfNode";
        
        cond->parent = this;
        stmts->parent = this;
        if(elsestmts!=0)
        {
            elsestmts->parent = this;
        }
        
        nodetype = IfNode_t;
        nodeLocation=0;
    }
    
    virtual ~IfNode()
    {
        delete cond;
        delete stmts;
        
        if(elsestmts != 0)
        {
            delete elsestmts;
        }
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        os << indent(depth);
        os <<"if(";
        cond->print(os,depth);
        os <<")\n";
        
        stmts->addedextradepth = true;
        stmts->print(os,depth+1);
        
        if(elsestmts != 0)
        {
            os << indent(depth);
            os <<"else\n";
            
            elsestmts->addedextradepth = true;
            elsestmts->print(os,depth+1);
        }

    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return cond;
        }
        if(i == 1)
        {
            return stmts;
        }
        if(i == 2)
        {
            return elsestmts;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        if(elsestmts == 0)
        {
            return 2;
        }
        return 3;
    }
    
    Node * deepCopy()
    {
        Node * t_cond = cond->deepCopy();
        Node * t_stmts = stmts->deepCopy();
        Node * t_elsestmts=0;
        
        if(elsestmts != 0)
        {
            t_elsestmts = elsestmts->deepCopy();
        }
        
        
        IfNode * t_node= new IfNode(t_cond,t_stmts,t_elsestmts);
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete cond;}
            cond=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete stmts;}
            stmts=newnode;
            return 1;
        }
        if(i == 2)
        {
            if(deleteOldChild){ delete elsestmts;}
            elsestmts=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return ifTypeCheck(this,vc,tm,"");
    }
    
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class CompoundStatementNode : public Node
{
public:
    Node* 	stmt;
    bool isproc;
    long timesgenerated;
    int procnumber;
    
    VariableContext * vctt;
    
    unsigned long long procxorgates;
    unsigned long long procnonxorgates;
    
public:
    explicit CompoundStatementNode(Node* _left, bool isproc_in) : Node(), stmt(_left)
    {
        allNodeSetup();
        nodeName = "CompoundStatementNode";
        
        if(stmt != 0)
        {
            stmt->parent = this;
        }
        
        nodetype = CompoundStatementNode_t;
        nodeLocation=0;
        isproc=isproc_in;
        timesgenerated=0;
        procxorgates=0;
        procnonxorgates=0;
        vctt=0;
        
    }
    
    virtual ~CompoundStatementNode()
    {
        delete vctt;
        delete stmt;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        if(addedextradepth)
        {
            depth--;
        }
        if(gatesFromNode != 0 || gatesFromNodeXor != 0)
        {
            os<<indent(depth) << "<" <<gatesFromNode<<","<<gatesFromNodeXor<<">" "{\n";
        }
        else
        {
            os<<indent(depth) << "{\n";
        }
        if(stmt != 0)
        {
            stmt->print(os, depth+1);
        }
        os<<indent(depth) <<"}\n";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return stmt;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        if(stmt == 0)
        {
            return 0;
        }
        return 1;
    }
    
    Node * deepCopy()
    {
        Node * t_stmt=0;
        if(stmt != 0)
        {
            t_stmt = stmt->deepCopy();
        }
        else
        {
            t_stmt = 0;
        }
        CompoundStatementNode * t_node = new CompoundStatementNode(t_stmt,isproc);
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }

    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(stmt!= 0)
            {
                if(deleteOldChild){ delete stmt;}
            }
            stmt=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        if(stmt != 0)
        {
            VariableContext * vct = new VariableContext();
            
            for ( std::unordered_map<string,Variable *>::iterator it = vc->begin(); it!= vc->end(); ++it )
            {
                (*vct)[it->first] = it->second;
            }
            
            stmt->typeCheck(vct, tm);
            
            delete vct;
            
        }
        return getNoType();
    }
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class ExpressionStatementNode : public Node
{
public:
    Node* 	operand;
    
public:
    explicit ExpressionStatementNode(Node* _left) : Node(), operand(_left)
    {
        allNodeSetup();
        nodeName = "ExpressionStatementNode";
        
        if(operand != 0)
        {
            operand->parent = this;
        }
        
        nodetype = ExpressionStatementNode_t;
        nodeLocation=0;
    }
    
    virtual ~ExpressionStatementNode()
    {
        if(operand != 0)
        {
            delete operand;
        }
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        os << indent(depth);
        
        if(operand != 0)
        {
            operand->print(os, depth);
        }
        
        os<<";\n";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return operand;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        if(operand == 0)
        {
            return 0;
        }
        return 1;
    }
    
    Node * deepCopy()
    {
        Node * t_operand=0;
        if(operand != 0)
        {
            t_operand = operand->deepCopy();
        }
        else
        {
            t_operand = 0;
        }
        Node * t_node = new ExpressionStatementNode(t_operand);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(operand!= 0)
            {
                if(deleteOldChild){ delete operand;}
            }
            operand=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        if(operand != 0)
        {
            operand->typeCheck(vc, tm);
        }
        
        return getNoType();
    }
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class VarDeclarationNode : public Node
{
public:
    Node* 	each_var;
    Node*   type;
    
public:
    explicit VarDeclarationNode(Node* _type, Node* _right) : Node(), each_var(_right), type(_type)
    {
        allNodeSetup();
        nodeName = "VarDeclarationNode";
        
        each_var->parent = this;
        type->parent = this;
        
        nodetype = VarDeclarationNode_t;
        nodeLocation=0;
    }
    
    virtual ~VarDeclarationNode()
    {
        delete type;
        delete each_var;
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        os << indent(depth);
        type->print(os,depth);
        os <<" ";
        each_var->print(os,depth);
        os <<";\n";
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return type;
        }
        
        if(i == 1)
        {
            return each_var;
        }
        
        return 0;
    }

    int getNumChildren()
    {
        return 2;
    }
    
    Node * deepCopy()
    {
        Node * t_type = type->deepCopy();
        Node * t_each_var = each_var->deepCopy();
        VarDeclarationNode * t_node = new VarDeclarationNode(t_type,t_each_var);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete type;}
            type=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete each_var;}
            each_var=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        Type * t =  unaryTypeCheckDeclarationVar(this,vc,tm,"");
        checkSign(t);
        return t;
    }
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};

class VarDeclarationForNode : public Node
{
public:
    Node* 	each_var;
    Node*   type;
    
public:
    explicit VarDeclarationForNode(Node* _type, Node* _right) : Node(), each_var(_right), type(_type)
    {
        allNodeSetup();
        nodeName = "VarDeclarationForNode";
        
        each_var->parent = this;
        type->parent = this;
        
        nodetype = VarDeclarationForNode_t;
        nodeLocation=0;
    }
    
    virtual ~VarDeclarationForNode()
    {
        if(type != 0)
        {
            delete type;
        }
        if(each_var != 0)
        {
            delete each_var;
        }
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        type->print(os,depth);
        os <<" ";
        each_var->print(os,depth);
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return type;
        }
        
        if(i == 1)
        {
            return each_var;
        }
        
        return 0;
    }
    
    int getNumChildren()
    {
        return 2;
    }
    
    Node * deepCopy()
    {
        Node * t_type = type->deepCopy();
        Node * t_each_var = each_var->deepCopy();
        VarDeclarationForNode * t_node = new VarDeclarationForNode(t_type,t_each_var);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete type;}
            type=newnode;
            return 1;
        }
        if(i == 1)
        {
            if(deleteOldChild){ delete each_var;}
            each_var=newnode;
            return 1;
        }
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return getNoType();
    }
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};


class VarDeclarationPrimNode : public Node
{
public:
    Node* 	name;
    Node*   init_expression;
    Node*   array_dec;
    
public:
    explicit VarDeclarationPrimNode(Node* _name, Node*  _arrayDec, Node* _init) : Node(), init_expression(_init), name(_name), array_dec(_arrayDec)
    {
        allNodeSetup();
        nodeName = "VarDeclarationPrimNode";
        
        if(init_expression != 0)
        {
            init_expression->parent = this;
        }
        if(array_dec != 0)
        {
            array_dec->parent = this;
        }
        name->parent = this;
        
        nodetype = VarDeclarationPrimNode_t;
        nodeLocation=0;
    }
    
    virtual ~VarDeclarationPrimNode()
    {
        delete name;
        
        if(init_expression != 0)
        {
            delete init_expression;
        }
        if(array_dec != 0)
        {
            delete array_dec;
        }
    }
    
 
    
    virtual void print(std::ostream &os, unsigned int depth) const
    {
        name->print(os,depth);
        
        if(array_dec != 0)
        {
            array_dec->print(os,depth);
        }
        
        if(init_expression != 0)
        {
            os <<" = ";
            init_expression->print(os,depth);
        }
    }
    
    Node * getChild(int i)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        
        if(i == 0)
        {
            return name;
        }
        
        int count = 1;
        
        if(array_dec != 0)
        {
            if(i == count)
            {
                return array_dec;
            }
            count++;
        }
        
        if(init_expression != 0)
        {
            if(i == count)
            {
                return init_expression;
            }
        }
        
        
        return 0;
    }
    
    int getNumChildren()
    {
        int count = 1;
        
        if(array_dec != 0)
        {
            count++;
        }
        
        if(init_expression != 0)
        {
            count++;
        }
        
        return count;
    }
    
    Node * deepCopy()
    {
        Node * t_name = name->deepCopy();
        Node * t_array_dec=0;
        if(array_dec != 0)
        {
            t_array_dec = array_dec->deepCopy();
        }
        Node * t_init_expression=0;
        if(init_expression)
        {
            t_init_expression = init_expression->deepCopy();
        }
        
        VarDeclarationPrimNode * t_node = new VarDeclarationPrimNode(t_name,t_array_dec,t_init_expression);
        
        t_node->nodeLocation = AstLocation::Duplicate(this->nodeLocation);
        t_node->isSigned = this->isSigned;
        return t_node;
    }
    
    int replaceChild(int i, Node * newnode, bool deleteOldChild)
    {
        if(i >= getNumChildren() || i < 0)
        {
            return 0;
        }
        if(i == 0)
        {
            if(deleteOldChild){ delete name;}
            name=newnode;
            return 1;
        }
        
        int count = 1;
        
        if(array_dec != 0)
        {
            if(i == count)
            {
                if(deleteOldChild){ delete array_dec;}
                array_dec=newnode;
                return 1;
            }
            count++;
        }
        
        if(init_expression != 0)
        {
            if(i == count)
            {
                if(deleteOldChild){ delete init_expression;}
                init_expression=newnode;
                return 1;
            }
        }
        
        return 0;
    }
    
    long getLongResultNoVar()
    {
        cout <<"Warning, computing getLongResultNoVar on a "<<nodeName<<", returning 0\n";
        return 0;
    }
    
    virtual Type * typeCheck(VariableContext * vc, TypeMap * tm)
    {
        return getNoType();
    }
    CORV  circuitOutput(VariableContext *, TypeMap *, int idxF);
};





Node * isNodeNode(Node *n);

BitwiseORNode * isBitwiseORNode(Node *n);

BitwiseANDNode * isBitwiseANDNode(Node *n);

BitwiseXORNode * isBitwiseXORNode(Node *n);

//ConditionalANDNode * isConditionalANDNode(Node *n);

//ConditionalORNode * isConditionalORNode(Node *n);

ArrayDecNode * isArrayDecNode(Node *n);

ArrayInitNode * isArrayInitNode(Node *n);

UnaryNOTNode * isUnaryNOTNode_t(Node *n);

UnaryMinusNode * isUnaryMinusNode(Node *n);

UnaryPostMinusMinusNode * isUnaryPostMinusMinusNode(Node *n);

UnaryPostPlusPlusNode * isUnaryPostPlusPlusNode(Node *n);

UnaryPrePlusPlusNode * isUnaryPrePlusPlusNode(Node *n);

UnaryPreMinusMinusNode * isUnaryPreMinusMinusNode(Node *n);

ArithModuloNode * isArithModuloNode(Node *n);

ArithPlusNode * isArithPlusNode(Node *n);

ArithMinusNode * isArithMinusNode(Node *n);

ArithDivNode * isArithDivNode(Node *n);

ArithMultNode * isArithMultNode(Node *n);

ConditionalLessNode * isConditionalLessNode(Node *n);

ConditionalGreaterNode * isConditionalGreaterNode(Node *n);

ConditionalGreaterEqualNode * isConditionalGreaterEqualNode(Node *n);

ConditionalLessEqualNode * isConditionalLessEqualNode(Node *n);

ConditionalEqualNode * isConditionalEqualNode(Node *n);

ConditionalNotEqualNode * isConditionalNotEqualNode(Node *n);

ShiftLeftNode * isShiftLeftNode(Node *n);

ShiftRightNode * isShiftRightNode(Node *n);

RotateLeftNode * isRotateLeftNode(Node *n);

ParenExprNode * isParenExprNode(Node *n);

TermNode * isTermNode(Node *n);

ArrayAccessNode * isArrayAccessNode(Node *n);

WireAccessNode * isWireAccessNode(Node *n);

AssignNode * isAssignNode(Node *n);

IntTypedefNode * isIntTypedefNode(Node *n);

UIntTypedefNode * isUIntTypedefNode(Node *n);

StructTypedefNode * isStructTypedefNode(Node *n);

DeclarationNode * isDeclarationNode(Node *n);

FunctionVarDeclarationNode * isFunctionVarDeclarationNode(Node *n);

VarDeclarationCommaNode * isVarDeclarationCommaNode(Node *n);

PartiesNode * isPartiesNode(Node *n);

InputNode * isInputNode(Node *n);

FunctionCallNode * isFunctionCallNode(Node *n);

DotOperatorNode * isDotOperatorNode(Node *n);

OutputNode * isOutputNode(Node *n);

DefineNode * isDefineNode(Node *n);

IncludeNode * isIncludeNode(Node *n);

VarDeclarationListNode * isVarDeclarationListNode(Node *n);

ArrayDeclarationListNode * isArrayDeclarationListNode(Node *n);

StatementListNode * isStatementListNode(Node *n);

DeclarationCommaListNode * isDeclarationCommaListNode(Node *n);

ArrayInitListNode * isArrayInitListNode(Node *n);

FunctionArgListNode * isFunctionArgListNode(Node *n);

DeclarationArgCommaListNode * isDeclarationArgCommaListNode(Node *n);

ProgramListNode * isProgramListNode(Node *n);

FunctionDeclarationNode * isFunctionDeclarationNode(Node *n);

ForNode * isForNode(Node *n);

IfNode * isIfNode(Node *n);

CompoundStatementNode * isCompoundStatementNode(Node *n);

ExpressionStatementNode * isExpressionStatementNode(Node *n);

VarDeclarationNode * isVarDeclarationNode(Node *n);

VarDeclarationForNode * isVarDeclarationForNode(Node *n);

VarDeclarationPrimNode * isVarDeclarationPrimNode(Node *n);

ReturnNode * isReturnNode(Node *n);

FunctionReturnTypeNode * isFunctionReturnTypeNode(Node *n);





/** Calculator context used to save the parsed expressions. This context is
 * passed along to the example::Driver class and fill during parsing via bison
 * actions. */
class ProgramContext
{
public:

    /// type of the variable storage
    typedef std::map<std::string, double> variablemap_type;

    /// variable storage. maps variable string to doubles
    variablemap_type		variables;

    /// array of unassigned expressions found by the parser. these are then
    /// outputted to the user.
    std::vector<Node*>	topLevelNodes;

    /// free the saved expression trees
    ~ProgramContext()
    {
        //clearNodes();
    }

    /// free all saved expression trees
    void clearNodes()
    {
        for(unsigned int i = 0; i < topLevelNodes.size(); ++i)
        {
            delete topLevelNodes[i];
        }
        topLevelNodes.clear();
    }

    /// check if the given variable name exists in the storage
    bool existsVariable(const std::string &varname) const
    {
        return variables.find(varname) != variables.end();
    }
    
    /// return the given variable from the storage. throws an exception if it
    /// does not exist.
    double getVariable(const std::string &varname) const
    {
        variablemap_type::const_iterator vi = variables.find(varname);
        if (vi == variables.end())
            throw(std::runtime_error("Unknown variable."));
        else
            return vi->second;
    }
};

#endif // EXPRESSION_H
