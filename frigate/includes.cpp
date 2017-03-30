#include "ast.h"
#include "exprtest.hh"
#include "includes.h"
#include "error.h"

#include <unordered_map>
#include <algorithm>

using namespace std;

string getFilePrefix(string fname)
{
    int pos = fname.find_last_of("/\\");
    
    if(pos == -1)
    {
        return "";
    }
    
    return fname.substr(0,pos+1);
}

std::unordered_map<string,char> include_map;

void includeIncludes(Node * topnode,string originalfilename)
{
    ProgramListNode * pln = (ProgramListNode * )topnode;
    string prefix = getFilePrefix(originalfilename);
 
    for(int i=0;i<pln->nodeList.size();i++)
    {
        IncludeNode * inode = isIncludeNode(pln->nodeList[i]);
        
        if(inode != 0)
        {
            string name = isTermNode(inode->filename)->var;
            

            name = prefix+name;
            
            
            std::unordered_map<string,char>::const_iterator found = include_map.find(name);
            
            if(found == include_map.end())
            {
                //not previously included
                include_map.insert(std::make_pair(name,'a'));
                
            }
            else
            {
                addWarning(inode->nodeLocation->fname,inode->nodeLocation->position,"File "+name +" already included.");
                
                //already included
                for(int j=i;j<pln->nodeList.size()-1;j++)
                {
                    pln->nodeList[j] = pln->nodeList[j+1];
                }
                pln->nodeList.resize(pln->nodeList.size()-1);
                
                
                
                
                i--;
                
                
                delete inode;
                continue;
                
            }
            
            Node * includetop = generateAst(name);
            
            
            if(includetop == 0)
            {
                 addError(inode->nodeLocation->fname,inode->nodeLocation->position,"Included file \""+name +"\" not found.");
                continue;
            }
            
            int origsize = pln->nodeList.size();
            ProgramListNode * plninclude = isProgramListNode(includetop);
            int newsize = pln->nodeList.size()-1+ plninclude->nodeList.size();
            if(origsize > newsize)
            {
                for(int j=i;j<pln->nodeList.size()-1;j++)
                {
                    pln->nodeList[j] = pln->nodeList[j+1];
                }
                pln->nodeList.resize(pln->nodeList.size()-1);
                
                i--;
                delete inode;
                continue;
            }
            
            pln->nodeList.resize(pln->nodeList.size()-1+ plninclude->nodeList.size());
            int diff = pln->nodeList.size() - origsize;
            
            for(int j=pln->nodeList.size()-1;j>i;j--)
            {
                pln->nodeList[j] = pln->nodeList[j-diff];
            }
            
            for(int j=0;j<plninclude->nodeList.size();j++)
            {
                pln->nodeList[i+j] = plninclude->nodeList[j];
            }
            i--;
            delete inode;
        }
    }
}




