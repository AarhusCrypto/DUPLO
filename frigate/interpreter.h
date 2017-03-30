//
//  interpreter.h
//  
//
//
//

#ifndef ____interpreter__
#define ____interpreter__

#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <unistd.h>


#include <vector>
#include <string>

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
using namespace std;


const int MAXBUFELEMENTS=32768;

class T_Gate
{
public:
    uint8_t table;
    uint32_t dest,x,y;
    uint16_t options;
};


uint8_t * openMMap(string name, long & size);

class Interpreter
{

public:

    T_Gate theGate;
    ifstream culfile;
    
    uint8_t * culfilemm;
    
    
    vector<uint8_t *> theStackmm;
    uint8_t * currentFilemm;
    vector<uint8_t *> functionsmm;
    vector<uint8_t *> functionsStartmm;
    
    vector<ifstream *> theStack;
    int stacksize;
    vector<ifstream *> functions;

    int parties;
    int numfunctions;
    
    vector<int> inputlist;
    vector<int> outputlist;
    
    vector<long> filesize;
    vector<long> filePlaceStack;
    long cfileplace;
    long cfilemax;
    
    int version;
    
    vector<int> fileStack;
    int activefile;
    
    
    char **buffer;
    vector<int> leftInBuffer;
    vector<int> bufferloc;
    
    uint32_t memblock[5000];
    
    long maxwire;
    
    ofstream fgatelistfile;
    bool outputgatelist;
    
    vector<string > inputs;
    vector<int> inputscount;
	
	//duplo
	bool printDuploIO;
	ofstream fDuploIO;
    
public:
    
    Interpreter(bool plots,bool pIO, bool pheader, bool pstats, bool validation_in, bool outputgatelist_in, string gatelistfile, bool dpIO)
    {
        //freadcount=0;
        //totalcount=0;
        cfileplace=0;
        
        stacksize=0;
        maxwire=0;
        printLots=plots;
        printIO = pIO;
        printHeaderInfo=pheader;
        printStats=pstats;
        indent=0;
        readtime=0;
        validation = validation_in;
        
        outputgatelist = outputgatelist_in;
        if(outputgatelist)
        {
            fgatelistfile.open(gatelistfile);
        }
	    printDuploIO = dpIO;
    }
    
