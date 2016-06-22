#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>  
#include "BTreeIndex.h"
#include "BTreeNode.h"

using namespace std;

BTreeIndex::BTreeIndex()
{
    rootPid = -1;
}

RC BTreeIndex::open(const string& indexname, char mode)
{
	

    char buffer[PageFile::PAGE_SIZE];
    memset(buffer, '\0', PageFile::PAGE_SIZE);
    
    RC rc=pf.open(indexname,mode);
    
    if(rc)
        return rc;
 
    
    if(pf.endPid()<=0)
    {
        rootPid=0;
        treeHeight=0;
    }
    else
    {
        rc=pf.read(0, buffer);

    
        if(rc)
            return rc;

        memcpy(&treeHeight,buffer+PageFile::PAGE_SIZE-sizeof(int),sizeof(int));
    }

    
    return 0;
    return 0;
}

RC BTreeIndex::close()
{
	RC rc=pf.close();
    
    return 0;
}

RC BTreeIndex::insert(int key, const RecordId& rid)
{
	RC rc;
    
    if(treeHeight==0)
    {
        
        BTLeafNode newTreeRoot;

        
        newTreeRoot.insert(key,rid);

        
        rootPid=pf.endPid();

        
        treeHeight=1;
        memcpy(newTreeRoot.getBuffer()+PageFile::PAGE_SIZE-sizeof(int),&treeHeight,sizeof(int));

        
        newTreeRoot.write(rootPid,pf);
        
        
        return 0;

    }
    else
    {
        
        int pKey;
        PageId pPid;

        
        rc=insertarRecursivo(key,rid,1,rootPid,pKey,pPid);
        
        if(rc && rc!=1000)
            return rc;

        
        if(!rc)
            return rc;

        
        if(rc==1000)
        {
            if(treeHeight==1)
            {
                BTLeafNode newLeaf;
                newLeaf.read(rootPid,pf);
                PageId newLeafPid=pf.endPid();
                newLeaf.setNextNodePtr(pPid);
                newLeaf.write(newLeafPid,pf);
                
                BTNonLeafNode rootNode;
                rootNode.initializeRoot(newLeafPid,pKey,pPid);

                
                treeHeight+=1;
                memcpy(rootNode.getBuffer()+PageFile::PAGE_SIZE-sizeof(int),&treeHeight,sizeof(int));

                
                rootNode.write(rootPid,pf);

            }
            else
            {
                BTNonLeafNode newNode;
                newNode.read(rootPid,pf);
                PageId newNodePid=pf.endPid();
                newNode.write(newNodePid,pf);
                
                BTNonLeafNode rootNode;
                rootNode.initializeRoot(newNodePid,pKey,pPid);


                treeHeight+=1;
                memcpy(rootNode.getBuffer()+PageFile::PAGE_SIZE-sizeof(int),&treeHeight,sizeof(int));

                rootNode.write(rootPid,pf);
            }

            return 0;
        }

    }

    return -1;
    return 0;
}


RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
    return 0;
}

RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
    return 0;
}

RC BTreeIndex::insertarRecursivo(int llave, const RecordId& id_record, int altura, PageId id_pagina, int& pllave, PageId& p_id_pagina)
{
    RC rc;
    BTLeafNode currLeaf;
    int sibKey;
    PageId sibPid;
    BTNonLeafNode nonLeaf;
    PageId nextPid;
    
    if(altura==treeHeight)
    {
        BTLeafNode sibNode;
        
        currLeaf.read(id_pagina,pf);

        
        rc=currLeaf.insert(llave,id_record);
        if(!rc)
        {
            
            if(treeHeight==1)
                memcpy(currLeaf.getBuffer()+PageFile::PAGE_SIZE-sizeof(int),&treeHeight,sizeof(int));
            currLeaf.write(id_pagina,pf);
            return rc;
        }
        
         if(treeHeight==1){
            int tempnum=0; 
            memcpy(currLeaf.getBuffer()+PageFile::PAGE_SIZE-sizeof(int),&tempnum,sizeof(int));
            }
        rc=currLeaf.insertAndSplit(llave,id_record,sibNode,sibKey);

        
        if(rc)
            return rc;

        sibPid=pf.endPid();

        currLeaf.setNextNodePtr(sibPid);
       
        currLeaf.write(id_pagina,pf);

        
        sibNode.write(sibPid,pf);

        
        pllave=sibKey;
        p_id_pagina=sibPid;
        
        
        return 1000;

    }
    else
    {
        BTNonLeafNode sibNode;
        
        nonLeaf.read(id_pagina,pf);

        rc=nonLeaf.locateChildPtr(llave,nextPid);

        
        if(rc)
            return rc;

        rc=insertarRecursivo(llave,id_record,altura+1,nextPid,pllave,p_id_pagina);

        if(rc && rc!=1000)
            return rc;

        if(rc==1000)
        {

            rc=nonLeaf.insert(pllave,p_id_pagina);
            if(!rc)
            {
                if(id_pagina==0)
                    memcpy(nonLeaf.getBuffer()+PageFile::PAGE_SIZE-sizeof(int),&treeHeight,sizeof(int));
                nonLeaf.write(id_pagina,pf);
                return 0;
            }
            else
            {
                nonLeaf.insertAndSplit(pllave,p_id_pagina,sibNode,sibKey);
                sibPid=pf.endPid();
                
                sibNode.write(sibPid,pf);
                nonLeaf.write(id_pagina,pf);

                
                pllave=sibKey;
                p_id_pagina=sibPid;

                return 1000;
            }
        }
    
        
        return rc;
    }

}
