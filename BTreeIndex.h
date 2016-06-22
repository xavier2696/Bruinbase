#ifndef BTREEINDEX_H
#define BTREEINDEX_H

#include "Bruinbase.h"
#include "PageFile.h"
#include "RecordFile.h"

typedef struct {
  
  PageId  pid;  
  
  int     eid;  
} IndexCursor;

class BTreeIndex {
 public:
  BTreeIndex();


  RC open(const std::string& indexname, char mode);

  RC close();

  RC insert(int key, const RecordId& rid);
  RC insertarRecursivo(int llave, const RecordId& id_record, int altura, PageId id_pagina, int& pllave, PageId& p_id_pagina);

  RC locate(int searchKey, IndexCursor& cursor);

  
  RC readForward(IndexCursor& cursor, int& key, RecordId& rid);
  
 private:
  PageFile pf;        

  PageId   rootPid;    
  int      treeHeight; 

};

#endif