    void readyProgram(string s)
    {
  
	    if(printDuploIO)
		    fDuploIO.open(s + ".dpIO"); //to save a real input value + output value
	    
	    
	    gettimeofday(&t0, 0);   
        
        
        stacksize=0;
        
        theStack.resize(0);
        culfile.open(s+"_cul.mfrig", ios::in|ios::binary|ios::ate);
        cfilemax = culfile.tellg();
        
        culfilemm = openMMap(s+"_cul.mfrig", cfilemax);
        uint32_t * memblockmm;
        memblockmm = (uint32_t *)culfilemm;
        
        culfile.seekg (0, ios::beg);
        culfile.read((char *)(&memblock[0]),4);
        cfileplace+=4;
        version = memblockmm[0];
        culfilemm+=4;
        
        if(printHeaderInfo) cout <<"version: "<< version <<"\n";
        
        if(version > 1)
        {
            cerr << "frigate input file is of more updated version than your current source, please aquire new version of the interpreter / compiler\n";
            exit(1);
        }
        
        culfile.read((char *)(&memblock[0]),4);
        memblockmm = (uint32_t *)culfilemm;
        cfileplace+=4;
        parties = memblockmm[0];
        culfilemm+=4;
        
        
        if(printHeaderInfo) cout <<"parties: "<< parties <<"\n";
        
        culfile.read((char *)(&memblock[0]),4);
        cfileplace+=4;
        
        memblockmm = (uint32_t *)culfilemm;
        
        //int numinputs = memblock[0];
        
        
        int numinputs = memblockmm[0];
        culfilemm+=4;
        if(printHeaderInfo) cout <<"numinputs: "<< numinputs <<"\n";
        
        
        inputlist.resize(numinputs*3);
        for(int i=0;i<numinputs;i++)
        {
            culfile.read((char *)(&memblock[0]),12);
            cfileplace+=12;
            
            memblockmm = (uint32_t *)culfilemm;
            
            /*inputlist[i*3+0] = memblock[0];
            inputlist[i*3+1] = memblock[1];
            inputlist[i*3+2] = memblock[2];*/
            
            inputlist[i*3+0] = memblockmm[0];
            inputlist[i*3+1] = memblockmm[1];
            inputlist[i*3+2] = memblockmm[2];
            culfilemm+=12;
        }
        
        
        for(int i=0;i<inputlist.size();i++)
        {
            if(i%3 == 0 && i != 0)
            {
                if(printHeaderInfo) cout << "\n";
            }
            if(printHeaderInfo) cout << inputlist[i]<<" ";

        }
        if(printHeaderInfo) cout << "\n";
        
        culfile.read((char *)(&memblock[0]),4);
        cfileplace+=4;
        
        
        memblockmm = (uint32_t *)culfilemm;
        
        int numoutputs = memblockmm[0];
        culfilemm+=4;
        
        if(printHeaderInfo) cout <<"numoutputs: "<< numoutputs <<"\n";
        
        outputlist.resize(numoutputs*3);
        for(int i=0;i<numoutputs;i++)
        {
            culfile.read((char *)(&memblock[0]),12);
            cfileplace+=12;
            
            memblockmm = (uint32_t *)culfilemm;
            
            outputlist[i*3+0] = memblockmm[0];
            outputlist[i*3+1] = memblockmm[1];
            outputlist[i*3+2] = memblockmm[2];
            
            culfilemm+=12;
        }
        
        for(int i=0;i<outputlist.size();i++)
        {
            if(i%3 == 0 && i != 0)
            {
                if(printHeaderInfo) cout << "\n";
            }
            if(printHeaderInfo) cout << outputlist[i]<<" ";
            
        }
        if(printHeaderInfo) cout << "\n";
        
        culfile.read((char *)(&memblock[0]),4);
        cfileplace+=4;
        
        
        memblockmm = (uint32_t *)culfilemm;
        
        numfunctions = memblock[0];
        culfilemm+=4;
        
        if(printHeaderInfo) cout <<"numfunctions: "<< numfunctions <<"\n";
        
        functions.resize(numfunctions);
        functionsmm.resize(numfunctions);
        filesize.resize(numfunctions+1);
        for(int i=0;i<numfunctions;i++)
        {
            functions[i] = new ifstream(s+"_f"+((to_string(static_cast<long long>(i))))+".ffrig", ios::in|ios::binary|ios::ate);
            
            char * buf = new char[524288];
            functions[i]->rdbuf()->pubsetbuf(buf, 524288);
            
            
            //functions[i]->open(s+"_f"+to_string(i)+".ffrig", ios::in|ios::binary|ios::ate);
            
            if(!(functions[i]->is_open()))
            {
                cerr <<"error in reading necessary input file: \""+(s+"_f"+to_string(static_cast<long long>(i))+".ffrig")+"\"\nExiting...\n";
                exit(1);
            }
            
            if(printHeaderInfo) cout <<"function "<<i<<" size: "<<functions[i]->tellg()<<"\n";
            
            string sstring =s+"_f"+to_string(static_cast<long long>(i))+".ffrig";
            functionsmm[i] = openMMap(sstring,filesize[i]);
        }
        
        culfile.read((char *)(&memblock[0]),4);
        cfileplace+=4;
        
        memblockmm = (uint32_t *)culfilemm;
        
        maxwire = memblockmm[0]+6;
        tiny_reg1 = memblockmm[0]+1;
        tiny_reg2 = memblockmm[0]+2;
        tiny_reg3 = memblockmm[0]+3;
        //tiny_reg4 = memblockmm[0]+4; //changes depending on circuit
        tiny_reg5 = memblockmm[0]+5;
        
        culfilemm+=4;
        
        
        if(printHeaderInfo) cout <<"maxwire: "<< maxwire <<"\n";
        if(printHeaderInfo) cout << "culfile size: " << cfilemax<<"\n";
        currentFile = &culfile;
        currentFilemm =culfilemm;
        
        functionsStartmm.resize(numfunctions+2);
        theStackmm.resize(numfunctions+2);
        theStack.resize(numfunctions+2);
        fileStack.resize(numfunctions+2);
        filePlaceStack.resize(numfunctions+2);
        
        
        leftInBuffer.resize(numfunctions+1);
        bufferloc.resize(numfunctions+1);
        
        activefile = numfunctions;
        
        
        for(int i=0;i<numfunctions;i++)
        {
            functionsStartmm[i] = functionsmm[i];
        }
        
        filesize[numfunctions] = cfilemax;
        
        for(int i=0;i<numfunctions+1;i++)
        {
            leftInBuffer[i]=0;
            bufferloc[i]=0;
        }
        
        buffer = new char*[numfunctions+1];
        
        for(int i=0;i<numfunctions+1;i++)
        {
            buffer[i] = new char[(MAXBUFELEMENTS+5)*16];
        }
        
        in_complex_op=false;
        
        //cfilemax = ;
        
        opplace=0;
        optype = -1;
        
        ifstream inputf("inputs.txt");
        
        inputs.resize(parties);
        inputscount.resize(parties);
        
        //string s2;
        //inputf >> s2;
        //cout << "input is :"<<s2<<"\n";
        
        for(int i=0;i<parties;i++)
        {
            //cout << inputf.eof()<<"\n";
            if(!inputf.eof())
            {
                inputf >> inputs[i] ;
                if(inputs[i].size() == 0)
                {
                    inputs[i] = "0";
                }
            }
            else
            {
                inputs[i] = "0";
            }
            inputscount[i] = 0;
        }
        
        /*for(int i=0;i<parties;i++)
        {
            cout <<"input "<<i<<": "<<inputs[i]<<"||\n";
        }*/
    }
    
