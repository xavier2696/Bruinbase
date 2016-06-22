#ifndef BTREENODE_H
#define BTREENODE_H

#include "RecordFile.h"
#include "PageFile.h"

class BTLeafNode {
  public:
    
    BTLeafNode();

    RC insert(int key, const RecordId& rid);

    RC insertAndSplit(int key, const RecordId& rid, BTLeafNode& sibling, int& siblingKey);

    RC localizar(int buscarLlave, int& codigo);

    RC readEntry(int eid, int& key, RecordId& rid);

    PageId getNextNodePtr();

    RC setNextNodePtr(PageId pid);

    int getKeyCount();

     int getMaxLlaves();

    RC read(PageId pid, const PageFile& pf);
    
    RC write(PageId pid, PageFile& pf);

    void print();
    char* getBuffer()
    {
        return buffer;
    }
  private:

    char buffer[PageFile::PAGE_SIZE];
}; 

class BTNonLeafNode {
  public:

    BTNonLeafNode();

    RC insert(int key, PageId pid);

    RC insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey);

    RC locateChildPtr(int searchKey, PageId& pid);

    RC initializeRoot(PageId pid1, int key, PageId pid2);

    int getKeyCount();

    int getMaxLlaves();

     RC localizar(int buscarLlave, int& codigo);

    RC read(PageId pid, const PageFile& pf);
    
    RC write(PageId pid, PageFile& pf);

    char* getBuffer()
    {
        return buffer;
    }
    void print();
  private:
    char buffer[PageFile::PAGE_SIZE];
}; 

#endif 