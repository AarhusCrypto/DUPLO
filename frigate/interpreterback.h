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

/*
#define RDTSC ({unsigned long long res;  unsigned hi, lo;   __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi)); res =  ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );res;})
unsigned long startTime, endTime;
unsigned long startTimeb, endTimeb;

unsigned long freadcount;
unsigned long totalcount;*/

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
using namespace std;


const int MAXBUFELEMENTS=32768;

class Gate
{
public:
    uint8_t table;
    uint32_t dest,x,y;
    uint16_t options;
};


uint8_t * openMMap(string name, long & size)
{
    int m_circuit_fd;
    struct stat statbuf;
    uint8_t * m_ptr_begin;

    if ((m_circuit_fd = open(name.c_str(), O_RDONLY)) < 0)
    {
        perror("can't open file for reading");
        //return false;
    }

    if (fstat(m_circuit_fd, &statbuf) < 0)
    {
        perror("fstat in load_binary failed");
        //return false;
    }
    
    //for empty files
    if(statbuf.st_size == 0)
    {
        size = 0;
        return 0;
    }

    if ((m_ptr_begin = (uint8_t *)mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, m_circuit_fd, 0)) == MAP_FAILED)
    {
        size=0;
        perror("Warning: mmap failed (This should happen if the function does nothing, returns nothing, and has no arguments).");
        return 0;
        //return false;
    }

    uint8_t * m_ptr = m_ptr_begin;
    size = statbuf.st_size;

    return m_ptr;
   
}

class Interpreter
{
    Gate theGate;
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
    
public:
    
    Interpreter(bool plots,bool pIO, bool pheader, bool pstats, bool validation_in, bool outputgatelist_in, string gatelistfile)
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
    }
    
    void readyProgram(string s)
    {
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
            functions[i] = new ifstream(s+"_f"+to_string(i)+".ffrig", ios::in|ios::binary|ios::ate);
            
            char * buf = new char[524288];
            functions[i]->rdbuf()->pubsetbuf(buf, 524288);
            
            
            //functions[i]->open(s+"_f"+to_string(i)+".ffrig", ios::in|ios::binary|ios::ate);
            
            if(!(functions[i]->is_open()))
            {
                cerr <<"error in reading necessary input file: \""+(s+"_f"+to_string(i)+".ffrig")+"\"\nExiting...\n";
                exit(1);
            }
            
            if(printHeaderInfo) cout <<"function "<<i<<" size: "<<functions[i]->tellg()<<"\n";
            
            string sstring =s+"_f"+to_string(i)+".ffrig";
            functionsmm[i] = openMMap(sstring,filesize[i]);
        }
        
        culfile.read((char *)(&memblock[0]),4);
        cfileplace+=4;
        
        memblockmm = (uint32_t *)culfilemm;
        
        maxwire = memblockmm[0];
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
        
        //cfilemax = ;
    }
    
    void closeProgram()
    {
        culfile.close();
        
        for(int i=0;i<numfunctions;i++)
        {
            functions[i]->close();
        }
    }
    
    ifstream * currentFile;
    
    long long readtime;
    
    struct timeval t0b,t1b;
    long long elaps;
    
    inline Gate * getNextGate()
    {
begin:
        
        //startTime = RDTSC;
        
        //currentFile->read((char *)(&memblock[0]),16);
        
        //endTime = RDTSC;
        //freadcount += endTime - startTime;
        
        
        
        
        

        
        
        uint32_t * memblockmm;
        memblockmm = (uint32_t *)currentFilemm;
        
        theGate.dest = memblockmm[0];
        theGate.x = memblockmm[2];
        theGate.y = memblockmm[3];
        theGate.table = memblockmm[1]&0xFF;
        theGate.options = (memblockmm[1]>>8)&0xFFFF;
        currentFilemm+=16;
        
        
        cfileplace+=16;

        //1 is input (d is wire, x is party)
        //2 is output(d is wire, x is party)
        //3 is copy (copies y to x)
        //4 is function (dest holds function number)
        
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
        
        return &theGate;
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
    
    void runprogram()
    {
        //runtestprogram();
        
        elaps=0;
        int * data;
        data = new int[maxwire+1];
        int cinput1;
        int cinput2;
        
        long input1 = 17523466567681;
        long input2 = 0;
        
        for(int i=0;i<maxwire;i++)
        {
            data[i] = 0;
        }
        
        short entry;
        int res;
        
        
        //startTimeb = RDTSC;
        
        
        Gate * g = getNextGate();
        
        long nonxorcount=0;
        long xorcount=0;
        
        long allgates=0;
        
        int validationerrorcount=0;
        int validationsuccesscount=0;
        
        long oneplace,zeroplace;
        bool foundzero=false, foundone =false;
        
        while(currentFilemm != 0)
        {
            
            //gettimeofday(&t0b,0);
            
            switch(g->options)
            {
                    //gate
                case 0:
                    if(printLots)cout << printindent() << g->dest <<" "<<to_string(g->table)<<" "<<g->x<<" "<<g->y<<"\n";
                    
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
                    
                    if(outputgatelist)
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
                        
                        fgatelistfile <<  (int)(g->table)<<" "<<g->dest <<" "<< g->x  <<" "<<g->y<<"\n";
                    }
                    
                    allgates++;
                    
                    break;
                    
                    //input
                case 1:
                    
                    if(g->x == 1)
                    {
                        data[g->dest] = (input1 ) &0x1;
                        input1 = input1 >> 1;
                    }
                    if(g->x == 2)
                    {
                        data[g->dest] = (input2 ) &0x1;
                        input2 = input2 >> 1;
                    }
                    if(printIO) cout << "input "<<data[g->dest] <<" "<<g->dest <<" "<<g->x<<"\n";
                    
                    
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