    void closeProgram()
    {
        culfile.close();
	    fDuploIO.close();
        
        for(int i=0;i<numfunctions;i++)
        {
            functions[i]->close();
        }
    }
    
    ifstream * currentFile;
    
    long long readtime;
    
    struct timeval t0b,t1b;
    long long elaps;
    
    int tiny_numLeft;
    int tiny_addr1;
    int tiny_addr2;
    int tiny_addrdest;
    int tiny_reg1,tiny_reg2,tiny_reg3,tiny_reg4, tiny_reg5;
    bool in_complex_op;
    int optype;
    int opplace;
    int isend;
    
    inline T_Gate * getNextGate()
    {
begin:
        
        //startTime = RDTSC;
        
        //currentFile->read((char *)(&memblock[0]),16);
        
        //endTime = RDTSC;
        //freadcount += endTime - startTime;
        
        
        if(in_complex_op)
        {
            goto complexop;
        }
        
        

        
        
        uint32_t * memblockmm;
        memblockmm = (uint32_t *)currentFilemm;
        
        theGate.dest = memblockmm[0];
        theGate.x = memblockmm[2];
        theGate.y = memblockmm[3];
        theGate.table = memblockmm[1]&0xFF;
        theGate.options = (memblockmm[1]>>8)&0xFFFF;
        currentFilemm+=16;
        
	   // cout << "+" << theGate.table << "+";
        
        cfileplace+=16;
        
        //0 is gate
        //1 is input (d is wire, x is party)
        //2 is output(d is wire, x is party)
        //3 is copy (copies y to x)
        //4 is function (dest holds function number)
        //5 is complex op
        
        
        //comlpex op
        if(theGate.options == 5)
        {
            //cout <<"op is 5\n";
            
            tiny_addr1 = theGate.x;
            tiny_addr2 = theGate.y;
            tiny_addrdest = theGate.dest;
            opplace = 0;
            
            //second read
            memblockmm = (uint32_t *)currentFilemm;
            theGate.dest = memblockmm[0];
            theGate.x = memblockmm[2];
            theGate.y = memblockmm[3];
            theGate.table = memblockmm[1]&0xFF;
            theGate.options = (memblockmm[1]>>8)&0xFFFF;
            currentFilemm+=16;
            cfileplace+=16;
            
            tiny_reg4 = theGate.dest;
            tiny_numLeft = theGate.x;
            optype = theGate.y;
            isend = theGate.table;
            
            in_complex_op=true;
            
            //cout <<"complex op: \nx:"<< tiny_addr1<<"\ny:"<<tiny_addr2<<"\ndest"<<tiny_addrdest<<"\nlength: "<<tiny_numLeft<<"\ntype: "<<optype<<"\n";
	        cout << "-" << theGate.table << "-";
            
            goto complexop;
        }
        
        
        if(theGate.options == 4)
        {
            //if file is empty then skip it
            if(filesize[theGate.dest] > 0)
            {
                filePlaceStack[stacksize] = cfileplace;
                fileStack[stacksize] = activefile;
                theStackmm[stacksize] = currentFilemm;
                
                stacksize++;
                
                activefile = theGate.dest;
                cfileplace=0;
                cfilemax = filesize[theGate.dest];
                currentFilemm = functionsStartmm[theGate.dest];
            }

        }
        
        
        
        //queue next file
        //startTime = RDTSC;
        while(currentFilemm != 0 && cfileplace > cfilemax)
        {
            //endTime = RDTSC;
            //freadcount += endTime - startTime;
            if(stacksize == 0)
            {
                //currentFile = 0;
                currentFilemm=0;
            }
            else
            {
                stacksize--;
                
                cfileplace = filePlaceStack[stacksize];
                
                currentFilemm = theStackmm[stacksize];
                
                activefile = fileStack[stacksize];
                cfilemax = filesize[activefile];
                goto begin;
            }
        }
        //endTime = RDTSC;
        //freadcount += endTime - startTime;
        
    returnplace:
        return &theGate;
        
        
    /* optype == 
     0 - adder
     1 - subtract
     2 - less than unsigned (can be used for signed as well, though I don't think it ever uses the end case)
     3 - input
     4 - output
     */
     
    complexop:
	    cout << "comp" << endl;
        switch(optype)
        {
            case 0: // adder
                switch(opplace)
                {
                        //xor ab
                    case 0:
                        theGate.dest = tiny_reg1;
                        theGate.x = tiny_addr1;
                        theGate.y = tiny_addr2;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        opplace++;
                        
                        goto returnplace;
                        break;
                        
                        //destv
                    case 1:
                        theGate.dest = tiny_addrdest;
                        theGate.x = tiny_reg4; //carry
                        theGate.y = tiny_reg1;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        if(tiny_numLeft==0 && isend)
                        {
                            //tiny_numLeft--;
                            in_complex_op=false;
                            opplace=0;
                        }
                        else
                        {
                            opplace++;
                        }
                        goto returnplace;
                        break;
                    // xorac
                    case 2:
                        theGate.dest = tiny_reg2;
                        theGate.x = tiny_reg4; //carry
                        theGate.y = tiny_addr1;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        opplace++;
                        
                        goto returnplace;
                        break;
                        
                    //and1
                    case 3:
                        theGate.dest = tiny_reg3;
                        theGate.x = tiny_reg1; //carry
                        theGate.y = tiny_reg2;
                        theGate.table = 8;
                        theGate.options = 0;
                        
                        opplace++;
                        
                        goto returnplace;
                        break;
                        
                        //carry
                    case 4:
                        theGate.dest = tiny_reg4;
                        theGate.x = tiny_reg3; //carry
                        theGate.y = tiny_addr1;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        if(tiny_numLeft==0 && !isend)
                        {
                            in_complex_op=false;
                            opplace=0;
                            goto returnplace;
                        }
                        
                        
                        tiny_numLeft--;
                        tiny_addr1++;
                        tiny_addr2++;
                        tiny_addrdest++;
                        opplace=0;
                        
                        
                        
                        
                        goto returnplace;
                        break;
                        
                        
                }
                
                break;
            case 1: //subtract
                switch(opplace)
                {
                        //invert a
                    case 0:
                        //cout << "a\n" << oneplace <<"\n";
                        theGate.dest = tiny_reg5;
                        theGate.x = tiny_addr1;
                        theGate.y = oneplace;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        opplace++;
                        
                        goto returnplace;
                        
                        //xor !ab
                    case 1:
                        //cout << "b\n";
                        theGate.dest = tiny_reg1;
                        theGate.x = tiny_reg5;
                        theGate.y = tiny_addr2;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        opplace++;
                        
                        goto returnplace;
                        break;
                        
                    case 2:
                        //destv
                        theGate.dest = tiny_reg3;
                        theGate.x = tiny_reg4; //carry
                        theGate.y = oneplace;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        opplace++;
                        
                        goto returnplace;
                        break;
                    case 3:
                        //cout << "c\n";
                        theGate.dest = tiny_addrdest;
                        theGate.x = tiny_reg3; //carry
                        theGate.y = tiny_reg1;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        if(tiny_numLeft==0 && isend)
                        {
                            //tiny_numLeft--;
                            in_complex_op=false;
                            opplace=0;
                        }
                        else
                        {
                            opplace++;
                        }
                        goto returnplace;
                        break;
                        // xorac
                    case 4:
                        //cout << "d\n";
                        theGate.dest = tiny_reg2;
                        theGate.x = tiny_reg4; //carry
                        theGate.y = tiny_reg5;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        opplace++;
                        
                        goto returnplace;
                        break;
                        
                        //and1
                    case 5:
                        //cout << "e\n";
                        theGate.dest = tiny_reg3;
                        theGate.x = tiny_reg1; //carry
                        theGate.y = tiny_reg2;
                        theGate.table = 8;
                        theGate.options = 0;
                        
                        opplace++;
                        
                        goto returnplace;
                        break;
                        
                        //carry
                    case 6:
                        //cout << "f\n";
                        theGate.dest = tiny_reg4;
                        theGate.x = tiny_reg3; //carry
                        theGate.y = tiny_reg5;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        if(tiny_numLeft==0 && !isend)
                        {
                            in_complex_op=false;
                            opplace=0;
                            goto returnplace;
                        }
                        
                        
                        tiny_numLeft--;
                        tiny_addr1++;
                        tiny_addr2++;
                        tiny_addrdest++;
                        opplace=0;
                        
                        
                        
                        
                        goto returnplace;
                        break;
                        
                        
                }
                
                break;
            case 2:
                switch(opplace)
                {
                        //invert a
                    case 0:
                        //cout << "a\n" << oneplace <<"\n";
                        theGate.dest = tiny_reg5;
                        theGate.x = tiny_addr1;
                        theGate.y = oneplace;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        opplace++;
                        
                        goto returnplace;
                        
                        //xor !ab
                    case 1:
                        //cout << "b\n";
                        theGate.dest = tiny_reg1;
                        theGate.x = tiny_reg5;
                        theGate.y = tiny_addr2;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        opplace++;
                        
                        goto returnplace;
                        break;
                        
                   /* case 2:
                        //destv
                        theGate.dest = tiny_reg3;
                        theGate.x = tiny_reg4; //carry
                        theGate.y = oneplace;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        opplace++;
                        
                        goto returnplace;
                        break;
                    case 3:
                        //cout << "c\n";
                        theGate.dest = tiny_addrdest;
                        theGate.x = tiny_reg3; //carry
                        theGate.y = tiny_reg1;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        if(tiny_numLeft==0 && isend)
                        {
                            //tiny_numLeft--;
                            in_complex_op=false;
                            opplace=0;
                        }
                        else
                        {
                            opplace++;
                        }
                        goto returnplace;
                        break;*/
                        // xorac
                    case 2:
                        
                        
                        if(tiny_numLeft==0 && isend)
                        {
                            theGate.dest = tiny_reg3;
                            theGate.x = tiny_reg4; //carry
                            theGate.y = oneplace;
                            theGate.table = 6;
                            theGate.options = 0;
                            
                            
                            opplace++;
                            goto returnplace;
                            break;
                        }
                        
                        
                        //cout << "d\n";
                        theGate.dest = tiny_reg2;
                        theGate.x = tiny_reg4; //carry
                        theGate.y = tiny_reg5;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        opplace++;
                        
                        goto returnplace;
                        break;
                        
                        //and1
                    case 3:
                        
                        if(tiny_numLeft==0 && isend)
                        {
                            theGate.dest = tiny_addrdest;
                            theGate.x = tiny_reg3; //carry
                            theGate.y = tiny_reg1;
                            theGate.table = 6;
                            theGate.options = 0;
                            
                            in_complex_op=false;
                            opplace=0;
                            
                            goto returnplace;
                            break;
                        }
                        
                        
                        //cout << "e\n";
                        theGate.dest = tiny_reg3;
                        theGate.x = tiny_reg1; //carry
                        theGate.y = tiny_reg2;
                        theGate.table = 8;
                        theGate.options = 0;
                        
                        opplace++;
                        
                        goto returnplace;
                        break;
                        
                        //carry
                    case 4:
                        //cout << "f\n";
                        theGate.dest = tiny_reg4;
                        theGate.x = tiny_reg3; //carry
                        theGate.y = tiny_reg5;
                        theGate.table = 6;
                        theGate.options = 0;
                        
                        if(tiny_numLeft==0 && !isend)
                        {
                            in_complex_op=false;
                            opplace=0;
                            goto returnplace;
                        }
                        
                        
                        tiny_numLeft--;
                        tiny_addr1++;
                        tiny_addr2++;
                        opplace=0;

                        
                        goto returnplace;
                        break;
                        
                }
                
                break;
            case 3:
                theGate.dest = tiny_addrdest; //destination
                theGate.x = tiny_addr1; // party
                theGate.y = 0;
                theGate.table = 0;
                theGate.options = 1;
                
                //cout <<"input\n";
                
                if(tiny_numLeft==0)
                {
                    in_complex_op=false;
                    opplace=0;
                    goto returnplace;
                }
                
                //tiny_addr1++;
                tiny_addrdest++;
                tiny_numLeft--;
                
                goto returnplace;
                break;
            case 4:
                theGate.dest = tiny_addrdest; //destination
                theGate.x = tiny_addr1; // party
                theGate.y = 0;
                theGate.table = 0;
                theGate.options = 2;
                
                //cout <<"input\n";
                
                if(tiny_numLeft==0)
                {
                    in_complex_op=false;
                    opplace=0;
                    goto returnplace;
                }
                
                //tiny_addr1++;
                tiny_addrdest++;
                tiny_numLeft--;
                
                goto returnplace;
                break;
            case 5:
                theGate.dest = 0; //destination
                theGate.x = tiny_addrdest; // party
                theGate.y = tiny_addr1;
                theGate.table = 0;
                theGate.options = 3;
                
                //cout <<"input\n";
                
                if(tiny_numLeft==0)
                {
                    in_complex_op=false;
                    opplace=0;
                    goto returnplace;
                }
                
                tiny_addr1++;
                tiny_addrdest++;
                tiny_numLeft--;
                
                goto returnplace;
                break;
        }
        
        goto returnplace;
        
        
        
    }
    
