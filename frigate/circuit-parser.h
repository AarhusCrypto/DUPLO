//
//  CircuitOutput.h
//  
//

//

#ifndef ____CircuitOutput__
#define ____CircuitOutput__

#include <stdio.h>
#include "ast.h"
#include "types.h"
#include "variable.h"
#include "wire.h"

#include <iostream>
#include <fstream>

using namespace std;


void printDuploGC(bool value);
void appendDuploGC(string value, bool cond);
bool isMainFunction();
void messyUnlock(Variable * cvar);
void outputEquals(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, int idxF);
void outputLessThanSigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, int idxF);
void outputSubtract(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, int idxF);
void outputAddition(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, int idxF);
void outputMultSigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, int idxF);
void outputDivideSigned(vector<Wire *> * leftv,vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv, int idxF);
void outputLessThanUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, int idxF);
void outputMultUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, int idxF);
void outputDivideUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv, int idxF);

void outputExMultSigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, int idxF);
void outputExMultUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, int idxF);
void outputReDivideSigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv, int l, int idxF);
void outputReDivideUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv, int l, int idxF);


void setTinyFiles(bool b);
bool getIsTiny();


int getDepth();
void increaseDepth();
void decreaseDepth();

void printwirevec(vector<Wire *> & v);

void clearReffedWire(Wire * w, int idxF);
void clearReffedWireForFn(Wire * w, int idxF);
Wire * clearWireForReuse(Wire * w, int idxF);

void outputFunctionCall(int num);
void outputFunctionCallDP(int num, string localInp, string globalInp);

void openOutputFile(string s);
void closeOutputFile();

void ensureSameSize(vector<Wire *> & w1, vector<Wire *> & w2, int idxF);
void ensureTypedSize(vector<Wire *> & w1, vector<Wire *> & w2, Type * t, int idxF);
void ensureTypedSize(vector<Wire *> & w1, Type * t, int idxF);
void ensureIntVariableToVec(CORV & c);
void ensureAnyVariableToVec(CORV & c);

void outputCircuitExt(ProgramListNode * topNode, string);

void ensureSize(vector<Wire *> & w1, int length, int idxF);
void putVariableToVector(CORV & c);

void addComplexOp(short op, int length, int starta, int startb, int startdest, vector<Wire *> a, vector<Wire *> b, vector<Wire *> dest, Wire * carry, int isend, int idxF);
void addComplexOpSingleDestBit(short op, int length, int starta, int startb, int startdest, vector<Wire *> a, vector<Wire *> b, vector<Wire *> dest, Wire * carry, int isend, int idxF);
void writeComplexGate(short op, int dest, int x, int y, int length, int carryadd, int isend);
void writeGate(short table, int d, int x, int y,ostream * os, int idxF);
void writeGate(short table, int d, int x, int y, int idxF);
void writeCopy(int to, int from, int idxF);
void writeFunctionCall(int function, ostream * os);

long getNonXorGates();
long getXorGates();
void incrementCountsBy(long nonxor, long xorg);

int getNumberOfFunctions();
void increaseFunctions();
string getFunctionPrefix();

//messyAssign assings from c to pattern
void messyAssignAndCopy(CORV & c, Variable * pattern, int idxF);
string messyAssignAndCopyForFn(CORV & c, Variable * pattern, int idxF);
void messyAssignAndCopy(Variable * cvar, Variable * pattern, int idxF);
void messyAssignAndCopyForFn(Variable * cvar, Variable * pattern, int idxF);
void messyMakeWireContainValueNoONEZEROcopy(Variable * pattern, int idxF);

Wire * invertWireNoInvertOutput(Wire * w2, int idxF);
Wire * invertWire(Wire * other, int idxF);
Wire * invertWireNoAllocUnlessNecessary(Wire * other, int idxF);
Wire * outputGate(short table, Wire * a, Wire * b, int idxF);
void outputGate(short table, Wire * a, Wire * b, Wire * dest, int idxF);

Wire * outputGateNoInvertOutput(short table, Wire * a, Wire * b, int idxF);
void outputGateNoInvertOutput(short table, Wire * a, Wire * b, Wire * dest, int idxF);

//inOut 0 - input
//inOut 1 - output
//party is party
Wire * outputGate(bool inOut, int party);
void makeWireNotOther(Wire * w, int idxF);
void makeWireContainValueNoONEZEROcopy(Wire * w, int idxF);
void makeWireContainValueNoONEZEROcopyForFn(Wire * w, int idxF);
void makeWireContainValue(Wire * w, int idxF);
void makeWireContainValueForFn(Wire * w, int idxF);

void makeWireContainValueNoONEZEROcopyTiny(Wire * w, int idxF);

void makeWireContainValueNoONEZEROcopyTinyEnd();

Wire * get_ONE_WIRE(int idxF);
Wire * get_ZERO_WIRE(int idxF);
void assignWire(Wire *  w1, Wire * w2, int idxF);
void assignWireCond(Wire *  w1, Wire * w2, Wire * w3, int idxF); //w3 is whether to assign or not
void assignWireCondForFn(Wire *  w1, Wire * w2, Wire * w3, int idxF);

void pushOutputFile(string s);
void popOutputFile();

void setSeeOutput(bool value);
void setPrintIOTypes(bool value);

#include "wirepool.h"

WirePool * getPool(int idxF);

#endif /* defined(____CircuitOutput__) */
