/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>  
#include "BTreeIndex.h"
#include "BTreeNode.h"

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
	//buffer to store stuff

    char buffer[PageFile::PAGE_SIZE];
    memset(buffer, '\0', PageFile::PAGE_SIZE);
    
    //error variable
    RC rc=pf.open(indexname,mode);
    //check for error
    if(rc)
        return rc;
 
    //first
    if(pf.endPid()<=0)
    {
        rootPid=0;
        treeHeight=0;
    }
    else//rest
    {
        rc=pf.read(0, buffer);

        //check for error
        if(rc)
            return rc;

        memcpy(&treeHeight,buffer+PageFile::PAGE_SIZE-sizeof(int),sizeof(int));
    }

    //if we got here then success
    return 0;
    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
	RC rc=pf.close();
    
    return 0;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
	RC rc;
    //if no root, make one
    if(treeHeight==0)
    {
        //create a root
        //root is leaf node at first
        BTLeafNode newTreeRoot;

        //insert the key and rid into root
        newTreeRoot.insert(key,rid);

        //set rootPid
        rootPid=pf.endPid();

        //root has height of 1
        treeHeight=1;
        memcpy(newTreeRoot.getBuffer()+PageFile::PAGE_SIZE-sizeof(int),&treeHeight,sizeof(int));

        //write data to disk
        newTreeRoot.write(rootPid,pf);
        
        //success
        return 0;

    }
    else//else root exists, insert recursively
    {
        //variables to pass data into on root overflow
        int pKey;
        PageId pPid;

        //attempt to insert recursively
        rc=insertRecursively(key,rid,1,rootPid,pKey,pPid);
        //failure
        if(rc && rc!=1000)
            return rc;

        //success
        if(!rc)
            return rc;

        //overflow at node level
        if(rc==1000)
        {
            if(treeHeight==1)//root is leaf
            {
                BTLeafNode newLeaf;
                newLeaf.read(rootPid,pf);
                PageId newLeafPid=pf.endPid();
                newLeaf.setNextNodePtr(pPid);
                newLeaf.write(newLeafPid,pf);
                
                BTNonLeafNode rootNode;
                rootNode.initializeRoot(newLeafPid,pKey,pPid);

                //increment treeHeight
                treeHeight+=1;
                memcpy(rootNode.getBuffer()+PageFile::PAGE_SIZE-sizeof(int),&treeHeight,sizeof(int));

                //write data to disk
                rootNode.write(rootPid,pf);

            }
            else//root is nonleaf 
            {
                BTNonLeafNode newNode;
                newNode.read(rootPid,pf);
                PageId newNodePid=pf.endPid();
                newNode.write(newNodePid,pf);
                
                BTNonLeafNode rootNode;
                rootNode.initializeRoot(newNodePid,pKey,pPid);

                //increment treeHeight
                treeHeight+=1;
                memcpy(rootNode.getBuffer()+PageFile::PAGE_SIZE-sizeof(int),&treeHeight,sizeof(int));
                //write data to disk
                rootNode.write(rootPid,pf);
            }

            //success
            return 0;
        }

    }

    //
    return -1;
    return 0;
}

/**
 * Run the standard B+Tree key search algorithm and identify the
 * leaf node where searchKey may exist. If an index entry with
 * searchKey exists in the leaf node, set IndexCursor to its location
 * (i.e., IndexCursor.pid = PageId of the leaf node, and
 * IndexCursor.eid = the searchKey index entry number.) and return 0.
 * If not, set IndexCursor.pid = PageId of the leaf node and
 * IndexCursor.eid = the index entry immediately after the largest
 * index key that is smaller than searchKey, and return the error
 * code RC_NO_SUCH_RECORD.
 * Using the returned "IndexCursor", you will have to call readForward()
 * to retrieve the actual (key, rid) pair from the index.
 * @param key[IN] the key to find
 * @param cursor[OUT] the cursor pointing to the index entry with
 *                    searchKey or immediately behind the largest key
 *                    smaller than searchKey.
 * @return 0 if searchKey is found. Othewise an error code
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
    return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
    return 0;
}

RC BTreeIndex::insertRecursively(int key, const RecordId& rid, int currHeight, PageId pid, int& pKey, PageId& pPid)
{
    RC rc;
    BTLeafNode currLeaf;
    int sibKey;
    PageId sibPid;
    BTNonLeafNode nonLeaf;
    PageId nextPid;
    //the current node is leaf node
    if(currHeight==treeHeight)
    {
        BTLeafNode sibNode;
        //obtain current leaf node data
        currLeaf.read(pid,pf);

        //insertion attempt
        rc=currLeaf.insert(key,rid);
        if(!rc)
        {
            //success: write and return
            if(treeHeight==1)
                memcpy(currLeaf.getBuffer()+PageFile::PAGE_SIZE-sizeof(int),&treeHeight,sizeof(int));
            currLeaf.write(pid,pf);
            return rc;
        }
        //failure: overflow
        //insertAndSplit since overflow
         if(treeHeight==1){
            int tempnum=0; 
            memcpy(currLeaf.getBuffer()+PageFile::PAGE_SIZE-sizeof(int),&tempnum,sizeof(int));
            }
        rc=currLeaf.insertAndSplit(key,rid,sibNode,sibKey);

        //return on failure
        if(rc)
            return rc;

        //successful insertAndSplit
        //insertion into parent node will be done by caller

        //set nextNodePtrs
        //insertAndSplit automatically transfers the next node ptr to the sibling node

        sibPid=pf.endPid();

        //set current leaf node's next leaf pid
        currLeaf.setNextNodePtr(sibPid);
       
        //update current leaf data
        currLeaf.write(pid,pf);

        //update sibling leaf data
        sibNode.write(sibPid,pf);

        //transfer over to p variables
        pKey=sibKey;
        pPid=sibPid;
        
        //successful but need to tell caller to insert into parent
        return 1000;

    }
    else//nonleaf node, continue traverasl
    {
        BTNonLeafNode sibNode;
        //locate child so we can recursively call on it
        nonLeaf.read(pid,pf);

        rc=nonLeaf.locateChildPtr(key,nextPid);

        //return on failure
        if(rc)
            return rc;

        //insert recursively into child
        rc=insertRecursively(key,rid,currHeight+1,nextPid,pKey,pPid);

        //insertion failed
        if(rc && rc!=1000)
            return rc;

        //insertion successful but split occured, handle any overflow insertions
        if(rc==1000)
        {
            //attempt insertion
            rc=nonLeaf.insert(pKey,pPid);
            if(!rc)//success
            {
                if(pid==0)
                    memcpy(nonLeaf.getBuffer()+PageFile::PAGE_SIZE-sizeof(int),&treeHeight,sizeof(int));
                nonLeaf.write(pid,pf);
                return 0;
            }
            else//insertion failure: overflow
            {
                nonLeaf.insertAndSplit(pKey,pPid,sibNode,sibKey);
                sibPid=pf.endPid();
                //write data
                sibNode.write(sibPid,pf);
                nonLeaf.write(pid,pf);

                //transfer data to p variables
                pKey=sibKey;
                pPid=sibPid;

                return 1000;
            }
        }
    
        //success
        return rc;
    }

}
