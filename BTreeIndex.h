/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).xcsfsdfsdfsdfdsfdfsdfsdfsd
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

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
