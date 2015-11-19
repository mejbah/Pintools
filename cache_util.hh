#ifndef _CACHE_UTIL_
#define _CACHE_UTIL_

#include "cache.hh"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <assert.h>
#include <math.h>
#include <new>

#define LOG2(x) ((int) rint((log((double) (x))) / (log(2.0))))

class CacheUtil{

private:
  int block_offset; //bits
  int set_index; //bits
  int tag; //bits 


  CacheUtil(){}

public:
#if 1
  static CacheUtil& getInstance(){
    static char buf[sizeof(CacheUtil)];
    static CacheUtil *theOneTrueObject = new (buf) CacheUtil();
    return *theOneTrueObject;
  }
#endif 
  //CacheUtil(){}

  void initialize(){

    int no_of_blocks = (CACHE_SIZE_KB * 1024) / BLOCK_SIZE;
    int no_of_sets = no_of_blocks / ASSOC;
    
    block_offset = LOG2(BLOCK_SIZE);
    set_index  = LOG2(no_of_sets);
    tag = ADDR_SIZE - (set_index + block_offset); 
     
    printf(" %d - %d - %d\n", tag, set_index, block_offset);

  }
  
  unsigned int getSetIndex( unsigned int address ){
    unsigned int setIndex = (address << tag) >> (tag+block_offset);
    return setIndex;    
  }

  unsigned int getTag( unsigned int address ){
    unsigned int tag  = address >> (set_index+block_offset);
    return tag;    
  }

};




#endif