    string printindent()
    {
        string tr="";
        for(int i=0;i<stacksize;i++)
        {
            tr+="--";
        }
        if(stacksize > 0)
        {
        tr+=" ";
        }
        return tr;
    }
    
    string printindentm1()
    {
        string tr="";
        for(int i=0;i<stacksize-1;i++)
        {
            tr+="--";
        }
        if(stacksize > 0)
        {
            tr+=" ";
        }
        return tr;
    }
    
    int indent;
    bool printLots;
    bool printIO;
    bool printHeaderInfo;
    bool printStats;
    bool validation;
    
    struct timeval t0,t1;
    
    long oneplace,zeroplace;
    
    
    void runprogram()
    {
        //runtestprogram();
        
        elaps=0;
        int * data;
        data = new int[maxwire+1];
        int cinput1;
        int cinput2;
        
        long input1 = 10;
        long input2 = 10;
        
        for(int i=0;i<maxwire;i++)
        {
            data[i] = 0;
        }
        
        short entry;
        int res;
        
        oneplace=-1;
        zeroplace=-1;
        
        //startTimeb = RDTSC;
        
        
        T_Gate * g = getNextGate();
        
        long nonxorcount=0;
        long xorcount=0;
        
        long allgates=0;
        
        int validationerrorcount=0;
        int validationsuccesscount=0;
        
        
        bool foundzero=false, foundone =false;
	    
	    string realInp[2], realOut[2];
        
        while(currentFilemm != 0)
        {
            
            //gettimeofday(&t0b,0);
            
            switch(g->options)
            {
                    //gate
                case 0:
	            
                    if(printLots)cout << printindent() << g->dest <<" "<<to_string(static_cast<long long>(g->table))<<" "<<g->x<<" "<<g->y<<"\n";
                    
                     entry = (data[g->x] << 1) | data[g->y];
                    
                    res = (g->table >> entry) & 1;
                    
                    data[g->dest] = res;
                    
                    if(g->table == 6)
                    {
                        xorcount++;
                    }
                    else
                    {
                        nonxorcount++;
                    }
                    
                    //
                    {
                        if(g->table == 15 && !foundone)
                        {
                            oneplace = g->dest;
                            foundone = true;
                        }
                        else if(g->table == 0 && !foundzero)
                        {
                            zeroplace = g->dest;
                            foundzero = true;
                        }
                        
                        if(outputgatelist) fgatelistfile <<  (int)(g->table)<<" "<<g->dest <<" "<< g->x  <<" "<<g->y<<"\n";
                    }
                    
                    allgates++;
                    
                    break;
                    
                    //input
                case 1:
                    
                    /*if(g->x == 1)
                    {
                        
                        
                        data[g->dest] = (input1 ) &0x1;
                        input1 = input1 >> 1;
                    }
                    if(g->x == 2)
                    {
                        data[g->dest] = (input2 ) &0x1;
                        input2 = input2 >> 1;
                    }*/
                    
                    if(inputs[g->x-1].size()-1 >= inputscount[g->x-1])
                    {
                        if(inputs[g->x-1][inputscount[g->x-1]] == '0')
                        {
                            data[g->dest] = 0;
                        }
                        else if(inputs[g->x-1][inputscount[g->x-1]] == '1')
                        {
                            data[g->dest] = 1;
                        }
                        else
                        {
                            cout << "unknown input digit: \""<<inputs[g->x-1][inputscount[g->x-1]]<<"\", should be 0 or 1\n";
                            data[g->dest] = 0;
                        }
                        
                        
                        inputscount[g->x-1]++;
                    }
                    else
                    {
                        data[g->dest] = 0;
                    }
                    
                    
                    if(printIO) cout << "input "<<data[g->dest] <<" "<<g->dest <<" "<<g->x<<"\n";
                    
					if (printDuploIO)
					{
						realInp[g->x - 1].append(to_string(data[g->dest]));
					}
                    if(outputgatelist)
                    {
                        fgatelistfile << "IN "<< g->dest  <<" "<<g->x<<"\n";
                    }
                    
                    xorcount++;
                    break;
                    
                    //output
                case 2:
                    if(validation && (data[g->dest] == 0) && g->x == 1)
                    {
                        cout << "validation test fail at: "<<g->dest<<"\n";
                        validationerrorcount++;
                    }
                    if(validation && (data[g->dest] == 1) && g->x == 1)
                    {
                        //cout << "-----validation test PASSS at: "<<g->dest<<"\n";
                        validationsuccesscount++;
                    }
                    if(printIO && (!validation)) cout << "output "<<data[g->dest] <<" "<<g->dest <<" "<<g->x<<"\n";
	            
					if (printDuploIO&& (!validation))
					{
						realOut[g->x - 1].append(to_string(data[g->dest]));
					}
                    
                    if(outputgatelist)
                    {
                        fgatelistfile << "OUT "<< g->dest  <<" "<<g->x<<"\n";
                    }
                    
                    xorcount++;
                    break;
                    
                    //copy
                case 3:
                    if(printLots)cout << printindent()<< "copy "<<g->x <<" "<<g->y<<"("<<data[g->y]<<")\n";
                    data[g->x] = data[g->y];
                    
                    if(outputgatelist)
                    {
                        fgatelistfile << "copy(6) "<<g->x<< " "<< g->y  <<" "<<zeroplace<<"\n";
                    }
                    
                    xorcount++;
                    break;
                    
                    //function
                case 4:
                    if(printLots)cout << printindentm1()<< "function call "<<g->dest<<"\n";
                    
                    xorcount++;
                    break;
                default:
                    cout << "undefined operation\n";
                    break;
            }
            
            //gettimeofday(&t1b,0);
            //elaps += (t1b.tv_sec-t0b.tv_sec)*1000000LL + t1b.tv_usec-t0b.tv_usec;

            g = getNextGate();
            

            
            
            //cout << g->dest <<" "<<to_string(g->table)<<" "<<g->options<<" "<<g->x<<" "<<g->y<<"\n";
        }
        
        
	    fDuploIO << realInp[0] << "\n";
        fDuploIO << realInp[1] << "\n";
	    fDuploIO << realOut[0] << "\n";
	    fDuploIO << realOut[1] << "\n";
        //endTimeb = RDTSC;
        
        //totalcount += endTimeb - startTimeb;
        
        gettimeofday(&t1, 0);
        if(printStats) cout <<"interpreter:\tgates: nonxor: "<<nonxorcount <<"  free ops: "<< xorcount << "   allgates: "<<allgates<<"\n";
        long long elapsed = (t1.tv_sec-t0.tv_sec)*1000000LL + t1.tv_usec-t0.tv_usec;
        if(printStats && !outputgatelist) cout << "\t\ttime(s): "<< (elapsed*1.0)/1000000 <<"\n";
        else if(printStats) cout << "\t\ttime(s): [this time takes into account file io time to output the program] "<< (elapsed*1.0)/1000000 <<"\n";
        
        if(validation)
        {
            if(validationerrorcount != 0)
            {
                cout <<"\nValidation test failure\n";
            }
            else
            {
                cout <<"\nValidation test pass\n";
            }
            cout << validationsuccesscount <<" tests were passed\n";
            cout << validationerrorcount <<" tests were failed\n\n";
        }
    }
    
    long getMaxWire(){return maxwire;}
};






#endif /* defined(____interpreter__) */
