#include "BTreeNode.h"
#include <math.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
using namespace std;

#define L_PAIR_SIZE (sizeof(RecordId)+sizeof(int))
#define NL_PAIR_SIZE (sizeof(PageId)+sizeof(int))

 BTLeafNode::BTLeafNode()
 {
    memset(buffer, '\0', PageFile::PAGE_SIZE);
 }

RC BTLeafNode::read(PageId pid, const PageFile& pf)
{ 
    
    return pf.read(pid, buffer);
}

RC BTLeafNode::write(PageId pid, PageFile& pf)
{
    
    return pf.write(pid, buffer);
}

int BTLeafNode::getKeyCount()
{   
    int keyCounter=0;
    char* tmp=buffer;
 
    int curr;
    int i=4;

    while(i<1024)
    {
        memcpy(&curr,tmp,sizeof(int));
        if(curr==0)
            break;
        
        keyCounter++;
        tmp+=L_PAIR_SIZE;
        i+=L_PAIR_SIZE;
    }

    if(keyCounter>1)
    {
        memcpy(&curr,tmp-sizeof(int),sizeof(int));
        if(curr==0)
        {
            memcpy(&curr,tmp-2*sizeof(int),sizeof(int));
            if(curr==0)
            {
                keyCounter--;
            }
        }
    }

    return keyCounter;
}


int BTLeafNode::getMaxLlaves()
{
    
    int parejas_maximas=floor((PageFile::PAGE_SIZE-sizeof(PageId))/(L_PAIR_SIZE));
    return parejas_maximas;
}


RC BTLeafNode::insert(int key, const RecordId& rid)
{ 
    
    if(getKeyCount()+1>getMaxLlaves())
        return RC_NODE_FULL;

    
    int insertIndex;
    localizar(key,insertIndex);
  
    
    char* buffer2=(char*)malloc(PageFile::PAGE_SIZE);
    memset(buffer2, '\0', PageFile::PAGE_SIZE);

    
    if(insertIndex>=0)
        memcpy(buffer2,buffer,insertIndex);

    
    memcpy(buffer2+insertIndex,&key,sizeof(int));
    memcpy(buffer2+insertIndex+sizeof(int),&rid,sizeof(RecordId));

    
    memcpy(buffer2+insertIndex+sizeof(int)+sizeof(RecordId),buffer+insertIndex,(PageFile::PAGE_SIZE-insertIndex-sizeof(int)-sizeof(RecordId)));

    
    memcpy(buffer,buffer2,PageFile::PAGE_SIZE);

    
    free(buffer2);
    
    return 0;
}

RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{ 
    int keyCount=getKeyCount();

    if(getMaxLlaves()>keyCount+1)
        return RC_INVALID_ATTRIBUTE;

    if(sibling.getKeyCount()!=0)
        return RC_INVALID_ATTRIBUTE;

    
    int insertIndex;
    localizar(key,insertIndex);

    
    char* buffer2=(char*)malloc(2*(PageFile::PAGE_SIZE));
    memset(buffer2, '\0', (2*PageFile::PAGE_SIZE));

    
    memcpy(buffer2,buffer,insertIndex);

    
    memcpy(buffer2+insertIndex,&key,sizeof(int));
    memcpy(buffer2+insertIndex+sizeof(int),&rid,sizeof(RecordId));
    
    
    memcpy(buffer2+insertIndex+sizeof(int)+sizeof(RecordId),buffer+insertIndex,(PageFile::PAGE_SIZE-insertIndex));

    
    double dKey=keyCount+1;
    double first=ceil((dKey)/2);

    int splitIndex=first*L_PAIR_SIZE;

    
    memcpy(buffer,buffer2,splitIndex);

    
    memcpy(sibling.buffer,buffer2+splitIndex,PageFile::PAGE_SIZE+L_PAIR_SIZE-splitIndex);

    
    memset(buffer+splitIndex,'\0',PageFile::PAGE_SIZE-splitIndex);

    
    memset(sibling.buffer+(PageFile::PAGE_SIZE+L_PAIR_SIZE-splitIndex),'\0',splitIndex-L_PAIR_SIZE);

    
    free(buffer2);

    
    memcpy(&siblingKey,sibling.buffer,sizeof(int));
    
    return 0; 
}

