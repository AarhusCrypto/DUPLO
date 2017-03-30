
#include <stdio.h>
//#include "ast.h"
#include "types.h"
#include "variable.h"
#include "wire.h"
#include "wirepool.h"
#include "ast.h"
#include <iostream>
#include <fstream>

using namespace std;

class InnerFunction {
	
public:
	Wire * ONE_WIRE;
	Wire * ZERO_WIRE;

	unsigned long one_wire_l;
	unsigned long zero_wire_l;
	int currentbasewire = 0;
	WirePool pool;
	FunctionVariable * v;
	long co_nonxorgates = 0;
	long co_xorgates = 0;
	
	bool isPrintDuploGC = true;
	string strDuploGC;
	string strDuploZeroOne;

	uint32_t functions = 0;
	
	uint32_t outbuffer[16];

	bool printIOTypeWires = true;
	string functionprefix = "";


	
	InnerFunction() {
	}
	
	~InnerFunction()
	{
	}
	string getFunctionPrefix();
	void outputCircuit(VariableContext * vc, TypeMap * tm, Node * selectedNode, string odir);
	string toFullEntry(string s);
	string toTable(int i);
	bool isMainFunction();
	void messyUnlock(Variable * cvar);
	void outputEquals(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv);
	void outputLessThanSigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv);
	void outputSubtract(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv);
	void outputAddition(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv);
	void outputMultSigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv);
	void outputDivideSigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv);
	void outputLessThanUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv);
	void outputMultUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv);
	void outputDivideUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv);

	void outputExMultSigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv);
	void outputExMultUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv);
	void outputReDivideSigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv, int l);
	void outputReDivideUnsigned(vector<Wire *> * leftv, vector<Wire *> * rightv, vector<Wire *> & destv, bool IsModDiv, int l);


	void setTinyFiles(bool b);
	bool getIsTiny();


	int getDepth();
	void increaseDepth();
	void decreaseDepth();

	void printwirevec(vector<Wire *> & v);

	void clearReffedWire(Wire * w);
	Wire * clearWireForReuse(Wire * w);

	void outputFunctionCall(int num);

	void openOutputFile(string s);
	void closeOutputFile();

	void ensureSameSize(vector<Wire *> & w1, vector<Wire *> & w2);
	void ensureTypedSize(vector<Wire *> & w1, vector<Wire *> & w2, Type * t);
	void ensureTypedSize(vector<Wire *> & w1, Type * t);
	void ensureIntVariableToVec(CORV & c);
	void ensureAnyVariableToVec(CORV & c);

	//void outputCircuit(ProgramListNode * topNode, string);

	void ensureSize(vector<Wire *> & w1, int length);
	void putVariableToVector(CORV & c);

	void addComplexOp(short op, int length, int starta, int startb, int startdest, vector<Wire *> a, vector<Wire *> b, vector<Wire *> dest, Wire * carry, int isend);
	void addComplexOpSingleDestBit(short op, int length, int starta, int startb, int startdest, vector<Wire *> a, vector<Wire *> b, vector<Wire *> dest, Wire * carry, int isend);
	void writeComplexGate(short op, int dest, int x, int y, int length, int carryadd, int isend);
	void writeGate(short table, int d, int x, int y, ostream * os);
	void writeGate(short table, int d, int x, int y);//
	void writeCopy(int to, int from);
	void writeFunctionCall(int function, ostream * os);

	long getNonXorGates();//
	long getXorGates();//
	void incrementCountsBy(long nonxor, long xorg);

	
	Wire * addGate(short table, Wire * a, Wire * b, Wire * dest);

	//messyAssign assings from c to pattern
	void messyAssignAndCopy(CORV & c, Variable * pattern);
	void messyAssignAndCopy(Variable * cvar, Variable * pattern);
	void messyMakeWireContainValueNoONEZEROcopy(Variable * pattern);

	Wire * invertWireNoInvertOutput(Wire * w2);
	Wire * invertWire(Wire * other);
	Wire * invertWireNoAllocUnlessNecessary(Wire * other);
	Wire * outputGate(short table, Wire * a, Wire * b);
	void makeONEandZERO(ostream & mos);
	void outputGate(short table, Wire * a, Wire * b, Wire * dest);

	Wire * outputGateNoInvertOutput(short table, Wire * a, Wire * b);
	void outputGateNoInvertOutput(short table, Wire * a, Wire * b, Wire * dest);

	//inOut 0 - input
	//inOut 1 - output
	//party is party
	Wire * outputGate(bool inOut, int party);
	void makeWireNotOther(Wire * w);
	void makeWireContainValueNoONEZEROcopy(Wire * w);
	void makeWireContainValue(Wire * w);

	void makeWireContainValueNoONEZEROcopyTiny(Wire * w);

	void makeWireContainValueNoONEZEROcopyTinyEnd();

	Wire * get_ONE_WIRE();//
	Wire * get_ZERO_WIRE();//
	void assignWire(Wire *  w1, Wire * w2);
	void assignWireCond(Wire *  w1, Wire * w2, Wire * w3); //w3 is whether to assign or not

	
	void setSeeOutput(bool value);
	void setPrintIOTypes(bool value);



	WirePool * getPool();//
	
};
