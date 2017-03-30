//
//  error.h
//  
//
//  Created by Benjamin Mood on 11/19/14.
//
//

#ifndef ____error__
#define ____error__



#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>



void addError(std::string file, std::string line, std::string error);
void addWarning(std::string file, std::string line, std::string warn);
void printErrors(std::ostream & os);
bool hasError();
bool hasWarning();

void setNoWarn();


void setEFileName(std::string s);
std::string getEFileName();

std::string locationToString(std::string filename, std::string line);


#endif /* defined(____error__) */
