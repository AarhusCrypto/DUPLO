//
//  error.cpp
//  
//
//  Created by Benjamin Mood on 11/19/14.
//
//

#include "error.h"
#include <vector>
#include <string>
#include <iostream>

using namespace std;

int errorcount = 0;
string error_filename;

vector<string> errors;

#define KNRM  string("\x1B[0m")
#define KRED  string("\x1B[1;31m")
#define KGRN  string("\x1B[32m")
#define KYEL  string("\x1B[1;33m")
#define KBLU  string("\x1B[34m")
#define KMAG  string("\x1B[35m")
#define KCYN  string("\x1B[36m")
#define KWHT  string("\x1B[37m")
#define KBLACK  string("\x1B[1;30m")

bool setnowarndata=false;

void setNoWarn()
{
    setnowarndata = true;
}

void addError(string file, string position, string error)
{
    errorcount++;
    errors.push_back(KBLACK+file+", "+KRED+"Error "+KBLACK+"line:"+position+" "+KNRM+error+KNRM);
}

void addWarning(string file, string position, string warn)
{
    //don't even add warnings
    if(setnowarndata)
        return;
    
    errors.push_back(KBLACK+file+", "+KYEL+"Warning "+KBLACK+"line:"+position+" "+KNRM+warn+KNRM);
}

void printErrors(std::ostream & os)
{
    for(int i=0;i<errors.size();i++)
    {
        os <<errors[i]<<"\n";
    }
}

bool hasWarning()
{
    return (hasError() == false && errors.size() > 0);
}

bool hasError()
{
    if(errorcount == 0)
    {
        return false;
    }
    
    return true;
}

std::string locationToString(std::string file, std::string position)
{
    return ""+KBLACK+file+", "+KRED+" "+KBLACK+"line:"+position+""+KNRM;
}


void setEFileName(std::string s){error_filename=s; }
std::string getEFileName(){return error_filename;}