RC BTLeafNode::localizar(int buscarLlave, int& codigo)
{ 
    int i=0;
    int cantidadLlaves=getKeyCount();
    if(cantidadLlaves==0)
    {
        codigo=i*L_PAIR_SIZE;
        return RC_NO_SUCH_RECORD;
    }

    for(i=0;i<cantidadLlaves;i++)
    {
        int llave_actual;
        memcpy(&llave_actual,buffer+(i*L_PAIR_SIZE),sizeof(int));
       
        if(buscarLlave==llave_actual)
        {
            codigo=i*L_PAIR_SIZE;
            return 0;
        }
        else if(buscarLlave<llave_actual)
        {
            codigo=i*L_PAIR_SIZE;
            return RC_NO_SUCH_RECORD;
        }
    }


    codigo=i*L_PAIR_SIZE;
    
    return RC_NO_SUCH_RECORD; 
}

RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{ 
    if(eid<0)
        return RC_NO_SUCH_RECORD;
    if(eid>=(getKeyCount()*L_PAIR_SIZE))
        return RC_NO_SUCH_RECORD;

    int shift=eid;
    memcpy(&key,buffer+shift,sizeof(int));
    memcpy(&rid,buffer+shift+sizeof(int),sizeof(RecordId));

    return 0;
}

PageId BTLeafNode::getNextNodePtr()
{ 
    PageId pid;
    memcpy(&pid, buffer+(getKeyCount()*L_PAIR_SIZE),sizeof(PageId));
    return pid;
}

RC BTLeafNode::setNextNodePtr(PageId pid)
{
    if(pid<0)
        return RC_INVALID_PID;
    memcpy(buffer+(getKeyCount()*L_PAIR_SIZE),&pid,sizeof(PageId));
    return 0;
}

void BTLeafNode::print()
{
    char* temp=buffer;
    for(int i=0;i<getKeyCount();i++)
    {
        int tempInt;
        memcpy(&tempInt,temp,sizeof(int));
        cout<<"key: "<<tempInt<<endl;
        temp+=L_PAIR_SIZE;
    }
}


BTNonLeafNode::BTNonLeafNode()
{
    
    memset(buffer, '\0', PageFile::PAGE_SIZE);
    memset(buffer,0,sizeof(int));
}


RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{ 
    return pf.read(pid,buffer);
}
    

RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{ 
    return pf.write(pid,buffer);
}


int BTNonLeafNode::getKeyCount()
{ 
    int numKeys;
    memcpy(&numKeys,buffer,sizeof(int));
    return numKeys;
}


int BTNonLeafNode::getMaxLlaves()
{
    
    int parejas_maximas=floor((PageFile::PAGE_SIZE-sizeof(int)-sizeof(PageId))/(NL_PAIR_SIZE));
    return parejas_maximas-1;
}


RC BTNonLeafNode::insert(int key, PageId pid)
{ 
    int numKeys=getKeyCount();
    
    if(numKeys+1>getMaxLlaves())
        return RC_NODE_FULL;

    
    int insertIndex;
    localizar(key,insertIndex);

    
    char* buffer2=(char*)malloc(PageFile::PAGE_SIZE);
    memset(buffer2, '\0', PageFile::PAGE_SIZE);

    
    memcpy(buffer2,buffer,insertIndex);

    
    memcpy(buffer2+insertIndex,&key,sizeof(int));
    memcpy(buffer2+insertIndex+sizeof(int),&pid,sizeof(PageId));

    
    memcpy(buffer2+insertIndex+sizeof(int)+sizeof(PageId),buffer+insertIndex,(PageFile::PAGE_SIZE-insertIndex-sizeof(int)-sizeof(PageId)));

    
    memcpy(buffer,buffer2,PageFile::PAGE_SIZE);

    
    free(buffer2);

    
    numKeys++;
    memcpy(buffer,&numKeys,sizeof(int));
    
    
    return 0;
}

RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{ 

    int keyCount=getKeyCount();

    if(getMaxLlaves()>keyCount+1)
        return RC_INVALID_ATTRIBUTE;

    
    if (sibling.getKeyCount()!=0)
        return RC_INVALID_ATTRIBUTE;

    
    int insertIndex;
    localizar(key,insertIndex);

    
    char* buffer2=(char*)malloc(2*(PageFile::PAGE_SIZE));
    memset(buffer2, '\0', (2*PageFile::PAGE_SIZE));

    
    memcpy(buffer2,buffer,insertIndex);

    
    memcpy(buffer2+insertIndex,&key,sizeof(int));
    memcpy(buffer2+insertIndex+sizeof(int),&pid,sizeof(PageId));
    
    
    memcpy(buffer2+insertIndex+sizeof(int)+sizeof(PageId),buffer+insertIndex,(PageFile::PAGE_SIZE-insertIndex));
 
    
    double dKey=keyCount;

    double first=ceil((dKey+1)/2);

    
    int splitIndex=(first*NL_PAIR_SIZE)+sizeof(int);

    
    memcpy(buffer,buffer2,splitIndex);

    
    memcpy(sibling.buffer+sizeof(int),buffer2+splitIndex,PageFile::PAGE_SIZE+NL_PAIR_SIZE-splitIndex);
    
   
    memset(buffer+splitIndex,'\0',PageFile::PAGE_SIZE-splitIndex);

    
    memset(sibling.buffer+(PageFile::PAGE_SIZE+NL_PAIR_SIZE-splitIndex+sizeof(PageId)),'\0',splitIndex-NL_PAIR_SIZE-sizeof(PageId));

    
    int siblingNumKey=keyCount+1-first;
    memcpy(sibling.buffer,&siblingNumKey,sizeof(int));

    
    int newKey=first-1;
    memcpy(buffer,&newKey,sizeof(int));

    
    free(buffer2);

    
    int midKeyPos=sizeof(int)+((first-1)*NL_PAIR_SIZE)+sizeof(PageId);
    memcpy(&midKey,buffer+midKeyPos,sizeof(int));

    
    memset(buffer+midKeyPos,'\0',sizeof(int));

    return 0;
}


RC BTNonLeafNode::localizar(int buscarLlave, int& codigo)
{ 
    
    int i;

    for(i=0;i<getKeyCount();i++)
    {
        int llave_actual;
        
        memcpy(&llave_actual,buffer+(i*NL_PAIR_SIZE)+sizeof(PageId)+sizeof(int),sizeof(int));
        if(buscarLlave==llave_actual)
        {
            codigo=(i*NL_PAIR_SIZE)+sizeof(PageId)+sizeof(int);
            return 0;
        }
        else if(buscarLlave<llave_actual)
        {
            codigo=(i*NL_PAIR_SIZE)+sizeof(PageId)+sizeof(int);
            return RC_NO_SUCH_RECORD;
        }
    }
    
    if(i<getMaxLlaves())
    {
        codigo=(i*NL_PAIR_SIZE)+sizeof(PageId)+sizeof(int);
        return RC_NO_SUCH_RECORD;
    }

    
    codigo=((i)*NL_PAIR_SIZE)+sizeof(PageId)+sizeof(int);

    return RC_NO_SUCH_RECORD; 
}

RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ 
    
    int tmpKey;
    PageId tmpPid;
    int i;

    int numKeys=getKeyCount();
    for(i=0;i<numKeys;i++)
    {
        memcpy(&tmpKey,buffer+(i*NL_PAIR_SIZE)+sizeof(PageId)+sizeof(int),sizeof(int));
        
        if(searchKey<tmpKey)
        {
            memcpy(&tmpPid,buffer+(i*NL_PAIR_SIZE)+sizeof(int),sizeof(PageId));
            pid=tmpPid;
            return 0;
        }
        
        if(i==numKeys-1)
        {
            memcpy(&tmpPid,buffer+((i+1)*NL_PAIR_SIZE)+sizeof(int),sizeof(PageId));
            pid=tmpPid;
            return 0;
        }
    }


    return RC_NO_SUCH_RECORD;
}


RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ 
    
    memset(buffer, '\0', PageFile::PAGE_SIZE);

    
    int numKeys=1;
    memcpy(buffer,&numKeys,sizeof(int));

    
    memcpy(buffer+sizeof(int),&pid1,sizeof(PageId));
    memcpy(buffer+sizeof(int)+sizeof(PageId),&key,sizeof(int));
    memcpy(buffer+sizeof(int)+sizeof(PageId)+sizeof(int),&pid2,sizeof(PageId));

    return 0;
}

void BTNonLeafNode::print()
{
    return;
    char* temp=buffer;
    int keyss=getKeyCount();
    for(int i=0;i<1024;i+=sizeof(int))
    {
        int tempInt;
        memcpy(&tempInt,temp,sizeof(int));
        cout<<tempInt<<" ";
        temp+=sizeof(int);
    }
    cout<<endl;
}